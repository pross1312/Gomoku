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
    bool is_game_end;

    Game(Mode mode);

    void run();
    std::optional<Coord> get_next_move();
    void restart();
    void switch_turn();
    void add_move(Coord pos);
    void pop_last_move();
    bool check_win(Coord pos) const;

    bool save_state(const char* file_path);
    bool load_state(const char* file_path);
};
