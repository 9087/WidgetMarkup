import unreal
import widget_markup
from Tests.TestComponent import TestComponent


class TestDynamicChild(TestComponent):
    def __init__(self):
        try:
            super().__init__("TestDynamicChild")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            # Static XML child: TestChild embedded as WidgetMarkup widget.
            existing_child = self.find_widget("ExistingChild")
            self.check_not_none(existing_child, "ExistingChild found (static TestChild)")

            # get_child on static child (TestChild has a Python component).
            static_comp = self.get_child("ExistingChild")
            self.check_not_none(static_comp, "get_child returns component for static TestChild")

            # Dynamic add_child via C++ API with class token string.
            child_name = widget_markup.WidgetLibrary.add_child_widget(
                uw, "RootCanvas", "TextBlock", "DynTextBlock")
            self.check_not_none(child_name, "add_child_widget returns child name")
            self.check_true(isinstance(child_name, str), "returned value is string")

            child_widget = self.find_widget(child_name)
            self.check_not_none(child_widget, "found DynTextBlock in WidgetTree")
            self.check_true(isinstance(child_widget, unreal.TextBlock), "DynTextBlock is TextBlock")

            # Dynamic remove_child
            result = self.remove_child("DynTextBlock")
            self.check_true(result, "remove_child(DynTextBlock) returned True")
            gone = self.find_widget("DynTextBlock")
            self.check_true(gone is None, "DynTextBlock gone after remove_child")

            # Python add_child with string path.
            new_comp = self.add_child("PyChild", "/Script/UMG.TextBlock", "RootCanvas")
            self.check_not_none(new_comp is None, "add_child returns None for plain UMG widget (no component)")
            self.remove_child("PyChild")  # cleanup

            # Python add_child returns component for WidgetMarkup widget.
            new_comp = self.add_child("WmChild", "/WidgetMarkup/Tests/TestChild", "RootCanvas")
            self.check_not_none(new_comp, "add_child returns component for WidgetMarkup widget")
            self.remove_child("WmChild")  # cleanup

            self.report()
        finally:
            if widget_markup.Application.is_test_mode():
                widget_markup.Application.request_shutdown()
