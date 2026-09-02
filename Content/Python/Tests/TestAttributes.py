from __future__ import annotations

import unreal
import widget_markup
from Tests.TestComponent import TestComponent


class TestAttributes(TestComponent):
    def __init__(self):
        super().__init__("TestAttributes")

        # C++ type: UWidgetMarkupLibrary (BlueprintFunctionLibrary;
        # stateless static functions, callable without an instance).
        lib = unreal.WidgetMarkupLibrary

        # C++ type: FWidgetMarkupElementInfoList { TArray<FWidgetMarkupElementInfo> Elements; }
        elements: unreal.WidgetMarkupElementList = lib.get_elements()
        # C++ type: TArray<FWidgetMarkupElementInfo>
        element_entries: list[unreal.WidgetMarkupElementInfo] = elements.get_editor_property("Elements")
        # C++ type: TArray<FString>
        element_names: list[str] = [entry.get_editor_property("Name") for entry in element_entries]
        for name in ("Button", "CanvasPanel", "ListView", "WidgetBlueprint", "Blueprint",
                     "WidgetTree", "Variable", "Pair", "StyleSheet", "Style", "Setter"):
            self.check_true(name in element_names, f"elements contain {name}")

        # C++ type: FWidgetMarkupAttributeInfoList { Element, Prefix, Attributes, Error }
        root: unreal.WidgetMarkupAttributeList = lib.get_attributes("Button", "")
        # C++ type: FString
        self.check_equal(root.get_editor_property("Error"), "", "button attributes: no error")
        # C++ type: TArray<FWidgetMarkupAttributeInfo>
        root_attributes: list[unreal.WidgetMarkupAttributeInfo] = root.get_editor_property("Attributes")
        root_names: list[str] = [attr.get_editor_property("Name") for attr in root_attributes]
        self.check_true("OnClicked" in root_names, "button has OnClicked")
        on_clicked: unreal.WidgetMarkupAttributeInfo = next(
            attr for attr in root_attributes if attr.get_editor_property("Name") == "OnClicked"
        )
        self.check_equal(on_clicked.get_editor_property("Type"), "Delegate<void()>", "OnClicked has a delegate signature")
        self.check_true("WidgetStyle" in root_names, "button has WidgetStyle")
        # C++ type: FWidgetMarkupAttributeInfo { Name, Type }
        name_attr: unreal.WidgetMarkupAttributeInfo = next(
            attr for attr in root_attributes if attr.get_editor_property("Name") == "Name"
        )
        self.check_equal(name_attr.get_editor_property("Type"), "FString", "Name attribute is a string custom run")

        # C++ type: FWidgetMarkupAttributeInfoList (Prefix = "WidgetStyle.")
        style: unreal.WidgetMarkupAttributeList = lib.get_attributes("Button", "WidgetStyle.")
        self.check_equal(style.get_editor_property("Error"), "", "WidgetStyle drill: no error")
        style_names: list[str] = [attr.get_editor_property("Name") for attr in style.get_editor_property("Attributes")]
        self.check_true("Normal" in style_names, "WidgetStyle drills into FButtonStyle")

        # C++ type: FWidgetMarkupAttributeInfoList (Slot special case for panels)
        slot_attributes: list[unreal.WidgetMarkupAttributeInfo] = lib.get_attributes(
            "CanvasPanel", ""
        ).get_editor_property("Attributes")
        slot_attr: unreal.WidgetMarkupAttributeInfo = next(
            attr for attr in slot_attributes if attr.get_editor_property("Name") == "Slot"
        )
        self.check_equal(slot_attr.get_editor_property("Type"), "CanvasPanelSlot", "Slot attribute points at CanvasPanelSlot")

        slot_fields: unreal.WidgetMarkupAttributeList = lib.get_attributes("CanvasPanel", "Slot.")
        slot_field_names: list[str] = [attr.get_editor_property("Name") for attr in slot_fields.get_editor_property("Attributes")]
        self.check_true("LayoutData" in slot_field_names, "Slot drills into CanvasPanelSlot")

        layout: unreal.WidgetMarkupAttributeList = lib.get_attributes("CanvasPanel", "Slot.LayoutData.")
        layout_names: list[str] = [attr.get_editor_property("Name") for attr in layout.get_editor_property("Attributes")]
        self.check_true("Offsets" in layout_names, "LayoutData drills into FAnchorData")

        # C++ type: FWidgetMarkupAttributeInfoList (custom property runs per element)
        widget_blueprint: unreal.WidgetMarkupAttributeList = lib.get_attributes("WidgetBlueprint", "")
        widget_blueprint_attributes: dict[str, unreal.WidgetMarkupAttributeInfo] = {
            attr.get_editor_property("Name"): attr for attr in widget_blueprint.get_editor_property("Attributes")
        }
        self.check_true("Script" in widget_blueprint_attributes, "WidgetBlueprint has Script")
        self.check_equal(widget_blueprint_attributes["Script"].get_editor_property("Type"), "FString", "Script attribute is a string custom run")
        self.check_true("Super" in widget_blueprint_attributes, "WidgetBlueprint has Super")

        variable: unreal.WidgetMarkupAttributeList = lib.get_attributes("Variable", "")
        variable_names: list[str] = [attr.get_editor_property("Name") for attr in variable.get_editor_property("Attributes")]
        self.check_true("Default" in variable_names, "Variable has Default custom run")

        list_items: list[unreal.WidgetMarkupAttributeInfo] = [
            attr for attr in lib.get_attributes("ListView", "").get_editor_property("Attributes")
            if attr.get_editor_property("Name") == "ListItems"
        ]
        self.check_true(len(list_items) == 1, "ListView has ListItems custom run")

        # C++ type: FWidgetMarkupAttributeInfoList with Error = "UNKNOWN_ELEMENT"
        unknown: unreal.WidgetMarkupAttributeList = lib.get_attributes("NotAThing", "")
        self.check_equal(unknown.get_editor_property("Error"), "UNKNOWN_ELEMENT", "unknown element error")
        self.check_equal(len(unknown.get_editor_property("Attributes")), 0, "unknown element: empty attributes")

        # C++ type: FWidgetMarkupAttributeInfoList with Error = "UNKNOWN_PATH"
        bad_path: unreal.WidgetMarkupAttributeList = lib.get_attributes("Button", "Font.")
        self.check_equal(bad_path.get_editor_property("Error"), "UNKNOWN_PATH", "unknown path error")

        # C++ type: FWidgetMarkupAttributeInfoList (leaf attribute ends the walk)
        leaf: unreal.WidgetMarkupAttributeList = lib.get_attributes("Button", "Visibility.")
        self.check_equal(leaf.get_editor_property("Error"), "", "leaf attribute: no error")
        self.check_equal(len(leaf.get_editor_property("Attributes")), 0, "leaf attribute: empty list")

        self.report()
        if widget_markup.Application.get_extra_arguments() == "test":
            widget_markup.Application.request_shutdown()
