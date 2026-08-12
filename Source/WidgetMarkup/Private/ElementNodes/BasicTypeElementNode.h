// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "ElementNode.h"

struct FPropertyBuffer;

/**
 * Leaf element node for basic-type literal values (float, int, FString, etc.)
 * inside TArray properties.
 *
 * Usage in XML:
 *   <ColumnFill>
 *     <float>1.0</float>
 *     <float>2.0</float>
 *   </ColumnFill>
 *
 * The element name (e.g. "float") is validated against FTypeParser::ToPinCategory()
 * and ignored during resolution — the actual FProperty type is derived from the
 * parent array's inner type.  The text value "1.0" is converted via
 * FConverterRegistry at Begin() time so that OnAddChild can immediately
 * copy it into the parent array.
 */
class FBasicTypeElementNode : public FElementNode
{
	DECLARE_ELEMENT_NODE(FBasicTypeElementNode, FElementNode)

public:
	explicit FBasicTypeElementNode(const FStringView& InTypeName);

	// ---- FElementNode overrides ----
	virtual void SetElementData(const TCHAR* InElementData) override;
	virtual FResult OnBegin(const FContext& Context, UObject* Outer, UStruct* Struct) override;
	virtual FResult OnEnd() override;
	virtual FResult OnAddChild(const TSharedRef<FElementNode>& Child) override;
	virtual bool HasProperty(const FStringView& AttributeName) override;
	virtual UObject* GetObject() const override { return nullptr; }
	virtual UStruct* GetPropertyOwnerStruct() const override { return nullptr; }

	/** Returns the buffered value, populated after Begin(). */
	TSharedPtr<const FPropertyBuffer> GetValueBuffer() const { return ValueBuffer; }

	/** Returns the raw text value (set via SetElementData). */
	const FString& GetValueString() const { return ValueString; }

private:
	FString TypeName;                         // "float", "int", "FString", etc.
	FString ValueString;                      // "1.0" (set via SetElementData)
	TSharedPtr<FPropertyBuffer> ValueBuffer;  // Parsed value; populated in Begin()
};
