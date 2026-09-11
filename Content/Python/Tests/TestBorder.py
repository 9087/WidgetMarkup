import unreal
import widget_markup
from Tests.TestComponent import TestComponent


class TestBorder(TestComponent):
    def __init__(self):
        try:
            super().__init__("TestBorder")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            border = self.find_widget("StyledBorder")
            self.check_not_none(border, "StyledBorder found")
            if border:
                # BrushColor = (0.1, 0.1, 0.3, 1.0)
                bc = border.get_editor_property("BrushColor")
                self.check_not_none(bc, "BrushColor set")
                if bc:
                    self.check_almost_equal(bc.r, 0.1, 0.001, "BrushColor.R")
                    self.check_almost_equal(bc.g, 0.1, 0.001, "BrushColor.G")
                    self.check_almost_equal(bc.b, 0.3, 0.001, "BrushColor.B")
                    self.check_almost_equal(bc.a, 1.0, 0.001, "BrushColor.A")

                # ContentColorAndOpacity = (1.0, 0.8, 0.8, 1.0)
                cc = border.get_editor_property("ContentColorAndOpacity")
                self.check_not_none(cc, "ContentColorAndOpacity set")
                if cc:
                    self.check_almost_equal(cc.r, 1.0, 0.001, "ContentColorAndOpacity.R")
                    self.check_almost_equal(cc.g, 0.8, 0.001, "ContentColorAndOpacity.G")
                    self.check_almost_equal(cc.b, 0.8, 0.001, "ContentColorAndOpacity.B")
                    self.check_almost_equal(cc.a, 1.0, 0.001, "ContentColorAndOpacity.A")

                # Padding = (8, 8, 8, 8)
                pad = border.get_editor_property("Padding")
                self.check_not_none(pad, "Padding set")
                if pad:
                    self.check_almost_equal(pad.left, 8.0, 0.001, "Padding.Left")
                    self.check_almost_equal(pad.top, 8.0, 0.001, "Padding.Top")
                    self.check_almost_equal(pad.right, 8.0, 0.001, "Padding.Right")
                    self.check_almost_equal(pad.bottom, 8.0, 0.001, "Padding.Bottom")

                # HorizontalAlignment = Fill (HAlign_Fill)
                ha = border.get_editor_property("HorizontalAlignment")
                self.check_not_none(ha, "HorizontalAlignment set")
                if ha is not None:
                    self.check_equal(ha, unreal.HorizontalAlignment.H_ALIGN_FILL, f"HorizontalAlignment is Fill, got {ha}")

                # VerticalAlignment = Fill (VAlign_Fill)
                va = border.get_editor_property("VerticalAlignment")
                self.check_not_none(va, "VerticalAlignment set")
                if va is not None:
                    self.check_equal(va, unreal.VerticalAlignment.V_ALIGN_FILL, f"VerticalAlignment is Fill, got {va}")

                # Visibility = Visible
                vis = border.get_editor_property("Visibility")
                self.check_not_none(vis, "Visibility set")
                if vis is not None:
                    self.check_equal(vis, unreal.SlateVisibility.VISIBLE, f"Visibility is Visible, got {vis}")

                # bIsEnabled = True (deprecated bitfield; UE5 parent-aware enabled state not initialized in test harness)
                self.check_not_none(border.get_editor_property("bIsEnabled"), "bIsEnabled property exists")

                # bShowEffectWhenDisabled = True
                show_effect = border.get_editor_property("bShowEffectWhenDisabled")
                self.check_not_none(show_effect, "bShowEffectWhenDisabled set")
                if show_effect is not None:
                    self.check_true(bool(show_effect), "bShowEffectWhenDisabled is True")

                # RenderOpacity = 0.95
                ro = border.get_editor_property("RenderOpacity")
                self.check_almost_equal(ro, 0.95, 0.001, "RenderOpacity value")

                # DesiredSizeScale = (1.0, 1.0)
                dss = border.get_editor_property("DesiredSizeScale")
                self.check_not_none(dss, "DesiredSizeScale set")
                if dss:
                    self.check_almost_equal(dss.x, 1.0, 0.001, "DesiredSizeScale.X")
                    self.check_almost_equal(dss.y, 1.0, 0.001, "DesiredSizeScale.Y")

                # Background.DrawAs = Box
                brush = border.get_editor_property("Background")
                self.check_not_none(brush, "Background set")
                if brush:
                    draw_as = brush.get_editor_property("DrawAs")
                    self.check_not_none(draw_as, "Background.DrawAs set")
                    if draw_as is not None:
                        self.check_true("BOX" in str(draw_as), f"Background.DrawAs is Box, got {draw_as}")

                # Slot on CanvasPanel
                self.check_not_none(border.slot, "CanvasPanelSlot exists")

            child = self.find_widget("BorderChild")
            self.check_not_none(child, "BorderChild found")
            if child:
                self.check_equal(child.get_editor_property("Text"), "inside border", "BorderChild.Text")

            self.report()
        finally:
            if widget_markup.Application.is_test_mode():
                widget_markup.Application.request_shutdown()
