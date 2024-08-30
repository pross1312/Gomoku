#include "Game.h"
#include "ThreatDetector.h"
#include <cassert>

Game::Game(Mode mode):
    mode(mode), turn(Turn::White),
    ui(Rectangle{ .x = 0, .y = 0, .width = (float)GetScreenWidth(), .height = (float)GetScreenHeight() }) {
}

void Game::run() {
    while (!WindowShouldClose()) {
        if (IsWindowResized()) {
            ui.bound = Rectangle {
                .x = 0,
                .y = 0,
                .width = (float)GetScreenWidth(),
                .height = (float)GetScreenHeight()
            };
        }
        BeginDrawing();

        ClearBackground(GetColor(0x101010));
        ui.render_board(board);

        auto coord = ui.get_cell_at_pos(GetMousePosition());
        if (coord.has_value() && board.get_cell(coord.value()) == Cell::None) {
            SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        } else {
            SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        }

        // static const auto print_value = [this](Coord move) {
        //     Line4 lines = board.get_lines_radius(move);
        //     TraceLog(LOG_INFO, "[%s, %s, %s, %s]",
        //             Threat::to_text(ThreatDetector::check(lines.h)),
        //             Threat::to_text(ThreatDetector::check(lines.v)),
        //             Threat::to_text(ThreatDetector::check(lines.main_d)),
        //             Threat::to_text(ThreatDetector::check(lines.sub_d)));
        // };

        if (coord.has_value() && board.get_cell(coord.value()) == Cell::None && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            add_move(coord.value());

            switch_turn();
            if (check_win(coord.value())) {
                TraceLog(LOG_INFO, "Player %s win!!!", turn == Turn::White ? "White" : "Black");
                restart();
            } else {
                switch_turn();
            }
        }

        if (turn == Turn::White && mode == Mode::Bot) {
            Coord move = engine.next_move(this);
            add_move(move);

            if (check_win(move)) {
                TraceLog(LOG_INFO, "Player %s win!!!", turn == Turn::White ? "White" : "Black");
                restart();
            } else {
                switch_turn();
            }
        }

        EndDrawing();
    }
}

void Game::restart() {
    board.clear();
    white_moves.clear();
    black_moves.clear();
    turn = Turn::White;
}

void Game::switch_turn() {
    turn = turn == Turn::White ? Turn::Black : Turn::White;
}

void Game::add_move(Coord pos) {
    if (turn == Turn::White) {
        board.set_cell(pos, Cell::White);
        white_moves.push_back(pos);
    } else {
        board.set_cell(pos, Cell::Black);
        black_moves.push_back(pos);
    }
}

void Game::pop_last_move() {
    if (moves_count() == 0) return;
    // White always moves first so white_moves.size() >= black_moves.size()
    if (white_moves.size() > black_moves.size()) {
        board.set_cell(white_moves.back(), Cell::None);
        white_moves.pop_back();
    } else {
        board.set_cell(black_moves.back(), Cell::None);
        black_moves.pop_back();
    }
}

bool Game::check_win(Coord pos) const {
    Line4 lines = board.get_lines_radius(pos);
    return ThreatDetector::check(lines.h)      == Threat::StraightFive ||
           ThreatDetector::check(lines.v)      == Threat::StraightFive ||
           ThreatDetector::check(lines.main_d) == Threat::StraightFive ||
           ThreatDetector::check(lines.sub_d)  == Threat::StraightFive;
}
