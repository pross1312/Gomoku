#pragma once
#include "BitBoard.h"
#include <vector>
#include <optional>

struct Operation {
    Coord atk;
    Coord def[3];
    bool operator==(const Operation& that) const {
        return atk == that.atk &&
               def[0] == that.def[0] &&
               def[1] == that.def[1] &&
               def[2] == that.def[2];
    }
};

enum Direction {
    Horizontal = 0,
    Vertical,
    Diagonal,
    SubDiagonal,
};
const Coord DIR_VECS[4] {
    [Direction::Horizontal] = Coord(0, 1),
    [Direction::Vertical] = Coord(1, 0),
    [Direction::Diagonal] = Coord(1, -1),
    [Direction::SubDiagonal] = Coord(1, 1),
};

struct OperationDetector {
    std::vector<bool> flags;
    std::vector<Operation> ops;
    BitBoard* board;
    OperationDetector(): flags(SIZE*SIZE*4, false) {}

    std::vector<Coord> white_moves, black_moves;
    void get_operations(BitBoard* board, Figure atk_fig);
    std::pair<std::optional<Coord>, std::optional<Coord>> find_first_none(Coord pos, Direction dir);
    void find_op_broken_three(Coord move);
    void get_operations(Coord c);
    void set_flag(Coord move, Direction dir);
    bool get_flag(Coord move, Direction dir);
};
