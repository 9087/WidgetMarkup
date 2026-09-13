import widget_markup
from WidgetMarkupComponent import reactive
from Tests.TestComponent import TestComponent


class TestArrayBinding(TestComponent):
    """Array values in XML (property elements) and array data binding.

    Both forms must reach the widget; they differ in *when* they are written,
    which matters for widgets that only read their array on construction
    (UComboBoxString expands DefaultOptions in PostInitProperties/PostLoad).
    """

    @reactive
    def options(self):
        return ["ComboBox", "Second entry"]

    @reactive
    def label(self):
        return "bound"

    def __init__(self):
        try:
            super().__init__("TestArrayBinding")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            literal = self.find_widget("ComboLiteral")
            bound = self.find_widget("ComboBound")
            label = self.find_widget("Label")
            self.check_not_none(literal, "property-element combo found")
            self.check_not_none(bound, "bound combo found")
            self.check_not_none(label, "label found")

            expected = ["ComboBox", "Second entry"]
            self.check_equal(
                list(literal.get_editor_property("DefaultOptions")),
                expected,
                "property-element DefaultOptions",
            )
            self.check_equal(
                list(bound.get_editor_property("DefaultOptions")),
                expected,
                "bound DefaultOptions",
            )
            self.check_equal(label.get_editor_property("Text"), "bound", "scalar binding")

            # GetOptionCount() reads the runtime option list SComboBox draws, not
            # DefaultOptions. UComboBoxString only fills it in PostInitProperties/
            # PostLoad, so the bound combo box only has options if the array write
            # also rebuilt them (FComboBoxStringDefaultOptionsPropertySetter).
            self.check_equal(literal.get_option_count(), 2, "property-element options reached the widget")
            self.check_equal(bound.get_option_count(), 2, "bound options reached the widget")
            # GetSelectedOption() reads the item SComboBox currently draws, which
            # only exists once the Slate widget was built. The property-element
            # combo is not built yet while this runs (the Python component is
            # created before TakeWidget), so assert its property instead; the
            # bound combo is selected here because the array write restores the
            # selection itself.
            self.check_equal(
                literal.get_editor_property("SelectedOption"), "ComboBox", "property-element selection property"
            )
            self.check_equal(bound.get_selected_option(), "ComboBox", "bound selection is restored")
            self.report()
        finally:
            if widget_markup.Application.is_test_mode():
                widget_markup.Application.request_shutdown()
