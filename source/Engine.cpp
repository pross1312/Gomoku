#include "Engine.h"
#include "Game.h"
#include "ThreatDetector.h"
#include <algorithm>
#include <cmath>
#include <chrono>
using namespace std;
using namespace std::chrono;

inline static high_resolution_clock _clock;
inline static time_point _start = _clock.now();

#define START_TIMER _start = _clock.now();
#define PRINT_ELAPSE(msg) do { \
    duration<double, ratio<1, 1>> diff = _clock.now() - _start; \
    TraceLog(LOG_INFO, "["#msg"] Time: %f", diff.count()); \
} while(false);

constexpr uint32_t RANDOM_THRESHOLD = 100;

Coord Engine::next_move(BitBoard* board, Figure atk_fig) {
    if (board->moves.size() == 0) {
        return Coord(SIZE/2, SIZE/2);
    }
    this->board = board;
    this->atk_fig = atk_fig;
    // this->cache.clear();
    this->cache_hit = 0;

    Coord result = search();
    // TraceLog(LOG_INFO, "Cache size: %zu", cache.size());
    // TraceLog(LOG_INFO, "Cache hit: %zu", cache_hit);
    return result;
}

Coord Engine::search() {
START_TIMER
    SearchResult atk_res = db_searcher.search(board, atk_fig, 5);
PRINT_ELAPSE("White search time")
START_TIMER
    SearchResult def_res = db_searcher.search(board, OPPOSITE_FIG(atk_fig), 2);
PRINT_ELAPSE("Black search time")
    TraceLog(LOG_INFO, "------------------------------");
    if (!IS_INVALID_RES(atk_res)) {
        LOG_RESULT("White", atk_res);
    } else {
        TraceLog(LOG_INFO, "White: no threat");
    }
    if (!IS_INVALID_RES(def_res)) {
        LOG_RESULT("Black", def_res);
    } else {
        TraceLog(LOG_INFO, "Black: no threat");
    }
    if (!IS_INVALID_RES(def_res) && def_res.threat == Threat::StraightFive) {
        if (!IS_INVALID_RES(atk_res) && atk_res.threat == Threat::StraightFive) {
            if (atk_res.node->op.type == Threat::StraightFive) {
                return atk_res.node->op.atk;
            } else if (def_res.node->op.type == Threat::StraightFive) {
                return def_res.node->op.atk;
            } else if (atk_res.node->op.type == Threat::StraightFour) {
                return atk_res.node->op.atk;
            }

            size_t atk_i_threat = count_immediate_threat(this->board, atk_res.node->op.atk, this->atk_fig);
            if (atk_i_threat >= 2) return atk_res.node->op.atk;
            else if (atk_i_threat == 1) {
                this->board->set_cell(atk_res.node->op.atk, this->atk_fig);
                auto def_moves = OperationDetector::find_defs(this->board, atk_res.node->op.atk, Threat::BrokenFour);
                this->board->set_cell(atk_res.node->op.atk, Figure::None);

                ThreatType best_def_move_threat = Threat::None;
                for (auto pos : def_moves) {
                    ThreatType threat = best_threat(this->board, pos, OPPOSITE_FIG(this->atk_fig));
                    if (threat >= Threat::BrokenFour) {
                        return def_res.node->op.atk;
                    }
                    best_def_move_threat = std::max(best_def_move_threat, threat);
                }
                if (best_def_move_threat == Threat::StraightThree &&
                    count_threat(this->board, atk_res.node->op.atk, this->atk_fig) < 2) {
                    return def_res.node->op.atk;
                }
                return atk_res.node->op.atk;
            } else {
                size_t def_i_threat = count_immediate_threat(this->board, def_res.node->op.atk, OPPOSITE_FIG(this->atk_fig));
                if (def_i_threat == 0) {
                    this->board->set_cell(atk_res.node->op.atk, this->atk_fig);
                    auto def_moves = OperationDetector::find_defs(this->board, atk_res.node->op.atk, Threat::BrokenFour);
                    this->board->set_cell(atk_res.node->op.atk, Figure::None);

                    for (auto pos : def_moves) {
                        ThreatType threat = best_threat(this->board, pos, OPPOSITE_FIG(this->atk_fig));
                        if (threat >= Threat::BrokenFour) {
                            return def_res.node->op.atk;
                        }
                    }
                    return atk_res.node->op.atk;
                }
            }
        }
        return def_res.node->op.atk;
    } else if (!IS_INVALID_RES(atk_res) && atk_res.threat >= Threat::StraightFour) {
        return atk_res.node->op.atk;
    }
    TraceLog(LOG_INFO, "Shallow search");
    std::vector<Coord> move_list = get_move_list();
    assert(move_list.size() > 0);
    std::sort(move_list.begin(), move_list.end(), [this](Coord x, Coord y) {
        return Engine::move_value(this->board, x, this->atk_fig) > Engine::move_value(this->board, y, this->atk_fig);
    });
    uint32_t best_move_value = Engine::move_value(this->board, move_list[0], this->atk_fig); 
    if (best_move_value > RANDOM_THRESHOLD) {
        for (size_t i = 0; i < move_list.size(); i++) {
            if (best_move_value - Engine::move_value(this->board, move_list[i], this->atk_fig) > RANDOM_THRESHOLD) {
                TraceLog(LOG_INFO, "Random move between: [0, %zu]", i-1);
                return move_list[std::rand()%i];
            }
        }
    }
    return move_list[0];
}

ThreatType Engine::best_threat(BitBoard* board, Coord pos, Figure fig) {
    Figure old = board->get_cell(pos);
    board->set_cell(pos, fig);
    Line4 lines = board->get_lines_radius(pos);
    board->set_cell(pos, old);
    return (ThreatType)std::max(std::max(ThreatDetector::check(lines[HORIZONTAL]),
                ThreatDetector::check(lines[VERTICAL])),
             std::max(ThreatDetector::check(lines[DIAGONAL]),
                ThreatDetector::check(lines[SUBDIAGONAL])));
}

size_t Engine::count_threat(BitBoard* board, Coord pos, Figure fig) {
    Figure old = board->get_cell(pos);
    board->set_cell(pos, fig);
    Line4 lines = board->get_lines_radius(pos);
    board->set_cell(pos, old);
    return  (ThreatDetector::check(lines[HORIZONTAL])  >= Threat::StraightThree) +
            (ThreatDetector::check(lines[VERTICAL])    >= Threat::StraightThree) +
            (ThreatDetector::check(lines[DIAGONAL])    >= Threat::StraightThree) +
            (ThreatDetector::check(lines[SUBDIAGONAL]) >= Threat::StraightThree);
}

size_t Engine::count_immediate_threat(BitBoard* board, Coord pos, Figure fig) {
    Figure old = board->get_cell(pos);
    board->set_cell(pos, fig);
    Line4 lines = board->get_lines_radius(pos);
    board->set_cell(pos, old);
    return  (ThreatDetector::check(lines[HORIZONTAL])  >= Threat::BrokenFour) +
            (ThreatDetector::check(lines[VERTICAL])    >= Threat::BrokenFour) +
            (ThreatDetector::check(lines[DIAGONAL])    >= Threat::BrokenFour) +
            (ThreatDetector::check(lines[SUBDIAGONAL]) >= Threat::BrokenFour);
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
