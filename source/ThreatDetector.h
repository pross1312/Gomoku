#pragma once
#include "BitBoard.h"
#include <cassert>
#include <array>

#define THREAT_RANGE (RADIUS*2+1)
#define CACHE_SIZE (1<<(2*THREAT_RANGE))

typedef uint8_t ThreatType;
struct Threat {
    enum : ThreatType {
        None = 0,
        BrokenTwo,
        StraightTwo,
        BrokenThree,
        StraightThree,
        BrokenFour,
        StraightFour,
        StraightFive,
    };
    static const char* to_text(ThreatType threat) {
        switch (threat) {
            case Threat::None: return "None";
            case Threat::BrokenTwo: return "BrokenTwo";
            case Threat::StraightTwo: return "StraightTwo";
            case Threat::BrokenThree: return "BrokenThree";
            case Threat::StraightThree: return "StraightThree";
            case Threat::BrokenFour: return "BrokenFour";
            case Threat::StraightFour: return "StraightFour";
            case Threat::StraightFive: return "StraightFive";
        }
        assert(false && "Unknown threat");
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

    static uint32_t atk_value(ThreatType threat);
    static uint32_t def_value(ThreatType threat);
    static uint32_t threshold();

    static bool is_broken_two(std::array<Figure, THREAT_RANGE> &line, Figure figure);
    static bool is_broken_three(std::array<Figure, THREAT_RANGE> &line, Figure figure);
    static bool is_broken_four(std::array<Figure, THREAT_RANGE> &line, Figure figure);
    static bool is_straight_two(std::array<Figure, THREAT_RANGE> &line, Figure figure);
    static bool is_straight_three(std::array<Figure, THREAT_RANGE> &line, Figure figure);
    static bool is_straight_four(std::array<Figure, THREAT_RANGE> &line, Figure figure);
    static bool is_straight_five(std::array<Figure, THREAT_RANGE> &line, Figure figure);
};
