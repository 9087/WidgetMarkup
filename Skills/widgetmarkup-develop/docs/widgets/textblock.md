# TextBlock

`UTextBlock : UTextLayoutWidget` — Static or bound text display. No children.

Inherits from: [shared-properties.md](shared-properties.md)

| Attribute | Type | Default | Description |
|---|---|---|---|
| `Text` | `FText` | (empty) | Display text, supports `{binding}` |
| `ColorAndOpacity` | [FSlateColor](../structs/slate-color.md) | `1,1,1,1` | Text color |
| `Font.FontObject` | asset path | `/Engine/EngineFonts/Roboto.Roboto` | Font face asset |
| `Font.Size` | `int32` | `24` | Font size in points. See [FSlateFontInfo](../structs/font-info.md) |
| `Font.TypefaceFontName` | `FName` | `Bold` | Typeface variant |
| `ShadowOffset` | [FVector2D](../structs/vector2d.md) | `1,1` | Drop shadow offset |
| `ShadowColorAndOpacity` | [FLinearColor](../structs/linear-color.md) | `0,0,0,0` | Drop shadow color |
| `MinDesiredWidth` | `float` | `0` | Minimum width before wrapping |
| `AutoWrapText` | `bool` | `false` | Enable text wrapping |
| `TextTransformPolicy` | `ETextTransformPolicy` | `None` | `None`, `ToLower`, `ToUpper` |
| `TextOverflowPolicy` | `ETextOverflowPolicy` | `Clip` | `Clip`, `Ellipsis` |
| `Justification` | `ETextJustify` | `Left` | `Left`, `Center`, `Right` |

```xml
<TextBlock Name="Title" Text="{title}" ColorAndOpacity="{title_color}"
           Font.Size="24" Justification="Center" />
```
