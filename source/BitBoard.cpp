#include "BitBoard.h"
#include <cassert>
#include <cmath>

BitBoard::BitBoard() {
    clear();
}

void BitBoard::clear() {
    for (Line &line : h_lines) line = (1 << 2*SIZE) - 1;
    for (Line &line : v_lines) line = (1 << 2*SIZE) - 1;
    for (size_t i = 1; i <= SIZE; i++) {
        Line bits = (1 << i*2) - 1;
        main_d_lines[i-1]         = bits << (SIZE-i)*2;
        main_d_lines[DIAG_SIZE-i] = bits;
    }
    for (size_t i = 1; i <= SIZE; i++) {
        Line bits = (1 << i*2) - 1;
        sub_d_lines[i-1]         = bits;
        sub_d_lines[DIAG_SIZE-i] = bits << (SIZE-i)*2;
    }
}

Figure BitBoard::get_cell(size_t row, size_t col) const {
    assert(row < SIZE && col < SIZE && "Out of bound");
    return Figure((h_lines[row] >> 2*(SIZE-1 - col)) & 0b11);
}

void BitBoard::set_cell(size_t row, size_t col, Figure cell) {
    assert(cell != Figure::Out && "Invalid cell");
    assert(row < SIZE && col < SIZE && "Out of bound");
    static const auto set = [](size_t right_gap, Line *line, Figure c) {
        *line |= (0b11 << 2*right_gap);
        *line &= ~((~(c) & 0b11) << 2*right_gap);
    };
    set(SIZE-1 - col, &h_lines[row], cell);
    set(SIZE-1 - row, &v_lines[col], cell);

    size_t right_out_gap = row+col < SIZE ? SIZE-1-(row+col) : 0;
    size_t right_pieces = std::min(row, SIZE-1 - col);
    set(right_pieces + right_out_gap, &main_d_lines[row+col], cell);
    right_out_gap = row > col ? row - col : 0;
    right_pieces = std::min(SIZE-1 - col, SIZE-1 - row);
    set(right_pieces + right_out_gap, &sub_d_lines[(SIZE-1-col) + row], cell);
}


Line BitBoard::get_line(size_t row, size_t col, Direction dir) const {
    assert(row < SIZE && col < SIZE && "Out of bound");
    switch (dir) {
        case HORIZONTAL: return h_lines[row];
        case VERTICAL: return v_lines[col];
        case DIAGONAL: return main_d_lines[row+col];
        case SUBDIAGONAL: return sub_d_lines[(SIZE-1-col) + row];
        default: assert(false && "Invalid direction");
    }
}

Line4 BitBoard::get_lines(size_t row, size_t col) const {
    assert(row < SIZE && col < SIZE && "Out of bound");
    return Line4 {
        h_lines[row],
        v_lines[col],
        main_d_lines[row+col],
        sub_d_lines[(SIZE-1-col) + row],
    };
}

Line BitBoard::get_line_radius(size_t row, size_t col, Direction dir) const {
    return get_lines_radius(row, col)[dir];
}

// (row, col) at center RADIUS*2 bits in total
Line4 BitBoard::get_lines_radius(size_t row, size_t col) const {
    Line4 result = get_lines(row, col);
    static const auto adjust = [](size_t left_pieces, size_t right_pieces, Line *line) {
        if (left_pieces > RADIUS)  *line &=  (1 << (right_pieces + 1 + RADIUS)*2) - 1;
        if (right_pieces < RADIUS) *line <<= (RADIUS - right_pieces)*2;
        else                     *line >>= (right_pieces - RADIUS)*2; // left + 1 + right - (left + 1 + RADIUS)
        *line &= (1 << (RADIUS*2 + 1)*2) - 1;
    };
    adjust(col, SIZE-1-col, &result[HORIZONTAL]);
    adjust(row, SIZE-1-row, &result[VERTICAL]);
    if (row + col < SIZE) result[DIAGONAL] >>= 2*(SIZE-1-(row+col));
    adjust(std::min(col, SIZE-1 - row), std::min(SIZE-1 - col, row), &result[DIAGONAL]);
    if (row > col) result[SUBDIAGONAL] >>= 2*(row - col);
    adjust(std::min(col, row), std::min(SIZE-1 - col, SIZE-1 - row), &result[SUBDIAGONAL]);
    return result;
}

