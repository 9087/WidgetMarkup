// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "UWidgetMarkupInputLibrary.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(UWidgetMarkupInputLibrary)

bool UWidgetMarkupInputLibrary::PointerEvent_IsMouseButtonDown(const FPointerEvent& Input, FKey MouseButton)
{
	return Input.IsMouseButtonDown(MouseButton);
}

FKey UWidgetMarkupInputLibrary::PointerEvent_GetEffectingButton(const FPointerEvent& Input)
{
	return Input.GetEffectingButton();
}
