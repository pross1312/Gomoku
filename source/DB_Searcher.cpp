#include "DB_Searcher.h"
#include "Engine.h"
#include <cmath>

constexpr size_t MAX_LEVEL = 100;
SearchResult DB_Searcher::search(BitBoard* b, Figure atk_fig) {
    this->tree_size_growed = true;
    this->board = b;
    this->atk_fig = atk_fig;
    this->root = make_db_node(
        DB_Node::Type::Combination,
        INVALID_OP,
        0,
        0
    );
    this->nodes.clear();

    for (size_t level = 1; this->tree_size_growed && level < MAX_LEVEL; level += 1) {
        this->tree_size_growed = false;
        dependency_stage(this->root, level);
        combination_stage(level);
    }
    return this->root->best_res;
}

void DB_Searcher::dependency_stage(DB_NodePtr node, size_t level) {
    if (node != this->root) {
        if (node->op.type == Threat::StraightFive) return;
        put_op(node->op);
        assert(play_threat_sequence(node->parent.second));
    }

    if (node->level + 1 == level) {

        std::vector<Operation> ops;
        if (node == this->root) {
            OperationDetector::find_operations(this->board, this->atk_fig).swap(ops);
        } else {
            assert(node->op != INVALID_OP);
            OperationDetector::find_operations(this->board, node->op.atk).swap(ops);
        }

        this->tree_size_growed |= ops.size() > 0;

        for (const Operation& op : ops) {
            DB_NodePtr child = make_db_node(
                DB_Node::Type::Dependency,
                op,
                node,
                node->level,
                node->depth+1
            );
            node->children.push_back(child);
            this->nodes.push_back(child);
            if (op.type == Threat::StraightFive) continue;
            dependency_stage(child, level);
            update_best_result(node, child);
        }

    } else {
        for (auto const& child : node->children) {
            dependency_stage(child, level);
            update_best_result(node, child);
        }
    }

    if (node != this->root) {
        remove_op(node->op);
        remove_threat_sequence(node->parent.second);
    }
}

void DB_Searcher::combination_stage(size_t level) {
    for (int i = this->nodes.size()-1; i > 0; i--) {
        DB_NodePtr comb_node = this->nodes[i];
        if (comb_node->level + 1 == level && comb_node->op.type < Threat::StraightFour && play_threat_sequence(comb_node)) {

            for (int j = i-1; j > 0; j--) {
                DB_NodePtr node_2 = this->nodes[j];
                if (is_nodes_combinable(comb_node, node_2) &&
                    play_threat_sequence(node_2)) {

                    Coord distance = comb_node->op.atk - node_2->op.atk;
                    Direction dir = distance.row == 0             ? HORIZONTAL :
                                    distance.col == 0             ? VERTICAL   :
                                    distance.col*distance.row < 0 ? DIAGONAL   : SUBDIAGONAL;

                    auto ops = OperationDetector::find_operations(this->board, comb_node->op.atk, dir);
                    this->tree_size_growed |= ops.size() > 0;

                    for (const auto& op : ops) {
                        DB_NodePtr child = make_db_node(
                            DB_Node::Type::Combination,
                            op,
                            comb_node,
                            node_2,
                            level,
                            comb_node->depth + node_2->depth + 1
                        );
                        update_best_result(comb_node, child);

                        this->nodes.push_back(child);
                        comb_node->children.push_back(child);
                    }

                    remove_threat_sequence(node_2);

                } else if (comb_node->op.atk == node_2->op.atk && node_2->parent.first == this->root) {
                    auto ops = OperationDetector::find_operations(this->board, node_2->op.atk, node_2->op.dir);
                    this->tree_size_growed |= ops.size() > 0;

                    for (const auto& op : ops) {
                        DB_NodePtr child = make_db_node(
                            DB_Node::Type::Combination,
                            op,
                            comb_node,
                            level,
                            comb_node->depth + 1
                        );
                        update_best_result(comb_node, child);
                        this->nodes.push_back(child);
                        comb_node->children.push_back(child);
                    }
                }
            }

            remove_threat_sequence(comb_node);
        }
    }
}

bool DB_Searcher::update_best_result(DB_NodePtr node, DB_NodePtr child) {
    SearchResult child_res = IS_INVALID_RES(child->best_res) ?
        SearchResult{.node = nullptr, .threat = child->op.type, .depth = child->depth} :
        child->best_res;
    if (IS_INVALID_RES(node->best_res) || child_res.threat > node->best_res.threat ||
       (child_res.threat == node->best_res.threat &&
            child_res.threat == Threat::StraightFive && child_res.depth < node->best_res.depth) ||
       (child_res.threat == node->best_res.threat &&
            Engine::move_value(this->board, child->op.atk, this->atk_fig) >
            Engine::move_value(this->board, node->best_res.node->op.atk, this->atk_fig)))
    {
        node->best_res.depth = child_res.depth;
        node->best_res.threat = child_res.threat;
        node->best_res.node = child;
        return true;
    }
    return false;
}

bool DB_Searcher::is_nodes_combinable(const DB_NodePtr& node_1, const DB_NodePtr& node_2) {
    // NOTE: handle at parent of 1 of 2 nodes
    if (node_1->op.atk == node_2->op.atk) return false;

    Coord distance = node_1->op.atk - node_2->op.atk;
    if (distance.row == 0) {
        return (node_1->op.dir != HORIZONTAL || node_2->op.dir != HORIZONTAL) &&abs(distance.col) <= RADIUS;
    } else if (distance.col == 0) {
        return (node_1->op.dir != VERTICAL || node_2->op.dir != VERTICAL) && abs(distance.row) <= RADIUS;
    } else if (abs(distance.row) == abs(distance.col)) {
        return abs(distance.row) <= RADIUS;
    }
    return false;
}

bool DB_Searcher::play_threat_sequence(const DB_NodePtr& node) {
    if (node == this->root || node == nullptr) return true;
    if (is_op_posible(node->op)) {
        if (play_threat_sequence(node->parent.first)) {
            if (play_threat_sequence(node->parent.second)) {
                put_op(node->op);
                return true;
            } else {
                remove_threat_sequence(node->parent.first);
            }
        }
    }
    return false;
}

void DB_Searcher::remove_threat_sequence(const DB_NodePtr& node) {
    if (node == this->root || node == nullptr) return;
    assert(!is_op_posible(node->op));
    remove_op(node->op);
    remove_threat_sequence(node->parent.first);
    remove_threat_sequence(node->parent.second);
}

bool DB_Searcher::is_op_posible(const Operation& op) {
    assert(op.atk.is_valid());
    return board->get_cell(op.atk) == Figure::None &&
           (!op.defs[0].is_valid() || board->get_cell(op.defs[0]) == Figure::None) &&
           (!op.defs[1].is_valid() || board->get_cell(op.defs[1]) == Figure::None) &&
           (!op.defs[2].is_valid() || board->get_cell(op.defs[2]) == Figure::None);
}

void DB_Searcher::put_op(const Operation& op) {
    assert(is_op_posible(op));

    board->set_cell(op.atk, this->atk_fig);
    if (op.defs[0].is_valid()) board->set_cell(op.defs[0], OPPOSITE_FIG(this->atk_fig));
    if (op.defs[1].is_valid()) board->set_cell(op.defs[1], OPPOSITE_FIG(this->atk_fig));
    if (op.defs[2].is_valid()) board->set_cell(op.defs[2], OPPOSITE_FIG(this->atk_fig));
}

void DB_Searcher::remove_op(const Operation& op) {
    assert(!is_op_posible(op));

    board->set_cell(op.atk, Figure::None);
    if (op.defs[0].is_valid()) board->set_cell(op.defs[0], Figure::None);
    if (op.defs[1].is_valid()) board->set_cell(op.defs[1], Figure::None);
    if (op.defs[2].is_valid()) board->set_cell(op.defs[2], Figure::None);
}

void DB_Searcher::log_tree(const DB_NodePtr& node) {
    LOG_NODE(*node);
    for (const DB_NodePtr& child : node->children) {
        log_tree(child);
    }
}

void DB_Searcher::log_best_threat_sequence() {
    // DB_NodePtr node = this->root;
    // while (node != nullptr) {
    //     LOG_NODE(*node);
    //     node = node->best_child;
    // }
}
