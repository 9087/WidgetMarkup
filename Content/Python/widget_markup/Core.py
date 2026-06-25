"""Wrapper for _widget_markup.Core with UObject parameters and return values."""

import sys as _sys

_wm = _sys.modules["_widget_markup"]
_Core = _wm.Core


def _get_unreal():
    return _sys.modules.get("unreal")


class Core:
    """Wrapper for _widget_markup.Core with UObject parameters and return values."""

    @staticmethod
    def resolve_class(token: str):
        """Resolve a class token, returning a UClass object or None."""
        path = _Core.resolve_class(token)
        if path:
            unreal = _get_unreal()
            return unreal.find_object(None, path) if unreal else path
        return None

    @staticmethod
    def get_component_by_widget(widget):
        """Get the Python WidgetMarkupComponent for a UWidget, or None."""
        return _Core.get_component_by_widget(str(widget.get_path_name()))
