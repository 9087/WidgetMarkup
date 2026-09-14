# ProgressBar

`UProgressBar : UWidget` — Fill bar 0..1. No children.

Inherits from: [shared-properties.md](shared-properties.md)

| Attribute | Type | Default | Description |
|---|---|---|---|
| `Percent` | `float` | `0` | Fill value 0.0–1.0, supports `{binding}` |
| `FillColorAndOpacity` | [FLinearColor](../structs/linear-color.md) | `1,1,1,1` | Fill color |
| `BarFillType` | `EProgressBarFillType` | `LeftToRight` | `LeftToRight`, `RightToLeft`, `TopToBottom`, `BottomToTop` |
| `BarFillStyle` | `EProgressBarFillStyle` | `Mask` | `Mask`, `Scale` |
| `bIsMarquee` | `bool` | `false` | Indeterminate marquee animation |
| `BorderPadding` | [FVector2D](../structs/vector2d.md) | `0,0` | Inner padding |

```xml
<ProgressBar Percent="{progress}" FillColorAndOpacity="{bar_color}" />
```
