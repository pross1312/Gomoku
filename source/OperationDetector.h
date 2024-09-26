#pragma once
#include "BitBoard.h"
#include "ThreatDetector.h"
#include <vector>
#include <optional>

constexpr size_t MAX_DEFS_MOVE = 3;
struct Operation {
    Coord atk;
    Coord defs[MAX_DEFS_MOVE];
    Direction dir;
    ThreatType type;
    Operation(): atk{}, defs{}, type{Threat::None} {}
    Operation(Coord atk, Direction dir, ThreatType type): atk(atk), dir(dir), type(type) {}
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
#define OP_FORMAT "{Atk: "COORD_FORMAT", Defs: ["COORD_FORMAT", "COORD_FORMAT", "COORD_FORMAT"]}, Dir: %s, Threat: %s"
#define FORMAT_OP(op) FORMAT_COORD((op).atk),\
             FORMAT_COORD((op).defs[0]),\
             FORMAT_COORD((op).defs[1]),\
             FORMAT_COORD((op).defs[2]),\
             (op).dir == HORIZONTAL ? "H" : (op).dir == VERTICAL ? "V" : (op).dir == DIAGONAL ? "D" : "AD",\
             Threat::to_text((op).type)
#define LOG_OP(op) TraceLog(LOG_INFO, OP_FORMAT, FORMAT_OP(op))

struct OperationDetector {
    static std::vector<bool> flags;
    static BitBoard* board;

    static std::vector<Operation> find_operations(BitBoard* board, Figure atk_fig);
    static std::vector<Operation> find_operations(BitBoard* board, Coord atk_move);
    static std::vector<Operation> find_operations(BitBoard* board, Coord atk_move, Direction dir);

    static std::vector<Coord> find_defs(BitBoard* board, Coord atk_move, ThreatType lower_limit);

private:
    static void find_operations(std::vector<Operation>& ops, Coord atk_move);
    static void set_flag(Coord move, Direction dir);
    static bool get_flag(Coord move, Direction dir);

    static std::pair<Coord, Coord> find_first_none(Coord pos, Direction dir, Figure atk_fig);
    static void find_defs_move_straight_three(Operation& op, Coord center, Direction dir);
    static void find_defs_move_broken_four(Operation& op, Coord center, Direction dir);
    static void find_ops_straight_two(std::vector<Operation>& ops, Coord move, Direction dir);
    static void find_ops_broken_three(std::vector<Operation>& ops, Coord move, Direction dir);
    static void find_ops_straight_three(std::vector<Operation>& ops, Coord move, Direction dir);
    static void find_ops_broken_four(std::vector<Operation>& ops, Coord move, Direction dir);
    static void find_ops_straight_four(std::vector<Operation>& ops, Coord move, Direction dir);
    // void get_operations(Coord c);
};
