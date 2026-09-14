# UniformGridPanel

`UUniformGridPanel : UPanelWidget` — Evenly-sized grid cells.

Inherits from: [shared-properties.md](shared-properties.md)

| Attribute | Type | Default | Description |
|---|---|---|---|
| `SlotPadding` | [FMargin](../structs/margin.md) | `0,0,0,0` | Padding added around every cell |
| `MinDesiredSlotWidth` | `float` | `0` | Minimum cell width |
| `MinDesiredSlotHeight` | `float` | `0` | Minimum cell height |

## Slot

Defaults below are UMG's (`UUniformGridSlot::UUniformGridSlot`) — the alignment is
**not** `Fill`, so children sit in the top-left of their cell unless you say otherwise.

| Slot Property | Type | Default | Description |
|---|---|---|---|
| `Row` | `int32` | `0` | Zero-based row index |
| `Column` | `int32` | `0` | Zero-based column index |
| `HorizontalAlignment` | `EHorizontalAlignment` | `HAlign_Left` | Content H-align |
| `VerticalAlignment` | `EVerticalAlignment` | `VAlign_Top` | Content V-align |
