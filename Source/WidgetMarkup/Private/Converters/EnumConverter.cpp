// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "EnumConverter.h"

#include "UObject/UnrealType.h"

TSharedRef<FConverter> FEnumConverter::Create()
{
	return MakeShared<FEnumConverter>();
}

bool FEnumConverter::Convert(const FStringView& String, UEnum* Enum, int64& OutValue)
{
	if (!Enum)
	{
		return false;
	}
	const FString Str(String);
	OutValue = Enum->GetValueByNameString(Str, EGetByNameFlags::CaseSensitive);
	// NOTE(2026-08-11): DisplayName shorthand matching is disabled so enums must
	// use their full value name (e.g. "VAlign_Center", not "Center"). Kept
	// commented out for reference in case shorthand matching is re-enabled later.
	// if (OutValue == INDEX_NONE)
	// {
	// 	// Fallback: match by DisplayName metadata (e.g. "Fill" → HAlign_Fill).
	// 	for (int32 i = 0; i < Enum->NumEnums() - 1; ++i)
	// 	{
	// 		const FString& DisplayName = Enum->GetMetaData(TEXT("DisplayName"), i);
	// 		if (!DisplayName.IsEmpty() && DisplayName.Equals(Str, ESearchCase::IgnoreCase))
	// 		{
	// 			OutValue = Enum->GetValueByIndex(i);
	// 			break;
	// 		}
	// 	}
	// }
	return OutValue != INDEX_NONE;
}

bool FEnumConverter::Convert(const FProperty& Property, void* Data, const FStringView& String)
{
	if (auto* EnumProperty = CastField<FEnumProperty>(&Property))
	{
		UEnum* Enum = EnumProperty->GetEnum();
		int64 EnumValue = INDEX_NONE;
		if (!Convert(String, Enum, EnumValue))
		{
			return false;
		}
		EnumProperty->GetUnderlyingProperty()->SetIntPropertyValue(Data, EnumValue);
		return true;
	}
	// FByteProperty may also be TEnumAsByte (with Enum)
	if (auto* ByteProperty = CastField<FByteProperty>(&Property))
	{
		if (UEnum* Enum = ByteProperty->GetIntPropertyEnum())
		{
			int64 EnumValue = INDEX_NONE;
			if (!Convert(String, Enum, EnumValue))
			{
				return false;
			}
			ByteProperty->SetIntPropertyValue(Data, static_cast<int64>(EnumValue));
			return true;
		}
	}
	return false;
}
