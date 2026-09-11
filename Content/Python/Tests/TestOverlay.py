import unreal
import widget_markup
from Tests.TestComponent import TestComponent


class TestOverlay(TestComponent):
    def __init__(self):
        try:
            super().__init__("TestOverlay")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            overlay_panel = self.find_widget("TestOverlayPanel")
            self.check_not_none(overlay_panel, "TestOverlayPanel found")
            if overlay_panel:
                self.check_equal(overlay_panel.get_children_count(), 2, "Overlay has 2 children")

            # Child1: fill aligned
            overlay_child1 = self.find_widget("OverlayChild1")
            self.check_not_none(overlay_child1, "OverlayChild1 found")
            if overlay_child1 and overlay_child1.slot:
                slot1 = overlay_child1.slot
                self.check_equal(slot1.get_editor_property("HorizontalAlignment"), unreal.HorizontalAlignment.H_ALIGN_FILL, "OverlayChild1.HAlign Fill")
                self.check_equal(slot1.get_editor_property("VerticalAlignment"), unreal.VerticalAlignment.V_ALIGN_FILL, "OverlayChild1.VAlign Fill")
                pad1 = slot1.get_editor_property("Padding")
                self.check_not_none(pad1, "OverlayChild1.Padding set")
                if pad1:
                    self.check_almost_equal(pad1.left, 4.0, 0.001, "OverlayChild1.Padding.Left")
                    self.check_almost_equal(pad1.top, 2.0, 0.001, "OverlayChild1.Padding.Top")

            # Child2: bottom-right aligned
            overlay_child2 = self.find_widget("OverlayChild2")
            self.check_not_none(overlay_child2, "OverlayChild2 found")
            if overlay_child2 and overlay_child2.slot:
                slot2 = overlay_child2.slot
                self.check_equal(slot2.get_editor_property("HorizontalAlignment"), unreal.HorizontalAlignment.H_ALIGN_RIGHT, "OverlayChild2.HAlign Right")
                self.check_equal(slot2.get_editor_property("VerticalAlignment"), unreal.VerticalAlignment.V_ALIGN_BOTTOM, "OverlayChild2.VAlign Bottom")
                pad2 = slot2.get_editor_property("Padding")
                self.check_not_none(pad2, "OverlayChild2.Padding set")
                if pad2:
                    self.check_almost_equal(pad2.left, 8.0, 0.001, "OverlayChild2.Padding.Left")
                    self.check_almost_equal(pad2.top, 4.0, 0.001, "OverlayChild2.Padding.Top")

            self.report()
        finally:
            if widget_markup.Application.is_test_mode():
                widget_markup.Application.request_shutdown()
