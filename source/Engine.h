#pragma once
#include "BitBoard.h"
#include "ArrayHasher.h"
#include "DB_Searcher.h"
#include <vector>
#include <unordered_map>


struct Engine {
    BitBoard* board;
    Figure atk_fig;
    std::unordered_map<std::array<uint32_t, SIZE>, std::pair<Coord, uint32_t>, ArrayHasher> cache;
    DB_Searcher db_searcher;
    size_t cache_hit;

    Coord next_move(BitBoard* board, Figure atk_fig);
    Coord search();
    static size_t count_immediate_threat(BitBoard* board, Coord pos, Figure fig);
    static uint32_t move_value(BitBoard* board, Coord pos, Figure atk_fig);
    std::vector<Coord> get_move_list();
};
