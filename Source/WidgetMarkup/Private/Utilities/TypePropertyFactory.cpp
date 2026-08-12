// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "Utilities/TypePropertyFactory.h"

#include "EdGraphSchema_K2.h"
#include "Serialization/Archive.h"
#include "UObject/Class.h"
#include "UObject/UObjectGlobals.h"
#include "UObject/UnrealType.h"

FTypePropertyFactory::FTypePropertyFactory() = default;

FTypePropertyFactory::~FTypePropertyFactory()
{
	Reset();
}

void FTypePropertyFactory::Reset()
{
	// Container properties own their inner fields and release them in their destructor,
	// so only the root property needs to be deleted explicitly.
	if (Property)
	{
		delete Property;
		Property = nullptr;
	}
	if (OwnerStruct)
	{
		OwnerStruct->RemoveFromRoot();
		OwnerStruct = nullptr;
	}
}

FProperty* FTypePropertyFactory::CreateProperty(const FEdGraphPinType& PinType, FString& OutError)
{
	Reset();

	OwnerStruct = NewObject<UScriptStruct>(GetTransientPackage(), NAME_None, RF_Transient);
	OwnerStruct->AddToRoot();

	Property = PinType.IsContainer()
		? CreateContainerProperty(PinType, OutError)
		: CreateScalarProperty(PinType, OutError, FFieldVariant(OwnerStruct));

	if (!Property)
	{
		Reset();
		return nullptr;
	}

	// Synthetic properties skip the Link pass UHT normally performs. Link finalizes
	// per-type layout data that runtime Set/Map helpers require: FSetProperty::SetLayout,
	// FMapProperty::MapLayout and FStructProperty element sizes are all computed there.
	// Without it FScriptSetHelper/FScriptMapHelper operate on all-zero layouts and corrupt
	// memory (hash/alignment values become garbage), and ExportText of containers fails.
	FArchive LinkArchive;
	Property->LinkWithoutChangingOffset(LinkArchive);

	return Property;
}

FProperty* FTypePropertyFactory::CreateScalarProperty(const FEdGraphPinType& PinType, FString& OutError, const FFieldVariant& InOwner)
{
	const FName& Category = PinType.PinCategory;

	FProperty* ScalarProperty = nullptr;

	if (Category == UEdGraphSchema_K2::PC_Boolean) { ScalarProperty = AllocateField<FBoolProperty>(InOwner); }
	else if (Category == UEdGraphSchema_K2::PC_Int) { ScalarProperty = AllocateField<FIntProperty>(InOwner); }
	else if (Category == UEdGraphSchema_K2::PC_Int64) { ScalarProperty = AllocateField<FInt64Property>(InOwner); }
	else if (Category == UEdGraphSchema_K2::PC_Real)
	{
		// Modern float/double: category PC_Real with PC_Float/PC_Double subcategory.
		if (PinType.PinSubCategory == UEdGraphSchema_K2::PC_Float) { ScalarProperty = AllocateField<FFloatProperty>(InOwner); }
		else if (PinType.PinSubCategory == UEdGraphSchema_K2::PC_Double) { ScalarProperty = AllocateField<FDoubleProperty>(InOwner); }
		else
		{
			OutError = FString::Printf(TEXT("Unsupported real-number subcategory '%s'."), *PinType.PinSubCategory.ToString());
			return nullptr;
		}
	}
	else if (Category == UEdGraphSchema_K2::PC_String) { ScalarProperty = AllocateField<FStrProperty>(InOwner); }
	else if (Category == UEdGraphSchema_K2::PC_Text) { ScalarProperty = AllocateField<FTextProperty>(InOwner); }
	else if (Category == UEdGraphSchema_K2::PC_Name) { ScalarProperty = AllocateField<FNameProperty>(InOwner); }
	else if (Category == UEdGraphSchema_K2::PC_Byte)
	{
		// Plain byte (no subcategory object) or TEnumAsByte-style enum byte
		// (PinSubCategoryObject is the UEnum). Both map to FByteProperty, matching
		// the converter registry's NAME_EnumProperty dispatch for enum bytes.
		FByteProperty* ByteProperty = AllocateField<FByteProperty>(InOwner);
		ByteProperty->Enum = Cast<UEnum>(PinType.PinSubCategoryObject.Get());
		ScalarProperty = ByteProperty;
	}
	else if (Category == UEdGraphSchema_K2::PC_Struct)
	{
		UScriptStruct* Struct = Cast<UScriptStruct>(PinType.PinSubCategoryObject.Get());
		if (!Struct)
		{
			OutError = TEXT("Struct variable type has no UScriptStruct object.");
			return nullptr;
		}
		FStructProperty* StructProperty = AllocateField<FStructProperty>(InOwner);
		StructProperty->Struct = Struct;
		StructProperty->ElementSize = Struct->GetStructureSize();
		ScalarProperty = StructProperty;
	}
	else if (Category == UEdGraphSchema_K2::PC_Object)
	{
		UClass* ObjectClass = Cast<UClass>(PinType.PinSubCategoryObject.Get());
		if (!ObjectClass)
		{
			OutError = TEXT("Object variable type has no UClass object.");
			return nullptr;
		}
		FObjectProperty* ObjectProperty = AllocateField<FObjectProperty>(InOwner);
		ObjectProperty->PropertyClass = ObjectClass;
		ScalarProperty = ObjectProperty;
	}
	else if (Category == UEdGraphSchema_K2::PC_Class)
	{
		FClassProperty* ClassProperty = AllocateField<FClassProperty>(InOwner);
		ClassProperty->PropertyClass = UClass::StaticClass();
		ClassProperty->MetaClass = Cast<UClass>(PinType.PinSubCategoryObject.Get());
		ScalarProperty = ClassProperty;
	}
	else if (Category == UEdGraphSchema_K2::PC_SoftObject)
	{
		UClass* ObjectClass = Cast<UClass>(PinType.PinSubCategoryObject.Get());
		if (!ObjectClass)
		{
			OutError = TEXT("SoftObject variable type has no UClass object.");
			return nullptr;
		}
		FSoftObjectProperty* SoftObjectProperty = AllocateField<FSoftObjectProperty>(InOwner);
		SoftObjectProperty->PropertyClass = ObjectClass;
		ScalarProperty = SoftObjectProperty;
	}
	else if (Category == UEdGraphSchema_K2::PC_SoftClass)
	{
		FSoftClassProperty* SoftClassProperty = AllocateField<FSoftClassProperty>(InOwner);
		SoftClassProperty->PropertyClass = UClass::StaticClass();
		SoftClassProperty->MetaClass = Cast<UClass>(PinType.PinSubCategoryObject.Get());
		ScalarProperty = SoftClassProperty;
	}
	else
	{
		OutError = FString::Printf(TEXT("Unsupported variable pin category '%s'."), *Category.ToString());
		return nullptr;
	}

	// Synthetic properties lack the flags UHT normally assigns. Set the hash flag so
	// Set/Map helpers (FScriptSetHelper/FScriptMapHelper) can hash elements/keys.
	SetHashableFlags(ScalarProperty);
	return ScalarProperty;
}

void FTypePropertyFactory::SetHashableFlags(FProperty* InProperty)
{
	if (const FStructProperty* StructProperty = CastField<FStructProperty>(InProperty))
	{
		// Structs are hashable only when their native ops declare GetTypeHash.
		if (const UScriptStruct* Struct = StructProperty->Struct)
		{
			if (const UScriptStruct::ICppStructOps* StructOps = Struct->GetCppStructOps())
			{
				if (StructOps->HasGetTypeHash())
				{
					InProperty->SetPropertyFlags(CPF_HasGetValueTypeHash);
				}
			}
		}
		return;
	}

	// All other scalar types the factory creates (numeric, bool, string, text, name,
	// enum, object, class, soft object/class) are hashable.
	InProperty->SetPropertyFlags(CPF_HasGetValueTypeHash);
}

FProperty* FTypePropertyFactory::CreateContainerProperty(const FEdGraphPinType& PinType, FString& OutError)
{
	FEdGraphPinType InnerPinType = PinType;
	InnerPinType.ContainerType = EPinContainerType::None;

	if (PinType.ContainerType == EPinContainerType::Array)
	{
		FArrayProperty* ArrayProperty = AllocateField<FArrayProperty>(OwnerStruct);
		ArrayProperty->Inner = CreateInnerProperty(ArrayProperty, InnerPinType, OutError);
		if (!ArrayProperty->Inner)
		{
			delete ArrayProperty;
			return nullptr;
		}
		return ArrayProperty;
	}

	if (PinType.ContainerType == EPinContainerType::Set)
	{
		FSetProperty* SetProperty = AllocateField<FSetProperty>(OwnerStruct);
		SetProperty->ElementProp = CreateInnerProperty(SetProperty, InnerPinType, OutError);
		if (!SetProperty->ElementProp)
		{
			delete SetProperty;
			return nullptr;
		}
		return SetProperty;
	}

	if (PinType.ContainerType == EPinContainerType::Map)
	{
		FMapProperty* MapProperty = AllocateField<FMapProperty>(OwnerStruct);

		FEdGraphPinType ValuePinType = InnerPinType;
		ValuePinType.PinCategory = PinType.PinValueType.TerminalCategory;
		ValuePinType.PinSubCategory = PinType.PinValueType.TerminalSubCategory;
		ValuePinType.PinSubCategoryObject = PinType.PinValueType.TerminalSubCategoryObject;

		MapProperty->KeyProp = CreateInnerProperty(MapProperty, InnerPinType, OutError);
		MapProperty->ValueProp = CreateInnerProperty(MapProperty, ValuePinType, OutError);
		if (!MapProperty->KeyProp || !MapProperty->ValueProp)
		{
			delete MapProperty;
			return nullptr;
		}
		return MapProperty;
	}

	OutError = TEXT("Unsupported container type.");
	return nullptr;
}

FProperty* FTypePropertyFactory::CreateInnerProperty(FProperty* InContainer, const FEdGraphPinType& PinType, FString& OutError)
{
	// Inner fields are owned by the container so its destructor releases them.
	return CreateScalarProperty(PinType, OutError, FFieldVariant(InContainer));
}
