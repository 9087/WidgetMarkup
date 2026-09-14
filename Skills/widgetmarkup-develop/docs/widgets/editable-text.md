# EditableText / EditableTextBox / MultiLine Variants

| Variant | UMG Class | Notes |
|---|---|---|
| `<EditableText>` | `UEditableText : UWidget` | Single-line, plain |
| `<EditableTextBox>` | `UEditableTextBox` | Adds `Style`, `HasTickMarks`, `ReadOnlyForegroundColor` |
| `<MultiLineEditableText>` | `UMultiLineEditableText` | Multi-line, plain |
| `<MultiLineEditableTextBox>` | `UMultiLineEditableTextBox` | Adds `AutoWrapText`, `Style` |

Inherits from: [shared-properties.md](shared-properties.md)

**Common attributes:**

| Attribute | Type | Default | Description |
|---|---|---|---|
| `Text` | `FText` | (empty) | Current text, supports `{binding}` |
| `HintText` | `FText` | (empty) | Placeholder text |
| `IsReadOnly` | `bool` | `false` | Prevent editing |
| `IsPassword` | `bool` | `false` | Mask characters |
| `MinimumDesiredWidth` | `float` | `0` | Min width |
| `SelectAllTextWhenFocused` | `bool` | `false` | Auto-select on focus |

**Delegates:**

| Delegate | Signature |
|---|---|
| `OnTextChanged` | `(FText Text)` |
| `OnTextCommitted` | `(FText Text, ETextCommit::Type CommitMethod)` |

```xml
<EditableText Text="{input_text}" HintText="Enter name..."
              OnTextChanged="on_name_changed" />
```
