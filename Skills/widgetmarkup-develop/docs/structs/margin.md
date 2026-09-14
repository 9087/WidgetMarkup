# FMargin

4-way margin/padding with float components.

**Format:** `"Left,Top,Right,Bottom"`.

**Member defaults** (`FMargin()`): `Left` / `Top` / `Right` / `Bottom` are all `0`.

| Example | Meaning |
|---|---|
| `"4,4,4,4"` | Uniform 4px on all sides |
| `"8,2,8,2"` | 8px horizontal, 2px vertical |
| `"0.25,0.25,0.25,0.25"` | 9-slice margins for brush |

Also used as `FAnchorData.Offsets` on CanvasPanel slots, where values represent Position X, Y, Width, Height instead of margins.

> **Reading back in Python:** Access via `.left`, `.top`, `.right`, `.bottom`:
> ```python
> pad = slot.get_editor_property("Padding")
> pad.left, pad.top, pad.right, pad.bottom  # float values
> ```

> **XML nesting:** Child-element syntax (`<Padding><Left>4</Left><Top>4</Top><Right>4</Right><Bottom>4</Bottom></Padding>`) is supported via `FPropertyPathResolver`. The `"L,T,R,B"` string format is typically more concise.
