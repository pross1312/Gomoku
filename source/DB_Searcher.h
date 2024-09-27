#pragma once
#include "BitBoard.h"
#include "OperationDetector.h"
#include <cassert>
#include <memory>
#include <unordered_map>

struct DB_Node;
typedef std::shared_ptr<DB_Node> DB_NodePtr;
#define make_db_node(...) std::make_shared<DB_Node>(__VA_ARGS__)

#define NODE_FORMAT "%*s[%s-%zu-%zu]"OP_FORMAT
#define FORMAT_NODE(node)\
        (node).depth*4, " ",\
        (node).type==DB_Node::Type::Dependency?"Dep":"Com",\
        (node).children.size(),\
        (node).depth,\
        FORMAT_OP((node).op)
#define LOG_NODE(node) TraceLog(LOG_INFO, NODE_FORMAT, FORMAT_NODE(node))

struct SearchResult {
    DB_NodePtr node;
    ThreatType threat;
    size_t depth;
};
#define LOG_RESULT(fig, res) TraceLog(LOG_INFO, #fig": "COORD_FORMAT" to get %s after %zu moves.",\
                FORMAT_COORD((res).node->op.atk), Threat::to_text((res).threat), (res).depth)

#define IS_INVALID_RES(res) ((res).node == nullptr)
struct DB_Node {
    enum Type : uint8_t {
        Dependency,
        Combination,
    } type;
    Operation op;
    std::pair<DB_NodePtr, DB_NodePtr> parent;
    std::vector<DB_NodePtr> children;
    size_t level;
    size_t depth;

    SearchResult best_res;

    DB_Node(Type type, Operation op, size_t level, size_t depth):
        DB_Node(type, op, nullptr, nullptr, level, depth) {}

    DB_Node(Type type, Operation op, DB_NodePtr parent_1, size_t level, size_t depth):
        DB_Node(type, op, parent_1, nullptr, level, depth) {}

    DB_Node(Type type, Operation op, DB_NodePtr parent_1, DB_NodePtr parent_2, size_t level, size_t depth):
        type{type}, op{op}, parent{parent_1, parent_2}, level{level}, depth{depth} {}
};

constexpr size_t MAX_LEVEL = 5;
struct DB_Searcher {
    static const SearchResult INVALID_RESULT;

    bool tree_size_growed;
    BitBoard* board;
    DB_NodePtr root;
    Figure atk_fig;
    std::vector<DB_NodePtr> nodes;

    void log_tree(const DB_NodePtr& node);
    void log_best_threat_sequence();

    SearchResult search(BitBoard* board, Figure atk_fig, size_t limit = MAX_LEVEL);
    void dependency_stage(DB_NodePtr node, size_t level);
    void combination_stage(size_t level);

    bool is_nodes_combinable(const DB_NodePtr& node_1, const DB_NodePtr& node_2);
    bool play_threat_sequence(const DB_NodePtr& node);
    void remove_threat_sequence(const DB_NodePtr& node);

    bool is_op_posible(const Operation& op);
    void put_op(const Operation& op);
    void remove_op(const Operation& op);

    bool update_best_result(DB_NodePtr node, DB_NodePtr child);
};
