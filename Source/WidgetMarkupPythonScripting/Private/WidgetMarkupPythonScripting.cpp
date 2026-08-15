// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "WidgetMarkupPythonScripting.h"

#include "Blueprint/UserWidget.h"
#include "Extensions/WidgetMarkupBlueprintExtension.h"
#include "Extensions/WidgetMarkupBlueprintGeneratedClassExtension.h"
#include "Extensions/WidgetMarkupUserWidgetExtension.h"
#include "IPythonScriptPlugin.h"
#include "Interfaces/IPluginManager.h"
#include "Misc/Paths.h"
#include "PythonWidgetMarkupComponent.h"
#include "Classes/PythonWidgetMarkupModule.h"
#include "WidgetBlueprint.h"
#include "WidgetBlueprintExtension.h"
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

void FWidgetMarkupPythonScripting::HandleRefreshRequest()
{
	if (!PythonScriptPlugin || !PythonScriptPlugin->IsPythonAvailable())
	{
		return;
	}

	// Collect script modules used by compiled widgets whose source file changed
	// since the last reload. Saving a .py file only changes its timestamp; the
	// actual reload happens here, on an explicit refresh request (e.g. F5).
	TSet<FString> ModulesToReload;
	TMap<FString, FDateTime> ModuleStamps;
	for (const auto& Pair : WidgetMarkupModule.GetCompiledObjects())
	{
		if (const UWidgetBlueprint* WidgetBlueprint = Cast<UWidgetBlueprint>(Pair.Value))
		{
			if (const UWidgetMarkupBlueprintExtension* Extension = UWidgetBlueprintExtension::GetExtension<UWidgetMarkupBlueprintExtension>(WidgetBlueprint))
			{
				const FString Script = Extension->GetScript().TrimStartAndEnd();
				if (Script.IsEmpty() || ModulesToReload.Contains(Script))
				{
					continue;
				}

				const FString ModuleFile = TryFindPythonModuleFile(Script);
				if (ModuleFile.IsEmpty())
				{
					continue; // No trackable source file; leave the module as-is.
				}
				const FDateTime FileStamp = IFileManager::Get().GetTimeStamp(*ModuleFile);
				if (FileStamp == FDateTime::MinValue())
				{
					continue; // Source file is missing.
				}
				const FDateTime* LastStamp = LastReloadTimes.Find(Script);
				if (LastStamp && *LastStamp >= FileStamp)
				{
					continue; // Unchanged since the last reload.
				}

				ModulesToReload.Add(Script);
				ModuleStamps.Add(Script, FileStamp);
			}
		}
	}

	if (ModulesToReload.IsEmpty())
	{
		UE_LOG(LogWidgetMarkupPythonScripting, Display, TEXT("WidgetMarkupPythonScripting: refresh requested but no Python module changed since the last reload."));
		return;
	}

	// Reload through Python's own import machinery (importlib.reload) via the
	// PythonScriptPlugin exec channel, so reload semantics match a manual
	// reload in the Python console exactly.
	FString ReloadScript = TEXT("import importlib, sys\n");
	for (const FString& ModuleName : ModulesToReload)
	{
		ReloadScript += FString::Printf(
			TEXT("_m = '%s'\nimportlib.reload(sys.modules[_m]) if _m in sys.modules else importlib.import_module(_m)\n"),
			*ModuleName);
	}
	if (!PythonScriptPlugin->ExecPythonCommand(*ReloadScript))
	{
		UE_LOG(LogWidgetMarkupPythonScripting, Warning, TEXT("WidgetMarkupPythonScripting: one or more modules failed to reload (see the Python traceback above)."));
	}

	// Record the timestamps even on failure: a later edit updates the mtime and
	// triggers another reload attempt on the next refresh.
	for (const auto& StampPair : ModuleStamps)
	{
		LastReloadTimes.Add(StampPair.Key, StampPair.Value);
	}
}

FString FWidgetMarkupPythonScripting::TryFindPythonModuleFile(const FString& ModuleName) const
{
	const FString RelativePath = ModuleName.Replace(TEXT("."), TEXT("/")) + TEXT(".py");

	TArray<FString> Candidates;
	Candidates.Add(FPaths::ProjectContentDir() / TEXT("Python") / RelativePath);
	if (const TSharedPtr<IPlugin> Plugin = IPluginManager::Get().FindPlugin(TEXT("WidgetMarkup")))
	{
		Candidates.Add(Plugin->GetContentDir() / TEXT("Python") / RelativePath);
	}

	for (const FString& Candidate : Candidates)
	{
		if (FPaths::FileExists(Candidate))
		{
			return Candidate;
		}
	}
	return FString();
}
