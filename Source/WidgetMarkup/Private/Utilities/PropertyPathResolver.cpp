// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "PropertyPathResolver.h"

#include "Templates/SharedPointer.h"
#include "UObject/UnrealType.h"
#include "WidgetMarkupModule.h"

TSharedPtr<FPropertyPathResolver::FOutput> FPropertyPathResolver::TryResolvePath(const FInitialState& InitialState, const FWidgetPropertyPath& Path, bool bTypeOnly)
{
	if (Path.IsEmpty() || !InitialState.Struct)
	{
		return nullptr;
	}
	// Type-only mode allows a null container (compile-time resolution without an
	// instance); the full mode requires a container to compute value addresses.
	if (!bTypeOnly && !InitialState.Container)
	{
		return nullptr;
	}

	const TArray<FWidgetPropertyPathElement>& Elements = Path.GetElements();
	if (Elements.IsEmpty())
	{
		return nullptr;
	}

	FProperty* CurrentProperty = InitialState.Property;
	void* CurrentContainer = InitialState.Container;
	UStruct* CurrentStruct = InitialState.Struct;

	for (int32 ElementIndex = 0; ElementIndex < Elements.Num(); ++ElementIndex)
	{
		const FWidgetPropertyPathElement& Element = Elements[ElementIndex];

		if (Element.bIsAny)
		{
			return nullptr;
		}

		void* ValueAddress = nullptr;
		if (Element.Type == EWidgetPropertyPathElementType::Property)
		{
			const FName PropertyFName(*Element.Name);
			CurrentProperty = CurrentStruct ? CurrentStruct->FindPropertyByName(PropertyFName) : nullptr;

			// Fallback: TEnumAsByte<> and some deprecated properties may not be found by
			// FindPropertyByName but exist in the class hierarchy. Use TFieldIterator.
			if (!CurrentProperty && CurrentStruct)
			{
				for (TFieldIterator<FProperty> It(CurrentStruct); It; ++It)
				{
					if (It->GetFName() == PropertyFName)
					{
						CurrentProperty = *It;
						break;
					}
				}
			}

			// Fallback: UE bool properties use 'b' prefix (e.g. bIsEnabled), but
			// XML authors write IsEnabled without the prefix. Only accept if
			// the matched property is actually a bool.
			if (!CurrentProperty && CurrentStruct)
			{
				const FName BPrefixedName(*(FString(TEXT("b")) + Element.Name));
				for (TFieldIterator<FProperty> It(CurrentStruct); It; ++It)
				{
					if (It->GetFName() == BPrefixedName)
					{
						if (It->IsA<FBoolProperty>())
						{
							CurrentProperty = *It;
						}
						break;
					}
				}
			}

			if (!CurrentProperty)
			{
				UE_LOG(LogWidgetMarkup, Warning, TEXT("PropertyPathResolver: FindPropertyByName FAILED for '%s' on struct '%s'"),
					*Element.Name, CurrentStruct ? *CurrentStruct->GetName() : TEXT("<null>"));
				return nullptr;
			}
		}
		else if (Element.Type == EWidgetPropertyPathElementType::ArrayIndex)
		{
			FArrayProperty* ArrayProperty = CastField<FArrayProperty>(CurrentProperty);
			if (!ArrayProperty)
			{
				return nullptr;
			}

			if (bTypeOnly && Element.ArrayIndex != INDEX_NONE)
			{
				// Without a container, a specific array element cannot be resolved.
				return nullptr;
			}

			FScriptArrayHelper ArrayHelper(ArrayProperty, CurrentContainer);
			if (Element.ArrayIndex == INDEX_NONE)
			{
				// Wildcard: resolve inner type only, no specific element.
				if (!ArrayProperty->Inner)
				{
					return nullptr;
				}
				CurrentProperty = ArrayProperty->Inner;
				ValueAddress = nullptr;
				CurrentContainer = nullptr;
			}
			else if (!ArrayHelper.IsValidIndex(Element.ArrayIndex))
			{
				return nullptr;
			}
			else
			{
				void* ElementValueAddress = ArrayHelper.GetRawPtr(Element.ArrayIndex);
				FProperty* InnerProperty = ArrayProperty->Inner;
				if (!InnerProperty)
				{
					return nullptr;
				}

				CurrentProperty = InnerProperty;
				ValueAddress = ElementValueAddress;
				CurrentContainer = ElementValueAddress;
			}
		}
		else
		{
			return nullptr;
		}

		if (!ValueAddress && CurrentContainer)
		{
			ValueAddress = CurrentProperty->ContainerPtrToValuePtr<void>(CurrentContainer);
		}

		const bool bIsLastElement = (ElementIndex == Elements.Num() - 1);
		if (bIsLastElement)
		{
			if (!CurrentProperty)
			{
				return nullptr;
			}

			TSharedPtr<FOutput> Output = MakeShared<FOutput>();
			Output->Property = CurrentProperty;
			Output->Container = CurrentContainer;
			Output->ValueAddress = ValueAddress;
			return Output;
		}

		if (FStructProperty* StructProperty = CastField<FStructProperty>(CurrentProperty))
		{
			CurrentContainer = ValueAddress;
			CurrentStruct = StructProperty->Struct;
		}
		else if (FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(CurrentProperty))
		{
			if (bTypeOnly)
			{
				// Without an instance, the concrete object subclass (and therefore
				// any further path segment) cannot be resolved.
				return nullptr;
			}
			UObject* ObjectValue = ObjectProperty->GetObjectPropertyValue(ValueAddress);
			if (!ObjectValue)
			{
				return nullptr;
			}

			CurrentContainer = ObjectValue;
			CurrentStruct = ObjectValue->GetClass();
		}
		else
		{
			CurrentContainer = ValueAddress;
			CurrentStruct = nullptr;
		}
	}

	return nullptr;
}
