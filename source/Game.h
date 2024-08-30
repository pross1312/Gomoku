#pragma once
#include "Ui.h"
#include "Engine.h"
#include <vector>

struct Game {
    enum Mode { Pvp, Bot } mode;
    enum Turn { White, Black, } turn;
    Ui ui;
    BitBoard board;
    Engine engine;
    std::vector<Coord> white_moves, black_moves;

    Game(Mode mode);

    size_t moves_count() const { return white_moves.size() + black_moves.size(); }
    Coord last_move() const { return white_moves.size() == black_moves.size() ? black_moves.back() : white_moves.back(); }
    void run();
    void restart();
    void switch_turn();
    void add_move(Coord pos);
    void pop_last_move();
    bool check_win(Coord pos) const;
};
