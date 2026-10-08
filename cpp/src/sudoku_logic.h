#pragma once

namespace sudoku {
enum class Difficulty { Easy, Medium, Hard };

class Game {
public:
    bool new_game(Difficulty difficulty);
    bool regenerate();
    bool set_cell(int row, int col, int value);
    bool clear_cell(int row, int col);
    bool give_hint();
    bool solve();
    bool is_solved() const;
    bool is_given(int row, int col) const;
    int count_empty() const;
    int clue_count() const;
    bool has_puzzle() const { return active_; }
    Difficulty difficulty() const { return difficulty_; }
    const int (&board() const)[9][9] { return board_; }
    const int (&puzzle() const)[9][9] { return puzzle_; }
    const int (&solution() const)[9][9] { return solution_; }
private:
    int board_[9][9] = {};
    int puzzle_[9][9] = {};
    int solution_[9][9] = {};
    Difficulty difficulty_ = Difficulty::Easy;
    bool active_ = false;
};
}
