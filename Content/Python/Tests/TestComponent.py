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
