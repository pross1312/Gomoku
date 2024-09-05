#pragma once
#include "BitBoard.h"
#include <vector>
#include <optional>

constexpr size_t MAX_DEFS_MOVE = 3;
struct Operation {
    Coord atk;
    Coord defs[MAX_DEFS_MOVE];
    Operation(Coord atk): atk(atk) {}
    size_t def_count() { return defs[0].is_valid() + defs[1].is_valid() + defs[2].is_valid(); }
    bool operator==(const Operation& that) const {
        return atk == that.atk &&
               defs[0] == that.defs[0] &&
               defs[1] == that.defs[1] &&
               defs[2] == that.defs[2];
    }
};
#define LOG_OP(op) \
    TraceLog(LOG_INFO, \
             "{Atk: "COORD_FORMAT", Defs: ["COORD_FORMAT", "COORD_FORMAT", "COORD_FORMAT"]}",\
             FORMAT_COORD((op).atk),\
             FORMAT_COORD((op).defs[0]),\
             FORMAT_COORD((op).defs[1]),\
             FORMAT_COORD((op).defs[2]))

struct OperationDetector {
    std::vector<bool> flags;
    std::vector<Operation> ops;
    BitBoard* board;
    std::vector<Coord> white_moves, black_moves;

    void find_operations(BitBoard* board, Figure atk_fig);
    void find_operations(Coord atk_move);
    void set_flag(Coord move, Direction dir);
    bool get_flag(Coord move, Direction dir);

    std::pair<Coord, Coord> find_first_none(Coord pos, Direction dir, Figure atk_fig);
    void find_defs_move_straight_three(Operation& op, Coord center, Direction dir);
    void find_ops_straight_two(Coord move, Direction dir);
    void find_ops_broken_three(Coord move, Direction dir);
    void find_ops_straight_three(Coord move, Direction dir);
    void find_ops_broken_four(Coord move, Direction dir);
    void find_ops_straight_four(Coord move, Direction dir);
    // void get_operations(Coord c);
};
