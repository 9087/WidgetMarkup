import unreal
import widget_markup
from Tests.TestComponent import TestComponent


class TestVariable(TestComponent):
    """Verify that Variable elements work correctly, including:
    - Scalar default values (String, Boolean, Integer, Float)
    - Struct defaults via channel A string format (comma-separated) and child elements
    - Enum and Text defaults
    - Container defaults (Array/Set/Map) via child elements (route A)
    - Brace-containing literal values NOT consumed as bindings"""

    def __init__(self):
        try:
            super().__init__("TestVariable")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            # --- Normal scalar default values ---
            self.check_equal(
                uw.get_editor_property("PlayerName"), "Unknown",
                "PlayerName default = 'Unknown'")
            self.check_equal(
                uw.get_editor_property("IsAlive"), True,
                "IsAlive default = True")
            self.check_equal(
                uw.get_editor_property("Score"), 0,
                "Score default = 0")
            self.check_equal(
                uw.get_editor_property("Health"), 100.0,
                "Health default = 100.0")

            # --- Struct default via channel A string format ---
            scale = uw.get_editor_property("Scale")
            self.check_not_none(scale, "Scale variable exists")
            self.check_almost_equal(
                float(scale.x), 0.5, msg="Scale.x = 0.5")
            self.check_almost_equal(
                float(scale.y), 0.5, msg="Scale.y = 0.5")

            tint = uw.get_editor_property("Tint")
            self.check_not_none(tint, "Tint variable exists")
            self.check_almost_equal(
                float(tint.r), 1.0, msg="Tint.r = 1.0")
            self.check_almost_equal(
                float(tint.g), 0.15, msg="Tint.g = 0.15")
            self.check_almost_equal(
                float(tint.b), 0.15, msg="Tint.b = 0.15")
            self.check_almost_equal(
                float(tint.a), 1.0, msg="Tint.a = 1.0")

            # --- Struct default via child elements ---
            offset = uw.get_editor_property("Offset")
            self.check_not_none(offset, "Offset variable exists")
            self.check_almost_equal(
                float(offset.x), 1.0, msg="Offset.x = 1.0")
            self.check_almost_equal(
                float(offset.y), 2.0, msg="Offset.y = 2.0")

            # --- Enum default (channel A enum name) ---
            self.check_equal(
                uw.get_editor_property("HAlign"),
                unreal.HorizontalAlignment.H_ALIGN_CENTER,
                "HAlign default = HAlign_Center")

            # --- Text default ---
            self.check_equal(
                str(uw.get_editor_property("Title")), "Hello",
                "Title default = 'Hello'")

            # --- Container defaults via child elements (route A) ---
            self.check_equal(
                uw.get_editor_property("Weights"), [1.0, 2.0, 3.0],
                "Weights default = [1.0, 2.0, 3.0]")

            # --- Struct elements inside a container ---
            points = uw.get_editor_property("Points")
            self.check_equal(len(points), 2, "Points has 2 elements")
            self.check_almost_equal(
                float(points[0].x), 1.0, msg="Points[0].x = 1.0")
            self.check_almost_equal(
                float(points[0].y), 2.0, msg="Points[0].y = 2.0")
            self.check_almost_equal(
                float(points[1].x), 3.0, msg="Points[1].x = 3.0")
            self.check_almost_equal(
                float(points[1].y), 4.0, msg="Points[1].y = 4.0")
            self.check_equal(
                uw.get_editor_property("Tags"), {"red", "blue"},
                "Tags default = {'red', 'blue'}")
            self.check_equal(
                uw.get_editor_property("Scores"), {"a": 1, "b": 2},
                "Scores default = {'a': 1, 'b': 2}")

            # --- Brace-containing values: MUST be literal strings ---
            self.check_equal(
                uw.get_editor_property("LiteralBraces"), "{some text}",
                "LiteralBraces = '{some text}' (NOT consumed as binding)")
            self.check_equal(
                uw.get_editor_property("LooksLikeBinding"), "{x.y.z}",
                "LooksLikeBinding = '{x.y.z}' (NOT consumed as binding)")
            self.check_equal(
                uw.get_editor_property("BracesInMiddle"), "before {x} after",
                "BracesInMiddle = 'before {x} after'")
            self.check_equal(
                uw.get_editor_property("SingleBrace"), "only open {",
                "SingleBrace = 'only open {'")
            self.check_equal(
                uw.get_editor_property("DoubleBraces"), "{{nested}}",
                "DoubleBraces = '{{nested}}'")

            # --- Empty default ---
            self.check_equal(
                uw.get_editor_property("EmptyDefault"), "",
                "EmptyDefault is empty string")

            self.report()
        finally:
            if widget_markup.Application.get_extra_arguments() == "test":
                widget_markup.Application.request_shutdown()
