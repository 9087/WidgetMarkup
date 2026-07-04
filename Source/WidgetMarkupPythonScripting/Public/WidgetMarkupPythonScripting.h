// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WidgetMarkupScriptIntegration.h"

WIDGETMARKUPPYTHONSCRIPTING_API DECLARE_LOG_CATEGORY_EXTERN(LogWidgetMarkupPythonScripting, Log, All);

class IPythonScriptPlugin;
class UUserWidget;
class UWidgetMarkupBlueprintGeneratedClassExtension;
class FWidgetMarkupPythonScriptingModule;

class WIDGETMARKUPPYTHONSCRIPTING_API FWidgetMarkupPythonScripting : public IWidgetMarkupScriptIntegration
{
public:
	explicit FWidgetMarkupPythonScripting(FWidgetMarkupModule& InWidgetMarkupModule);

protected:
	void HandlePythonInitialized();
	void HandleWidgetMarkupUserWidgetInitialized(UUserWidget* UserWidget, UWidgetMarkupBlueprintGeneratedClassExtension* GeneratedClassExtension);

	IPythonScriptPlugin* PythonScriptPlugin = nullptr;
};
