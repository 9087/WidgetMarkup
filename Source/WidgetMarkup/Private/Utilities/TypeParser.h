// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "EdGraph/EdGraphPin.h"

class FTypeParser
{
public:
	/** Maps a basic type token ("Float", "int", "FString", etc.) to a K2 pin category.
	 *  Returns NAME_None for unrecognised tokens.  Case-insensitive. */
	static FName ToPinCategory(const FString& TypeToken);

	static bool ParseType(const FString& InTypeText, FEdGraphPinType& OutPinType, FString& OutError);
	static UClass* ResolveClass(const FString& InClassText);
	static UClass* ResolveInterface(const FString& InInterfaceText);
	static UScriptStruct* ResolveStruct(const FString& InStructText);
	static UEnum* ResolveEnum(const FString& InEnumText);

	static bool ParseContainer(const FString& InTypeText, FString& OutContainerName, FString& OutInnerText, FString& OutError);

private:
	static bool ParseTypeInternal(const FString& InTypeText, FEdGraphPinType& OutPinType, FString& OutError, bool bAllowContainer);
	static bool ParseMapContainer(const FString& InInnerText, FString& OutKeyText, FString& OutValueText, FString& OutError);
	static FString NormalizeToken(const FString& InText);
};
