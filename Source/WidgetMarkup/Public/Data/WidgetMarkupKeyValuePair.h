// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

#include "WidgetMarkupKeyValuePair.generated.h"

/** Key/value pair element used for Map variable defaults (alias "Pair"). */
USTRUCT()
struct FWidgetMarkupKeyValuePair
{
	GENERATED_BODY()

	UPROPERTY()
	FString Key;

	UPROPERTY()
	FString Value;
};
