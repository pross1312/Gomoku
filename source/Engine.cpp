#include "Engine.h"
#include "Game.h"
#include "ThreatDetector.h"
#include <algorithm>
#include <cmath>

Coord Engine::next_move(Game* game) {
    this->game = game;
    if (game->moves_count() == 0) {
        return Coord{SIZE/2, SIZE/2};
    }
    cache.clear();
    cache_hit = 0;
    auto result = search(0, 10).first;
    TraceLog(LOG_INFO, "Cache size: %zu", cache.size());
    TraceLog(LOG_INFO, "Cache hit: %zu", cache_hit);
    // for (auto&[k, v] : cache) {
    //     TraceLog(LOG_INFO, "[%zu, %zu] - %d", v.first.row, v.first.col, v.second);
    // }
    return result;
}

std::pair<Coord, uint32_t> Engine::search(size_t depth, size_t max_depth) {
    MoveList potential_moves = get_move_list();
    assert(potential_moves.size() > 0);

    std::pair<Coord, uint32_t> result;

    auto value = cache.find(game->board.h_lines);
    if (value != cache.end()) {
        cache_hit += 1;
        return value->second;
    }

    Cell atk_fig = game->turn == Game::Turn::White ? Cell::White : Cell::Black;
    std::sort(potential_moves.begin(), potential_moves.end(), [&](Coord a, Coord b) {
        return move_value(a, atk_fig) > move_value(b, atk_fig);
    });
    game->add_move(potential_moves[0]);
    if (depth >= max_depth) {
        game->pop_last_move();
        result = std::pair(potential_moves[0], move_value(potential_moves[0], atk_fig));
    } else if (game->check_win(potential_moves[0])) {
        game->pop_last_move();
        result = std::pair(potential_moves[0], UINT32_MAX);
    } else {
        game->switch_turn();
        result.first = potential_moves[0];
        result.second = search(depth+1, max_depth).second;
        game->switch_turn();
        game->pop_last_move();
        if (result.second != UINT32_MAX) {
            for (size_t i = 1; i < potential_moves.size() && move_value(potential_moves[i], atk_fig) > ThreatDetector::threshold(); i++) {
                game->add_move(potential_moves[0]);
                game->switch_turn();
                auto[_, temp_move_value] = search(depth+1, max_depth);
                game->switch_turn();
                game->pop_last_move();
                if (temp_move_value > result.second) {
                    result = std::pair(potential_moves[i], temp_move_value);
                }
                if (result.second == UINT32_MAX) break;
            }
        }
    }
    cache[game->board.h_lines] = result;
    return result;
}

uint32_t Engine::move_value(Coord pos, Cell atk_fig) {
    Cell old_fig = game->board.get_cell(pos);
    game->board.set_cell(pos, atk_fig);
    Line4 atk_lines = game->board.get_lines_radius(pos);
    Cell def_fig = OPPOSITE_FIG(atk_fig);
    game->board.set_cell(pos, def_fig);
    Line4 def_lines = game->board.get_lines_radius(pos);
    game->board.set_cell(pos, old_fig);
    return ThreatDetector::atk_value(ThreatDetector::check(atk_lines.h))      +
           ThreatDetector::atk_value(ThreatDetector::check(atk_lines.v))      +
           ThreatDetector::atk_value(ThreatDetector::check(atk_lines.main_d)) +
           ThreatDetector::atk_value(ThreatDetector::check(atk_lines.sub_d))  +
           ThreatDetector::def_value(ThreatDetector::check(def_lines.h))      +
           ThreatDetector::def_value(ThreatDetector::check(def_lines.v))      +
           ThreatDetector::def_value(ThreatDetector::check(def_lines.main_d)) +
           ThreatDetector::def_value(ThreatDetector::check(def_lines.sub_d));
}

MoveList Engine::get_move_list() {
    MoveList result;
    for (size_t row = 0; row < SIZE; row++) {
        for (size_t col = 0; col < SIZE; col++) {
            if (game->board.get_cell(row, col) == Cell::None) {
                result.push_back(Coord{.row = row, .col = col});
            }
        }
    }
    return result;
}
// MoveList Engine::get_move_list() {
//     MoveList result;
//     Coord center = game->white_moves.back();
//     SearchRegion white_reg;
//     white_reg.top    = center.row < REGION_SIZE/2        ? 0           : center.row - REGION_SIZE/2;
//     white_reg.left   = center.col < REGION_SIZE/2        ? 0           : center.col - REGION_SIZE/2;
//     white_reg.width  = center.col + REGION_SIZE/2 < SIZE ? REGION_SIZE : SIZE-1-white_reg.left;
//     white_reg.height = center.row + REGION_SIZE/2 < SIZE ? REGION_SIZE : SIZE-1-white_reg.top;
//     for (size_t row = 0; row < white_reg.height; row++) {
//         for (size_t col = 0; col < white_reg.width; col++) {
//             if (white_reg.top + row < SIZE && white_reg.left + col < SIZE) {
//                 Coord pos { .row = white_reg.top + row, .col = white_reg.left + col };
//                 if (game->board.get_cell(pos) == Cell::None) {
//                     result.push_back(pos);
//                 }
//             }
//         }
//     }
//     if (game->moves_count() > 1) {
//         center = game->black_moves.back();
//         SearchRegion black_reg;
//         black_reg.top    = center.row < REGION_SIZE/2        ? 0           : center.row - REGION_SIZE/2;
//         black_reg.left   = center.col < REGION_SIZE/2        ? 0           : center.col - REGION_SIZE/2;
//         black_reg.width  = center.col + REGION_SIZE/2 < SIZE ? REGION_SIZE : SIZE-1-black_reg.left;
//         black_reg.height = center.row + REGION_SIZE/2 < SIZE ? REGION_SIZE : SIZE-1-black_reg.top;
//         for (size_t row = 0; row < black_reg.height; row++) {
//             for (size_t col = 0; col < black_reg.width; col++) {
//                 if (black_reg.top + row < SIZE && black_reg.left + col < SIZE) {
//                     Coord pos { .row = black_reg.top + row, .col = black_reg.left + col };
//                     if (!white_reg.contain(pos.row, pos.col) && game->board.get_cell(pos) == Cell::None) {
//                         result.push_back(pos);
//                     }
//                 }
//             }
//         }
//     }
//     return result;
// }
