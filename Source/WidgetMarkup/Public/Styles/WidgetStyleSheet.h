// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PropertyBuffer.h"
#include "Utilities/WidgetPropertyPath.h"

#include "WidgetStyleSheet.generated.h"

class UUserWidget;
class UWidget;

USTRUCT()
struct FWidgetStyleSetter
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, Category = "Style")
	FWidgetPropertyPath Property;

	/** Pre-parsed buffer set via child elements. */
	UPROPERTY(EditAnywhere, Category = "Style")
	FPropertyBuffer Buffer;

	/** String value set via attribute, converted at apply time. */
	UPROPERTY(EditAnywhere, Category = "Style")
	FString Value;

	bool ApplyToWidget(UWidget* Widget) const;
};

USTRUCT()
struct FWidgetStyleEntry
{
	GENERATED_BODY()

	/** Target widget class name. "*" matches all widgets of TargetType (implicit style). */
	UPROPERTY(EditAnywhere, Category = "Style")
	FName TargetType = NAME_None;

	/** Explicit style name (matched via Style="Name" attribute). NAME_None for implicit. */
	UPROPERTY(EditAnywhere, Category = "Style")
	FName Name = NAME_None;

	/**
	 * Optional name of another style with the same TargetType to start from.
	 * This entry's setters are applied on top of that style's setters, so a
	 * variant only has to spell out what it changes (the equivalent of the
	 * engine's FButtonStyle(BaseStyle).SetXxx(...) composition). Chains are
	 * resolved in ResolveComputedStyles(); a missing or cyclic Base is reported
	 * as a warning and otherwise ignored.
	 */
	UPROPERTY(EditAnywhere, Category = "Style")
	FName Base = NAME_None;

	UPROPERTY(EditAnywhere, Category = "Style")
	TArray<FWidgetStyleSetter> Setters;
};

UCLASS(BlueprintType)
class WIDGETMARKUP_API UWidgetStyleSheet : public UDataAsset
{
	GENERATED_BODY()

public:
	/** Base stylesheet this one inherits from (resolved at compile time). */
	UPROPERTY(EditAnywhere, Instanced, Category = "Style")
	TObjectPtr<UWidgetStyleSheet> Inherit;

	UPROPERTY(EditAnywhere, Category = "Style")
	TArray<FWidgetStyleEntry> Styles;

	/** Final merged styles (base + overrides, computed by ResolveComputedStyles). */
	UPROPERTY(EditAnywhere, Transient, Category = "Style")
	TArray<FWidgetStyleEntry> ComputedStyles;

	void AddOrReplaceStyleEntry(const FWidgetStyleEntry& Entry);

	/** Walk Inherit chain and compute ComputedStyles (base Inherits first, then local Styles override). */
	void ResolveComputedStyles();

	/** Apply ComputedStyles to the given UserWidget using the provided style assignments. */
	void ApplyToUserWidget(UUserWidget* UserWidget) const;

	//~Begin UObject interface
	static void AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector);
	//~End UObject interface

private:
	/**
	 * Expand every Base chain in ComputedStyles into flat setter lists
	 * (base-most ancestor first, the entry's own setters last so they win).
	 * Runs after the Inherit merge, which means a Base may also name an entry
	 * that came from the inherited sheet.
	 */
	void ExpandBaseStyles();
};
