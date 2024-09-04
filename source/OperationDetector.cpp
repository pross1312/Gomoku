#include "OperationDetector.h"
#include "ThreatDetector.h"
#include <cassert>
#include <optional>
using namespace std;

void OperationDetector::get_operations(BitBoard* board, Figure atk_fig) {
    this->board = board;
    flags.assign(flags.size(), false);
    ops.clear();

    assert(atk_fig == Figure::White || atk_fig == Figure::Black);
    if (atk_fig == Figure::White) {
        for (Coord move : white_moves) {
            get_operations(move);
        }
    } else if (atk_fig == Figure::Black) {
        for (Coord move : black_moves) {
            get_operations(move);
        }
    } else {
        assert(false && "Unreachable");
    }
}

pair<optional<Coord>, optional<Coord>> OperationDetector::find_first_none(Coord pos, Direction dir) {
    Figure atk_fig = board->get_cell(pos);
    pair<optional<Coord>, optional<Coord>> result;
    assert(atk_fig != Figure::None);
    Coord temp = pos + DIR_VECS[dir];
    while (temp.is_valid() && board->get_cell(temp) == atk_fig) {
        temp += DIR_VECS[dir];
    }
    if (temp.is_valid() && board->get_cell(temp) == Figure::None) {
        result.second = temp;
    } else {
        result.second = nullopt;
    }

    temp = pos - DIR_VECS[dir];
    while (temp.is_valid() && board->get_cell(temp) == atk_fig) {
        temp -= DIR_VECS[dir];
    }
    if (temp.is_valid() && board->get_cell(temp) == Figure::None) {
        result.first = temp;
    } else {
        result.first = nullopt;
    }
    return result;
}

void OperationDetector::get_operations(Coord move) {
    assert(board->get_cell(move) != Figure::None);
    Line4 lines = board->get_lines_radius(move);
    ThreatType threat = ThreatDetector::check(lines.h);
    switch (threat) {
        case Threat::BrokenThree: {
            find_op_broken_three(move);
        } break;
        case Threat::StraightFive: case Threat::StraightFour: case Threat::Two: case Threat::None:
            break;
        default:
            assert(false && "Unimplemented");
    }
}

void OperationDetector::find_op_broken_three(Coord move) {
    Figure atk_fig = board->get_cell(move);
    auto[atk_1, atk_2] = find_first_none(move, Direction::Horizontal);
    const auto check = [this, atk_fig](optional<Coord> atk_move) {
        if (atk_move.has_value() && !get_flag(atk_move.value(), Direction::Horizontal)) {
            set_flag(atk_move.value(), Direction::Horizontal);
            Operation op;
            op.atk = atk_move.value();
            board->set_cell(atk_move.value(), atk_fig);
            auto[def_1, def_2] = find_first_none(atk_move.value(), Direction::Horizontal);
            size_t i = 0;
            if (def_1.has_value()) {
                board->set_cell(def_1.value(), atk_fig);
                if (ThreatDetector::check(board->get_lines_radius(def_1.value()).h) == Threat::StraightFive) {
                    op.def[i++] = def_1.value();
                }
                board->set_cell(def_1.value(), Figure::None);
            }
            if (def_2.has_value()) {
                board->set_cell(def_2.value(), atk_fig);
                if (ThreatDetector::check(board->get_lines_radius(def_2.value()).h) == Threat::StraightFive) {
                    op.def[i++] = def_2.value();
                }
                board->set_cell(def_2.value(), Figure::None);
            }
            board->set_cell(atk_move.value(), Figure::None);
            assert(i > 0);
            ops.push_back(op);
        }
    };
    check(atk_1);
    check(atk_2);
}


void something(BitBoard& board, Coord move, vector<Operation>& ops) {
    (void)board, (void)move, (void)ops;
}

void OperationDetector::set_flag(Coord move, Direction dir) {
    assert(move.is_valid());
    size_t index = (size_t)move.row*SIZE + move.col;
    flags[(dir+1)*index] = true;
}

bool OperationDetector::get_flag(Coord move, Direction dir) {
    assert(move.is_valid());
    size_t index = (size_t)move.row*SIZE + move.col;
    return flags[(dir+1)*index];
}
