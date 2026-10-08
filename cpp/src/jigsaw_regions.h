#pragma once
#include <array>
#include <random>

namespace jigsaw {
using Regions = std::array<int, 81>;
bool valid_regions(const Regions &regions);
bool generate_regions(Regions &regions, std::mt19937 &random, int budget = 30000);
}
