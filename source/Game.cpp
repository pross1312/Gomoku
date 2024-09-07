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

        static const auto print_value = [this](Coord move) {
            Line4 lines = board.get_lines_radius(move);
            TraceLog(LOG_INFO, "[%s, %s, %s, %s]",
                    Threat::to_text(ThreatDetector::check(lines[HORIZONTAL])),
                    Threat::to_text(ThreatDetector::check(lines[VERTICAL])),
                    Threat::to_text(ThreatDetector::check(lines[DIAGONAL])),
                    Threat::to_text(ThreatDetector::check(lines[SUBDIAGONAL])));
        };


        if (mode == Game::Mode::Custom) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
                auto opt_fig = ui.get_cell_at_pos(GetMousePosition());
                if (opt_fig.has_value()) {
                    print_value(opt_fig.value());
                }
            }
            if (IsKeyPressed(KEY_BACKSPACE) && moves_count() > 0) {
                pop_last_move();
            } else if (IsKeyPressed(KEY_SPACE)) {
                // if (moves_count() > 0) {
                //     for (auto op : detector.find_operations(&board, white_moves.back())) {
                //         LOG_OP(op);
                //     }
                // }
                if (moves_count() > 0) searcher.search(&board, white_moves.back());

                // op_detector.find_operations(white_moves, &board);
                // TraceLog(LOG_INFO, "White");
                // for (Operation op : op_detector.ops) {
                //     LOG_OP(op);
                // }
                // TraceLog(LOG_INFO, "Black");
                // op_detector.find_operations(black_moves, &board);
                // for (Operation op : op_detector.ops) {
                //     LOG_OP(op);
                // }
            }
        }

        auto opt_move = get_next_move();
        if (opt_move.has_value()) {
            if (mode == Game::Mode::Custom) {
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    turn = Turn::White;
                    add_move(opt_move.value());
                } else {
                    turn = Turn::Black;
                    add_move(opt_move.value());
                }
            } else {
                add_move(opt_move.value());
            }
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
            }
        }

        // auto coord = ui.get_cell_at_pos(GetMousePosition());
        // if (coord[HORIZONTAL]as_value() && board.get_cell(coord.value()) == Figure::None) {
        //     SetMouseCursor(MOUSE_CURSOR_POINTING_HAND);
        // } else {
        //     SetMouseCursor(MOUSE_CURSOR_DEFAULT);
        // }

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
        case Game::Mode::Pvp:
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                result = ui.get_cell_at_pos(GetMousePosition());
            }
            break;
        case Game::Mode::Custom:
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
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
    } else {
        board.set_cell(pos, Figure::Black);
        black_moves.push_back(pos);
    }
}

void Game::pop_last_move() {
    if (moves_count() == 0) return;
    // White always moves first so white_moves.size() >= black_moves.size()
    if (white_moves.size() > black_moves.size()) {
        board.set_cell(white_moves.back(), Figure::None);
        white_moves.pop_back();
    } else {
        board.set_cell(black_moves.back(), Figure::None);
        black_moves.pop_back();
    }
}

bool Game::check_win(Coord pos) const {
    Line4 lines = board.get_lines_radius(pos);
    return ThreatDetector::check(lines[HORIZONTAL])      == Threat::StraightFive ||
           ThreatDetector::check(lines[VERTICAL])      == Threat::StraightFive ||
           ThreatDetector::check(lines[DIAGONAL]) == Threat::StraightFive ||
           ThreatDetector::check(lines[SUBDIAGONAL])  == Threat::StraightFive;
}
