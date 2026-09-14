# Panels & Content Widgets — Slot types and defaults

`Slot` is the object the **parent** creates for a child widget. It lives on the child
(`<TextBlock Slot.Padding="4,2,4,2" />`) but its concrete class is decided by the
parent — a `<Button>` inside a `<HorizontalBox>` has a `UHorizontalBoxSlot`, while the
`<TextBlock>` inside that Button has a `UButtonSlot`.

Rules:

- A **root widget** (direct child of `<WidgetTree>`) has no slot — never set `Slot.*` on it.
- Properties of the **panel itself** (`RowFill`, `ColumnFill`, `bAutoSize`, …) are written
  **without** the `Slot.` prefix; they belong to the container, not to the slot.
- Slot objects are created in `OnAddChild` (`UPanelWidget::AddChild`) **before** widget
  attributes are processed, so `Slot.*` attributes and setters always find their target.

The defaults below are what a child gets when the markup sets nothing. They are the
initialisers of the `U*Slot` constructors in
`Runtime/UMG/Private/Components/*Slot.cpp`.

## Panel slots

| Parent | Slot class | `Padding` | `HorizontalAlignment` | `VerticalAlignment` | Other defaults |
|---|---|---|---|---|---|
| `<CanvasPanel>` | `UCanvasPanelSlot` | — | — | — | `LayoutData.Offsets=(0,0,100,30)`, `LayoutData.Anchors=((0,0),(0,0))`, `LayoutData.Alignment=(0,0)`, `bAutoSize=false`, `ZOrder=0` |
| `<VerticalBox>` | `UVerticalBoxSlot` | `0` | `HAlign_Fill` | `VAlign_Fill` | `Size.SizeRule=Automatic` |
| `<HorizontalBox>` | `UHorizontalBoxSlot` | `0` | `HAlign_Fill` | `VAlign_Fill` | `Size.SizeRule=Automatic` |
| `<GridPanel>` | `UGridSlot` | `0` | `HAlign_Fill` | `VAlign_Fill` | `Row=0`, `Column=0`, `RowSpan=1`\*, `ColumnSpan=1`\*, `Layer=0`, `Nudge=(0,0)` |
| `<UniformGridPanel>` | `UUniformGridSlot` | — | **`HAlign_Left`** | **`VAlign_Top`** | `Row=0`, `Column=0` |
| `<WrapBox>` | `UWrapBoxSlot` | `0` | `HAlign_Fill` | `VAlign_Fill` | `bFillEmptySpace=false`, `FillSpanWhenLessThan=0`, `bForceNewLine=false` |
| `<Overlay>` | `UOverlaySlot` | `0` | **`HAlign_Left`** | **`VAlign_Top`** | — |
| `<ScrollBox>` | `UScrollBoxSlot` | `0` | `HAlign_Fill` | `VAlign_Fill` | `Size.SizeRule=Automatic` |
| `<SizeBox>` | `USizeBoxSlot` | `0,0` | `HAlign_Fill` | `VAlign_Fill` | — |
| `<ScaleBox>` | `UScaleBoxSlot` | — | **`HAlign_Center`** | **`VAlign_Center`** | — |
| `<WidgetSwitcher>` | `UWidgetSwitcherSlot` | — | `HAlign_Fill` | `VAlign_Fill` | — |
| `<WindowTitleBarArea>` | `UWindowTitleBarAreaSlot` | `0` | `HAlign_Fill` | `VAlign_Fill` | — |
| `<SafeZone>` | `USafeZoneSlot` | — | — | — | `bIsTitleSafe=true`, `SafeAreaScale=(1,1,1,1)` |

\* `UGridSlot::RowSpan` / `ColumnSpan` are zero-initialised as UPROPERTYs, but
`SGridPanel` clamps them with `FMath::Max(1, …)` (`SGridPanel.cpp`), so the effective
default is `1`.

## Border / Button / BackgroundBlur

These content widgets create their own slot for the single child, with a 4 px / 2 px
inset by default:

| Parent | Slot class | `Padding` | `HorizontalAlignment` | `VerticalAlignment` |
|---|---|---|---|---|
| `<Border>` | `UBorderSlot` | `(4,2)` | `HAlign_Fill` | `VAlign_Fill` |
| `<BackgroundBlur>` | `UBackgroundBlurSlot` | `(4,2)` | `HAlign_Fill` | `VAlign_Fill` |
| `<Button>` | `UButtonSlot` | `(4,2)` | **`HAlign_Center`** | **`VAlign_Center`** |

> **Why a button label can render 1 px lower than the same label in Slate.**
> Slate's `SButton` is an `SBorder` with `_VAlign(VAlign_Fill)` (`SButton.h`), so when a
> row stretches the button, the extra height goes to the label and the label's box stays
> on whole pixels. UMG's `UButtonSlot` defaults to `VAlign_Center`, so the label keeps its
> own height and is centred — e.g. a 15 px label in a 20 px content area lands at
> `pos y=876.50`, which rounds to 1 px lower than Slate's 16 px box at `876.00`.
> Setting `Slot.VerticalAlignment="VAlign_Fill"` on the label reproduces Slate exactly
> (the slot's own `(4,2)` padding provides the inset); because that is a property of the
> *label*, it has to be written on the label widget (or on its TextBlock style), not on
> the button.

## SizeBox Slot

| Property | Default |
|---|---|
| `Padding` | `0,0` |
| `HorizontalAlignment` | `HAlign_Fill` |
| `VerticalAlignment` | `VAlign_Fill` |

## Slot paths in style sheets

`Slot.*` works in a style sheet setter, but note **when** it is resolved:

- `Value="…"`-form setters (`<Setter Property="Slot.Padding" Value="4,2,4,2" />`) are
  converted at runtime, so they resolve the slot object and work normally.
- Child-element-form setters (`<Setter Property="Slot.Padding"><Margin …/></Setter>`)
  need the tail property at **compile** time; the resolver runs in type-only mode there
  and bails out on object-pointer segments such as `Slot`
  (`PropertyPathResolver` → object properties cannot be dereferenced without an
  instance). Such a setter is dropped — with a warning from `FSetterElementNode::OnEnd`.
  Use the `Value=` form for slot properties.
