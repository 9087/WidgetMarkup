# Button

`UButton : UContentWidget` — Clickable button with a single child.

Inherits from: [shared-properties.md](shared-properties.md)

| Attribute | Type | Default | Description |
|---|---|---|---|
| `ColorAndOpacity` | [FLinearColor](../structs/linear-color.md) | `1,1,1,1` | Content color multiplier |
| `BackgroundColor` | [FLinearColor](../structs/linear-color.md) | `1,1,1,1` | Background color multiplier |
| `ClickMethod` | `EButtonClickMethod` | `DownAndUp` | `DownAndUp`, `MouseDown`, `MouseUp`, `PreciseClick` |
| `TouchMethod` | `EButtonTouchMethod` | `DownAndUp` | `DownAndUp`, `Down`, `PreciseTap` |
| `PressMethod` | `EButtonPressMethod` | `DownAndUp` | `DownAndUp`, `ButtonPress`, `ButtonRelease` |
| `IsFocusable` | `bool` | `true` | Keyboard focusable |

**Delegates:**

| Delegate | Signature |
|---|---|
| `OnClicked` | `()` |
| `OnPressed` | `()` |
| `OnReleased` | `()` |
| `OnHovered` | `()` |
| `OnUnhovered` | `()` |

```xml
<Button Name="MyButton" OnClicked="on_button_clicked" OnHovered="on_hover">
  <TextBlock Text="Click Me" />
</Button>
```

**Slot** (`UButtonSlot`, created for the single child): `Padding` ([FMargin](../structs/margin.md)) — default **`4,2`**; [Alignment](../structs/alignment.md) — default **`Center` / `Center`**. Slate's `SButton` fills its content instead, which is why the same label can end up 1 px lower in UMG — see [panels.md](panels.md#border--button--backgroundblur).
