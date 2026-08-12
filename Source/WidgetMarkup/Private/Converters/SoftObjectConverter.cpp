// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "SoftObjectConverter.h"

#include "UObject/UnrealType.h"

TSharedRef<FConverter> FSoftObjectConverter::Create()
{
	return MakeShared<FSoftObjectConverter>();
}

bool FSoftObjectConverter::Convert(const FProperty& Property, void* Data, const FStringView& String)
{
	// Unlike the object converter this does NOT load the asset: the soft path is
	// stored verbatim, which is what a SoftObject variable default expects.
	const FSoftObjectProperty* SoftObjectProperty = CastField<FSoftObjectProperty>(&Property);
	if (!SoftObjectProperty)
	{
		return false;
	}

	// Brace-init avoids the most vexing parse (FString(String) would otherwise be
	// read as a function parameter declaration).
	const FSoftObjectPath Path{ FString(String) };
	if (!Path.IsValid())
	{
		return false;
	}

	SoftObjectProperty->SetPropertyValue(Data, FSoftObjectPtr{ Path });
	return true;
}
