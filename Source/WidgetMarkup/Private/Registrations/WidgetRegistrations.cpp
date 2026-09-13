// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#include "Registrations/WidgetRegistrations.h"

#include "Blueprint/WidgetTree.h"
#include "Components/ComboBox.h"
#include "Components/ComboBoxKey.h"
#include "Components/ComboBoxString.h"
#include "Components/ContentWidget.h"
#include "Components/ListView.h"
#include "Components/PanelWidget.h"
#include "Components/RichTextBlock.h"
#include "Components/RichTextBlockDecorator.h"
#include "Components/Widget.h"
#include "ConverterRegistry.h"
#include "Converters/SlateChildSizeConverter.h"
#include "Converters/SlateColorConverter.h"
#include "Converters/VectorConverter.h"
#include "ElementNodeFactory.h"
#include "ElementNodes/ContentWidgetElementNode.h"
#include "ElementNodes/PanelWidgetElementNode.h"
#include "ElementNodes/WidgetBlueprintElementNode.h"
#include "ElementNodes/WidgetElementNode.h"
#include "ElementNodes/WidgetTreeElementNode.h"
#include "PropertyRuns/DeferredPropertyRun.h"
#include "PropertyRuns/WidgetBlueprintScriptPropertyRun.h"
#include "PropertyRuns/WidgetStylePropertyRun.h"
#include "Types/SlateVector2.h"
#include "UObject/UnrealType.h"
#include "WidgetBlueprint.h"
#include "WidgetMarkupModule.h"
#include "Widgets/Input/SComboBox.h"

void RegisterWidgetRegistrations(FWidgetMarkupModule& Module)
{
	// --- Element nodes for UMG classes -------------------------------------------------
	FElementNodeFactory::Get().Register<UWidgetTree>(FElementNodeFactory::FOnCreateElementNode::CreateStatic(FWidgetTreeElementNode::Create));
	FElementNodeFactory::Get().Register<UWidget>(FElementNodeFactory::FOnCreateElementNode::CreateStatic(FWidgetElementNode::Create));
	FElementNodeFactory::Get().Register<UPanelWidget>(FElementNodeFactory::FOnCreateElementNode::CreateStatic(FPanelWidgetElementNode::Create));
	FElementNodeFactory::Get().Register<UContentWidget>(FElementNodeFactory::FOnCreateElementNode::CreateStatic(FContentWidgetElementNode::Create));
	FElementNodeFactory::Get().Register<UWidgetBlueprint>(FElementNodeFactory::FOnCreateElementNode::CreateStatic(FWidgetBlueprintElementNode::Create));

	// --- Converters for Slate types ----------------------------------------------------
	FConverterRegistry::Get().Register(FSlateChildSize::StaticStruct()->GetFName(), FConverterRegistry::FOnCreateConverter::CreateStatic(FSlateChildSizeConverter::Create));
	FConverterRegistry::Get().Register(FSlateColor::StaticStruct()->GetFName(), FConverterRegistry::FOnCreateConverter::CreateStatic(FSlateColorConverter::Create));
	FConverterRegistry::Get().Register(StaticStruct<FDeprecateSlateVector2D>()->GetFName(), FConverterRegistry::FOnCreateConverter::CreateStatic(TVectorConverter<decltype(FDeprecateSlateVector2D::X), 2>::Create));

	// --- XML behaviour on UMG types ----------------------------------------------------
	Module.RegisterCustomPropertyRun(UWidgetBlueprint::StaticClass(), TEXT("Script"), FWidgetMarkupModule::FOnCreatePropertyRun::CreateStatic(&FWidgetBlueprintScriptPropertyRun::Create));
	Module.RegisterCustomPropertyRun(UWidget::StaticClass(), TEXT("Style"), FWidgetMarkupModule::FOnCreatePropertyRun::CreateStatic(&FWidgetStylePropertyRun::Create));

	// --- Adapters for specific widgets -------------------------------------------------
	// ListItems is transient runtime state, so a value written into the template never
	// reaches an instance; the deferred run captures it and applies it per instance.
	Module.RegisterCustomPropertyRun(UListView::StaticClass(), TEXT("ListItems"), FWidgetMarkupModule::FOnCreatePropertyRun::CreateStatic(&FDeferredPropertyRun::Create));

	// Properties whose plain copy is not the correct runtime operation: the engine
	// reads them only at construction (DefaultOptions) or only through a dedicated
	// API (ListItems, SelectedOption).
	Module.RegisterPropertyResync<UListView, TArray<UObject*>>(TEXT("ListItems"),
		[](UListView& ListView, const TArray<UObject*>& Items)
		{
			// Transient runtime state: the engine API also notifies OnItemsChanged.
			ListView.SetListItems(Items);
		});

	// Applies UComboBoxString::SelectedOption the way the widget itself would.
	//
	// UComboBoxString::SetSelectedIndex() updates the content area only when the
	// property differs from the option it is about to select, and the generic property
	// write already stored the new value - so the selection would be remembered but
	// never drawn (GetSelectedOption() would even report success, because it reads the
	// internal pointer). Rewinding the property forces a real change; the widget then
	// writes it back itself.
	//
	// Selection is taken by value on purpose: the caller may hand over a value that
	// aliases the property memory this lambda rewrites.
	const auto SelectComboBoxOption = [](UComboBoxString& ComboBox, FString Selection)
	{
		// SelectedOption is private, so reflection is the only way in.
		FStrProperty* SelectionProperty = FindFProperty<FStrProperty>(UComboBoxString::StaticClass(), TEXT("SelectedOption"));
		if (!SelectionProperty)
		{
			return;
		}

		void* SelectionAddress = SelectionProperty->ContainerPtrToValuePtr<void>(&ComboBox);
		if (Selection.IsEmpty())
		{
			ComboBox.ClearSelection();
			SelectionProperty->SetPropertyValue(SelectionAddress, FString());
			return;
		}

		SelectionProperty->SetPropertyValue(SelectionAddress, FString());
		ComboBox.SetSelectedOption(Selection);

		if (SelectionProperty->GetPropertyValue(SelectionAddress) != Selection)
		{
			// The option is not in the list: nothing can be drawn, so drop the shown
			// item but keep the requested value for a later DefaultOptions write.
			ComboBox.ClearSelection();
			SelectionProperty->SetPropertyValue(SelectionAddress, Selection);
		}
	};

	Module.RegisterPropertyResync<UComboBoxString, TArray<FString>>(TEXT("DefaultOptions"),
		[SelectComboBoxOption](UComboBoxString& ComboBox, const TArray<FString>& Options)
		{
			// UComboBoxString expands DefaultOptions into its runtime option list only
			// in PostInitProperties()/PostLoad() and never re-reads it, so rebuild the
			// list here. ClearOptions() also drops the drawn selection, so read the
			// pending one (private property: reflection) and restore it afterwards.
			FStrProperty* SelectionProperty = FindFProperty<FStrProperty>(UComboBoxString::StaticClass(), TEXT("SelectedOption"));
			void* SelectionAddress = SelectionProperty ? SelectionProperty->ContainerPtrToValuePtr<void>(&ComboBox) : nullptr;
			const FString PreviousSelection = SelectionAddress ? SelectionProperty->GetPropertyValue(SelectionAddress) : FString();

			ComboBox.ClearOptions();
			for (const FString& Option : Options)
			{
				ComboBox.AddOption(Option);
			}

			// Restoring the selection is also what makes RebuildWidget() draw the item
			// when the Slate widget does not exist yet.
			if (SelectionAddress)
			{
				SelectComboBoxOption(ComboBox, PreviousSelection);
			}
		});

	Module.RegisterPropertyResync<UComboBoxString, FString>(TEXT("SelectedOption"),
		[SelectComboBoxOption](UComboBoxString& ComboBox, const FString& Selection)
		{
			// A plain copy changes the string but leaves SComboBox drawing the item it
			// was last given.
			SelectComboBoxOption(ComboBox, Selection);
		});

	// UComboBoxKey has no refresh API either, so the option list is rebuilt the same way.
	// Its SelectedOption is different: SetSelectedOption() calls the Slate widget when one
	// exists and does not write the property back itself, so the requested value is
	// restored after the call (and there is nothing to clear when the option is unknown -
	// the property should simply keep what was asked for).
	const auto SelectComboBoxKeyOption = [](UComboBoxKey& ComboBox, FName Option)
	{
		FNameProperty* SelectionProperty = FindFProperty<FNameProperty>(UComboBoxKey::StaticClass(), TEXT("SelectedOption"));
		if (!SelectionProperty)
		{
			return;
		}

		void* SelectionAddress = SelectionProperty->ContainerPtrToValuePtr<void>(&ComboBox);
		if (Option.IsNone())
		{
			ComboBox.ClearSelection();
			SelectionProperty->SetPropertyValue(SelectionAddress, NAME_None);
			return;
		}

		SelectionProperty->SetPropertyValue(SelectionAddress, NAME_None);
		ComboBox.SetSelectedOption(Option);

		if (SelectionProperty->GetPropertyValue(SelectionAddress) != Option)
		{
			SelectionProperty->SetPropertyValue(SelectionAddress, Option);
		}
	};

	Module.RegisterPropertyResync<UComboBoxKey, TArray<FName>>(TEXT("Options"),
		[SelectComboBoxKeyOption](UComboBoxKey& ComboBox, const TArray<FName>& Options)
		{
			// Options is the array SComboBox draws from, but only AddOption()/ClearOptions()
			// refresh the widget. ClearOptions() also drops the selection, so restore it.
			FNameProperty* SelectionProperty = FindFProperty<FNameProperty>(UComboBoxKey::StaticClass(), TEXT("SelectedOption"));
			void* SelectionAddress = SelectionProperty ? SelectionProperty->ContainerPtrToValuePtr<void>(&ComboBox) : nullptr;
			const FName PreviousSelection = SelectionAddress ? SelectionProperty->GetPropertyValue(SelectionAddress) : NAME_None;

			ComboBox.ClearOptions();
			for (const FName& Option : Options)
			{
				ComboBox.AddOption(Option);
			}

			if (SelectionAddress)
			{
				SelectComboBoxKeyOption(ComboBox, PreviousSelection);
			}
		});

	Module.RegisterPropertyResync<UComboBoxKey, FName>(TEXT("SelectedOption"),
		[SelectComboBoxKeyOption](UComboBoxKey& ComboBox, const FName& Selection)
		{
			SelectComboBoxKeyOption(ComboBox, Selection);
		});

	// UComboBox exposes no refresh API at all: RebuildWidget() only hands SComboBox a
	// pointer to Items, while the Slate widget keeps its own copy of the options, so the
	// cached widget has to be told to re-read the array.
	Module.RegisterPropertyResync<UComboBox, TArray<UObject*>>(TEXT("Items"),
		[](UComboBox& ComboBox, const TArray<UObject*>& /*Items*/)
		{
			if (TSharedPtr<SWidget> CachedWidget = ComboBox.GetCachedWidget())
			{
				if (TSharedPtr<SComboBox<UObject*>> SlateComboBox = StaticCastSharedPtr<SComboBox<UObject*>>(CachedWidget))
				{
					SlateComboBox->RefreshOptions();
				}
			}
		});

	// DecoratorClasses is only the input: URichTextBlock builds its decorators (and a
	// style set) from it, so SetDecorators() is the call that makes a write take effect.
	Module.RegisterPropertyResync<URichTextBlock, TArray<UObject*>>(TEXT("DecoratorClasses"),
		[](URichTextBlock& TextBlock, const TArray<UObject*>& DecoratorClasses)
		{
			TArray<TSubclassOf<URichTextBlockDecorator>> DecoratorClassList;
			DecoratorClassList.Reserve(DecoratorClasses.Num());
			for (UObject* Entry : DecoratorClasses)
			{
				if (UClass* EntryClass = Cast<UClass>(Entry))
				{
					if (EntryClass->IsChildOf(URichTextBlockDecorator::StaticClass()))
					{
						DecoratorClassList.Add(EntryClass);
					}
				}
			}

			if (DecoratorClassList.Num() != DecoratorClasses.Num())
			{
				UE_LOG(LogWidgetMarkup, Warning, TEXT("RichTextBlock: dropped %d DecoratorClasses entr(ies) that are not URichTextBlockDecorator classes."),
					DecoratorClasses.Num() - DecoratorClassList.Num());
			}

			TextBlock.SetDecorators(DecoratorClassList);
		});
}

void UnregisterWidgetRegistrations(FWidgetMarkupModule& Module)
{
	// Same order as RegisterWidgetRegistrations, so both lists can be compared line by line.
	FElementNodeFactory::Get().Unregister<UWidgetTree>();
	FElementNodeFactory::Get().Unregister<UWidget>();
	FElementNodeFactory::Get().Unregister<UPanelWidget>();
	FElementNodeFactory::Get().Unregister<UContentWidget>();
	FElementNodeFactory::Get().Unregister<UWidgetBlueprint>();

	FConverterRegistry::Get().Unregister(FSlateChildSize::StaticStruct()->GetFName());
	FConverterRegistry::Get().Unregister(FSlateColor::StaticStruct()->GetFName());
	FConverterRegistry::Get().Unregister(StaticStruct<FDeprecateSlateVector2D>()->GetFName());

	Module.UnregisterCustomPropertyRun(UWidgetBlueprint::StaticClass(), TEXT("Script"));
	Module.UnregisterCustomPropertyRun(UWidget::StaticClass(), TEXT("Style"));

	Module.UnregisterCustomPropertyRun(UListView::StaticClass(), TEXT("ListItems"));
	Module.UnregisterCustomPropertySetter(UListView::StaticClass(), TEXT("ListItems"));
	Module.UnregisterCustomPropertySetter(UComboBoxString::StaticClass(), TEXT("DefaultOptions"));
	Module.UnregisterCustomPropertySetter(UComboBoxString::StaticClass(), TEXT("SelectedOption"));
	Module.UnregisterCustomPropertySetter(UComboBoxKey::StaticClass(), TEXT("Options"));
	Module.UnregisterCustomPropertySetter(UComboBoxKey::StaticClass(), TEXT("SelectedOption"));
	Module.UnregisterCustomPropertySetter(UComboBox::StaticClass(), TEXT("Items"));
	Module.UnregisterCustomPropertySetter(URichTextBlock::StaticClass(), TEXT("DecoratorClasses"));
}
