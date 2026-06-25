"""Wrapper for _widget_markup.DataBinding with UObject parameters."""

import sys as _sys

_wm = _sys.modules["_widget_markup"]
_DataBinding = _wm.DataBinding


class DataBinding:
    """Wrapper for _widget_markup.DataBinding with UObject parameters."""

    @staticmethod
    def apply_property_binding(user_widget, binding, value):
        """Apply a WidgetMarkup property binding with a Python value."""
        return _DataBinding.apply_property_binding(
            str(user_widget.get_path_name()),
            binding.source_expression,
            str(binding.target_object_name),
            str(binding.target_property_path),
            value,
        )
