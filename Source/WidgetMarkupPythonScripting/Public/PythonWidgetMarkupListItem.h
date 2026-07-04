// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "PythonUtilities.h"
#include "UObject/Object.h"
#include "PythonWidgetMarkupListItem.generated.h"

/**
 * List item data wrapper that caches a Python object for WidgetMarkup ListView items.
 * Lives in WidgetMarkupPythonScripting so it can hold a PyObject reference.
 */
UCLASS(BlueprintType)
class WIDGETMARKUPPYTHONSCRIPTING_API UPythonWidgetMarkupListItem : public UObject
{
	GENERATED_BODY()

public:
	~UPythonWidgetMarkupListItem();

	/** Creates a list item that caches the given Python value. */
	static UPythonWidgetMarkupListItem* Create(UObject* Outer, PyObject* InPyValue);

	/** Returns the cached Python object, borrowed reference. */
	PyObject* GetPythonObject() const { return PyValue; }

	/** Returns a display string representation of the cached value. */
	UFUNCTION(BlueprintCallable, Category = "WidgetMarkup")
	FString GetDisplayText() const;

private:
	// Cached Python value (new reference).
	PyObject* PyValue = nullptr;
};
