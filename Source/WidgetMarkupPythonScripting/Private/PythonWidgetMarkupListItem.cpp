// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "PythonWidgetMarkupListItem.h"

#include "PythonUtilities.h"

UPythonWidgetMarkupListItem::~UPythonWidgetMarkupListItem()
{
	if (PyValue)
	{
		Py_DECREF(PyValue);
		PyValue = nullptr;
	}
}

UPythonWidgetMarkupListItem* UPythonWidgetMarkupListItem::Create(UObject* Outer, PyObject* InPyValue)
{
	UPythonWidgetMarkupListItem* Item = NewObject<UPythonWidgetMarkupListItem>(Outer);
	if (InPyValue)
	{
		Py_INCREF(InPyValue);
		Item->PyValue = InPyValue;
	}
	return Item;
}

FString UPythonWidgetMarkupListItem::GetDisplayText() const
{
	if (PyValue)
	{
		return FPythonUtilities::PythonObjectToString(PyValue);
	}
	return TEXT("");
}
