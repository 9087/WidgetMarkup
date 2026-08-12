// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "BasicTypeElementNode.h"
#include "ElementNodes/PropertyElementNode.h"
#include "ElementNodes/SetterElementNode.h"
#include "PropertyBuffer.h"
#include "UObject/UnrealType.h"

IMPLEMENT_ELEMENT_NODE(FBasicTypeElementNode, FElementNode)

FBasicTypeElementNode::FBasicTypeElementNode(const FStringView& InTypeName)
	: TypeName(InTypeName)
{
}

void FBasicTypeElementNode::SetElementData(const TCHAR* InElementData)
{
	if (InElementData)
	{
		ValueString = InElementData;
	}
}

FElementNode::FResult FBasicTypeElementNode::OnBegin(const FContext& Context, UObject* Outer, UStruct* /*Struct*/)
{
	// The parent node supplies the expected element property:
	// - FPropertyElementNode (array/set property chain, e.g. ColumnFill) returns
	//   the container's element property (Inner / ElementProp).
	// - FBlueprintVariableElementNode (Variable container) returns its synthesized
	//   inner property.
	FProperty* TailProperty = nullptr;
	TSharedPtr<FElementNode> Parent = Context.GetLastNode();
	if (Parent.IsValid())
	{
		TailProperty = Parent->ResolveExpectedChildProperty();
		if (!TailProperty)
		{
			// The parent could not provide an element type. A Map property needs
			// Pair children, and any other property node is neither an Array nor
			// a Set, so it cannot host bare basic-type elements.
			if (auto PropertyParent = CastElementNode<FPropertyElementNode>(Parent.Get()))
			{
				if (FProperty* ParentTail = PropertyParent->GetTailProperty())
				{
					if (CastField<FMapProperty>(ParentTail))
					{
						return FResult::Failure().Error(FText::Format(
							FText::FromString(TEXT("BasicTypeElementNode: parent is a map property, use pair child elements for '{0}'.")),
							FText::FromString(TypeName)));
					}
				}
				return FResult::Failure().Error(FText::Format(
					FText::FromString(TEXT("BasicTypeElementNode: parent is not an array or set property for '{0}'.")),
					FText::FromString(TypeName)));
			}
		}
	}
	if (!TailProperty)
	{
		// A Setter parent with an unresolvable path (object-pointer segment such
		// as Slot.*, array index, or unknown property) decides in its own OnEnd:
		// it warns and ignores the child element. Let the Setter handle it
		// instead of aborting here.
		if (Parent.IsValid() && CastElementNode<FSetterElementNode>(Parent.Get()))
		{
			return FResult::Success();
		}
		return FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("BasicTypeElementNode: cannot resolve element property for '{0}'.")),
			FText::FromString(TypeName)));
	}

	ValueBuffer = MakeShared<FPropertyBuffer>(TailProperty, FStringView(ValueString));
	if (!ValueBuffer->HasValue())
	{
		return FResult::Failure().Error(FText::Format(
			FText::FromString(TEXT("BasicTypeElementNode: failed to convert '{0}' to type '{1}'.")),
			FText::FromString(ValueString), FText::FromString(TailProperty->GetClass()->GetName())));
	}

	return FResult::Success();
}

FElementNode::FResult FBasicTypeElementNode::OnEnd()
{
	ValueBuffer.Reset();
	return FResult::Success();
}

FElementNode::FResult FBasicTypeElementNode::OnAddChild(const TSharedRef<FElementNode>& /*Child*/)
{
	return FResult::Failure().Error(FText::FromString(TEXT("BasicTypeElementNode: leaf node cannot have children.")));
}

bool FBasicTypeElementNode::HasProperty(const FStringView& /*AttributeName*/)
{
	return false;
}
