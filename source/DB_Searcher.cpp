#include "DB_Searcher.h"
#include <cmath>
#include <cassert>

void DB_Searcher::search(BitBoard* board, Coord last_move) {
    tree_sized_growed = true;
    root = make_db_node(DB_Node::Type::Combination, 0, Operation(), std::initializer_list<DB_NodePtr>{nullptr});
    this->board = board;
    // detector.reset(board);

    size_t level = 1;
    while (tree_sized_growed) {
        tree_sized_growed = false;
        dependency_stage(root, last_move, level, 0, 2);
        combination_stage(level, board->get_cell(last_move));
        level += 1;
        break;
    }
}

void DB_Searcher::dependency_stage(DB_NodePtr node, Coord last_move, const size_t level, size_t depth, size_t max_depth) {
    if (depth >= max_depth) return;
    if (node->level + 1 == level) {
        // if (node->type == DB_Node::Type::Combination) {
        const Figure atk_fig = board->get_cell(last_move);
        assert(IS_PIECE(atk_fig));

        std::vector ops = detector.find_operations(board, last_move);
        for (const Operation& op : ops) {
            DB_NodePtr child = make_db_node(DB_Node::Type::Dependency, node->level, op, std::initializer_list{node});
            TraceLog(LOG_INFO, "%*s"NODE_FORMAT, depth*4, " ", FORMAT_NODE(*child));
            nodes.push_back(child);
            node->children.push_back(child);
            if (op.def_count() == 0) {
                TraceLog(LOG_INFO, "Found");
                return;
            }

            board->set_cell(op.atk, atk_fig);
            for (size_t i = 0; op.defs[i].is_valid(); i++) {
                board->set_cell(op.defs[i], OPPOSITE_FIG(atk_fig));
            }

            dependency_stage(child, child->op.atk, level, depth+1, max_depth);

            for (size_t i = 0; op.defs[i].is_valid(); i++) {
                board->set_cell(op.defs[i], Figure::None);
            }
            assert(op.atk.is_valid());
            board->set_cell(op.atk, Figure::None);
        }
    } else {
        for (DB_NodePtr child : node->children) {
            dependency_stage(child, last_move, level, depth+1, max_depth);
        }
    }
}

bool DB_Searcher::is_operation_posible(Operation op) {
    return board->get_cell(op.atk)     == Figure::None &&
           board->get_cell(op.defs[0]) == Figure::None &&
           board->get_cell(op.defs[1]) == Figure::None &&
           board->get_cell(op.defs[2]) == Figure::None;
}

bool DB_Searcher::replay_node(DB_NodePtr node, Figure atk_fig) {
    static std::vector<DB_NodePtr> stack;
    assert(node != nullptr);
    // NOTE: can't never reach null node as root->op is INVALID_OP
    if (node == root) return true;
    assert(node->op != INVALID_OP);
    size_t len = stack.size();
    bool failed = false;
    if (is_operation_posible(node->op)) {
        board->set_cell(node->op.atk, atk_fig);
        board->set_cell(node->op.defs[0], OPPOSITE_FIG(atk_fig));
        board->set_cell(node->op.defs[1], OPPOSITE_FIG(atk_fig));
        board->set_cell(node->op.defs[2], OPPOSITE_FIG(atk_fig));
        stack.push_back(node);
        for (DB_NodePtr parent : node->parents) {
            if (!replay_node(parent, atk_fig)) {
                failed = true;
                break;
            }
        }
        if (failed) { // NOTE: roll back in case of conflict
            for (size_t i = len; i < stack.size(); i++) {
                board->set_cell(stack[i]->op.atk, Figure::None);
                board->set_cell(stack[i]->op.defs[0], Figure::None);
                board->set_cell(stack[i]->op.defs[1], Figure::None);
                board->set_cell(stack[i]->op.defs[2], Figure::None);
            }
        }
    }
    stack.resize(len);
    return !failed;
}

void DB_Searcher::remove_node(DB_NodePtr node) {
    assert(node != nullptr);
    // NOTE: can't never reach null node as root->op is INVALID_OP
    if (node == root) return;
    board->set_cell(node->op.atk, Figure::None);
    board->set_cell(node->op.defs[0], Figure::None);
    board->set_cell(node->op.defs[1], Figure::None);
    board->set_cell(node->op.defs[2], Figure::None);
    for (DB_NodePtr parent : node->parents) remove_node(parent);
}

bool DB_Searcher::is_combinable(Operation a, Operation b) {
    Coord distance = a.atk - b.atk;
    return a.dir != b.dir && (distance.row == 0 || distance.col == 0 || std::abs(distance.row) == std::abs(distance.col));
}

void DB_Searcher::combination_stage(const size_t level, Figure atk_fig) {
    assert(nodes.size() > 1);
    //NOTE: 2 nodes combination
    for (size_t i = 1; i >= 1; i--) {
        DB_NodePtr node = nodes[i];
        // TODO: recheck this part
        if (node->type == DB_Node::Type::Dependency && node->level + 1 == level) {
            bool success = replay_node(node, atk_fig);
            assert(success && "Can't fail here as this is the first branch evaluated");
            for (size_t j = 0; j < i; j++) {
                DB_NodePtr that = nodes[j];

                if (that->type == DB_Node::Type::Dependency &&
                    is_combinable(node->op, that->op) &&
                    replay_node(that, atk_fig)) {
                    tree_sized_growed = true;
                    TraceLog(LOG_INFO, "Combine");
                    TraceLog(LOG_INFO, "        "NODE_FORMAT, FORMAT_NODE(*node));
                    TraceLog(LOG_INFO, "        "NODE_FORMAT, FORMAT_NODE(*that));

                    std::vector<Operation> ops = detector.find_operations(board, that->op.atk);
                    for (const Operation& op : ops) {
                        root = std::make_shared<DB_Node>(DB_Node::Type::Combination, level, op, std::initializer_list{node, that});
                    }

                    remove_node(that);
                }
            }
            remove_node(node);
        }
    }
}
