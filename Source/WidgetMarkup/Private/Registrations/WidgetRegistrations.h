// Copyright 2025 Wu Zhiwei. All Rights Reserved.

#pragma once

class FWidgetMarkupModule;

/**
 * Registrations that target UMG/Slate types rather than the framework's own.
 *
 * They live in their own file so each registration sits next to its matching
 * unregistration and the two lists can be compared at a glance - in the module they
 * were ~100 lines apart. Everything else the module registers stays there: the
 * framework's own types (UWidgetStyleSheet, FWidgetMarkupBlueprintVariable,
 * FWidgetStyleEntry, FWidgetStyleSetter, FWidgetPropertyPath) and the generic UE
 * types (numerics, strings, enums, colors, vectors, ...).
 *
 * Moved here:
 *  - element nodes for UMG classes: UWidgetTree, UWidget, UPanelWidget,
 *    UContentWidget, UWidgetBlueprint
 *  - converters for Slate types: FSlateChildSize, FSlateColor,
 *    FDeprecateSlateVector2D
 *  - XML behaviour on UMG types: UWidgetBlueprint::Script, UWidget::Style
 *  - adapters for specific widgets: UListView::ListItems,
 *    UComboBoxString::DefaultOptions and ::SelectedOption
 *
 * Called from FWidgetMarkupModule::StartupModule / ShutdownModule.
 */
void RegisterWidgetRegistrations(FWidgetMarkupModule& Module);
void UnregisterWidgetRegistrations(FWidgetMarkupModule& Module);
