# FSlateBrush

Describes how to draw a widget background/image: what texture/material, how to stretch it, tint color, and margins.

Defaults below are `FSlateBrush()`'s, i.e. what an image/border/fill brush has before any markup or style setter touches it.

**Attributes (set via dotted paths like `Brush.ResourceObject`):**

| Path | Type | Default | Description |
|---|---|---|---|
| `Brush.ResourceObject` | asset path | `None` | Texture2D, Material, or other UObject |
| `Brush.ResourceName` | asset path | `None` | The string form of the same thing (prefer `ResourceObject`) |
| `Brush.DrawAs` | enum | `Image` | `NoDrawType`, `Box`, `Border`, `Image`, `RoundedBox` |
| `Brush.ImageType` | enum | `NoImage` | `NoImage`, `FullColor`, `Vector`, `Linear` |
| `Brush.Tiling` | enum | `NoTile` | `NoTile`, `Horizontal`, `Vertical`, `Both` |
| `Brush.Mirroring` | enum | `NoMirror` | `NoMirror`, `Horizontal`, `Vertical`, `Both` |
| `Brush.ImageSize` | `"W,H"` | `32,32` | Desired image size in pixels ([FVector2D](vector2d.md)) |
| `Brush.Margin` | `"L,T,R,B"` | `0,0,0,0` | 9-slice margins ([FMargin](margin.md)) — normalized UV units for `Box` brushes |
| `Brush.TintColor` | `"R,G,B,A"` | `1,1,1,1` | Tint color ([FSlateColor](slate-color.md)) |
| `Brush.OutlineSettings.Width` | `float` | `0` | Outline thickness — only drawn for `DrawAs="RoundedBox"` |
| `Brush.OutlineSettings.CornerRadii` | `"X,Y,Z,W"` | `0,0,0,0` | Corner radius (TL, TR, BR, BL) |
| `Brush.OutlineSettings.RoundingType` | enum | `HalfHeightRadius` | `FixedRadius` when you want `CornerRadii` to be used literally |
| `Brush.OutlineSettings.Color` | `"R,G,B,A"` | `0,0,0,0` | Outline colour |
| `Brush.OutlineSettings.bUseBrushTransparency` | `bool` | `false` | Multiply the outline by the brush's tint alpha |

> **`Box` brushes and `Margin`:** for `DrawAs="Box"` the nine-slice geometry is computed from
> the *texture* size (`ElementBatcher.cpp`: `TextureWidth * Margin.Left`), so `Margin` is a
> normalized UV fraction (e.g. `2.0f / 8.0f` = `0.25` for an 8 px border in an 8 px texture).
> `ImageSize` does not affect the geometry — only the desired size.
>
> **Only `RoundedBox` draws an outline.** `OutlineSettings` is ignored for every other draw
> type, so a leftover `Width=1` is harmless there but reappears the moment you switch a brush
> back to `RoundedBox`.

```xml
<Image Brush.ResourceObject="/Game/Textures/MyIcon.MyIcon"
       Brush.ImageSize="64,64"
       Brush.TintColor="1,1,1,1" />
```

Used by: [Image](../widgets/image.md) (`Brush`), [Border](../widgets/border.md) (`Background`), and every `WidgetStyle.*` brush in a style sheet.

> **XML nesting:** Child-element syntax (`<Brush><ResourceObject>/Game/Textures/Icon</ResourceObject></Brush>`) is supported via `FPropertyPathResolver`. Dotted attributes (`Brush.ResourceObject="..."`) are typically more concise.
