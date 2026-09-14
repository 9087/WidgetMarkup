# Widget class defaults (complete reference)

Every reflected property of the UMG widget classes WidgetMarkup supports, with the value it has when markup does not set it. Dumped from the class default objects (`UClass::GetDefaultObject`) of a running engine, so these are UMG's own defaults — not this plugin's and not the Slate style's.

- Properties inherited from `UWidget` (`Visibility`, `IsEnabled`, `RenderOpacity`, `RenderTransform`, `Cursor`, `ToolTipText`, …) are omitted — see [shared-properties.md](shared-properties.md).
- `WidgetStyle` / `ItemStyle` / `ScrollBarStyle` leaves are omitted: those start from **UMG's default style set** (`FDefaultStyleCache`), never from `FAppStyle`, so a style sheet that mirrors the Slate style has to override every brush it cares about. See [default-values.md](default-values.md).
- `Slots` / `Slot` (children), delegates and sounds are omitted — see [panels.md](panels.md) for slot classes and their defaults.
- Struct properties are flattened to their leaves (`Padding.Left = 4`, `ShadowOffset.X = 1`, …); `(empty)` marks an empty string, array or set. Brush-valued properties are shown as a one-line summary.

## TextBlock

| Property | Default |
|---|---|
| `ApplyLineHeightToBottomLine` | `true` |
| `AutoWrapText` | `false` |
| `bSimpleTextMode` | `false` |
| `bWrapWithInvalidationPanel` | `false` |
| `ColorAndOpacity` | `(1,1,1,1 )` |
| `Font.bForceMonospaced` | `false` |
| `Font.FontMaterial` | `None` |
| `Font.FontName` | `None` |
| `Font.FontObject` | `/Engine/EngineFonts/Roboto.Roboto` |
| `Font.Hinting` | `Default` |
| `Font.LetterSpacing` | `0` |
| `Font.MonospacedWidth` | `1` |
| `Font.OutlineSettings.bApplyOutlineToDropShadows` | `false` |
| `Font.OutlineSettings.bMiteredCorners` | `false` |
| `Font.OutlineSettings.bSeparateFillAlpha` | `false` |
| `Font.OutlineSettings.OutlineColor.A` | `1` |
| `Font.OutlineSettings.OutlineColor.B` | `0` |
| `Font.OutlineSettings.OutlineColor.G` | `0` |
| `Font.OutlineSettings.OutlineColor.R` | `0` |
| `Font.OutlineSettings.OutlineMaterial` | `None` |
| `Font.OutlineSettings.OutlineSize` | `0` |
| `Font.Size` | `24` |
| `Font.SkewAmount` | `0` |
| `Font.TypefaceFontName` | `Bold` |
| `Justification` | `Left` |
| `LineHeightPercentage` | `1` |
| `Margin.Bottom` | `0` |
| `Margin.Left` | `0` |
| `Margin.Right` | `0` |
| `Margin.Top` | `0` |
| `MinDesiredWidth` | `0` |
| `ShadowColorAndOpacity.A` | `0` |
| `ShadowColorAndOpacity.B` | `0` |
| `ShadowColorAndOpacity.G` | `0` |
| `ShadowColorAndOpacity.R` | `0` |
| `ShadowOffset.X` | `1` |
| `ShadowOffset.Y` | `1` |
| `ShapedTextOptions.bOverride_TextFlowDirection` | `false` |
| `ShapedTextOptions.bOverride_TextShapingMethod` | `false` |
| `ShapedTextOptions.TextFlowDirection` | `Auto` |
| `ShapedTextOptions.TextShapingMethod` | `Auto` |
| `StrikeBrush` | `DrawAs=3 ImageType=0 Tiling=0 Resource=None Margin=(0,0,0,0) ImageSize=(32,32) Tint=(1,1,1,1 ) O…` |
| `Text` | `(empty)` |
| `TextOverflowPolicy` | `Clip` |
| `TextTransformPolicy` | `None` |
| `WrappingPolicy` | `DefaultWrapping` |
| `WrapTextAt` | `0` |

## RichTextBlock

| Property | Default |
|---|---|
| `ApplyLineHeightToBottomLine` | `true` |
| `AutoWrapText` | `false` |
| `bOverrideDefaultStyle` | `false` |
| `DecoratorClasses` | `()` |
| `DefaultTextStyle.ColorAndOpacity` | `(1,0,1,1 )` |
| `DefaultTextStyle.Font.bForceMonospaced` | `false` |
| `DefaultTextStyle.Font.FontMaterial` | `None` |
| `DefaultTextStyle.Font.FontName` | `None` |
| `DefaultTextStyle.Font.FontObject` | `None` |
| `DefaultTextStyle.Font.Hinting` | `Default` |
| `DefaultTextStyle.Font.LetterSpacing` | `0` |
| `DefaultTextStyle.Font.MonospacedWidth` | `1` |
| `DefaultTextStyle.Font.OutlineSettings.bApplyOutlineToDropShadows` | `false` |
| `DefaultTextStyle.Font.OutlineSettings.bMiteredCorners` | `false` |
| `DefaultTextStyle.Font.OutlineSettings.bSeparateFillAlpha` | `false` |
| `DefaultTextStyle.Font.OutlineSettings.OutlineColor.A` | `1` |
| `DefaultTextStyle.Font.OutlineSettings.OutlineColor.B` | `0` |
| `DefaultTextStyle.Font.OutlineSettings.OutlineColor.G` | `0` |
| `DefaultTextStyle.Font.OutlineSettings.OutlineColor.R` | `0` |
| `DefaultTextStyle.Font.OutlineSettings.OutlineMaterial` | `None` |
| `DefaultTextStyle.Font.OutlineSettings.OutlineSize` | `0` |
| `DefaultTextStyle.Font.Size` | `24` |
| `DefaultTextStyle.Font.SkewAmount` | `0` |
| `DefaultTextStyle.Font.TypefaceFontName` | `None` |
| `DefaultTextStyle.HighlightColor` | `(0,0,0,1 )` |
| `DefaultTextStyle.HighlightShape` | `DrawAs=3 ImageType=0 Tiling=0 Resource=None Margin=(0,0,0,0) ImageSize=(32,32) Tint=(1,1,1,1 ) O…` |
| `DefaultTextStyle.OverflowPolicy` | `Clip` |
| `DefaultTextStyle.SelectedBackgroundColor` | `(1,0,1,1 UseColor_Foreground)` |
| `DefaultTextStyle.ShadowColorAndOpacity.A` | `1` |
| `DefaultTextStyle.ShadowColorAndOpacity.B` | `0` |
| `DefaultTextStyle.ShadowColorAndOpacity.G` | `0` |
| `DefaultTextStyle.ShadowColorAndOpacity.R` | `0` |
| `DefaultTextStyle.ShadowOffset.X` | `0` |
| `DefaultTextStyle.ShadowOffset.Y` | `0` |
| `DefaultTextStyle.StrikeBrush` | `DrawAs=3 ImageType=0 Tiling=0 Resource=None Margin=(0,0,0,0) ImageSize=(32,32) Tint=(1,1,1,1 ) O…` |
| `DefaultTextStyle.TransformPolicy` | `None` |
| `DefaultTextStyle.UnderlineBrush` | `DrawAs=3 ImageType=0 Tiling=0 Resource=None Margin=(0,0,0,0) ImageSize=(32,32) Tint=(1,1,1,1 ) O…` |
| `DefaultTextStyleOverride.ColorAndOpacity` | `(1,0,1,1 )` |
| `DefaultTextStyleOverride.Font.bForceMonospaced` | `false` |
| `DefaultTextStyleOverride.Font.FontMaterial` | `None` |
| `DefaultTextStyleOverride.Font.FontName` | `None` |
| `DefaultTextStyleOverride.Font.FontObject` | `None` |
| `DefaultTextStyleOverride.Font.Hinting` | `Default` |
| `DefaultTextStyleOverride.Font.LetterSpacing` | `0` |
| `DefaultTextStyleOverride.Font.MonospacedWidth` | `1` |
| `DefaultTextStyleOverride.Font.OutlineSettings.bApplyOutlineToDropShadows` | `false` |
| `DefaultTextStyleOverride.Font.OutlineSettings.bMiteredCorners` | `false` |
| `DefaultTextStyleOverride.Font.OutlineSettings.bSeparateFillAlpha` | `false` |
| `DefaultTextStyleOverride.Font.OutlineSettings.OutlineColor.A` | `1` |
| `DefaultTextStyleOverride.Font.OutlineSettings.OutlineColor.B` | `0` |
| `DefaultTextStyleOverride.Font.OutlineSettings.OutlineColor.G` | `0` |
| `DefaultTextStyleOverride.Font.OutlineSettings.OutlineColor.R` | `0` |
| `DefaultTextStyleOverride.Font.OutlineSettings.OutlineMaterial` | `None` |
| `DefaultTextStyleOverride.Font.OutlineSettings.OutlineSize` | `0` |
| `DefaultTextStyleOverride.Font.Size` | `24` |
| `DefaultTextStyleOverride.Font.SkewAmount` | `0` |
| `DefaultTextStyleOverride.Font.TypefaceFontName` | `None` |
| `DefaultTextStyleOverride.HighlightColor` | `(0,0,0,1 )` |
| `DefaultTextStyleOverride.HighlightShape` | `DrawAs=3 ImageType=0 Tiling=0 Resource=None Margin=(0,0,0,0) ImageSize=(32,32) Tint=(1,1,1,1 ) O…` |
| `DefaultTextStyleOverride.OverflowPolicy` | `Clip` |
| `DefaultTextStyleOverride.SelectedBackgroundColor` | `(1,0,1,1 UseColor_Foreground)` |
| `DefaultTextStyleOverride.ShadowColorAndOpacity.A` | `1` |
| `DefaultTextStyleOverride.ShadowColorAndOpacity.B` | `0` |
| `DefaultTextStyleOverride.ShadowColorAndOpacity.G` | `0` |
| `DefaultTextStyleOverride.ShadowColorAndOpacity.R` | `0` |
| `DefaultTextStyleOverride.ShadowOffset.X` | `0` |
| `DefaultTextStyleOverride.ShadowOffset.Y` | `0` |
| `DefaultTextStyleOverride.StrikeBrush` | `DrawAs=3 ImageType=0 Tiling=0 Resource=None Margin=(0,0,0,0) ImageSize=(32,32) Tint=(1,1,1,1 ) O…` |
| `DefaultTextStyleOverride.TransformPolicy` | `None` |
| `DefaultTextStyleOverride.UnderlineBrush` | `DrawAs=3 ImageType=0 Tiling=0 Resource=None Margin=(0,0,0,0) ImageSize=(32,32) Tint=(1,1,1,1 ) O…` |
| `InstanceDecorators` | `()` |
| `Justification` | `Left` |
| `LineHeightPercentage` | `1` |
| `Margin.Bottom` | `0` |
| `Margin.Left` | `0` |
| `Margin.Right` | `0` |
| `Margin.Top` | `0` |
| `MinDesiredWidth` | `0` |
| `ShapedTextOptions.bOverride_TextFlowDirection` | `false` |
| `ShapedTextOptions.bOverride_TextShapingMethod` | `false` |
| `ShapedTextOptions.TextFlowDirection` | `Auto` |
| `ShapedTextOptions.TextShapingMethod` | `Auto` |
| `Text` | `(empty)` |
| `TextOverflowPolicy` | `Clip` |
| `TextStyleSet` | `None` |
| `TextTransformPolicy` | `None` |
| `WrappingPolicy` | `DefaultWrapping` |
| `WrapTextAt` | `0` |

## Button

| Property | Default |
|---|---|
| `BackgroundColor.A` | `1` |
| `BackgroundColor.B` | `1` |
| `BackgroundColor.G` | `1` |
| `BackgroundColor.R` | `1` |
| `ClickMethod` | `DownAndUp` |
| `ColorAndOpacity.A` | `1` |
| `ColorAndOpacity.B` | `1` |
| `ColorAndOpacity.G` | `1` |
| `ColorAndOpacity.R` | `1` |
| `IsFocusable` | `true` |
| `PressMethod` | `DownAndUp` |
| `TouchMethod` | `DownAndUp` |

## CheckBox

| Property | Default |
|---|---|
| `CheckedState` | `Unchecked` |
| `ClickMethod` | `DownAndUp` |
| `HorizontalAlignment` | `HAlign_Fill` |
| `IsFocusable` | `true` |
| `PressMethod` | `DownAndUp` |
| `TouchMethod` | `DownAndUp` |

## Slider

| Property | Default |
|---|---|
| `IndentHandle` | `false` |
| `IsFocusable` | `true` |
| `Locked` | `false` |
| `MaxValue` | `1` |
| `MinValue` | `0` |
| `MouseUsesStep` | `false` |
| `Orientation` | `Orient_Horizontal` |
| `RequiresControllerLock` | `true` |
| `SliderBarColor.A` | `1` |
| `SliderBarColor.B` | `1` |
| `SliderBarColor.G` | `1` |
| `SliderBarColor.R` | `1` |
| `SliderHandleColor.A` | `1` |
| `SliderHandleColor.B` | `1` |
| `SliderHandleColor.G` | `1` |
| `SliderHandleColor.R` | `1` |
| `StepSize` | `0.01` |
| `Value` | `0` |

## ProgressBar

| Property | Default |
|---|---|
| `BarFillStyle` | `Mask` |
| `BarFillType` | `LeftToRight` |
| `bIsMarquee` | `false` |
| `BorderPadding.X` | `0` |
| `BorderPadding.Y` | `0` |
| `FillColorAndOpacity.A` | `1` |
| `FillColorAndOpacity.B` | `1` |
| `FillColorAndOpacity.G` | `1` |
| `FillColorAndOpacity.R` | `1` |
| `Percent` | `0` |

## Image

| Property | Default |
|---|---|
| `bFlipForRightToLeftFlowDirection` | `false` |
| `Brush` | `DrawAs=3 ImageType=0 Tiling=0 Resource=None Margin=(0,0,0,0) ImageSize=(32,32) Tint=(1,1,1,1 ) O…` |
| `ColorAndOpacity.A` | `1` |
| `ColorAndOpacity.B` | `1` |
| `ColorAndOpacity.G` | `1` |
| `ColorAndOpacity.R` | `1` |

## Border

| Property | Default |
|---|---|
| `Background` | `DrawAs=3 ImageType=0 Tiling=0 Resource=None Margin=(0,0,0,0) ImageSize=(32,32) Tint=(1,1,1,1 ) O…` |
| `bFlipForRightToLeftFlowDirection` | `false` |
| `BrushColor.A` | `1` |
| `BrushColor.B` | `1` |
| `BrushColor.G` | `1` |
| `BrushColor.R` | `1` |
| `bShowEffectWhenDisabled` | `true` |
| `ContentColorAndOpacity.A` | `1` |
| `ContentColorAndOpacity.B` | `1` |
| `ContentColorAndOpacity.G` | `1` |
| `ContentColorAndOpacity.R` | `1` |
| `DesiredSizeScale.X` | `1` |
| `DesiredSizeScale.Y` | `1` |
| `HorizontalAlignment` | `HAlign_Fill` |
| `Padding.Bottom` | `2` |
| `Padding.Left` | `4` |
| `Padding.Right` | `4` |
| `Padding.Top` | `2` |
| `VerticalAlignment` | `VAlign_Fill` |

## SizeBox

| Property | Default |
|---|---|
| `bOverride_HeightOverride` | `false` |
| `bOverride_MaxAspectRatio` | `false` |
| `bOverride_MaxDesiredHeight` | `false` |
| `bOverride_MaxDesiredWidth` | `false` |
| `bOverride_MinAspectRatio` | `false` |
| `bOverride_MinDesiredHeight` | `false` |
| `bOverride_MinDesiredWidth` | `false` |
| `bOverride_WidthOverride` | `false` |
| `HeightOverride` | `0` |
| `MaxAspectRatio` | `1` |
| `MaxDesiredHeight` | `0` |
| `MaxDesiredWidth` | `0` |
| `MinAspectRatio` | `1` |
| `MinDesiredHeight` | `0` |
| `MinDesiredWidth` | `0` |
| `WidthOverride` | `0` |

## ScaleBox

| Property | Default |
|---|---|
| `IgnoreInheritedScale` | `false` |
| `Stretch` | `ScaleToFit` |
| `StretchDirection` | `Both` |
| `UserSpecifiedScale` | `1` |

## Spacer

| Property | Default |
|---|---|
| `Size.X` | `1` |
| `Size.Y` | `1` |

## ScrollBox

| Property | Default |
|---|---|
| `AllowOverscroll` | `true` |
| `AlwaysShowScrollbar` | `false` |
| `AlwaysShowScrollbarTrack` | `false` |
| `BackPadScrolling` | `false` |
| `bAllowRightClickDragScrolling` | `true` |
| `bAnimateWheelScrolling` | `false` |
| `ConsumeMouseWheel` | `WhenScrollingPossible` |
| `FrontPadScrolling` | `false` |
| `NavigationDestination` | `IntoView` |
| `NavigationScrollPadding` | `0` |
| `Orientation` | `Orient_Vertical` |
| `ScrollbarPadding.Bottom` | `2` |
| `ScrollbarPadding.Left` | `2` |
| `ScrollbarPadding.Right` | `2` |
| `ScrollbarPadding.Top` | `2` |
| `ScrollbarThickness.X` | `9` |
| `ScrollbarThickness.Y` | `9` |
| `ScrollBarVisibility` | `Visible` |
| `ScrollWhenFocusChanges` | `NoScroll` |
| `WheelScrollMultiplier` | `1` |

## GridPanel

| Property | Default |
|---|---|
| `ColumnFill` | `()` |
| `RowFill` | `()` |

## UniformGridPanel

| Property | Default |
|---|---|
| `MinDesiredSlotHeight` | `0` |
| `MinDesiredSlotWidth` | `0` |
| `SlotPadding.Bottom` | `0` |
| `SlotPadding.Left` | `0` |
| `SlotPadding.Right` | `0` |
| `SlotPadding.Top` | `0` |

## WrapBox

| Property | Default |
|---|---|
| `bExplicitWrapSize` | `false` |
| `HorizontalAlignment` | `HAlign_Left` |
| `InnerSlotPadding.X` | `0` |
| `InnerSlotPadding.Y` | `0` |
| `Orientation` | `Orient_Horizontal` |
| `WrapSize` | `500` |

## WidgetSwitcher

| Property | Default |
|---|---|
| `ActiveWidgetIndex` | `0` |

## ListView

| Property | Default |
|---|---|
| `AllowOverscroll` | `true` |
| `bAllowDragging` | `true` |
| `bClearSelectionOnClick` | `false` |
| `bEnableFixedLineOffset` | `false` |
| `bEnableRightClickScrolling` | `true` |
| `bEnableScrollAnimation` | `false` |
| `bEnableTouchScrolling` | `true` |
| `bInEnableTouchAnimatedScrolling` | `false` |
| `bIsFocusable` | `true` |
| `bIsPointerScrollingEnabled` | `true` |
| `bReturnFocusToSelection` | `false` |
| `ConsumeMouseWheel` | `WhenScrollingPossible` |
| `EntrySpacing` | `0` |
| `EntryWidgetClass` | `None` |
| `EntryWidgetPool.ActiveWidgets` | `()` |
| `EntryWidgetPool.InactiveWidgets` | `()` |
| `FixedLineScrollOffset` | `0` |
| `HorizontalEntrySpacing` | `0` |
| `ListItems` | `()` |
| `NumDesignerPreviewEntries` | `5` |
| `Orientation` | `Orient_Vertical` |
| `SelectionMode` | `Single` |
| `VerticalEntrySpacing` | `0` |
| `WheelScrollMultiplier` | `1` |

## TileView

| Property | Default |
|---|---|
| `AllowOverscroll` | `true` |
| `bAllowDragging` | `true` |
| `bClearSelectionOnClick` | `false` |
| `bEnableFixedLineOffset` | `false` |
| `bEnableRightClickScrolling` | `true` |
| `bEnableScrollAnimation` | `false` |
| `bEnableTouchScrolling` | `true` |
| `bEntrySizeIncludesEntrySpacing` | `true` |
| `bInEnableTouchAnimatedScrolling` | `false` |
| `bIsFocusable` | `true` |
| `bIsPointerScrollingEnabled` | `true` |
| `bReturnFocusToSelection` | `false` |
| `bWrapHorizontalNavigation` | `false` |
| `ConsumeMouseWheel` | `WhenScrollingPossible` |
| `EntryHeight` | `128` |
| `EntrySpacing` | `0` |
| `EntryWidgetClass` | `None` |
| `EntryWidgetPool.ActiveWidgets` | `()` |
| `EntryWidgetPool.InactiveWidgets` | `()` |
| `EntryWidth` | `128` |
| `FixedLineScrollOffset` | `0` |
| `HorizontalEntrySpacing` | `0` |
| `ListItems` | `()` |
| `NumDesignerPreviewEntries` | `5` |
| `Orientation` | `Orient_Vertical` |
| `ScrollbarDisabledVisibility` | `Collapsed` |
| `SelectionMode` | `Single` |
| `TileAlignment` | `EvenlyDistributed` |
| `VerticalEntrySpacing` | `0` |
| `WheelScrollMultiplier` | `1` |

## ComboBoxString

| Property | Default |
|---|---|
| `bIsFocusable` | `true` |
| `ContentPadding.Bottom` | `2` |
| `ContentPadding.Left` | `4` |
| `ContentPadding.Right` | `4` |
| `ContentPadding.Top` | `2` |
| `DefaultOptions` | `()` |
| `EnableGamepadNavigationMode` | `true` |
| `Font.bForceMonospaced` | `false` |
| `Font.FontMaterial` | `None` |
| `Font.FontName` | `None` |
| `Font.FontObject` | `/Engine/EngineFonts/Roboto.Roboto` |
| `Font.Hinting` | `Default` |
| `Font.LetterSpacing` | `0` |
| `Font.MonospacedWidth` | `1` |
| `Font.OutlineSettings.bApplyOutlineToDropShadows` | `false` |
| `Font.OutlineSettings.bMiteredCorners` | `false` |
| `Font.OutlineSettings.bSeparateFillAlpha` | `false` |
| `Font.OutlineSettings.OutlineColor.A` | `1` |
| `Font.OutlineSettings.OutlineColor.B` | `0` |
| `Font.OutlineSettings.OutlineColor.G` | `0` |
| `Font.OutlineSettings.OutlineColor.R` | `0` |
| `Font.OutlineSettings.OutlineMaterial` | `None` |
| `Font.OutlineSettings.OutlineSize` | `0` |
| `Font.Size` | `16` |
| `Font.SkewAmount` | `0` |
| `Font.TypefaceFontName` | `Bold` |
| `ForegroundColor` | `(0,0,0,1 )` |
| `HasDownArrow` | `true` |
| `MaxListHeight` | `450` |
| `SelectedOption` | `(empty)` |

## SpinBox

| Property | Default |
|---|---|
| `bAlwaysUsesDeltaSnap` | `false` |
| `bEnableSlider` | `true` |
| `bOverride_MaxSliderValue` | `false` |
| `bOverride_MaxValue` | `false` |
| `bOverride_MinSliderValue` | `false` |
| `bOverride_MinValue` | `false` |
| `ClearKeyboardFocusOnCommit` | `false` |
| `Delta` | `0` |
| `Font.bForceMonospaced` | `false` |
| `Font.FontMaterial` | `None` |
| `Font.FontName` | `None` |
| `Font.FontObject` | `/Engine/EngineFonts/Roboto.Roboto` |
| `Font.Hinting` | `Default` |
| `Font.LetterSpacing` | `0` |
| `Font.MonospacedWidth` | `1` |
| `Font.OutlineSettings.bApplyOutlineToDropShadows` | `false` |
| `Font.OutlineSettings.bMiteredCorners` | `false` |
| `Font.OutlineSettings.bSeparateFillAlpha` | `false` |
| `Font.OutlineSettings.OutlineColor.A` | `1` |
| `Font.OutlineSettings.OutlineColor.B` | `0` |
| `Font.OutlineSettings.OutlineColor.G` | `0` |
| `Font.OutlineSettings.OutlineColor.R` | `0` |
| `Font.OutlineSettings.OutlineMaterial` | `None` |
| `Font.OutlineSettings.OutlineSize` | `0` |
| `Font.Size` | `12` |
| `Font.SkewAmount` | `0` |
| `Font.TypefaceFontName` | `Bold` |
| `ForegroundColor` | `(0.472885,0.472885,0.472885,1 )` |
| `Justification` | `Left` |
| `KeyboardType` | `Number` |
| `MaxFractionalDigits` | `6` |
| `MaxSliderValue` | `0` |
| `MaxValue` | `0` |
| `MinDesiredWidth` | `0` |
| `MinFractionalDigits` | `1` |
| `MinSliderValue` | `0` |
| `MinValue` | `0` |
| `SelectAllTextOnCommit` | `true` |
| `SliderExponent` | `1` |
| `Value` | `0` |

## EditableTextBox

| Property | Default |
|---|---|
| `AllowContextMenu` | `true` |
| `bIsFontDeprecationDone` | `false` |
| `ClearKeyboardFocusOnCommit` | `true` |
| `HintText` | `(empty)` |
| `IsCaretMovedWhenGainFocus` | `true` |
| `IsPassword` | `false` |
| `IsReadOnly` | `false` |
| `Justification` | `Left` |
| `KeyboardType` | `Default` |
| `MinimumDesiredWidth` | `0` |
| `OverflowPolicy` | `Clip` |
| `RevertTextOnEscape` | `false` |
| `SelectAllTextOnCommit` | `false` |
| `SelectAllTextWhenFocused` | `false` |
| `ShapedTextOptions.bOverride_TextFlowDirection` | `false` |
| `ShapedTextOptions.bOverride_TextShapingMethod` | `false` |
| `ShapedTextOptions.TextFlowDirection` | `Auto` |
| `ShapedTextOptions.TextShapingMethod` | `Auto` |
| `Text` | `(empty)` |
| `VirtualKeyboardDismissAction` | `TextChangeOnDismiss` |
| `VirtualKeyboardOptions.bEnableAutocorrect` | `false` |
| `VirtualKeyboardTrigger` | `OnFocusByPointer` |

## MultiLineEditableTextBox

| Property | Default |
|---|---|
| `AllowContextMenu` | `true` |
| `ApplyLineHeightToBottomLine` | `true` |
| `AutoWrapText` | `true` |
| `bIsFontDeprecationDone` | `false` |
| `bIsReadOnly` | `false` |
| `HintText` | `(empty)` |
| `Justification` | `Left` |
| `LineHeightPercentage` | `1` |
| `Margin.Bottom` | `0` |
| `Margin.Left` | `0` |
| `Margin.Right` | `0` |
| `Margin.Top` | `0` |
| `ShapedTextOptions.bOverride_TextFlowDirection` | `false` |
| `ShapedTextOptions.bOverride_TextShapingMethod` | `false` |
| `ShapedTextOptions.TextFlowDirection` | `Auto` |
| `ShapedTextOptions.TextShapingMethod` | `Auto` |
| `Text` | `(empty)` |
| `TextStyle.ColorAndOpacity` | `(1,0,1,1 UseColor_Foreground)` |
| `TextStyle.Font.bForceMonospaced` | `false` |
| `TextStyle.Font.FontMaterial` | `None` |
| `TextStyle.Font.FontName` | `None` |
| `TextStyle.Font.FontObject` | `None` |
| `TextStyle.Font.Hinting` | `Default` |
| `TextStyle.Font.LetterSpacing` | `0` |
| `TextStyle.Font.MonospacedWidth` | `1` |
| `TextStyle.Font.OutlineSettings.bApplyOutlineToDropShadows` | `false` |
| `TextStyle.Font.OutlineSettings.bMiteredCorners` | `false` |
| `TextStyle.Font.OutlineSettings.bSeparateFillAlpha` | `false` |
| `TextStyle.Font.OutlineSettings.OutlineColor.A` | `1` |
| `TextStyle.Font.OutlineSettings.OutlineColor.B` | `0` |
| `TextStyle.Font.OutlineSettings.OutlineColor.G` | `0` |
| `TextStyle.Font.OutlineSettings.OutlineColor.R` | `0` |
| `TextStyle.Font.OutlineSettings.OutlineMaterial` | `None` |
| `TextStyle.Font.OutlineSettings.OutlineSize` | `0` |
| `TextStyle.Font.Size` | `10` |
| `TextStyle.Font.SkewAmount` | `0` |
| `TextStyle.Font.TypefaceFontName` | `Regular` |
| `TextStyle.HighlightColor` | `(0.02,0.3,0,1 )` |
| `TextStyle.HighlightShape` | `DrawAs=1 ImageType=1 Tiling=0 Resource=../../../Engine/Content/Slate/Common/TextBlockHighlightSh…` |
| `TextStyle.OverflowPolicy` | `Clip` |
| `TextStyle.SelectedBackgroundColor` | `(1,0,1,1 UseColor_Foreground)` |
| `TextStyle.ShadowColorAndOpacity.A` | `1` |
| `TextStyle.ShadowColorAndOpacity.B` | `0` |
| `TextStyle.ShadowColorAndOpacity.G` | `0` |
| `TextStyle.ShadowColorAndOpacity.R` | `0` |
| `TextStyle.ShadowOffset.X` | `0` |
| `TextStyle.ShadowOffset.Y` | `0` |
| `TextStyle.StrikeBrush` | `DrawAs=3 ImageType=0 Tiling=0 Resource=None Margin=(0,0,0,0) ImageSize=(32,32) Tint=(1,1,1,1 ) O…` |
| `TextStyle.TransformPolicy` | `None` |
| `TextStyle.UnderlineBrush` | `DrawAs=3 ImageType=0 Tiling=0 Resource=None Margin=(0,0,0,0) ImageSize=(32,32) Tint=(1,1,1,1 ) O…` |
| `VirtualKeyboardDismissAction` | `TextChangeOnDismiss` |
| `VirtualKeyboardOptions.bEnableAutocorrect` | `false` |
| `WrappingPolicy` | `DefaultWrapping` |
| `WrapTextAt` | `0` |

## NamedSlot

| Property | Default |
|---|---|
| `bExposeOnInstanceOnly` | `false` |
| `SlotGuid.A` | `-1867023082` |
| `SlotGuid.B` | `1185096643` |
| `SlotGuid.C` | `-1487445609` |
| `SlotGuid.D` | `1352693152` |

## BackgroundBlur

| Property | Default |
|---|---|
| `bApplyAlphaToBlur` | `true` |
| `BlurRadius` | `0` |
| `BlurStrength` | `0` |
| `bOverrideAutoRadiusCalculation` | `false` |
| `CornerRadius.W` | `0` |
| `CornerRadius.X` | `0` |
| `CornerRadius.Y` | `0` |
| `CornerRadius.Z` | `0` |
| `HorizontalAlignment` | `HAlign_Fill` |
| `LowQualityFallbackBrush` | `DrawAs=0 ImageType=0 Tiling=0 Resource=None Margin=(0,0,0,0) ImageSize=(0,0) Tint=(1,1,1,1 ) Out…` |
| `Padding.Bottom` | `0` |
| `Padding.Left` | `0` |
| `Padding.Right` | `0` |
| `Padding.Top` | `0` |
| `VerticalAlignment` | `VAlign_Fill` |

## Throbber

| Property | Default |
|---|---|
| `bAnimateHorizontally` | `true` |
| `bAnimateOpacity` | `true` |
| `bAnimateVertically` | `true` |
| `Image` | `DrawAs=3 ImageType=1 Tiling=0 Resource=../../../Engine/Content/Slate/Common/Throbber_Piece.png M…` |
| `NumberOfPieces` | `3` |

## CircularThrobber

| Property | Default |
|---|---|
| `bEnableRadius` | `true` |
| `Image` | `DrawAs=3 ImageType=1 Tiling=0 Resource=../../../Engine/Content/Slate/Common/Throbber_Piece.png M…` |
| `NumberOfPieces` | `6` |
| `Period` | `0.75` |
| `Radius` | `16` |

