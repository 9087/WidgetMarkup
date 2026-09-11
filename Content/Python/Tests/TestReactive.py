import unreal
import widget_markup
from WidgetMarkupComponent import reactive
from Tests.TestComponent import TestComponent


class TestReactive(TestComponent):
    def __init__(self):
        try:
            super().__init__( "TestReactive")

            # --- reactive defaults ---
            t = _ReactiveTarget()
            self.check_equal(t.a, 0, "reactive default: a == 0")
            self.check_equal(t.b, "hello", "reactive default: b == 'hello'")
            self.check_true(t.c, "reactive default: c is True")
            self.check_equal(t.d, 3.14, "reactive default: d == 3.14")

            # --- reactive setter ---
            t.a = 42
            self.check_equal(t.a, 42, "reactive setter: a == 42")
            t.b = "world"
            self.check_equal(t.b, "world", "reactive setter: b == 'world'")
            t.c = False
            self.check_equal(t.c, False, "reactive setter: c is False")

            # --- reactive notification ---
            t.a = 100
            self.check_equal(t.last_changed_name, "a", "on_property_changed received name='a'")
            self.check_equal(t.last_changed_value, 100, "on_property_changed received value=100")

            # --- reactive None default ---
            self.check_true(t.e is None, "reactive default: e is None")
            t.e = "not none"
            self.check_equal(t.e, "not none", "reactive setter: e == 'not none'")
            t.e = None
            self.check_true(t.e is None, "reactive setter: e back to None")

            # --- reactive list auto-wrap to ObservableCollection ---
            self.check_equal(len(t.f), 0, "reactive list default: empty")
            t.f.append("X")
            self.check_equal(len(t.f), 1, "reactive list: after append length == 1")
            self.check_equal(t.f[0], "X", "reactive list: f[0] == 'X'")
            t.f.append("Y")
            t.f.append("Z")
            self.check_equal(len(t.f), 3, "reactive list: length == 3")
            t.f.pop(1)
            self.check_equal(t.f[1], "Z", "reactive list: after pop(1) f[1] == 'Z'")
            t.f.clear()
            self.check_equal(len(t.f), 0, "reactive list: after clear length == 0")

            self.report()
        finally:
            if widget_markup.Application.is_test_mode():
                widget_markup.Application.request_shutdown()


class _ReactiveTarget(TestComponent):
    @reactive
    def a(self): return 0

    @reactive
    def b(self): return "hello"

    @reactive
    def c(self): return True

    @reactive
    def d(self): return 3.14

    @reactive
    def e(self): return None

    @reactive
    def f(self): return []

    def __init__(self):
        self.last_changed_name = None
        self.last_changed_value = None

    def on_property_changed(self, name, value):
        self.last_changed_name = name
        self.last_changed_value = value
