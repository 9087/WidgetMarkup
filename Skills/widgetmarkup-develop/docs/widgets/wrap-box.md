# WrapBox

`UWrapBox : UPanelWidget` — Auto-wrapping flow layout.

Inherits from: [shared-properties.md](shared-properties.md)

| Attribute | Type | Default | Description |
|---|---|---|---|
| `WrapSize` | `float` | `500` | Line length before wrapping, used when `bExplicitWrapSize` is true |
| `bExplicitWrapSize` | `bool` | `false` | Use `WrapSize` instead of the allotted size |
| `InnerSlotPadding` | [FVector2D](../structs/vector2d.md) | `0,0` | Padding between slots |
| `HorizontalAlignment` | [EHorizontalAlignment](../structs/alignment.md) | `HAlign_Left` | Line alignment |
| `Orientation` | `EOrientation` | `Orient_Horizontal` | Wrap direction |

## Slot

Defaults are UMG's (`UWrapBoxSlot::UWrapBoxSlot`).

| Slot Property | Type | Default | Description |
|---|---|---|---|
| `Padding` | `FMargin` | `0` | Internal spacing |
| `bFillEmptySpace` | `bool` | `false` | Fill remaining space on the line |
| `bForceNewLine` | `bool` | `false` | Start on a new line before this child |
| `FillSpanWhenLessThan` | `float` | `0` | Fill entire line if available space drops below this threshold (0 = never) |
| `HorizontalAlignment` | `EHorizontalAlignment` | `HAlign_Fill` | Content H-align |
| `VerticalAlignment` | `EVerticalAlignment` | `VAlign_Fill` | Content V-align |
