# ScrollBox

`UScrollBox : UPanelWidget` — Scrollable container.

Inherits from: [shared-properties.md](shared-properties.md)

| Attribute | Type | Default | Description |
|---|---|---|---|
| `Orientation` | `EOrientation` | `Orient_Vertical` | Scroll direction |
| `ScrollBarVisibility` | `ESlateVisibility` | `Visible` | When the scroll bar is shown |
| `ConsumeMouseWheel` | `EConsumeMouseWheel` | `WhenScrollingPossible` | Whether the box swallows the wheel event |
| `AlwaysShowScrollbar` | `bool` | `false` | Keep the bar visible even when not scrollable |
| `AlwaysShowScrollbarTrack` | `bool` | `false` | Keep the track visible too |
| `AllowOverscroll` | `bool` | `true` | Bounce past the ends |
| `BackPadScrolling` / `FrontPadScrolling` | `bool` | `false` | Include the padding in the scrollable range |
| `bAnimateWheelScrolling` | `bool` | `false` | Interpolate wheel scrolling |
| `bAllowRightClickDragScrolling` | `bool` | `true` | Drag to scroll with the right button |
| `WheelScrollMultiplier` | `float` | `1` | Wheel speed multiplier |
| `ScrollWhenFocusChanges` | `EScrollWhenFocusChanges` | `NoScroll` | Auto-scroll on focus change |
| `NavigationDestination` | `EScrollBoxNavigationDestination` | `IntoView` | Gamepad navigation target |
| `NavigationScrollPadding` | `float` | `0` | Padding used by gamepad scrolling |
| `ScrollbarThickness` | [FVector2D](../structs/vector2d.md) | `9,9` | Bar thickness (note the engine's spelling: `Scrollbar…`, not `ScrollBar…`) |
| `ScrollbarPadding` | [FMargin](../structs/margin.md) | `2,2,2,2` | Space between bar and content |

## Slot

Defaults are UMG's (`UScrollBoxSlot::UScrollBoxSlot`).

| Slot Property | Type | Default | Description |
|---|---|---|---|
| `Size.SizeRule` | `ESlateSizeRule` | `Automatic` | `Automatic` or `Fill` |
| `Size.Value` | `float` | `1.0` | Size when Fill |
| `Padding` | `FMargin` | `0` | Internal spacing |
| `HorizontalAlignment` | `EHorizontalAlignment` | `HAlign_Fill` | Content H-align |
| `VerticalAlignment` | `EVerticalAlignment` | `VAlign_Fill` | Content V-align |
