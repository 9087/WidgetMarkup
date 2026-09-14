# Slider

`USlider : UWidget` — Numeric slider. No children.

Inherits from: [shared-properties.md](shared-properties.md)

| Attribute | Type | Default | Description |
|---|---|---|---|
| `Value` | `float` | `0` | Current value, supports `{binding}` |
| `MinValue` | `float` | `0` | Minimum value |
| `MaxValue` | `float` | `1` | Maximum value |
| `StepSize` | `float` | `0.01` | Keyboard/controller step |
| `Orientation` | `EOrientation` | `Orient_Horizontal` | `Orient_Horizontal`, `Orient_Vertical` |
| `SliderBarColor` | [FLinearColor](../structs/linear-color.md) | `1,1,1,1` | Bar color |
| `SliderHandleColor` | [FLinearColor](../structs/linear-color.md) | `1,1,1,1` | Handle color |
| `Locked` | `bool` | `false` | Disable interaction |
| `IndentHandle` | `bool` | `false` | Indent for handle fit |

**Delegate:**

| Delegate | Signature |
|---|---|
| `OnValueChanged` | `(float Value)` |

```xml
<Slider Value="{volume}" MinValue="0.0" MaxValue="1.0" StepSize="0.05"
        OnValueChanged="on_volume_changed" />
```
