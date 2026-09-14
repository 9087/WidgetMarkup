# FSlateFontInfo

Font configuration for text widgets.

Defaults below are `FSlateFontInfo()`'s — what a text widget has before any markup or style setter touches it.

| Path | Type | Default | Description |
|---|---|---|---|
| `Font.FontObject` | asset path | `None` | Font face asset (e.g., `/Engine/EngineFonts/Roboto.Roboto`) |
| `Font.TypefaceFontName` | `FName` | `None` | Typeface variant within the font (e.g., `"Bold"`, `"Italic"`) |
| `Font.Size` | `int32` | **`24`** | Font size in points |
| `Font.LetterSpacing` | `int32` | `0` | Extra tracking (1/1000 em) |
| `Font.SkewAmount` | `float` | `0` | Italic skew |
| `Font.bForceMonospaced` | `bool` | `false` | Force monospaced digits |
| `Font.MonospacedWidth` | `float` | `1` | Digit advance when forced |
| `Font.Hinting` | enum | `Default` | Font hinting mode |
| `Font.OutlineSettings.OutlineSize` | `int32` | `0` | Text outline thickness |
| `Font.OutlineSettings.OutlineColor` | `"R,G,B,A"` | `0,0,0,1` | Text outline colour |
| `Font.FontMaterial` | asset path | `None` | Material override |

```xml
<TextBlock Text="Hello" Font.Size="24" Font.TypefaceFontName="Bold" />
```

Used by: [TextBlock](../widgets/textblock.md)

> **XML nesting:** Child-element syntax (`<Font><Size>18</Size></Font>`) is supported via `FPropertyPathResolver`.
