# CanvasPanel

Free-form positioning via `UCanvasPanelSlot`. Defaults below are UMG's
(`UCanvasPanelSlot::UCanvasPanelSlot`).

| Slot Property | Type | Default | Description |
|---|---|---|---|
| `LayoutData.Anchors.Minimum` | `FVector2D` | `0,0` | Anchor corner (0–1) |
| `LayoutData.Anchors.Maximum` | `FVector2D` | `0,0` | Anchor corner (0–1) |
| `LayoutData.Alignment` | `FVector2D` | `0,0` | Pivot alignment (0–1) |
| `LayoutData.Offsets` | `FMargin` | `0,0,100,30` | Position X, Y, Width, Height in pixels |
| `bAutoSize` | `bool` | `false` | Slot size follows child's desired size (editor: "Size To Content") |
| `ZOrder` | `int32` | `0` | Render order, higher = on top |

With `Anchors.Minimum == Anchors.Maximum` the offsets mean "position + size"; with
different anchors they are insets from the anchor rectangle.

```xml
<CanvasPanel>
  <TextBlock Text="Top Left"
    Slot.LayoutData.Anchors.Minimum="0,0"
    Slot.LayoutData.Anchors.Maximum="0,0"
    Slot.LayoutData.Alignment="0,0"
    Slot.LayoutData.Offsets="10,10,100,20" />
  <TextBlock Text="Auto-sized">
    <Slot bAutoSize="True"
          LayoutData.Anchors.Minimum="0.5,0.5"
          LayoutData.Anchors.Maximum="0.5,0.5"
          LayoutData.Alignment="0.5,0.5" />
  </TextBlock>
</CanvasPanel>
```
