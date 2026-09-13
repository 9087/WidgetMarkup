// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Templates/Function.h"
#include "Utilities/WidgetPropertyPath.h"

class FProperty;
struct FPropertyBuffer;

/**
 * Called by a resync setter after the property value has been written, to apply it
 * through the widget's own runtime API.
 *
 * @param InTarget The object that owns the property (the widget).
 * @param InProperty The property that was just written.
 * @param InValueAddress Address of the written value, for setters that must write
 *        it again (see the SelectedOption registration in WidgetMarkupModule.cpp).
 */
using FPropertyResyncDelegate = TFunction<void(UObject& InTarget, const FProperty& InProperty, const void* InValueAddress)>;

class WIDGETMARKUP_API FPropertySetter : public TSharedFromThis<FPropertySetter>
{
public:
	virtual ~FPropertySetter() = default;

	/**
	 * Applies a prepared property buffer to the resolved target property.
	 *
	 * @param InContainer The container object/struct that owns the target property.
	 * @param InPropertyPath Canonical property path used to resolve the target.
	 * @param InTargetProperty The resolved target property definition.
	 * @param InTargetValueAddress The resolved writable target value address.
	 * @param InPropertyBuffer The source value buffer to apply.
	 */
	virtual bool SetValue(
		void* InContainer,
		const FWidgetPropertyPath& InPropertyPath,
		FProperty* InTargetProperty,
		void* InTargetValueAddress,
		const FPropertyBuffer& InPropertyBuffer) const;
};
