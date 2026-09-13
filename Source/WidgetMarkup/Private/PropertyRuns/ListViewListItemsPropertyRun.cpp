// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "PropertyRuns/ListViewListItemsPropertyRun.h"

#include "Binding/WidgetPropertyBindingCollection.h"
#include "Binding/WidgetPropertyBindingUtility.h"
#include "PropertyRuns/ObjectNamePropertyRun.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintExtension.h"
#include "Components/ListView.h"
#include "PropertyBuffer.h"
#include "ElementNodes/PropertyElementNode.h"
#include "Extensions/WidgetMarkupBlueprintExtension.h"
#include "Styles/WidgetStyleSheet.h"
#include "UObject/UnrealType.h"

TSharedRef<IPropertyRun> FListViewListItemsPropertyRun::Create()
{
	return MakeShared<FListViewListItemsPropertyRun>();
}

FElementNode::FResult FListViewListItemsPropertyRun::OnBegin(FElementNode::FContext& Context, UObject* Object, const FStringView& /*PropertyName*/, const FStringView& PropertyValue)
{
	UListView* ListView = Cast<UListView>(Object);
	if (!ListView)
	{
		const FString ObjectTypeName = Object ? Object->GetClass()->GetName() : TEXT("null");
		return FElementNode::FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("ListItems property target must be a UListView (or subclass), but got '{0}'.")),
			FText::FromString(ObjectTypeName)));
	}

	FWidgetPropertyBindingToken BindingToken;
	if (Context.HasMetaData<FWidgetPropertyAttributeValueScope>()
		&& TryParseWidgetPropertyBindingToken(PropertyValue, BindingToken))
	{
		FText NameError;
		if (!FObjectNamePropertyMetaData::IsWidgetMarkupObjectNameRegistered(Context, ListView)
			&& !FObjectNamePropertyMetaData::TryApplyGeneratedWidgetMarkupObjectName(Context, ListView, NameError))
		{
			return FElementNode::FResult::Failure().Error(NameError);
		}

		FWidgetPropertyBinding Binding = FWidgetPropertyBinding::Create(
			BindingToken.SourceExpression,
			ListView->GetFName(),
			FWidgetPropertyPath(TEXT("ListItems")));
		Context.GetOrAddMetaData<FWidgetPropertyBindingCollection>()->Bindings.Add(Binding);
		PropertyElementNode.Reset();
		return FElementNode::FResult::Success();
	}

	// The children are assembled by the generic property-element path, which writes
	// them into the widget tree template. No buffered root value is used: ListItems
	// holds object references, and a string cannot produce one.
	const FString LiteralPropertyValue = UnescapeWidgetPropertyBindingLiteral(PropertyValue);
	TSharedPtr<FPropertyElementNode> NewPropertyElementNode = MakeShared<FPropertyElementNode>(TEXT("ListItems"), LiteralPropertyValue, false);
	FElementNode::FResult Result = NewPropertyElementNode->Begin(Context, ListView, nullptr);
	if (!Result)
	{
		return Result;
	}

	NewPropertyElementNode->SetPropertyRun(this->AsShared().ToSharedPtr());
	PropertyElementNode = NewPropertyElementNode;
	Context.Push(NewPropertyElementNode.ToSharedRef());
	return FElementNode::FResult::Success();
}

FElementNode::FResult FListViewListItemsPropertyRun::OnEnd(FElementNode::FContext& Context)
{
	if (!PropertyElementNode.IsValid())
	{
		return FElementNode::FResult::Success();
	}

	UListView* ListView = Context.FindObject<UListView>();
	if (!ListView)
	{
		Context.Pop();
		PropertyElementNode.Reset();
		return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Failed to finalize ListItems property: no UListView object was found in context.")));
	}

	// Let the property element node assemble its child elements into the ListView on
	// the widget tree template before the result is snapshotted.
	FElementNode::FResult Result = PropertyElementNode->End();
	Context.Pop();
	PropertyElementNode.Reset();
	if (!Result)
	{
		return Result;
	}

	// ListItems is transient runtime state, so the value that was just written into
	// the template never reaches a widget instance. Snapshot it and defer it as a
	// per-widget style assignment instead, which is applied to every instance.
	FArrayProperty* ListItemsProperty = FindFProperty<FArrayProperty>(UListView::StaticClass(), TEXT("ListItems"));
	const FObjectPropertyBase* ItemsInnerProperty = ListItemsProperty ? CastField<FObjectPropertyBase>(ListItemsProperty->Inner) : nullptr;
	if (!ListItemsProperty || !ItemsInnerProperty)
	{
		return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Failed to finalize ListItems property: UListView::ListItems is not an object array.")));
	}

	void* ListItemsAddress = ListItemsProperty->ContainerPtrToValuePtr<void>(ListView);
	if (!ListItemsAddress)
	{
		return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Failed to finalize ListItems property: could not resolve the ListItems value address.")));
	}

	const FPropertyBuffer ListItemsSnapshot(ListItemsProperty);
	if (!ListItemsSnapshot.HasValue())
	{
		return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Failed to finalize ListItems property: could not allocate a value buffer.")));
	}
	ListItemsProperty->CopyCompleteValue(ListItemsSnapshot.GetValueData(), ListItemsAddress);

	// Store the snapshot as a single FWidgetStyleEntry in the default StyleSheet.
	UWidgetBlueprint* WidgetBlueprint = Context.FindObject<UWidgetBlueprint>();
	if (!WidgetBlueprint)
	{
		return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Failed to finalize ListItems property: no WidgetBlueprint was found in context.")));
	}

	UWidgetMarkupBlueprintExtension* WidgetMarkupBlueprintExtension = UWidgetBlueprintExtension::GetExtension<UWidgetMarkupBlueprintExtension>(WidgetBlueprint);
	if (!WidgetMarkupBlueprintExtension)
	{
		return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Failed to finalize ListItems property: the WidgetMarkup blueprint extension is missing.")));
	}

	FWidgetStyleEntry Entry;
	Entry.TargetType = UListView::StaticClass()->GetFName();
	Entry.Name = FName(*FString::Printf(TEXT("ListItemsDeferredSetter_%s"), *ListView->GetName()));
	FWidgetStyleSetter Setter;
	Setter.Property = FWidgetPropertyPath(TEXT("ListItems"));
	Setter.Buffer = ListItemsSnapshot;
	Entry.Setters.Add(Setter);
	WidgetMarkupBlueprintExtension->GetStyleSheet()->AddOrReplaceStyleEntry(Entry);
	WidgetMarkupBlueprintExtension->AddWidgetStyleAssignment(ListView->GetFName(), Entry.Name);
	return FElementNode::FResult::Success();
}
