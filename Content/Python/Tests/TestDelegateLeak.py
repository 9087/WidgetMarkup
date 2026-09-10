import widget_markup
from Tests.TestComponent import TestComponent


class TestDelegateLeak(TestComponent):
    """Verify remove_child breaks the component <-> widget reference cycle.

    A WidgetMarkup child component holds its widget through
    _widget_markup_user_widget, while the widget's extension holds the component
    back. If remove_child does not clear that reference, the widget stays
    reachable through the Python reference collector and leaks on every
    add_child/remove_child cycle.
    """

    def __init__(self):
        try:
            super().__init__("TestDelegateLeak")
            uw = getattr(self, "_widget_markup_user_widget", None)
            self.check_not_none(uw, "user widget loaded")

            cycles = 20
            cleared = 0
            for i in range(cycles):
                name = f"LeakChild_{i}"
                comp = self.add_child(name, "/WidgetMarkup/Tests/TestChild", "RootCanvas")
                if comp is None:
                    self.check_true(False, f"add_child {name} failed")
                    continue

                had_ref = hasattr(comp, "_widget_markup_user_widget")
                self.remove_child(name)
                still_has_ref = hasattr(comp, "_widget_markup_user_widget")

                if had_ref and not still_has_ref:
                    cleared += 1
                else:
                    self.check_true(False,
                                    f"remove_child did not clear {name} user widget ref (before={had_ref}, after={still_has_ref})")

            self.check_true(cleared == cycles,
                            f"cleared user widget ref on all {cycles} children (cleared={cleared})")

            self.report()
        finally:
            if widget_markup.Application.get_extra_arguments() == "test":
                widget_markup.Application.request_shutdown()
