// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "SetterElementNode.h"

#include "BasicTypeElementNode.h"
#include "ObjectElementNode.h"
#include "PropertyBuffer.h"
#include "StyleElementNode.h"
#include "Utilities/PropertyPathResolver.h"
#include "Utilities/PropertyValueAssembler.h"
#include "Utilities/TypeParser.h"
#include "WidgetMarkupModule.h"

IMPLEMENT_ELEMENT_NODE(FSetterElementNode, FStructElementNode)

TSharedRef<FElementNode> FSetterElementNode::Create()
{
	return MakeShared<FSetterElementNode>();
}

FElementNode::FResult FSetterElementNode::OnBegin(const FContext& Context, UObject* Outer, UStruct* Struct)
{
	FResult Result = FStructElementNode::OnBegin(Context, Outer, Struct);
	if (!Result)
	{
		return Result;
	}

	// Capture the owning Style's TargetType so OnEnd can resolve the property
	// path type without a widget instance.
	TSharedPtr<FElementNode> Parent = Context.GetLastNode();
	if (auto StyleNode = CastElementNode<FStyleElementNode>(Parent))
	{
		if (const auto* Memory = static_cast<const FWidgetStyleEntry*>(StyleNode->GetStructMemory()))
		{
			TargetClass = FTypeParser::ResolveClass(Memory->TargetType.ToString());
		}
	}
	return FResult::Success();
}

FElementNode::FResult FSetterElementNode::OnEnd()
{
	FWidgetStyleSetter* Memory = static_cast<FWidgetStyleSetter*>(GetStructMemory());
	if (!Memory)
	{
		return FResult::Failure().Error(FText::FromString(TEXT("Setter: no struct data.")));
	}

	if (ChildNodes.Num() > 0)
	{
		if (!Memory->Value.IsEmpty())
		{
			return FResult::Failure().Error(FText::FromString(TEXT("Setter: cannot use both Value attribute and child elements.")));
		}

		FProperty* TailProperty = ResolveTailProperty(Memory->Property);
		if (!TailProperty)
		{
			// Paths that cannot be resolved at compile time (object-pointer segments
			// like Slot.*, array indices, unknown target) fall back to the runtime
			// Value path. Warn instead of silently dropping the child element.
			UE_LOG(LogWidgetMarkup, Warning,
				TEXT("Setter: cannot resolve property path '%s' on target '%s' at compile time; child element ignored."),
				*Memory->Property.GetPathName().ToString(),
				TargetClass ? *TargetClass->GetName() : TEXT("<unknown>"));
			return FStructElementNode::OnEnd();
		}

		Memory->Buffer.SetProperty(TailProperty);
		if (!Memory->Buffer.HasValue())
		{
			return FResult::Failure().Error(FText::FromString(TEXT("Setter: failed to allocate buffer for property path.")));
		}

		// Container property: each child element appends one entry.
		if (CastField<FArrayProperty>(TailProperty) || CastField<FSetProperty>(TailProperty) || CastField<FMapProperty>(TailProperty))
		{
			for (const TSharedRef<FElementNode>& Child : ChildNodes)
			{
				FResult ChildResult = FPropertyValueAssembler::AppendChildToContainer(TailProperty, Memory->Buffer, Child);
				if (!ChildResult)
				{
					return ChildResult;
				}
			}
			return FStructElementNode::OnEnd();
		}

		// Scalar/struct/object: a single child element is the whole value.
		if (ChildNodes.Num() > 1)
		{
			return FResult::Failure().Error(FText::FromString(TEXT("Setter: multiple child elements require a container property.")));
		}
		return FPropertyValueAssembler::CopyChildIntoBuffer(TailProperty, Memory->Buffer, ChildNodes[0]);
	}

	return FStructElementNode::OnEnd();
}

FElementNode::FResult FSetterElementNode::OnAddChild(const TSharedRef<FElementNode>& Child)
{
	// Accept inline values: BasicType, Struct, or Object children.
	if (CastElementNode<FBasicTypeElementNode>(TSharedPtr<FElementNode>(Child))
		|| CastElementNode<FStructElementNode>(TSharedPtr<FElementNode>(Child))
		|| CastElementNode<FObjectElementNode>(TSharedPtr<FElementNode>(Child)))
	{
		ChildNodes.Add(Child);
		return FResult::Success();
	}
	return FStructElementNode::OnAddChild(Child);
}

FProperty* FSetterElementNode::ResolveExpectedChildProperty()
{
	// Children Begin before our OnEnd, so resolve once here and cache. For
	// containers, return the element type (like PropertyElementNode) so Basic
	// children can build their value buffer; Map has no bare elements.
	if (ResolvedTailProperty)
	{
		return ResolvedTailProperty;
	}
	FProperty* Tail = nullptr;
	if (const auto* Memory = static_cast<const FWidgetStyleSetter*>(GetStructMemory()))
	{
		Tail = ResolveTailProperty(Memory->Property);
	}
	if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Tail))
	{
		ResolvedTailProperty = ArrayProperty->Inner;
	}
	else if (FSetProperty* SetProperty = CastField<FSetProperty>(Tail))
	{
		ResolvedTailProperty = SetProperty->ElementProp;
	}
	else
	{
		// Map returns nullptr (Pair children); scalars return themselves.
		ResolvedTailProperty = CastField<FMapProperty>(Tail) ? nullptr : Tail;
	}
	return ResolvedTailProperty;
}

FProperty* FSetterElementNode::ResolveTailProperty(const FWidgetPropertyPath& PropertyPath) const
{
	if (!TargetClass || PropertyPath.IsEmpty())
	{
		return nullptr;
	}
	FPropertyPathResolver::FInitialState InitialState(nullptr, TargetClass);
	TSharedPtr<FPropertyPathResolver::FOutput> Resolved =
		FPropertyPathResolver::TryResolvePath(InitialState, PropertyPath, /*bTypeOnly=*/true);
	return Resolved ? Resolved->Property : nullptr;
}

FWidgetStyleSetter FSetterElementNode::MakeSetter() const
{
	FWidgetStyleSetter Setter;
	if (const auto* Memory = static_cast<const FWidgetStyleSetter*>(GetStructMemory()))
	{
		Setter = *Memory;
	}
	return Setter;
}
