# SizeBox

`USizeBox : UContentWidget` — Constrains child to fixed dimensions. Single child.

Inherits from: [shared-properties.md](shared-properties.md)

| Attribute | Type | Default | Description |
|---|---|---|---|
| `WidthOverride` | `float` | `0` | Fixed width |
| `HeightOverride` | `float` | `0` | Fixed height |
| `MinDesiredWidth` | `float` | `0` | Minimum width |
| `MinDesiredHeight` | `float` | `0` | Minimum height |
| `MaxDesiredWidth` | `float` | `0` | Maximum width |
| `MaxDesiredHeight` | `float` | `0` | Maximum height |

```xml
<SizeBox WidthOverride="200" HeightOverride="100">
  <TextBlock Text="Constrained" />
</SizeBox>
```

**Slot** (`USizeBoxSlot`): `Padding` ([FMargin](../structs/margin.md)) — default **`0,0`**; [Alignment](../structs/alignment.md) — default **`Fill` / `Fill`**. See [panels.md](panels.md#sizebox-slot).
