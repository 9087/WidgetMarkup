import unreal
import widget_markup
from Tests.TestComponent import TestComponent


class TestVerticalBox(TestComponent):
    def __init__(self):
        try:
            super().__init__("TestVerticalBox")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            vertical_box = self.find_widget("TestVertical")
            self.check_not_none(vertical_box, "TestVertical found")
            if vertical_box:
                self.check_equal(vertical_box.get_children_count(), 2, "VerticalBox has 2 children")

            # Child1: Fill slot
            vertical_child1 = self.find_widget("VerticalChild1")
            self.check_not_none(vertical_child1, "VerticalChild1 found")
            if vertical_child1 and vertical_child1.slot:
                slot1 = vertical_child1.slot
                size1 = slot1.get_editor_property("Size")
                self.check_not_none(size1, "VerticalChild1.Size set")
                if size1:
                    self.check_equal(size1.get_editor_property("SizeRule"), unreal.SlateSizeRule.FILL, "VerticalChild1.Size.SizeRule is Fill")
                    self.check_almost_equal(size1.get_editor_property("Value"), 1.0, 0.001, "VerticalChild1.Size.Value")
                self.check_equal(slot1.get_editor_property("HorizontalAlignment"), unreal.HorizontalAlignment.H_ALIGN_FILL, "VerticalChild1.HAlign Fill")
                self.check_equal(slot1.get_editor_property("VerticalAlignment"), unreal.VerticalAlignment.V_ALIGN_CENTER, "VerticalChild1.VAlign Center")
                pad1 = slot1.get_editor_property("Padding")
                self.check_not_none(pad1, "VerticalChild1.Padding set")
                if pad1:
                    self.check_almost_equal(pad1.left, 4.0, 0.001, "VerticalChild1.Padding.Left")
                    self.check_almost_equal(pad1.top, 2.0, 0.001, "VerticalChild1.Padding.Top")

            # Child2: Automatic slot
            vertical_child2 = self.find_widget("VerticalChild2")
            self.check_not_none(vertical_child2, "VerticalChild2 found")
            if vertical_child2 and vertical_child2.slot:
                slot2 = vertical_child2.slot
                size2 = slot2.get_editor_property("Size")
                self.check_not_none(size2, "VerticalChild2.Size set")
                if size2:
                    self.check_equal(size2.get_editor_property("SizeRule"), unreal.SlateSizeRule.AUTOMATIC, "VerticalChild2.Size.SizeRule is Automatic")
                self.check_equal(slot2.get_editor_property("HorizontalAlignment"), unreal.HorizontalAlignment.H_ALIGN_RIGHT, "VerticalChild2.HAlign Right")
                self.check_equal(slot2.get_editor_property("VerticalAlignment"), unreal.VerticalAlignment.V_ALIGN_BOTTOM, "VerticalChild2.VAlign Bottom")
                pad2 = slot2.get_editor_property("Padding")
                self.check_not_none(pad2, "VerticalChild2.Padding set")
                if pad2:
                    self.check_almost_equal(pad2.left, 8.0, 0.001, "VerticalChild2.Padding.Left")
                    self.check_almost_equal(pad2.top, 4.0, 0.001, "VerticalChild2.Padding.Top")

            self.report()
        finally:
            if widget_markup.Application.is_test_mode():
                widget_markup.Application.request_shutdown()
