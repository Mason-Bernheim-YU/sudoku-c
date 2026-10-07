#include "board.h"

#include <stdlib.h>

int board_coordinates_in_range(int row, int column) {
    return row >= 0 && row < SUDOKU_SIZE &&
           column >= 0 && column < SUDOKU_SIZE;
}

SudokuBoard *board_create(void) {
    SudokuBoard *board;
    board = malloc(sizeof(*board));
    if (board == NULL) {
        return NULL;
    }
    board->cells = calloc(SUDOKU_CELL_COUNT, sizeof(*board->cells));
    if (board->cells == NULL) {
        free(board);
        return NULL;
    }

    return board;
}

SudokuBoard *board_clone(const SudokuBoard *source) {
    SudokuBoard *copy;

    if (source == NULL || source->cells == NULL) {
        return NULL;
    }
    copy = board_create();
    if (copy == NULL) {
        return NULL;
    }

    if (!board_copy(copy, source)) {
        board_destroy(&copy);
        return NULL;
    }

    return copy;
}

void board_destroy(SudokuBoard **board_ptr) {
    SudokuBoard *board;

    if (board_ptr == NULL || *board_ptr == NULL) {
        return;
    }

    board = *board_ptr;

    free(board->cells);
    board->cells = NULL;
    free(board);

    *board_ptr = NULL;
}

int *board_cell(SudokuBoard *board, int row, int column) {
    if (board == NULL || board->cells == NULL ||
        !board_coordinates_in_range(row, column)) {
        return NULL;
    }

    return board->cells + (size_t)row * SUDOKU_SIZE + (size_t)column;
}

const int *board_cell_const(const SudokuBoard *board, int row, int column) {
    if (board == NULL || board->cells == NULL ||
        !board_coordinates_in_range(row, column)) {
        return NULL;
    }

    return board->cells + (size_t)row * SUDOKU_SIZE + (size_t)column;
}

void board_clear(SudokuBoard *board) {
    int *cursor;
    int *end;

    if (board == NULL || board->cells == NULL) {
        return;
    }

    cursor = board->cells;
    end = board->cells + SUDOKU_CELL_COUNT;

    while (cursor < end) {
        *cursor = SUDOKU_EMPTY;
        cursor++;
    }
}

int board_copy(SudokuBoard *destination, const SudokuBoard *source) {
    if (destination == NULL || destination->cells == NULL ||
        source == NULL || source->cells == NULL) {
        return 0;
    }

    for (size_t index = 0; index < SUDOKU_CELL_COUNT; index++) {
        destination->cells[index] = source->cells[index];
    }

    return 1;
}

int board_equal(const SudokuBoard *first, const SudokuBoard *second) {
    if (first == NULL || first->cells == NULL ||
        second == NULL || second->cells == NULL) {
        return 0;
    }

    for (size_t index = 0; index < SUDOKU_CELL_COUNT; index++) {
        if (first->cells[index] != second->cells[index]) {
            return 0;
        }
    }

    return 1;
}