import unreal
import widget_markup
from Tests.TestComponent import TestComponent


class TestHorizontalBox(TestComponent):
    def __init__(self):
        try:
            super().__init__("TestHorizontalBox")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            horizontal_box = self.find_widget("TestHorizontal")
            self.check_not_none(horizontal_box, "TestHorizontal found")
            if horizontal_box:
                self.check_equal(horizontal_box.get_children_count(), 2, "HorizontalBox has 2 children")

            # Child1: Fill slot
            horizontal_child1 = self.find_widget("HorizontalChild1")
            self.check_not_none(horizontal_child1, "HorizontalChild1 found")
            if horizontal_child1 and horizontal_child1.slot:
                slot1 = horizontal_child1.slot
                self.check_equal(slot1.get_editor_property("HorizontalAlignment"), unreal.HorizontalAlignment.H_ALIGN_CENTER, "HorizontalChild1.HAlign Center")
                self.check_equal(slot1.get_editor_property("VerticalAlignment"), unreal.VerticalAlignment.V_ALIGN_CENTER, "HorizontalChild1.VAlign Center")
                pad1 = slot1.get_editor_property("Padding")
                self.check_not_none(pad1, "HorizontalChild1.Padding set")
                if pad1:
                    self.check_almost_equal(pad1.left, 4.0, 0.001, "HorizontalChild1.Padding.Left")
                    self.check_almost_equal(pad1.top, 2.0, 0.001, "HorizontalChild1.Padding.Top")

            # Child2: Automatic slot
            horizontal_child2 = self.find_widget("HorizontalChild2")
            self.check_not_none(horizontal_child2, "HorizontalChild2 found")
            if horizontal_child2 and horizontal_child2.slot:
                slot2 = horizontal_child2.slot
                self.check_equal(slot2.get_editor_property("HorizontalAlignment"), unreal.HorizontalAlignment.H_ALIGN_LEFT, "HorizontalChild2.HAlign Left")
                self.check_equal(slot2.get_editor_property("VerticalAlignment"), unreal.VerticalAlignment.V_ALIGN_TOP, "HorizontalChild2.VAlign Top")
                pad2 = slot2.get_editor_property("Padding")
                self.check_not_none(pad2, "HorizontalChild2.Padding set")
                if pad2:
                    self.check_almost_equal(pad2.left, 8.0, 0.001, "HorizontalChild2.Padding.Left")
                    self.check_almost_equal(pad2.top, 4.0, 0.001, "HorizontalChild2.Padding.Top")

            self.report()
        finally:
            if widget_markup.Application.is_test_mode():
                widget_markup.Application.request_shutdown()
