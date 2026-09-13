// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "PropertySetter.h"

/**
 * Setter created by FWidgetMarkupModule::RegisterPropertyResync: writes the
 * property, then hands the result to the registered resync callback.
 *
 * Writing first keeps reflection, serialization and the engine's own
 * PostInitProperties/PostLoad expansion consistent; the callback then applies the
 * value through the widget's runtime API for properties where a plain copy is not
 * enough (the engine reads them only at construction, or only through a dedicated
 * API - e.g. UComboBoxString::DefaultOptions).
 */
class FResyncPropertySetter : public FPropertySetter
{
public:
	static TSharedRef<FPropertySetter> Create(FPropertyResyncDelegate InResync);

	explicit FResyncPropertySetter(FPropertyResyncDelegate InResync)
		: Resync(MoveTemp(InResync))
	{
	}

	virtual bool SetValue(
		void* InContainer,
		const FWidgetPropertyPath& InPropertyPath,
		FProperty* InTargetProperty,
		void* InTargetValueAddress,
		const FPropertyBuffer& InPropertyBuffer) const override;

private:
	FPropertyResyncDelegate Resync;
};
