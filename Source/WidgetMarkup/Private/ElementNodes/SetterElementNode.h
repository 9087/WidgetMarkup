// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "StructElementNode.h"
#include "Styles/WidgetStyleSheet.h"

class UClass;

class FSetterElementNode : public FStructElementNode
{
	DECLARE_ELEMENT_NODE(FSetterElementNode, FStructElementNode)

public:
	static TSharedRef<FElementNode> Create();

	virtual FResult OnBegin(const FContext& Context, UObject* Outer, UStruct* Struct) override;
	virtual FResult OnEnd() override;
	virtual FResult OnAddChild(const TSharedRef<FElementNode>& Child) override;

	//~Begin FElementNode interface
	/** Returns the compiled-in tail property type for Basic child elements. */
	virtual FProperty* ResolveExpectedChildProperty() override;
	//~End FElementNode interface

	FWidgetStyleSetter MakeSetter() const;

private:
	/** Compile-time, instance-free resolution of the setter property's tail type. */
	FProperty* ResolveTailProperty(const FWidgetPropertyPath& PropertyPath) const;

	/** Child elements captured in OnAddChild (Basic/Struct/Object inline values). */
	TArray<TSharedRef<FElementNode>> ChildNodes;

	/** Owning Style's TargetType, resolved in OnBegin for compile-time type lookup. */
	UClass* TargetClass = nullptr;

	/** Cached result of ResolveTailProperty (children Begin before our OnEnd). */
	FProperty* ResolvedTailProperty = nullptr;
};
