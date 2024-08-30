#include "BitBoard.h"
#include <functional>

struct ArrayHasher {
    std::size_t operator()(const std::array<uint32_t, SIZE>& a) const {
        std::size_t h = 0;

        for (auto e : a) {
            h ^= std::hash<uint32_t>{}(e)  + 0x9e3779b9 + (h << 6) + (h >> 2); 
        }
        return h;
    }
};
