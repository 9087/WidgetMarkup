// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "TypeParser.h"

#include "EdGraphSchema_K2.h"
#include "UObject/Interface.h"
#include "UObject/UObjectIterator.h"

template<typename T>
T* FTypeParser::TryResolveType(const FString& Token)
{
	// Cache: each short type name goes through TryFindTypeSlow at most once.
	// Subsequent lookups hit the cache with zero warnings.
	static TMap<FName, TWeakObjectPtr<UObject>> Cache;

	const FName Key(*Token);
	if (const TWeakObjectPtr<UObject>* Cached = Cache.Find(Key))
	{
		if (Cached->IsValid())
		{
			return Cast<T>(Cached->Get());
		}
		Cache.Remove(Key);
	}

	// TryFindTypeSlow — will warn for unknown types, but only once per name.
	if (T* Result = UClass::TryFindTypeSlow<T>(Token, EFindFirstObjectOptions::None))
	{
		Cache.Add(Key, Result);
		return Result;
	}
	return nullptr;
}

// Explicit template instantiation for the three types we resolve.
template UClass* FTypeParser::TryResolveType<UClass>(const FString& Token);
template UScriptStruct* FTypeParser::TryResolveType<UScriptStruct>(const FString& Token);
template UEnum* FTypeParser::TryResolveType<UEnum>(const FString& Token);

namespace
{
	UScriptStruct* ResolveBuiltinStruct(const FString& TypeToken)
	{
		if (TypeToken.Equals(TEXT("Vector"))) return TBaseStructure<FVector>::Get();
		if (TypeToken.Equals(TEXT("Vector2D"))) return TBaseStructure<FVector2D>::Get();
		if (TypeToken.Equals(TEXT("Rotator"))) return TBaseStructure<FRotator>::Get();
		if (TypeToken.Equals(TEXT("Transform"))) return TBaseStructure<FTransform>::Get();
		if (TypeToken.Equals(TEXT("Color"))) return TBaseStructure<FColor>::Get();
		if (TypeToken.Equals(TEXT("LinearColor"))) return TBaseStructure<FLinearColor>::Get();
		return nullptr;
	}

	FEdGraphTerminalType ToTerminalType(const FEdGraphPinType& InPinType)
	{
		return FEdGraphTerminalType::FromPinType(InPinType);
	}
}

FName FTypeParser::ToPinCategory(const FString& TypeToken)
{
	if (TypeToken.Equals(TEXT("Boolean"))) return UEdGraphSchema_K2::PC_Boolean;
	if (TypeToken.Equals(TEXT("Integer"))) return UEdGraphSchema_K2::PC_Int;
	if (TypeToken.Equals(TEXT("Integer64"))) return UEdGraphSchema_K2::PC_Int64;
	if (TypeToken.Equals(TEXT("Float"))) return UEdGraphSchema_K2::PC_Real;
	if (TypeToken.Equals(TEXT("Double"))) return UEdGraphSchema_K2::PC_Real;
	if (TypeToken.Equals(TEXT("Byte"))) return UEdGraphSchema_K2::PC_Byte;
	if (TypeToken.Equals(TEXT("String"))) return UEdGraphSchema_K2::PC_String;
	if (TypeToken.Equals(TEXT("Text"))) return UEdGraphSchema_K2::PC_Text;
	if (TypeToken.Equals(TEXT("Name"))) return UEdGraphSchema_K2::PC_Name;
	if (TypeToken.Equals(TEXT("Vector"))) return UEdGraphSchema_K2::PC_Struct;
	if (TypeToken.Equals(TEXT("Vector2D"))) return UEdGraphSchema_K2::PC_Struct;
	if (TypeToken.Equals(TEXT("Rotator"))) return UEdGraphSchema_K2::PC_Struct;
	if (TypeToken.Equals(TEXT("Transform"))) return UEdGraphSchema_K2::PC_Struct;
	if (TypeToken.Equals(TEXT("Color"))) return UEdGraphSchema_K2::PC_Struct;
	if (TypeToken.Equals(TEXT("LinearColor"))) return UEdGraphSchema_K2::PC_Struct;
	return NAME_None;
}

bool FTypeParser::ParseType(const FString& InTypeText, FEdGraphPinType& OutPinType, FString& OutError)
{
	return ParseTypeInternal(InTypeText, OutPinType, OutError, true);
}

UClass* FTypeParser::ResolveClass(const FString& InClassText)
{
	const FString Token = NormalizeToken(InClassText);
	if (Token.IsEmpty())
	{
		return nullptr;
	}

	if (UClass* Class = TryResolveType<UClass>(Token))
	{
		return Class;
	}

	const FString PrefixedToken = Token.StartsWith(TEXT("U"), ESearchCase::CaseSensitive) ? Token : FString(TEXT("U")) + Token;
	if (UClass* Class = TryResolveType<UClass>(PrefixedToken))
	{
		return Class;
	}

	for (TObjectIterator<UClass> It; It; ++It)
	{
		UClass* Class = *It;
		if (!Class)
		{
			continue;
		}
		const FString ClassName = Class->GetName();
		if (ClassName.Equals(Token, ESearchCase::IgnoreCase))
		{
			return Class;
		}
		if (ClassName.Len() > 1 && ClassName[0] == TCHAR('U') && ClassName.RightChop(1).Equals(Token, ESearchCase::IgnoreCase))
		{
			return Class;
		}
	}

	return nullptr;
}

UClass* FTypeParser::ResolveInterface(const FString& InInterfaceText)
{
	UClass* Class = ResolveClass(InInterfaceText);
	if (!Class || !Class->IsChildOf(UInterface::StaticClass()))
	{
		return nullptr;
	}
	return Class;
}

UScriptStruct* FTypeParser::ResolveStruct(const FString& InStructText)
{
	const FString Token = NormalizeToken(InStructText);
	if (Token.IsEmpty())
	{
		return nullptr;
	}

	if (UScriptStruct* Struct = TryResolveType<UScriptStruct>(Token))
	{
		return Struct;
	}

	const FString PrefixedToken = Token.StartsWith(TEXT("F"), ESearchCase::CaseSensitive) ? Token : FString(TEXT("F")) + Token;
	if (UScriptStruct* Struct = TryResolveType<UScriptStruct>(PrefixedToken))
	{
		return Struct;
	}

	for (TObjectIterator<UScriptStruct> It; It; ++It)
	{
		UScriptStruct* Struct = *It;
		if (!Struct)
		{
			continue;
		}
		const FString StructName = Struct->GetName();
		if (StructName.Equals(Token, ESearchCase::IgnoreCase))
		{
			return Struct;
		}
		if (StructName.Len() > 1 && StructName[0] == TCHAR('F') && StructName.RightChop(1).Equals(Token, ESearchCase::IgnoreCase))
		{
			return Struct;
		}
	}

	return nullptr;
}

UEnum* FTypeParser::ResolveEnum(const FString& InEnumText)
{
	const FString Token = NormalizeToken(InEnumText);
	if (Token.IsEmpty())
	{
		return nullptr;
	}

	if (UEnum* Enum = TryResolveType<UEnum>(Token))
	{
		return Enum;
	}

	const FString PrefixedToken = Token.StartsWith(TEXT("E"), ESearchCase::CaseSensitive) ? Token : FString(TEXT("E")) + Token;
	if (UEnum* Enum = TryResolveType<UEnum>(PrefixedToken))
	{
		return Enum;
	}

	for (TObjectIterator<UEnum> It; It; ++It)
	{
		UEnum* Enum = *It;
		if (!Enum)
		{
			continue;
		}
		const FString EnumName = Enum->GetName();
		if (EnumName.Equals(Token, ESearchCase::IgnoreCase))
		{
			return Enum;
		}
		if (EnumName.Len() > 1 && EnumName[0] == TCHAR('E') && EnumName.RightChop(1).Equals(Token, ESearchCase::IgnoreCase))
		{
			return Enum;
		}
	}

	return nullptr;
}

bool FTypeParser::ParseTypeInternal(const FString& InTypeText, FEdGraphPinType& OutPinType, FString& OutError, bool bAllowContainer)
{
	const FString TypeText = NormalizeToken(InTypeText);
	if (TypeText.IsEmpty())
	{
		OutError = TEXT("Type must not be empty.");
		return false;
	}

	FString ContainerName;
	FString InnerText;
	if (ParseContainer(TypeText, ContainerName, InnerText, OutError))
	{
		if (!bAllowContainer)
		{
			OutError = FString::Printf(TEXT("Container nesting is not supported: '%s'"), *TypeText);
			return false;
		}

		if (ContainerName.Equals(TEXT("Array")) || ContainerName.Equals(TEXT("Set")))
		{
			FEdGraphPinType InnerPinType;
			if (!ParseTypeInternal(InnerText, InnerPinType, OutError, false))
			{
				return false;
			}
			OutPinType = InnerPinType;
			OutPinType.ContainerType = ContainerName.Equals(TEXT("Array"))
				? EPinContainerType::Array
				: EPinContainerType::Set;
			return true;
		}

		if (ContainerName.Equals(TEXT("Map")))
		{
			FString KeyText;
			FString ValueText;
			if (!ParseMapContainer(InnerText, KeyText, ValueText, OutError))
			{
				return false;
			}

			FEdGraphPinType KeyPinType;
			if (!ParseTypeInternal(KeyText, KeyPinType, OutError, false))
			{
				return false;
			}
			if (KeyPinType.IsContainer())
			{
				OutError = FString::Printf(TEXT("Map key type cannot be a container: '%s'"), *KeyText);
				return false;
			}

			FEdGraphPinType ValuePinType;
			if (!ParseTypeInternal(ValueText, ValuePinType, OutError, false))
			{
				return false;
			}
			if (ValuePinType.IsContainer())
			{
				OutError = FString::Printf(TEXT("Map value type cannot be a container: '%s'"), *ValueText);
				return false;
			}

			OutPinType = KeyPinType;
			OutPinType.ContainerType = EPinContainerType::Map;
			OutPinType.PinValueType = ToTerminalType(ValuePinType);
			return true;
		}

		OutError = FString::Printf(TEXT("Unsupported container type '%s' in '%s'."), *ContainerName, *TypeText);
		return false;
	}
	else if (!OutError.IsEmpty())
	{
		return false;
	}

	OutError.Empty();

	// Parse type-qualifier prefixes: Object(Texture2D), Class(...), SoftObject(...), etc.
	auto TryParsePrefixed = [&](const TCHAR* PrefixName, int32 PrefixLen, const FName& PinCategory) -> bool
	{
		if (!TypeText.StartsWith(PrefixName) || TypeText[PrefixLen] != TCHAR('(') || !TypeText.EndsWith(TEXT(")")))
		{
			return false;
		}
		const FString ValueText = NormalizeToken(TypeText.Mid(PrefixLen + 1, TypeText.Len() - PrefixLen - 2));

		if (ValueText.IsEmpty())
		{
			OutError = FString::Printf(TEXT("Empty type name after '%s' prefix."), PrefixName);
			return true;
		}

		OutPinType.PinCategory = PinCategory;
		if (UClass* Class = ResolveClass(ValueText))
		{
			OutPinType.PinSubCategoryObject = Class;
			return true;
		}
		OutError = FString::Printf(TEXT("Failed to resolve class type '%s'."), *ValueText);
		return true;
	};

	if (TryParsePrefixed(TEXT("Object"), 6, UEdGraphSchema_K2::PC_Object)) { return !OutError.IsEmpty() ? false : true; }
	if (TryParsePrefixed(TEXT("Class"), 5, UEdGraphSchema_K2::PC_Class)) { return !OutError.IsEmpty() ? false : true; }
	if (TryParsePrefixed(TEXT("SoftObject"), 10, UEdGraphSchema_K2::PC_SoftObject)) { return !OutError.IsEmpty() ? false : true; }
	if (TryParsePrefixed(TEXT("SoftClass"), 9, UEdGraphSchema_K2::PC_SoftClass)) { return !OutError.IsEmpty() ? false : true; }

	if (const FName PinCategory = ToPinCategory(TypeText); PinCategory != NAME_None)
	{
		OutPinType.PinCategory = PinCategory;
		if (PinCategory == UEdGraphSchema_K2::PC_Struct)
		{
			if (UScriptStruct* BuiltinStruct = ResolveBuiltinStruct(TypeText))
			{
				OutPinType.PinSubCategoryObject = BuiltinStruct;
				return true;
			}
		}
		else if (PinCategory == UEdGraphSchema_K2::PC_Real)
		{
			// Modern float/double use category PC_Real with a subcategory
			// (PC_Float / PC_Double) that selects the concrete property type.
			// The Kismet compiler's CreatePrimitiveProperty only understands
			// this modern form; the legacy bare PC_Float/PC_Double categories
			// fall through to an FIntProperty fallback.
			OutPinType.PinSubCategory = TypeText.Equals(TEXT("Double"))
				? UEdGraphSchema_K2::PC_Double
				: UEdGraphSchema_K2::PC_Float;
		}
		return true;
	}

	if (UScriptStruct* Struct = ResolveStruct(TypeText))
	{
		OutPinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
		OutPinType.PinSubCategoryObject = Struct;
		return true;
	}

	if (UEnum* Enum = ResolveEnum(TypeText))
	{
		// Enums use category PC_Byte with the UEnum as the subcategory object —
		// the same representation the Kismet compiler expects (the legacy
		// PC_Enum category is not handled by CreatePrimitiveProperty).
		OutPinType.PinCategory = UEdGraphSchema_K2::PC_Byte;
		OutPinType.PinSubCategoryObject = Enum;
		return true;
	}

	// Auto-detect UClass: if nothing above matched, try resolving as a class.
	// This allows Type="Texture2D" without the Object: prefix.
	if (UClass* Class = ResolveClass(TypeText))
	{
		OutPinType.PinCategory = UEdGraphSchema_K2::PC_Object;
		OutPinType.PinSubCategoryObject = Class;
		return true;
	}

	OutError = FString::Printf(TEXT("Unsupported variable Type '%s'."), *TypeText);
	return false;
}

bool FTypeParser::ParseContainer(const FString& InTypeText, FString& OutContainerName, FString& OutInnerText, FString& OutError)
{
	OutContainerName.Empty();
	OutInnerText.Empty();
	OutError.Empty();

	// Container syntax: Array(InnerType), Map(KeyType,ValueType), Set(InnerType).
	const int32 OpenIndex = InTypeText.Find(TEXT("("));
	if (OpenIndex == INDEX_NONE)
	{
		return false; // Not a container — no error.
	}
	if (!InTypeText.EndsWith(TEXT(")")))
	{
		OutError = FString::Printf(TEXT("Invalid container type syntax '%s': missing closing ')'."), *InTypeText);
		return false;
	}

	OutContainerName = NormalizeToken(InTypeText.Left(OpenIndex));
	OutInnerText = NormalizeToken(InTypeText.Mid(OpenIndex + 1, InTypeText.Len() - OpenIndex - 2));
	if (OutContainerName.IsEmpty() || OutInnerText.IsEmpty())
	{
		OutError = FString::Printf(TEXT("Invalid container type syntax '%s': empty container name or inner type."), *InTypeText);
		return false;
	}
	return true;
}

bool FTypeParser::ParseMapContainer(const FString& InInnerText, FString& OutKeyText, FString& OutValueText, FString& OutError)
{
	OutKeyText.Empty();
	OutValueText.Empty();

	const int32 CommaIndex = InInnerText.Find(TEXT(","));
	if (CommaIndex == INDEX_NONE)
	{
		OutError = FString::Printf(TEXT("Map type requires two type parameters: '%s'."), *InInnerText);
		return false;
	}

	if (InInnerText.Find(TEXT(","), ESearchCase::CaseSensitive, ESearchDir::FromStart, CommaIndex + 1) != INDEX_NONE)
	{
		OutError = FString::Printf(TEXT("Map type must have exactly two type parameters: '%s'."), *InInnerText);
		return false;
	}

	OutKeyText = NormalizeToken(InInnerText.Left(CommaIndex));
	OutValueText = NormalizeToken(InInnerText.Mid(CommaIndex + 1));
	if (OutKeyText.IsEmpty() || OutValueText.IsEmpty())
	{
		OutError = FString::Printf(TEXT("Map key/value type must not be empty: '%s'."), *InInnerText);
		return false;
	}
	return true;
}

FString FTypeParser::NormalizeToken(const FString& InText)
{
	return InText.TrimStartAndEnd();
}
