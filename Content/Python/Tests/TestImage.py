import unreal
import widget_markup
from Tests.TestComponent import TestComponent


class TestImage(TestComponent):
    def __init__(self):
        try:
            super().__init__("TestImage")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            image = self.find_widget("StyledImage")
            self.check_not_none(image, "StyledImage found")
            if image:
                co = image.get_editor_property("ColorAndOpacity")
                self.check_not_none(co, "ColorAndOpacity set")
                if co:
                    self.check_almost_equal(co.r, 1.0, 0.001, "ColorAndOpacity.R")
                    self.check_almost_equal(co.g, 1.0, 0.001, "ColorAndOpacity.G")
                    self.check_almost_equal(co.b, 1.0, 0.001, "ColorAndOpacity.B")
                    self.check_almost_equal(co.a, 0.8, 0.001, "ColorAndOpacity.A")

                self.check_almost_equal(image.get_editor_property("RenderOpacity"), 0.9, 0.001, "RenderOpacity value")

                brush = image.get_editor_property("Brush")
                self.check_not_none(brush, "Brush set")
                if brush:
                    draw_as = brush.get_editor_property("DrawAs")
                    self.check_not_none(draw_as, "Brush.DrawAs set")
                    if draw_as is not None:
                        self.check_true("BOX" in str(draw_as), f"Brush.DrawAs is Box, got {draw_as}")
                    img_size = brush.get_editor_property("ImageSize")
                    self.check_not_none(img_size, "Brush.ImageSize set")
                    if img_size:
                        et = img_size.export_text()
                        self.check_true("X=128" in et and "Y=128" in et, f"Brush.ImageSize = (128,128), got {et}")

                self.check_not_none(image.slot, "CanvasPanelSlot exists")

            self.report()
        finally:
            if widget_markup.Application.is_test_mode():
                widget_markup.Application.request_shutdown()
