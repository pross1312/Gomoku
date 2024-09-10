#include "OperationDetector.h"
#include "ThreatDetector.h"
#include <cmath>
#include <cassert>
#include <optional>

// NOTE: call reset before doing anything here
std::vector<Operation> OperationDetector::find_operations(BitBoard* board, const std::vector<Coord>& moves) {
    this->board = board;
    this->flags.assign(SIZE*SIZE*4, false);
    std::vector<Operation> result;
    for (Coord move : moves) {
        assert(IS_PIECE(board->get_cell(move)));
        find_operations(result, move);
    }
    return result;
}

std::vector<Operation> OperationDetector::find_operations(BitBoard* board, Coord atk_move) {
    std::vector<Operation> result;
    this->board = board;
    this->flags.assign(SIZE*SIZE*4, false);
    find_operations(result, atk_move);
    return result;
}

void OperationDetector::find_operations(std::vector<Operation>& ops, Coord atk_move) {
    for (Direction d = HORIZONTAL; d < DIR_COUNT; d += 1) {
        if (get_flag(atk_move, d)) continue;
        set_flag(atk_move, d);
        Line line = board->get_line_radius(atk_move, d);
        switch (ThreatDetector::check(line)) {
            case Threat::None: case Threat::BrokenTwo:
                break;
            case Threat::StraightTwo:
                find_ops_straight_two(ops, atk_move, d);
                break;
            case Threat::BrokenThree:
                find_ops_broken_three(ops, atk_move, d);
                break;
            case Threat::StraightThree:
                find_ops_straight_three(ops, atk_move, d);
                break;
            case Threat::BrokenFour:
                find_ops_broken_four(ops, atk_move, d);
                break;
            case Threat::StraightFour:
                find_ops_straight_four(ops, atk_move, d);
                break;
            case Threat::StraightFive:
                break;
        }
    }
}

// NOTE:            __XXX__     __XX_X___          -> radius = 3
void OperationDetector::find_ops_straight_three(std::vector<Operation>& ops, Coord center, Direction dir) {
    const Figure atk_fig = board->get_cell(center);
    for (int8_t i = 1; i <= 3; i++) {
        Coord left = center - DIR_VECS[dir]*i;
        if (left.is_valid() && board->get_cell(left) == Figure::None && !get_flag(left, dir)) {
            set_flag(left, dir);

            board->set_cell(left, atk_fig);
            if (ThreatDetector::check(board->get_line_radius(left, dir)) == Threat::StraightFour) {
                Operation op(left, dir, Threat::StraightFour);
                op.defs[0] = find_first_none(left, dir, atk_fig).second;
                ops.push_back(op);
            }
            board->set_cell(left, Figure::None);
        }
        Coord right = center + DIR_VECS[dir]*i;
        if (right.is_valid() && board->get_cell(right) == Figure::None && !get_flag(right, dir)) {
            set_flag(right, dir);

            board->set_cell(right, atk_fig);
            if (ThreatDetector::check(board->get_line_radius(right, dir)) == Threat::StraightFour) {
                Operation op(right, dir, Threat::StraightFour);
                op.defs[0] = find_first_none(right, dir, atk_fig).first;
                ops.push_back(op);
            }
            board->set_cell(right, Figure::None);
        }
    }
}

void OperationDetector::find_ops_straight_four(std::vector<Operation>& ops, Coord center, Direction dir) {
    auto[first, second] = find_first_none(center, dir, board->get_cell(center));
    assert(first.is_valid() && second.is_valid());
    if (!get_flag(first, dir)) {
        set_flag(first, dir);
        ops.push_back(Operation(first, dir, Threat::StraightFive));
    }
    if (!get_flag(second, dir)) {
        set_flag(second, dir);
        ops.push_back(Operation(second, dir, Threat::StraightFive));
    }
}

void OperationDetector::find_ops_broken_four(std::vector<Operation>& ops, Coord center, Direction dir) {
    Figure atk_fig = board->get_cell(center);
    auto[first, second] = find_first_none(center, dir, board->get_cell(center));
    bool found = false;
    if (first.is_valid() && !get_flag(first, dir)) {
        set_flag(first, dir);
        board->set_cell(first, atk_fig);
        if (ThreatDetector::check(board->get_line_radius(center, dir)) == Threat::StraightFive) {
            ops.push_back(Operation(first, dir, Threat::StraightFive));
            found = true;
        }
        board->set_cell(first, Figure::None);
    }
    if (!found && second.is_valid() && !get_flag(second, dir)) {
        set_flag(second, dir);
        board->set_cell(second, atk_fig);
        if (ThreatDetector::check(board->get_line_radius(center, dir)) == Threat::StraightFive) {
            ops.push_back(Operation(second, dir, Threat::StraightFive));
        }
        board->set_cell(second, Figure::None);
    }
}

// NOTE:            __XX___     ___X_X___       _X__X_      -> radius = 3
void OperationDetector::find_ops_straight_two(std::vector<Operation>& ops, Coord center, Direction dir) {
    const Figure atk_fig = board->get_cell(center);
    for (int8_t i = 1; i <= 3; i++) {
        Coord left = center - DIR_VECS[dir]*i;
        if (left.is_valid() && board->get_cell(left) == Figure::None && !get_flag(left, dir)) {
            set_flag(left, dir);

            board->set_cell(left, atk_fig);
            if (ThreatDetector::check(board->get_line_radius(left, dir)) == Threat::StraightThree) {
                Operation op(left, dir, Threat::StraightThree);
                find_defs_move_straight_three(op, left, dir);
                ops.push_back(op);
            }
            board->set_cell(left, Figure::None);
        }
        Coord right = center + DIR_VECS[dir]*i;
        if (right.is_valid() && board->get_cell(right) == Figure::None && !get_flag(right, dir)) {
            set_flag(right, dir);

            board->set_cell(right, atk_fig);
            if (ThreatDetector::check(board->get_line_radius(right, dir)) == Threat::StraightThree) {
                Operation op(right, dir, Threat::StraightThree);
                find_defs_move_straight_three(op, right, dir);
                ops.push_back(op);
            }
            board->set_cell(right, Figure::None);
        }
    }
}

void OperationDetector::find_defs_move_straight_three(Operation& op, const Coord center, Direction dir) {
    Figure atk_fig = board->get_cell(center);
    assert(IS_PIECE(atk_fig));

    auto[left, right] = find_first_none(center, dir, atk_fig);
    assert(left.is_valid() && right.is_valid());

    int8_t distance = std::max(std::abs(left.col - right.col), std::abs(left.row - right.row));

    // 3 case: _0C0_        _0C_0_           _00_C_
    assert(distance == 4 || distance == 3 || distance == 2);
    op.defs[0] = left;
    op.defs[1] = right;

    if (distance != 4) {
        const auto check_last_def_move = [&](Coord potential_coord) -> bool {
            bool found = false; 
            if (potential_coord.is_valid()) {
                board->set_cell(potential_coord, atk_fig);
                if (ThreatDetector::check(board->get_line_radius(center, dir)) == Threat::BrokenFour) {
                    found = true;
                }
                board->set_cell(potential_coord, Figure::None);
            }
            return found;
        };
        // find_first_none -> left (-), right (+)
        Coord potential_1 = find_first_none(right, dir, atk_fig).second;
        Coord potential_2 = find_first_none(left, dir, atk_fig).first;
        if (check_last_def_move(potential_1))      op.defs[2] = potential_1;
        else if (check_last_def_move(potential_2)) op.defs[2] = potential_2;
        else assert(false && "Can't find last defense move");
    }
}

void OperationDetector::find_defs_move_broken_four(Operation& op, const Coord center, Direction dir) {
    const Figure atk_fig = board->get_cell(center);
    auto[def_1, def_2] = find_first_none(center, dir, atk_fig);
    if (def_1.is_valid() && board->get_cell(def_1) == Figure::None) {
        board->set_cell(def_1, atk_fig);
        if (ThreatDetector::check(board->get_line_radius(center, dir)) == Threat::StraightFive) {
            op.defs[0] = def_1;
            board->set_cell(def_1, Figure::None);
            return;
        }
        board->set_cell(def_1, Figure::None);
    }
    if (def_2.is_valid() && board->get_cell(def_2) == Figure::None) {
        board->set_cell(def_2, atk_fig);
        if (ThreatDetector::check(board->get_line_radius(center, dir)) == Threat::StraightFive) {
            op.defs[0] = def_2;
            board->set_cell(def_2, Figure::None);
            return;
        }
        board->set_cell(def_2, Figure::None);
    }
}

// NOTE:            _OXXX__     _OXX_X___   _OX_XX_         -> radius = 4
void OperationDetector::find_ops_broken_three(std::vector<Operation>& ops, Coord center, Direction dir) {
    const Figure atk_fig = board->get_cell(center);
    for (int8_t i = 1; i <= 4; i++) {
        Coord left = center - DIR_VECS[dir]*i;
        if (left.is_valid() && board->get_cell(left) == Figure::None && !get_flag(left, dir)) {
            set_flag(left, dir);

            board->set_cell(left, atk_fig);
            if (ThreatDetector::check(board->get_line_radius(left, dir)) == Threat::BrokenFour) {
                Operation op(left, dir, Threat::BrokenFour);
                find_defs_move_broken_four(op, left, dir);
                ops.push_back(op);

                assert(!get_flag(op.defs[0], dir));
                set_flag(op.defs[0], dir);
                Operation op2(op.defs[0], dir, Threat::BrokenFour);
                op2.defs[0] = op.atk;
                ops.push_back(op2);
                board->set_cell(left, Figure::None);
                return;
            }
            board->set_cell(left, Figure::None);
        }
        Coord right = center + DIR_VECS[dir]*i;
        if (right.is_valid() && board->get_cell(right) == Figure::None && !get_flag(right, dir)) {
            set_flag(right, dir);

            board->set_cell(right, atk_fig);
            if (ThreatDetector::check(board->get_line_radius(right, dir)) == Threat::BrokenFour) {
                Operation op(right, dir, Threat::BrokenThree);
                find_defs_move_broken_four(op, right, dir);
                ops.push_back(op);

                assert(!get_flag(op.defs[0], dir));
                set_flag(op.defs[0], dir);
                Operation op2(op.defs[0], dir, Threat::BrokenThree);
                op2.defs[0] = op.atk;
                ops.push_back(op2);
                board->set_cell(right, Figure::None);
                return;
            }
            board->set_cell(right, Figure::None);
        }
    }
}

void OperationDetector::set_flag(Coord move, Direction dir) {
    assert(move.is_valid());
    size_t index = (size_t)move.row*SIZE + move.col;
    flags[index + dir*SIZE*SIZE] = true;
}

bool OperationDetector::get_flag(Coord move, Direction dir) {
    assert(move.is_valid());
    size_t index = (size_t)move.row*SIZE + move.col;
    count += flags[index + dir*SIZE*SIZE];
    return flags[index + dir*SIZE*SIZE];
}

// return [left (pos -.. dir), right (pos +.. dir)]
std::pair<Coord, Coord> OperationDetector::find_first_none(Coord pos, Direction dir, Figure atk_fig) {
    assert(IS_PIECE(atk_fig));

    std::pair<Coord, Coord> result;

    Coord temp = pos + DIR_VECS[dir];
    while (temp.is_valid() && board->get_cell(temp) == atk_fig) temp += DIR_VECS[dir];
    if (temp.is_valid() && board->get_cell(temp) == Figure::None) {
        result.second = temp;
    }

    temp = pos - DIR_VECS[dir];
    while (temp.is_valid() && board->get_cell(temp) == atk_fig) temp -= DIR_VECS[dir];
    if (temp.is_valid() && board->get_cell(temp) == Figure::None) {
        result.first = temp;
    }

    return result;
}


