// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "Classes/PythonWidgetMarkupModule.h"

#include "Classes/PythonApplication.h"
#include "Classes/PythonCore.h"
#include "Classes/PythonDataBinding.h"
#include "PythonUtilities.h"
#include "Classes/PythonWidgetLibrary.h"
#include "WidgetMarkupPythonIntegration.h"

#if defined(WITH_PYTHON) && WITH_PYTHON

namespace
{
	const char* NativeModuleName = "widget_markup";

	PyMethodDef NativeModuleMethods[] =
	{
		{ nullptr, nullptr, 0, nullptr }
	};

	PyModuleDef NativeModuleDefinition =
	{
		PyModuleDef_HEAD_INIT,
		NativeModuleName,
		"Native helpers for WidgetMarkup Python integration.",
		-1,
		NativeModuleMethods,
		nullptr,
		nullptr,
		nullptr,
		nullptr
	};
}

bool RegisterPythonWidgetMarkupModule()
{
	if (!Py_IsInitialized())
	{
		UE_LOG(LogWidgetMarkupPythonIntegration, Display, TEXT("WidgetMarkup native Python module registration skipped because Python is not available yet."));
		return false;
	}

	FPythonGILScope GILScope;
	PyObject* PyModules = PyImport_GetModuleDict();
	if (!PyModules)
	{
		return false;
	}

	if (PyDict_GetItemString(PyModules, NativeModuleName))
	{
		return true;
	}

	FPythonAutoRelease PyModule(PyModule_Create(&NativeModuleDefinition));
	if (!PyModule)
	{
		const FString ErrorMessage = FPythonUtilities::ConsumePythonErrorMessage();
		UE_LOG(LogWidgetMarkupPythonIntegration, Warning, TEXT("Failed to register native Python module '%hs': %s"), NativeModuleName, *ErrorMessage);
		return false;
	}

	const bool bRegisteredTypes =
		RegisterPythonCoreType(PyModule.Get())
		&& RegisterPythonDataBindingType(PyModule.Get())
		&& RegisterPythonWidgetLibraryType(PyModule.Get())
		&& RegisterPythonApplicationType(PyModule.Get());

	if (!bRegisteredTypes)
	{
		const FString ErrorMessage = FPythonUtilities::ConsumePythonErrorMessage();
		UE_LOG(LogWidgetMarkupPythonIntegration, Warning, TEXT("Failed to register widget_markup Python types: %s"), *ErrorMessage);
		return false;
	}

	if (PyDict_SetItemString(PyModules, NativeModuleName, PyModule.Get()) != 0)
	{
		const FString ErrorMessage = FPythonUtilities::ConsumePythonErrorMessage();
		UE_LOG(LogWidgetMarkupPythonIntegration, Warning, TEXT("Failed to expose native Python module '%hs': %s"), NativeModuleName, *ErrorMessage);
		return false;
	}

	return true;
}

#else

bool RegisterPythonWidgetMarkupModule()
{
	return false;
}

#endif
