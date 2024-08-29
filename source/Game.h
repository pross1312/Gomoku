#pragma once
#include "Ui.h"
#include <vector>

struct Game {
    enum Turn {
        White,
        Black,
    } turn;
    Ui ui;
    BitBoard board;
    std::vector<Coord> white_moves, black_moves;

    Game();

    void run();
    void restart();
    void switch_turn();
    void add_move(Coord pos);
    bool check_win(Coord pos);
};
