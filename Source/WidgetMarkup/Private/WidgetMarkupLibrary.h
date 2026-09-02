// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"

#include "WidgetMarkupLibrary.generated.h"

/** One attribute (property) of an element or of a nested path level. */
USTRUCT(BlueprintType)
struct FWidgetMarkupAttributeInfo
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	FString Name;

	UPROPERTY(BlueprintReadOnly)
	FString Type;
};

/** Payload for GetAttributes. */
USTRUCT(BlueprintType)
struct FWidgetMarkupAttributeInfoList
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FWidgetMarkupAttributeInfo> Attributes;

	/** Empty on success; "UNKNOWN_ELEMENT" or "UNKNOWN_PATH" otherwise. */
	UPROPERTY(BlueprintReadOnly)
	FString Error;
};

/** One entry of the element index. */
USTRUCT(BlueprintType)
struct FWidgetMarkupElementInfo
{
	GENERATED_BODY()

	/** Name used in markup source (e.g. "Button", "Variable"). */
	UPROPERTY(BlueprintReadOnly)
	FString Name;

	/** "class" or "struct". */
	UPROPERTY(BlueprintReadOnly)
	FString Kind;

	/** Underlying reflected type name (e.g. "Button", "FWidgetMarkupBlueprintVariable"). */
	UPROPERTY(BlueprintReadOnly)
	FString Type;
};

/** Payload for GetElements. */
USTRUCT(BlueprintType)
struct FWidgetMarkupElementInfoList
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	TArray<FWidgetMarkupElementInfo> Elements;
};

/**
 * General-purpose WidgetMarkup function library. The attribute discovery
 * functions are exposed over the Web Remote Control HTTP API through an
 * embedded Remote Control preset bound to the class default object, so
 * editor tooling (e.g. the WidgetMarkup VSCode extension) can call them.
 */
UCLASS()
class UWidgetMarkupLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** All element names usable in markup. */
	UFUNCTION(BlueprintCallable, Category = "WidgetMarkup")
	static FWidgetMarkupElementInfoList GetElements();

	/**
	 * Attributes available at the dotted Prefix below Element. An empty Prefix
	 * returns the element's own attributes; a prefix like "Slot." or
	 * "Style." returns the fields of the nested struct it points to.
	 */
	UFUNCTION(BlueprintCallable, Category = "WidgetMarkup")
	static FWidgetMarkupAttributeInfoList GetAttributes(const FString& Element, const FString& Prefix);
};
