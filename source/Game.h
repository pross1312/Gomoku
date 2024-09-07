#pragma once
#include "Ui.h"
#include "Engine.h"
#include "DB_Searcher.h"
#include <vector>

struct Game {
    enum Mode { Pvp, Bot, Custom } mode;
    enum Turn { White, Black, } turn;
    Ui ui;
    BitBoard board;
    Engine engine;
    DB_Searcher searcher;
    OperationDetector detector;
    std::vector<Coord> white_moves, black_moves;

    Game(Mode mode);

    size_t moves_count() const { return white_moves.size() + black_moves.size(); }
    Coord last_move() const { return white_moves.size() == black_moves.size() ? black_moves.back() : white_moves.back(); }
    void run();
    std::optional<Coord> get_next_move();
    void restart();
    void switch_turn();
    void add_move(Coord pos);
    void pop_last_move();
    bool check_win(Coord pos) const;
};
