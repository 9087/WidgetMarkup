# unreal Python API (WidgetMarkup)

WidgetMarkup components run with UE's Python bindings exposed by the **WidgetMarkup** plugin. The subset of `unreal` types and libraries available at runtime differs from the full Editor — do not assume Editor documentation applies verbatim.

## `widget_markup` vs `unreal`

Most APIs live on **`unreal`**: widget types, delegates, `unreal.WidgetLibrary`, `unreal.SystemLibrary`, and the rest of the UE Python surface. **`widget_markup`** is a thin extension for WidgetMarkup-specific helpers and workarounds.

**Lookup order:** check `widget_markup` only for APIs listed in [widget_markup native module](#widget_markup-native-module) below (or called out elsewhere in this doc). If a function or type is not there, use native **`unreal`** — but confirm names against [ScriptName](#scriptname-vs-c-class-name) rules; WidgetMarkup does not expose the full Editor API.

Examples on `unreal`: `unreal.WidgetLibrary.handled()`, `unreal.UserWidget`, `unreal.SystemLibrary.print_string`. For pointer-event input (mouse buttons), use `mouse_event.effecting_button.get_editor_property("key_name")` directly — see [PointerEvent](#pointerevent--use-effecting_buttonget_editor_propertykey_name).

## ScriptName vs C++ class name

Blueprint function libraries are exported under their `ScriptName` meta, **not** the C++ class name:

| C++ class | Python module on `unreal` |
|---|---|
| `UKismetInputLibrary` | `unreal.InputLibrary` |
| `UWidgetBlueprintLibrary` | `unreal.WidgetLibrary` |
| `UKismetSystemLibrary` | `unreal.SystemLibrary` |

`unreal.KismetInputLibrary` and `unreal.WidgetBlueprintLibrary` are **`None`** in WidgetMarkup. `unreal.load_class(None, "/Script/Engine.KismetInputLibrary")` returns a `UClass` but does **not** expose static UFUNCTIONs as Python methods — use the `ScriptName` entry on `unreal` instead.

## PointerEvent — use `effecting_button.get_editor_property("key_name")`

`unreal.PointerEvent` is passed to `OnMouseButtonDownEvent` / `OnMouseButtonUpEvent` handlers. The event type in WidgetMarkup is `unreal.WidgetMarkupPointerEvent` (not `unreal.PointerEvent`).

**`mouse_event.effecting_button`** is a readable property returning an `unreal.Key`. Use `get_editor_property("key_name")` to read the key name:

```python
key_name = str(mouse_event.effecting_button.get_editor_property("key_name"))
# Returns strings like "LeftMouseButton", "RightMouseButton", "MiddleMouseButton"
```

## FKey — use `get_editor_property("key_name")`

`unreal.Key` wraps the `FKey` struct. UE does not export `EKeys` constants as class attributes on `unreal.Key`, nor does it expose `.key_name` as a Python property. Use `get_editor_property("key_name")` and compare as string:

```python
key_name = str(mouse_event.effecting_button.get_editor_property("key_name"))

if "RightMouseButton" in key_name:
    self.flag_cell()
elif "LeftMouseButton" in key_name:
    self.reveal_cell()
```

For chord detection (simultaneous left+right press), track button state across `OnMouseButtonDownEvent` / `OnMouseButtonUpEvent` since each event carries only one effecting button:

```python
# In on_mouse_down:
if "RightMouseButton" in key_name:
    self._right_down = True
else:
    self._left_down = True

if self._left_down and self._right_down:
    self._chord(row, column)

# In on_mouse_up:
if "RightMouseButton" in key_name:
    self._right_down = False
else:
    self._left_down = False
```

> **Note:** `unreal.Key` has no class-level key constants. Use plain strings like `"LeftMouseButton"` for comparison. The `.key_name` attribute is not a readable Python property — always use `get_editor_property("key_name")`.

## Event Reply — FWidgetMarkupEventReply

WidgetMarkup provides its own Blueprint-compatible event reply struct (`unreal.WidgetMarkupEventReply`) that mirrors Slate's `FReply`. Python handlers **must** return this type from `OnMouseButtonDownEvent` / `OnMouseButtonUpEvent` handlers — the old `unreal.WidgetLibrary.handled()` returns `FEventReply` which is incompatible with the payload system.

**Fields** (read/write via `set_editor_property`): `bIsHandled` (bool), `MouseCaptor` (UWidget*), `MouseLock` (UWidget*), `FocusRecipient` (UWidget*), `bReleaseMouseCapture` (bool), `bShouldSetMousePos` (bool), `RequestedMousePos` (FVector2D).

Prefer `widget_markup.WidgetLibrary` static helpers over raw `set_editor_property`:

```python
import widget_markup

reply = widget_markup.WidgetLibrary.handled()
widget_markup.WidgetLibrary.capture_mouse(reply, border_widget)       # DOWN
widget_markup.WidgetLibrary.release_mouse_capture(reply)               # UP
```

## Geometry — FWidgetMarkupGeometry

WidgetMarkup replaces Slate's `FGeometry` with `unreal.WidgetMarkupGeometry`. Fields via `get_editor_property`: `AbsolutePosition` (top-left), `Size` (local size).

**Check boundary on UP** to cancel actions when cursor leaves the widget:

```python
def on_mouse_up(self, geometry, mouse_event):
    cursor = widget_markup.WidgetLibrary.get_screen_space_position(mouse_event)
    if not widget_markup.WidgetLibrary.is_under_location(geometry, cursor):
        reply = widget_markup.WidgetLibrary.handled()
        widget_markup.WidgetLibrary.release_mouse_capture(reply)
        return reply  # cancel — cursor left the widget
    self.do_action()
```

> `geometry.is_under_location()` is a C++ method not exposed to Python. Always use `widget_markup.WidgetLibrary.is_under_location(geometry, position)`.

## PointerEvent — ScreenSpacePosition

`FWidgetMarkupPointerEvent` now includes the cursor position:

```python
cursor = widget_markup.WidgetLibrary.get_screen_space_position(mouse_event)
```

## WidgetLibrary for event replies (legacy)

```python
# DEPRECATED — returns FEventReply (wrong type)
# return unreal.WidgetLibrary.handled()
```

Use `widget_markup.WidgetLibrary.handled()` instead.

## Border: distinguishing mouse buttons

`Button` only supports `OnClicked` (primary button). For distinct left/right actions, use `Border` + `OnMouseButtonDownEvent`:

```xml
<Border Name="ItemTile" OnMouseButtonDownEvent="on_item_tile_mouse_down"
        BrushColor="0.2,0.2,0.22,1" Padding="8,8,8,8">
  <TextBlock Name="ItemTileLabel" Text="{item_label}" Visibility="HitTestInvisible"
             Font.Size="14" Justification="Center" />
</Border>
```

Set decorative child text to `Visibility="HitTestInvisible"` so it does not steal hit tests from the `Border`.

```python
def on_item_tile_mouse_down(self, geometry: unreal.Geometry, mouse_event: unreal.WidgetMarkupPointerEvent):
    key_name = str(mouse_event.effecting_button.get_editor_property("key_name"))
    if "RightMouseButton" in key_name:
        self.open_item_context_menu()
    elif "LeftMouseButton" in key_name:
        self.select_item()
    return unreal.WidgetLibrary.handled()
```

`OnMouseButtonDownEvent` is a **single-cast** delegate (`bind_callable`); `OnClicked` on `Button` is multicast (`add_callable`).

## widget_markup native module

WidgetMarkup-specific helpers on `import widget_markup`. For anything not listed here, use **`unreal`** (see [lookup order](#widget_markup-vs-unreal) above).

### `widget_markup.DataBinding`

Internal binding pipeline used by the markup compiler and reactive property updates.

- **`apply_property_binding(user_widget, binding, value)`** — apply a resolved property binding to a widget instance.

### `widget_markup.WidgetLibrary`

Widget tree lookup, list-entry data access, and **event reply helpers**.

- **`handled()`** — create a handled `FWidgetMarkupEventReply`.
- **`unhandled()`** — create an unhandled reply.
- **`capture_mouse(reply, widget)`** — set `MouseCaptor` on the reply.
- **`release_mouse_capture(reply)`** — set `bReleaseMouseCapture` on the reply.
- **`lock_mouse_to_widget(reply, widget)`** — set `MouseLock`.
- **`set_user_focus(reply, widget)`** — set `FocusRecipient`.
- **`set_mouse_position(reply, position)`** — set cursor position request.
- **`is_under_location(geometry, screen_position)`** — check if screen point is inside geometry.
- **`get_screen_space_position(mouse_event)`** — read cursor position from event.
- **`find_widget_in_user_widget(user_widget, name)`** — find a named widget.
- **`get_python_object_from_list_item(list_item)`** — get Python data from a list item.
- **`add_child_widget(user_widget, parent_name, class_token, name)`** — create child widget.
- **`remove_child_widget(user_widget, child)`** — remove child widget.

### `widget_markup.Application`

WidgetMarkupApp launcher utilities (CLI and process control). Only relevant when running the standalone `.exe`.

- **`get_extra_arguments()`** — extra command-line arguments passed after the blueprint path.
- **`request_shutdown()`** — request process exit (used by automated tests).

## Reading Widget Properties in Python

When reading back properties set via XML, use `get_editor_property()`. The Python access pattern depends on the UE type:

| Type | Read pattern | See |
|---|---|---|
| FLinearColor / FSlateColor | `.r`, `.g`, `.b`, `.a` | [linear-color.md](structs/linear-color.md) |
| FVector2D (double) | `.x`, `.y` (lowercase) | [vector2d.md](structs/vector2d.md) |
| FVector2f | `.X`, `.Y` (uppercase) | [vector2d.md](structs/vector2d.md) |
| FDeprecateSlateVector2D | `export_text()` substring match | [vector2d.md](structs/vector2d.md) |
| FMargin | `.left`, `.top`, `.right`, `.bottom` | [margin.md](structs/margin.md) |
| bool / float / int | direct value | — |
| FText | Python string (auto-converted) | — |

### Enum Properties

Compare directly against `unreal.EnumName` values. Note that Python enum members use **UPPER_SNAKE_CASE** (e.g. `H_ALIGN_FILL`), not the C++ name (`HAlign_Fill`):

```python
# HorizontalAlignment
self.check_equal(
    slot.get_editor_property("HorizontalAlignment"),
    unreal.HorizontalAlignment.H_ALIGN_FILL)

# VerticalAlignment
self.check_equal(
    slot.get_editor_property("VerticalAlignment"),
    unreal.VerticalAlignment.V_ALIGN_CENTER)

# SlateSizeRule (nested in Size struct)
self.check_equal(
    size.get_editor_property("SizeRule"),
    unreal.SlateSizeRule.FILL)

# ButtonClickMethod
self.check_equal(
    button.get_editor_property("ClickMethod"),
    unreal.ButtonClickMethod.DOWN_AND_UP)

# SlateVisibility
self.check_equal(
    widget.get_editor_property("Visibility"),
    unreal.SlateVisibility.VISIBLE)
```

### Nested Struct Access

Chain `get_editor_property()` for nested structs:

```python
# Slot → Size → SizeRule
slot = widget.slot
size = slot.get_editor_property("Size")
rule = size.get_editor_property("SizeRule")

# LayoutData → Anchors → Minimum
layout = slot.get_editor_property("LayoutData")
min_pt = layout.get_editor_property("Anchors").get_editor_property("Minimum")
min_pt.x  # FVector2D (double): lowercase
```
