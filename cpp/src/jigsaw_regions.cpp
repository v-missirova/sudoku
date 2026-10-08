#include "jigsaw_regions.h"
#include <algorithm>
#include <vector>

namespace jigsaw {
namespace {
std::vector<int> neighbors(int cell) {
    std::vector<int> result;
    if (cell / 9 > 0) result.push_back(cell - 9);
    if (cell / 9 < 8) result.push_back(cell + 9);
    if (cell % 9 > 0) result.push_back(cell - 1);
    if (cell % 9 < 8) result.push_back(cell + 1);
    return result;
}

bool divisible_components(const Regions &regions) {
    std::array<bool, 81> seen{};
    for (int start = 0; start < 81; ++start) {
        if (regions[start] != -1 || seen[start]) continue;
        std::vector<int> pending{start};
        seen[start] = true;
        for (size_t i = 0; i < pending.size(); ++i)
            for (int next : neighbors(pending[i]))
                if (regions[next] == -1 && !seen[next]) {
                    seen[next] = true;
                    pending.push_back(next);
                }
        if (pending.size() % 9 != 0) return false;
    }
    return true;
}

bool grow(Regions &regions, int id, std::vector<int> &shape,
          std::mt19937 &random, int &budget) {
    if (--budget < 0) return false;
    if (shape.size() == 9) {
        if (!divisible_components(regions)) return false;
        if (id == 8) return true;
        int anchor = 0;
        while (regions[anchor] != -1) ++anchor;
        regions[anchor] = id + 1;
        std::vector<int> next_shape{anchor};
        if (grow(regions, id + 1, next_shape, random, budget)) return true;
        regions[anchor] = -1;
        return false;
    }
    std::vector<int> frontier;
    std::array<bool, 81> added{};
    for (int cell : shape)
        for (int next : neighbors(cell))
            if (regions[next] == -1 && !added[next]) {
                frontier.push_back(next);
                added[next] = true;
            }
    std::shuffle(frontier.begin(), frontier.end(), random);
    for (int cell : frontier) {
        regions[cell] = id;
        shape.push_back(cell);
        if (grow(regions, id, shape, random, budget)) return true;
        shape.pop_back();
        regions[cell] = -1;
        if (budget < 0) break;
    }
    return false;
}
}

bool valid_regions(const Regions &regions) {
    std::array<int, 9> counts{};
    for (int id : regions) {
        if (id < 0 || id > 8) return false;
        ++counts[id];
    }
    for (int id = 0; id < 9; ++id) {
        if (counts[id] != 9) return false;
        int start = 0;
        while (regions[start] != id) ++start;
        std::array<bool, 81> seen{};
        std::vector<int> pending{start};
        seen[start] = true;
        for (size_t i = 0; i < pending.size(); ++i)
            for (int next : neighbors(pending[i]))
                if (regions[next] == id && !seen[next]) {
                    seen[next] = true;
                    pending.push_back(next);
                }
        if (pending.size() != 9) return false;
    }
    return true;
}

bool generate_regions(Regions &regions, std::mt19937 &random, int budget) {
    Regions candidate;
    candidate.fill(-1);
    candidate[0] = 0;
    std::vector<int> shape{0};
    if (!grow(candidate, 0, shape, random, budget)) return false;
    bool irregular = false;
    for (int a = 0; a < 81; ++a)
        for (int b = a + 1; b < 81; ++b)
            if (candidate[a] == candidate[b] &&
                (a / 27 != b / 27 || (a % 9) / 3 != (b % 9) / 3))
                irregular = true;
    if (!irregular) return false;
    regions = candidate;
    return true;
}
}
