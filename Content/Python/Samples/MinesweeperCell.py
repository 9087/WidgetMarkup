from __future__ import annotations

from typing import Any, Callable

import enum

import unreal
from WidgetMarkupComponent import WidgetMarkupComponent, computed, reactive


class CellState(enum.Enum):
    HIDDEN = 0
    FLAGGED = 1
    REVEALED = 2

_NUMBER_COLORS = {
    1: unreal.LinearColor(0.0, 0.0, 1.0, 1.0),
    2: unreal.LinearColor(0.0, 0.55, 0.0, 1.0),
    3: unreal.LinearColor(1.0, 0.0, 0.0, 1.0),
    4: unreal.LinearColor(0.0, 0.0, 0.55, 1.0),
    5: unreal.LinearColor(0.55, 0.0, 0.0, 1.0),
    6: unreal.LinearColor(0.0, 0.75, 0.75, 1.0),
    7: unreal.LinearColor(0.1, 0.1, 0.1, 1.0),
    8: unreal.LinearColor(0.45, 0.45, 0.45, 1.0),
}


class MinesweeperCell(WidgetMarkupComponent):
    """Single Minesweeper cell – holds its own data and drives visuals via reactive properties."""

    # --- Source data (reactive – changing these auto-updates computed properties) ---

    @reactive
    def state(self) -> CellState:
        return CellState.HIDDEN

    @reactive
    def is_mine(self) -> bool:
        return False

    @reactive
    def adjacent_mines(self) -> int:
        return 0

    # --- Computed visual properties (bound in XML, auto-track reactive deps) ---

    @computed
    def label(self) -> str:
        if self.state == CellState.FLAGGED:
            return "F"
        if self.state == CellState.HIDDEN:
            return ""
        if self.is_mine:
            return "*"
        if self.adjacent_mines == 0:
            return ""
        return str(self.adjacent_mines)

    @computed
    def background_color(self) -> unreal.LinearColor:
        if self.state == CellState.HIDDEN:
            return unreal.LinearColor(0.72, 0.72, 0.74, 1.0)
        if self.state == CellState.FLAGGED:
            return unreal.LinearColor(0.95, 0.88, 0.35, 1.0)
        if self.is_mine and self.state == CellState.REVEALED:
            return unreal.LinearColor(0.95, 0.35, 0.35, 1.0)
        return unreal.LinearColor(0.86, 0.86, 0.88, 1.0)

    @computed
    def text_color(self) -> unreal.SlateColor:
        if self.state == CellState.REVEALED and not self.is_mine and self.adjacent_mines > 0:
            color = _NUMBER_COLORS.get(self.adjacent_mines, unreal.LinearColor(0.1, 0.1, 0.1, 1.0))
            return unreal.SlateColor(color)
        if self.state == CellState.FLAGGED:
            return unreal.SlateColor(unreal.LinearColor(0.55, 0.2, 0.0, 1.0))
        if self.is_mine and self.state == CellState.REVEALED:
            return unreal.SlateColor(unreal.LinearColor(0.1, 0.1, 0.1, 1.0))
        return unreal.SlateColor(unreal.LinearColor(0.15, 0.15, 0.15, 1.0))

    # --- Lifecycle ---

    def __init__(self) -> None:
        self.on_click: Callable[[unreal.Geometry, unreal.WidgetMarkupPointerEvent], Any] | None = None
        super().__init__()

    # --- Input ---

    def on_mouse_down(
        self, geometry: unreal.Geometry, mouse_event: unreal.WidgetMarkupPointerEvent,
    ) -> Any:
        if self.on_click is not None:
            return self.on_click(geometry, mouse_event)
        return unreal.WidgetLibrary.handled()
