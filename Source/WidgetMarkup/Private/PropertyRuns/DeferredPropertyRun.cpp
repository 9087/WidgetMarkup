// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "PropertyRuns/DeferredPropertyRun.h"

#include "Binding/WidgetPropertyBindingCollection.h"
#include "Binding/WidgetPropertyBindingUtility.h"
#include "Components/Widget.h"
#include "PropertyBuffer.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintExtension.h"
#include "Extensions/WidgetMarkupBlueprintExtension.h"
#include "Styles/WidgetStyleSheet.h"
#include "UObject/UnrealType.h"

TSharedRef<IPropertyRun> FDeferredPropertyRun::Create()
{
	return MakeShared<FDeferredPropertyRun>();
}

FElementNode::FResult FDeferredPropertyRun::OnBegin(FElementNode::FContext& Context, UObject* Outer, const FStringView& PropertyName, const FStringView& PropertyValue)
{
	DeferredPropertyName = NAME_None;
	TargetWidget = Cast<UWidget>(Outer);

	// A binding is applied to the widget instance itself, so it must not be deferred
	// (a deferred snapshot of the template would overwrite it). Only the literal and
	// property-element forms are captured here.
	FWidgetPropertyBindingToken BindingToken;
	const bool bIsBinding = Context.HasMetaData<FWidgetPropertyAttributeValueScope>()
		&& TryParseWidgetPropertyBindingToken(PropertyValue, BindingToken);

	FElementNode::FResult Result = FPropertyRun::OnBegin(Context, Outer, PropertyName, PropertyValue);
	if (!Result)
	{
		return Result;
	}

	if (bIsBinding)
	{
		return Result;
	}

	// Deferral works by assigning a style to the widget, so the target must be one.
	if (!TargetWidget.IsValid())
	{
		return FElementNode::FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("Deferred property '{0}' is declared on '{1}', which is not a widget, so it cannot be deferred.")),
			FText::FromString(FString(PropertyName)),
			FText::FromString(Outer ? Outer->GetClass()->GetName() : TEXT("null"))));
	}

	DeferredPropertyName = FName(PropertyName);
	return Result;
}

FElementNode::FResult FDeferredPropertyRun::OnEnd(FElementNode::FContext& Context)
{
	// Finish the generic side first: it assembles the child elements into the property
	// on the widget tree template.
	FElementNode::FResult Result = FPropertyRun::OnEnd(Context);
	if (!Result)
	{
		return Result;
	}

	UWidget* Widget = TargetWidget.Get();
	const FName PropertyName = DeferredPropertyName;
	DeferredPropertyName = NAME_None;
	if (!Widget || PropertyName.IsNone())
	{
		return FElementNode::FResult::Success();
	}

	// Snapshot what the template ended up with. ListItems and friends are transient
	// runtime state, so that value is discarded when a user widget instances the tree;
	// deferring it as a per-widget style assignment is what makes it reach instances.
	FProperty* Property = FindFProperty<FProperty>(Widget->GetClass(), PropertyName);
	if (!Property)
	{
		return FElementNode::FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("Deferred property '{0}' does not exist on '{1}'.")),
			FText::FromString(PropertyName.ToString()),
			FText::FromString(Widget->GetClass()->GetName())));
	}

	void* ValueAddress = Property->ContainerPtrToValuePtr<void>(Widget);
	if (!ValueAddress)
	{
		return FElementNode::FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("Deferred property '{0}' has no value address on '{1}'.")),
			FText::FromString(PropertyName.ToString()),
			FText::FromString(Widget->GetClass()->GetName())));
	}

	const FPropertyBuffer ValueSnapshot(Property);
	if (!ValueSnapshot.HasValue())
	{
		return FElementNode::FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("Could not allocate a value buffer for deferred property '{0}'.")),
			FText::FromString(PropertyName.ToString())));
	}
	Property->CopyCompleteValue(ValueSnapshot.GetValueData(), ValueAddress);

	// Store the snapshot as a style setter that is assigned to this widget by name, so
	// the style application writes it into every instance.
	UWidgetBlueprint* WidgetBlueprint = Context.FindObject<UWidgetBlueprint>();
	if (!WidgetBlueprint)
	{
		return FElementNode::FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("Could not defer property '{0}': no WidgetBlueprint was found in context.")),
			FText::FromString(PropertyName.ToString())));
	}

	UWidgetMarkupBlueprintExtension* WidgetMarkupBlueprintExtension = UWidgetBlueprintExtension::GetExtension<UWidgetMarkupBlueprintExtension>(WidgetBlueprint);
	if (!WidgetMarkupBlueprintExtension)
	{
		return FElementNode::FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("Could not defer property '{0}': the WidgetMarkup blueprint extension is missing.")),
			FText::FromString(PropertyName.ToString())));
	}

	FWidgetStyleEntry Entry;
	Entry.TargetType = Widget->GetClass()->GetFName();
	Entry.Name = FName(*FString::Printf(TEXT("%sDeferredSetter_%s"), *PropertyName.ToString(), *Widget->GetName()));
	const FString PropertyPathString = PropertyName.ToString();
	FWidgetStyleSetter Setter;
	Setter.Property = FWidgetPropertyPath(FStringView(PropertyPathString));
	Setter.Buffer = ValueSnapshot;
	Entry.Setters.Add(Setter);
	WidgetMarkupBlueprintExtension->GetStyleSheet()->AddOrReplaceStyleEntry(Entry);
	WidgetMarkupBlueprintExtension->AddWidgetStyleAssignment(Widget->GetFName(), Entry.Name);
	return FElementNode::FResult::Success();
}
