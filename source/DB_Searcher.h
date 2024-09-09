#pragma once
#include "BitBoard.h"
#include "OperationDetector.h"
#include <cassert>
#include <memory>

struct DB_Node;
typedef std::shared_ptr<DB_Node> DB_NodePtr;
#define make_db_node(...) std::make_shared<DB_Node>(__VA_ARGS__)

#define NODE_FORMAT "[%s-%zu-%zu]"OP_FORMAT
#define FORMAT_NODE(node)\
        (node).type==DB_Node::Type::Dependency?"Dep":"Com",\
        (node).children.size(),\
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

    DB_Node(Type type, size_t level, Operation op):
        DB_Node(type, level, op, std::initializer_list<DB_NodePtr>{nullptr}) {}
    DB_Node(Type type, size_t level, Operation op, const std::initializer_list<DB_NodePtr>&& parents):
        type{type}, op{op}, level{level}, parents{parents}, children{} {}
};

struct DB_Searcher {
    bool tree_sized_growed;
    bool found_goal;        // StraightFour or StraightFive
    OperationDetector detector;
    BitBoard* board;
    DB_NodePtr root;
    std::vector<DB_NodePtr> nodes;
    std::vector<Coord>* moves;

    bool is_op_applicable(Operation op);
    void put_op(Operation op, Figure fig);
    void remove_op(Operation op);

    void search(BitBoard* board, std::vector<Coord>* moves);
    void dependency_stage(DB_NodePtr node, Coord last_move, const size_t level, size_t depth, size_t max_depth);
    void combination_stage(const size_t level, Figure atk_fig);

    //NOTE: Each node is a threat
    //      A threat sequence here is sequence of threat from root to leave
    bool apply_threat_sequence(DB_NodePtr node, Figure atk_fig);
    void remove_threat_sequence(DB_NodePtr node);

    bool is_nodes_combinable(DB_NodePtr node_1, DB_NodePtr node_2);

    bool is_operation_posible(Operation op);
    bool replay_node(DB_NodePtr node, Figure atk_fig);
    void remove_node(DB_NodePtr node);
    bool is_combinable(Operation a, Operation b);
};
