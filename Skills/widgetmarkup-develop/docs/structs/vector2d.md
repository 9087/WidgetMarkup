# FVector2D

2D vector with float components.

**Format:** `"X,Y"`.

**Member defaults** (`FVector2D()`): `X` = `0`, `Y` = `0`.

| Example | Meaning |
|---|---|
| `"10,20"` | X=10, Y=20 |
| `"0.5,0.5"` | Center pivot or half-scale |
| `"128,128"` | Image size |

**Python:** `unreal.Vector2D(x, y)`

> **Python access:** For FVector2D (double), use lowercase `.x`, `.y`. For FVector2f, use uppercase `.X`, `.Y`. See [unreal-python-api.md](../unreal-python-api.md#fvector2d--fvector2f) for details.
>
> **FDeprecateSlateVector2D:** This struct inherits from `FVector2f` but its `.X`/`.Y` members are not exposed to Python. To read values (e.g. `Brush.ImageSize`), use `export_text()` and substring-match the result.
>
> **XML nesting:** Child-element syntax (`<ShadowOffset><X>2</X><Y>2</Y></ShadowOffset>`) is supported via `FPropertyPathResolver`.
