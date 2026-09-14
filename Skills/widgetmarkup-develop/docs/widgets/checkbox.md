# CheckBox

`UCheckBox : UContentWidget` — Toggle checkbox/toggle button with optional child.

Inherits from: [shared-properties.md](shared-properties.md)

| Attribute | Type | Default | Description |
|---|---|---|---|
| `CheckedState` | `ECheckBoxState` | `Unchecked` | `Unchecked`, `Checked`, `Undetermined`. Bind a Python string holding the value name: `CheckedState="{state}"` |
| `HorizontalAlignment` | [EHorizontalAlignment](../structs/alignment.md) | `HAlign_Fill` | Content H-align |
| `ClickMethod` | `EButtonClickMethod` | `DownAndUp` | See [Button](button.md) |
| `IsFocusable` | `bool` | `true` | Keyboard focusable |

> **There is no `IsChecked` attribute.** `UCheckBox` only exposes `IsChecked()` / `SetIsChecked()` as Blueprint functions; markup attributes resolve *properties*, so `IsChecked="…"` fails to compile:
> `PropertyPathResolver: FindPropertyByName FAILED for 'IsChecked'` → `CompileFromSourceCode failed`. Use `CheckedState`.

**Delegate:**

| Delegate | Signature |
|---|---|
| `OnCheckStateChanged` | `(bool bIsChecked)` |

```xml
<CheckBox CheckedState="{checked_state}" OnCheckStateChanged="on_toggle">
  <TextBlock Text="Enable Feature" />
</CheckBox>
```
