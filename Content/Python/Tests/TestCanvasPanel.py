import unreal
import widget_markup
from Tests.TestComponent import TestComponent


class TestCanvasPanel(TestComponent):
    def __init__(self):
        try:
            super().__init__("TestCanvasPanel")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            canvas_panel = self.find_widget("TestCanvas")
            self.check_not_none(canvas_panel, "TestCanvas found")
            if canvas_panel:
                self.check_equal(canvas_panel.get_children_count(), 3, "CanvasPanel has 3 children")

            canvas_child1 = self.find_widget("CanvasChild1")
            self.check_not_none(canvas_child1, "CanvasChild1 found")
            if canvas_child1 and canvas_child1.slot:
                slot1 = canvas_child1.slot
                self.check_equal(slot1.get_editor_property("ZOrder"), 0, "CanvasChild1.ZOrder")
                layout1 = slot1.get_editor_property("LayoutData")
                self.check_not_none(layout1, "CanvasChild1.LayoutData set")
                if layout1:
                    a1 = layout1.get_editor_property("Anchors")
                    self.check_not_none(a1, "CanvasChild1.Anchors set")
                    if a1:
                        min1 = a1.get_editor_property("Minimum")
                        self.check_not_none(min1, "CanvasChild1.Minimum set")
                        if min1:
                            self.check_almost_equal(min1.x, 0.0, 0.001, "CanvasChild1.Minimum.x")
                            self.check_almost_equal(min1.y, 0.0, 0.001, "CanvasChild1.Minimum.y")
                        max1 = a1.get_editor_property("Maximum")
                        self.check_not_none(max1, "CanvasChild1.Maximum set")
                        if max1:
                            self.check_almost_equal(max1.x, 0.5, 0.001, "CanvasChild1.Maximum.x")
                            self.check_almost_equal(max1.y, 0.5, 0.001, "CanvasChild1.Maximum.y")
                    o1 = layout1.get_editor_property("Offsets")
                    self.check_not_none(o1, "CanvasChild1.Offsets set")
                    if o1:
                        self.check_almost_equal(o1.left, 10.0, 0.001, "CanvasChild1.Offsets.left")
                        self.check_almost_equal(o1.top, 10.0, 0.001, "CanvasChild1.Offsets.top")

            canvas_child2 = self.find_widget("CanvasChild2")
            self.check_not_none(canvas_child2, "CanvasChild2 found")
            if canvas_child2 and canvas_child2.slot:
                slot2 = canvas_child2.slot
                self.check_equal(slot2.get_editor_property("ZOrder"), 1, "CanvasChild2.ZOrder")
                layout2 = slot2.get_editor_property("LayoutData")
                self.check_not_none(layout2, "CanvasChild2.LayoutData set")
                if layout2:
                    a2 = layout2.get_editor_property("Anchors")
                    self.check_not_none(a2, "CanvasChild2.Anchors set")
                    if a2:
                        min2 = a2.get_editor_property("Minimum")
                        self.check_not_none(min2, "CanvasChild2.Minimum set")
                        if min2:
                            self.check_almost_equal(min2.x, 0.5, 0.001, "CanvasChild2.Minimum.x")
                        max2 = a2.get_editor_property("Maximum")
                        self.check_not_none(max2, "CanvasChild2.Maximum set")
                        if max2:
                            self.check_almost_equal(max2.x, 1.0, 0.001, "CanvasChild2.Maximum.x")
                            self.check_almost_equal(max2.y, 0.5, 0.001, "CanvasChild2.Maximum.y")
                    o2 = layout2.get_editor_property("Offsets")
                    self.check_not_none(o2, "CanvasChild2.Offsets set")
                    if o2:
                        self.check_almost_equal(o2.right, 10.0, 0.001, "CanvasChild2.Offsets.right")
                        self.check_almost_equal(o2.top, 10.0, 0.001, "CanvasChild2.Offsets.top")

            canvas_child3 = self.find_widget("CanvasChild3")
            self.check_not_none(canvas_child3, "CanvasChild3 found")
            if canvas_child3 and canvas_child3.slot:
                self.check_equal(canvas_child3.slot.get_editor_property("bAutoSize"), True, "CanvasChild3.bAutoSize")
                self.check_equal(canvas_child3.slot.get_editor_property("ZOrder"), 2, "CanvasChild3.ZOrder")

            self.report()
        finally:
            if widget_markup.Application.is_test_mode():
                widget_markup.Application.request_shutdown()
