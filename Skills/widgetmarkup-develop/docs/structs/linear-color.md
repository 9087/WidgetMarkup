# FLinearColor

RGBA color with float components (0–1). Used by most color properties.

**Format:** `"R,G,B,A"` or `"R,G,B"` (A defaults to 1.0).

```
"1,0,0,1"     → Red, fully opaque
"0,0.5,0"     → Dark green, opaque (A=1.0)
"{my_color}"  → Bind to @reactive property returning unreal.LinearColor
```

**Python:** Use `unreal.LinearColor(r, g, b, a)`.

**Member defaults** (`FLinearColor()`, and therefore any widget colour property you never set): `R` / `G` / `B` / `A` are all `0` — transparent black. Note the string form differs: `"R,G,B"` goes through the converter, which fills `A` with `1.0`.

> **Reading back in Python:** Access via `.r`, `.g`, `.b`, `.a`:
> ```python
> co = widget.get_editor_property("ColorAndOpacity")
> co.r, co.g, co.b, co.a  # float values
> ```

> **XML nesting:** Child-element syntax (`<ColorAndOpacity><R>1</R><G>0</G><B>0</B><A>1</A></ColorAndOpacity>`) is supported via `FPropertyPathResolver`.
