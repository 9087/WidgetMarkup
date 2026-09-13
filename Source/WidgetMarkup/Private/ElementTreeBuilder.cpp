// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "ElementTreeBuilder.h"

#include "Binding/WidgetPropertyBindingCollection.h"
#include "ElementNode.h"
#include "ElementNodeFactory.h"
#include "Extensions/WidgetMarkupBlueprintExtension.h"
#include "WidgetMarkupModule.h"
#include "ElementNodes/ObjectElementNode.h"
#include "ElementNodes/PropertyElementNode.h"
#include "Utilities/TypeResolver.h"
#include "Modules/ModuleManager.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintExtension.h"

FElementTreeBuilder::FElementTreeBuilder(UObject* InOuter)
	: Outer(InOuter)
	, WidgetMarkupModule(&FModuleManager::GetModuleChecked<FWidgetMarkupModule>("WidgetMarkup"))
{
}

bool FElementTreeBuilder::ProcessXmlDeclaration(const TCHAR* ElementData, int32 XmlFileLineNumber)
{
	return true;
}

bool FElementTreeBuilder::ProcessElement(const TCHAR* ElementName, const TCHAR* ElementData, int32 XmlFileLineNumber)
{
	UStruct* Struct = nullptr;
	TSharedPtr<FElementNode> ElementNode;

	FString ElementNameString(ElementName);
	ElementNameString.TrimStartAndEndInline();
	ElementNode = FElementNodeFactory::Get().CreateElementNode(Outer, *ElementNameString, ElementData, Struct);

	if (!ElementNode.IsValid())
	{
		TSharedPtr<FElementNode> ObjectNode = Context.GetLastObjectNode();
		if (!ObjectNode.IsValid())
		{
			UE_LOG(LogWidgetMarkup, Warning, TEXT("Unknown element '%s' at line %d."), *ElementNameString, XmlFileLineNumber);
			return false;
		}

		UObject* Object = ObjectNode->GetObject();
		TSharedPtr<FElementNode> ParentNode = Context.GetLastNode();
		UStruct* OwnerStruct = ParentNode.IsValid() ? ParentNode->GetPropertyOwnerStruct() : nullptr;
		TSharedRef<IPropertyRun> PropertyRun = WidgetMarkupModule->CreatePropertyRun(OwnerStruct, FName(*ElementNameString));
		FStringView PropertyName(*ElementNameString);
		FStringView PropertyValue(ElementData ? ElementData : TEXT(""));
		FElementNode::FResult PropertyRunResult = PropertyRun->OnBegin(Context, Object, PropertyName, PropertyValue);
		if (PropertyRunResult)
		{
			return true;
		}

		// Keep the reason: without it the parse just aborts and the log only says
		// "User aborted the parsing process".
		CaptureFirstError(PropertyRunResult);
	}

	if (ElementNode)
	{
		if (ElementData)
		{
			ElementNode->SetElementData(ElementData);
		}
		FElementNode::FResult Result = ElementNode->Begin(Context, Outer, Struct);
		CaptureFirstError(Result);
		if (!Result)
		{
			ElementNode = nullptr;
		}
	}
	if (!ElementNode)
	{
		auto FallbackClass = TTypeResolver<UClass>::Resolve(FStringView(*ElementNameString));
		if (!FallbackClass)
		{
			return false;
		}
		TSharedPtr<FElementNode> FallbackObjectNode = MakeShared<FObjectElementNode>();
		if (ElementData)
		{
			FallbackObjectNode->SetElementData(ElementData);
		}
		auto FallbackResult = FallbackObjectNode->Begin(Context, Outer, FallbackClass);
		CaptureFirstError(FallbackResult);
		if (!FallbackResult)
		{
			FallbackResult.PrintOnFailure();
			return false;
		}
		ElementNode = FallbackObjectNode;
		Struct = FallbackClass;
	}
	if (auto Current = GetCurrentElementNode())
	{
		FElementNode::FResult AddResult = Current->OnAddChild(ElementNode.ToSharedRef());
		CaptureFirstError(AddResult);
		if (!AddResult.PrintOnFailure())
		{
			return false;
		}
	}
	Context.Push(ElementNode.ToSharedRef());
	return true;
}

bool FElementTreeBuilder::ProcessAttribute(const TCHAR* AttributeName, const TCHAR* AttributeValue)
{
	auto Current = GetCurrentElementNode();
	if (!Current.IsValid())
	{
		UE_LOG(LogWidgetMarkup, Warning, TEXT("Attribute '%s' has no open element."), AttributeName);
		return false;
	}

	FStringView PropertyName(AttributeName);
	FStringView PropertyValue(AttributeValue);

	UObject* Object = Current->GetObject();
	if (!Object)
	{
		TSharedPtr<FElementNode> ObjectNode = Context.GetLastObjectNode();
		Object = ObjectNode.IsValid() ? ObjectNode->GetObject() : nullptr;
	}

	TSharedRef<IPropertyRun> PropertyRun = WidgetMarkupModule->CreatePropertyRun(Current->GetPropertyOwnerStruct(), FName(PropertyName));
	Context.AddMetaData<FWidgetPropertyAttributeValueScope>();
	auto Result = PropertyRun->OnBegin(Context, Object, PropertyName, PropertyValue);
	if (Result)
	{
		Result = PropertyRun->OnEnd(Context);
	}

	if (!Result)
	{
		CaptureFirstError(Result);
		UE_LOG(LogWidgetMarkup, Error, TEXT("ProcessAttribute FAILED: '%s'='%s' on class '%s'"),
			*FString(PropertyName), *FString(PropertyValue),
			Object ? *Object->GetClass()->GetName() : TEXT("<null>"));
		Result.PrintOnFailure();
	}

	Context.RemoveMetaData<FWidgetPropertyAttributeValueScope>();
	return !!Result;
}

bool FElementTreeBuilder::ProcessClose(const TCHAR* Element)
{
	auto Current = GetCurrentElementNode();
	if (!Current.IsValid())
	{
		UE_LOG(LogWidgetMarkup, Warning, TEXT("Close element '%s' has no open element."), Element);
		return false;
	}

	if (FPropertyElementNode* PropertyElementNode = CastElementNode<FPropertyElementNode>(Current.Get()))
	{
		FElementNode::FResult PropertyEndResult = PropertyElementNode->GetPropertyRunInternal()->OnEnd(Context);
		CaptureFirstError(PropertyEndResult);
		return PropertyEndResult ? true : false;
	}

	Current = Context.Pop();
	if (Context.IsEmpty())
	{
		if (UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(Current->GetObject()))
		{
			TArray<FWidgetPropertyBinding> PropertyBindings;
			if (TSharedPtr<FWidgetPropertyBindingCollection> BindingCollection = Context.GetMetaData<FWidgetPropertyBindingCollection>())
			{
				PropertyBindings = BindingCollection->Bindings;
			}

			UWidgetMarkupBlueprintExtension* Extension = UWidgetBlueprintExtension::RequestExtension<UWidgetMarkupBlueprintExtension>(WidgetBlueprint);
			if (Extension)
			{
				Extension->SetPropertyBindings(PropertyBindings);
			}
		}
	}
	FElementNode::FResult EndResult = Current->End();
	CaptureFirstError(EndResult);
	if (!EndResult.PrintOnFailure())
	{
		return false;
	}
	if (Context.IsEmpty())
	{
		RootElementNode = Current;
	}
	return true;
}

bool FElementTreeBuilder::ProcessComment(const TCHAR* Comment)
{
	return true;
}

void FElementTreeBuilder::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(Outer);
}

FString FElementTreeBuilder::GetReferencerName() const
{
	return TEXT("WidgetTreeBuilder");
}

TSharedPtr<FElementNode> FElementTreeBuilder::GetRootElementNode()
{
	return RootElementNode;
}

TSharedPtr<FElementNode> FElementTreeBuilder::GetCurrentElementNode() const
{
	return !Context.IsEmpty() ? Context.GetLastNode() : TSharedPtr<FElementNode>();
}

void FElementTreeBuilder::CaptureFirstError(const FElementNode::FResult& Result)
{
	if (FirstErrorText.IsEmpty())
	{
		FirstErrorText = Result.GetFirstErrorText();
	}
}
