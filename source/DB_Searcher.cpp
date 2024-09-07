#include "DB_Searcher.h"
#include <cmath>

void DB_Searcher::search(BitBoard* b, const std::vector<Coord>* m) {
    this->tree_sized_growed = true;
    this->root = make_db_node(DB_Node::Type::Combination, 0, Operation(), std::initializer_list<DB_NodePtr>{nullptr});
    this->board = b;
    this->moves = m;
    this->nodes.clear();
    assert(this->moves->size() > 0);

    size_t level = 1;
    while (tree_sized_growed) {
        tree_sized_growed = false;
        dependency_stage(root, moves->back(), level, 0, 1);
        combination_stage(level, board->get_cell(this->moves->back()));
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

        std::vector<Operation> ops;
        if (node->op == INVALID_OP) {
            detector.find_operations(board, *moves).swap(ops);
        } else {
            detector.find_operations(board, last_move).swap(ops);
        }
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
           (!op.defs[0].is_valid() || board->get_cell(op.defs[0]) == Figure::None) &&
           (!op.defs[1].is_valid() || board->get_cell(op.defs[1]) == Figure::None) &&
           (!op.defs[2].is_valid() || board->get_cell(op.defs[2]) == Figure::None);
}

bool DB_Searcher::replay_node(DB_NodePtr node, Figure atk_fig) {
    static std::vector<DB_NodePtr> stack;
    assert(node != nullptr);
    // NOTE: can't never reach null node as root->op is INVALID_OP
    if (node == root) return true;
    assert(root->op == INVALID_OP);
    assert(node->op != INVALID_OP);
    size_t len = stack.size();
    bool failed = false;
    if (is_operation_posible(node->op)) {
        put_op(node->op, atk_fig);
        stack.push_back(node);
        for (DB_NodePtr parent : node->parents) {
            if (!replay_node(parent, atk_fig)) {
                failed = true;
                break;
            }
        }
        if (failed) { // NOTE: roll back in case of conflict
            for (size_t i = len; i < stack.size(); i++) {
                put_op(stack[i]->op, Figure::None);
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
    put_op(node->op, Figure::None);
    for (DB_NodePtr parent : node->parents) remove_node(parent);
}

bool DB_Searcher::is_combinable(Operation a, Operation b) {
    Coord distance = a.atk - b.atk;
    if (std::abs(distance.row) == std::abs(distance.col) && a.dir != b.dir && std::abs(distance.row) <= 3) {
        return true;
    } else if (distance.row == 0 && (a.dir != HORIZONTAL || b.dir != HORIZONTAL) && std::abs(distance.col) <= 3) {
        return true;
    } else if (distance.col == 0 && (a.dir != VERTICAL || b.dir != VERTICAL) && std::abs(distance.col) <= 3) {
        return true;
    }
    return false;
}

void DB_Searcher::combination_stage(const size_t level, Figure atk_fig) {
    assert(nodes.size() > 1);
    //NOTE: 2 nodes combination
    TraceLog(LOG_INFO, "Combination stage with %zu", nodes.size());
    for (size_t i = nodes.size()-1; i >= 1; i--) {
        DB_NodePtr node = nodes[i];
        // TODO: recheck this part
        if (node->type == DB_Node::Type::Dependency && node->level + 1 == level) {
            bool success = replay_node(node, atk_fig);
            assert(success && "Can't fail here as this is the first branch evaluated");
            for (size_t j = 0; j < i; j++) {
                DB_NodePtr that = nodes[j];

                if (that->type == DB_Node::Type::Dependency) {
                    TraceLog(LOG_INFO, "Try to Combine");
                    TraceLog(LOG_INFO, "        "NODE_FORMAT, FORMAT_NODE(*node));
                    TraceLog(LOG_INFO, "        "NODE_FORMAT, FORMAT_NODE(*that));
                    if (!is_combinable(node->op, that->op)) {
                        TraceLog(LOG_INFO, "        Not combinable");
                        continue;
                    }
                    if (!replay_node(that, atk_fig)) {
                        TraceLog(LOG_INFO, "        Not replable");
                        continue;
                    }
                    tree_sized_growed = true;

                    std::vector<Operation> ops = detector.find_operations(board, that->op.atk);
                    TraceLog(LOG_INFO, "Combination childs");
                    for (const Operation& op : ops) {
                        // TODO: find out type of threat
                        //       then extend it based on that threat (with a switch case maybe)
                        DB_NodePtr child = make_db_node(DB_Node::Type::Combination, level, op, std::initializer_list{node, that});
                        TraceLog(LOG_INFO, "        "NODE_FORMAT, FORMAT_NODE(*child));
                        nodes.push_back(child);
                        node->children.push_back(child);
                    }

                    remove_node(that);
                }
            }
            remove_node(node);
        }
    }
}
