#include "DB_Searcher.h"
#include <cmath>

void DB_Searcher::search(BitBoard* b, std::vector<Coord>* m) {
    this->tree_sized_growed = true;
    this->found_goal = false;
    this->root = make_db_node(DB_Node::Type::Combination, 0, Operation());
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

bool DB_Searcher::is_op_applicable(Operation op) {
    assert(op.atk.is_valid());
    return board->get_cell(op.atk) == Figure::None &&
           (!op.defs[0].is_valid() || board->get_cell(op.defs[0]) == Figure::None) &&
           (!op.defs[1].is_valid() || board->get_cell(op.defs[1]) == Figure::None) &&
           (!op.defs[2].is_valid() || board->get_cell(op.defs[2]) == Figure::None);
}

void DB_Searcher::put_op(Operation op, Figure atk_fig) {
    assert(is_op_applicable(op));
    board->set_cell(op.atk, atk_fig);
    if (op.defs[0].is_valid()) board->set_cell(op.defs[0], OPPOSITE_FIG(atk_fig));
    if (op.defs[1].is_valid()) board->set_cell(op.defs[1], OPPOSITE_FIG(atk_fig));
    if (op.defs[2].is_valid()) board->set_cell(op.defs[2], OPPOSITE_FIG(atk_fig));
}

void DB_Searcher::remove_op(Operation op) {
    assert(!is_op_applicable(op));
    board->set_cell(op.atk, Figure::None);
    if (op.defs[0].is_valid()) board->set_cell(op.defs[0], Figure::None);
    if (op.defs[1].is_valid()) board->set_cell(op.defs[1], Figure::None);
    if (op.defs[2].is_valid()) board->set_cell(op.defs[2], Figure::None);
}

void DB_Searcher::dependency_stage(DB_NodePtr node, Coord last_move,
                                   const size_t level, size_t depth, size_t max_depth) {
    TraceLog(LOG_INFO, "%*s%zu: "NODE_FORMAT, depth*4, " ", depth, FORMAT_NODE(*node));
    if (depth >= max_depth) return;

    Figure atk_fig = board->get_cell(last_move);
    assert(IS_PIECE(atk_fig));
    if (node->level + 1 == level) {

        std::vector<Operation> ops;
        if (node->op == INVALID_OP) {
            this->detector.find_operations(board, *moves).swap(ops);
        } else if (node->type == DB_Node::Type::Dependency) {
            this->detector.find_operations(board, last_move).swap(ops);
        } else {
            assert(false && "Unknown node type");
        }
        for (Operation op : ops) {
            DB_NodePtr child = make_db_node(DB_Node::Type::Dependency, node->level, op, std::initializer_list{node});
            node->children.push_back(child);
            this->nodes.push_back(child);
            if (op.type == Threat::StraightFive) {
                this->found_goal = true;
                return;
            }

            this->moves->push_back(op.atk);
            put_op(op, atk_fig);

            dependency_stage(child, op.atk, level, depth+1, max_depth);

            remove_op(op);
            this->moves->pop_back();
        }
    } else {
        if (node->op != INVALID_OP) {
            moves->push_back(node->op.atk);
            put_op(node->op, atk_fig);
            for (DB_NodePtr child : node->children) {
                dependency_stage(child, node->op.atk, level, depth+1, max_depth);
            }
            remove_op(node->op);
            moves->pop_back();
        }
    }
}

void DB_Searcher::remove_threat_sequence(DB_NodePtr node) {
    if (node == root) return;
    assert(!is_op_applicable(node->op));
    remove_op(node->op);
    for (DB_NodePtr parent : node->parents) {
        remove_threat_sequence(parent);
    }
}

bool DB_Searcher::apply_threat_sequence(DB_NodePtr node, Figure atk_fig) {
    if (node == root) return true;
    for (DB_NodePtr parent : node->parents) {
        if (!apply_threat_sequence(parent, atk_fig)) {
            return false;
        }
    }
    if (is_op_applicable(node->op)) {
        put_op(node->op, atk_fig);
        this->moves->push_back(node->op.atk);
        return true;
    } else {
        for (DB_NodePtr parent : node->parents) {
            remove_threat_sequence(parent);
        }
        return false;
    }
}

bool DB_Searcher::is_nodes_combinable(DB_NodePtr node_1, DB_NodePtr node_2) {
    // NOTE: double threat will be handle by node_1 combine with node_2 parent
    //       with special case that node_2 parent is root
    //
    // TODO: reconfirm that 2 threat with the same direction does not need to be checked
    if (node_1->op.atk != node_2->op.atk) {
        // NOTE: return true if distance in squares <= RADIUS
        Coord distance(
            abs(node_1->op.atk.row - node_2->op.atk.row),
            abs(node_1->op.atk.col - node_2->op.atk.col)
        );
        if (distance.row == 0) {                   // horizontal
            return (node_1->op.dir != HORIZONTAL || node_2->op.dir != HORIZONTAL) &&
                    distance.col <= RADIUS;
        } else if (distance.col == 0) {            // vertical
            return (node_1->op.dir != VERTICAL || node_2->op.dir != VERTICAL) &&
                    distance.row <= RADIUS;
        } else if (distance.row == distance.col) { // AntiDiagonal or Diagonal
            return distance.row <= RADIUS;
        }
    }
    return false;
}

void DB_Searcher::combination_stage(const size_t level, Figure atk_fig) {
    TraceLog(LOG_INFO, "Enter combination stage with %zu nodes", this->nodes.size());
    assert(this->nodes.size() > 1);
    // NOTE: 2 nodes combination
    for (size_t i = this->nodes.size() - 1; i >= 1; i--) {
        DB_NodePtr comb_node = this->nodes[i];
        if (comb_node->level + 1 == level && comb_node->type == DB_Node::Type::Dependency &&
            apply_threat_sequence(comb_node, atk_fig)) {

            for (size_t j = 0; j < i; j++) {
                DB_NodePtr node_2 = this->nodes[j];
                TraceLog(LOG_INFO, "Try to Combine");
                TraceLog(LOG_INFO, "        "NODE_FORMAT, FORMAT_NODE(*comb_node));
                TraceLog(LOG_INFO, "        "NODE_FORMAT, FORMAT_NODE(*node_2));

                if (is_nodes_combinable(comb_node, node_2)) {
                    if (apply_threat_sequence(node_2, atk_fig)) {
                        TraceLog(LOG_INFO, "        Combinable");

                        // TODO: find operations at node_2->atk
                        //       generate children
                        std::vector<Operation> ops = this->detector.find_operations(board, node_2->op.atk);
                        TraceLog(LOG_INFO, "        Generate %zu new threats", ops.size());

                        remove_threat_sequence(node_2);
                    } else {
                        TraceLog(LOG_INFO, "        Threat sequence conflicted");
                    }
                }
                // NOTE: special case for double threat
                //       other cases are already handled by combine comb_node and parent of node_2
                else if (comb_node->op.atk == node_2->op.atk &&
                         node_2->parents.size() == 1 && node_2->parents[0] == root) {
                    TraceLog(LOG_INFO, "        Got double threat special case.");
                        // TODO: find operations at node_2->atk
                        //       generate children
                } else {
                    TraceLog(LOG_INFO, "        Not combinable");
                }
            }

            remove_threat_sequence(comb_node);
        }
    }
}
