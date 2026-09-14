# Border

`UBorder : UContentWidget` — Container with background brush and single child.

Inherits from: [shared-properties.md](shared-properties.md)

See [FSlateBrush](../structs/slate-brush.md) for `Background.*` attribute details.

| Attribute | Type | Default | Description |
|---|---|---|---|
| `Background.ResourceObject` | asset path | `None` | Background texture |
| `Background.DrawAs` | `ESlateBrushDrawType` | `Image` | See [Image](image.md) |
| `BrushColor` | [FLinearColor](../structs/linear-color.md) | `1,1,1,1` | Background tint |
| `ContentColorAndOpacity` | [FLinearColor](../structs/linear-color.md) | `1,1,1,1` | Child content color multiplier |
| `Padding` | [FMargin](../structs/margin.md) | `4,2,4,2` | `"4,4,4,4"` |
| `HorizontalAlignment` | [EHorizontalAlignment](../structs/alignment.md) | `HAlign_Fill` | Child H-align |
| `VerticalAlignment` | [EVerticalAlignment](../structs/alignment.md) | `VAlign_Fill` | Child V-align |
| `OnMouseButtonDownEvent` | delegate | — | Pointer pressed (left/right/middle) |
| `OnMouseButtonUpEvent` | delegate | — | Pointer released |

**Pointer events on Border:** Unlike `Button` (which only supports primary-click `OnClicked`), `Border` supports `OnMouseButtonDownEvent` and `OnMouseButtonUpEvent` for per-button mouse handling. This enables right-click actions, chord detection (simultaneous left+right), and drag-aware interactions with boundary checking. See [unreal-python-api.md](../unreal-python-api.md) for event reply, capture/release, and geometry boundary patterns.

```xml
<Border Padding="8,8" BrushColor="0.1,0.1,0.1,1">
  <TextBlock Text="Inside border" />
</Border>
```

**Slot** (`UBorderSlot`): `Padding` ([FMargin](../structs/margin.md)) — default **`4,2`**; [Alignment](../structs/alignment.md) — default **`Fill` / `Fill`**. See [panels.md](panels.md#border--button--backgroundblur).
