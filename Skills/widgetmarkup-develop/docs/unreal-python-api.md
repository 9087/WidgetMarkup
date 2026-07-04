# unreal Python API (WidgetMarkup)

WidgetMarkup components run with UE's Python bindings exposed by the **WidgetMarkup** plugin. The subset of `unreal` types and libraries available at runtime differs from the full Editor — do not assume Editor documentation applies verbatim.

## `widget_markup` vs `unreal`

Most APIs live on **`unreal`**: widget types, delegates, `unreal.WidgetLibrary`, `unreal.SystemLibrary`, and the rest of the UE Python surface. **`widget_markup`** is a thin extension for WidgetMarkup-specific helpers and workarounds.

**Lookup order:** check `widget_markup` only for APIs listed in [widget_markup native module](#widget_markup-native-module) below (or called out elsewhere in this doc). If a function or type is not there, use native **`unreal`** — but confirm names against [ScriptName](#scriptname-vs-c-class-name) rules; WidgetMarkup does not expose the full Editor API.

Examples on `unreal`: `unreal.WidgetLibrary.handled()`, `unreal.UserWidget`, `unreal.SystemLibrary.print_string`. Prefer `widget_markup` only where this doc says so (e.g. `InputLibrary` / `Key` for pointer events in delegate handlers).

## ScriptName vs C++ class name

Blueprint function libraries are exported under their `ScriptName` meta, **not** the C++ class name:

| C++ class | Python module on `unreal` |
|---|---|
| `UKismetInputLibrary` | `unreal.InputLibrary` |
| `UWidgetBlueprintLibrary` | `unreal.WidgetLibrary` |
| `UKismetSystemLibrary` | `unreal.SystemLibrary` |

`unreal.KismetInputLibrary` and `unreal.WidgetBlueprintLibrary` are **`None`** in WidgetMarkup. `unreal.load_class(None, "/Script/Engine.KismetInputLibrary")` returns a `UClass` but does **not** expose static UFUNCTIONs as Python methods — use the `ScriptName` entry on `unreal` instead.

## PointerEvent — prefer `widget_markup.InputLibrary`

`unreal.PointerEvent` is passed to `OnMouseButtonDownEvent` handlers, but **`effecting_button` is not a readable Python property** (the underlying `FPointerEvent::EffectingButton` member is not exported to reflection).

In widget delegate handlers, **prefer** `widget_markup.InputLibrary` for reading pointer input:

```python
import widget_markup

button_key = widget_markup.InputLibrary.pointer_event_get_effecting_button(mouse_event)
button_name = str(button_key.key_name)  # e.g. "LeftMouseButton", "RightMouseButton"

is_right = widget_markup.InputLibrary.pointer_event_is_mouse_button_down(
    mouse_event, widget_markup.Key.RightMouseButton
)
```

`pointer_event_is_mouse_button_down` accepts an `unreal.Key` instance, a `widget_markup.Key` constant, or a key **name string** (the `FName` used by `EKeys`, e.g. `"LeftMouseButton"`).

These helpers mirror `unreal.InputLibrary` pointer-event APIs and are tailored for the WidgetMarkup delegate workflow. `mouse_event.get_editor_property("effecting_button")` is also unavailable.

> **Why the wrapper?** UE's `FPointerEvent::EffectingButton` is not exposed to Python reflection, so WidgetMarkup provides `UWidgetMarkupInputLibrary` (C++) with `UPARAM(ref)` to correctly marshal `FKey` and `FPointerEvent` between C++ and Python. The `widget_markup.InputLibrary` Python module wraps this C++ class. When writing `OnMouseButtonDownEvent` handlers, always use `widget_markup.InputLibrary` for reading button state — do NOT attempt `mouse_event.effecting_button` or `mouse_event.get_editor_property(...)`.

## FKey — use `widget_markup.Key` for `EKeys` constants

`unreal.Key` wraps the `FKey` **struct** (`key_name: FName`). C++ defines well-known keys as `EKeys::LeftMouseButton`, `EKeys::RightMouseButton`, and so on — static `FKey` values, **not** a `UENUM`. UE does not export them on `unreal.Key`.

WidgetMarkup provides **`widget_markup.Key`**: a namespace class whose attributes mirror `EKeys` member names. Each value is an `unreal.Key` ready for `InputLibrary` helpers or direct comparison:

```python
import widget_markup

button_key = widget_markup.InputLibrary.pointer_event_get_effecting_button(mouse_event)
if button_key == widget_markup.Key.RightMouseButton:
    self.flag_cell()
elif button_key == widget_markup.Key.LeftMouseButton:
    self.reveal_cell()

widget_markup.InputLibrary.pointer_event_is_mouse_button_down(
    mouse_event, widget_markup.Key.LeftMouseButton
)
```

`widget_markup.Key` cannot be instantiated. VR-controller keys from `EKeys` are not exported yet; use a key name string if needed.

## WidgetLibrary for event replies

Return a handled reply from pointer-event handlers:

```python
return unreal.WidgetLibrary.handled()
```

Not `unreal.WidgetBlueprintLibrary.handled()`.

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
def on_item_tile_mouse_down(self, geometry: unreal.Geometry, mouse_event: unreal.PointerEvent):
    button_key = widget_markup.InputLibrary.pointer_event_get_effecting_button(mouse_event)
    if button_key == widget_markup.Key.RightMouseButton:
        self.open_item_context_menu()
    elif button_key == widget_markup.Key.LeftMouseButton:
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

Widget tree lookup and list-entry data access.

- **`find_widget_in_user_widget(user_widget, name)`** — find a named widget in the user widget tree.
- **`get_python_object_from_list_item(list_item)`** — get the Python data object from a `PythonWidgetMarkupListItem`.

### `widget_markup.Application`

WidgetMarkupApp launcher utilities (CLI and process control). Only relevant when running the standalone `.exe`.

- **`get_extra_arguments()`** — extra command-line arguments passed after the blueprint path.
- **`request_shutdown()`** — request process exit (used by automated tests).

### `widget_markup.Key`

`EKeys` constants as `unreal.Key` class attributes (keyboard, mouse, gamepad, platform). Not instantiable.

- **`LeftMouseButton`**, **`RightMouseButton`**, **`MiddleMouseButton`**, … — mouse buttons
- **`A`** … **`Z`**, **`Zero`** … **`Nine`**, **`F1`** … **`F12`**, arrow keys, modifiers, … — keyboard
- **`Gamepad_FaceButton_Bottom`**, **`Gamepad_DPad_Up`**, … — gamepad
- See `PythonKey.cpp` for the full exported list

### `widget_markup.InputLibrary`

Pointer-event helpers safe to call from widget delegate handlers (see [PointerEvent](#pointerevent--prefer-widget_markupinputlibrary) above).

- **`pointer_event_get_effecting_button(mouse_event)`** — effecting button as `unreal.Key`.
- **`pointer_event_is_mouse_button_down(mouse_event, key)`** — whether a button is down; `key` is an `unreal.Key`, a `widget_markup.Key` constant, or a key name string.

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
