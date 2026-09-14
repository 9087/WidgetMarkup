# ESlateVisibility

Controls widget visibility and hit-testing.

| Value | Visible | Hit-testable | Occupies space |
|---|---|---|---|
| `Visible` | Yes | Yes | Yes |
| `Collapsed` | No | No | No |
| `Hidden` | No | No | Yes |
| `HitTestInvisible` | Yes | No (children can) | Yes |
| `SelfHitTestInvisible` | Yes | No (self only) | Yes |

> **Default when unset:** `Visible` — every `UWidget` starts visible, and the two widgets that
> change it in their constructor are `UNamedSlot` and `USpacer`, which set
> `SelfHitTestInvisible`. A `ScrollBox`'s `ScrollBarVisibility` also defaults to `Visible`.

```xml
<TextBlock Text="Can be hidden" Visibility="Collapsed" />
```
