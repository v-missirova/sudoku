#ifndef SUDOKU_SOLVER_ADVANCED_H
#define SUDOKU_SOLVER_ADVANCED_H
#include <cstdint>

bool find_empty_mrv(int board[9][9], uint16_t domains[9][9], int &out_row, int &out_col);

bool forward_check(int board[9][9], uint16_t domains[9][9], int row, int col, int val);

bool solver(int board[9][9], uint16_t domains[9][9]);

bool multi_solve(int board[9][9], uint16_t domains[9][9], int& num_solutions);

#endif
