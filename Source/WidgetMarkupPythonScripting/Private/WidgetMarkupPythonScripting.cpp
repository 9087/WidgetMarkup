// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "WidgetMarkupPythonScripting.h"

#include "Blueprint/UserWidget.h"
#include "Extensions/WidgetMarkupBlueprintGeneratedClassExtension.h"
#include "Extensions/WidgetMarkupUserWidgetExtension.h"
#include "IPythonScriptPlugin.h"
#include "Misc/Paths.h"
#include "PythonWidgetMarkupComponent.h"
#include "Classes/PythonWidgetMarkupModule.h"
#include "WidgetMarkupModule.h"

FWidgetMarkupPythonScripting::FWidgetMarkupPythonScripting(FWidgetMarkupModule& InWidgetMarkupModule)
	: IWidgetMarkupScriptIntegration(InWidgetMarkupModule)
{
	PythonScriptPlugin = IPythonScriptPlugin::Get();
	if (!PythonScriptPlugin)
	{
		Initialize(false);
		return;
	}

	WidgetMarkupModule.GetOnWidgetMarkupUserWidgetInitialized().AddRaw(this, &FWidgetMarkupPythonScripting::HandleWidgetMarkupUserWidgetInitialized);

	if (PythonScriptPlugin->IsPythonAvailable())
	{
		HandlePythonInitialized();
	}
  else
  {
    PythonScriptPlugin->OnPythonInitialized().AddRaw(this, &FWidgetMarkupPythonScripting::HandlePythonInitialized);
  }
}

void FWidgetMarkupPythonScripting::HandlePythonInitialized()
{
	// Ensure the project's Content/Python directory is on sys.path so top-level
	// scripts (e.g. ScientificCalculatorComponent) are importable.
	const FString ProjectPythonPath = FPaths::ProjectContentDir() / TEXT("Python");
	if (PythonScriptPlugin)
	{
		const FString Command = FString::Printf(
			TEXT("import sys; p = r'%s'; sys.path.insert(0, p) if p not in sys.path else None"),
			*ProjectPythonPath);
		PythonScriptPlugin->ExecPythonCommand(*Command);
		UE_LOG(LogWidgetMarkupPythonScripting, Display,
			TEXT("WidgetMarkupPythonScripting: registered project Python path '%s'."), *ProjectPythonPath);
	}

	Initialize(true);
}

void FWidgetMarkupPythonScripting::HandleWidgetMarkupUserWidgetInitialized(UUserWidget* UserWidget, UWidgetMarkupBlueprintGeneratedClassExtension* GeneratedClassExtension)
{
	if (!UserWidget || !GeneratedClassExtension)
	{
		UE_LOG(LogWidgetMarkupPythonScripting, Warning, TEXT("WidgetMarkupPythonScripting: HandleWidgetMarkupUserWidgetInitialized called with null UserWidget or Extension."));
		return;
	}

	const FString Script = GeneratedClassExtension->GetScript().TrimStartAndEnd();
	if (Script.IsEmpty())
	{
		UE_LOG(LogWidgetMarkupPythonScripting, Display, TEXT("WidgetMarkupPythonScripting: skipped component creation because Script is empty for UserWidget '%s'."), *GetNameSafe(UserWidget));
		return;
	}

	UE_LOG(LogWidgetMarkupPythonScripting, Display, TEXT("WidgetMarkupPythonScripting: creating Python component for Script '%s' on UserWidget '%s'."), *Script, *GetNameSafe(UserWidget));

	RegisterPythonWidgetMarkupModule();
	UWidgetMarkupUserWidgetExtension::GetOrAddExtension(UserWidget)->SetWidgetMarkupComponent(FPythonWidgetMarkupComponent::Create(UserWidget, Script));
}
