from __future__ import annotations

from typing import Any

import widget_markup
from IUserListEntry import IUserListEntry


class IUserObjectListEntry(IUserListEntry):
    """Mixin adding object-list-item support on top of IUserListEntry.

    Provides get_list_item() which returns the raw Python data object.
    on_data_refresh is defined in WidgetMarkupComponent; override it there.
    """

    def get_list_item(self) -> Any:
        """Return the raw Python value this entry represents in the owning ListView."""
        entry = self._widget_markup_user_widget.get_entry_list_item()
        if entry is not None:
            return widget_markup.WidgetLibrary.get_python_object_from_list_item(entry)
        return None
