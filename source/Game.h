#pragma once
#include "Ui.h"
#include "Engine.h"
#include "DB_Searcher.h"
#include <vector>

enum class GameResult
{
    Win,
    Draw,
    None,
};
enum class GameMode {
    Pvp,
    Bot,
    Custom,
};

struct Game {
    GameMode mode = GameMode::Pvp;
    Ui ui;
    BitBoard board;
    Engine engine;
    DB_Searcher searcher;
    OperationDetector detector;
    GameResult game_result = GameResult::None;
    Figure current_turn = Figure::White;
    Figure bot_turn = Figure::White;

    Game(GameMode mode);

    void run();

    Coord mode_pvp_move();
    Coord mode_bot_move();
    Coord mode_custom_move();

    void restart();
    bool check_win(Coord pos) const;

    bool save_state(const char* file_path);
    bool load_state(const char* file_path);
};
