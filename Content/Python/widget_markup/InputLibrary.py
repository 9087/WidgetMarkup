"""Thin wrapper around unreal.WidgetMarkupInputLibrary for FWidgetMarkupPointerEvent.

All functions accept and return FKey directly — no Python-side Key wrapper needed.
"""

import unreal


class InputLibrary:
    """Static helper class for FWidgetMarkupPointerEvent input queries."""

    @staticmethod
    def pointer_event_is_mouse_button_down(
        mouse_event: unreal.WidgetMarkupPointerEvent, key: unreal.Key
    ) -> bool:
        return unreal.WidgetMarkupInputLibrary.pointer_event_is_mouse_button_down(mouse_event, key)

    @staticmethod
    def pointer_event_get_effecting_button(
        mouse_event: unreal.WidgetMarkupPointerEvent,
    ) -> unreal.Key:
        return unreal.WidgetMarkupInputLibrary.pointer_event_get_effecting_button(mouse_event)

    @staticmethod
    def pointer_event_get_pressed_buttons(
        mouse_event: unreal.WidgetMarkupPointerEvent,
    ) -> list[unreal.Key]:
        return unreal.WidgetMarkupInputLibrary.pointer_event_get_pressed_buttons(mouse_event)
