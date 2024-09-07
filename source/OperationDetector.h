#pragma once
#include "BitBoard.h"
#include <vector>
#include <optional>

constexpr size_t MAX_DEFS_MOVE = 3;
struct Operation {
    Coord atk;
    Coord defs[MAX_DEFS_MOVE];
    Direction dir;
    Operation(): atk{}, defs{} {}
    Operation(Coord atk, Direction dir): atk(atk), dir(dir) {}
    size_t def_count() const { return defs[0].is_valid() + defs[1].is_valid() + defs[2].is_valid(); }
    bool operator!=(const Operation& that) const { return !(this->operator==(that)); }
    bool operator==(const Operation& that) const {
        return atk == that.atk &&
               defs[0] == that.defs[0] &&
               defs[1] == that.defs[1] &&
               defs[2] == that.defs[2];
    }
};
#define INVALID_OP (Operation{})
#define OP_FORMAT "{Atk: "COORD_FORMAT", Defs: ["COORD_FORMAT", "COORD_FORMAT", "COORD_FORMAT"]}"
#define FORMAT_OP(op) FORMAT_COORD((op).atk),\
             FORMAT_COORD((op).defs[0]),\
             FORMAT_COORD((op).defs[1]),\
             FORMAT_COORD((op).defs[2])
#define LOG_OP(op) TraceLog(LOG_INFO, OP_FORMAT, FORMAT_OP(op))

struct OperationDetector {
    std::vector<bool> flags;
    BitBoard* board;
    size_t count = 0;

    std::vector<Operation> find_operations(BitBoard* board, const std::vector<Coord>& moves);
    std::vector<Operation> find_operations(BitBoard* board, Coord atk_move);

private:
    void find_operations(std::vector<Operation>& ops, Coord atk_move);
    void set_flag(Coord move, Direction dir);
    bool get_flag(Coord move, Direction dir);

    std::pair<Coord, Coord> find_first_none(Coord pos, Direction dir, Figure atk_fig);
    void find_defs_move_straight_three(Operation& op, Coord center, Direction dir);
    void find_defs_move_broken_four(Operation& op, Coord center, Direction dir);
    void find_ops_straight_two(std::vector<Operation>& ops, Coord move, Direction dir);
    void find_ops_broken_three(std::vector<Operation>& ops, Coord move, Direction dir);
    void find_ops_straight_three(std::vector<Operation>& ops, Coord move, Direction dir);
    void find_ops_broken_four(std::vector<Operation>& ops, Coord move, Direction dir);
    void find_ops_straight_four(std::vector<Operation>& ops, Coord move, Direction dir);
    // void get_operations(Coord c);
};
