// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "Styles/WidgetStyleSheet.h"

#include "Blueprint/UserWidget.h"
#include "Blueprint/WidgetBlueprintGeneratedClass.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Widget.h"
#include "ElementNodes/PropertyChainHandle.h"
#include "Extensions/WidgetMarkupBlueprintGeneratedClassExtension.h"
#include "Utilities/TypeParser.h"
#include "WidgetMarkupModule.h"

namespace
{
	/** Add Setter, or replace the existing one that targets the same property path. */
	void AddOrReplaceSetter(TArray<FWidgetStyleSetter>& InOutSetters, const FWidgetStyleSetter& Setter)
	{
		const int32 ExistingIndex = InOutSetters.IndexOfByPredicate(
			[&Setter](const FWidgetStyleSetter& Candidate) { return Candidate.Property == Setter.Property; });
		if (ExistingIndex != INDEX_NONE)
		{
			InOutSetters[ExistingIndex] = Setter;
		}
		else
		{
			InOutSetters.Add(Setter);
		}
	}
}

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
				AddOrReplaceSetter(Existing->Setters, Setter);
			}
			// A local "Base" replaces whatever the inherited entry carried.
			if (!Entry.Base.IsNone())
			{
				Existing->Base = Entry.Base;
			}
		}
		else
		{
			ComputedStyles.Add(Entry);
		}
	}

	ExpandBaseStyles();
}

void UWidgetStyleSheet::ExpandBaseStyles()
{
	const int32 NumEntries = ComputedStyles.Num();
	if (NumEntries == 0)
	{
		return;
	}

	// Compute everything up front, then apply: the chain walk reads other
	// entries' raw setters/Base, so mutating in place would make the result
	// depend on iteration order.
	TArray<TArray<FWidgetStyleSetter>> ExpandedSetters;
	TArray<bool> bResolved;
	ExpandedSetters.SetNum(NumEntries);
	bResolved.Init(false, NumEntries);

	for (int32 Index = 0; Index < NumEntries; ++Index)
	{
		const FWidgetStyleEntry& Entry = ComputedStyles[Index];
		if (Entry.Base.IsNone())
		{
			continue;
		}

		// Walk the chain collecting ancestors nearest-first, then append the entry
		// itself. Names are unique per TargetType, so the visited set also
		// guarantees termination. With the entry last, walking the array backwards
		// lays the base-most style down first and lets every layer override the one
		// below it.
		TArray<const FWidgetStyleEntry*> Chain;
		TSet<FName> Visited;
		Visited.Add(Entry.Name);

		FName NextBase = Entry.Base;
		while (!NextBase.IsNone())
		{
			if (Visited.Contains(NextBase))
			{
				UE_LOG(LogWidgetMarkup, Warning,
					TEXT("StyleSheet: Base cycle on style '%s' (TargetType=%s) at Base=\"%s\"; the rest of the chain is ignored."),
					*Entry.Name.ToString(), *Entry.TargetType.ToString(), *NextBase.ToString());
				break;
			}

			const FWidgetStyleEntry* Found = ComputedStyles.FindByPredicate(
				[&NextBase, &Entry](const FWidgetStyleEntry& Candidate)
				{
					return Candidate.Name == NextBase && Candidate.TargetType == Entry.TargetType;
				});

			if (!Found)
			{
				UE_LOG(LogWidgetMarkup, Warning,
					TEXT("StyleSheet: style '%s' (TargetType=%s) has Base=\"%s\" but no such style exists; Base ignored."),
					*Entry.Name.ToString(), *Entry.TargetType.ToString(), *NextBase.ToString());
				break;
			}

			Chain.Add(Found);
			Visited.Add(NextBase);
			NextBase = Found->Base;
		}
		Chain.Add(&Entry);

		TArray<FWidgetStyleSetter> Merged;
		for (int32 LayerIndex = Chain.Num() - 1; LayerIndex >= 0; --LayerIndex)
		{
			for (const FWidgetStyleSetter& Setter : Chain[LayerIndex]->Setters)
			{
				AddOrReplaceSetter(Merged, Setter);
			}
		}

		ExpandedSetters[Index] = MoveTemp(Merged);
		bResolved[Index] = true;
	}

	for (int32 Index = 0; Index < NumEntries; ++Index)
	{
		if (!bResolved[Index])
		{
			continue;
		}
		ComputedStyles[Index].Setters = MoveTemp(ExpandedSetters[Index]);
		// Base is now baked into the setter list; clearing it keeps repeated
		// ResolveComputedStyles() calls (and derived sheets) idempotent.
		ComputedStyles[Index].Base = NAME_None;
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
	if (ComputedStyles.Num() == 0 && !Styles.IsEmpty())
	{
		// Applying the raw entries: ResolveComputedStyles() was never called, so
		// Inherit merges and Base chains have not been expanded.
		UE_LOG(LogWidgetMarkup, Warning,
			TEXT("StyleSheet: applied without ResolveComputedStyles(); Inherit/Base composition is skipped for '%s'."),
			*GetName());
	}

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
