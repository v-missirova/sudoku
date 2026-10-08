#include "jigsaw_solver.h"
#include <algorithm>
#include <cstdint>
#include <vector>

namespace jigsaw {
namespace {
using Domains = std::array<uint16_t, 81>;
int count_bits(uint16_t bits) {
    int count = 0;
    while (bits) { bits &= bits - 1; ++count; }
    return count;
}
struct Solver {
    std::array<std::vector<int>, 81> peers;
    SearchResult result;
    int limit, budget;
    std::mt19937 *random;

    void visit(Board &board, const Domains &domains) {
        if (result.solutions >= limit || result.exhausted) return;
        if (--budget < 0) { result.exhausted = true; return; }
        int cell = -1, smallest = 10;
        for (int i = 0; i < 81; ++i) {
            if (board[i]) continue;
            int count = count_bits(domains[i]);
            if (count == 0) return;
            if (count < smallest) { smallest = count; cell = i; }
        }
        if (cell == -1) {
            if (result.solutions == 0) result.first = board;
            ++result.solutions;
            return;
        }
        std::vector<int> values;
        for (int value = 1; value <= 9; ++value)
            if (domains[cell] & (1 << (value - 1))) values.push_back(value);
        if (random) std::shuffle(values.begin(), values.end(), *random);
        for (int value : values) {
            Domains next = domains;
            bool possible = true;
            board[cell] = value;
            next[cell] = 0;
            for (int peer : peers[cell]) {
                if (board[peer]) continue;
                next[peer] &= ~(1 << (value - 1));
                if (!next[peer]) { possible = false; break; }
            }
            if (possible) visit(board, next);
            board[cell] = 0;
            if (result.solutions >= limit || result.exhausted) return;
        }
    }
};
}

SearchResult search(const Board &input, const Regions &regions, int limit,
                    int budget, std::mt19937 *random) {
    if (limit < 1 || !valid_regions(regions)) return {};
    Solver solver{{}, {}, limit, budget, random};
    Domains domains;
    domains.fill(0x1ff);
    Board board = input;
    for (int i = 0; i < 81; ++i) {
        if (board[i] < 0 || board[i] > 9) return {};
        for (int j = 0; j < 81; ++j)
            if (i != j && (i / 9 == j / 9 || i % 9 == j % 9 || regions[i] == regions[j]))
                solver.peers[i].push_back(j);
    }
    for (int i = 0; i < 81; ++i) {
        if (!board[i]) continue;
        domains[i] = 0;
        for (int peer : solver.peers[i]) {
            if (board[peer] == board[i]) return {};
            if (!board[peer]) domains[peer] &= ~(1 << (board[i] - 1));
        }
    }
    solver.visit(board, domains);
    return solver.result;
}
}
