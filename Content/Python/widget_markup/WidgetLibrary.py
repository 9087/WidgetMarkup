"""Wrapper for _widget_markup.WidgetLibrary with UObject parameters and return values."""

import sys as _sys

_wm = _sys.modules["_widget_markup"]
_WidgetLibrary = _wm.WidgetLibrary


def _get_unreal():
    return _sys.modules.get("unreal")


class WidgetLibrary:
    """Wrapper for _widget_markup.WidgetLibrary with UObject parameters and return values."""

    @staticmethod
    def find_widget_in_user_widget(user_widget, widget_name: str):
        """Find a widget by name, returning the UWidget or None."""
        path = _WidgetLibrary.find_widget_in_user_widget(
            str(user_widget.get_path_name()), widget_name
        )
        if path:
            unreal = _get_unreal()
            return unreal.find_object(None, path) if unreal else path
        return None

    @staticmethod
    def get_python_object_from_list_item(entry):
        """Get the raw Python value from a list item entry."""
        return _WidgetLibrary.get_python_object_from_list_item(
            str(entry.get_path_name())
        )

    @staticmethod
    def add_child_widget(user_widget, parent_name: str, class_token: str, name: str):
        """Create a child widget and add it to a parent panel."""
        return _WidgetLibrary.add_child_widget(
            str(user_widget.get_path_name()), parent_name, class_token, name
        )

    @staticmethod
    def remove_child_widget(user_widget, child):
        """Remove a child widget by name, path, or UWidget reference."""
        if hasattr(child, "get_path_name"):
            child = str(child.get_path_name())
        return _WidgetLibrary.remove_child_widget(
            str(user_widget.get_path_name()), child
        )
