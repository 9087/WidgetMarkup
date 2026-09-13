import widget_markup
from WidgetMarkupComponent import reactive
from Tests.TestComponent import TestComponent


class TestListView(TestComponent):
    @reactive
    def items(self):
        return list("ABC")

    def __init__(self):
        try:
            super().__init__("TestListView")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            list_view = self.find_widget("TestListView")
            self.check_not_none(list_view, "ListView found")

            entry_class = list_view.get_editor_property("EntryWidgetClass")
            self.check_not_none(entry_class, "EntryWidgetClass is set")

            self.check_equal(len(self.items), 3, "initial length == 3")
            self.check_equal(self.items[0], "A", "items[0] == 'A'")
            self.check_equal(self.items[1], "B", "items[1] == 'B'")

            self.items.append("D")
            self.check_equal(len(self.items), 4, "after append length == 4")
            self.check_equal(self.items[3], "D", "items[3] == 'D'")

            self.items.pop(0)
            self.check_equal(len(self.items), 3, "after pop(0) length == 3")
            self.check_equal(self.items[0], "B", "items[0] == 'B'")

            self.items.clear()
            self.check_equal(len(self.items), 0, "after clear length == 0")

            # Inline items: GetNumItems() reads ListItems itself, so these only pass when
            # the deferred style assignment reached this instance.
            inline_view = self.find_widget("InlineList")
            self.check_not_none(inline_view, "inline list found")
            self.check_equal(inline_view.get_num_items(), 3, "three inline items")
            for index, expected in enumerate(("InlineItemA", "InlineItemB", "InlineItemC")):
                item = inline_view.get_item_at(index)
                self.check_equal(str(item.get_name()) if item else None, expected, f"inline item {index} name")
                self.check_equal(
                    item.get_class().get_name() if item else None, "DataTable", f"inline item {index} class"
                )

            self.report()
        finally:
            if widget_markup.Application.is_test_mode():
                widget_markup.Application.request_shutdown()
