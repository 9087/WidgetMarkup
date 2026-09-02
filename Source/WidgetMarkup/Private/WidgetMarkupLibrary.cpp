// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "WidgetMarkupLibrary.h"

#include "Components/PanelWidget.h"
#include "Components/Widget.h"
#include "ElementNodeFactory.h"
#include "Modules/ModuleManager.h"
#include "UObject/UObjectIterator.h"
#include "WidgetMarkupModule.h"

namespace
{
	// -----------------------------------------------------------------------
	// Element registry: all names usable in markup source, built lazily from
	// reflection plus the fixed markup struct elements.
	// -----------------------------------------------------------------------
	struct FElementRegistry
	{
		TArray<FWidgetMarkupElementInfo> Elements;
		TMap<FString, UStruct*> Lookup;
		bool bBuilt = false;

		void Build();
	};

	FElementRegistry& GetElementRegistry()
	{
		static FElementRegistry Registry;
		return Registry;
	}

	void FElementRegistry::Build()
	{
		if (bBuilt)
		{
			return;
		}

		// UMG classes are lazy-loaded; load the module so the element index
		// contains them instead of only classes already in memory.
		FModuleManager::Get().LoadModule(TEXT("UMG"));

		Elements.Reset();
		Lookup.Reset();

		auto AddElement = [this](const FString& Name, const FString& Kind, const FString& Type, UStruct* Struct)
		{
			FWidgetMarkupElementInfo Info;
			Info.Name = Name;
			Info.Kind = Kind;
			Info.Type = Type;
			Elements.Add(Info);
			Lookup.Add(Name, Struct);
		};

		// Native non-abstract widget classes are the primary elements.
		for (TObjectIterator<UClass> It; It; ++It)
		{
			UClass* Class = *It;
			if (Class->HasAnyClassFlags(CLASS_Transient | CLASS_Deprecated | CLASS_Abstract))
			{
				continue;
			}
			// Native classes live in PKG_InMemoryOnly /Script/* packages;
			// this skips blueprint-generated classes from asset packages.
			if (!Class->GetOutermost()->HasAnyPackageFlags(PKG_InMemoryOnly))
			{
				continue;
			}
			if (!Class->IsChildOf<UWidget>())
			{
				continue;
			}
			AddElement(Class->GetName(), TEXT("class"), Class->GetName(), Class);
		}

		// Markup elements registered with the element node factory (root
		// elements and struct elements, e.g. Blueprint, Variable, StyleSheet).
		TArray<TPair<FString, UStruct*>> RegisteredElements;
		FElementNodeFactory::Get().GetRegisteredElements(RegisteredElements);
		for (const auto& Pair : RegisteredElements)
		{
			if (UStruct* Struct = Pair.Value)
			{
				const FString Kind = Struct->IsA<UClass>() ? TEXT("class") : TEXT("struct");
				AddElement(Pair.Key, Kind, Struct->GetName(), Struct);
			}
		}

		Elements.Sort([](const FWidgetMarkupElementInfo& A, const FWidgetMarkupElementInfo& B)
		{
			return A.Name < B.Name;
		});

		bBuilt = true;
	}

	/** Exposes UPanelWidget::GetSlotClass() (protected) for CDO-based reflection probing. */
	class FWidgetMarkupPanelSlotProbe : public UPanelWidget
	{
	public:
		using UPanelWidget::GetSlotClass;
	};

	/** Renders a delegate signature as "Delegate<ReturnType(ArgumentTypes)>". */
	FString DescribeDelegateSignature(const UFunction* SignatureFunction);

	FString ResolvePropertyTypeName(FProperty* Property)
	{
		if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property))
		{
			return ObjectProperty->PropertyClass ? ObjectProperty->PropertyClass->GetName() : TEXT("Object");
		}
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			return StructProperty->Struct->GetName();
		}
		if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			return FString::Printf(TEXT("Array<%s>"), *ResolvePropertyTypeName(ArrayProperty->Inner));
		}
		if (const FSetProperty* SetProperty = CastField<FSetProperty>(Property))
		{
			return FString::Printf(TEXT("Set<%s>"), *ResolvePropertyTypeName(SetProperty->ElementProp));
		}
		if (const FMapProperty* MapProperty = CastField<FMapProperty>(Property))
		{
			return FString::Printf(TEXT("Map<%s,%s>"), *ResolvePropertyTypeName(MapProperty->KeyProp), *ResolvePropertyTypeName(MapProperty->ValueProp));
		}
		if (const FMulticastDelegateProperty* MulticastDelegateProperty = CastField<FMulticastDelegateProperty>(Property))
		{
			return DescribeDelegateSignature(MulticastDelegateProperty->SignatureFunction);
		}
		if (const FDelegateProperty* DelegateProperty = CastField<FDelegateProperty>(Property))
		{
			return DescribeDelegateSignature(DelegateProperty->SignatureFunction);
		}
		return Property->GetCPPType();
	}

	/** Renders a delegate signature as "Delegate<ReturnType(ArgumentTypes)>". */
	FString DescribeDelegateSignature(const UFunction* SignatureFunction)
	{
		if (!SignatureFunction)
		{
			return TEXT("Delegate");
		}

		FString ReturnType = TEXT("void");
		TArray<FString> ParameterTypes;
		for (TFieldIterator<FProperty> It(SignatureFunction); It; ++It)
		{
			FProperty* ParamProperty = *It;
			if (ParamProperty->HasAnyPropertyFlags(CPF_ReturnParm))
			{
				ReturnType = ResolvePropertyTypeName(ParamProperty);
			}
			else if (ParamProperty->HasAnyPropertyFlags(CPF_Parm))
			{
				ParameterTypes.Add(ResolvePropertyTypeName(ParamProperty));
			}
		}
		return FString::Printf(TEXT("Delegate<%s(%s)>"), *ReturnType, *FString::Join(ParameterTypes, TEXT(", ")));
	}

	FProperty* FindPropertyByName(UStruct* Struct, const FString& Name)
	{
		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			if ((*It)->GetName() == Name)
			{
				return *It;
			}
		}
		return nullptr;
	}

	/** Returns the struct to drill into for a property, or null for leaf properties. */
	UStruct* GetStructFromProperty(FProperty* Property)
	{
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			return StructProperty->Struct;
		}
		if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			return GetStructFromProperty(ArrayProperty->Inner);
		}
		if (const FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property))
		{
			return ObjectProperty->PropertyClass;
		}
		return nullptr;
	}

	UClass* GetSlotClass(UStruct* Struct)
	{
		UClass* Class = Cast<UClass>(Struct);
		if (!Class || !Class->IsChildOf<UPanelWidget>() || Class->HasAnyClassFlags(CLASS_Abstract))
		{
			return nullptr;
		}
		const UPanelWidget* PanelCDO = Class->GetDefaultObject<UPanelWidget>();
		return static_cast<const FWidgetMarkupPanelSlotProbe*>(PanelCDO)->GetSlotClass();
	}

	void AppendAttributesForStruct(UStruct* Struct, TArray<FWidgetMarkupAttributeInfo>& OutAttributes)
	{
		TSet<FString> SeenNames;

		for (TFieldIterator<FProperty> It(Struct); It; ++It)
		{
			FProperty* Property = *It;
			if (Property->HasAnyPropertyFlags(CPF_Transient | CPF_Deprecated))
			{
				continue;
			}

			FWidgetMarkupAttributeInfo Info;
			Info.Name = Property->GetName();
			Info.Type = ResolvePropertyTypeName(Property);
			if (SeenNames.Contains(Info.Name))
			{
				continue;
			}
			SeenNames.Add(Info.Name);
			OutAttributes.Add(MoveTemp(Info));
		}

		// Synthetic attributes registered with the module as custom property
		// runs (e.g. Script, Super, ListItems, VariableDefault).
		for (const FName& CustomName : FWidgetMarkupModule::Get().GetCustomPropertyRunNames(Struct))
		{
			FWidgetMarkupAttributeInfo Info;
			Info.Name = CustomName.ToString();
			Info.Type = TEXT("FString");
			if (SeenNames.Contains(Info.Name))
			{
				continue;
			}
			SeenNames.Add(Info.Name);
			OutAttributes.Add(MoveTemp(Info));
		}

		// Panel and content widgets accept a synthetic Slot attribute that
		// drills into their child slot class (e.g. CanvasPanelSlot,
		// ButtonSlot). It takes precedence over the widget's own "Slot"
		// property (the object pointing at its parent slot).
		if (const UClass* SlotClass = GetSlotClass(Struct))
		{
			FWidgetMarkupAttributeInfo Info;
			Info.Name = TEXT("Slot");
			Info.Type = SlotClass->GetName();

			if (FWidgetMarkupAttributeInfo* ExistingSlot = OutAttributes.FindByPredicate(
				[](const FWidgetMarkupAttributeInfo& Entry) { return Entry.Name == TEXT("Slot"); }))
			{
				*ExistingSlot = MoveTemp(Info);
			}
			else
			{
				SeenNames.Add(Info.Name);
				OutAttributes.Add(MoveTemp(Info));
			}
		}
	}
}

// ---------------------------------------------------------------------------
// UWidgetMarkupLibrary
// ---------------------------------------------------------------------------

FWidgetMarkupElementInfoList UWidgetMarkupLibrary::GetElements()
{
	FElementRegistry& Registry = GetElementRegistry();
	Registry.Build();

	FWidgetMarkupElementInfoList Result;
	Result.Elements = Registry.Elements;
	return Result;
}

FWidgetMarkupAttributeInfoList UWidgetMarkupLibrary::GetAttributes(const FString& Element, const FString& Prefix)
{
	FWidgetMarkupAttributeInfoList Result;

	FElementRegistry& Registry = GetElementRegistry();
	Registry.Build();

	UStruct* Current = Registry.Lookup.FindRef(Element);
	if (!Current)
	{
		Result.Error = TEXT("UNKNOWN_ELEMENT");
		return Result;
	}

	TArray<FString> Segments;
	Prefix.ParseIntoArray(Segments, TEXT("."), /*InCullEmpty=*/ true);

	for (int32 Index = 0; Index < Segments.Num(); ++Index)
	{
		const FString& Segment = Segments[Index];
		const bool bLastSegment = Index == Segments.Num() - 1;

		UStruct* Child = nullptr;
		bool bMatched = false;

		// The synthetic Slot attribute takes precedence over the widget's own
		// "Slot" property for panel and content widgets.
		if (Segment == TEXT("Slot"))
		{
			if (UClass* SlotClass = GetSlotClass(Current))
			{
				bMatched = true;
				Child = SlotClass;
			}
		}

		if (!bMatched)
		{
			if (FProperty* Property = FindPropertyByName(Current, Segment))
			{
				bMatched = true;
				Child = GetStructFromProperty(Property);
			}
		}

		if (!bMatched)
		{
			for (const FName& CustomName : FWidgetMarkupModule::Get().GetCustomPropertyRunNames(Current))
			{
				if (CustomName.ToString() == Segment)
				{
					// Custom runs are leaf attributes.
					bMatched = true;
					Child = nullptr;
					break;
				}
			}
		}

		if (!bMatched)
		{
			Result.Error = TEXT("UNKNOWN_PATH");
			return Result;
		}

		if (!Child)
		{
			// Leaf attribute: valid only as the last segment; nothing to
			// drill into, so the attribute list stays empty.
			if (!bLastSegment)
			{
				Result.Error = TEXT("UNKNOWN_PATH");
			}
			return Result;
		}

		Current = Child;
	}

	AppendAttributesForStruct(Current, Result.Attributes);
	return Result;
}
