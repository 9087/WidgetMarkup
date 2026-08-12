// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Field.h"

class FProperty;
class UStruct;
struct FEdGraphPinType;

/**
 * Synthesizes an FProperty (including Array/Set/Map containers) from an
 * FEdGraphPinType so the plugin's converter registry (channel A) can be used
 * on blueprint Variable default values. Owns the created property tree for its
 * lifetime; Reset() or destruction releases it.
 *
 * Only the property TYPES supported by the plugin's type system are handled;
 * unsupported pin categories produce a descriptive error.
 */
class FTypePropertyFactory
{
public:
	FTypePropertyFactory();
	~FTypePropertyFactory();

	FTypePropertyFactory(const FTypePropertyFactory&) = delete;
	FTypePropertyFactory& operator=(const FTypePropertyFactory&) = delete;

	/** Builds the property tree for PinType. Returns nullptr on failure (OutError filled). */
	FProperty* CreateProperty(const FEdGraphPinType& PinType, FString& OutError);

	/** Releases the current property tree and its owner struct. */
	void Reset();

	FProperty* GetProperty() const { return Property; }

private:
	FProperty* CreateScalarProperty(const FEdGraphPinType& PinType, FString& OutError, const FFieldVariant& InOwner);
	FProperty* CreateContainerProperty(const FEdGraphPinType& PinType, FString& OutError);
	FProperty* CreateInnerProperty(FProperty* InContainer, const FEdGraphPinType& PinType, FString& OutError);

	/** Sets CPF_HasGetValueTypeHash on scalar properties (structs only if hashable). */
	void SetHashableFlags(FProperty* Property);

	/** Allocates a runtime FProperty owned by InOwner (containers own their inner fields). */
	template <typename TField>
	TField* AllocateField(const FFieldVariant& InOwner)
	{
		return new TField(InOwner, NAME_None, RF_NoFlags);
	}

	UStruct* OwnerStruct = nullptr;
	FProperty* Property = nullptr;
};
