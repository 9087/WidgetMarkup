# ListView / TileView

Scrollable item lists. See [python-components.md](../python-components.md) for full ListView setup with entry widgets and Python components.

Inherits from: [shared-properties.md](shared-properties.md)

| Attribute | Type | Default | Description |
|---|---|---|---|
| `ListItems` | `{binding}` or child elements | (empty) | List items — a reactive binding, or objects declared inline |
| `EntryWidgetClass` | `/Path/To/Entry` | `None` | Entry blueprint path |
| `Orientation` | `EOrientation` | `Orient_Vertical` | Scroll direction |
| `SelectionMode` | `ESelectionMode` | `Single` | `Single`, `SingleToggle`, `Multi`, `None` |
| `EntrySpacing` | `float` | `0` | Spacing between entries (both axes) |
| `HorizontalEntrySpacing` / `VerticalEntrySpacing` | `float` | `0` | Per-axis spacing; `-1` falls back to `EntrySpacing` |
| `ConsumeMouseWheel` | `EConsumeMouseWheel` | `WhenScrollingPossible` | Whether the list swallows the wheel event |
| `bClearSelectionOnClick` | `bool` | `false` | Clear the selection when clicking empty space |
| `bIsFocusable` | `bool` | `true` | Keyboard focusable |
| `bReturnFocusToSelection` | `bool` | `false` | Return focus to the selected entry |

> **`ScrollBarVisibility` is not an attribute.** `UListViewBase` only exposes the Blueprint
> function `SetScrollbarVisibility()`; there is no reflected property, and markup attributes
> resolve properties only. Use the widget's `ScrollBarStyle` / scrolling flags instead, or
> call the setter from Python.

**Binding** (the usual form — see [python-components.md](../python-components.md)):

```xml
<ListView ListItems="{items}" EntryWidgetClass="/Game/MyEntry" />
```

**Inline items** — `<ListItems>` child elements become the items. The element tag is a
UClass name (case-sensitive, without the `U` prefix), so this needs a data object class;
element data is treated as an asset path to reference an existing object instead of
creating one:

```xml
<ListView Name="MyList" EntryWidgetClass="/Game/MyEntry">
  <ListItems>
    <MyDataObject Name="First" />
    <MyDataObject Name="Second" />
  </ListItems>
</ListView>
```

> `ListItems` is transient runtime state: a widget instance does not inherit it from the
> template. Items declared inline are therefore captured while the blueprint is compiled
> and applied to every instance through the style sheet. For the same reason a string
> value cannot be used — `ListItems="…"` is a compile error, since an object array has no
> string representation.
