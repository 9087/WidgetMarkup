import unreal
import widget_markup
from Tests.TestComponent import TestComponent


class TestStarshipStyle(TestComponent):
    """Verify the framework style sheets (editor layer on top of core layer)."""

    @staticmethod
    def _font_size(widget):
        font = widget.get_editor_property("Font")
        return font.get_editor_property("Size") if font else None

    @staticmethod
    def _paddings(button):
        style = button.get_editor_property("WidgetStyle")
        normal = style.get_editor_property("NormalPadding")
        pressed = style.get_editor_property("PressedPadding")
        return (
            (normal.left, normal.top, normal.right, normal.bottom),
            (pressed.left, pressed.top, pressed.right, pressed.bottom),
        )

    def __init__(self):
        try:
            super().__init__("TestStarshipStyle")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            # --- text: editor layer entries -----------------------------------
            # The test sheet overrides NormalText.Important locally with
            # Font.Size=18 only; the editor layer's ShadowOffset must survive.
            important = self.find_widget("ImportantText")
            self.check_not_none(important, "ImportantText found")
            if important:
                self.check_equal(self._font_size(important), 18, "NormalText.Important Font.Size (local override)")
                offset = important.get_editor_property("ShadowOffset")
                self.check_equal((offset.x, offset.y), (1.0, 1.0), "NormalText.Important ShadowOffset (from editor layer)")

            for name, size in (("SmallTextBlock", 8), ("LargeTextBlock", 11), ("TinyTextBlock", 8)):
                widget = self.find_widget(name)
                self.check_not_none(widget, f"{name} found")
                if widget:
                    self.check_equal(self._font_size(widget), size, f"{name} Font.Size")

            warning = self.find_widget("WarningText")
            self.check_not_none(warning, "TextBlock.ShadowedTextWarning found")
            if warning:
                offset = warning.get_editor_property("ShadowOffset")
                self.check_equal((offset.x, offset.y), (1.0, 1.0), "ShadowedTextWarning ShadowOffset")

            # --- text: core layer entries reachable through the editor layer ---
            plain = self.find_widget("PlainText")
            self.check_not_none(plain, "PlainText found")
            if plain:
                self.check_equal(self._font_size(plain), 10, "implicit TextBlock Font.Size (core layer)")

            # --- buttons ------------------------------------------------------
            for name in ("PrimaryButton", "FlatButton", "FlatDangerButton", "SimpleSharpButton", "ToggleButton"):
                button = self.find_widget(name)
                self.check_not_none(button, f"{name} found")

            # FlatButton.Danger is derived via Base="FlatButton": its own tint
            # applies, while padding still comes from the core layer's implicit
            # Button entry -> proves the two sheets compose.
            danger = self.find_widget("FlatDangerButton")
            if danger:
                _, pressed = self._paddings(danger)
                self.check_equal(pressed, (12.0, 2.5, 12.0, 0.5), "FlatButton.Danger PressedPadding (core implicit Button)")
                normal_brush = danger.get_editor_property("WidgetStyle").get_editor_property("Normal")
                tint = normal_brush.get_editor_property("TintColor").get_editor_property("SpecifiedColor")
                self.check_almost_equal(float(tint.r), 0.5609, 0.002, "FlatButton.Danger Normal tint R")
                # The derived style must win over its Base: FlatButton sets
                # Normal.DrawAs = NoDrawType, FlatButton.Danger sets Box (the engine
                # draws these buttons with a nine-slice texture, not a rounded box).
                # Without this check a Base that overrides the derived style
                # (wrong layering order) only shows up in a screenshot.
                draw_as = normal_brush.get_editor_property("DrawAs")
                self.check_equal(draw_as, unreal.SlateBrushDrawType.BOX,
                                 f"FlatButton.Danger Normal.DrawAs (derived overrides Base), got {draw_as}")

            # SimpleSharpButton has its own tight padding.
            sharp = self.find_widget("SimpleSharpButton")
            if sharp:
                normal, pressed = self._paddings(sharp)
                self.check_equal(normal, (0.0, 0.0, 0.0, 1.0), "SimpleSharpButton NormalPadding")
                self.check_equal(pressed, (0.0, 1.0, 0.0, 0.0), "SimpleSharpButton PressedPadding")

            # --- Base= composition probes -------------------------------------
            # (widget, expected NormalPadding, expected PressedPadding or None)
            probes = [
                ("BaseProbeButton", (11.0, 12.0, 13.0, 14.0), (21.0, 22.0, 23.0, 24.0)),
                ("DerivedProbeButton", (11.0, 12.0, 13.0, 14.0), (31.0, 32.0, 33.0, 34.0)),
                ("ChainedProbeButton", (41.0, 42.0, 43.0, 44.0), (31.0, 32.0, 33.0, 34.0)),
                ("MissingBaseProbeButton", (51.0, 52.0, 53.0, 54.0), None),
            ]
            for name, want_normal, want_pressed in probes:
                button = self.find_widget(name)
                self.check_not_none(button, f"{name} found")
                if not button:
                    continue
                normal, pressed = self._paddings(button)
                self.check_equal(normal, want_normal, f"{name} NormalPadding (Base= composition)")
                if want_pressed is not None:
                    self.check_equal(pressed, want_pressed, f"{name} PressedPadding")

            # --- remaining styled controls ------------------------------------
            for name in ("CheckBoxA", "ToggleLookCheckBox", "InputBox", "SliderA",
                         "ProgressA", "ComboA", "ToolbarCombo", "SpinA"):
                widget = self.find_widget(name)
                self.check_not_none(widget, f"{name} found")

            self.report()
        finally:
            if widget_markup.Application.is_test_mode():
                uw = getattr(self, "_widget_markup_user_widget", None)
                if uw is not None and uw.get_parent() is None:
                    widget_markup.Application.request_shutdown()
