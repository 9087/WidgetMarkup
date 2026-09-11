import unreal
import widget_markup
from Tests.TestComponent import TestComponent


class TestButton(TestComponent):
    def __init__(self):
        try:
            super().__init__("TestButton")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            button = self.find_widget("StyledButton")
            self.check_not_none(button, "StyledButton found")
            if button:
                # ColorAndOpacity = (1.0, 1.0, 1.0, 1.0)
                co = button.get_editor_property("ColorAndOpacity")
                self.check_not_none(co, "ColorAndOpacity set")
                if co:
                    self.check_almost_equal(co.r, 1.0, 0.001, "ColorAndOpacity.R")
                    self.check_almost_equal(co.g, 1.0, 0.001, "ColorAndOpacity.G")
                    self.check_almost_equal(co.b, 1.0, 0.001, "ColorAndOpacity.B")
                    self.check_almost_equal(co.a, 1.0, 0.001, "ColorAndOpacity.A")

                # BackgroundColor = (0.2, 0.4, 0.8, 1.0)
                bg = button.get_editor_property("BackgroundColor")
                self.check_not_none(bg, "BackgroundColor set")
                if bg:
                    self.check_almost_equal(bg.r, 0.2, 0.001, "BackgroundColor.R")
                    self.check_almost_equal(bg.g, 0.4, 0.001, "BackgroundColor.G")
                    self.check_almost_equal(bg.b, 0.8, 0.001, "BackgroundColor.B")
                    self.check_almost_equal(bg.a, 1.0, 0.001, "BackgroundColor.A")

                # ClickMethod = DownAndUp
                cm = button.get_editor_property("ClickMethod")
                self.check_not_none(cm, "ClickMethod set")
                if cm is not None:
                    self.check_equal(cm, unreal.ButtonClickMethod.DOWN_AND_UP, f"ClickMethod is DownAndUp, got {cm}")

                # TouchMethod = DownAndUp
                tm = button.get_editor_property("TouchMethod")
                self.check_not_none(tm, "TouchMethod set")
                if tm is not None:
                    self.check_equal(tm, unreal.ButtonTouchMethod.DOWN_AND_UP, f"TouchMethod is DownAndUp, got {tm}")

                # PressMethod = DownAndUp
                pm = button.get_editor_property("PressMethod")
                self.check_not_none(pm, "PressMethod set")
                if pm is not None:
                    self.check_equal(pm, unreal.ButtonPressMethod.DOWN_AND_UP, f"PressMethod is DownAndUp, got {pm}")

                # IsFocusable = True
                self.check_equal(button.get_editor_property("IsFocusable"), True, "IsFocusable")

                # RenderOpacity = 1.0
                self.check_almost_equal(button.get_editor_property("RenderOpacity"), 1.0, 0.001, "RenderOpacity value")

                # Events (cannot verify bindings easily, just check property resolved)
                # OnClicked, OnPressed, OnReleased, OnHovered, OnUnhovered

                # Slot on CanvasPanel
                self.check_not_none(button.slot, "CanvasPanelSlot exists")

            button_label = self.find_widget("ButtonLabel")
            self.check_not_none(button_label, "ButtonLabel child found")
            if button_label:
                self.check_equal(button_label.get_editor_property("Text"), "Click Me", "ButtonLabel.Text")

            self.report()
        finally:
            if widget_markup.Application.is_test_mode():
                widget_markup.Application.request_shutdown()
