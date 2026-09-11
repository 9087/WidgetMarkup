import unreal
import widget_markup
from WidgetMarkupComponent import reactive, computed
from Tests.TestComponent import TestComponent


class TestComputed(TestComponent):
    def __init__(self):
        try:
            super().__init__( "TestComputed")

            c = _ComputedTarget()

            # --- computed reads from reactive ---
            self.check_equal(c.total, 0, "total: 0+0 == 0")

            # --- computed updates when dependency changes ---
            c.a = 5
            self.check_equal(c.total, 5, "total: 5+0 == 5")
            c.b = 3
            self.check_equal(c.total, 8, "total: 5+3 == 8")

            # --- computed dependency chain ---
            self.check_equal(c.doubled, 16, "doubled: total*2 == 16")

            # --- computed 3-level chain: a→total→doubled→tripled ---
            self.check_equal(c.tripled, 48, "tripled: doubled*3 == 48")

            # --- computed read-only ---
            try:
                c.total = 999
                self.check_true(False, "computed setter should have raised")
            except AttributeError:
                self.check_true(True, "computed setter raises AttributeError")

            # --- computed notification ---
            c.b = 10
            self.check_equal(c.total, 15, "total: after b=10, 5+10=15")
            self.check_equal(c.doubled, 30, "doubled: (5+10)*2=30")
            self.check_equal(c.tripled, 90, "tripled: (5+10)*6=90")
            self.check_true("total" in c.changed_names, "total change notified")
            self.check_true("doubled" in c.changed_names, "doubled change notified")
            self.check_true("tripled" in c.changed_names, "tripled change notified")

            self.report()
        finally:
            if widget_markup.Application.is_test_mode():
                widget_markup.Application.request_shutdown()


class _ComputedTarget(TestComponent):
    @reactive
    def a(self): return 0

    @reactive
    def b(self): return 0

    @computed
    def total(self):
        return self.a + self.b

    @computed
    def doubled(self):
        return self.total * 2

    @computed
    def tripled(self):
        return self.doubled * 3

    def __init__(self):
        self.changed_names = []

    def on_property_changed(self, name, value):
        self.changed_names.append(name)
