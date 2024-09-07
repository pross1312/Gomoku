#pragma once
#include <cstdint>
#include <cstdio>
#include <array>
#include <raylib.h>

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
#define IS_PIECE(figure) ((figure) != Figure::None && (figure) != Figure::Out)
#define OPPOSITE_FIG(_cell) ((_cell) == Figure::White ? Figure::Black : Figure::White)
#define INVALID_COORD (Coord{})
#define COORD_FORMAT "(%d, %d)"
#define FORMAT_COORD(coord) (coord).row, (coord).col
#define LOG_COORD(c) TraceLog(LOG_INFO, COORD_FORMAT, FORMAT_COORD(c))

struct Coord {
    int8_t row, col;
    Coord(): row{-1}, col{-1} {}
    Coord(int8_t row, int8_t col): row{row}, col{col} {}
    bool operator==(Coord c) const {
        return row == c.row && col == c.col;
    }
    bool is_valid() const { return row >= 0 && row < SIZE && col >= 0 && col < SIZE; }
    Coord operator*(int8_t x) const { return Coord(row*x, col*x); }
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

typedef uint32_t Line;
typedef std::array<Line, 4> Line4;
typedef size_t Direction;
enum : size_t {
    HORIZONTAL = 0,
    VERTICAL,
    DIAGONAL,
    SUBDIAGONAL,
    DIR_COUNT,
};
const Coord DIR_VECS[DIR_COUNT] {
    [HORIZONTAL] = Coord(0, 1),
    [VERTICAL] = Coord(1, 0),
    [DIAGONAL] = Coord(1, -1),
    [SUBDIAGONAL] = Coord(1, 1),
};

struct BitBoard {
    std::array<Line, SIZE> h_lines;
    std::array<Line, SIZE> v_lines;
    std::array<Line, DIAG_SIZE> main_d_lines;
    std::array<Line, DIAG_SIZE> sub_d_lines;

    BitBoard();

    void clear();

    Figure get_cell(size_t row, size_t col) const;
    Figure get_cell(Coord coord) const { return get_cell((size_t)coord.row, (size_t)coord.col); }

    void set_cell(size_t row, size_t col, Figure cell);
    void set_cell(Coord coord, Figure cell) { set_cell((size_t)coord.row, (size_t)coord.col, cell); };

    Line get_line(size_t row, size_t col, Direction dir) const;
    Line4 get_lines(size_t row, size_t col) const;
    Line4 get_lines(Coord coord) const { return get_lines((size_t)coord.row, (size_t)coord.col); }

    Line get_line_radius(size_t row, size_t col, Direction dir) const;
    Line get_line_radius(Coord coord, Direction dir) const { return get_line_radius(coord.row, coord.col, dir); }
    Line4 get_lines_radius(size_t row, size_t col) const; // get RADIUS pieces around pos
    Line4 get_lines_radius(Coord coord) const { return get_lines_radius((size_t)coord.row, (size_t)coord.col); } // get RADIUS pieces around pos
};
