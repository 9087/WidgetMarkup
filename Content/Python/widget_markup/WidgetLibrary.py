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

    # --- Event Reply helpers (mirrors UWidgetBlueprintLibrary) ---

    @staticmethod
    def handled():
        """Return a handled FWidgetMarkupEventReply."""
        unreal_mod = _get_unreal()
        reply = unreal_mod.WidgetMarkupEventReply()
        reply.set_editor_property("bIsHandled", True)
        return reply

    @staticmethod
    def unhandled():
        """Return an unhandled FWidgetMarkupEventReply."""
        unreal_mod = _get_unreal()
        return unreal_mod.WidgetMarkupEventReply()

    @staticmethod
    def capture_mouse(reply, widget):
        """Add mouse capture to the reply (sets MouseCaptor)."""
        reply.set_editor_property("MouseCaptor", widget)

    @staticmethod
    def release_mouse_capture(reply):
        """Request mouse capture release."""
        reply.set_editor_property("bReleaseMouseCapture", True)

    @staticmethod
    def lock_mouse_to_widget(reply, widget):
        """Lock the mouse to the given widget."""
        reply.set_editor_property("MouseLock", widget)

    @staticmethod
    def set_user_focus(reply, widget):
        """Set keyboard focus to the given widget."""
        reply.set_editor_property("FocusRecipient", widget)

    @staticmethod
    def set_mouse_position(reply, position):
        """Request the cursor be moved to the given position."""
        reply.set_editor_property("bShouldSetMousePos", True)
        reply.set_editor_property("RequestedMousePos", position)

    @staticmethod
    def is_under_location(geometry, screen_position):
        """Check whether screen_position is inside the geometry rect."""
        pos = geometry.get_editor_property("AbsolutePosition")
        size = geometry.get_editor_property("Size")
        return (pos.x <= screen_position.x <= pos.x + size.x and
                pos.y <= screen_position.y <= pos.y + size.y)

    @staticmethod
    def get_screen_space_position(mouse_event):
        """Read ScreenSpacePosition from a WidgetMarkupPointerEvent."""
        return mouse_event.get_editor_property("ScreenSpacePosition")
