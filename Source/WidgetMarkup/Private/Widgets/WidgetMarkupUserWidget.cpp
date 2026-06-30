// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "Widgets/WidgetMarkupUserWidget.h"
#include "Extensions/WidgetMarkupUserWidgetExtension.h"
#include "Components/IWidgetMarkupComponent.h"
#include "Components/ListViewBase.h"
#include "Components/Widget.h"
#include "Input/Events.h"

void UWidgetMarkupUserWidget::NativeOnListItemObjectSet(UObject* ListItemObject)
{
	IUserObjectListEntry::NativeOnListItemObjectSet(ListItemObject);

	// Find THIS entry widget's own WidgetMarkupComponent and notify it.
	if (UWidgetMarkupUserWidgetExtension* Extension = UWidgetMarkupUserWidgetExtension::GetOrAddExtension(this))
	{
		if (const TSharedPtr<IWidgetMarkupComponent>& Component = Extension->GetWidgetMarkupComponent())
		{
			Component->OnDataRefresh(ListItemObject);
		}
	}
}

void UWidgetMarkupUserWidget::NativeOnEntryReleased()
{
	IUserListEntry::NativeOnEntryReleased();
	OnEntryReleased.Broadcast();
}

void UWidgetMarkupUserWidget::NativeOnItemSelectionChanged(bool bIsSelected)
{
	IUserListEntry::NativeOnItemSelectionChanged(bIsSelected);
	OnItemSelectionChanged.Broadcast(bIsSelected);
}

void UWidgetMarkupUserWidget::NativeOnItemExpansionChanged(bool bIsExpanded)
{
	IUserListEntry::NativeOnItemExpansionChanged(bIsExpanded);
	OnItemExpansionChanged.Broadcast(bIsExpanded);
}

UObject* UWidgetMarkupUserWidget::GetEntryListItem() const
{
	return GetListItem<UObject>();
}

bool UWidgetMarkupUserWidget::IsEntrySelected() const
{
	return IsListItemSelected();
}

UListViewBase* UWidgetMarkupUserWidget::GetOwningEntryListView() const
{
	return GetOwningListView();
}

bool UWidgetMarkupUserWidget::IsOnPointerEvent(UWidget* Widget, FName DelegateName)
{
	if (!Widget) return false;

	FProperty* Property = Widget->GetClass()->FindPropertyByName(DelegateName);
	if (!Property) return false;

	UFunction* Signature = nullptr;
	if (const FMulticastDelegateProperty* MulticastDelegate = CastField<FMulticastDelegateProperty>(Property))
		Signature = MulticastDelegate->SignatureFunction;
	else if (const FDelegateProperty* SingleDelegate = CastField<FDelegateProperty>(Property))
		Signature = SingleDelegate->SignatureFunction;
	else
		return false;

	if (!Signature) return false;

	// Check if any parameter is an FPointerEvent (or derived) struct.
	for (TFieldIterator<FProperty> It(Signature); It; ++It)
	{
		if (const FStructProperty* StructProp = CastField<FStructProperty>(*It))
		{
			if (StructProp->Struct->GetFName() == FPointerEvent::StaticStruct()->GetFName())
				return true;
		}
	}

	return false;
}

bool UWidgetMarkupUserWidget::BindOnPointerEvent(UWidget* Widget, FName DelegateName, FWidgetMarkupOnPointerEvent NewDelegate)
{
	if (!Widget) return false;

	FProperty* Property = Widget->GetClass()->FindPropertyByName(DelegateName);
	if (!Property) return false;

	UWidgetMarkupOnPointerEventDelegate* Delegate = NewObject<UWidgetMarkupOnPointerEventDelegate>(Widget);
	Delegate->TargetDelegate = NewDelegate;

	FScriptDelegate ScriptDelegate;
	ScriptDelegate.BindUFunction(Delegate,
		GET_FUNCTION_NAME_CHECKED(UWidgetMarkupOnPointerEventDelegate, HandlePointerEvent));

	if (const FMulticastDelegateProperty* MulticastDelegate = CastField<FMulticastDelegateProperty>(Property))
	{
		const_cast<FMulticastScriptDelegate*>(
			MulticastDelegate->GetMulticastDelegate(Widget))->AddUnique(ScriptDelegate);
		return true;
	}

	if (const FDelegateProperty* SingleDelegate = CastField<FDelegateProperty>(Property))
	{
		FScriptDelegate* DelegateSlot = SingleDelegate->GetPropertyValuePtr_InContainer(Widget);
		if (!DelegateSlot) return false;
		*DelegateSlot = ScriptDelegate;
		return true;
	}

	return false;
}

FEventReply UWidgetMarkupOnPointerEventDelegate::HandlePointerEvent(FGeometry Geometry, FPointerEvent PointerEvent)
{
	FWidgetMarkupPointerEvent WidgetMarkupEvent;
	WidgetMarkupEvent.EffectingButton = PointerEvent.GetEffectingButton();
	WidgetMarkupEvent.PressedButtons = PointerEvent.GetPressedButtons().Array();

	UWidgetMarkupOnPointerEventPayload* Payload = NewObject<UWidgetMarkupOnPointerEventPayload>(this);
	Payload->Geometry = Geometry;
	Payload->MouseEvent = MoveTemp(WidgetMarkupEvent);

	TargetDelegate.ExecuteIfBound(Payload);

	return Payload->Reply;
}

