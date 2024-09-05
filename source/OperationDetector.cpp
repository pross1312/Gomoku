#include "OperationDetector.h"
#include "ThreatDetector.h"
#include <cmath>
#include <cassert>
#include <optional>

void OperationDetector::find_operations(BitBoard* board, Figure atk_fig) {
    this->board = board;
    flags.assign(SIZE*SIZE*4, false);
    ops.clear();

    assert(atk_fig == Figure::White || atk_fig == Figure::Black);
    if (atk_fig == Figure::White) {
        for (Coord move : white_moves) {
            assert(board->get_cell(move) == atk_fig);
            find_operations(move);
        }
    } else if (atk_fig == Figure::Black) {
        for (Coord move : black_moves) {
            assert(board->get_cell(move) == atk_fig);
            find_operations(move);
        }
    } else { assert(false && "Unreachable");
    }
}

void OperationDetector::find_operations(Coord atk_move) {
    for (Direction d = HORIZONTAL; d < DIR_COUNT; d += 1) {
        if (get_flag(atk_move, d)) continue;
        set_flag(atk_move, d);
        Line line = board->get_line_radius(atk_move, d);
        switch (ThreatDetector::check(line)) {
            case Threat::None: case Threat::BrokenTwo:
                break;
            case Threat::StraightTwo:
                find_ops_straight_two(atk_move, d);
                break;
            case Threat::BrokenThree:
                find_ops_broken_three(atk_move, d);
                break;
            case Threat::StraightThree:
                find_ops_straight_three(atk_move, d);
                break;
            case Threat::BrokenFour:
                find_ops_broken_four(atk_move, d);
                break;
            case Threat::StraightFour:
                find_ops_straight_four(atk_move, d);
                break;
            case Threat::StraightFive:
                break;
        }
    }
}

void OperationDetector::find_ops_straight_three(Coord center, Direction dir) {
    Figure atk_fig = board->get_cell(center);
    auto[first, second] = find_first_none(center, dir, board->get_cell(center));
    if (first.is_valid() && !get_flag(first, dir)) {
        set_flag(first, dir);
        board->set_cell(first, atk_fig);
        if (ThreatDetector::check(board->get_line_radius(center, dir)) == Threat::StraightFour) {
            Operation op(first);
            op.defs[0] = find_first_none(first, dir, atk_fig).first;
            ops.push_back(op);
        }
        board->set_cell(first, Figure::None);
    }
    if (second.is_valid() && !get_flag(second, dir)) {
        set_flag(second, dir);
        board->set_cell(second, atk_fig);
        if (ThreatDetector::check(board->get_line_radius(center, dir)) == Threat::StraightFour) {
            Operation op(second);
            op.defs[0] = find_first_none(second, dir, atk_fig).second;
            ops.push_back(op);
        }
        board->set_cell(second, Figure::None);
    }
}

void OperationDetector::find_ops_straight_four(Coord center, Direction dir) {
    auto[first, second] = find_first_none(center, dir, board->get_cell(center));
    assert(first.is_valid() && second.is_valid());
    if (!get_flag(first, dir)) {
        set_flag(first, dir);
        ops.push_back(Operation(first));
    }
    if (!get_flag(second, dir)) {
        set_flag(second, dir);
        ops.push_back(Operation(second));
    }
}

void OperationDetector::find_ops_broken_four(Coord center, Direction dir) {
    Figure atk_fig = board->get_cell(center);
    auto[first, second] = find_first_none(center, dir, board->get_cell(center));
    bool found = false;
    if (first.is_valid() && !get_flag(first, dir)) {
        set_flag(first, dir);
        board->set_cell(first, atk_fig);
        if (ThreatDetector::check(board->get_line_radius(center, dir)) == Threat::StraightFive) {
            ops.push_back(Operation(first));
            found = true;
        }
        board->set_cell(first, Figure::None);
    }
    if (!found && second.is_valid() && !get_flag(second, dir)) {
        set_flag(second, dir);
        board->set_cell(second, atk_fig);
        if (ThreatDetector::check(board->get_line_radius(center, dir)) == Threat::StraightFive) {
            ops.push_back(Operation(second));
        }
        board->set_cell(second, Figure::None);
    }
}

void OperationDetector::find_ops_straight_two(Coord center, Direction dir) {
    auto[start_1, start_2] = find_first_none(center, dir, board->get_cell(center));
    const auto find_op = [this, center, dir](Coord potential_atk, const Coord dir_vec) {
        const Figure atk_fig = board->get_cell(center);
        while (potential_atk.is_valid() && board->get_cell(potential_atk) == Figure::None) {
            if (get_flag(potential_atk, dir)) {
                potential_atk += dir_vec;
                continue;
            }
            set_flag(potential_atk, dir);

            board->set_cell(potential_atk, atk_fig);
            Line line = board->get_line_radius(potential_atk, dir);
            if (ThreatDetector::check(line) == Threat::StraightThree) {
                Operation op(potential_atk);

                find_defs_move_straight_three(op, potential_atk, dir);

                ops.push_back(op);
            }
            board->set_cell(potential_atk, Figure::None);

            potential_atk += dir_vec;
        }
    };
    find_op(start_1, start_1 - center);
    find_op(start_2, start_2 - center);
}

void OperationDetector::find_defs_move_straight_three(Operation& op, const Coord center, Direction dir) {
    Figure atk_fig = board->get_cell(center);
    assert(IS_PIECE(atk_fig));

    auto[left, right] = find_first_none(center, dir, atk_fig);
    assert(left.is_valid() && right.is_valid());

    int8_t distance = std::abs(left.col - right.col);

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

// TODO: optimise this
//       maybe get rid of unnecessary second check
//       as it's always the reverse of the first one
void OperationDetector::find_ops_broken_three(Coord center, Direction dir) {
    auto[start_1, start_2] = find_first_none(center, dir, board->get_cell(center));

    const auto find_op = [this, center, dir](Coord potential_atk, const Coord dir_vec) {
        const Figure atk_fig = board->get_cell(center);
        while (potential_atk.is_valid() && board->get_cell(potential_atk) == Figure::None) {
            if (get_flag(potential_atk, dir)) {
                potential_atk += dir_vec;
                continue;
            }
            set_flag(potential_atk, dir);

            board->set_cell(potential_atk, atk_fig);
            Line line = board->get_line_radius(potential_atk, dir);
            if (ThreatDetector::check(line) == Threat::BrokenFour) {
                Operation op(potential_atk);
                size_t def_count = 0;
                auto[def_1, def_2] = find_first_none(potential_atk, dir, atk_fig);

                if (def_1.is_valid()) {
                    board->set_cell(def_1, atk_fig);
                    line = board->get_line_radius(potential_atk, dir);
                    if (ThreatDetector::check(line) == Threat::StraightFive) {
                        op.defs[def_count++] = def_1;
                    }
                    board->set_cell(def_1, Figure::None);
                }

                if (def_2.is_valid()) {
                    board->set_cell(def_2, atk_fig);
                    line = board->get_line_radius(potential_atk, dir);
                    if (ThreatDetector::check(line) == Threat::StraightFive) {
                        op.defs[def_count++] = def_2;
                    }
                    board->set_cell(def_2, Figure::None);
                }

                ops.push_back(op);
            }
            board->set_cell(potential_atk, Figure::None);
            potential_atk += dir_vec;
        }
    };
    find_op(start_1, start_1 - center);
    find_op(start_2, start_2 - center);
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


