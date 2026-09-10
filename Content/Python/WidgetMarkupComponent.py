from __future__ import annotations

import importlib
import weakref
from typing import Any, Callable

from ObservableCollection import ObservableCollection

import re as _re
import unreal
import widget_markup

_MISSING = object()


def _camel_to_snake(name: str) -> str:
    """Convert CamelCase to snake_case. e.g. 'OnClicked' -> 'on_clicked'."""
    return _re.sub(r'(?<!^)(?=[A-Z])', '_', name).lower()

# Stack of (instance, computed_property_name) currently being evaluated.
# Used to auto-collect dependencies when a property is read inside a computed.
_compute_stack = []

_STATE_ATTR = "_widget_markup_reactive_state"
_USER_WIDGET_ATTR = "_widget_markup_user_widget"


class _ReactiveState:
    __slots__ = ("values", "cached", "dirty", "dependencies", "dependents", "evaluating")

    def __init__(self) -> None:
        # Settable property values: name -> value
        self.values: dict[str, Any] = {}
        # Computed property cached values: name -> value
        self.cached: dict[str, Any] = {}
        # Computed property names whose cache is invalid
        self.dirty: set[str] = set()
        # computed_name -> set of property names it depends on
        self.dependencies: dict[str, set[str]] = {}
        # property_name -> set of computed names that depend on it
        self.dependents: dict[str, set[str]] = {}
        # Computed property names currently being evaluated on this instance.
        self.evaluating: set[str] = set()


class _ReactiveProperty:
    def __init__(self, default_factory: Callable[[WidgetMarkupComponent], Any]) -> None:
        self._default_factory: Callable[[WidgetMarkupComponent], Any] = default_factory
        self._name: str | None = None

    def __set_name__(self, owner: type[WidgetMarkupComponent], name: str) -> None:
        self._name = name

    def __get__(self, instance: WidgetMarkupComponent | None, owner: type[WidgetMarkupComponent] | None = None) -> Any:
        if instance is None:
            return self
        _track_dependency(instance, self._name)
        state = _ensure_state(instance)
        if self._name in state.values:
            value = state.values[self._name]
            coerced_value = _coerce_reactive_value(instance, self._name, value)
            if coerced_value is not value:
                state.values[self._name] = coerced_value
            return coerced_value

        default_value = self._default_factory(instance)
        coerced_default = _coerce_reactive_value(instance, self._name, default_value)
        state.values[self._name] = coerced_default
        return coerced_default

    def __set__(self, instance: WidgetMarkupComponent, value: Any) -> None:
        state = _ensure_state(instance)
        value = _coerce_reactive_value(instance, self._name, value)
        old_value = state.values.get(self._name, _MISSING)
        if old_value is not _MISSING and old_value == value:
            return
        state.values[self._name] = value
        _notify(instance, self._name, value)
        _propagate(instance, self._name)


class _ComputedProperty:
    def __init__(self, function: Callable[[WidgetMarkupComponent], Any]) -> None:
        self._function: Callable[[WidgetMarkupComponent], Any] = function
        self._name: str | None = None

    def __set_name__(self, owner: type[WidgetMarkupComponent], name: str) -> None:
        self._name = name

    def __get__(self, instance: WidgetMarkupComponent | None, owner: type[WidgetMarkupComponent] | None = None) -> Any:
        if instance is None:
            return self
        _track_dependency(instance, self._name)
        state = _ensure_state(instance)
        if self._name not in state.cached or self._name in state.dirty:
            self._recompute(instance)
        return state.cached[self._name]

    def __set__(self, instance: WidgetMarkupComponent, value: Any) -> None:
        raise AttributeError(
            f"computed property '{self._name}' is read-only"
        )

    def _recompute(self, instance: WidgetMarkupComponent) -> tuple[Any, Any]:
        state = _ensure_state(instance)
        if self._name in state.evaluating:
            cycle_path = _format_cycle_path(instance, self._name)
            raise RuntimeError(
                f"Cyclic computed dependency detected: {cycle_path}"
            )

        # Clear previous dependency edges so conditional dependencies stay accurate.
        previous_dependencies = state.dependencies.get(self._name)
        if previous_dependencies:
            for dependency_name in previous_dependencies:
                dependents = state.dependents.get(dependency_name)
                if dependents is not None:
                    dependents.discard(self._name)
        state.dependencies[self._name] = set()

        state.evaluating.add(self._name)
        _compute_stack.append((instance, self._name))
        try:
            new_value = self._function(instance)
        finally:
            _compute_stack.pop()
            state.evaluating.discard(self._name)

        state.dirty.discard(self._name)
        old_value = state.cached.get(self._name, _MISSING)
        state.cached[self._name] = new_value
        return old_value, new_value


def _on_observable_collection_changed(instance: WidgetMarkupComponent, property_name: str, value: list[Any]) -> None:
    _notify(instance, property_name, value)
    _propagate(instance, property_name)


def _coerce_reactive_value(instance: WidgetMarkupComponent, property_name: str, value: Any) -> Any:
    if isinstance(value, ObservableCollection):
        if value._owner is instance and value._property_name == property_name:
            return value
        return ObservableCollection(instance, property_name, _on_observable_collection_changed, list(value))

    if isinstance(value, list):
        return ObservableCollection(instance, property_name, _on_observable_collection_changed, list(value))

    return value


def _get_computed_descriptors(instance: WidgetMarkupComponent) -> dict[str, _ComputedProperty]:
    descriptors: dict[str, _ComputedProperty] = {}
    for cls in reversed(type(instance).__mro__):
        for name, descriptor in cls.__dict__.items():
            if isinstance(descriptor, _ComputedProperty):
                descriptors[name] = descriptor
    return descriptors


def prime_computed_dependencies(instance: WidgetMarkupComponent) -> None:
    """Warm up computed dependency tracking after external initialization is complete."""
    _ensure_state(instance)
    descriptors = _get_computed_descriptors(instance)
    for descriptor in descriptors.values():
        descriptor._recompute(instance)


def _format_cycle_path(instance: WidgetMarkupComponent, loop_name: str) -> str:
    path: list[str] = [
        current_name
        for current_instance, current_name in _compute_stack
        if current_instance is instance
    ]
    if loop_name in path:
        start_index = path.index(loop_name)
        cycle_path = path[start_index:] + [loop_name]
    else:
        cycle_path = path + [loop_name]
    return " -> ".join(cycle_path)


def _ensure_state(instance: WidgetMarkupComponent) -> _ReactiveState:
    state = instance.__dict__.get(_STATE_ATTR)
    if state is None:
        state = _ReactiveState()
        instance.__dict__[_STATE_ATTR] = state
    return state


def _track_dependency(instance: WidgetMarkupComponent, accessed_name: str) -> None:
    if not _compute_stack:
        return
    current_instance, current_computed_name = _compute_stack[-1]
    if current_instance is not instance:
        return
    if current_computed_name == accessed_name:
        return
    state = _ensure_state(instance)
    state.dependencies.setdefault(current_computed_name, set()).add(accessed_name)
    state.dependents.setdefault(accessed_name, set()).add(current_computed_name)


def reactive(function: Callable[[WidgetMarkupComponent], Any]) -> _ReactiveProperty:
    """Create a settable reactive property descriptor.

    Decorate a method whose return value is the per-instance default.
    The property notifies and triggers dependency propagation when its value changes.
    """
    return _ReactiveProperty(function)


def computed(function: Callable[[WidgetMarkupComponent], Any]) -> _ComputedProperty:
    """Wrap a function as a read-only computed property descriptor.

    Dependencies are collected at runtime based on accessed reactive/computed properties.
    """
    return _ComputedProperty(function)


def _notify(instance: WidgetMarkupComponent, property_name: str, value: Any) -> None:
    instance.on_property_changed(property_name, value)


def _propagate(instance: WidgetMarkupComponent, source_property_name: str) -> None:
    state = _ensure_state(instance)
    pending = list(state.dependents.get(source_property_name, ()))
    visited: set[str] = set()
    while pending:
        computed_name = pending.pop(0)
        if computed_name in visited:
            continue
        visited.add(computed_name)

        descriptor = getattr(type(instance), computed_name, None)
        if not isinstance(descriptor, _ComputedProperty):
            continue

        state.dirty.add(computed_name)
        old_value, new_value = descriptor._recompute(instance)
        if old_value is _MISSING or old_value != new_value:
            _notify(instance, computed_name, new_value)
            for downstream in state.dependents.get(computed_name, ()): 
                if downstream not in visited:
                    pending.append(downstream)


class WidgetMarkupComponent:
    """WidgetMarkup-facing reactive base type.

    Keep WidgetMarkup-specific extension points on this class while preserving
    a cohesive reactive behavior surface.
    """

    @staticmethod
    def create(module_name: str, user_widget: Any = None) -> Any:
        """Create a WidgetMarkup component instance from a module path.

        module_name follows the pattern A.B.C and class name is resolved as C.
        """
        if not module_name or not isinstance(module_name, str):
            raise ValueError("module_name must be a non-empty string")

        module = importlib.import_module(module_name)

        class_name = module_name.rsplit(".", 1)[-1]
        component_type = getattr(module, class_name)
        instance = component_type.__new__(component_type)
        if user_widget is not None:
            if isinstance(user_widget, str):
                user_widget = unreal.find_object(None, user_widget)
            setattr(instance, _USER_WIDGET_ATTR, user_widget)
        component_type.__init__(instance)
        return instance

    def __init__(self) -> None:
        try:
            self.apply_property_bindings()
        except Exception as exc:
            unreal.log_warning(f"WidgetMarkup: apply_property_bindings failed: {exc}")
        try:
            self.apply_delegate_bindings()
        except Exception as exc:
            unreal.log_warning(f"WidgetMarkup: apply_delegate_bindings failed: {exc}")

    def apply_property_bindings(self) -> None:
        user_widget = getattr(self, _USER_WIDGET_ATTR, None)
        if user_widget is None:
            return

        extension = unreal.WidgetMarkupBlueprintGeneratedClassExtension.get_widget_markup_extension(user_widget)
        if extension is None:
            return

        for binding in extension.get_property_bindings():
            self.apply_property_binding(binding)

    def apply_property_binding(self, binding: Any) -> None:
        user_widget = getattr(self, _USER_WIDGET_ATTR, None)
        if user_widget is None:
            return

        value = getattr(self, binding.source_expression)
        widget_markup.DataBinding.apply_property_binding(user_widget, binding, value)

    def apply_delegate_bindings(self) -> None:
        user_widget = getattr(self, _USER_WIDGET_ATTR, None)
        if user_widget is None:
            return

        extension = unreal.WidgetMarkupBlueprintGeneratedClassExtension.get_widget_markup_extension(user_widget)
        if extension is None:
            return

        bindings = extension.get_delegate_bindings()
        for binding in bindings:
            self.apply_delegate_binding(user_widget, binding)

    def apply_delegate_binding(self, user_widget: Any, binding: Any) -> None:
        self.bind_delegate(
            str(binding.target_widget_name),
            str(binding.delegate_property_name),
            str(binding.function_name),
        )

    def bind_delegate(self, target: str | Any, delegate_name: str, function: str | Callable[..., Any]) -> None:
        """Bind a callable to a named delegate on a widget.

        Args:
            target: Widget name (str) or UWidget instance.
            delegate_name: Delegate property name (e.g. 'OnClicked', 'OnMouseButtonDownEvent').
            function: Python method name on this component (str), or an arbitrary callable.
        """
        if isinstance(target, str):
            target_widget = self.find_widget(target)
            target_name = target
        else:
            target_name = str(getattr(target, 'get_name', lambda: '')())
            target_widget = self.find_widget(target_name)
        if target_widget is None:
            unreal.log_warning(f"WidgetMarkup: widget '{target_name}' not found in WidgetTree")
            return

        # Resolve the method only to validate it exists. The retained callable
        # must NOT capture a bound method (which holds a strong reference to
        # self), or the delegate proxy -> callable -> self chain keeps this
        # component and its widget alive after removal, leaking on every
        # add_child/remove_child cycle.
        if callable(function):
            method_name = None
        else:
            method_name = function
            if not callable(getattr(self, function, None)):
                unreal.log_warning(f"WidgetMarkup: method '{function}' not found on component")
                return
        self_ref = weakref.ref(self)

        # For FOnPointerEvent delegates, wrap the Python method in an adapter
        # that unpacks payload → (geometry, mouse_event) and writes reply back.
        if unreal.WidgetMarkupUserWidget.is_on_pointer_event(target_widget, delegate_name):
            def _pointer_event_adapter(payload):
                comp = self_ref()
                if comp is None:
                    payload.reply = widget_markup.WidgetLibrary.unhandled()
                    return
                method = function if method_name is None else getattr(comp, method_name)
                result = method(payload.geometry, payload.mouse_event)
                payload.reply = result

            pointer_delegate = unreal.WidgetMarkupOnPointerEvent()
            pointer_delegate.bind_callable(_pointer_event_adapter)

            user_widget = getattr(self, _USER_WIDGET_ATTR, None)
            if user_widget is None:
                unreal.log_warning(f"WidgetMarkup: user widget not available to bind '{delegate_name}'")
                return
            user_widget.bind_on_pointer_event(target_widget, delegate_name, pointer_delegate)

            # Keep the Python delegate wrapper alive. The wrapper's reference
            # collector keeps the underlying UPythonCallableForDelegate proxy
            # reachable across GC; delegates only hold a weak reference to that
            # proxy, so without this the binding silently dies after ~60s.
            if not hasattr(self, "_delegate_keepalive"):
                self._delegate_keepalive = []
            self._delegate_keepalive.append(pointer_delegate)
            return

        # Try C++ name first (OnClicked), then snake_case fallback (on_clicked),
        # then get_editor_property for protected properties.
        delegate_attr = None
        tried_names = []
        for name in (delegate_name, _camel_to_snake(delegate_name)):
            tried_names.append(name)
            try:
                delegate_attr = getattr(target_widget, name, None)
                if delegate_attr is not None:
                    break
            except Exception:
                pass
            try:
                delegate_attr = target_widget.get_editor_property(delegate_name)
                if delegate_attr is not None:
                    break
            except Exception:
                pass
        if delegate_attr is None:
            unreal.log_warning(f"WidgetMarkup: delegate '{delegate_name}' not found on '{target_name}', tried: {', '.join(tried_names)}")
            return

        def _weak_callable(*args, **kwargs):
            comp = self_ref()
            if comp is None:
                return None
            method = function if method_name is None else getattr(comp, method_name)
            return method(*args, **kwargs)

        if hasattr(delegate_attr, "bind_callable"):
            delegate_attr.bind_callable(_weak_callable)
        elif hasattr(delegate_attr, "add_callable"):
            delegate_attr.add_callable(_weak_callable)
        else:
            unreal.log_warning(
                f"WidgetMarkup: delegate '{delegate_name}' on '{target_name}' does not support bind_callable or add_callable"
            )
            return

        # Keep the Python delegate wrapper alive across GC (same reason as the
        # pointer-event branch above: the wrapper's reference collector keeps the
        # UPythonCallableForDelegate proxy reachable).
        if not hasattr(self, "_delegate_keepalive"):
            self._delegate_keepalive = []
        self._delegate_keepalive.append(delegate_attr)

    def prime_computed_dependencies(self) -> None:
        """Warm up computed dependencies after the owner finishes initialization.

        Call this method explicitly from the upper layer once the instance is fully
        configured and any computed properties can safely read initialization-time state.
        """
        prime_computed_dependencies(self)

    def on_property_changed(self, property_name: str, value: Any) -> None:
        """Push changed reactive property value back to bound UMG widgets."""
        user_widget = getattr(self, _USER_WIDGET_ATTR, None)
        if user_widget is None:
            return

        extension = unreal.WidgetMarkupBlueprintGeneratedClassExtension.get_widget_markup_extension(user_widget)
        if extension is None:
            return

        bindings = extension.get_property_bindings()
        for binding in bindings:
            if binding.source_expression == property_name:
                widget_markup.DataBinding.apply_property_binding(user_widget, binding, value)
                return

    def refresh(self, data: Any) -> None:
        """Set data on this component by calling on_data_refresh."""
        self.on_data_refresh(data)

    def on_data_refresh(self, data: Any) -> None:
        """Called when data is pushed to this component via refresh().

        Override in subclasses to update UI from data. The component's owning
        UserWidget is available via self._widget_markup_user_widget.
        Same data on same component type should produce identical display.
        """
        pass

    # ------------------------------------------------------------------
    # Child component management
    # ------------------------------------------------------------------

    def find_widget(self, name: str) -> Any:
        """Find a widget by name in the owning UserWidget's WidgetTree.

        Returns the UWidget instance, or None if not found.
        """
        user_widget = getattr(self, _USER_WIDGET_ATTR, None)
        if user_widget is None:
            return None
        widget = widget_markup.WidgetLibrary.find_widget_in_user_widget(
            user_widget, name)
        return widget

    def add_child(self, name: str, cls: Any, parent: Any) -> Any:
        """Add a child widget component to a parent panel widget.

        Args:
            name: Unique name for the child widget in the WidgetTree.
            cls: Widget class. Accepts:
                - A UClass (e.g., unreal.TextBlock).
                - A string token (e.g., 'TextBlock', 'Game.WidgetMarkup.MyWidget',
                  '/Game/MyWidget').
            parent: The parent panel widget. Accepts:
                - A str widget name or path name.
                - A UWidget instance.

        Returns:
            The WidgetMarkupComponent of the created child widget, or None if
            the child widget has no Python component.
        """
        user_widget = getattr(self, _USER_WIDGET_ATTR, None)
        if user_widget is None:
            raise RuntimeError("Cannot add child: component is not attached to a UserWidget.")

        parent_name_or_path = parent if isinstance(parent, str) else str(parent.get_path_name())

        class_token = cls if isinstance(cls, str) else str(cls.get_name())
        child_name = widget_markup.WidgetLibrary.add_child_widget(
            user_widget, parent_name_or_path, class_token, name)

        if child_name is None:
            return None

        child_widget = self.find_widget(child_name)
        if child_widget is None:
            return None
        return widget_markup.Core.get_component_by_widget(child_widget)

    def remove_child(self, child: Any) -> bool:
        """Remove a child widget.

        Args:
            child: The child widget name (str), the child UWidget instance, or
                the child's WidgetMarkupComponent.

        Returns:
            True if the child was found and removed, False otherwise.
        """
        user_widget = getattr(self, _USER_WIDGET_ATTR, None)
        if user_widget is None:
            return False

        # Resolve the child widget path and its Python component. The component
        # holds the widget through _widget_markup_user_widget, while the widget's
        # extension holds the component back, forming a cycle that keeps the
        # widget reachable through the Python reference collector even after
        # RemoveFromParent + MarkAsGarbage. Break that cycle before removal.
        child_component = None
        child_path = None
        if isinstance(child, str):
            child_widget = self.find_widget(child)
            child_path = str(child_widget.get_path_name()) if child_widget is not None else child
            child_component = self.get_child(child)
        else:
            child_widget = getattr(child, _USER_WIDGET_ATTR, None)
            if child_widget is not None:
                child_path = str(child_widget.get_path_name())
                child_component = child
            else:
                child_path = str(child.get_path_name())
                child_component = widget_markup.Core.get_component_by_widget(child)

        if child_component is not None:
            if hasattr(child_component, _USER_WIDGET_ATTR):
                try:
                    delattr(child_component, _USER_WIDGET_ATTR)
                except AttributeError:
                    pass
            if hasattr(child_component, "_delegate_keepalive"):
                child_component._delegate_keepalive = []

        return widget_markup.WidgetLibrary.remove_child_widget(user_widget, child_path)

    def get_child(self, name: str) -> Any:
        """Get a child WidgetMarkupComponent by name.

        Args:
            name: The child widget name in the WidgetTree.

        Returns:
            The WidgetMarkupComponent if the child has one, or None if no
            widget with the given name exists or it has no component.
        """
        user_widget = getattr(self, _USER_WIDGET_ATTR, None)
        if user_widget is None:
            return None
        widget = self.find_widget(name)
        if widget is None:
            return None
        return widget_markup.Core.get_component_by_widget(widget)