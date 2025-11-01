#include "Game.h"
#include "ThreatDetector.h"
#include <cassert>

Game::Game(GameMode mode):
    mode(mode),
    ui(Rectangle{ .x = 0, .y = 0, .width = (float)GetScreenWidth(), .height = (float)GetScreenHeight() })
{
}

void Game::run() {
    static const auto print_value = [this](Coord move) {
        Line4 lines = board.get_lines_radius(move);
        TraceLog(LOG_INFO, "[%s, %s, %s, %s]",
                Threat::to_text(ThreatDetector::check(lines[HORIZONTAL])),
                Threat::to_text(ThreatDetector::check(lines[VERTICAL])),
                Threat::to_text(ThreatDetector::check(lines[DIAGONAL])),
                Threat::to_text(ThreatDetector::check(lines[SUBDIAGONAL])));
    };

    while (!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(GetColor(0x101010));

        if (IsWindowResized()) {
            ui.set_bound(Rectangle {
                .x = 0,
                .y = 0,
                .width = (float)GetScreenWidth(),
                .height = (float)GetScreenHeight()
            });
        }

        if (this->mode == GameMode::Custom && IsKeyPressed(KEY_BACKSPACE) && this->board.moves.size() > 0) {
            this->board.pop_move();
        }

        if (auto coord = ui.get_cell_at_pos(GetMousePosition()); coord != INVALID_COORD && IsMouseButtonPressed(MOUSE_BUTTON_MIDDLE)) {
            print_value(coord);
            TraceLog(LOG_INFO, COORD_FORMAT" - %llu", FORMAT_COORD(coord), Engine::move_value(&this->board, coord, this->board.get_cell(coord)));

            if (Engine::count_immediate_threat(&this->board, coord, this->current_turn) > 0) {
                for (auto pos : OperationDetector::find_defs(&this->board, coord, Threat::BrokenFour)) {
                    LOG_COORD(pos);
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

        if (IsKeyPressed(KEY_R)) {
            this->restart();
        }

        do {
            if (this->game_result != GameResult::None) break;

            auto move = INVALID_COORD;
            switch (this->mode) {
                case GameMode::Pvp:
                    move = this->mode_pvp_move();
                    break;
                case GameMode::Custom:
                    move = this->mode_custom_move();
                    break;
                case GameMode::Bot:
                    move = this->mode_bot_move();
                    break;
            }

            if (move == INVALID_COORD) break;

            this->board.add_move(move, this->current_turn);

            if (this->check_win(move)) {
                this->game_result = GameResult::Win;
                break;
            }

            this->current_turn = OPPOSITE_FIG(this->current_turn);
        } while (false);

        switch (this->game_result) {
            case GameResult::None:
                this->ui.render_board(this->board);
                break;
            case GameResult::Win:
                this->ui.render_win(this->board, this->current_turn);
                break;
            case GameResult::Draw:
                this->ui.render_draw(this->board);
                break;
        }

        EndDrawing();
    }
}

Coord Game::mode_pvp_move()
{
    auto coord = this->ui.get_cell_at_pos(GetMousePosition());
    if (coord == INVALID_COORD || !IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || this->board.get_cell(coord) != Figure::None) {
        return INVALID_COORD;
    }

    return coord;
}

Coord Game::mode_bot_move()
{
    if (this->current_turn == this->bot_turn) {
        return this->engine.next_move(&this->board, this->current_turn);
    }

    return this->mode_pvp_move();
}

Coord Game::mode_custom_move()
{
    auto coord = this->ui.get_cell_at_pos(GetMousePosition());
    if (coord == INVALID_COORD || (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && !IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) || this->board.get_cell(coord) != Figure::None) {
        return INVALID_COORD;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        this->current_turn = Figure::White;
    } else if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
        this->current_turn = Figure::Black;
    }

    return coord;
}


void Game::restart() {
    this->board.clear();
    this->current_turn = Figure::White;
    this->game_result = GameResult::None;
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
    fwrite(&this->current_turn, sizeof(this->current_turn), 1, f);
    fwrite(&this->game_result, sizeof(this->game_result), 1, f);
    fwrite(&this->bot_turn, sizeof(this->bot_turn), 1, f);
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
    fread(&this->current_turn, sizeof(this->current_turn), 1, f);
    fread(&this->game_result, sizeof(this->game_result), 1, f);
    fread(&this->bot_turn, sizeof(this->bot_turn), 1, f);
    fclose(f);

    for (auto& move : this->board.moves) {
        this->board.set_cell(move.pos, move.fig);
    }
    return true;
}
