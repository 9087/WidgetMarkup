# ComboBoxString

`UComboBoxString : UWidget` — Dropdown selector. No children.

Inherits from: [shared-properties.md](shared-properties.md)

| Attribute | Type | Default | Description |
|---|---|---|---|
| `DefaultOptions` | container (child elements) | (empty) | Dropdown options — a string value is a compile error, see SKILL.md §3.4 |
| `SelectedOption` | `FString` | (empty) | Selected item |
| `MaxListHeight` | `float` | `450` | Dropdown max height |
| `HasDownArrow` | `bool` | `true` | Show arrow indicator |

**Delegate:**

| Delegate | Signature |
|---|---|
| `OnSelectionChanged` | `(FString SelectedItem, ESelectInfo::Type SelectionType)` |

```xml
<ComboBoxString SelectedOption="Option A" OnSelectionChanged="on_item_selected">
  <DefaultOptions>
    <String>Option A</String>
    <String>Option B</String>
    <String>Option C</String>
  </DefaultOptions>
</ComboBoxString>
```
