# ListView / TileView

Scrollable item lists. See [python-components.md](../python-components.md) for full ListView setup with entry widgets and Python components.

Inherits from: [shared-properties.md](shared-properties.md)

| Attribute | Type | Description |
|---|---|---|
| `ListItems` | `{binding}` or child elements | List items — a reactive binding, or objects declared inline |
| `EntryWidgetClass` | `/Path/To/Entry` | Entry blueprint path |
| `Orientation` | `EOrientation` | Scroll direction |
| `ScrollBarVisibility` | `ESlateVisibility` | Scroll bar visibility |

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
