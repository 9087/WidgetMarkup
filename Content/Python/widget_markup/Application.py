"""Wrapper for _widget_markup.Application (no PathName conversion needed)."""

import sys as _sys

_wm = _sys.modules["_widget_markup"]
_Application = _wm.Application


class Application:
    """Wrapper for _widget_markup.Application (no PathName conversion needed)."""

    @staticmethod
    def get_extra_arguments() -> str:
        """Get the current WidgetMarkupApp extra arguments string."""
        return _Application.get_extra_arguments()

    @staticmethod
    def is_test_mode() -> bool:
        """True when the app was launched with the 'test' extra argument token."""
        return _Application.is_test_mode()

    @staticmethod
    def request_shutdown():
        """Request engine exit (for standalone programs)."""
        return _Application.request_shutdown()

    @staticmethod
    def set_exit_code(code: int) -> None:
        """Set the process exit code (for standalone programs and test runs)."""
        return _Application.set_exit_code(code)
