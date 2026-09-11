import unreal
import widget_markup
from Tests.TestComponent import TestComponent


class TestStyleSheetOverride(TestComponent):
    def __init__(self):
        try:
            super().__init__( "TestStyleSheetOverride")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            # The inherited TestStyleSheetFile defines StyledText (blue, size 20).
            styled = self.find_widget("InheritedStyled")
            self.check_not_none(styled, "InheritedStyled found (style inherited from file)")

            unstyled = self.find_widget("UnstyledRef")
            self.check_not_none(unstyled, "UnstyledRef found")

            self.report()
        finally:
            if widget_markup.Application.is_test_mode():
                widget_markup.Application.request_shutdown()
