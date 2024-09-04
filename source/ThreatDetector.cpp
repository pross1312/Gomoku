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
    std::array<Figure, THREAT_RANGE> cells;
    for (size_t i = 0; i < THREAT_RANGE; i++) {
        cells[i] = Figure((line >> 2*(THREAT_RANGE-1-i)) & 0b11);
    }
    Figure figure = cells[cells.size()/2];
    if (figure == Figure::Out || figure == Figure::None) return Threat::None;
    if (is_straight_five(cells, figure)) return Threat::StraightFive;
    else if (is_straight_four(cells, figure)) return Threat::StraightFour;
    else if (is_broken_four(cells, figure)) return Threat::BrokenFour;
    else if (is_straight_three(cells, figure)) return Threat::StraightThree;
    else if (is_broken_three(cells, figure)) return Threat::BrokenThree;
    else if (is_two(cells, figure)) return Threat::Two;
    return Threat::None;
}

uint32_t ThreatDetector::atk_value(ThreatType threat) {
    switch (threat) {
        case Threat::None: return 0;
        case Threat::Two: return 10;
        case Threat::BrokenThree: return 60;
        case Threat::StraightThree: return 100;
        case Threat::BrokenFour: return 110;
        case Threat::StraightFour: return 1000;
        case Threat::StraightFive: return 10000;
        default: assert(false && "Unknown threat");
    }
    return 0;
}

uint32_t ThreatDetector::def_value(ThreatType threat) {
    switch (threat) {
        case Threat::None: return 0;
        case Threat::Two: return 10;
        case Threat::BrokenThree: return 60;
        case Threat::StraightThree: return 70;
        case Threat::BrokenFour: return 110;
        case Threat::StraightFour: return 500;
        case Threat::StraightFive: return 5000;
        default: assert(false && "Unknown threat");
    }
    return 0;
}

uint32_t ThreatDetector::threshold() {
    return 15;
}

bool ThreatDetector::is_two(std::array<Figure, THREAT_RANGE> &line, Figure figure) {
    for (size_t i = 2; i < line.size()-2; i++) {
        if (line[i] == Figure::None) {
            line[i] = figure;
            if (is_straight_three(line, figure) || is_broken_three(line, figure)) {
                line[i] = Figure::None;
                return true;
            }
            line[i] = Figure::None;
        }
    }
    return false;
}

bool ThreatDetector::is_broken_three(std::array<Figure, THREAT_RANGE> &line, Figure figure) {
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == Figure::None) {
            line[i] = figure;
            if (is_broken_four(line, figure)) {
                line[i] = Figure::None;
                return true;
            }
            line[i] = Figure::None;
        }
    }
    return false;
}

bool ThreatDetector::is_straight_three(std::array<Figure, THREAT_RANGE> &line, Figure figure) {
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == Figure::None) {
            line[i] = figure;
            if (is_straight_four(line, figure)) {
                line[i] = Figure::None;
                return true;
            }
            line[i] = Figure::None;
        }
    }
    return false;
}

bool ThreatDetector::is_broken_four(std::array<Figure, THREAT_RANGE> &line, Figure figure) {
    for (size_t i = 0; i < line.size(); i++) {
        if (line[i] == Figure::None) {
            line[i] = figure;
            if (is_straight_five(line, figure)) {
                line[i] = Figure::None;
                return true;
            }
            line[i] = Figure::None;
        }
    }
    return false;
}

bool ThreatDetector::is_straight_four(std::array<Figure, THREAT_RANGE> &line, Figure figure) {
    for (size_t i = 0; i < line.size()-5; i++) {
        if (line[i+0] == Figure::None &&
            line[i+1] == figure &&
            line[i+2] == figure &&
            line[i+3] == figure &&
            line[i+4] == figure &&
            line[i+5] == Figure::None) return true;
    }
    return false;
}

bool ThreatDetector::is_straight_five(std::array<Figure, THREAT_RANGE> &line, Figure figure) {
    for (size_t i = 0; i < line.size() - 4; i++) {
        if (line[i+0] == figure &&
            line[i+1] == figure &&
            line[i+2] == figure &&
            line[i+3] == figure &&
            line[i+4] == figure) return true;
    }
    return false;
}
