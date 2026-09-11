import unreal
import widget_markup
from Tests.TestComponent import TestComponent


class TestStaticChild(TestComponent):
    def __init__(self):
        try:
            super().__init__("TestStaticChild")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            # Static XML child: TestChild embedded as WidgetMarkup widget.
            child_widget = self.find_widget("StaticChild")
            self.check_not_none(child_widget, "StaticChild found in WidgetTree")

            # get_child should return the component since TestChild has a Script.
            child_comp = self.get_child("StaticChild")
            self.check_not_none(child_comp, "get_child returns component for static TestChild")

            # remove_child on the static child.
            result = self.remove_child("StaticChild")
            self.check_true(result, "remove_child(StaticChild) returned True")
            gone = self.find_widget("StaticChild")
            self.check_true(gone is None, "StaticChild gone after remove_child")

            self.report()
        finally:
            if widget_markup.Application.is_test_mode():
                widget_markup.Application.request_shutdown()
