# Border

`UBorder : UContentWidget` — Container with background brush and single child.

Inherits from: [shared-properties.md](shared-properties.md)

See [FSlateBrush](../structs/slate-brush.md) for `Background.*` attribute details.

| Attribute | Type | Description |
|---|---|---|
| `Background.ResourceObject` | asset path | Background texture |
| `Background.DrawAs` | `ESlateBrushDrawType` | See [Image](image.md) |
| `BrushColor` | [FLinearColor](../structs/linear-color.md) | Background tint |
| `ContentColorAndOpacity` | [FLinearColor](../structs/linear-color.md) | Child content color multiplier |
| `Padding` | [FMargin](../structs/margin.md) | `"4,4,4,4"` |
| `HorizontalAlignment` | [EHorizontalAlignment](../structs/alignment.md) | Child H-align |
| `VerticalAlignment` | [EVerticalAlignment](../structs/alignment.md) | Child V-align |
| `OnMouseButtonDownEvent` | delegate | Pointer pressed (left/right/middle) |
| `OnMouseButtonUpEvent` | delegate | Pointer released |

**Pointer events on Border:** Unlike `Button` (which only supports primary-click `OnClicked`), `Border` supports `OnMouseButtonDownEvent` and `OnMouseButtonUpEvent` for per-button mouse handling. This enables right-click actions, chord detection (simultaneous left+right), and drag-aware interactions with boundary checking. See [unreal-python-api.md](../unreal-python-api.md) for event reply, capture/release, and geometry boundary patterns.

```xml
<Border Padding="8,8" BrushColor="0.1,0.1,0.1,1">
  <TextBlock Text="Inside border" />
</Border>
```

**Slot:** Padding ([FMargin](../structs/margin.md)), [Alignment](../structs/alignment.md) — see [panels.md](panels.md#border--button--backgroundblur).
