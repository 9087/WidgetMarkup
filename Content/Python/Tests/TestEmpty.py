import unreal
import widget_markup
from Tests.TestComponent import TestComponent


class TestEmpty(TestComponent):
    def __init__(self):
        try:
            super().__init__("TestEmpty")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget is not None")
            self.report()
        finally:
            # Only request shutdown if this is the root widget (not embedded).
            if widget_markup.Application.is_test_mode():
                uw = getattr(self, "_widget_markup_user_widget", None)
                if uw is not None and uw.get_parent() is None:
                    widget_markup.Application.request_shutdown()
