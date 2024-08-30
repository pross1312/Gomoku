#pragma once
#include "BitBoard.h"
#include <vector>

#define REGION_SIZE 5
struct Game;
typedef std::vector<Coord> MoveList;
struct SearchRegion {
    size_t top, left, width, height;
    bool contain(size_t row, size_t col) { return row >= top && row < top + height && col >= left && col < left + width; }
};

struct Engine {
    Game* game;

    Coord next_move(Game* game);
    std::pair<Coord, uint32_t> search(size_t depth, size_t max_depth);
    uint32_t move_value(Coord pos, Cell atk_fig);
    MoveList get_move_list();
};
