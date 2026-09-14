# VerticalBox / HorizontalBox

Stack children vertically/horizontally via `UVerticalBoxSlot` / `UHorizontalBoxSlot`.
Both slot classes have the same defaults, from their constructors.

| Slot Property | Type | Default | Description |
|---|---|---|---|
| `Size.Value` | `float` | `1.0` | Size when SizeRule is `Fill` |
| `Size.SizeRule` | `ESlateSizeRule` | `Automatic` | `Automatic` (follow content) or `Fill` (use Value) |
| `Padding` | `FMargin` | `0` | `"Left,Top,Right,Bottom"` |
| `HorizontalAlignment` | `EHorizontalAlignment` | `HAlign_Fill` | `HAlign_Left`, `HAlign_Center`, `HAlign_Right`, `HAlign_Fill` |
| `VerticalAlignment` | `EVerticalAlignment` | `VAlign_Fill` | `VAlign_Top`, `VAlign_Center`, `VAlign_Bottom`, `VAlign_Fill` |

```xml
<VerticalBox>
  <TextBlock Text="Auto height">
    <Slot Size.SizeRule="Automatic" />
  </TextBlock>
  <TextBlock Text="Fill remainder">
    <Slot Size.SizeRule="Fill" Size.Value="1"
          Padding="4,2,4,2"
          HorizontalAlignment="HAlign_Center" />
  </TextBlock>
</VerticalBox>
```
