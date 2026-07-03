from __future__ import annotations

from typing import Any

import unreal
from WidgetMarkupComponent import WidgetMarkupComponent, computed, reactive

from Samples.Minesweeper import CellState

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
        # Game-over red relies on parent_game.is_game_over which is cross-
        # instance and not auto-tracked; _end_game calls refresh() for this.
        if self.is_mine and self.parent_game is not None and self.parent_game.is_game_over:
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
        self.row: int = -1
        self.column: int = -1
        self.parent_game: Any = None  # Minesweeper instance
        super().__init__()

    # --- Public API ---

    def refresh(self) -> None:
        """Force recompute of computed properties (for cross-instance deps like game-over)."""
        # Accessing each computed property triggers recomputation and UI notification.
        _ = self.label
        _ = self.background_color
        _ = self.text_color

    # --- Input ---

    def on_mouse_down(
        self, geometry: unreal.Geometry, mouse_event: unreal.WidgetMarkupPointerEvent,
    ) -> Any:
        if self.parent_game is not None:
            return self.parent_game.on_cell_mouse_down(
                self.row, self.column, geometry, mouse_event,
            )
        return unreal.WidgetLibrary.handled()
