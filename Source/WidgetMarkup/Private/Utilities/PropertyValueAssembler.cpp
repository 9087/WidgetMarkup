// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "Utilities/PropertyValueAssembler.h"

#include "Data/WidgetMarkupKeyValuePair.h"
#include "ElementNodes/BasicTypeElementNode.h"
#include "ElementNodes/ObjectElementNode.h"
#include "ElementNodes/StructElementNode.h"
#include "PropertyBuffer.h"
#include "UObject/UnrealType.h"

namespace FPropertyValueAssembler
{
	FElementNode::FResult CopyChildIntoBuffer(FProperty* Property, FPropertyBuffer& TargetBuffer, const TSharedRef<FElementNode>& Child)
	{
		if (!Property || !TargetBuffer.HasValue())
		{
			return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Assembler: invalid property or target buffer.")));
		}

		if (auto BasicNode = CastElementNode<FBasicTypeElementNode>(Child))
		{
			FPropertyBuffer ChildBuffer(Property, FStringView(BasicNode->GetValueString()));
			if (!ChildBuffer.HasValue())
			{
				return FElementNode::FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Assembler: failed to convert value '{0}' to type '{1}'.")),
					FText::FromString(BasicNode->GetValueString()),
					FText::FromString(Property->GetClass()->GetName())));
			}
			Property->CopyCompleteValue(TargetBuffer.GetValueData(), ChildBuffer.GetValueData());
			return FElementNode::FResult::Success();
		}

		if (auto StructNode = CastElementNode<FStructElementNode>(Child))
		{
			UScriptStruct* StructType = StructNode->GetScriptStruct();
			void* StructMemory = StructNode->GetStructMemory();
			if (!StructType || !StructMemory)
			{
				return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Assembler: struct child element has no data.")));
			}
			const FStructProperty* StructProperty = CastField<FStructProperty>(Property);
			if (!StructProperty || StructProperty->Struct != StructType)
			{
				return FElementNode::FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Assembler: property type '{1}' does not match child struct '{0}'.")),
					FText::FromString(StructType->GetName()),
					FText::FromString(Property->GetClass()->GetName())));
			}
			Property->CopyCompleteValue(TargetBuffer.GetValueData(), StructMemory);
			return FElementNode::FResult::Success();
		}

		if (auto ObjectNode = CastElementNode<FObjectElementNode>(Child))
		{
			UObject* ChildObject = ObjectNode->GetObject();
			if (!ChildObject)
			{
				return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Assembler: object child element has no object.")));
			}
			FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property);
			if (!ObjectProperty || !ChildObject->IsA(ObjectProperty->PropertyClass))
			{
				return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Assembler: property type does not match child object class.")));
			}
			ObjectProperty->SetObjectPropertyValue(TargetBuffer.GetValueData(), ChildObject);
			return FElementNode::FResult::Success();
		}

		return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Assembler: unsupported child element.")));
	}

	FElementNode::FResult AppendChildToContainer(FProperty* ContainerProperty, FPropertyBuffer& TargetBuffer, const TSharedRef<FElementNode>& Child)
	{
		if (!ContainerProperty || !TargetBuffer.HasValue())
		{
			return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Assembler: invalid container property or target buffer.")));
		}

		void* ContainerData = TargetBuffer.GetValueData();

		if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(ContainerProperty))
		{
			FScriptArrayHelper ArrayHelper(ArrayProperty, ContainerData);
			const int32 NewArrayIndex = ArrayHelper.AddValue();
			void* NewElementPointer = ArrayHelper.GetRawPtr(NewArrayIndex);

			if (auto BasicNode = CastElementNode<FBasicTypeElementNode>(Child))
			{
				FPropertyBuffer ChildBuffer(ArrayProperty->Inner, FStringView(BasicNode->GetValueString()));
				if (!ChildBuffer.HasValue())
				{
					return FElementNode::FResult::Failure().Error(FText::Format(
						FText::FromString(TEXT("Assembler: failed to convert array element '{0}'.")),
						FText::FromString(BasicNode->GetValueString())));
				}
				ArrayProperty->Inner->CopyCompleteValue(NewElementPointer, ChildBuffer.GetValueData());
				return FElementNode::FResult::Success();
			}

			if (auto StructNode = CastElementNode<FStructElementNode>(Child))
			{
				UScriptStruct* StructType = StructNode->GetScriptStruct();
				void* StructMemory = StructNode->GetStructMemory();
				if (!StructType || !StructMemory)
				{
					return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Assembler: struct child element has no data.")));
				}
				const FStructProperty* InnerStructProperty = CastField<FStructProperty>(ArrayProperty->Inner);
				if (!InnerStructProperty || InnerStructProperty->Struct != StructType)
				{
					return FElementNode::FResult::Failure().Error(FText::Format(
						FText::FromString(TEXT("Assembler: array element type does not match child struct '{0}'.")),
						FText::FromString(StructType->GetName())));
				}
				ArrayProperty->Inner->CopyCompleteValue(NewElementPointer, StructMemory);
				return FElementNode::FResult::Success();
			}

			if (auto ObjectNode = CastElementNode<FObjectElementNode>(Child))
			{
				UObject* ChildObject = ObjectNode->GetObject();
				if (!ChildObject)
				{
					return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Assembler: object child element has no object.")));
				}
				FObjectPropertyBase* InnerObjectProperty = CastField<FObjectPropertyBase>(ArrayProperty->Inner);
				if (!InnerObjectProperty || !ChildObject->IsA(InnerObjectProperty->PropertyClass))
				{
					return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Assembler: array element type does not match child object class.")));
				}
				InnerObjectProperty->SetObjectPropertyValue(NewElementPointer, ChildObject);
				return FElementNode::FResult::Success();
			}

			return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Assembler: unsupported child element for Array type.")));
		}

		if (FSetProperty* SetProperty = CastField<FSetProperty>(ContainerProperty))
		{
			FScriptSetHelper SetHelper(SetProperty, ContainerData);

			if (auto BasicNode = CastElementNode<FBasicTypeElementNode>(Child))
			{
				FPropertyBuffer ChildBuffer(SetProperty->ElementProp, FStringView(BasicNode->GetValueString()));
				if (!ChildBuffer.HasValue())
				{
					return FElementNode::FResult::Failure().Error(FText::Format(
						FText::FromString(TEXT("Assembler: failed to convert set element '{0}'.")),
						FText::FromString(BasicNode->GetValueString())));
				}
				SetHelper.AddElement(ChildBuffer.GetValueData());
				return FElementNode::FResult::Success();
			}

			if (auto StructNode = CastElementNode<FStructElementNode>(Child))
			{
				UScriptStruct* StructType = StructNode->GetScriptStruct();
				void* StructMemory = StructNode->GetStructMemory();
				if (!StructType || !StructMemory)
				{
					return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Assembler: struct child element has no data.")));
				}
				const FStructProperty* InnerStructProperty = CastField<FStructProperty>(SetProperty->ElementProp);
				if (!InnerStructProperty || InnerStructProperty->Struct != StructType)
				{
					return FElementNode::FResult::Failure().Error(FText::Format(
						FText::FromString(TEXT("Assembler: set element type does not match child struct '{0}'.")),
						FText::FromString(StructType->GetName())));
				}
				SetHelper.AddElement(StructMemory);
				return FElementNode::FResult::Success();
			}

			if (auto ObjectNode = CastElementNode<FObjectElementNode>(Child))
			{
				UObject* ChildObject = ObjectNode->GetObject();
				if (!ChildObject)
				{
					return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Assembler: object child element has no object.")));
				}
				FObjectPropertyBase* InnerObjectProperty = CastField<FObjectPropertyBase>(SetProperty->ElementProp);
				if (!InnerObjectProperty || !ChildObject->IsA(InnerObjectProperty->PropertyClass))
				{
					return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Assembler: set element type does not match child object class.")));
				}
				TObjectPtr<UObject> ObjectPointer = ChildObject;
				SetHelper.AddElement(&ObjectPointer);
				return FElementNode::FResult::Success();
			}

			return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Assembler: unsupported child element for Set type.")));
		}

		if (FMapProperty* MapProperty = CastField<FMapProperty>(ContainerProperty))
		{
			// Map children must be Pair elements.
			auto StructNode = CastElementNode<FStructElementNode>(TSharedPtr<FElementNode>(Child));
			if (!StructNode || StructNode->GetScriptStruct() != FWidgetMarkupKeyValuePair::StaticStruct())
			{
				return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Assembler: Map children must be Pair elements.")));
			}
			const FWidgetMarkupKeyValuePair* Pair = static_cast<const FWidgetMarkupKeyValuePair*>(StructNode->GetStructMemory());
			if (!Pair)
			{
				return FElementNode::FResult::Failure().Error(FText::FromString(TEXT("Assembler: Pair child element has no data.")));
			}
			FPropertyBuffer KeyBuffer(MapProperty->KeyProp, FStringView(Pair->Key));
			if (!KeyBuffer.HasValue())
			{
				return FElementNode::FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Assembler: failed to convert map key '{0}'.")),
					FText::FromString(Pair->Key)));
			}
			FPropertyBuffer ValueBuffer(MapProperty->ValueProp, FStringView(Pair->Value));
			if (!ValueBuffer.HasValue())
			{
				return FElementNode::FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("Assembler: failed to convert map value '{0}'.")),
					FText::FromString(Pair->Value)));
			}
			FScriptMapHelper MapHelper(MapProperty, ContainerData);
			MapHelper.AddPair(KeyBuffer.GetValueData(), ValueBuffer.GetValueData());
			return FElementNode::FResult::Success();
		}

		return FElementNode::FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("Assembler: property '{0}' is not a container.")),
			FText::FromString(ContainerProperty->GetClass()->GetName())));
	}
}
