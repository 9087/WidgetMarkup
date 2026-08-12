import unreal
import widget_markup
from Tests.TestComponent import TestComponent


class TestGridPanel(TestComponent):
    """Verify ordinary-property Array containers (child-element route A) via
    GridPanel.ColumnFill / RowFill, plus UGridSlot properties."""

    def __init__(self):
        try:
            super().__init__("TestGridPanel")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            grid = self.find_widget("TestGrid")
            self.check_not_none(grid, "TestGrid found")
            if not grid:
                return

            # Array container assembled from Basic-type child elements.
            column_fill = grid.get_editor_property("ColumnFill")
            self.check_equal(column_fill, [1.0, 2.0, 1.0], "ColumnFill = [1.0, 2.0, 1.0]")
            row_fill = grid.get_editor_property("RowFill")
            self.check_equal(row_fill, [1.0, 1.0], "RowFill = [1.0, 1.0]")

            self.check_equal(grid.get_children_count(), 2, "Grid has 2 children")

            child0 = self.find_widget("GridChild0")
            self.check_not_none(child0, "GridChild0 found")
            if child0 and child0.slot:
                slot0 = child0.slot
                self.check_equal(slot0.get_editor_property("Row"), 0, "GridChild0.Slot.Row")
                self.check_equal(slot0.get_editor_property("Column"), 0, "GridChild0.Slot.Column")

            child1 = self.find_widget("GridChild1")
            self.check_not_none(child1, "GridChild1 found")
            if child1 and child1.slot:
                slot1 = child1.slot
                self.check_equal(slot1.get_editor_property("Row"), 0, "GridChild1.Slot.Row")
                self.check_equal(slot1.get_editor_property("Column"), 1, "GridChild1.Slot.Column")

            self.report()
        finally:
            if widget_markup.Application.get_extra_arguments() == "test":
                widget_markup.Application.request_shutdown()
