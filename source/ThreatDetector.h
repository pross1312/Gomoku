#pragma once
#include "BitBoard.h"
#include <cassert>
#include <array>

#define THREAT_RANGE (RADIUS*2+1)
#define CACHE_SIZE (1<<(2*THREAT_RANGE))

typedef uint32_t ThreatType;
struct Threat {
    enum : ThreatType {
        None = 0,
        BrokenThree = 5,
        StraightThree = 10,
        BrokenFour = 50,
        StraightFour = 100,
        StraightFive = 1000,
    };
    static const char* to_text(ThreatType threat) {
        switch (threat) {
            case Threat::None: return "None";
            case Threat::BrokenThree: return "BrokenThree";
            case Threat::StraightThree: return "StraightThree";
            case Threat::BrokenFour: return "BrokenFour";
            case Threat::StraightFour: return "StraightFour";
            case Threat::StraightFive: return "StraightFive";
            default: assert(false && "Unknown threat");
        }
    }
};

struct ThreatDetector {
    struct Cache {
        ThreatType table[CACHE_SIZE];

        Cache();
        ThreatType operator[](size_t i) const { return table[i]; }
    };

    static ThreatType detect(uint32_t line);
    static ThreatType check(uint32_t line);

    static bool is_broken_three(std::array<Cell, THREAT_RANGE> &line, Cell figure);
    static bool is_straight_three(std::array<Cell, THREAT_RANGE> &line, Cell figure);
    static bool is_straight_five(std::array<Cell, THREAT_RANGE> &line, Cell figure);
    static bool is_straight_four(std::array<Cell, THREAT_RANGE> &line, Cell figure);
    static bool is_broken_four(std::array<Cell, THREAT_RANGE> &line, Cell figure);
};
