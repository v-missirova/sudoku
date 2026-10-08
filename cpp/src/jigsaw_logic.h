#pragma once
#include "sudoku_logic.h"
#include "jigsaw_solver.h"

namespace jigsaw {
class Game {
public:
    explicit Game(unsigned seed = std::random_device{}()) : random_(seed) {}
    bool new_game(sudoku::Difficulty difficulty);
    bool regenerate() { return active_ && new_game(difficulty_); }
    bool set_cell(int row, int col, int value);
    bool clear_cell(int row, int col);
    bool give_hint();
    bool solve();
    bool is_solved() const;
    bool is_given(int row, int col) const;
    int count_empty() const;
    int clue_count() const;
    bool has_puzzle() const { return active_; }
    sudoku::Difficulty difficulty() const { return difficulty_; }
    const int (&board() const)[9][9] { return board_; }
    const int (&puzzle() const)[9][9] { return puzzle_; }
    const int (&solution() const)[9][9] { return solution_; }
    const Regions &regions() const { return regions_; }
private:
    int board_[9][9] = {}, puzzle_[9][9] = {}, solution_[9][9] = {};
    Regions regions_{};
    std::mt19937 random_;
    sudoku::Difficulty difficulty_ = sudoku::Difficulty::Easy;
    bool active_ = false;
};
}
