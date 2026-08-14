// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

class WIDGETMARKUP_API IWidgetMarkupComponent
{
public:
	virtual ~IWidgetMarkupComponent() = default;

	/** Refresh the component's display with the given data object. */
	virtual void OnDataRefresh(class UObject* Data) {}

	/**
	 * Returns an opaque pointer to the backing script instance, or nullptr if
	 * this component has no script backend. Only the owning script integration
	 * may interpret the pointer.
	 */
	virtual void* GetScriptInstance() const { return nullptr; }
};
