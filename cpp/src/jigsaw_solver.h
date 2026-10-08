#pragma once
#include "jigsaw_regions.h"

namespace jigsaw {
using Board = std::array<int, 81>;
struct SearchResult {
    int solutions = 0;
    bool exhausted = false;
    Board first{};
};

SearchResult search(const Board &board, const Regions &regions, int limit,
                    int budget, std::mt19937 *random = nullptr);
}
