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
            ui.set_bound(Rectangle {
                .x = 0,
                .y = 0,
                .width = (float)GetScreenWidth(),
                .height = (float)GetScreenHeight()
            });
        }
        BeginDrawing();

        ClearBackground(GetColor(0x101010));
        ui.render_board(board);

        if (mode == Game::Mode::Custom) {
            if (IsKeyPressed(KEY_BACKSPACE) && moves_count() > 0) {
                pop_last_move();
            } else if (IsKeyPressed(KEY_SPACE)) {
                op_detector.get_operations(&board, Figure::White);
            }
        }

        auto opt_move = get_next_move();
        if (opt_move.has_value()) {
            add_move(opt_move.value());
            if (mode == Game::Mode::Bot || mode == Game::Mode::Pvp) {
                if (check_win(opt_move.value())) {
                    TraceLog(LOG_INFO, "Player %s win!!!", turn == Turn::White ? "White" : "Black");
                    restart();
                } else if (moves_count() == SIZE*SIZE) {
                    TraceLog(LOG_INFO, "Game draw!");
                    restart();
                } else {
                    switch_turn();
                }
            } else {
                switch_turn();
            }
        }

        // auto coord = ui.get_cell_at_pos(GetMousePosition());
        // if (coord.has_value() && board.get_cell(coord.value()) == Figure::None) {
        //     SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        // } else {
        //     SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        // }

        // static const auto print_value = [this](Coord move) {
        //     Line4 lines = board.get_lines_radius(move);
        //     TraceLog(LOG_INFO, "[%s, %s, %s, %s]",
        //             Threat::to_text(ThreatDetector::check(lines.h)),
        //             Threat::to_text(ThreatDetector::check(lines.v)),
        //             Threat::to_text(ThreatDetector::check(lines.main_d)),
        //             Threat::to_text(ThreatDetector::check(lines.sub_d)));
        // };

        EndDrawing();
    }
}

std::optional<Coord> Game::get_next_move() {
    std::optional<Coord> result;
    switch (mode) {
        case Game::Mode::Bot:
            if (turn == Turn::White) {
                result = engine.next_move(this);
            } else if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                result = ui.get_cell_at_pos(GetMousePosition());
            } else {
                result = std::nullopt;
            }
            break;
        case Game::Mode::Pvp: case Game::Mode::Custom:
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                result = ui.get_cell_at_pos(GetMousePosition());
            }
            break;
    }
    return result;
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
        board.set_cell(pos, Figure::White);
        white_moves.push_back(pos);
        op_detector.white_moves.push_back(pos);
    } else {
        board.set_cell(pos, Figure::Black);
        black_moves.push_back(pos);
        op_detector.black_moves.push_back(pos);
    }
}

void Game::pop_last_move() {
    if (moves_count() == 0) return;
    // White always moves first so white_moves.size() >= black_moves.size()
    if (white_moves.size() > black_moves.size()) {
        board.set_cell(white_moves.back(), Figure::None);
        op_detector.white_moves.pop_back();
        white_moves.pop_back();
    } else {
        board.set_cell(black_moves.back(), Figure::None);
        black_moves.pop_back();
        op_detector.black_moves.pop_back();
    }
}

bool Game::check_win(Coord pos) const {
    Line4 lines = board.get_lines_radius(pos);
    return ThreatDetector::check(lines.h)      == Threat::StraightFive ||
           ThreatDetector::check(lines.v)      == Threat::StraightFive ||
           ThreatDetector::check(lines.main_d) == Threat::StraightFive ||
           ThreatDetector::check(lines.sub_d)  == Threat::StraightFive;
}
