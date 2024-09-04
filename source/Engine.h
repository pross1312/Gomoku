#pragma once
#include "BitBoard.h"
#include "ArrayHasher.h"
#include <vector>
#include <unordered_map>

struct Game;
typedef std::vector<Coord> MoveList;
struct SearchRegion {
    size_t top, left, width, height;
    bool contain(size_t row, size_t col) { return row >= top && row < top + height && col >= left && col < left + width; }
};

struct Engine {
    Game* game;
    std::unordered_map<std::array<uint32_t, SIZE>, std::pair<Coord, uint32_t>, ArrayHasher> cache;
    size_t cache_hit;

    Coord next_move(Game* game);
    std::pair<Coord, uint32_t> search(size_t depth, size_t max_depth);
    uint32_t move_value(Coord pos, Figure atk_fig);
    MoveList get_move_list();
};
