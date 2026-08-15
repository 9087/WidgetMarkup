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
	virtual void HandleRefreshRequest() override;

	void HandlePythonInitialized();
	void HandleWidgetMarkupUserWidgetInitialized(UUserWidget* UserWidget, UWidgetMarkupBlueprintGeneratedClassExtension* GeneratedClassExtension);

	/** Locate the .py file for a Python module name under the known Python roots. */
	FString TryFindPythonModuleFile(const FString& ModuleName) const;

	IPythonScriptPlugin* PythonScriptPlugin = nullptr;

	/** Module name -> file timestamp at the last reload, used to detect which modules changed. */
	TMap<FString, FDateTime> LastReloadTimes;
};
