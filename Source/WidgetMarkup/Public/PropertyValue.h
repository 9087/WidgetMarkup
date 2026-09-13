// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/UnrealType.h"

/**
 * Describes a property value type TValue: which engine property stores it, what it
 * is called, and how to copy it out of a property.
 *
 * This is the value-type side of the property setter mechanism, used by
 * FWidgetMarkupModule::RegisterPropertyResync to
 *  - reject a registration whose declared type does not match the property
 *    (IsCompatible),
 *  - read the written value for the resync callback (Read),
 *  - name the expected type in the diagnostic (GetTypeName).
 *
 * Two things the reader must get right, neither of which a plain cast of the value
 * address can:
 *  - Values are always *copied* out. The typed getters may return references into
 *    the property memory, and a resync callback usually rewrites that same memory,
 *    so a reference would be cleared as a side effect.
 *  - The C++ member type is not always the type to hand to a callback:
 *    `TArray<TObjectPtr<UObject>>` (UListView::ListItems) and
 *    `TArray<TSubclassOf<T>>` (URichTextBlock::DecoratorClasses) both read as
 *    `TArray<UObject*>` through the property, without reinterpreting memory.
 *
 * Specialize to support another value type. Add further members (writing the value
 * back into a property, for instance) when something actually needs them.
 */
template <typename TValue>
struct TPropertyValue;

template <>
struct TPropertyValue<FString>
{
	static FString GetTypeName() { return TEXT("FString"); }
	static bool IsCompatible(const FProperty& Property) { return Property.IsA<FStrProperty>(); }

	static void Read(const FProperty& Property, const void* Address, FString& Out)
	{
		Out = static_cast<const FStrProperty&>(Property).GetPropertyValue(Address);
	}
};

template <>
struct TPropertyValue<FName>
{
	static FString GetTypeName() { return TEXT("FName"); }
	static bool IsCompatible(const FProperty& Property) { return Property.IsA<FNameProperty>(); }

	static void Read(const FProperty& Property, const void* Address, FName& Out)
	{
		Out = static_cast<const FNameProperty&>(Property).GetPropertyValue(Address);
	}
};

template <>
struct TPropertyValue<FText>
{
	static FString GetTypeName() { return TEXT("FText"); }
	static bool IsCompatible(const FProperty& Property) { return Property.IsA<FTextProperty>(); }

	static void Read(const FProperty& Property, const void* Address, FText& Out)
	{
		Out = static_cast<const FTextProperty&>(Property).GetPropertyValue(Address);
	}
};

template <>
struct TPropertyValue<bool>
{
	static FString GetTypeName() { return TEXT("bool"); }
	static bool IsCompatible(const FProperty& Property) { return Property.IsA<FBoolProperty>(); }

	static void Read(const FProperty& Property, const void* Address, bool& Out)
	{
		Out = static_cast<const FBoolProperty&>(Property).GetPropertyValue(Address);
	}
};

template <>
struct TPropertyValue<float>
{
	static FString GetTypeName() { return TEXT("float"); }
	static bool IsCompatible(const FProperty& Property) { return Property.IsA<FFloatProperty>(); }

	static void Read(const FProperty& Property, const void* Address, float& Out)
	{
		Out = static_cast<const FFloatProperty&>(Property).GetPropertyValue(Address);
	}
};

template <>
struct TPropertyValue<double>
{
	static FString GetTypeName() { return TEXT("double"); }
	static bool IsCompatible(const FProperty& Property) { return Property.IsA<FDoubleProperty>(); }

	static void Read(const FProperty& Property, const void* Address, double& Out)
	{
		Out = static_cast<const FDoubleProperty&>(Property).GetPropertyValue(Address);
	}
};

template <>
struct TPropertyValue<int32>
{
	static FString GetTypeName() { return TEXT("int32"); }
	static bool IsCompatible(const FProperty& Property) { return Property.IsA<FIntProperty>(); }

	static void Read(const FProperty& Property, const void* Address, int32& Out)
	{
		Out = static_cast<const FIntProperty&>(Property).GetPropertyValue(Address);
	}
};

template <>
struct TPropertyValue<int64>
{
	static FString GetTypeName() { return TEXT("int64"); }
	static bool IsCompatible(const FProperty& Property) { return Property.IsA<FInt64Property>(); }

	static void Read(const FProperty& Property, const void* Address, int64& Out)
	{
		Out = static_cast<const FInt64Property&>(Property).GetPropertyValue(Address);
	}
};

/** Object references (direct, class and subclass references all read as UObject*). */
template <>
struct TPropertyValue<UObject*>
{
	static FString GetTypeName() { return TEXT("UObject*"); }
	static bool IsCompatible(const FProperty& Property) { return Property.IsA<FObjectPropertyBase>(); }

	static void Read(const FProperty& Property, const void* Address, UObject*& Out)
	{
		Out = static_cast<const FObjectPropertyBase&>(Property).GetObjectPropertyValue(Address);
	}
};

/** Arrays of any supported element type, read one element at a time. */
template <typename TElement>
struct TPropertyValue<TArray<TElement>>
{
	static FString GetTypeName() { return FString::Printf(TEXT("TArray<%s>"), *TPropertyValue<TElement>::GetTypeName()); }

	static bool IsCompatible(const FProperty& Property)
	{
		const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(&Property);
		return ArrayProperty && TPropertyValue<TElement>::IsCompatible(*ArrayProperty->Inner);
	}

	static void Read(const FProperty& Property, const void* Address, TArray<TElement>& Out)
	{
		const FArrayProperty& ArrayProperty = static_cast<const FArrayProperty&>(Property);
		FScriptArrayHelper ArrayHelper(&ArrayProperty, Address);
		Out.Reset(ArrayHelper.Num());
		for (int32 Index = 0; Index < ArrayHelper.Num(); ++Index)
		{
			TElement Element;
			TPropertyValue<TElement>::Read(*ArrayProperty.Inner, ArrayHelper.GetRawPtr(Index), Element);
			Out.Add(MoveTemp(Element));
		}
	}
};
