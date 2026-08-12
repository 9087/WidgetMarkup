// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "EdGraph/EdGraphPin.h"
#include "ElementNodes/StructElementNode.h"
#include "PropertyBuffer.h"
#include "Utilities/TypePropertyFactory.h"

class UBlueprint;
class FArrayProperty;
class FSetProperty;
class FMapProperty;
class FStructElementNode;

class FBlueprintVariableElementNode : public FStructElementNode
{
	DECLARE_ELEMENT_NODE(FBlueprintVariableElementNode, FStructElementNode)

public:
	static TSharedRef<FElementNode> Create();

protected:
	virtual FResult OnBegin(const FContext& Context, UObject* Outer, UStruct* Struct) override;
	virtual FResult OnEnd() override;
	virtual FResult OnAddChild(const TSharedRef<FElementNode>& Child) override;

	//~Begin FElementNode interface
	virtual FProperty* ResolveExpectedChildProperty() override;
	//~End FElementNode interface

private:
	/** Parses the Type attribute and synthesizes the property tree + default buffer. */
	bool EnsureTypeParsed();

	// Child value assembly (runs in OnEnd, when all child attributes are final).
	FResult CopyScalarChild(FProperty* Property, const TSharedRef<FElementNode>& Child);

	/** Lightweight child-kind validation performed when the child is added. */
	FResult ValidateChild(const TSharedRef<FElementNode>& Child);

	TWeakObjectPtr<UBlueprint> ParentBlueprint;

	// NOTE: TypeFactory must be declared BEFORE DefaultBuffer so that on destruction
	// DefaultBuffer (which references the factory-owned property) is released first.
	FTypePropertyFactory TypeFactory;
	FPropertyBuffer DefaultBuffer;
	FEdGraphPinType CachedPinType;
	bool bTypeParsed = false;
	FString TypeParseError;
	TArray<TSharedRef<FElementNode>> DefaultValueChildren;
};
