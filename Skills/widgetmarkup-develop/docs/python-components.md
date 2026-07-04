# Python Components

> **WidgetMarkupApp `unreal` API:** See [unreal-python-api.md](unreal-python-api.md) for `ScriptName` mappings (`InputLibrary`, `WidgetLibrary`), `PointerEvent` / mouse-button handling, and common pitfalls.

## Design Principles

**Prefer declarations over imperative code.** The framework's three declarative mechanisms should be your first choice:

| Priority | Mechanism | When |
|---|---|---|
| 1 | **Data binding** (`{reactive}` / `{computed}`) | Push data from Python to widget properties |
| 2 | **Event binding** (`OnClicked="handler"`) | React to user input on widgets |
| 3 | Manual `find_widget` + setter | Only when binding cannot express the behavior |

**Minimize coupling between WidgetMarkupComponents.** Each component should be self-contained:
- Use `@computed` to derive visuals from local `@reactive` state — don't reach into parent components.
- Use **callback registration** (e.g., `cell.on_click = lambda ...`) instead of children calling parent methods directly.
- Cross-instance dependencies are NOT auto-tracked by the reactive system. If unavoidable, keep them explicit and minimal.

**In a dependency chain, every mutable property MUST be `@reactive`.** `@computed` tracks dependencies by intercepting reads of `@reactive` and `@computed` descriptors. Plain Python attributes (e.g. `self._start_time`) are invisible to the tracker — changing them will NOT trigger recomputation. If a `@computed` property reads a value that can change, that value must be a `@reactive` property:

```python
# BROKEN: _start_time is a plain attribute, elapsed won't recompute on its own
@computed
def elapsed(self) -> int:
    return int(self.current_time - self._start_time)   # _start_time change ignored!

# CORRECT: both sources are @reactive, both trigger recomputation
@reactive
def start_time(self) -> float:
    return 0.0

@computed
def elapsed(self) -> int:
    return int(self.current_time - self.start_time)  # tracks both
```

## Basic Component

```python
from WidgetMarkupComponent import WidgetMarkupComponent, reactive

class MyComponent(WidgetMarkupComponent):
    @reactive
    def count(self):
        return 0

    def on_button_clicked(self):
        self.count += 1
```

## Reactive Properties

`@reactive` wraps a method in `_ReactiveProperty`, a descriptor that stores values in per-instance state. The method body serves as the **default value** — called by `__get__` when no value has been explicitly set.

**The getter should return a plain constant**, not read other attributes. The descriptor itself is the store — `__set__` writes to internal state, `__get__` reads from it. If you need a value derived from other reactive properties, use `@computed` instead.

```python
@reactive
def label(self):
    return ""          # Plain default — the descriptor IS the store

# Assignment calls _ReactiveProperty.__set__:
self.label = "Hello"  # stores in state, triggers notify → apply_property_binding
```

**Important**: `@reactive` does not support the `.setter` decorator pattern. Writing `@label.setter` will crash with `'_ReactiveProperty' object has no attribute 'setter'`. Assign directly: `self.label = value`.

**Anti-pattern** — the getter should not reference instance fields:
```python
# Incorrect: this is what @computed is for
@reactive
def label(self):
    return self._label  # depends on another attribute
```

**Usage tips**:
- Keep internal state in plain instance attributes (for example `self._state`), not in reactive getters. Update the reactive property via assignment when the UI needs to reflect a change.
- When the property changes, `on_property_changed()` triggers `apply_property_binding`. The binding pipeline converts the Python value to the target UMG property type.

## Multi-Property State

Reactive properties can interact — changing one can trigger updates to others via `@computed`.

```python
class CalcComponent(WidgetMarkupComponent):
    @reactive
    def display(self):
        return "0"

    @reactive
    def operand1(self):
        return 0.0

    @reactive
    def operator(self):
        return ""

    @computed
    def result_text(self):
        """Auto-updates whenever display, operand1, or operator changes."""
        if self.operator and self.display:
            return f"{self.operand1} {self.operator} {self.display}"
        return self.display

    def on_digit(self, digit):
        if self.display == "0":
            self.display = str(digit)
        else:
            self.display += str(digit)
```

```xml
<TextBlock Text="{result_text}" Font.Size="32" Justification="Right" />
```

## Reactive Lists (ObservableCollection)

`@reactive` properties returning a `list` are auto-wrapped in `ObservableCollection`. Mutations (`append`, `pop`, `clear`, `insert`, `remove`) automatically trigger binding refresh:

```python
@reactive
def items(self):
    return []

def add_item(self):
    self.items.append(42)  # Auto-triggers ListView refresh
```

No need for `self.items = list(self.items)` — the ObservableCollection handles notification internally.

## Computed Properties

`@computed` creates a read-only property whose value is computed from other reactive properties. Dependencies are tracked automatically:

```python
@computed
def full_name(self):
    return f"{self.first_name} {self.last_name}"
```

## Accessing the User Widget

The owning `UUserWidget` is available as `self._widget_markup_user_widget`, but **prefer data binding whenever possible.** Only fall back to `get_editor_property()` when binding can't express the desired behavior:

```python
# Prefer data binding (automatic, reactive):
#   <TextBlock Text="{display_text}" />

# Only when binding is impractical:
label = self._widget_markup_user_widget.get_editor_property('MyLabel')
```

---

## Property Bindings (XML Syntax)

Reactive properties connect to UMG widgets via the `{propertyName}` binding syntax in XML attributes:

```xml
<TextBlock Name="Display" Text="{display}" />
<ProgressBar Name="Progress" Percent="{progress}" />
<CheckBox IsChecked="{checked}" />
```

- `{display}` → binding to `self.display` reactive property
- `"0"` → literal string value, no binding
- `{}literal` → escape `{` for literal text starting with `{`

Bindings are parsed during `apply_property_bindings()` in `WidgetMarkupComponent.__init__()`. The binding system matches `source_expression` against `@reactive` property names to push updates to target widget properties.

## Handler Dispatch

Button clicks are routed by **method name** on the Python component:

```xml
<Button OnClicked="on_digit_1"><TextBlock Text="1" /></Button>
```
→ calls `component.on_digit_1()`

Each button needs an individual handler method. The framework does not support tag-based dispatch or generic handler routing.

## Init Order

If `@reactive` getters return plain constants (as recommended), `super().__init__()` can go anywhere. The only ordering constraint exists when a getter reads an instance field — set those fields first:

```python
class MyComponent(WidgetMarkupComponent):
    @reactive
    def label(self):
        return self._data.get("label", "untitled")  # reads instance field

    def __init__(self):
        self._data = {"label": "Hello"}   # must come before super().__init__()
        super().__init__()                # triggers apply_property_bindings
```

Better: avoid this entirely by keeping getters to plain defaults and managing state separately.

## Module Import & Resolve Order

WidgetMarkup resolves `Script="ModuleName"` to a Python module import. `Game/Content/Python/` is on `sys.path`:

```
File location                    → Script attribute
Game/Content/Python/MyComponent.py    → Script="MyComponent"
Plugin/Content/Python/Samples/X  → Script="Samples.X"
```

Do not wrap the import in try/except — if the import fails, it should crash visibly:

```python
from WidgetMarkupComponent import WidgetMarkupComponent, reactive
# Do not:
# try:
#     from WidgetMarkupComponent import ...
# except ImportError:
#     ...  # silent fallback hides real errors
```

---

## Common Patterns

### Delegating to Private Helpers

Since each button maps to a named handler, delegate to private methods to reduce boilerplate:

```python
def on_color_red(self):   self._set_color("red")
def on_color_blue(self):  self._set_color("blue")
def on_color_green(self): self._set_color("green")

def _set_color(self, name: str):
    self.selected_color = name
```

This pattern is useful whenever multiple buttons share similar logic but need distinct handler names in XML.

## List View Setup

The list (parent) blueprint declares a `ListView` with `ListItems` binding and `EntryWidgetClass`:

```xml
<WidgetBlueprint Script="Samples.ListComponent">
  <WidgetTree>
    <CanvasPanel>
      <ListView ListItems="{items}"
                EntryWidgetClass="/WidgetMarkup/Samples/ListEntry" />
    </CanvasPanel>
  </WidgetTree>
</WidgetBlueprint>
```

## Entry Widget

The entry blueprint is a separate `.widgetmarkup` file:

```xml
<WidgetBlueprint Script="Samples.ListEntryComponent">
  <WidgetTree>
    <TextBlock Name="ItemLabel" Text="{display_text}" ColorAndOpacity="{color}" />
  </WidgetTree>
</WidgetBlueprint>
```

## Entry Python Component

```python
from WidgetMarkupComponent import WidgetMarkupComponent, reactive
from IUserObjectListEntry import IUserObjectListEntry

class ListEntryComponent(WidgetMarkupComponent, IUserObjectListEntry):
    @reactive
    def display_text(self):
        return ""

    @reactive
    def color(self):
        return unreal.SlateColor()

    def on_data_refresh(self, data):
        """data is the raw Python value from the list."""
        self.display_text = str(data)
        self.color = _COLORS[data % len(_COLORS)]

    def on_entry_released(self):
        """Entry recycled — cleanup here."""
        pass
```

## Interface Hierarchy

| Mixin | Provides |
|---|---|
| `IUserListEntry` | `on_entry_released()`, `on_item_selection_changed(b)`, `on_item_expansion_changed(b)`, `is_selected()`, `get_owning_list_view()` |
| `IUserObjectListEntry(IUserListEntry)` | `get_list_item()` (returns raw Python value) |

`on_data_refresh(data)` is defined in `WidgetMarkupComponent` — override it to receive list item data.

The Python mixin auto-binds to `UWidgetMarkupUserWidget`'s multicast delegates in `__init__`.

> **Entry without IUserObjectListEntry:** For simple entry widgets that only need `on_data_refresh`, inheriting `IUserObjectListEntry` is optional. `on_data_refresh` is on `WidgetMarkupComponent` itself. Only inherit `IUserObjectListEntry` if you need `get_list_item()`, selection/expansion callbacks, or `is_selected()`.

## Property Change Notification

Override `on_property_changed(name, value)` to observe all reactive/computed property changes:

```python
class MyComponent(WidgetMarkupComponent):
    def __init__(self):
        self.changed_names = []
        super().__init__()

    def on_property_changed(self, name, value):
        self.changed_names.append(name)
```

Called for every reactive setter and computed recalculation. The `name` is the Python property name (e.g. `"total"`, `"doubled"`), and `value` is the new value.

## Reactive Property Details

### Default Values

The method body of `@reactive` serves as the default. It runs once on first access:

```python
@reactive
def count(self): return 0          # int
@reactive
def label(self): return "hello"    # str
@reactive
def flag(self): return True        # bool
@reactive
def pi(self): return 3.14          # float
@reactive
def optional(self): return None    # None — supported, not treated as "uninitialized"
@reactive
def items(self): return []         # list — auto-wrapped in ObservableCollection
```

Setting `None` on a previously-set reactive property works normally: `self.optional = None`.

## Computed Property Chains

`@computed` tracks dependencies through multiple levels. A 3-level chain works correctly:

```python
@computed
def total(self): return self.a + self.b          # depends on a, b
@computed
def doubled(self): return self.total * 2         # depends on total
@computed
def tripled(self): return self.doubled * 3       # depends on doubled
```

When `b` changes → `total` recalculates → `doubled` recalculates → `tripled` recalculates. All three fire `on_property_changed`.

---

## Child Component Management

WidgetMarkup provides APIs to dynamically add and remove child widgets from a parent component.

### get_child

Retrieve a child WidgetMarkupComponent by its widget name:

```python
child_comp = self.get_child("ChildName")
# Returns the WidgetMarkupComponent if the child has one, or None
```

Works on both statically defined (XML) and dynamically added children. The child widget must have a `Script` attribute to have a component.

### add_child

Dynamically create and add a widget to a parent panel:

```python
# Add a plain UMG widget (returns None — no Python component):
self.add_child("MyText", "/Script/UMG.TextBlock", "RootCanvas")

# Add a WidgetMarkup widget (returns its component):
comp = self.add_child("MyChild", "/WidgetMarkup/Tests/TestChild", "RootCanvas")
```

Parameters:
- `name`: Unique widget name in the WidgetTree
- `cls`: Widget class — accepts string path (`"/Script/UMG.TextBlock"`), dot-notation (`"WidgetMarkup.Tests.TestChild"`), or UClass (`unreal.TextBlock`)
- `parent_name`: Name of the parent panel widget (must be a `UPanelWidget` like `CanvasPanel`)

The return value is the `WidgetMarkupComponent` if the created widget has a Python component, or `None` for plain UMG widgets.

### remove_child

Remove a child widget by name or reference:

```python
self.remove_child("MyText")        # by name
self.remove_child(widget_ref)      # by UWidget reference
self.remove_child(component_ref)   # by component reference
```

Returns `True` if the child was found and removed.

## Reusable WidgetMarkup Components

WidgetMarkup blueprints can embed other WidgetMarkup blueprints as children using the `<WidgetMarkup.X.Y>` syntax:

```xml
<WidgetBlueprint Script="Tests.TestStaticChild">
  <WidgetTree>
    <CanvasPanel Name="RootCanvas">
      <WidgetMarkup.Tests.TestChild Name="StaticChild" />
    </CanvasPanel>
  </WidgetTree>
</WidgetBlueprint>
```

This compiles `TestChild.widgetmarkup` on demand and embeds the resulting widget tree. The embedded widget's `Script` component is created as usual — `get_child("StaticChild")` returns the TestChild component.

The same `TestChild` can also serve as a ListView entry widget:

```xml
<ListView ListItems="{items}"
          EntryWidgetClass="/WidgetMarkup/Tests/TestChild" />
```

A reusable child widget should not call `request_shutdown()` — that is the responsibility of the root test component only.

## Dynamic Event Routing

When creating child widgets dynamically via `add_child`, route events back to the parent using **callback registration** rather than directly calling parent methods:

```python
# Cell component — exposes a click callback:
class MinesweeperCell(WidgetMarkupComponent):
    def __init__(self):
        self.on_click: Callable[[unreal.Geometry, unreal.PointerEvent], Any] | None = None
        super().__init__()

    def on_mouse_down(self, geometry, mouse_event):
        if self.on_click is not None:
            return self.on_click(geometry, mouse_event)
        return unreal.WidgetLibrary.handled()

# Parent component — registers callback per cell:
def _create_cells(self):
    for row, col in ...:
        cell = self.add_child(f"Cell_{row}_{col}", "/WidgetMarkup/Samples/MinesweeperCell", game_grid)
        cell.on_click = lambda geo, evt, r=row, c=col: self._on_cell_clicked(r, c, geo, evt)
```

This pattern keeps the child component **decoupled** from the parent — the child only needs a public callback attribute, not knowledge of the parent's API. Use `lambda` default-argument binding (`r=row, c=col`) to capture loop variables correctly.

## Computed Property Auto-Tracking

`@computed` automatically discovers dependencies by tracking which `@reactive` or `@computed` properties are read during evaluation:

```python
@reactive
def state(self) -> CellState:
    return CellState.HIDDEN

@reactive
def is_mine(self) -> bool:
    return False

@computed
def label(self) -> str:
    if self.state == CellState.FLAGGED:   # reads state → tracked
        return "F"
    if self.is_mine:                       # reads is_mine → tracked
        return "*"
    return ""
```

When `self.state = CellState.REVEALED` is assigned:
1. `_ReactiveProperty.__set__` stores the new value
2. `_propagate` finds all `@computed` properties that read `state` → marks them dirty
3. Each dirty computed property recomputes and fires `on_property_changed`
4. UI bindings update automatically

**Cross-instance dependencies are NOT tracked.** Computed properties can only auto-track reactive properties on the same component instance. For cross-instance changes (e.g., a parent's game-over state affecting child visuals), you must call the child's methods manually.

> **Minesweeper sample:** See `Content/Samples/Minesweeper.widgetmarkup` and `Content/Python/Samples/Minesweeper.py` for a complete example combining `add_child`, `on_click` callback registration, and `@computed` auto-tracking.
