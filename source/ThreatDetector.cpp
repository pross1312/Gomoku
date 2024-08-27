#include "ThreatDetector.h"
#include <cassert>

const ThreatDetector::Cache cache;

ThreatDetector::Cache::Cache() {
    for (uint32_t line = 0; line < CACHE_SIZE; line++) {
        table[line] = ThreatDetector::detect(line);
    }
}

ThreatType ThreatDetector::check(uint32_t line) {
    assert(line < CACHE_SIZE && "Out of bound");
    return cache[line];
}

ThreatType ThreatDetector::detect(uint32_t line) {
    std::array<Cell, THREAT_RANGE> cells;
    for (size_t i = 0; i < THREAT_RANGE; i++) {
        cells[i] = Cell((line >> 2*(THREAT_RANGE-1-i)) & 0b11);
    }
    Cell figure = cells[cells.size()/2];
    if (figure == Cell::Out || figure == Cell::None) return ThreatType::ThreatNone;
    if (is_straight_five(cells, figure)) return ThreatType::StraightFive;
    else if (is_straight_four(cells, figure)) return ThreatType::StraightFour;
    else if (is_straight_three(cells, figure)) return ThreatType::StraightThree;
    return ThreatType::ThreatNone;
}

bool ThreatDetector::is_straight_three(std::array<Cell, THREAT_RANGE> &line, Cell figure) {
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == Cell::None) {
            line[i] = figure;
            if (is_straight_four(line, figure)) {
                line[i] = Cell::None;
                return true;
            }
            line[i] = Cell::None;
        }
    }
    return false;
}

bool ThreatDetector::is_straight_four(std::array<Cell, THREAT_RANGE> &line, Cell figure) {
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == Cell::None) {
            line[i] = figure;
            if (is_straight_five(line, figure)) {
                line[i] = Cell::None;
                return true;
            }
            line[i] = Cell::None;
        }
    }
    return false;
}

bool ThreatDetector::is_straight_five(std::array<Cell, THREAT_RANGE> &line, Cell figure) {
    for (size_t i = 0; i < THREAT_RANGE - 5; i++) {
        if (line[i]   == figure &&
            line[i+1] == figure &&
            line[i+2] == figure &&
            line[i+3] == figure &&
            line[i+4] == figure) return true;
    }
    return false;
}
