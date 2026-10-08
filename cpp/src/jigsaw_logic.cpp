#include "jigsaw_logic.h"
#include <algorithm>
#include <numeric>
#include <vector>

namespace jigsaw {
namespace {
bool coordinates(int row, int col) { return row >= 0 && row < 9 && col >= 0 && col < 9; }
}
bool Game::new_game(sudoku::Difficulty difficulty) {
    Regions regions;
    SearchResult generated;
    for (int attempt = 0; attempt < 48; ++attempt) {
        if (!generate_regions(regions, random_)) continue;
        generated = search(Board{}, regions, 1, 20000, &random_);
        if (generated.solutions == 1) break;
    }
    if (generated.solutions != 1) return false;
    Board puzzle = generated.first;
    int target = 48;
    switch (difficulty) {
        case sudoku::Difficulty::Easy: target = std::uniform_int_distribution<int>(45, 50)(random_); break;
        case sudoku::Difficulty::Medium: target = std::uniform_int_distribution<int>(36, 42)(random_); break;
        case sudoku::Difficulty::Hard: target = std::uniform_int_distribution<int>(28, 34)(random_); break;
    }
    std::array<int, 81> order;
    std::iota(order.begin(), order.end(), 0);
    std::shuffle(order.begin(), order.end(), random_);
    int clues = 81;
    for (int cell : order) {
        if (clues <= target) break;
        int saved = puzzle[cell];
        puzzle[cell] = 0;
        auto checked = search(puzzle, regions, 2, 10000);
        if (checked.solutions == 1 && !checked.exhausted) --clues;
        else puzzle[cell] = saved;
    }
    for (int i = 0; i < 81; ++i) {
        board_[i / 9][i % 9] = puzzle_[i / 9][i % 9] = puzzle[i];
        solution_[i / 9][i % 9] = generated.first[i];
    }
    regions_ = regions;
    difficulty_ = difficulty;
    active_ = true;
    return true;
}
bool Game::is_given(int row, int col) const {
    return active_ && coordinates(row, col) && puzzle_[row][col] != 0;
}
bool Game::set_cell(int row, int col, int value) {
    if (!active_ || !coordinates(row, col) || is_given(row, col) || value < 1 || value > 9) return false;
    board_[row][col] = value;
    return true;
}
bool Game::clear_cell(int row, int col) {
    if (!active_ || !coordinates(row, col) || is_given(row, col)) return false;
    board_[row][col] = 0;
    return true;
}
bool Game::give_hint() {
    if (!active_) return false;
    std::vector<int> candidates;
    for (int i = 0; i < 81; ++i)
        if (board_[i / 9][i % 9] != solution_[i / 9][i % 9]) candidates.push_back(i);
    if (candidates.empty()) return false;
    int cell = candidates[std::uniform_int_distribution<int>(0, static_cast<int>(candidates.size()) - 1)(random_)];
    board_[cell / 9][cell % 9] = solution_[cell / 9][cell % 9];
    return true;
}
bool Game::solve() {
    if (!active_) return false;
    for (int i = 0; i < 81; ++i) board_[i / 9][i % 9] = solution_[i / 9][i % 9];
    return true;
}
bool Game::is_solved() const {
    if (!active_) return false;
    for (int i = 0; i < 81; ++i)
        if (board_[i / 9][i % 9] != solution_[i / 9][i % 9]) return false;
    return true;
}
int Game::count_empty() const {
    int count = 0;
    for (auto &row : board_) for (int value : row) if (!value) ++count;
    return count;
}
int Game::clue_count() const {
    int count = 0;
    for (auto &row : puzzle_) for (int value : row) if (value) ++count;
    return count;
}
}
