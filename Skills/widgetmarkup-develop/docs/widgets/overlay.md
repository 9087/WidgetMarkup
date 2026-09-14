# Overlay

Layers children via `UOverlaySlot`. Defaults below are UMG's
(`UOverlaySlot::UOverlaySlot`) — note the alignment is **not** `Fill`, so a child keeps
its desired size in the top-left corner unless you say otherwise.

| Slot Property | Type | Default | Description |
|---|---|---|---|
| `Padding` | `FMargin` | `0` | Internal spacing |
| `HorizontalAlignment` | `EHorizontalAlignment` | `HAlign_Left` | Content H-align |
| `VerticalAlignment` | `EVerticalAlignment` | `VAlign_Top` | Content V-align |
