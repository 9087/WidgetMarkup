// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InputCoreTypes.h"
#include "Input/Events.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UWidgetMarkupInputLibrary.generated.h"

UCLASS(meta = (ScriptName = "WidgetMarkupInputLibrary"))
class WIDGETMARKUPPYTHONINTEGRATION_API UWidgetMarkupInputLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintPure, Category = "WidgetMarkup|Input")
	static bool PointerEvent_IsMouseButtonDown(UPARAM(ref) const FPointerEvent& Input, FKey MouseButton);

	UFUNCTION(BlueprintPure, Category = "WidgetMarkup|Input")
	static FKey PointerEvent_GetEffectingButton(UPARAM(ref) const FPointerEvent& Input);
};
