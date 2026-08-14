// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "Classes/PythonCore.h"

#include "Blueprint/UserWidget.h"
#include "Components/IWidgetMarkupComponent.h"
#include "Components/Widget.h"
#include "Engine/Blueprint.h"
#include "Extensions/WidgetMarkupUserWidgetExtension.h"
#include "Misc/PackageName.h"
#include "Modules/ModuleManager.h"
#include "PythonUtilities.h"
#include "UObject/UObjectIterator.h"
#include "WidgetMarkupModule.h"

#if defined(WITH_PYTHON) && WITH_PYTHON

// -----------------------------------------------------------------------
// Shared implementation (also used by PythonWidgetLibrary)
// -----------------------------------------------------------------------

UClass* FWidgetMarkupCore::ResolveClass(const FString& Token)
{
	if (Token.IsEmpty())
	{
		return nullptr;
	}

	// 1. TryFindTypeSlow for short names and path-like tokens.
	if (UClass* Class = UClass::TryFindTypeSlow<UClass>(Token, EFindFirstObjectOptions::None))
	{
		return Class;
	}

	// 2. "/Game/..." – WidgetMarkup package path.
	if (Token.StartsWith(TEXT("/")))
	{
		if (FWidgetMarkupModule* Module = FModuleManager::GetModulePtr<FWidgetMarkupModule>(TEXT("WidgetMarkup")))
		{
			if (UObject* Compiled = Module->GetObjectOrCompileFromPackage(Token))
			{
				if (UBlueprint* BP = Cast<UBlueprint>(Compiled))
				{
					return BP->GeneratedClass;
				}
			}
		}
		return nullptr;
	}

	// 3. Dot-notation ("Game.WidgetMarkup.MyWidget").
	if (Token.Contains(TEXT(".")) && !Token.Contains(TEXT("/")))
	{
		const FString PackagePath = TEXT("/") + Token.Replace(TEXT("."), TEXT("/"));

		if (FWidgetMarkupModule* Module = FModuleManager::GetModulePtr<FWidgetMarkupModule>(TEXT("WidgetMarkup")))
		{
			if (UObject* Compiled = Module->GetObjectOrCompileFromPackage(PackagePath))
			{
				if (UBlueprint* BP = Cast<UBlueprint>(Compiled))
				{
					return BP->GeneratedClass;
				}
			}
		}

		const FString AssetName = FPackageName::GetShortName(PackagePath);
		const FString ClassPath = FString::Printf(TEXT("%s.%s_C"), *PackagePath, *AssetName);
		if (UClass* Class = LoadObject<UClass>(nullptr, *ClassPath))
		{
			return Class;
		}

		return nullptr;
	}

	// 4. Case-sensitive exact match via object iterator.
	for (TObjectIterator<UClass> It; It; ++It)
	{
		if (It->GetName().Equals(Token, ESearchCase::CaseSensitive))
		{
			return *It;
		}
	}

	return nullptr;
}

namespace
{

	PyObject* PyResolveClass(PyObject* /*Self*/, PyObject* Args)
	{
		const char* ClassToken = nullptr;
		if (!PyArg_ParseTuple(Args, "s:resolve_class", &ClassToken))
		{
			return nullptr;
		}

		UClass* Resolved = FWidgetMarkupCore::ResolveClass(FString(UTF8_TO_TCHAR(ClassToken)));
		if (!Resolved)
		{
			Py_RETURN_NONE;
		}

		return PyUnicode_FromString(TCHAR_TO_UTF8(*Resolved->GetPathName()));
	}

	PyObject* PyGetComponentByWidget(PyObject* /*Self*/, PyObject* Args)
	{
		const char* WidgetPathName = nullptr;
		if (!PyArg_ParseTuple(Args, "s:get_component_by_widget", &WidgetPathName))
		{
			return nullptr;
		}

		UWidget* Widget = FindObject<UWidget>(nullptr, UTF8_TO_TCHAR(WidgetPathName));
		if (!Widget)
		{
			Py_RETURN_NONE;
		}

		if (UUserWidget* UserWidget = Cast<UUserWidget>(Widget))
		{
			if (UWidgetMarkupUserWidgetExtension* Extension =
				UWidgetMarkupUserWidgetExtension::GetOrAddExtension(UserWidget))
			{
				if (const TSharedPtr<IWidgetMarkupComponent>& Component = Extension->GetWidgetMarkupComponent())
				{
					if (void* PyInstance = Component->GetScriptInstance())
					{
						PyObject* PyObj = static_cast<PyObject*>(PyInstance);
						Py_INCREF(PyObj);
						return PyObj;
					}
				}
			}
		}

		Py_RETURN_NONE;
	}

	PyMethodDef CoreMethods[] =
	{
		{ "resolve_class", PyResolveClass, METH_VARARGS | METH_STATIC, "Resolve a class token to a UClass (short name, dot-notation, or /Game/ path)." },
		{ "get_component_by_widget", PyGetComponentByWidget, METH_VARARGS | METH_STATIC, "Get the Python WidgetMarkupComponent for a UWidget, or None." },
		{ nullptr, nullptr, 0, nullptr }
	};
}

bool RegisterPythonCoreType(PyObject* Module)
{
	return FPythonUtilities::AddStaticMethodType(
		Module,
		"Core",
		"_widget_markup.Core",
		CoreMethods,
		"WidgetMarkup core utilities (class resolution, component lookup).");
}

#endif
