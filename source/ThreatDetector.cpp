#include "ThreatDetector.h"
#include <raylib.h>
#include <cassert>

const ThreatDetector::Cache cache;

ThreatDetector::Cache::Cache() {
    FILE *file = fopen(".cache", "rb");
    if (file != NULL) {
        const size_t count = fread(table, sizeof(*table), CACHE_SIZE, file);
        TraceLog(LOG_INFO, "Read %zu bytes from .cache", count);
        fclose(file);
    } else {
        for (uint32_t line = 0; line < CACHE_SIZE; line++) {
            table[line] = ThreatDetector::detect(line);
        }
        file = fopen(".cache", "wb");
        fwrite(table, sizeof(*table), CACHE_SIZE, file);
        fclose(file);
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
    if (figure == Cell::Out || figure == Cell::None) return Threat::None;
    if (is_straight_five(cells, figure)) return Threat::StraightFive;
    else if (is_straight_four(cells, figure)) return Threat::StraightFour;
    else if (is_broken_four(cells, figure)) return Threat::BrokenFour;
    else if (is_straight_three(cells, figure)) return Threat::StraightThree;
    return Threat::None;
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

bool ThreatDetector::is_broken_four(std::array<Cell, THREAT_RANGE> &line, Cell figure) {
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

bool ThreatDetector::is_straight_four(std::array<Cell, THREAT_RANGE> &line, Cell figure) {
    for (size_t i = 0; i < line.size()-5; i++) {
        if (line[i+0] == Cell::None &&
            line[i+1] == figure &&
            line[i+2] == figure &&
            line[i+3] == figure &&
            line[i+4] == figure &&
            line[i+5] == Cell::None) return true;
    }
    return false;
}

bool ThreatDetector::is_straight_five(std::array<Cell, THREAT_RANGE> &line, Cell figure) {
    for (size_t i = 0; i < line.size() - 4; i++) {
        if (line[i+0] == figure &&
            line[i+1] == figure &&
            line[i+2] == figure &&
            line[i+3] == figure &&
            line[i+4] == figure) return true;
    }
    return false;
}
