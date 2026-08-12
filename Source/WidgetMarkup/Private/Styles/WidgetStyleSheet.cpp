// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "Styles/WidgetStyleSheet.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "ElementNodes/PropertyChainHandle.h"
#include "Extensions/WidgetMarkupBlueprintGeneratedClassExtension.h"
#include "Utilities/TypeParser.h"

void UWidgetStyleSheet::AddReferencedObjects(UObject* InThis, FReferenceCollector& Collector)
{
	UWidgetStyleSheet* This = CastChecked<UWidgetStyleSheet>(InThis);
	Super::AddReferencedObjects(InThis, Collector);

	const auto CollectSetterBuffers = [&Collector](TArray<FWidgetStyleEntry>& Entries)
	{
		for (FWidgetStyleEntry& Entry : Entries)
		{
			for (FWidgetStyleSetter& Setter : Entry.Setters)
			{
				// FPropertyBuffer holds a raw allocation whose object references are not visible to the GC otherwise.
				Setter.Buffer.AddStructReferencedObjects(Collector);
			}
		}
	};

	CollectSetterBuffers(This->Styles);
	CollectSetterBuffers(This->ComputedStyles);
}

bool FWidgetStyleSetter::ApplyToWidget(UWidget* Widget) const
{
	if (!Widget || Property.IsEmpty()) return false;
	if (!Buffer.HasValue() && Value.IsEmpty()) return false;

	const TSharedPtr<FPropertyChainHandle> Handle = FPropertyChainHandle::Create(Widget, Property);
	if (!Handle.IsValid()) return false;

	return Buffer.HasValue()
		? Handle->SetValue(Buffer)
		: Handle->SetValue(FStringView(Value));
}

void UWidgetStyleSheet::AddOrReplaceStyleEntry(const FWidgetStyleEntry& Entry)
{
	for (FWidgetStyleEntry& Existing : Styles)
	{
		if (Existing.Name == Entry.Name && Existing.TargetType == Entry.TargetType)
		{
			Existing = Entry;
			return;
		}
	}
	Styles.Add(Entry);
}

void UWidgetStyleSheet::ResolveComputedStyles()
{
	ComputedStyles.Reset();
	if (Inherit)
	{
		Inherit->ResolveComputedStyles();
		ComputedStyles = Inherit->ComputedStyles;
	}
	// Merge local Styles into ComputedStyles at the setter level:
	// - If a matching entry (same Name + TargetType) exists, merge setters:
	//   local setters override matching inherited ones, non-overridden inherited setters are kept.
	// - If no matching entry exists, add the whole entry.
	for (const FWidgetStyleEntry& Entry : Styles)
	{
		FWidgetStyleEntry* Existing = ComputedStyles.FindByPredicate(
			[&Entry](const FWidgetStyleEntry& E)
			{
				return E.Name == Entry.Name && E.TargetType == Entry.TargetType;
			});

		if (Existing)
		{
			// Setter-level merge: local setters override inherited ones by Property path.
			for (const FWidgetStyleSetter& Setter : Entry.Setters)
			{
				const int32 ExistingIndex = Existing->Setters.IndexOfByPredicate(
					[&Setter](const FWidgetStyleSetter& S) { return S.Property == Setter.Property; });
				if (ExistingIndex != INDEX_NONE)
				{
					Existing->Setters[ExistingIndex] = Setter;
				}
				else
				{
					Existing->Setters.Add(Setter);
				}
			}
		}
		else
		{
			ComputedStyles.Add(Entry);
		}
	}
}

void UWidgetStyleSheet::ApplyToUserWidget(UUserWidget* UserWidget) const
{
	if (!UserWidget) return;

	UWidgetBlueprintGeneratedClass* WidgetClass = Cast<UWidgetBlueprintGeneratedClass>(UserWidget->GetClass());
	UWidgetMarkupBlueprintGeneratedClassExtension* ClassExtension = WidgetClass
		? WidgetClass->GetExtension<UWidgetMarkupBlueprintGeneratedClassExtension>() : nullptr;
	const TMap<FName, FName>& Assignments = ClassExtension
		? ClassExtension->GetWidgetStyleAssignments() : TMap<FName, FName>();

	const TArray<FWidgetStyleEntry>& EffectiveStyles = ComputedStyles.Num() > 0 ? ComputedStyles : Styles;

	TArray<UWidget*> AllWidgets;
	UserWidget->WidgetTree->GetAllWidgets(AllWidgets);

	for (const FWidgetStyleEntry& Entry : EffectiveStyles)
	{
		if (Entry.TargetType.IsNone()) continue;
		const bool bIsImplicit = Entry.Name.IsNone();

		// Resolve TargetType once per entry via FTypeParser to avoid
		// TryFindTypeSlow short-name warnings for built-in UMG types.
		UClass* TargetClass = FTypeParser::ResolveClass(Entry.TargetType.ToString());
		if (!TargetClass) continue;

		for (UWidget* WidgetNode : AllWidgets)
		{
			if (!WidgetNode) continue;
			if (!WidgetNode->IsA(TargetClass)) continue;
			if (!bIsImplicit)
			{
				const FName* Assigned = Assignments.Find(WidgetNode->GetFName());
				if (!Assigned || *Assigned != Entry.Name) continue;
			}

			for (const FWidgetStyleSetter& Setter : Entry.Setters)
			{
				Setter.ApplyToWidget(WidgetNode);
			}
		}
	}
}
