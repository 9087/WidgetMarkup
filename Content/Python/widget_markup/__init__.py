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
