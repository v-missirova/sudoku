#pragma once
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>
#include "sudoku_logic.h"
#include "jigsaw_logic.h"

namespace godot {
// Thin adapter routes calls to the separate Classic and Jigsaw games.
// Arrays contain 81 values, row-major, with separate clue flags and region IDs.
class SudokuNode : public Node {
    GDCLASS(SudokuNode, Node)
public:
    bool new_game(const String &difficulty, const String &variant = "classic");
    String get_variant() const;
    Array get_regions() const;
    bool regenerate();
    Array get_board() const;
    Array get_puzzle() const;
    Array get_solution() const;
    Array get_clues() const;
    String get_difficulty() const;
    bool has_puzzle() const;
    int get_clue_count() const;
    bool set_cell(int row, int col, int value);
    bool clear_cell(int row, int col);
    bool give_hint();
    bool is_solved() const;
    int count_empty() const;
    bool solve();
protected:
    static void _bind_methods();
private:
    sudoku::Game game_;
    jigsaw::Game jigsaw_game_;
    bool jigsaw_ = false;
};
} // namespace godot
