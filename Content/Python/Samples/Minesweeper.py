from __future__ import annotations

import random
import time

import unreal
from WidgetMarkupComponent import WidgetMarkupComponent, computed, reactive

from Samples.MinesweeperCell import CellState, MinesweeperCell

ROWS = 9
COLS = 9
MINES = 10


class Minesweeper(WidgetMarkupComponent):
    @computed
    def mines_remaining(self):
        remaining = max(MINES - self.flag_count, 0)
        return f"{remaining:03d}"

    @reactive
    def status_text(self):
        return "Left click to reveal, right click to flag"

    @reactive
    def flag_count(self) -> int:
        return 0

    @reactive
    def current_time(self) -> float:
        return 0.0

    @reactive
    def start_time(self) -> float:
        return 0.0

    @computed
    def elapsed(self) -> int:
        return int(self.current_time - self.start_time)

    @computed
    def elapsed_time(self) -> str:
        minutes = self.elapsed // 60
        seconds = self.elapsed % 60
        return f"{minutes:02d}:{seconds:02d}"

    def __init__(self) -> None:
        self._grid: list[list[MinesweeperCell]] = []
        self._cells_created = False
        self._game_over = False
        self._won = False
        self._mines_placed = False
        self._timer_handle = None
        self._left_down = False
        self._right_down = False
        self._action_row = -1
        self._action_column = -1
        self._chord_handled = False
        super().__init__()
        self.start_new_game()

    def _ensure_cells(self) -> None:
        """Create cell widgets if not already created."""
        if self._cells_created:
            return
        self._create_cells()

    def _create_cells(self) -> None:
        """Dynamically create CellWidget instances and add them to the grid."""
        game_grid = self.find_widget("GameGrid")
        if game_grid is None:
            unreal.log_warning("Minesweeper: GameGrid not found")
            return

        unreal.log_warning(f"Minesweeper: creating {ROWS}x{COLS} cells...")
        self._grid = [[None] * COLS for _ in range(ROWS)]
        created = 0
        for row in range(ROWS):
            for column in range(COLS):
                name = f"Cell_{row}_{column}"
                try:
                    cell = self.add_child(name, "/WidgetMarkup/Samples/MinesweeperCell", game_grid)
                except Exception as exc:
                    unreal.log_error(f"Minesweeper: add_child failed for {name}: {exc}")
                    continue
                if cell is None:
                    unreal.log_warning(f"Minesweeper: failed to create {name}")
                    continue

                cell.on_click = lambda geo, evt, r=row, c=column: self._on_cell_clicked(r, c, geo, evt)
                cell.on_release = lambda geo, evt, r=row, c=column: self._on_button_released(r, c, geo, evt)
                self._grid[row][column] = cell

                child_widget = self.find_widget(name)
                if child_widget is not None:
                    slot = child_widget.slot
                    if slot is not None:
                        slot.set_row(row)
                        slot.set_column(column)
                        slot.set_padding(unreal.Margin(2.0, 2.0, 2.0, 2.0))
                        slot.set_horizontal_alignment(unreal.HorizontalAlignment.H_ALIGN_FILL)
                        slot.set_vertical_alignment(unreal.VerticalAlignment.V_ALIGN_FILL)

                created += 1

        unreal.log_warning(f"Minesweeper: created {created}/{ROWS*COLS} cells")
        self._cells_created = True

    def _on_cell_clicked(
        self, row: int, column: int,
        geometry: unreal.Geometry,
        mouse_event: unreal.WidgetMarkupPointerEvent,
    ) -> Any:
        """Record button state on press — action is deferred to release."""
        self._action_row = row
        self._action_column = column
        if self._is_right_mouse_button(mouse_event):
            self._right_down = True
        else:
            self._left_down = True

        if self._left_down and self._right_down and not self._chord_handled:
            self._highlight_chord_neighbors(row, column)
        return unreal.WidgetLibrary.handled()

    def _on_button_released(
        self, row: int, column: int,
        geometry: unreal.Geometry,
        mouse_event: unreal.WidgetMarkupPointerEvent,
    ) -> Any:
        """Decide action on release: chord if both were down, else flag or reveal."""
        is_right = self._is_right_mouse_button(mouse_event)

        if self._left_down and self._right_down:
            if not self._chord_handled:
                self._chord(self._action_row, self._action_column)
                self._chord_handled = True
        elif is_right and not self._chord_handled:
            self._toggle_flag(self._grid[self._action_row][self._action_column])
        elif not is_right and not self._chord_handled:
            self.handle_cell_click(self._action_row, self._action_column)

        if is_right:
            self._right_down = False
        else:
            self._left_down = False

        if not self._left_down and not self._right_down:
            self._chord_handled = False
            self._clear_all_highlights()

        return unreal.WidgetLibrary.handled()

    @staticmethod
    def _is_right_mouse_button(mouse_event: unreal.WidgetMarkupPointerEvent) -> bool:
        key_name = mouse_event.effecting_button.get_editor_property("KeyName")
        return "RightMouseButton" in str(key_name)

    @property
    def is_game_over(self) -> bool:
        return self._game_over

    def start_new_game(self) -> None:
        unreal.log_warning("Minesweeper: start_new_game called")
        self._ensure_cells()

        for row in range(ROWS):
            for column in range(COLS):
                cell = self._grid[row][column]
                if cell is not None:
                    cell.state = CellState.HIDDEN
                    cell.is_mine = False
                    cell.adjacent_mines = 0
        self._game_over = False
        self._won = False
        self._mines_placed = False
        self._left_down = False
        self._right_down = False
        self._chord_handled = False
        self._clear_all_highlights()
        self.flag_count = 0
        self.start_time = 0.0
        self.current_time = 0.0
        self._stop_timer()
        self.status_text = "Left click to reveal, right click to flag"

    def handle_cell_click(self, row: int, column: int) -> None:
        if self._game_over or self._won:
            return

        cell = self._grid[row][column]
        if cell.state == CellState.FLAGGED:
            return

        if not self._mines_placed:
            self._place_mines(row, column)
            self._mines_placed = True
            self._start_timer()

        if cell.state != CellState.HIDDEN:
            return

        self._reveal_cell(row, column)
        if cell.is_mine:
            self._end_game(won=False)
            return

        if self._check_win():
            self._end_game(won=True)

    def _toggle_flag(self, cell: MinesweeperCell) -> None:
        if self._game_over or self._won:
            return
        if cell.state == CellState.REVEALED:
            return

        if cell.state == CellState.FLAGGED:
            cell.state = CellState.HIDDEN
            self.flag_count -= 1
        else:
            cell.state = CellState.FLAGGED
            self.flag_count += 1

    def _chord(self, row: int, column: int) -> None:
        """Reveal neighbors when flags match the cell's number (chord action)."""
        if self._game_over or self._won:
            return
        cell = self._grid[row][column]
        if cell.state != CellState.REVEALED or cell.is_mine or cell.adjacent_mines == 0:
            return

        flag_count = 0
        hidden_neighbors: list[tuple[int, int]] = []
        for delta_row in (-1, 0, 1):
            for delta_column in (-1, 0, 1):
                if delta_row == 0 and delta_column == 0:
                    continue
                neighbor_row = row + delta_row
                neighbor_column = column + delta_column
                if 0 <= neighbor_row < ROWS and 0 <= neighbor_column < COLS:
                    neighbor = self._grid[neighbor_row][neighbor_column]
                    if neighbor.state == CellState.FLAGGED:
                        flag_count += 1
                    elif neighbor.state == CellState.HIDDEN:
                        hidden_neighbors.append((neighbor_row, neighbor_column))

        if flag_count == cell.adjacent_mines:
            for neighbor_row, neighbor_column in hidden_neighbors:
                self._reveal_cell(neighbor_row, neighbor_column)
                if self._grid[neighbor_row][neighbor_column].is_mine:
                    self._end_game(won=False)
                    return
            if self._check_win():
                self._end_game(won=True)

    def _highlight_chord_neighbors(self, row: int, column: int) -> None:
        """Light up hidden neighbors when both mouse buttons are held."""
        if self._game_over or self._won:
            return
        cell = self._grid[row][column]
        if cell.state != CellState.REVEALED or cell.is_mine or cell.adjacent_mines == 0:
            return
        for delta_row in (-1, 0, 1):
            for delta_column in (-1, 0, 1):
                if delta_row == 0 and delta_column == 0:
                    continue
                neighbor_row = row + delta_row
                neighbor_column = column + delta_column
                if 0 <= neighbor_row < ROWS and 0 <= neighbor_column < COLS:
                    neighbor = self._grid[neighbor_row][neighbor_column]
                    if neighbor.state == CellState.HIDDEN:
                        neighbor.highlighted = True

    def _clear_all_highlights(self) -> None:
        """Remove chord highlights from all cells."""
        for row in range(ROWS):
            for column in range(COLS):
                cell = self._grid[row][column]
                if cell is not None:
                    cell.highlighted = False

    def _place_mines(self, safe_row: int, safe_column: int) -> None:
        safe_cells = {(safe_row, safe_column)}
        for delta_row in (-1, 0, 1):
            for delta_column in (-1, 0, 1):
                neighbor_row = safe_row + delta_row
                neighbor_column = safe_column + delta_column
                if 0 <= neighbor_row < ROWS and 0 <= neighbor_column < COLS:
                    safe_cells.add((neighbor_row, neighbor_column))

        candidates = [
            (row, column)
            for row in range(ROWS)
            for column in range(COLS)
            if (row, column) not in safe_cells
        ]
        random.shuffle(candidates)

        for row, column in candidates[:MINES]:
            self._grid[row][column].is_mine = True

        for row in range(ROWS):
            for column in range(COLS):
                if self._grid[row][column].is_mine:
                    continue
                self._grid[row][column].adjacent_mines = self._count_adjacent_mines(row, column)

    def _count_adjacent_mines(self, row: int, column: int) -> int:
        count = 0
        for delta_row in (-1, 0, 1):
            for delta_column in (-1, 0, 1):
                if delta_row == 0 and delta_column == 0:
                    continue
                neighbor_row = row + delta_row
                neighbor_column = column + delta_column
                if 0 <= neighbor_row < ROWS and 0 <= neighbor_column < COLS:
                    if self._grid[neighbor_row][neighbor_column].is_mine:
                        count += 1
        return count

    def _reveal_cell(self, row: int, column: int) -> None:
        cell = self._grid[row][column]
        if cell.state == CellState.REVEALED:
            return

        if cell.state == CellState.FLAGGED:
            cell.state = CellState.HIDDEN
            self.flag_count -= 1

        cell.state = CellState.REVEALED

        if not cell.is_mine and cell.adjacent_mines == 0:
            for delta_row in (-1, 0, 1):
                for delta_column in (-1, 0, 1):
                    if delta_row == 0 and delta_column == 0:
                        continue
                    neighbor_row = row + delta_row
                    neighbor_column = column + delta_column
                    if 0 <= neighbor_row < ROWS and 0 <= neighbor_column < COLS:
                        neighbor = self._grid[neighbor_row][neighbor_column]
                        if neighbor.state == CellState.HIDDEN:
                            self._reveal_cell(neighbor_row, neighbor_column)

    def _check_win(self) -> bool:
        for row in range(ROWS):
            for column in range(COLS):
                cell = self._grid[row][column]
                if not cell.is_mine and cell.state != CellState.REVEALED:
                    return False
        return True

    def _end_game(self, won: bool) -> None:
        self._stop_timer()
        self._won = won
        self._game_over = not won

        if won:
            self.status_text = "You Win!"
            for row in range(ROWS):
                for column in range(COLS):
                    cell = self._grid[row][column]
                    if cell.is_mine and cell.state != CellState.FLAGGED:
                        cell.state = CellState.FLAGGED
                        self.flag_count += 1
        else:
            self.status_text = "Game Over"
            for row in range(ROWS):
                for column in range(COLS):
                    cell = self._grid[row][column]
                    if cell.is_mine:
                        cell.state = CellState.REVEALED

    # --- Timer ---

    def _start_timer(self) -> None:
        """Begin elapsed-time counting via post-tick callback."""
        if self._timer_handle is not None:
            return
        self.start_time = time.time()
        self.current_time = self.start_time
        self._timer_handle = unreal.register_slate_post_tick_callback(self._on_timer_tick)

    def _stop_timer(self) -> None:
        """Stop the timer tick callback if active."""
        if self._timer_handle is None:
            return
        unreal.unregister_slate_post_tick_callback(self._timer_handle)
        self._timer_handle = None

    def _on_timer_tick(self, delta_time: float) -> None:
        self.current_time = time.time()
