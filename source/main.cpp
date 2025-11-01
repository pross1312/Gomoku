#include "BitBoard.h"
#include "ThreatDetector.h"
#include "Engine.h"
#include "Ui.h"
#include <algorithm>
#include <random>
#include <ctime>
#include <cstring>

enum class GameResult : uint8_t {
    Win,
    Draw,
    None,
};

enum class GameMode : uint8_t {
    Pvp,
    BotVsBot,
    Pvb,
};

struct GameState
{
    Ui ui;
    BitBoard board;
    GameResult game_result;
    Figure current_turn;

    Figure player_figure;
};

bool last_move_is_win(const GameState& game_state)
{
    auto threats = ThreatDetector::check(game_state.board.get_lines_radius(std::prev(game_state.board.moves.end())->pos));
    for (auto threat : threats) {
        if (threat == Threat::StraightFive) {
            return true;
        }
    }
    return false;
}

std::vector<Coord> get_moves_list(const BitBoard& board)
{
    std::vector<Coord> result;
    for (size_t i = 0; i < SIZE; i++) {
        for (size_t j = 0; j < SIZE; j++) {
            if (board.get_cell(i, j) == Figure::None) result.emplace_back(Coord(i, j));
        }
    }
    return result;
}

void reset(GameState& game_state)
{
    game_state.board.clear();
    game_state.current_turn = Figure::White;
    game_state.game_result = GameResult::None;
}

int64_t evaluate_board(GameState& game_state)
{
    int64_t result = 0;
    for (const auto& move : game_state.board.moves) {
        auto threats = ThreatDetector::check(game_state.board.get_lines_radius(move.pos));
        for (size_t i = 0; i < threats.size(); i++) {
            result += threats[i];
        }
    }
    return result;
}

void order_moves(std::vector<Coord>& moves, GameState& game_state)
{
    std::sort(moves.begin(), moves.end(), [&](Coord x, Coord y) {
        return Engine::move_value(&game_state.board, x, game_state.current_turn) > Engine::move_value(&game_state.board, y, game_state.current_turn);
    });
    for (size_t i = 1; i < moves.size(); i++) {
        if (Engine::move_value(&game_state.board, moves[i], game_state.current_turn) == 0) {
            moves.resize(i);
            return;
        }
    }
}

int64_t search(GameState& game_state, int depth, int64_t alpha, int64_t beta, size_t& prune_count, size_t& search_count)
{
    if (depth == 0) {
        return evaluate_board(game_state);
    }

    if (game_state.board.moves.size() > 0 && last_move_is_win(game_state)) {
        return INT64_MIN;
    }

    if (!game_state.board.has_move_left()) {
        return 0;
    }

    auto moves = get_moves_list(game_state.board);
    order_moves(moves, game_state);

    size_t i = 0;
    for (auto move : moves) {
        game_state.board.set_cell(move, game_state.current_turn);
        auto current_turn = game_state.current_turn;
        game_state.current_turn = OPPOSITE_FIG(game_state.current_turn);

        int64_t eval = -search(game_state, depth-1, -beta, -alpha, prune_count, search_count);

        game_state.current_turn = current_turn;
        game_state.board.set_cell(move, Figure::None);

        search_count++;
        if (eval >= beta) {
            prune_count += moves.size() - i;
            return beta;
        }

        alpha = std::max(alpha, eval);
        i++;
    }

    return alpha;
}

Coord get_best_move(GameState& game_state)
{
    if (game_state.board.moves.size() == 0) {
        return Coord(SIZE/2, SIZE/2);
    }
    auto moves = get_moves_list(game_state.board);
    order_moves(moves, game_state);

    Coord best_move = moves[0];

    size_t search_count = moves.size();
    size_t prune_count = 0;
    int64_t best_eval = INT64_MIN;
    for (auto move : moves) {
        game_state.board.set_cell(move, game_state.current_turn);
        auto current_turn = game_state.current_turn;
        game_state.current_turn = OPPOSITE_FIG(game_state.current_turn);

        int64_t eval = -search(game_state, 2, INT64_MIN, INT64_MAX, prune_count, search_count);

        game_state.current_turn = current_turn;
        game_state.board.set_cell(move, Figure::None);

        if (best_eval < eval) {
            best_eval = eval;
            best_move = move;
        }
    }
    TraceLog(LOG_INFO, "Searching %zu moves", search_count);
    TraceLog(LOG_INFO, "Prunning %zu moves", moves.size());
    return best_move;
}

Coord pvp(GameState& game_state)
{
    auto coord = game_state.ui.get_cell_at_pos(GetMousePosition());
    if (coord == INVALID_COORD || !IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || game_state.board.get_cell(coord) != Figure::None) {
        return INVALID_COORD;
    }

    return coord;
}

Coord pvb(GameState& game_state)
{
    if (game_state.current_turn == game_state.player_figure) {
        return pvp(game_state);
    }

    return get_best_move(game_state);
}

int main()
{
    std::srand(time(0));
    InitWindow(1300, 800, "Gomoku");
    SetWindowState(FLAG_WINDOW_RESIZABLE);
    SetWindowMinSize(100, 100);
    SetTargetFPS(30);

    GameMode mode = GameMode::Pvb;
    bool step_debug = false;
    bool allow_move = true;

    GameState game_state {
        .ui = Ui(Rectangle {
            .x = 0,
            .y = 0,
            .width = (float)GetScreenWidth(),
            .height = (float)GetScreenHeight(),
        }),
        .board = BitBoard(),
        .game_result = GameResult::None,
        .current_turn = Figure::White,
        .player_figure = Figure::Black,
    };

    while (!WindowShouldClose()) {
        BeginDrawing();
        game_state.ui.set_bound(Rectangle {
            .x = 0,
            .y = 0,
            .width = (float)GetScreenWidth(),
            .height = (float)GetScreenHeight(),
        });
        ClearBackground(BLACK);

        do {
            if (IsKeyPressed(KEY_R)) {
                reset(game_state);
            }

            if (game_state.game_result != GameResult::None) {
                break;
            }

            static const auto print_value = [&](Coord move) {
                Line4 lines = game_state.board.get_lines_radius(move);
                TraceLog(LOG_INFO, "[%s, %s, %s, %s]",
                        Threat::to_text(ThreatDetector::check(lines[HORIZONTAL])),
                        Threat::to_text(ThreatDetector::check(lines[VERTICAL])),
                        Threat::to_text(ThreatDetector::check(lines[DIAGONAL])),
                        Threat::to_text(ThreatDetector::check(lines[SUBDIAGONAL])));
            };

            if (auto coord = game_state.ui.get_cell_at_pos(GetMousePosition()); coord != INVALID_COORD && IsMouseButtonPressed(MOUSE_BUTTON_RIGHT)) {
                print_value(coord);
            }

            if (!game_state.board.has_move_left()) {
                game_state.game_result = GameResult::Draw;
                break;
            }

            auto coord = INVALID_COORD;
            switch (mode) {
                case GameMode::Pvp:
                    coord = pvp(game_state);
                    break;
                case GameMode::BotVsBot:
                    if (!step_debug) {
                        get_best_move(game_state);
                    } else if (allow_move) {
                        coord = get_best_move(game_state);
                        allow_move = false;
                    } else if (IsKeyPressed(KEY_SPACE)) {
                        allow_move = true;
                    }
                    break;
                case GameMode::Pvb:
                    coord = pvb(game_state);
                    break;
            }

            if (coord == INVALID_COORD) {
                break;
            }

            game_state.board.add_move(coord, game_state.current_turn);

            if (game_state.game_result != GameResult::None) {
                break;
            }

            game_state.current_turn = OPPOSITE_FIG(game_state.current_turn);
        } while (false);

        switch (game_state.game_result) {
            case GameResult::Draw:
                game_state.ui.render_draw(game_state.board);
                break;
            case GameResult::Win:
                game_state.ui.render_win(game_state.board, game_state.current_turn);
                break;
            case GameResult::None:
                game_state.ui.render_board(game_state.board);
                break;
        }

        EndDrawing();
    }

    CloseWindow();
}
