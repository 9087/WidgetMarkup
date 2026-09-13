// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "PropertySetters/ResyncPropertySetter.h"

#include "PropertyBuffer.h"

TSharedRef<FPropertySetter> FResyncPropertySetter::Create(FPropertyResyncDelegate InResync)
{
	return MakeShared<FResyncPropertySetter>(MoveTemp(InResync));
}

bool FResyncPropertySetter::SetValue(
	void* InContainer,
	const FWidgetPropertyPath& InPropertyPath,
	FProperty* InTargetProperty,
	void* InTargetValueAddress,
	const FPropertyBuffer& InPropertyBuffer) const
{
	if (!InContainer || !InTargetProperty || !InTargetValueAddress || !Resync)
	{
		return false;
	}

	if (!FPropertySetter::SetValue(InContainer, InPropertyPath, InTargetProperty, InTargetValueAddress, InPropertyBuffer))
	{
		return false;
	}

	// Registration only accepts direct properties of a UClass (see
	// FWidgetMarkupModule::RegisterPropertyResync) and dispatch matches the exact
	// path, so the tail container is the widget itself.
	UObject* Target = static_cast<UObject*>(InContainer);
	Resync(*Target, *InTargetProperty, InTargetValueAddress);
	return true;
}
