// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "ElementNode.h"

#include "WidgetMarkupModule.h"

const FElementNodeClass* FElementNode::StaticClass()
{
	static const FElementNodeClass Instance(nullptr);
	return &Instance;
}

FElementNode::FMessage::FMessage(const FText& InText, EMessageType InType)
	: Text(InText)
	, Type(InType)
{
}

const FText& FElementNode::FMessage::GetText() const
{
	return Text;
}

FElementNode::FResult& FElementNode::FResult::Log(const FText& MessageText)
{
	Messages.Add(MakeShared<FMessage>(MessageText, EMessageType::Log));
	return *this;
}

FElementNode::FResult& FElementNode::FResult::Error(const FText& MessageText)
{
	Messages.Add(MakeShared<FMessage>(MessageText, EMessageType::Error));
	return *this;
}

FElementNode::FResult FElementNode::FResult::Success()
{
	FResult Result;
	Result.bOK = true;
	return Result;
}

FElementNode::FResult FElementNode::FResult::Failure()
{
	FResult Result;
	Result.bOK = false;
	return Result;
}

FElementNode::FResult::operator bool() const
{
	return bOK;
}

bool FElementNode::FResult::operator!() const
{
	return !bOK;
}

FElementNode::FResult& FElementNode::FResult::PrintOnFailure()
{
	if (!bOK)
	{
		UE_LOG(LogWidgetMarkup, Error, TEXT("Failed to Compile the Widget Markup."));
		for (const auto& Message : Messages)
		{
			UE_LOG(LogWidgetMarkup, Error, TEXT("%s"), *Message.Get().GetText().ToString());
		}
	}
	return *this;
}

FText FElementNode::FResult::GetFirstErrorText() const
{
	for (const TSharedRef<FMessage>& Message : Messages)
	{
		if (Message->GetType() == EMessageType::Error)
		{
			return Message->GetText();
		}
	}
	return FText();
}

void FElementNode::FContext::Push(const TSharedRef<FElementNode>& ElementNode)
{
	Nodes.Push(ElementNode);
}

TSharedRef<FElementNode> FElementNode::FContext::Pop()
{
	return Nodes.Pop();
}

bool FElementNode::FContext::IsEmpty() const
{
	return Nodes.IsEmpty();
}

TSharedPtr<FElementNode> FElementNode::FContext::GetLastNode() const
{
	if (Nodes.IsEmpty())
	{
		return nullptr;
	}
	return Nodes.Last();
}

TSharedPtr<FElementNode> FElementNode::FContext::GetLastObjectNode() const
{
	for (int32 Index = Nodes.Num() - 1; Index >= 0; --Index)
	{
		TSharedPtr<FElementNode> Node = Nodes[Index];
		if (Node.IsValid() && Node->GetObject() != nullptr)
		{
			return Node;
		}
	}
	return nullptr;
}

UObject* FElementNode::FContext::FindObject(UClass* Class) const
{
	if (!Class)
	{
		return nullptr;
	}

	for (int32 Index = Nodes.Num() - 1; Index >= 0; --Index)
	{
		const TSharedRef<FElementNode>& Node = Nodes[Index];
		if (UObject* Object = Node->GetObject(); Object && Object->IsA(Class))
		{
			return Object;
		}
	}
	return nullptr;
}

FElementNode::FElementNode()
{
}

FElementNode::FResult FElementNode::Begin(const FContext& Context, UObject* Outer, UStruct* Struct)
{
	return OnBegin(Context, Outer, Struct);
}

FElementNode::FResult FElementNode::End()
{
	return OnEnd();
}

FElementNode::~FElementNode()
{
}