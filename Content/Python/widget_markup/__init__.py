"""Wraps _widget_markup C types with UObject-friendly public API.

The C++ module is registered as _widget_markup.
This package provides the public widget_markup namespace with classes
that accept/return UObject instances while delegating PathName strings to C++.
"""

from .Core import Core
from .DataBinding import DataBinding
from .InputLibrary import InputLibrary
from .WidgetLibrary import WidgetLibrary
from .Application import Application

import sys as _sys

_wm = _sys.modules["_widget_markup"]

# Replace the C types on the _widget_markup module with wrapper classes.
_wm.Core = Core
_wm.DataBinding = DataBinding
_wm.WidgetLibrary = WidgetLibrary
_wm.Application = Application
