// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "BlueprintVariableElementNode.h"

#include "BasicTypeElementNode.h"
#include "ConverterRegistry.h"
#include "ObjectElementNode.h"
#include "PropertyBuffer.h"
#include "StructElementNode.h"
#include "WidgetMarkupBlueprintVariable.h"
#include "Data/WidgetMarkupKeyValuePair.h"
#include "../Utilities/PropertyValueAssembler.h"
#include "../Utilities/TypeParser.h"
#include "Engine/Blueprint.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "UObject/UnrealType.h"

IMPLEMENT_ELEMENT_NODE(FBlueprintVariableElementNode, FStructElementNode)

TSharedRef<FElementNode> FBlueprintVariableElementNode::Create()
{
	return MakeShared<FBlueprintVariableElementNode>();
}

FElementNode::FResult FBlueprintVariableElementNode::OnBegin(const FContext& Context, UObject* Outer, UStruct* Struct)
{
	FResult Result = FStructElementNode::OnBegin(Context, Outer, Struct);
	if (!Result)
	{
		return Result;
	}

	ParentBlueprint = Context.FindObject<UBlueprint>();
	if (!ParentBlueprint.IsValid())
	{
		return FResult::Failure().Error(FText::FromString(TEXT("Variable element must be nested under a Blueprint or WidgetBlueprint element.")));
	}
	return FResult::Success();
}

bool FBlueprintVariableElementNode::EnsureTypeParsed()
{
	if (bTypeParsed || !TypeParseError.IsEmpty())
	{
		return bTypeParsed;
	}

	const auto* VariableData = static_cast<const FWidgetMarkupBlueprintVariable*>(GetStructMemory());
	if (!VariableData)
	{
		TypeParseError = TEXT("Variable element has no struct data.");
		return false;
	}

	const FString VariableType = VariableData->Type.TrimStartAndEnd();
	if (VariableType.IsEmpty())
	{
		TypeParseError = TEXT("Variable element requires a non-empty Type attribute.");
		return false;
	}
	if (!FTypeParser::ParseType(VariableType, CachedPinType, TypeParseError))
	{
		return false;
	}

	FString FactoryError;
	FProperty* SyntheticProperty = TypeFactory.CreateProperty(CachedPinType, FactoryError);
	if (!SyntheticProperty)
	{
		TypeParseError = FactoryError;
		return false;
	}

	DefaultBuffer.SetProperty(SyntheticProperty);
	if (!DefaultBuffer.HasValue())
	{
		TypeParseError = TEXT("Failed to allocate default value memory.");
		return false;
	}

	bTypeParsed = true;
	return true;
}

FProperty* FBlueprintVariableElementNode::ResolveExpectedChildProperty()
{
	if (!EnsureTypeParsed())
	{
		return nullptr;
	}

	FProperty* Property = DefaultBuffer.GetProperty();
	if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
	{
		return ArrayProperty->Inner;
	}
	if (FSetProperty* SetProperty = CastField<FSetProperty>(Property))
	{
		return SetProperty->ElementProp;
	}
	if (FMapProperty* MapProperty = CastField<FMapProperty>(Property))
	{
		// Map elements must be Pair elements; a bare basic-type child has no key/value split.
		return nullptr;
	}
	return Property;
}

FElementNode::FResult FBlueprintVariableElementNode::OnAddChild(const TSharedRef<FElementNode>& Child)
{
	if (!EnsureTypeParsed())
	{
		return FResult::Failure().Error(FText::FromString(TypeParseError));
	}

	FResult ValidationResult = ValidateChild(Child);
	if (!ValidationResult)
	{
		return ValidationResult;
	}

	DefaultValueChildren.Add(Child);
	return FResult::Success();
}

FElementNode::FResult FBlueprintVariableElementNode::ValidateChild(const TSharedRef<FElementNode>& Child)
{
	const bool bIsBasic = CastElementNode<FBasicTypeElementNode>(TSharedPtr<FElementNode>(Child)) != nullptr;
	const bool bIsObject = CastElementNode<FObjectElementNode>(TSharedPtr<FElementNode>(Child)) != nullptr;
	auto StructChild = CastElementNode<FStructElementNode>(TSharedPtr<FElementNode>(Child));
	const bool bIsPair = StructChild.IsValid() && StructChild->GetScriptStruct() == FWidgetMarkupKeyValuePair::StaticStruct();
	const bool bIsStruct = StructChild.IsValid() && !bIsPair;

	FProperty* Property = DefaultBuffer.GetProperty();

	if (CastField<FArrayProperty>(Property) || CastField<FSetProperty>(Property))
	{
		if (!bIsBasic && !bIsStruct && !bIsObject)
		{
			return FResult::Failure().Error(FText::FromString(
				TEXT("Variable: container children must be basic-type, struct, or object elements (Pair is only valid in Map).")));
		}
		return FResult::Success();
	}

	if (CastField<FMapProperty>(Property))
	{
		if (!bIsPair)
		{
			return FResult::Failure().Error(FText::FromString(
				TEXT("Variable: Map children must be Pair elements.")));
		}
		return FResult::Success();
	}

	// Non-container: at most one child, and it cannot be a Pair.
	if (DefaultValueChildren.Num() > 0)
	{
		return FResult::Failure().Error(FText::FromString(
			TEXT("Variable: only one child element is allowed for a non-container variable.")));
	}
	if (bIsPair)
	{
		return FResult::Failure().Error(FText::FromString(
			TEXT("Variable: Pair child elements are only valid in Map variables.")));
	}
	if (!bIsBasic && !bIsStruct && !bIsObject)
	{
		return FResult::Failure().Error(FText::FromString(
			TEXT("Variable: unsupported child element for this variable type.")));
	}
	return FResult::Success();
}

FElementNode::FResult FBlueprintVariableElementNode::OnEnd()
{
	UBlueprint* ParentBlueprintObject = ParentBlueprint.Get();
	if (!ParentBlueprintObject)
	{
		return FResult::Failure().Error(FText::FromString(TEXT("Variable element has no owning Blueprint.")));
	}

	const auto* VariableData = static_cast<const FWidgetMarkupBlueprintVariable*>(GetStructMemory());
	if (!VariableData)
	{
		return FResult::Failure().Error(FText::FromString(TEXT("Variable element has no struct data.")));
	}

	const FString VariableName = VariableData->Name.TrimStartAndEnd();
	const FString VariableType = VariableData->Type.TrimStartAndEnd();
	if (VariableName.IsEmpty())
	{
		return FResult::Failure().Error(FText::FromString(TEXT("Variable element requires a non-empty Name attribute.")));
	}
	if (VariableType.IsEmpty())
	{
		return FResult::Failure().Error(FText::FromString(TEXT("Variable element requires a non-empty Type attribute.")));
	}

	if (!EnsureTypeParsed())
	{
		return FResult::Failure().Error(FText::FromString(TypeParseError));
	}

	FProperty* SyntheticProperty = DefaultBuffer.GetProperty();
	const bool bIsContainer = SyntheticProperty->IsA<FArrayProperty>()
		|| SyntheticProperty->IsA<FSetProperty>()
		|| SyntheticProperty->IsA<FMapProperty>();

	const FString VariableDefaultValue = VariableData->Default;
	const bool bHasStringDefault = !VariableDefaultValue.IsEmpty();
	const bool bHasChildren = DefaultValueChildren.Num() > 0;
	if (bHasStringDefault && bHasChildren)
	{
		return FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("Variable '{0}': Default attribute conflicts with child elements.")),
			FText::FromString(VariableName)));
	}
	if (bHasStringDefault && bIsContainer)
	{
		// Route A: containers must use child elements.
		return FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("Variable '{0}' is a container type. Use child elements instead of the Default attribute.")),
			FText::FromString(VariableName)));
	}

	// Channel A: convert the Default string via the converter registry (non-container only).
	if (bHasStringDefault)
	{
		if (!FConverterRegistry::Get().Convert(*SyntheticProperty, DefaultBuffer.GetValueData(), VariableDefaultValue))
		{
			return FResult::Failure().Error(FText::Format(
				FText::FromString(TEXT("Variable '{0}': failed to convert default value '{1}' to type '{2}'.")),
				FText::FromString(VariableName),
				FText::FromString(VariableDefaultValue),
				FText::FromString(VariableType)));
		}
	}
	else if (bHasChildren)
	{
		// Assemble child values into the typed default buffer (shared assembler).
		if (bIsContainer)
		{
			for (const TSharedRef<FElementNode>& Child : DefaultValueChildren)
			{
				FResult ChildResult = FPropertyValueAssembler::AppendChildToContainer(SyntheticProperty, DefaultBuffer, Child);
				if (!ChildResult)
				{
					return ChildResult;
				}
			}
		}
		else
		{
			FResult ScalarResult = CopyScalarChild(SyntheticProperty, DefaultValueChildren[0]);
			if (!ScalarResult)
			{
				return ScalarResult;
			}
		}
	}

	// Export the typed default back to UE's property text format for AddMemberVariable.
	FString ExportedDefault;
	if (!DefaultBuffer.ExportValueText(ExportedDefault))
	{
		return FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("Variable '{0}': failed to export default value.")),
			FText::FromString(VariableName)));
	}

	if (!FBlueprintEditorUtils::AddMemberVariable(ParentBlueprintObject, FName(VariableName), CachedPinType, ExportedDefault))
	{
		return FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("Failed to add variable '{0}' to Blueprint '{1}'. Variable name may conflict with existing member or parent class.")),
			FText::FromString(VariableName),
			FText::FromString(ParentBlueprintObject->GetName())));
	}

	return FStructElementNode::OnEnd();
}

FElementNode::FResult FBlueprintVariableElementNode::CopyScalarChild(FProperty* Property, const TSharedRef<FElementNode>& Child)
{
	return FPropertyValueAssembler::CopyChildIntoBuffer(Property, DefaultBuffer, Child);
}
