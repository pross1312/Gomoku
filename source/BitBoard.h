#pragma once
#include <cstdint>
#include <cstdio>
#include <array>

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
#define RADIUS 5

enum Figure : uint8_t {
    Out = 0b00,
    Black = 0b01,
    White = 0b10,
    None = 0b11,
};
#define OPPOSITE_FIG(_cell) ((_cell) == Figure::White ? Figure::Black : Figure::White)
#define INVALID_COORD (Coord{})
struct Coord {
    int8_t row, col;
    Coord(): row{-1}, col{-1} {}
    Coord(int8_t row, int8_t col): row{row}, col{col} {}
    bool operator==(Coord c) const {
        return row == c.row && col == c.col;
    }
    bool is_valid() { return row >= 0 && row < SIZE && col >= 0 && col < SIZE; }
    Coord operator+(const Coord& b) const { return Coord(row + b.row, col + b.col); }
    Coord operator-(const Coord& b) const { return Coord(row - b.row, col - b.col); }
    Coord& operator+=(const Coord& b) {
        row += b.row;
        col += b.col;
        return *this;
    }
    Coord& operator-=(const Coord& b) {
        row -= b.row;
        col -= b.col;
        return *this;
    }
};

struct Line4 {
    uint32_t h;
    uint32_t v;
    uint32_t main_d;
    uint32_t sub_d;
};
struct BitBoard {
    std::array<uint32_t, SIZE> h_lines;
    std::array<uint32_t, SIZE> v_lines;
    std::array<uint32_t, DIAG_SIZE> main_d_lines;
    std::array<uint32_t, DIAG_SIZE> sub_d_lines;

    BitBoard();

    void clear();

    Figure get_cell(size_t row, size_t col) const;
    Figure get_cell(Coord coord) const { return get_cell((size_t)coord.row, (size_t)coord.col); }

    void set_cell(size_t row, size_t col, Figure cell);
    void set_cell(Coord coord, Figure cell) { set_cell((size_t)coord.row, (size_t)coord.col, cell); };

    Line4 get_lines(size_t row, size_t col) const;
    Line4 get_lines(Coord coord) const { return get_lines((size_t)coord.row, (size_t)coord.col); }

    Line4 get_lines_radius(size_t row, size_t col) const; // get RADIUS pieces around pos
    Line4 get_lines_radius(Coord coord) const { return get_lines_radius((size_t)coord.row, (size_t)coord.col); } // get RADIUS pieces around pos
};
