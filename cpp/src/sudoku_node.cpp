#include "sudoku_node.h"
#include <godot_cpp/core/class_db.hpp>

namespace godot {
namespace {
Array flatten(const int (&board)[9][9]) {
    Array result;
    result.resize(81);
    for (int row = 0; row < 9; ++row)
        for (int col = 0; col < 9; ++col)
            result[row * 9 + col] = board[row][col];
    return result;
}
}
void SudokuNode::_bind_methods() {
    ClassDB::bind_method(D_METHOD("new_game", "difficulty", "variant"), &SudokuNode::new_game, DEFVAL("classic"));
    ClassDB::bind_method(D_METHOD("get_variant"), &SudokuNode::get_variant);
    ClassDB::bind_method(D_METHOD("get_regions"), &SudokuNode::get_regions);
    ClassDB::bind_method(D_METHOD("regenerate"), &SudokuNode::regenerate);
    ClassDB::bind_method(D_METHOD("get_board"), &SudokuNode::get_board);
    ClassDB::bind_method(D_METHOD("get_puzzle"), &SudokuNode::get_puzzle);
    ClassDB::bind_method(D_METHOD("get_solution"), &SudokuNode::get_solution);
    ClassDB::bind_method(D_METHOD("get_clues"), &SudokuNode::get_clues);
    ClassDB::bind_method(D_METHOD("get_difficulty"), &SudokuNode::get_difficulty);
    ClassDB::bind_method(D_METHOD("has_puzzle"), &SudokuNode::has_puzzle);
    ClassDB::bind_method(D_METHOD("get_clue_count"), &SudokuNode::get_clue_count);
    ClassDB::bind_method(D_METHOD("set_cell", "row", "col", "value"), &SudokuNode::set_cell);
    ClassDB::bind_method(D_METHOD("clear_cell", "row", "col"), &SudokuNode::clear_cell);
    ClassDB::bind_method(D_METHOD("give_hint"), &SudokuNode::give_hint);
    ClassDB::bind_method(D_METHOD("is_solved"), &SudokuNode::is_solved);
    ClassDB::bind_method(D_METHOD("count_empty"), &SudokuNode::count_empty);
    ClassDB::bind_method(D_METHOD("solve"), &SudokuNode::solve);
}
bool SudokuNode::new_game(const String &difficulty, const String &variant) {
    sudoku::Difficulty level;
    if (difficulty == "easy") level = sudoku::Difficulty::Easy;
    else if (difficulty == "medium") level = sudoku::Difficulty::Medium;
    else if (difficulty == "hard") level = sudoku::Difficulty::Hard;
    else return false;
    if (variant != "classic" && variant != "jigsaw") return false;
    bool success = variant == "jigsaw" ? jigsaw_game_.new_game(level) : game_.new_game(level);
    if (success) jigsaw_ = variant == "jigsaw";
    return success;
}
String SudokuNode::get_variant() const { return jigsaw_ ? "jigsaw" : "classic"; }
Array SudokuNode::get_regions() const {
    Array result;
    result.resize(81);
    for (int i = 0; i < 81; ++i)
        result[i] = jigsaw_ ? jigsaw_game_.regions()[i] : (i / 27) * 3 + (i % 9) / 3;
    return result;
}
bool SudokuNode::regenerate() { return jigsaw_ ? jigsaw_game_.regenerate() : game_.regenerate(); }
Array SudokuNode::get_board() const { return flatten(jigsaw_ ? jigsaw_game_.board() : game_.board()); }
Array SudokuNode::get_puzzle() const { return flatten(jigsaw_ ? jigsaw_game_.puzzle() : game_.puzzle()); }
Array SudokuNode::get_solution() const { return flatten(jigsaw_ ? jigsaw_game_.solution() : game_.solution()); }
Array SudokuNode::get_clues() const {
    Array result;
    result.resize(81);
    for (int row = 0; row < 9; ++row)
        for (int col = 0; col < 9; ++col)
            result[row * 9 + col] = jigsaw_ ? jigsaw_game_.is_given(row, col) : game_.is_given(row, col);
    return result;
}
String SudokuNode::get_difficulty() const {
    switch (jigsaw_ ? jigsaw_game_.difficulty() : game_.difficulty()) {
        case sudoku::Difficulty::Easy: return "easy";
        case sudoku::Difficulty::Medium: return "medium";
        case sudoku::Difficulty::Hard: return "hard";
    }
    return "easy";
}
bool SudokuNode::has_puzzle() const { return jigsaw_ ? jigsaw_game_.has_puzzle() : game_.has_puzzle(); }
int SudokuNode::get_clue_count() const { return jigsaw_ ? jigsaw_game_.clue_count() : game_.clue_count(); }
bool SudokuNode::set_cell(int row, int col, int value) { return jigsaw_ ? jigsaw_game_.set_cell(row, col, value) : game_.set_cell(row, col, value); }
bool SudokuNode::clear_cell(int row, int col) { return jigsaw_ ? jigsaw_game_.clear_cell(row, col) : game_.clear_cell(row, col); }
bool SudokuNode::give_hint() { return jigsaw_ ? jigsaw_game_.give_hint() : game_.give_hint(); }
bool SudokuNode::is_solved() const { return jigsaw_ ? jigsaw_game_.is_solved() : game_.is_solved(); }
int SudokuNode::count_empty() const { return jigsaw_ ? jigsaw_game_.count_empty() : game_.count_empty(); }
bool SudokuNode::solve() { return jigsaw_ ? jigsaw_game_.solve() : game_.solve(); }
}
