#pragma once
#include <cstdint>
#include <cstdio>

// 15x15
// 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11
// 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11
// 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11
// 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11
// 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11
// 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11
// 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11
// 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11
// 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11
// 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11
// 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11
// 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11
// 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11
// 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11
// 11 11 11 11 11 11 11 11 11 11 11 11 11 11 11

//Main diag (/)                        Sub diag (\)
//# _ _ _ _ _ _ _ _ _ _ _ _ _ _        _ _ _ _ _ _ _ _ _ _ _ _ _ _ #
//# # _ _ _ _ _ _ _ _ _ _ _ _ _        _ _ _ _ _ _ _ _ _ _ _ _ _ # #
//# # # _ _ _ _ _ _ _ _ _ _ _ _        _ _ _ _ _ _ _ _ _ _ _ _ # # #
//# # # # _ _ _ _ _ _ _ _ _ _ _        _ _ _ _ _ _ _ _ _ _ _ # # # #
//# # # # # _ _ _ _ _ _ _ _ _ _        _ _ _ _ _ _ _ _ _ _ # # # # #
//# # # # # # _ _ _ _ _ _ _ _ _        _ _ _ _ _ _ _ _ _ # # # # # #
//# # # # # # # _ _ _ _ _ _ _ _        _ _ _ _ _ _ _ _ # # # # # # #
//# # # # # # # # _ _ _ _ _ _ _        _ _ _ _ _ _ _ # # # # # # # #
//# # # # # # # # # _ _ _ _ _ _        _ _ _ _ _ _ # # # # # # # # #
//# # # # # # # # # # _ _ _ _ _        _ _ _ _ _ # # # # # # # # # #
//# # # # # # # # # # # _ _ _ _        _ _ _ _ # # # # # # # # # # #
//# # # # # # # # # # # # _ _ _        _ _ _ # # # # # # # # # # # #
//# # # # # # # # # # # # # _ _        _ _ # # # # # # # # # # # # #
//# # # # # # # # # # # # # # _        _ # # # # # # # # # # # # # #
//# # # # # # # # # # # # # # #        # # # # # # # # # # # # # # #
//_ # # # # # # # # # # # # # #        # # # # # # # # # # # # # # _
//_ _ # # # # # # # # # # # # #        # # # # # # # # # # # # # _ _
//_ _ _ # # # # # # # # # # # #        # # # # # # # # # # # # _ _ _
//_ _ _ _ # # # # # # # # # # #        # # # # # # # # # # # _ _ _ _
//_ _ _ _ _ # # # # # # # # # #        # # # # # # # # # # _ _ _ _ _
//_ _ _ _ _ _ # # # # # # # # #        # # # # # # # # # _ _ _ _ _ _
//_ _ _ _ _ _ _ # # # # # # # #        # # # # # # # # _ _ _ _ _ _ _
//_ _ _ _ _ _ _ _ # # # # # # #        # # # # # # # _ _ _ _ _ _ _ _
//_ _ _ _ _ _ _ _ _ # # # # # #        # # # # # # _ _ _ _ _ _ _ _ _
//_ _ _ _ _ _ _ _ _ _ # # # # #        # # # # # _ _ _ _ _ _ _ _ _ _
//_ _ _ _ _ _ _ _ _ _ _ # # # #        # # # # _ _ _ _ _ _ _ _ _ _ _
//_ _ _ _ _ _ _ _ _ _ _ _ # # #        # # # _ _ _ _ _ _ _ _ _ _ _ _
//_ _ _ _ _ _ _ _ _ _ _ _ _ # #        # # _ _ _ _ _ _ _ _ _ _ _ _ _
//_ _ _ _ _ _ _ _ _ _ _ _ _ _ #        # _ _ _ _ _ _ _ _ _ _ _ _ _ _

#define SIZE 15
#define DIAG_SIZE (2*SIZE-1)
#define RADIUS 5 // for get_pieces_radius, must be odd and greater or equal to 3

enum Cell : uint8_t {
    Out = 0b00,
    Black = 0b01,
    White = 0b10,
    None = 0b11,
};
struct Line4 {
    uint32_t h;
    uint32_t v;
    uint32_t main_d;
    uint32_t sub_d;
};
struct BitBoard {
    uint32_t h_lines[SIZE];
    uint32_t v_lines[SIZE];
    uint32_t main_d_lines[DIAG_SIZE];
    uint32_t sub_d_lines[DIAG_SIZE];

    BitBoard();

    Cell get_cell(size_t row, size_t col) const;
    void set_cell(size_t row, size_t col, Cell cell);
    Line4 get_lines(size_t row, size_t col) const;
    Line4 get_lines_radius(size_t row, size_t col) const; // get RADIUS pieces around pos
};

inline Cell inverse(Cell c) { return Cell(0b11 - c); }
