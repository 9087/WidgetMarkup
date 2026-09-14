# Widget property defaults

UMG widgets ship with defaults of their own. They are applied when *no* markup attribute
and *no* style setter touches the property — which makes them the usual reason a widget
does not look like the Slate/editor style it is supposed to mirror.

Values below are the constructors `U*::U*()` in
`Runtime/UMG/Private/Components/*.cpp`, so they are the engine's, not this plugin's.

## Widgets whose look comes from UMG's default style set

`UButton`, `UCheckBox`, `UComboBoxString`, `UProgressBar`, … initialise `WidgetStyle`
from `UE::Slate::Private::FDefaultStyleCache` — i.e. **UMG's own style set**, not
`FAppStyle`/`FCoreStyle`. A style sheet that mirrors the Slate style therefore has to
override every brush it cares about; nothing is inherited from the Slate side.

| Widget | `WidgetStyle` source |
|---|---|
| `Button` | `FDefaultStyleCache::GetRuntime().GetButtonStyle()` (editor: `GetEditor()`) |
| `CheckBox` | `…GetCheckboxStyle()` |
| `ComboBoxString` | `…GetComboBoxStyle()` + `ItemStyle = …GetTableRowStyle()` |
| `ProgressBar` | `…GetProgressBarStyle()` |
| `Slider`, `SpinBox`, `EditableTextBox` | their own default styles, same pattern |

## Defaults that bite

| Widget | Property | Default | Why it matters |
|---|---|---|---|
| `TextBlock` | `Font` | `Roboto 24 Bold` | Not writing `Font.Size` gives 24 px; not writing `Font.TypefaceFontName` keeps **Bold**. The Slate `NormalText` is `Regular 10`. |
| `TextBlock` | `ShadowOffset` | `(1,1)` | `STextBlock::ComputeDesiredSize` adds the offset to the desired size, so a missing `ShadowOffset="0,0"` makes the box 1 px taller than the Slate style. |
| `TextBlock` | `ShadowColorAndOpacity` | `Transparent` (A=0) | The Slate `NormalText` uses an opaque black shadow; with offset `(0,0)` it is invisible either way. |
| `TextBlock` | `ColorAndOpacity` | `White` (specified) | Slate text styles often use `UseColor_Foreground` instead — write `ColorAndOpacity.ColorUseRule` when the text sits inside a button. |
| `TextBlock` | `TextOverflowPolicy` / `TextTransformPolicy` | `Clip` / `None` | Same as the Slate defaults; only relevant if you override them. |
| `ComboBoxString` | `Font` | `Roboto 16 Bold` | The visible option text is drawn by an inner `STextBlock` using this property, so a style sheet must set `Font.TypefaceFontName="Regular"` + `Font.Size` — otherwise the text is noticeably bolder than Slate's `Regular 10`. |
| `ComboBoxString` | `ForegroundColor` | `ItemStyle.TextColor` (UMG default `#C0C0C0`) | The Slate `SComboBox` tints its text with the button style's `NormalForeground`; UMG uses this **widget** property, so it must be set explicitly. |
| `ComboBoxString` | `ContentPadding` | `(4,2)` | Passed to `SComboBox` as the button content padding and therefore **overrides** `ComboButtonStyle.ContentPadding` — set it whenever you set the style's value. |
| `ComboBoxString` | `MaxListHeight` / `HasDownArrow` / `EnableGamepadNavigationMode` | `450` / `true` / `true` | — |
| `SpinBox` | `Font` | `Roboto 12 Bold` | Same trap as `ComboBoxString`. |
| `SpinBox` | `ForegroundColor` | `WidgetStyle.ForegroundColor` | Copied **once** at construction, so a style setter for `WidgetStyle.ForegroundColor` does not move it — set the widget property too. |
| `EditableTextBox` | `Font.CompositeFont` | `WidgetStyle.TextStyle.Font.CompositeFont` | The hint/typed text comes from `WidgetStyle.TextStyle`; leaving it empty falls back to the Slate default (~24 px) and overflows the box. |
| `CheckBox` | `CheckedState` | `Unchecked` | — |
| `Button` | `ColorAndOpacity` / `BackgroundColor` | `White` / `White` | Multiplied on top of the style's brushes. |
| `Slider` | `MinValue` / `MaxValue` / `StepSize` | `0` / `1` / `0.01` | — |
| `Slider` | `SliderBarColor` / `SliderHandleColor` | `White` / `White` | Type `ESliderBarFillType`/`EOrientation` defaults: `Orient_Horizontal`. |
| `ProgressBar` | `Percent` / `FillColorAndOpacity` / `BarFillType` / `BarFillStyle` / `bIsMarquee` | `0` / `White` / `LeftToRight` / `Mask` / `false` | `WidgetStyle.FillImage.TintColor` is also reset to `White` at construction. |
| `Image` | `ColorAndOpacity` | `White` | — |

## Slot defaults

The slot a widget gets is decided by its parent — see
[panels.md](panels.md) for every panel/content widget's slot class and its defaults
(e.g. `UButtonSlot` = `Padding(4,2)` + `Center/Center`, `UOverlaySlot` = `Left/Top`,
`USizeBoxSlot` = `Padding(0,0)` + `Fill/Fill`).
