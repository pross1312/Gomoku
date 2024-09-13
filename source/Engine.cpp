#include "Engine.h"
#include "Game.h"
#include "ThreatDetector.h"
#include <algorithm>
#include <cmath>

Coord Engine::next_move(BitBoard* board, Figure atk_fig) {
    if (board->moves.size() == 0) {
        return Coord(SIZE/2, SIZE/2);
    }
    this->board = board;
    this->atk_fig = atk_fig;
    this->cache.clear();
    this->cache_hit = 0;

    Coord result = search();
    // TraceLog(LOG_INFO, "Cache size: %zu", cache.size());
    // TraceLog(LOG_INFO, "Cache hit: %zu", cache_hit);
    return result;
}

Coord Engine::search() {
    SearchResult atk_res = db_searcher.search(board, atk_fig);
    SearchResult def_res = db_searcher.search(board, OPPOSITE_FIG(atk_fig));
    TraceLog(LOG_INFO, "------------------------------");
    if (!IS_INVALID_RES(atk_res)) {
        TraceLog(LOG_INFO, "White: "COORD_FORMAT" to get %s after %zu moves.",
                FORMAT_COORD(atk_res.coord), Threat::to_text(atk_res.threat), atk_res.depth);
    } else {
        TraceLog(LOG_INFO, "White: no threat");
    }
    if (!IS_INVALID_RES(def_res)) {
        TraceLog(LOG_INFO, "Black: "COORD_FORMAT" to get %s after %zu moves.",
                FORMAT_COORD(def_res.coord), Threat::to_text(def_res.threat), def_res.depth);
    } else {
        TraceLog(LOG_INFO, "Black: no threat");
    }
    if (!IS_INVALID_RES(def_res) && def_res.threat == Threat::StraightFive && def_res.depth <= 3) {
        if (!IS_INVALID_RES(atk_res) && atk_res.threat >= Threat::StraightFour) {
            this->board->set_cell(def_res.coord, OPPOSITE_FIG(this->atk_fig));
            Line4 lines = this->board->get_lines_radius(def_res.coord);
            this->board->set_cell(def_res.coord, Figure::None);
            if (ThreatDetector::check(lines[HORIZONTAL]) < Threat::BrokenFour && 
                ThreatDetector::check(lines[VERTICAL]) < Threat::BrokenFour &&
                ThreatDetector::check(lines[DIAGONAL]) < Threat::BrokenFour &&
                ThreatDetector::check(lines[SUBDIAGONAL]) < Threat::BrokenFour) {
                return atk_res.coord;
            }
        }
        return def_res.coord;
    } else if (!IS_INVALID_RES(atk_res) && atk_res.threat >= Threat::StraightFour) {
        return atk_res.coord;
    }
    TraceLog(LOG_INFO, "Shallow search");
    std::vector<Coord> move_list = get_move_list();
    assert(move_list.size() > 0);
    std::sort(move_list.begin(), move_list.end(), [this](Coord x, Coord y) {
        return Engine::move_value(this->board, x, this->atk_fig) > Engine::move_value(this->board, y, this->atk_fig);
    });
    return move_list[0];
}

uint32_t Engine::move_value(BitBoard* board, Coord pos, Figure atk_fig) {
    Figure old_fig = board->get_cell(pos);
    board->set_cell(pos, atk_fig);
    Line4 atk_lines = board->get_lines_radius(pos);
    Figure def_fig = OPPOSITE_FIG(atk_fig);
    board->set_cell(pos, def_fig);
    Line4 def_lines = board->get_lines_radius(pos);
    board->set_cell(pos, old_fig);
    return ThreatDetector::atk_value(ThreatDetector::check(atk_lines[HORIZONTAL]))      +
           ThreatDetector::atk_value(ThreatDetector::check(atk_lines[VERTICAL]))      +
           ThreatDetector::atk_value(ThreatDetector::check(atk_lines[DIAGONAL])) +
           ThreatDetector::atk_value(ThreatDetector::check(atk_lines[SUBDIAGONAL]))  +
           ThreatDetector::def_value(ThreatDetector::check(def_lines[HORIZONTAL]))      +
           ThreatDetector::def_value(ThreatDetector::check(def_lines[VERTICAL]))      +
           ThreatDetector::def_value(ThreatDetector::check(def_lines[DIAGONAL])) +
           ThreatDetector::def_value(ThreatDetector::check(def_lines[SUBDIAGONAL]));
}

std::vector<Coord> Engine::get_move_list() {
    std::vector<Coord> result;
    for (size_t row = 0; row < SIZE; row++) {
        for (size_t col = 0; col < SIZE; col++) {
            if (board->get_cell(row, col) == Figure::None) {
                result.push_back(Coord(row, col));
            }
        }
    }
    return result;
}
