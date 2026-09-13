import widget_markup
from WidgetMarkupComponent import reactive
from Tests.TestComponent import TestComponent


class TestComboBoxString(TestComponent):
    """ComboBoxString option list and selection, written from XML and from a binding.

    UComboBoxString expands DefaultOptions into its runtime option list only in
    PostInitProperties()/PostLoad(), so a write has to rebuild it; and it redraws the
    content area only when the selection property changes, so a late update has to
    force that. Both are the setter's job, and both are observed here through the
    widget (GetOptionCount() reads the runtime list, GetSelectedOption() the drawn item).
    """

    @reactive
    def combo_options(self):
        return ["ComboBox", "Second entry"]

    @reactive
    def selection(self):
        return "ComboBox"

    def __init__(self):
        try:
            super().__init__("TestComboBoxString")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            literal = self.find_widget("LiteralCombo")
            bound = self.find_widget("BoundCombo")
            self.check_not_none(literal, "property-element combo found")
            self.check_not_none(bound, "bound combo found")

            expected = ["ComboBox", "Second entry"]
            self.check_equal(
                list(literal.get_editor_property("DefaultOptions")), expected, "property-element DefaultOptions"
            )
            self.check_equal(list(bound.get_editor_property("DefaultOptions")), expected, "bound DefaultOptions")

            # GetOptionCount() reads the runtime option list SComboBox draws, which the
            # setter rebuilds; the property alone would not be enough.
            self.check_equal(literal.get_option_count(), 2, "property-element options rebuilt")
            self.check_equal(bound.get_option_count(), 2, "bound options rebuilt")

            # GetSelectedOption() reads the item SComboBox currently draws, which needs the
            # built widget - see the deferred phase below. The property is checkable here.
            self.check_equal(
                literal.get_editor_property("SelectedOption"), "ComboBox", "property-element selection property"
            )
            self.check_equal(bound.get_editor_property("SelectedOption"), "ComboBox", "bound selection property")

            self.defer_checks(self.check_initial_selection)
            self.defer_checks(self.check_late_update)
        finally:
            self.shutdown_if_test_mode()

    def check_initial_selection(self):
        """The built widget must draw the selection that was written before it was taken."""
        combo = self.find_widget("BoundCombo")
        self.check_equal(combo.get_selected_option(), "ComboBox", "drawn selection after build")

    def check_late_update(self):
        """A reactive update after the widget exists must reach the widget, not just the property."""
        combo = self.find_widget("BoundCombo")
        self.check_equal(combo.get_selected_option(), "ComboBox", "drawn selection before the late update")

        self.selection = "Second entry"
        self.combo_options = ["ComboBox", "Second entry", "Third entry"]

        # Another phase: a reactive update is pushed from Python when it is set.
        self.defer_checks(self.check_late_update_applied)

    def check_late_update_applied(self):
        combo = self.find_widget("BoundCombo")
        self.check_equal(combo.get_selected_option(), "Second entry", "late selection reached the widget")
        self.check_equal(combo.get_option_count(), 3, "late option list reached the widget")
        self.check_equal(len(list(combo.get_editor_property("DefaultOptions"))), 3, "late DefaultOptions property")
