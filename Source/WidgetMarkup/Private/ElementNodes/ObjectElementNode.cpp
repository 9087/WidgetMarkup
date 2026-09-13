// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "ObjectElementNode.h"

#include "ElementNodes/PropertyChainHandle.h"
#include "PropertyElementNode.h"
#include "UObject/UObjectGlobals.h"

IMPLEMENT_ELEMENT_NODE(FObjectElementNode, FElementNode)

UObject* FObjectElementNode::GetObject() const
{
	return Object;
}

UStruct* FObjectElementNode::GetPropertyOwnerStruct() const
{
	return Object ? Object->GetClass() : nullptr;
}

void FObjectElementNode::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(Object);
}

FString FObjectElementNode::GetReferencerName() const
{
	return TEXT("ObjectElementNode");
}

void FObjectElementNode::SetElementData(const TCHAR* InElementData)
{
	if (InElementData)
	{
		ObjectPath = FString(InElementData).TrimStartAndEnd();
	}
}

FElementNode::FResult FObjectElementNode::OnBegin(const FContext& Context, UObject* Outer, UStruct* Struct)
{
	if (!ensure(Struct->IsA<UClass>()))
	{
		return FResult::Failure().Error(FText::FromString(TEXT("ObjectElementNode: struct type must be a UClass when creating an object.")));
	}
	auto Class = CastChecked<UClass>(Struct);

	if (Class->HasAnyClassFlags(CLASS_Abstract))
	{
		return FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("ObjectElementNode: class '{0}' is abstract and cannot be instantiated. Element data (an asset path) could reference an existing instance instead.")),
			FText::FromString(Class->GetName())));
	}

	if (!ObjectPath.IsEmpty())
	{
		// Path-reference mode: load existing asset.
		Object = StaticLoadObject(Class, Outer, *ObjectPath);
		if (!Object)
		{
			return FResult::Failure().Error(FText::Format(
				FText::FromString(TEXT("ObjectElementNode: failed to load object of class '{0}' from path '{1}'.")),
				FText::FromString(Class->GetName()),
				FText::FromString(ObjectPath)));
		}
	}
	else
	{
		// Inline-definition mode: create new instance.
		Object = NewObject<UObject>(Outer, Class);
	}
	return FResult::Success();
}

FElementNode::FResult FObjectElementNode::OnEnd()
{
	return FResult::Success();
}

FElementNode::FResult FObjectElementNode::OnAddChild(const TSharedRef<FElementNode>& Child)
{
	return FResult::Success();
}

bool FObjectElementNode::HasProperty(const FStringView& AttributeName)
{
	if (!Object)
	{
		return false;
	}
	return FPropertyChainHandle::Create(Object, AttributeName).IsValid();
}
