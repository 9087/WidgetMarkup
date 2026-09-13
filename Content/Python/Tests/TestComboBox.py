import widget_markup
from Tests.TestComponent import TestComponent


class TestComboBox(TestComponent):
    """UComboBox item list written as child elements.

    Items is the array SComboBox reads, but the Slate widget keeps its own copy of the
    list, so a write only shows up once the cached widget is refreshed - which is what
    the setter does (UMG offers no API for it).
    """

    def __init__(self):
        try:
            super().__init__("TestComboBox")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            combo = self.find_widget("ObjectCombo")
            self.check_not_none(combo, "object combo found")

            items = list(combo.get_editor_property("Items"))
            self.check_equal(len(items), 2, "item count")
            self.check_equal(
                [item.get_class().get_name() for item in items], ["DataTable", "DataTable"], "item classes"
            )
            self.check_equal([item.get_name() for item in items], ["ObjectItemA", "ObjectItemB"], "item names")
            self.report()
        finally:
            if widget_markup.Application.is_test_mode():
                widget_markup.Application.request_shutdown()
