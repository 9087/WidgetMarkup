// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "WidgetMarkupPythonScripting.h"

#include "Modules/ModuleManager.h"
#include "WidgetMarkupModule.h"

DEFINE_LOG_CATEGORY(LogWidgetMarkupPythonScripting);

class FWidgetMarkupPythonScriptingModule : public IModuleInterface
{
public:
	virtual void StartupModule() override
	{
		FWidgetMarkupModule::Get().StartUp<FWidgetMarkupPythonScripting>();
	}

	virtual void ShutdownModule() override
	{
		FWidgetMarkupModule::Get().Shutdown();
	}
};

IMPLEMENT_MODULE(FWidgetMarkupPythonScriptingModule, WidgetMarkupPythonScripting)