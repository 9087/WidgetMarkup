import widget_markup
from WidgetMarkupComponent import reactive
from Tests.TestComponent import TestComponent


class TestComboBoxKey(TestComponent):
    """ComboBoxKey option list and selection written from bindings.

    Options is the array SComboBox draws from and only AddOption()/ClearOptions() notify
    the widget, so the setter rebuilds it; SelectedOption is different from
    UComboBoxString - its SetSelectedOption() calls the Slate widget and does not write
    the property back, so the setter restores the requested value after the call.
    """

    @reactive
    def key_options(self):
        return ["FirstKey", "SecondKey"]

    @reactive
    def key_selection(self):
        return "SecondKey"

    def __init__(self):
        try:
            super().__init__("TestComboBoxKey")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            combo = self.find_widget("KeyCombo")
            self.check_not_none(combo, "combo key found")

            self.check_equal(
                [str(option) for option in combo.get_editor_property("Options")],
                ["FirstKey", "SecondKey"],
                "option list",
            )
            # GetSelectedOption() returns the property, so it can be read before the widget
            # is built; the Slate item itself is generated from this property.
            self.check_equal(str(combo.get_selected_option()), "SecondKey", "selection")

            self.defer_checks(self.check_late_update)
        finally:
            self.shutdown_if_test_mode()

    def check_late_update(self):
        """A reactive selection change after the widget exists must reach it."""
        combo = self.find_widget("KeyCombo")
        self.selection_target = "FirstKey"
        self.key_selection = "FirstKey"

        self.defer_checks(self.check_late_update_applied)

    def check_late_update_applied(self):
        combo = self.find_widget("KeyCombo")
        self.check_equal(str(combo.get_selected_option()), "FirstKey", "late selection")
