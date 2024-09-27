#include "Game.h"
#include "ThreatDetector.h"
#include <cassert>

Game::Game(Mode mode):
    mode(mode), turn(Turn::White),
    ui(Rectangle{ .x = 0, .y = 0, .width = (float)GetScreenWidth(), .height = (float)GetScreenHeight() }),
    is_game_end{false} {
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
            if (IsKeyPressed(KEY_BACKSPACE)) {
                pop_last_move();
            }
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
            auto opt_fig = ui.get_cell_at_pos(GetMousePosition());
            if (opt_fig.has_value()) {
                print_value(opt_fig.value());
                TraceLog(LOG_INFO, "Move value %zu", Engine::move_value(&this->board, opt_fig.value(), this->board.get_cell(opt_fig.value())));
                if (Engine::count_immediate_threat(&this->board, opt_fig.value(), Figure::White) > 0) {
                    for (auto pos : OperationDetector::find_defs(&this->board, opt_fig.value(), Threat::BrokenFour)) {
                        LOG_COORD(pos);
                    }
                }
            }
        }
        if (IsKeyPressed(KEY_SPACE)) {
            TraceLog(LOG_INFO, "------------------------------");
            if (board.moves.size() > 0) {
                SearchResult res = searcher.search(&board, Figure::White);
                if (IS_INVALID_RES(res)) {
                    LOG_RESULT("Black", res);
                } else {
                    TraceLog(LOG_INFO, "White: no threat");
                }
            }
            if (board.moves.size() > 0) {
                SearchResult res = searcher.search(&board, Figure::Black);
                if (IS_INVALID_RES(res)) {
                    LOG_RESULT("Black", res);
                } else {
                    TraceLog(LOG_INFO, "Black: no threat");
                }
            }
        }

        if (IsKeyPressed(KEY_S)) {
            save_state(".last_state");
        } else if (IsKeyPressed(KEY_L)) {
            load_state(".last_state");
        }

        if (!is_game_end) {
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
                } else if (this->board.get_cell(opt_move.value()) == Figure::None) {
                    add_move(opt_move.value());
                    if (mode == Game::Mode::Bot || mode == Game::Mode::Pvp) {
                        if (check_win(opt_move.value())) {
                            TraceLog(LOG_INFO, "Player %s win!!!", turn == Turn::White ? "White" : "Black");
                            is_game_end = true;
                        } else if (board.moves.size() == SIZE*SIZE) {
                            TraceLog(LOG_INFO, "Game draw!");
                            is_game_end = true;
                        } else {
                            switch_turn();
                        }
                    }
                }
            }
        } else if (IsKeyPressed(KEY_R)) {
            is_game_end = false;
            restart();
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
                result = engine.next_move(&this->board, Figure::White);
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
    turn = Turn::White;
}

void Game::switch_turn() {
    turn = turn == Turn::White ? Turn::Black : Turn::White;
}

void Game::add_move(Coord pos) {
    if (turn == Turn::White) {
        board.add_move(pos, Figure::White);
    } else {
        board.add_move(pos, Figure::Black);
    }
}

void Game::pop_last_move() {
    if (board.moves.size() == 0) return;
    board.pop_move();
}

bool Game::check_win(Coord pos) const {
    Line4 lines = board.get_lines_radius(pos);
    return ThreatDetector::check(lines[HORIZONTAL])      == Threat::StraightFive ||
           ThreatDetector::check(lines[VERTICAL])      == Threat::StraightFive ||
           ThreatDetector::check(lines[DIAGONAL]) == Threat::StraightFive ||
           ThreatDetector::check(lines[SUBDIAGONAL])  == Threat::StraightFive;
}

bool Game::save_state(const char* file_path) {
    FILE* f = fopen(file_path, "wb");
    if (f == NULL) {
        TraceLog(LOG_INFO, "Can't save board state to `%s`", file_path);
        return false;
    }
    size_t move_count = this->board.moves.size();
    fwrite(&move_count, sizeof(move_count), 1, f);
    fwrite(this->board.moves.data(), sizeof(*this->board.moves.data()), this->board.moves.size(), f);
    fwrite(&turn, sizeof(turn), 1, f);
    fwrite(&is_game_end, sizeof(is_game_end), 1, f);
    fclose(f);
    return true;
}

bool Game::load_state(const char* file_path) {
    this->board.clear();

    FILE* f = fopen(file_path, "rb");
    if (f == NULL) {
        TraceLog(LOG_INFO, "Can't save board state to `%s`", file_path);
        return false;
    }
    size_t move_count = 0;
    fread(&move_count, sizeof(move_count), 1, f);
    this->board.moves.resize(move_count);
    fread(this->board.moves.data(), sizeof(*this->board.moves.data()), move_count, f);
    fread(&turn, sizeof(turn), 1, f);
    fread(&is_game_end, sizeof(is_game_end), 1, f);
    fclose(f);

    for (auto& move : this->board.moves) {
        this->board.set_cell(move.pos, move.fig);
    }
    return true;
}
