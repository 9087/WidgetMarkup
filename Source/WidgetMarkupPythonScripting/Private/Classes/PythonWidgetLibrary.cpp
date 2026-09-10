// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "Classes/PythonWidgetLibrary.h"
#include "Classes/PythonCore.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetTree.h"
#include "Components/PanelWidget.h"
#include "Components/Widget.h"
#include "Misc/PackageName.h"
#include "PythonUtilities.h"
#include "PythonWidgetMarkupListItem.h"
#include "WidgetMarkupModule.h"

#if defined(WITH_PYTHON) && WITH_PYTHON

namespace
{
	// -----------------------------------------------------------------------
	// Python-callable helpers
	// -----------------------------------------------------------------------

	PyObject* PyFindWidgetInUserWidget(PyObject* /*Self*/, PyObject* Args)
	{
		const char* UserWidgetNameOrPath = nullptr;
		const char* WidgetName = nullptr;
		if (!PyArg_ParseTuple(Args, "ss:find_widget_in_user_widget", &UserWidgetNameOrPath, &WidgetName))
		{
			return nullptr;
		}

		const FString UserWidgetStr(UTF8_TO_TCHAR(UserWidgetNameOrPath));
		UObject* Found = FindObject<UObject>(nullptr, *UserWidgetStr);
		UUserWidget* UserWidget = Cast<UUserWidget>(Found);
		if (!UserWidget || !UserWidget->WidgetTree || !WidgetName)
		{
			Py_RETURN_NONE;
		}

		UWidget* FoundWidget = UserWidget->WidgetTree->FindWidget(FName(UTF8_TO_TCHAR(WidgetName)));
		if (!FoundWidget)
		{
			Py_RETURN_NONE;
		}

		return PyUnicode_FromString(TCHAR_TO_UTF8(*FoundWidget->GetPathName()));
	}

	PyObject* PyGetPythonObjectFromListItem(PyObject* /*Self*/, PyObject* Args)
	{
		const char* ItemPath = nullptr;
		if (!PyArg_ParseTuple(Args, "s:get_python_object_from_list_item", &ItemPath))
		{
			return nullptr;
		}

		UPythonWidgetMarkupListItem* ListItem = FindObject<UPythonWidgetMarkupListItem>(
			nullptr, UTF8_TO_TCHAR(ItemPath));
		if (ListItem)
		{
			PyObject* PythonObject = ListItem->GetPythonObject();
			if (PythonObject)
			{
				Py_INCREF(PythonObject);
				return PythonObject;
			}
		}
		Py_RETURN_NONE;
	}

	PyObject* PyAddChildWidget(PyObject* /*Self*/, PyObject* Args)
	{
		const char* UserWidgetNameOrPath = nullptr;  // UserWidget name or path name
		const char* ParentNameOrPath = nullptr;      // parent widget name or path name
		const char* ClassToken = nullptr;            // widget class string token
		const char* ChildName = nullptr;
		if (!PyArg_ParseTuple(Args, "ssss:add_child_widget",
			&UserWidgetNameOrPath, &ParentNameOrPath, &ClassToken, &ChildName))
		{
			return nullptr;
		}

		// --- Resolve owning UserWidget ---
		const FString UserWidgetStr(UTF8_TO_TCHAR(UserWidgetNameOrPath));
		UUserWidget* UserWidget = nullptr;

		// Try FindObject by path name, then FindWidget by name.
		UObject* FoundUserWidget = FindObject<UObject>(nullptr, *UserWidgetStr);
		UserWidget = Cast<UUserWidget>(FoundUserWidget);
		if (!UserWidget)
		{
			// May be a simple name — not supported for UserWidget; require path.
			PyErr_SetString(PyExc_TypeError, "First argument must be a valid UserWidget path name.");
			return nullptr;
		}

		if (!UserWidget->WidgetTree)
		{
			PyErr_SetString(PyExc_ValueError, "UserWidget has no WidgetTree.");
			return nullptr;
		}

		// --- Resolve widget class ---
		if (!ClassToken || ClassToken[0] == '\0')
		{
			PyErr_SetString(PyExc_ValueError, "ClassToken must be a non-empty widget class name.");
			return nullptr;
		}
		UClass* WidgetClass = FWidgetMarkupCore::ResolveClass(FString(UTF8_TO_TCHAR(ClassToken)));
		if (!WidgetClass)
		{
			PyErr_SetString(PyExc_ValueError, "Could not resolve widget class.");
			return nullptr;
		}
		if (!WidgetClass->IsChildOf(UWidget::StaticClass()))
		{
			PyErr_SetString(PyExc_TypeError, "Resolved class is not a UWidget subclass.");
			return nullptr;
		}

		const FName ChildFName(UTF8_TO_TCHAR(ChildName));

		// --- Resolve parent widget ---
		const FString ParentStr(UTF8_TO_TCHAR(ParentNameOrPath));
		UWidget* ParentWidget = nullptr;

		// Try name first, then path.
		ParentWidget = UserWidget->WidgetTree->FindWidget(FName(*ParentStr));
		if (!ParentWidget)
		{
			UObject* Found = FindObject<UObject>(nullptr, *ParentStr);
			ParentWidget = Cast<UWidget>(Found);
		}

		// Verify parent lives in this UserWidget's tree.
		if (ParentWidget)
		{
			TArray<UWidget*> AllWidgets;
			UserWidget->WidgetTree->GetAllWidgets(AllWidgets);
			if (!AllWidgets.Contains(ParentWidget))
			{
				ParentWidget = nullptr;
			}
		}

		if (!ParentWidget)
		{
			PyErr_Format(PyExc_ValueError, "Parent widget '%s' not found in WidgetTree.", ParentNameOrPath);
			return nullptr;
		}

		UPanelWidget* ParentPanel = Cast<UPanelWidget>(ParentWidget);
		if (!ParentPanel)
		{
			PyErr_Format(PyExc_TypeError, "Parent widget '%s' is not a panel widget and cannot hold children.", ParentNameOrPath);
			return nullptr;
		}

		// --- Validate child name uniqueness ---
		if (UserWidget->WidgetTree->FindWidget(ChildFName))
		{
			PyErr_Format(PyExc_ValueError, "A widget named '%s' already exists in the WidgetTree.", ChildName);
			return nullptr;
		}

		// --- Validate slot capacity ---
		if (!ParentPanel->CanHaveMultipleChildren() && ParentPanel->GetChildrenCount() > 0)
		{
			PyErr_Format(PyExc_ValueError, "Parent widget '%s' only supports a single child and already has one.", ParentNameOrPath);
			return nullptr;
		}
		if (!ParentPanel->CanAddMoreChildren())
		{
			PyErr_Format(PyExc_ValueError, "Parent widget '%s' cannot accept more children.", ParentNameOrPath);
			return nullptr;
		}

		// --- Create the child widget ---
		UWidget* ChildWidget = nullptr;
		if (WidgetClass->IsChildOf(UUserWidget::StaticClass()))
		{
			ChildWidget = UUserWidget::CreateWidgetInstance(*UserWidget->WidgetTree,
				TSubclassOf<UUserWidget>(WidgetClass), ChildFName);
		}
		else
		{
			ChildWidget = UserWidget->WidgetTree->ConstructWidget<UWidget>(WidgetClass, ChildFName);
		}

		if (!ChildWidget)
		{
			PyErr_SetString(PyExc_RuntimeError, "Failed to create child widget.");
			return nullptr;
		}

		// --- Add to parent ---
		ParentPanel->AddChild(ChildWidget);

		return PyUnicode_FromString(ChildName);
	}

	PyObject* PyRemoveChildWidget(PyObject* /*Self*/, PyObject* Args)
	{
		const char* UserWidgetNameOrPath = nullptr;
		const char* NameOrPath = nullptr;  // widget name or object path name
		if (!PyArg_ParseTuple(Args, "ss:remove_child_widget", &UserWidgetNameOrPath, &NameOrPath))
		{
			return nullptr;
		}

		UUserWidget* UserWidget = FindObject<UUserWidget>(nullptr, UTF8_TO_TCHAR(UserWidgetNameOrPath));
		if (!UserWidget || !UserWidget->WidgetTree)
		{
			Py_RETURN_FALSE;
		}

		const FString NameOrPathStr(UTF8_TO_TCHAR(NameOrPath));
		UWidget* ChildWidget = nullptr;

		// Try widget name first.
		ChildWidget = UserWidget->WidgetTree->FindWidget(FName(*NameOrPathStr));
		if (!ChildWidget)
		{
			// Try object path name.
			UObject* Found = FindObject<UObject>(nullptr, *NameOrPathStr);
			ChildWidget = Cast<UWidget>(Found);
		}

		// Verify the resolved widget actually lives in this UserWidget's tree.
		if (ChildWidget)
		{
			bool bFoundInTree = false;
			TArray<UWidget*> AllWidgets;
			UserWidget->WidgetTree->GetAllWidgets(AllWidgets);
			bFoundInTree = AllWidgets.Contains(ChildWidget);
			if (!bFoundInTree)
			{
				ChildWidget = nullptr;
			}
		}

		if (!ChildWidget)
		{
			Py_RETURN_FALSE;
		}

		ChildWidget->RemoveFromParent();

		// Break the UWidget <-> SObjectWidget self-cycle so the removed widget is
		// no longer reachable through its own Slate widget. Combined with the
		// reference cleanup done in remove_child, this leaves the widget
		// unreachable and it is collected by the engine GC.
		ChildWidget->ReleaseSlateResources(true);
		Py_RETURN_TRUE;
	}

	PyMethodDef WidgetLibraryMethods[] =
	{
		{ "find_widget_in_user_widget", PyFindWidgetInUserWidget, METH_VARARGS | METH_STATIC, "Find a widget by name in a UserWidget's WidgetTree." },
		{ "get_python_object_from_list_item", PyGetPythonObjectFromListItem, METH_VARARGS | METH_STATIC, "Get the raw Python value from a PythonWidgetMarkupListItem." },
		{ "add_child_widget", PyAddChildWidget, METH_VARARGS | METH_STATIC, "Create a child widget and add it to a parent panel in the WidgetTree." },
		{ "remove_child_widget", PyRemoveChildWidget, METH_VARARGS | METH_STATIC, "Remove a child widget by name or reference." },
		{ nullptr, nullptr, 0, nullptr }
	};
}

bool RegisterPythonWidgetLibraryType(PyObject* Module)
{
	return FPythonUtilities::AddStaticMethodType(
		Module,
		"WidgetLibrary",
		"_widget_markup.WidgetLibrary",
		WidgetLibraryMethods,
		"WidgetMarkup widget lookup helpers.");
}

#endif
