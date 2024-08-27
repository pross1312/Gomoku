#include "BitBoard.h"
#include <cassert>

BitBoard::BitBoard() {
    for (uint32_t &line : h_lines) line = (1 << 2*SIZE) - 1;
    for (uint32_t &line : v_lines) line = (1 << 2*SIZE) - 1;
    for (size_t i = 1; i <= SIZE; i++) {
        uint32_t bits = (1 << i*2) - 1;
        main_d_lines[i-1]         = bits << (SIZE-i)*2;
        main_d_lines[DIAG_SIZE-i] = bits;
    }
    for (size_t i = 1; i <= SIZE; i++) {
        uint32_t bits = (1 << i*2) - 1;
        sub_d_lines[i-1]         = bits;
        sub_d_lines[DIAG_SIZE-i] = bits << (SIZE-i)*2;
    }
}

Cell BitBoard::get_cell(size_t row, size_t col) const {
    assert(row < SIZE && col < SIZE && "Out of bound");
    return Cell((h_lines[row] >> 2*(SIZE-1 - col)) & 0b11);
}

void BitBoard::set_cell(size_t row, size_t col, Cell cell) {
    assert(cell != Cell::Out && "Invalid cell");
    assert(row < SIZE && col < SIZE && "Out of bound");
    static const auto set = [&cell](size_t right_pieces, uint32_t *line) {
        *line |= (0b11 << 2*right_pieces);
        *line &= ~(inverse(cell) << 2*right_pieces);
    };
    set(SIZE-1 - col, &h_lines[row]);
    set(SIZE-1 - row, &v_lines[col]);
    size_t right_out_gap = row+col < SIZE ? SIZE-1-(row+col) : 0;
    set(SIZE-1 - col + right_out_gap, &main_d_lines[row+col]);
    right_out_gap = row > col ? row - col : 0;
    set(SIZE-1 - col + right_out_gap, &sub_d_lines[(SIZE-1-col) + row]);
}

Line4 BitBoard::get_lines(size_t row, size_t col) {
    assert(row < SIZE && col < SIZE && "Out of bound");
    return Line4{
        .h = h_lines[row],
        .v = v_lines[col],
        .main_d = main_d_lines[row+col],
        .sub_d = sub_d_lines[(SIZE-1-col) + row],
    };
}

// (row, col) at center RADIUS*2 bits in total
Line4 BitBoard::get_lines_radius(size_t row, size_t col) {
    Line4 result = get_lines(row, col);
    static const auto adjust = [&](size_t left_pieces, size_t right_pieces, uint32_t *line) {
        if (left_pieces > RADIUS)  *line &=  (1 << (right_pieces + 1 + RADIUS)*2) - 1;
        if (right_pieces < RADIUS) *line <<= (RADIUS - right_pieces)*2;
        else                     *line >>= (right_pieces - RADIUS)*2; // left + 1 + right - (left + 1 + RADIUS)
        *line &= (1 << (RADIUS*2 + 1)*2) - 1;
    };
    adjust(col, SIZE-1-col, &result.h);
    adjust(row, SIZE-1-row, &result.v);
    if (row + col < SIZE) result.main_d >>= 2*(SIZE-1-(row+col));
    adjust(SIZE-1-row, SIZE-1-col, &result.main_d);
    if (row > col) result.sub_d >>= 2*(row - col);
    adjust(row, SIZE-1-col, &result.sub_d);
    return result;
}

