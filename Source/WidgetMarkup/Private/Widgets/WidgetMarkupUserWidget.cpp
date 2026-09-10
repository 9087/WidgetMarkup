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
	if (!Widget)
	{
		UE_LOG(LogTemp, Warning, TEXT("WidgetMarkup: BindOnPointerEvent FAILED (null widget, delegate=%s)."), *DelegateName.ToString());
		return false;
	}

	FProperty* Property = Widget->GetClass()->FindPropertyByName(DelegateName);
	if (!Property)
	{
		UE_LOG(LogTemp, Warning, TEXT("WidgetMarkup: BindOnPointerEvent FAILED (property '%s' not found on '%s')."),
			*DelegateName.ToString(), *Widget->GetClass()->GetName());
		return false;
	}

	UWidgetMarkupOnPointerEventDelegate* Delegate = NewObject<UWidgetMarkupOnPointerEventDelegate>(Widget);
	Delegate->TargetDelegate = NewDelegate;

	FScriptDelegate ScriptDelegate;
	ScriptDelegate.BindUFunction(Delegate,
		GET_FUNCTION_NAME_CHECKED(UWidgetMarkupOnPointerEventDelegate, HandlePointerEvent));

	bool bBoundReflected = false;
	if (const FMulticastDelegateProperty* MulticastDelegate = CastField<FMulticastDelegateProperty>(Property))
	{
		const_cast<FMulticastScriptDelegate*>(
			MulticastDelegate->GetMulticastDelegate(Widget))->AddUnique(ScriptDelegate);
		bBoundReflected = true;
	}
	else if (const FDelegateProperty* SingleDelegate = CastField<FDelegateProperty>(Property))
	{
		FScriptDelegate* DelegateSlot = SingleDelegate->GetPropertyValuePtr_InContainer(Widget);
		if (!DelegateSlot)
		{
			UE_LOG(LogTemp, Warning, TEXT("WidgetMarkup: BindOnPointerEvent FAILED (no single-delegate slot for '%s')."), *DelegateName.ToString());
			return false;
		}
		*DelegateSlot = ScriptDelegate;
		bBoundReflected = true;
	}

	if (!bBoundReflected)
	{
		UE_LOG(LogTemp, Warning, TEXT("WidgetMarkup: BindOnPointerEvent FAILED (property '%s' is not a delegate)."), *DelegateName.ToString());
		return false;
	}

	// Keep the delegate alive across GC (the reflection binding only holds a weak
	// reference), then rely on the UMG reflection chain to invoke it.
	PointerEventDelegateKeepAlive.Add(Delegate);

	return true;
}

FEventReply FWidgetMarkupEventReply::ToReply() const
{
	FEventReply OutReply;

	OutReply.NativeReply = bIsHandled ? FReply::Handled() : FReply::Unhandled();

	if (MouseCaptor)
	{
		if (const TSharedPtr<SWidget> SlateWidget = MouseCaptor->GetCachedWidget())
		{
			OutReply.NativeReply.CaptureMouse(SlateWidget.ToSharedRef());
		}
	}

	if (MouseLock)
	{
		if (const TSharedPtr<SWidget> SlateWidget = MouseLock->GetCachedWidget())
		{
			OutReply.NativeReply.LockMouseToWidget(SlateWidget.ToSharedRef());
		}
	}

	if (FocusRecipient)
	{
		if (const TSharedPtr<SWidget> SlateWidget = FocusRecipient->GetCachedWidget())
		{
			OutReply.NativeReply.SetUserFocus(SlateWidget.ToSharedRef(), EFocusCause::SetDirectly);
		}
	}

	if (bReleaseMouseCapture)
	{
		OutReply.NativeReply.ReleaseMouseCapture();
	}

	if (bShouldSetMousePos)
	{
		OutReply.NativeReply.SetMousePos(FIntPoint(FMath::RoundToInt32(RequestedMousePos.X), FMath::RoundToInt32(RequestedMousePos.Y)));
	}

	return OutReply;
}

FEventReply UWidgetMarkupOnPointerEventDelegate::HandlePointerEvent(FGeometry Geometry, FPointerEvent PointerEvent)
{
	FWidgetMarkupPointerEvent WidgetMarkupEvent;
	WidgetMarkupEvent.EffectingButton = PointerEvent.GetEffectingButton();
	WidgetMarkupEvent.PressedButtons = PointerEvent.GetPressedButtons().Array();
	WidgetMarkupEvent.ScreenSpacePosition = PointerEvent.GetScreenSpacePosition();

	UWidgetMarkupOnPointerEventPayload* Payload = NewObject<UWidgetMarkupOnPointerEventPayload>(this);
	Payload->Geometry = FWidgetMarkupGeometry(Geometry);
	Payload->MouseEvent = MoveTemp(WidgetMarkupEvent);

	TargetDelegate.ExecuteIfBound(Payload);

	return Payload->Reply.ToReply();
}

