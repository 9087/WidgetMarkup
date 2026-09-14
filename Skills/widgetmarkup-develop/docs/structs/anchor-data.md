# FAnchorData

Complete layout descriptor for a CanvasPanel slot, combining anchors, offsets, and alignment.

| Path | Type | Default | Description |
|---|---|---|
| `LayoutData.Anchors.Minimum` | `"X,Y"` | `0,0` | See [FAnchors](anchors.md) |
| `LayoutData.Anchors.Maximum` | `"X,Y"` | `0,0` | See [FAnchors](anchors.md) |
| `LayoutData.Offsets` | `"L,T,R,B"` | `0,0,0,0` | Position/Size as [FMargin](margin.md) |
| `LayoutData.Alignment` | `"X,Y"` | `0,0` | Pivot point (0–1) as [FVector2D](vector2d.md) |

```xml
<!-- 100x20 widget at top-left, 10px offset -->
<TextBlock Text="Hello"
  Slot.LayoutData.Anchors.Minimum="0,0"
  Slot.LayoutData.Anchors.Maximum="0,0"
  Slot.LayoutData.Alignment="0,0"
  Slot.LayoutData.Offsets="10,10,100,20" />
```

Used by: [CanvasPanel](../widgets/panels.md#canvaspanel)

`FAnchorData()` itself is all zeros. `UCanvasPanelSlot` overrides `Offsets` to `0,0,100,30` when it creates the slot — see [panels.md](../widgets/panels.md#panel-slots).
