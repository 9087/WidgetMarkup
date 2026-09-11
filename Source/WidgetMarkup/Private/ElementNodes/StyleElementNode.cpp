// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "StyleElementNode.h"

#include "SetterElementNode.h"

IMPLEMENT_ELEMENT_NODE(FStyleElementNode, FStructElementNode)

TSharedRef<FElementNode> FStyleElementNode::Create()
{
	return MakeShared<FStyleElementNode>();
}

FElementNode::FResult FStyleElementNode::OnEnd()
{
	return FStructElementNode::OnEnd();
}

FWidgetStyleEntry FStyleElementNode::MakeStyle() const
{
	FWidgetStyleEntry Entry;
	if (const auto* Memory = static_cast<const FWidgetStyleEntry*>(GetStructMemory()))
	{
		Entry.TargetType = Memory->TargetType;
		Entry.Name = Memory->Name;
		Entry.Base = Memory->Base;
	}
	for (const TSharedPtr<FSetterElementNode>& Node : SetterNodes)
	{
		if (Node.IsValid())
		{
			Entry.Setters.Add(Node->MakeSetter());
		}
	}
	return Entry;
}

FElementNode::FResult FStyleElementNode::OnAddChild(const TSharedRef<FElementNode>& Child)
{
	if (auto SetterNode = CastElementNode<FSetterElementNode>(TSharedPtr<FElementNode>(Child)))
	{
		SetterNodes.Add(StaticCastSharedPtr<FSetterElementNode>(TSharedPtr<FElementNode>(Child)));
		return FResult::Success();
	}
	return FStructElementNode::OnAddChild(Child);
}
