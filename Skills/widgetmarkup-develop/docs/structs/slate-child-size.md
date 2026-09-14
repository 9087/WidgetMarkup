# FSlateChildSize

Controls how much space a slot occupies in the direction of its parent panel. Used by VerticalBox, HorizontalBox, ScrollBox, and StackBox slots.

| Property | Type | Default | Description |
|---|---|---|
| `Size.SizeRule` | `ESlateSizeRule` | `Fill`\* | `Automatic` — follow child's desired size; `Fill` — use `Size.Value` |
| `Size.Value` | `float` | `1` | Proportional size when `Fill` (e.g., `1` = one share of remaining space) |

```xml
<VerticalBox>
  <!-- Auto height: slot = child height -->
  <TextBlock Text="Header">
    <Slot Size.SizeRule="Automatic" />
  </TextBlock>
  <!-- Fill: takes remaining space -->
  <TextBlock Text="Content">
    <Slot Size.SizeRule="Fill" Size.Value="1" />
  </TextBlock>
</VerticalBox>
```

> **XML nesting:** Child-element syntax (`<Size><SizeRule>Automatic</SizeRule></Size>`) is supported via `FPropertyPathResolver`. Dotted attributes (`Size.SizeRule="Automatic"`) are typically more concise.

\* The struct itself defaults to `Fill`. The box slot classes construct it as `Automatic` (`UVerticalBoxSlot`, `UHorizontalBoxSlot`, `UScrollBoxSlot` …), so a child that sets nothing follows its own content inside those panels.
