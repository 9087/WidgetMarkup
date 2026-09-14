# FSlateColor

A color that can be either a literal `FLinearColor` or a reference to a color from the widget style. Used for text and brush tint colors.

**Format:** Same as [FLinearColor](linear-color.md) — `"R,G,B,A"`.

```
"1,1,1,1"     → White (from FLinearColor)
"{tint}"      → Bind to @reactive property returning unreal.SlateColor
```

**Python:** Use `unreal.SlateColor(unreal.LinearColor(r, g, b, a))`. Do NOT use bare `unreal.LinearColor` — the binding expects `SlateColor`.

**Default value:** a default-constructed `FSlateColor` is the engine's "unset" sentinel — `SpecifiedColor = 1,0,1,1` (magenta) with `ColorUseRule = UseColor_Specified`. Widget styles always write it explicitly, so if you read that magenta back it means nobody set the property, not that the colour is magenta.

> **Reading back in Python:** Same as [FLinearColor](linear-color.md) — use `.r`, `.g`, `.b`, `.a`.

> **XML nesting:** Child-element syntax (`<ColorAndOpacity><R>1</R><G>0</G><B>0</B><A>1</A></ColorAndOpacity>`) is supported via `FPropertyPathResolver`.
