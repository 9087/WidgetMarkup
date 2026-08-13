// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "PropertyElementNode.h"

#include "BasicTypeElementNode.h"
#include "Data/WidgetMarkupKeyValuePair.h"
#include "ObjectElementNode.h"
#include "PropertyBuffer.h"
#include "ElementNodes/PropertyChainHandle.h"
#include "StructElementNode.h"
#include "Misc/ScopeExit.h"
#include "Utilities/WidgetPropertyPath.h"
#include "UObject/UnrealType.h"

IMPLEMENT_ELEMENT_NODE(FPropertyElementNode, FElementNode)

TSharedRef<FElementNode> FPropertyElementNode::Create(const FStringView& InPropertyName, const FStringView& InPropertyValue)
{
	return MakeShared<FPropertyElementNode>(InPropertyName, InPropertyValue);
}

FPropertyElementNode::FPropertyElementNode(const FStringView& InPropertyName, const FStringView& InPropertyValue, bool bInUseBufferedWrite)
	: PropertyName(InPropertyName)
	, PropertyValue(InPropertyValue)
	, bUseBufferedWrite(bInUseBufferedWrite)
{
}

bool FPropertyElementNode::TryResolvePropertyPath(
	const FContext& Context,
	const FStringView& PropertyName,
	bool& bInOutUseBufferedWrite,
	FWidgetPropertyPath& OutPropertyPath,
	FBufferedPropertyContext& OutBufferedPropertyContext,
	FText* OutError)
{
	TSharedPtr<FElementNode> ObjectNode = Context.GetLastObjectNode();
	TSharedPtr<FElementNode> Parent = Context.GetLastNode();
	OutPropertyPath.Reset();
	OutBufferedPropertyContext.Reset();

	if (!Parent.IsValid() || Parent == ObjectNode)
	{
		OutPropertyPath = OutPropertyPath.WithAppendedProperty(PropertyName);
		return true;
	}

	if (TSharedPtr<FStructElementNode> StructParent = CastElementNode<FStructElementNode>(Parent))
	{
		OutPropertyPath = StructParent->GetBufferedPropertyContext().GetRootPropertyPath().WithAppendedProperty(PropertyName);
		OutBufferedPropertyContext = StructParent->GetBufferedPropertyContext();
		return true;
	}

	if (TSharedPtr<FPropertyElementNode> PropertyParent = CastElementNode<FPropertyElementNode>(Parent))
	{
		OutPropertyPath = PropertyParent->PropertyPath;
		OutBufferedPropertyContext = PropertyParent->BufferedPropertyContext;
		if (PropertyParent->PropertyChain && PropertyParent->PropertyChain->IsArrayProperty())
		{
			bInOutUseBufferedWrite = true;
			OutPropertyPath = OutPropertyPath.WithAppendedArrayIndex(PropertyParent->ElementChildren.Num());
			return true;
		}

		OutPropertyPath = OutPropertyPath.WithAppendedProperty(PropertyName);
		return true;
	}

	if (OutError)
	{
		*OutError = FText::Format(
			FText::FromString(TEXT("Expected property, struct, or object parent node for nested property '{0}'.")),
			FText::FromString(FString(PropertyName)));
	}
	return false;
}

void FPropertyElementNode::AddReferencedObjects(FReferenceCollector& Collector)
{
	if (const TSharedPtr<FPropertyBuffer> PropertyBuffer = BufferedPropertyContext.GetPropertyBuffer(); PropertyBuffer.IsValid())
	{
		PropertyBuffer->AddStructReferencedObjects(Collector);
	}

	// Container child snapshots may hold UObject references (e.g. struct children containing object pointers).
	for (const TSharedPtr<const FPropertyBuffer>& Snapshot : ContainerChildSnapshots)
	{
		if (Snapshot.IsValid())
		{
			Snapshot->AddStructReferencedObjects(Collector);
		}
	}
}

FString FPropertyElementNode::GetReferencerName() const
{
	return TEXT("PropertyElementNode");
}

FElementNode::FResult FPropertyElementNode::OnBegin(const FContext& Context, UObject* Outer, UStruct* Struct)
{
	check(!Context.IsEmpty());

	TSharedPtr<FElementNode> ObjectNode = Context.GetLastObjectNode();
	if (!ObjectNode.IsValid())
	{
		return FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("No object node in context for property '{0}'.")),
			FText::FromString(PropertyName)));
	}

	UObject* Object = ObjectNode->GetObject();
	if (!Object)
	{
		return FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("Object is null for property '{0}'.")),
			FText::FromString(PropertyName)));
	}

	PropertyPath.Reset();
	BufferedPropertyContext.Reset();
	FText PropertyPathError;
	if (!TryResolvePropertyPath(Context, PropertyName, bUseBufferedWrite, PropertyPath, BufferedPropertyContext, &PropertyPathError))
	{
		return FResult::Failure().Error(PropertyPathError);
	}

	if (bUseBufferedWrite && BufferedPropertyContext.InValid())
	{
		TSharedPtr<FPropertyChainHandle> DirectPropertyChain = FPropertyChainHandle::Create(Object, PropertyPath);
		FProperty* TailProperty = DirectPropertyChain.IsValid() ? DirectPropertyChain->GetTailProperty() : nullptr;
		if (!TailProperty)
		{
			return FResult::Failure().Error(FText::Format(
				FText::FromString(TEXT("Failed to resolve buffered root property for property path '{0}'.")),
				FText::FromString(PropertyPath.GetPathName().ToString())));
		}

		const TSharedPtr<FPropertyBuffer> PropertyBuffer = MakeShared<FPropertyBuffer>(TailProperty, FStringView(PropertyValue));
		if (!PropertyBuffer->HasValue())
		{
			return FResult::Failure().Error(FText::Format(
				FText::FromString(TEXT("Failed to initialize buffered root value for property path '{0}'.")),
				FText::FromString(PropertyPath.GetPathName().ToString())));
		}

		BufferedPropertyContext = FBufferedPropertyContext(PropertyPath, PropertyBuffer);
		if (BufferedPropertyContext.InValid())
		{
			return FResult::Failure().Error(FText::Format(
				FText::FromString(TEXT("Failed to initialize buffered root value for property path '{0}'.")),
				FText::FromString(PropertyPath.GetPathName().ToString())));
		}
	}

	if (!BufferedPropertyContext.InValid())
	{
		PropertyChain = FPropertyChainHandle::Create(Object, PropertyPath, BufferedPropertyContext);
	}
	else
	{
		PropertyChain = FPropertyChainHandle::Create(Object, PropertyPath);
	}
	if (!PropertyChain.IsValid())
	{
		return FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("Failed to recognize the property path '{0}'.")),
			FText::FromString(PropertyPath.GetPathName().ToString())));
	}

	return FResult::Success();
}

FElementNode::FResult FPropertyElementNode::OnEnd()
{
	// A property element cannot carry both a value and child elements.
	if (!PropertyValue.IsEmpty() && ElementChildren.Num() > 0)
	{
		return FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("Property '{0}': cannot use both a value and child elements.")),
			FText::FromString(PropertyPath.GetPathName().ToString())));
	}

	if (const bool bShouldSetValue = PropertyChain.IsValid() && !PropertyValue.IsEmpty() && ElementChildren.Num() == 0)
	{
		if (!PropertyChain->SetValue(FStringView(PropertyValue)))
		{
			return FResult::Failure().Error(FText::Format(
				FText::FromString(TEXT("Failed to set value for property path '{0}'.")),
				FText::FromString(PropertyPath.GetPathName().ToString())));
		}
	}

	// Assemble container children (Array/Set/Map) once all children have
	// finalized their values: struct children need their own OnEnd to have run,
	// and basic-type children must be rebuilt from their value string because
	// their buffer is released at their own OnEnd.
	if (PropertyChain.IsValid())
	{
		FProperty* TailProperty = PropertyChain->GetTailProperty();
		if (TailProperty && (CastField<FArrayProperty>(TailProperty) || CastField<FSetProperty>(TailProperty) || CastField<FMapProperty>(TailProperty)))
		{
			void* TailValueAddress = PropertyChain->GetTailValueAddress();
			check(TailValueAddress);
			for (int32 ChildIndex = 0; ChildIndex < ElementChildren.Num(); ++ChildIndex)
			{
				const TSharedPtr<const FPropertyBuffer>& Snapshot =
					ContainerChildSnapshots.IsValidIndex(ChildIndex) ? ContainerChildSnapshots[ChildIndex] : nullptr;
				FResult AssembleResult = AssembleContainerElement(TailProperty, TailValueAddress, ElementChildren[ChildIndex], Snapshot);
				if (!AssembleResult)
				{
					return AssembleResult;
				}
			}
		}
	}

	if (const bool bMatchesPropertyBufferPath = BufferedPropertyContext.MatchesPath(PropertyPath))
	{
		const TSharedPtr<FPropertyBuffer> PropertyBuffer = BufferedPropertyContext.GetPropertyBuffer();
		if (!ensureMsgf(PropertyBuffer.IsValid(), TEXT("BufferedPropertyContext matches path but has no property buffer.")) || !PropertyBuffer->HasValue())
		{
			return FResult::Failure().Error(FText::Format(
				FText::FromString(TEXT("Buffered root has no value for property path '{0}'.")),
				FText::FromString(PropertyPath.GetPathName().ToString())));
		}

		TSharedPtr<FPropertyChainHandle> DirectHandle = PropertyChain.IsValid() ? PropertyChain->GetDirectHandle() : nullptr;
		if (!DirectHandle.IsValid() || !DirectHandle->SetValue(*PropertyBuffer))
		{
			return FResult::Failure().Error(FText::Format(
				FText::FromString(TEXT("Failed to commit buffered root value for property path '{0}'.")),
				FText::FromString(PropertyPath.GetPathName().ToString())));
		}
	}

	PropertyPath.Reset();
	BufferedPropertyContext.Reset();
	PropertyChain = nullptr;
	return FResult::Success();
}

FElementNode::FResult FPropertyElementNode::OnAddChild(const TSharedRef<FElementNode>& Child)
{
	ElementChildren.Add(Child);

	// Snapshot buffered property children now: container assembly happens at our
	// OnEnd, but a buffered child releases its buffer at its own OnEnd.
	TSharedPtr<const FPropertyBuffer> BufferedSnapshot;
	if (auto ChildPropertyNode = CastElementNode<FPropertyElementNode>(Child))
	{
		if (ChildPropertyNode->bUseBufferedWrite)
		{
			BufferedSnapshot = ChildPropertyNode->GetPropertyBuffer();
		}
	}
	ContainerChildSnapshots.Add(BufferedSnapshot);

	if (!PropertyChain.IsValid())
	{
		return FResult::Failure();
	}

	FProperty* TailProperty = PropertyChain->GetTailProperty();
	void* TailValueAddress = PropertyChain->GetTailValueAddress();

	// Containers (Array/Set/Map) are assembled in OnEnd via AssembleContainerElement.
	if (CastField<FArrayProperty>(TailProperty) || CastField<FSetProperty>(TailProperty) || CastField<FMapProperty>(TailProperty))
	{
		return FResult::Success();
	}

	if (FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(TailProperty))
	{
		// Single object property: accept one ObjectElementNode child.
		if (ElementChildren.Num() > 1)
		{
			return FResult::Failure().Error(FText::Format(
				FText::FromString(TEXT("Failed to add child to object property path '{0}': only one child element is allowed for a non-array object property.")),
				FText::FromString(PropertyPath.GetPathName().ToString())));
		}

		if (auto ChildObjectElementNode = CastElementNode<FObjectElementNode>(Child))
		{
			UObject* ChildObject = ChildObjectElementNode->GetObject();
			if (!ChildObject)
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add object child to property path '{0}': object element node returned null object.")),
					FText::FromString(PropertyPath.GetPathName().ToString())));
			}
			if (ObjectProperty->PropertyClass && !ChildObject->IsA(ObjectProperty->PropertyClass))
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add object child to property path '{0}': child class '{1}' is not compatible with '{2}'.")),
					FText::FromString(PropertyPath.GetPathName().ToString()),
					FText::FromString(ChildObject->GetClass()->GetName()),
					FText::FromString(ObjectProperty->PropertyClass->GetName())));
			}
			ObjectProperty->SetObjectPropertyValue(TailValueAddress, ChildObject);
		}
	}
	else if (FStructProperty* StructProperty = CastField<FStructProperty>(TailProperty))
	{
		// Single struct property: accept one StructElementNode child.
		if (ElementChildren.Num() > 1)
		{
			return FResult::Failure().Error(FText::Format(
				FText::FromString(TEXT("Failed to add child to struct property path '{0}': only one child element is allowed for a non-array struct property.")),
				FText::FromString(PropertyPath.GetPathName().ToString())));
		}

		if (auto ChildStructNode = CastElementNode<FStructElementNode>(Child))
		{
			UScriptStruct* ChildStructType = ChildStructNode->GetScriptStruct();
			void* ChildStructMemory = ChildStructNode->GetStructMemory();
			if (!ChildStructType || !ChildStructMemory)
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add struct child to property path '{0}': struct child element has no data.")),
					FText::FromString(PropertyPath.GetPathName().ToString())));
			}
			if (!StructProperty->Struct->IsChildOf(ChildStructType) && !ChildStructType->IsChildOf(StructProperty->Struct))
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add struct child to property path '{0}': child struct type '{1}' is not compatible with '{2}'.")),
					FText::FromString(PropertyPath.GetPathName().ToString()),
					FText::FromString(ChildStructType->GetName()),
					FText::FromString(StructProperty->Struct->GetName())));
			}
			StructProperty->CopyCompleteValue(TailValueAddress, ChildStructMemory);
		}
	}

	return FResult::Success();
}

FElementNode::FResult FPropertyElementNode::AssembleContainerElement(
	FProperty* ContainerProperty,
	void* ContainerValueAddress,
	const TSharedRef<FElementNode>& Child,
	const TSharedPtr<const FPropertyBuffer>& BufferedSnapshot)
{
	// Thin dispatcher: each container type has its own assembly function so the
	// per-container logic stays short and readable.
	if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(ContainerProperty))
	{
		return AssembleArrayElement(ArrayProperty, ContainerValueAddress, Child, BufferedSnapshot);
	}
	if (FSetProperty* SetProperty = CastField<FSetProperty>(ContainerProperty))
	{
		return AssembleSetElement(SetProperty, ContainerValueAddress, Child, BufferedSnapshot);
	}
	if (FMapProperty* MapProperty = CastField<FMapProperty>(ContainerProperty))
	{
		return AssembleMapElement(MapProperty, ContainerValueAddress, Child);
	}

	const FString PathString = PropertyPath.GetPathName().ToString();
	return FResult::Failure().Error(FText::Format(
		FText::FromString(TEXT("Failed to assemble container property path '{0}': property '{1}' is not a container.")),
		FText::FromString(PathString),
		FText::FromString(ContainerProperty ? ContainerProperty->GetClass()->GetName() : TEXT("null"))));
}

FElementNode::FResult FPropertyElementNode::AssembleArrayElement(
	FArrayProperty* ArrayProperty,
	void* ContainerValueAddress,
	const TSharedRef<FElementNode>& Child,
	const TSharedPtr<const FPropertyBuffer>& BufferedSnapshot)
{
	const FString PathString = PropertyPath.GetPathName().ToString();

	check(ContainerValueAddress);
	FScriptArrayHelper ArrayHelper(ArrayProperty, ContainerValueAddress);
	const int32 NewArrayIndex = ArrayHelper.AddValue();
	void* NewElementPointer = ArrayHelper.GetRawPtr(NewArrayIndex);

	if (auto ChildObjectElementNode = CastElementNode<FObjectElementNode>(Child))
		{
			UObject* ChildObject = ChildObjectElementNode->GetObject();
			if (!ChildObject)
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add object child to array property path '{0}': object element node returned null object.")),
					FText::FromString(PathString)));
			}
			FObjectPropertyBase* InnerObjectProperty = CastField<FObjectPropertyBase>(ArrayProperty->Inner);
			if (!InnerObjectProperty)
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add child object to array property path '{0}': array inner type '{1}' is not an object property.")),
					FText::FromString(PathString),
					FText::FromString(ArrayProperty->Inner ? ArrayProperty->Inner->GetClass()->GetName() : TEXT("null"))));
			}
			if (InnerObjectProperty->PropertyClass && !ChildObject->IsA(InnerObjectProperty->PropertyClass))
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add child object to array property path '{0}': child class '{1}' is not compatible with '{2}'.")),
					FText::FromString(PathString),
					FText::FromString(ChildObject->GetClass()->GetName()),
					FText::FromString(InnerObjectProperty->PropertyClass->GetName())));
			}
			InnerObjectProperty->SetObjectPropertyValue(NewElementPointer, ChildObject);
			return FResult::Success();
		}

		if (auto ChildStructNode = CastElementNode<FStructElementNode>(Child))
		{
			UScriptStruct* ChildStructType = ChildStructNode->GetScriptStruct();
			void* ChildStructMemory = ChildStructNode->GetStructMemory();
			if (!ChildStructType || !ChildStructMemory)
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add struct child to array property path '{0}': struct child element has no data.")),
					FText::FromString(PathString)));
			}
			const FStructProperty* InnerStructProperty = CastField<FStructProperty>(ArrayProperty->Inner);
			if (!InnerStructProperty || InnerStructProperty->Struct != ChildStructType)
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add struct child to array property path '{0}': array element type '{1}' is not compatible with child struct '{2}'.")),
					FText::FromString(PathString),
					FText::FromString(ArrayProperty->Inner ? ArrayProperty->Inner->GetClass()->GetName() : TEXT("null")),
					FText::FromString(ChildStructType->GetName())));
			}
			ArrayProperty->Inner->CopyCompleteValue(NewElementPointer, ChildStructMemory);
			return FResult::Success();
		}

		if (auto ChildPropertyElementNode = CastElementNode<FPropertyElementNode>(Child))
		{
			const TSharedPtr<const FPropertyBuffer> ChildPropertyBuffer = BufferedSnapshot;
			if (!ChildPropertyBuffer.IsValid() || !ChildPropertyBuffer->GetValueData())
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add child property to array property path '{0}': child property buffer is invalid or uninitialized.")),
					FText::FromString(PathString)));
			}
			FProperty* ChildProperty = ChildPropertyBuffer->GetProperty();
			if (!ChildProperty)
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add child property to array property path '{0}': child buffer root property is null.")),
					FText::FromString(PathString)));
			}
			if (!ArrayProperty->Inner->SameType(ChildProperty))
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add child property to array property path '{0}': array inner type '{1}' is not compatible with child buffered root type '{2}'.")),
					FText::FromString(PathString),
					FText::FromString(ArrayProperty->Inner->GetClass()->GetName()),
					FText::FromString(ChildProperty->GetClass()->GetName())));
			}
			ArrayProperty->Inner->CopyCompleteValue(NewElementPointer, ChildPropertyBuffer->GetValueData());
			return FResult::Success();
		}

		if (auto ChildBasicTypeNode = CastElementNode<FBasicTypeElementNode>(Child))
		{
			// Basic children release their value buffer at their own OnEnd, so
			// rebuild the value from the original string (same as the Variable path).
			FPropertyBuffer RebuiltBuffer(ArrayProperty->Inner, FStringView(ChildBasicTypeNode->GetValueString()));
			if (!RebuiltBuffer.HasValue())
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add basic-type child to array property path '{0}': cannot convert '{1}' to element type '{2}'.")),
					FText::FromString(PathString),
					FText::FromString(ChildBasicTypeNode->GetValueString()),
					FText::FromString(ArrayProperty->Inner ? ArrayProperty->Inner->GetClass()->GetName() : TEXT("null"))));
			}
			ArrayProperty->Inner->CopyCompleteValue(NewElementPointer, RebuiltBuffer.GetValueData());
			return FResult::Success();
		}

		return FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("Failed to add child to array property path '{0}': unsupported child element.")),
			FText::FromString(PathString)));
}

FElementNode::FResult FPropertyElementNode::AssembleSetElement(
	FSetProperty* SetProperty,
	void* ContainerValueAddress,
	const TSharedRef<FElementNode>& Child,
	const TSharedPtr<const FPropertyBuffer>& BufferedSnapshot)
{
	const FString PathString = PropertyPath.GetPathName().ToString();

	check(ContainerValueAddress);
	FScriptSetHelper SetHelper(SetProperty, ContainerValueAddress);

	if (auto ChildObjectElementNode = CastElementNode<FObjectElementNode>(Child))
		{
			UObject* ChildObject = ChildObjectElementNode->GetObject();
			if (!ChildObject)
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add object child to set property path '{0}': object element node returned null object.")),
					FText::FromString(PathString)));
			}
			FObjectPropertyBase* ElementObjectProperty = CastField<FObjectPropertyBase>(SetProperty->ElementProp);
			if (!ElementObjectProperty)
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add child object to set property path '{0}': set element type '{1}' is not an object property.")),
					FText::FromString(PathString),
					FText::FromString(SetProperty->ElementProp ? SetProperty->ElementProp->GetClass()->GetName() : TEXT("null"))));
			}
			if (ElementObjectProperty->PropertyClass && !ChildObject->IsA(ElementObjectProperty->PropertyClass))
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add child object to set property path '{0}': child class '{1}' is not compatible with '{2}'.")),
					FText::FromString(PathString),
					FText::FromString(ChildObject->GetClass()->GetName()),
					FText::FromString(ElementObjectProperty->PropertyClass->GetName())));
			}
			TObjectPtr<UObject> ObjectPointer = ChildObject;
			SetHelper.AddElement(&ObjectPointer);
			return FResult::Success();
		}

		if (auto ChildStructNode = CastElementNode<FStructElementNode>(Child))
		{
			UScriptStruct* ChildStructType = ChildStructNode->GetScriptStruct();
			void* ChildStructMemory = ChildStructNode->GetStructMemory();
			if (!ChildStructType || !ChildStructMemory)
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add struct child to set property path '{0}': struct child element has no data.")),
					FText::FromString(PathString)));
			}
			const FStructProperty* InnerStructProperty = CastField<FStructProperty>(SetProperty->ElementProp);
			if (!InnerStructProperty || InnerStructProperty->Struct != ChildStructType)
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add struct child to set property path '{0}': set element type '{1}' is not compatible with child struct '{2}'.")),
					FText::FromString(PathString),
					FText::FromString(SetProperty->ElementProp ? SetProperty->ElementProp->GetClass()->GetName() : TEXT("null")),
					FText::FromString(ChildStructType->GetName())));
			}
			SetHelper.AddElement(ChildStructMemory);
			return FResult::Success();
		}

		if (auto ChildPropertyElementNode = CastElementNode<FPropertyElementNode>(Child))
		{
			const TSharedPtr<const FPropertyBuffer> ChildPropertyBuffer = BufferedSnapshot;
			if (!ChildPropertyBuffer.IsValid() || !ChildPropertyBuffer->GetValueData())
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add child property to set property path '{0}': child property buffer is invalid or uninitialized.")),
					FText::FromString(PathString)));
			}
			FProperty* ChildProperty = ChildPropertyBuffer->GetProperty();
			if (!ChildProperty)
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add child property to set property path '{0}': child buffer root property is null.")),
					FText::FromString(PathString)));
			}
			if (!SetProperty->ElementProp->SameType(ChildProperty))
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add child property to set property path '{0}': set element type '{1}' is not compatible with child buffered root type '{2}'.")),
					FText::FromString(PathString),
					FText::FromString(SetProperty->ElementProp->GetClass()->GetName()),
					FText::FromString(ChildProperty->GetClass()->GetName())));
			}
			SetHelper.AddElement(ChildPropertyBuffer->GetValueData());
			return FResult::Success();
		}

		if (auto ChildBasicTypeNode = CastElementNode<FBasicTypeElementNode>(Child))
		{
			// Basic children release their value buffer at their own OnEnd, so
			// rebuild the value from the original string (same as the Variable path).
			FPropertyBuffer RebuiltBuffer(SetProperty->ElementProp, FStringView(ChildBasicTypeNode->GetValueString()));
			if (!RebuiltBuffer.HasValue())
			{
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Failed to add basic-type child to set property path '{0}': cannot convert '{1}' to element type '{2}'.")),
					FText::FromString(PathString),
					FText::FromString(ChildBasicTypeNode->GetValueString()),
					FText::FromString(SetProperty->ElementProp ? SetProperty->ElementProp->GetClass()->GetName() : TEXT("null"))));
			}
			SetHelper.AddElement(RebuiltBuffer.GetValueData());
			return FResult::Success();
		}

		return FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("Failed to add child to set property path '{0}': unsupported child element.")),
			FText::FromString(PathString)));
}

FElementNode::FResult FPropertyElementNode::AssembleMapElement(
	FMapProperty* MapProperty,
	void* ContainerValueAddress,
	const TSharedRef<FElementNode>& Child)
{
	const FString PathString = PropertyPath.GetPathName().ToString();

	// Map children must be Pair elements. Their Key/Value attributes are
	// processed after OnAddChild, so the pairs are assembled here in OnEnd().
	auto StructChild = CastElementNode<FStructElementNode>(TSharedPtr<FElementNode>(Child));
	if (!StructChild || StructChild->GetScriptStruct() != FWidgetMarkupKeyValuePair::StaticStruct())
	{
		return FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("Failed to add child to map property path '{0}': Map children must be Pair elements.")),
			FText::FromString(PathString)));
	}
	const FWidgetMarkupKeyValuePair* Pair = static_cast<const FWidgetMarkupKeyValuePair*>(StructChild->GetStructMemory());
	if (!Pair)
	{
		return FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("Failed to assemble map property path '{0}': Pair child element has no data.")),
			FText::FromString(PathString)));
	}
	FPropertyBuffer KeyBuffer(MapProperty->KeyProp, FStringView(Pair->Key));
	FPropertyBuffer ValueBuffer(MapProperty->ValueProp, FStringView(Pair->Value));
	if (!KeyBuffer.HasValue() || !ValueBuffer.HasValue())
	{
		return FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("Failed to assemble map property path '{0}': cannot convert pair ('{1}', '{2}').")),
			FText::FromString(PathString),
			FText::FromString(Pair->Key),
			FText::FromString(Pair->Value)));
	}
	FScriptMapHelper MapHelper(MapProperty, ContainerValueAddress);
	MapHelper.AddPair(KeyBuffer.GetValueData(), ValueBuffer.GetValueData());
	return FResult::Success();
}

UStruct* FPropertyElementNode::GetPropertyOwnerStruct() const
{
	if (!PropertyChain.IsValid())
	{
		return nullptr;
	}
	FProperty* TailProperty = PropertyChain->GetTailProperty();
	if (!TailProperty)
	{
		return nullptr;
	}
	if (FStructProperty* StructProperty = CastField<FStructProperty>(TailProperty))
	{
		return StructProperty->Struct;
	}
	if (FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(TailProperty))
	{
		return ObjectProperty->PropertyClass;
	}
	return nullptr;
}

FProperty* FPropertyElementNode::ResolveExpectedChildProperty()
{
	if (!PropertyChain.IsValid())
	{
		return nullptr;
	}
	FProperty* TailProperty = PropertyChain->GetTailProperty();
	if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(TailProperty))
	{
		return ArrayProperty->Inner;
	}
	if (FSetProperty* SetProperty = CastField<FSetProperty>(TailProperty))
	{
		return SetProperty->ElementProp;
	}
	// Map properties use Pair children (handled in OnAddChild/OnEnd), not bare
	// basic-type elements.
	return nullptr;
}

bool FPropertyElementNode::HasProperty(const FStringView& AttributeName)
{
	return PropertyChain.IsValid() && PropertyChain->GetChildHandle(AttributeName).IsValid();
}

void FPropertyElementNode::SetPropertyRun(TSharedPtr<IPropertyRun> InPropertyRun)
{
	PropertyRun = InPropertyRun;
}
