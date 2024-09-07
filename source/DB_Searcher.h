#pragma once
#include "BitBoard.h"
#include "OperationDetector.h"
#include <cassert>
#include <memory>

struct DB_Node;
typedef std::shared_ptr<DB_Node> DB_NodePtr;
#define make_db_node(...) std::make_shared<DB_Node>(__VA_ARGS__)

#define NODE_FORMAT "[%s-%zu]-"OP_FORMAT
#define FORMAT_NODE(node)\
        (node).type==DB_Node::Type::Dependency?"Dependency":"Combination",\
        (node).level,\
        FORMAT_OP((node).op)
#define LOG_NODE(node) TraceLog(LOG_INFO, NODE_FORMAT, FORMAT_NODE(node))
struct DB_Node {
    enum Type : uint8_t {
        Dependency,
        Combination,
    } type;
    Operation op;
    size_t level;
    std::vector<DB_NodePtr> parents;
    std::vector<DB_NodePtr> children;

    DB_Node(Type type, size_t level, Operation op, const std::initializer_list<DB_NodePtr>&& parents):
        type{type}, op{op}, level{level}, parents{parents}, children{} {}
};

struct DB_Searcher {
    bool tree_sized_growed;
    OperationDetector detector;
    BitBoard* board;
    DB_NodePtr root;
    std::vector<DB_NodePtr> nodes;
    const std::vector<Coord>* moves;

    void put_op(Operation op, Figure fig) {
        assert(op.atk.is_valid());
        board->set_cell(op.atk, fig);
        if (op.defs[0].is_valid()) board->set_cell(op.defs[0], fig);
        if (op.defs[1].is_valid()) board->set_cell(op.defs[1], fig);
        if (op.defs[2].is_valid()) board->set_cell(op.defs[2], fig);
    }

    void search(BitBoard* board, const std::vector<Coord>* moves);

    void dependency_stage(DB_NodePtr node, Coord last_move, const size_t level, size_t depth, size_t max_depth);
    bool is_operation_posible(Operation op);
    bool replay_node(DB_NodePtr node, Figure atk_fig);
    void remove_node(DB_NodePtr node);
    bool is_combinable(Operation a, Operation b);
    void combination_stage(const size_t level, Figure atk_fig);
};
