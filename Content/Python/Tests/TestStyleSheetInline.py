import unreal
import widget_markup
from Tests.TestComponent import TestComponent


class TestStyleSheetInline(TestComponent):
    def __init__(self):
        try:
            super().__init__( "TestStyleSheetInline")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            inline = self.find_widget("InlineStyled")
            self.check_not_none(inline, "InlineStyled TextBlock found")
            if inline:
                self.check_equal(inline.get_editor_property("Text"), "inline styled text", "InlineStyled.Text")
                # Font.Size set via a Basic child element (<Float>24</Float>)
                font = inline.get_editor_property("Font")
                self.check_not_none(font, "InlineStyled.Font")
                if font:
                    self.check_equal(font.get_editor_property("Size"), 24, "InlineStyled.Font.Size (child element)")

            card = self.find_widget("CardBorder")
            self.check_not_none(card, "CardBorder found")
            if card:
                # BrushColor set via a struct child element (<LinearColor .../>)
                color = card.get_editor_property("BrushColor")
                self.check_not_none(color, "CardBorder.BrushColor")
                if color:
                    self.check_almost_equal(float(color.r), 0.3, 0.001, "CardBorder.BrushColor.R")
                    self.check_almost_equal(float(color.g), 0.2, 0.001, "CardBorder.BrushColor.G")
                    self.check_almost_equal(float(color.b), 0.1, 0.001, "CardBorder.BrushColor.B")
                    self.check_almost_equal(float(color.a), 1.0, 0.001, "CardBorder.BrushColor.A")

            grid = self.find_widget("StyledGrid")
            self.check_not_none(grid, "StyledGrid found")
            if grid:
                # ColumnFill set via multiple Basic child elements (container setter)
                fill = grid.get_editor_property("ColumnFill")
                self.check_equal(fill, [1.0, 2.0], "StyledGrid.ColumnFill (container child elements)")

            slot_label = self.find_widget("SlotStyled")
            self.check_not_none(slot_label, "SlotStyled found")
            if slot_label:
                # Slot.Padding resolves through an object pointer (unresolvable at compile time);
                # the setter must be ignored with a warning, so Offsets stays at its default.
                self.check_equal(slot_label.get_editor_property("Text"), "slot styled text", "SlotStyled.Text")
                slot = slot_label.slot
                self.check_not_none(slot, "SlotStyled.Slot")
                if slot:
                    layout = slot.get_editor_property("LayoutData")
                    self.check_not_none(layout, "SlotStyled.Slot.LayoutData")
                    if layout:
                        offsets = layout.offsets
                        self.check_almost_equal(float(offsets.left), 0.0, 0.001, "SlotStyled.Slot.LayoutData.Offsets.Left (unresolvable setter ignored)")

            self.report()
        finally:
            if widget_markup.Application.get_extra_arguments() == "test":
                widget_markup.Application.request_shutdown()
