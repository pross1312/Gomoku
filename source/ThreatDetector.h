#pragma once
#include "BitBoard.h"
#include <array>

#define THREAT_RANGE (RADIUS*2+1)
#define CACHE_SIZE (1<<(2*THREAT_RANGE))

enum ThreatType : uint32_t {
    ThreatNone = 0,
    StraightThree = 10,
    StraightFour = 100,
    StraightFive = 1000,
};

struct ThreatDetector {
    struct Cache {
        ThreatType table[CACHE_SIZE];

        Cache();
        ThreatType operator[](size_t i) const { return table[i]; }
    };

    static ThreatType detect(uint32_t line);
    static ThreatType check(uint32_t line);

    static bool is_straight_three(std::array<Cell, THREAT_RANGE> &line, Cell figure);
    static bool is_straight_five(std::array<Cell, THREAT_RANGE> &line, Cell figure);
    static bool is_straight_four(std::array<Cell, THREAT_RANGE> &line, Cell figure);
};
