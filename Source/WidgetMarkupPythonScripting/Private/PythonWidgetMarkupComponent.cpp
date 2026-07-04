// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "PythonWidgetMarkupComponent.h"

#include "Blueprint/UserWidget.h"
#include "IPythonScriptPlugin.h"
#include "PythonUtilities.h"
#include "PythonWidgetMarkupListItem.h"
#include "WidgetMarkupPythonScripting.h"

TSharedPtr<FPythonWidgetMarkupComponent> FPythonWidgetMarkupComponent::Create(UUserWidget* InUserWidget, const FString& InScript)
{
	if (!InUserWidget)
	{
		UE_LOG(LogWidgetMarkupPythonScripting, Warning, TEXT("Cannot create Python component because UserWidget is null."));
		return nullptr;
	}

	const FString Script = InScript.TrimStartAndEnd();
	if (Script.IsEmpty())
	{
		UE_LOG(LogWidgetMarkupPythonScripting, Warning, TEXT("Script is empty."));
		return nullptr;
	}

	const FString ScriptModuleName = Script;

	IPythonScriptPlugin* PythonScriptPlugin = IPythonScriptPlugin::Get();
	if (!PythonScriptPlugin)
	{
		UE_LOG(LogWidgetMarkupPythonScripting, Warning, TEXT("PythonScriptPlugin module is not loaded."));
		return nullptr;
	}

	if (!PythonScriptPlugin->IsPythonAvailable())
	{
		UE_LOG(LogWidgetMarkupPythonScripting, Warning, TEXT("Python is not available; Script '%s' was skipped."), *Script);
		return nullptr;
	}

#if !defined(WITH_PYTHON) || !WITH_PYTHON
	UE_LOG(LogWidgetMarkupPythonScripting, Warning, TEXT("WITH_PYTHON is disabled, cannot create python component for Script '%s'."), *Script);
	return nullptr;
#else
	FPythonGILScope GILScope;

	const TCHAR* WidgetMarkupComponentModuleName = TEXT("WidgetMarkupComponent");
	const char* WidgetMarkupComponentClassName = "WidgetMarkupComponent";
	const char* WidgetMarkupComponentCreateFunctionName = "create";

	FPythonAutoRelease PyComponentBaseModule(PyImport_ImportModule(TCHAR_TO_UTF8(WidgetMarkupComponentModuleName)));
	if (!PyComponentBaseModule)
	{
		const FString ErrorMessage = FPythonUtilities::ConsumePythonErrorMessage();
		UE_LOG(LogWidgetMarkupPythonScripting, Warning, TEXT("Failed to import component base module '%s' for Script '%s': %s"), WidgetMarkupComponentModuleName, *Script, *ErrorMessage);
		return nullptr;
	}

	FPythonAutoRelease PyComponentBaseClass(PyObject_GetAttrString(PyComponentBaseModule.Get(), WidgetMarkupComponentClassName));
	if (!PyComponentBaseClass)
	{
		const FString ErrorMessage = FPythonUtilities::ConsumePythonErrorMessage();
		UE_LOG(LogWidgetMarkupPythonScripting, Warning, TEXT("Module '%s' does not expose class '%hs' for Script '%s': %s"), WidgetMarkupComponentModuleName, WidgetMarkupComponentClassName, *Script, *ErrorMessage);
		return nullptr;
	}

	FPythonAutoRelease PyFactoryFunction(PyObject_GetAttrString(PyComponentBaseClass.Get(), WidgetMarkupComponentCreateFunctionName));
	if (!PyFactoryFunction)
	{
		const FString ErrorMessage = FPythonUtilities::ConsumePythonErrorMessage();
		UE_LOG(LogWidgetMarkupPythonScripting, Warning, TEXT("Class '%s.%hs' does not expose static method '%hs' for Script '%s': %s"), WidgetMarkupComponentModuleName, WidgetMarkupComponentClassName, WidgetMarkupComponentCreateFunctionName, *Script, *ErrorMessage);
		return nullptr;
	}

	if (!PyCallable_Check(PyFactoryFunction.Get()))
	{
		UE_LOG(LogWidgetMarkupPythonScripting, Warning, TEXT("'%s.%hs.%hs' is not callable for Script '%s'."), WidgetMarkupComponentModuleName, WidgetMarkupComponentClassName, WidgetMarkupComponentCreateFunctionName, *Script);
		return nullptr;
	}

	TSharedPtr<FPythonWidgetMarkupComponent> PythonWidgetMarkupComponent = MakeShareable(new FPythonWidgetMarkupComponent());

	FPythonAutoRelease PyUserWidget(PyUnicode_FromString(TCHAR_TO_UTF8(*InUserWidget->GetPathName())));
	FPythonAutoRelease PyArgs(PyTuple_New(2));
	if (!PyArgs)
	{
		const FString ErrorMessage = FPythonUtilities::ConsumePythonErrorMessage();
		UE_LOG(LogWidgetMarkupPythonScripting, Warning, TEXT("Failed to allocate create arguments for Script '%s': %s"), *Script, *ErrorMessage);
		return nullptr;
	}

	FPythonAutoRelease PyModuleNameArgument(PyUnicode_FromString(TCHAR_TO_UTF8(*ScriptModuleName)));
	if (!PyModuleNameArgument)
	{
		const FString ErrorMessage = FPythonUtilities::ConsumePythonErrorMessage();
		UE_LOG(LogWidgetMarkupPythonScripting, Warning, TEXT("Failed to convert module name '%s' to python argument for Script '%s': %s"), *ScriptModuleName, *Script, *ErrorMessage);
		return nullptr;
	}

	PyTuple_SET_ITEM(PyArgs.Get(), 0, PyModuleNameArgument.Release());
	PyTuple_SET_ITEM(PyArgs.Get(), 1, PyUserWidget.Release());

	FPythonAutoRelease PyInstanceObject(PyObject_CallObject(PyFactoryFunction.Get(), PyArgs.Get()));
	if (!PyInstanceObject)
	{
		const FString ErrorMessage = FPythonUtilities::ConsumePythonErrorMessage();
		UE_LOG(LogWidgetMarkupPythonScripting, Warning, TEXT("'%s.%hs.%hs' failed for module '%s' (Script '%s'): %s"), WidgetMarkupComponentModuleName, WidgetMarkupComponentClassName, WidgetMarkupComponentCreateFunctionName, *ScriptModuleName, *Script, *ErrorMessage);
		return nullptr;
	}

	PythonWidgetMarkupComponent->PythonComponentInstance = PyInstanceObject.Release();

	return PythonWidgetMarkupComponent;
#endif
}

FPythonWidgetMarkupComponent::~FPythonWidgetMarkupComponent()
{
	if (!PythonComponentInstance)
	{
		return;
	}

	#if defined(WITH_PYTHON) && WITH_PYTHON
	FPythonGILScope GILScope;
	Py_DECREF(reinterpret_cast<PyObject*>(PythonComponentInstance));
	#endif

	PythonComponentInstance = nullptr;
}

void FPythonWidgetMarkupComponent::OnDataRefresh(UObject* Data)
{
	if (!PythonComponentInstance || !Data)
	{
		return;
	}

	UPythonWidgetMarkupListItem* ListItem = Cast<UPythonWidgetMarkupListItem>(Data);
	if (!ListItem)
	{
		return;
	}

	PyObject* PythonObject = ListItem->GetPythonObject();
	if (!PythonObject)
	{
		return;
	}

#if defined(WITH_PYTHON) && WITH_PYTHON
	FPythonGILScope GILScope;
	PyObject* PyInstance = reinterpret_cast<PyObject*>(PythonComponentInstance);
	FPythonAutoRelease PyResult(PyObject_CallMethod(PyInstance, "refresh", "O", PythonObject));
	if (!PyResult)
	{
		PyErr_Print();
	}
#endif
}
