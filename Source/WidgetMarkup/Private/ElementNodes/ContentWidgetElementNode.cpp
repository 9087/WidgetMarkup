// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "ContentWidgetElementNode.h"

#include "Components/ContentWidget.h"
#include "Components/Widget.h"

IMPLEMENT_ELEMENT_NODE(FContentWidgetElementNode, FPanelWidgetElementNode)

TSharedRef<FElementNode> FContentWidgetElementNode::Create()
{
	return MakeShared<FContentWidgetElementNode>();
}

FElementNode::FResult FContentWidgetElementNode::OnAddChild(const TSharedRef<FElementNode>& Child)
{
	auto ContentWidget = Cast<UContentWidget>(Object);
	if (!ensure(ContentWidget))
	{
		return FResult::Failure().Error(FText::FromString(TEXT("ContentWidgetElementNode: underlying object is not a valid UContentWidget instance.")));
	}

	// A content widget accepts multiple logical element nodes (properties, structs, scalars)
	// but only one widget child in its content slot.
	if (!Cast<UWidget>(Child->GetObject()))
	{
		return FResult::Success();
	}

	if (ContentWidget->GetContent() != nullptr)
	{
		return FResult::Failure().Error(FText::Format(FText::FromString(TEXT("{0} instance only allows one child.")), Object->GetClass()->GetDisplayNameText()));
	}
	return FPanelWidgetElementNode::OnAddChild(Child);
}
