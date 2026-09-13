from __future__ import annotations

import unreal
import widget_markup
from WidgetMarkupComponent import WidgetMarkupComponent

# Process exit code asserted by RunTests.bat when any check fails.
EXIT_CODE_FAILURE = 1


class TestComponent(WidgetMarkupComponent):
    """Base class for WidgetMarkup test components with built-in check helpers."""

    def __init__(self, test_name: str = "Test"):
        super().__init__()
        self._test_name = test_name
        self._pass_count = 0
        self._fail_count = 0
        self._deferred_phases = []
        self._deferred_handle = None

    def defer_checks(self, callback, ticks: int = 3) -> None:
        """Queue a check phase that runs `ticks` frames later.

        Everything in __init__ runs before the Slate widget is built (the Python
        component is created before TakeWidget), so checks that depend on the built
        widget - or on a reactive update applied to a built widget - belong in a
        deferred phase. Phases run in the order they were queued; once the queue is
        empty the test reports and shuts down.
        """
        self._deferred_phases.append((callback, ticks))
        if self._deferred_handle is None:
            self._deferred_handle = unreal.register_slate_post_tick_callback(self._on_deferred_tick)

    def shutdown_if_test_mode(self) -> None:
        """Shut down after __init__, unless queued phases report on their own."""
        if self._deferred_phases or self._deferred_handle is not None:
            return
        if widget_markup.Application.is_test_mode():
            widget_markup.Application.request_shutdown()

    def _on_deferred_tick(self, delta_seconds) -> None:
        if not self._deferred_phases:
            self._finish_deferred()
            return

        callback, ticks_left = self._deferred_phases[0]
        if ticks_left > 0:
            self._deferred_phases[0] = (callback, ticks_left - 1)
            return

        self._deferred_phases.pop(0)
        callback()
        if not self._deferred_phases:
            self._finish_deferred()

    def _finish_deferred(self) -> None:
        if self._deferred_handle is not None:
            unreal.unregister_slate_post_tick_callback(self._deferred_handle)
            self._deferred_handle = None
        self.report()
        if widget_markup.Application.is_test_mode():
            widget_markup.Application.request_shutdown()

    def _record_failure(self) -> None:
        """Count a failed check and mark the process exit code as failed."""
        self._fail_count += 1
        widget_markup.Application.set_exit_code(EXIT_CODE_FAILURE)

    def check_equal(self, actual, expected, msg: str = "") -> None:
        try:
            if actual == expected:
                self._pass_count += 1
                unreal.log(f"[PASS] {self._test_name}: {msg or f'{actual!r} == {expected!r}'}")
            else:
                self._record_failure()
                unreal.log_error(f"[FAIL] {self._test_name}: {msg}: expected {expected!r}, got {actual!r}")
        except Exception as e:
            self._record_failure()
            unreal.log_error(f"[FAIL] {self._test_name}: {msg}: exception: {e}")

    def check_almost_equal(self, actual: float, expected: float, delta: float = 1e-6, msg: str = "") -> None:
        try:
            if abs(actual - expected) <= delta:
                self._pass_count += 1
                unreal.log(f"[PASS] {self._test_name}: {msg or f'{actual} ~= {expected}'}")
            else:
                self._record_failure()
                unreal.log_error(f"[FAIL] {self._test_name}: {msg}: expected ~{expected} (delta={delta}), got {actual}")
        except Exception as e:
            self._record_failure()
            unreal.log_error(f"[FAIL] {self._test_name}: {msg}: exception: {e}")

    def check_true(self, condition: bool, msg: str = "") -> None:
        try:
            if condition:
                self._pass_count += 1
                unreal.log(f"[PASS] {self._test_name}: {msg or 'condition is True'}")
            else:
                self._record_failure()
                unreal.log_error(f"[FAIL] {self._test_name}: {msg or 'condition is False'}")
        except Exception as e:
            self._record_failure()
            unreal.log_error(f"[FAIL] {self._test_name}: {msg}: exception: {e}")

    def check_not_none(self, value, msg: str = "") -> None:
        try:
            if value is not None:
                self._pass_count += 1
                unreal.log(f"[PASS] {self._test_name}: {msg or f'{value!r} is not None'}")
            else:
                self._record_failure()
                unreal.log_error(f"[FAIL] {self._test_name}: {msg or 'value is None'}")
        except Exception as e:
            self._record_failure()
            unreal.log_error(f"[FAIL] {self._test_name}: {msg}: exception: {e}")

    def report(self) -> None:
        total = self._pass_count + self._fail_count
        if total == 0:
            widget_markup.Application.set_exit_code(EXIT_CODE_FAILURE)
            unreal.log_warning(f"[{self._test_name}] no checks executed.")
        elif self._fail_count == 0:
            unreal.log_warning(f"[{self._test_name}] ALL {total} CHECKS PASSED.")
        else:
            unreal.log_error(f"[{self._test_name}] {self._fail_count}/{total} CHECKS FAILED.")
