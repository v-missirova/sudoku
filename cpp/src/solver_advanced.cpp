#include "solver_advanced.h"

#include <cstdint>
#include <cstring>

bool find_empty_mrv(int board[9][9], uint16_t domains[9][9], int &out_row, int &out_col) {
    int min_candidates = 10;
    bool found = false;

    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (board[i][j] == 0) {
                int candidates = 0;
                for (uint16_t bits = domains[i][j]; bits != 0; bits &= bits - 1)
                    ++candidates;
                if (candidates < min_candidates) {
                    min_candidates = candidates;
                    out_row = i;
                    out_col = j;
                    found = true;
                }
            }
        }
    }
    return found;
}

bool forward_check(int board[9][9], uint16_t domains[9][9], int row, int col, int val) {
    uint16_t mask = ~(1 << val);
    int start_row = (row / 3) * 3;
    int start_col = (col / 3) * 3;

    for (int i = 0; i < 9; i++) {
        if (board[i][col] == 0) {
            domains[i][col] &= mask;
            if (domains[i][col] == 0) return false;
        }
        if (board[row][i] == 0) {
            domains[row][i] &= mask;
            if (domains[row][i] == 0) return false;
        }
        int r = start_row + (i / 3);
        int c = start_col + (i % 3);
        if (board[r][c] == 0) {
            domains[r][c] &= mask;
            if (domains[r][c] == 0) return false;
        }
    }
    return true;
}

bool solver(int board[9][9], uint16_t domains[9][9]) {
    int row, col;

    if (!find_empty_mrv(board, domains, row, col)) {
        return true;
    }

    for (int k = 1; k <= 9; k++) {
        if (domains[row][col] & (1 << k)) {
            uint16_t next_domains[9][9];
            std::memcpy(next_domains, domains, sizeof(next_domains));
            board[row][col] = k;

            if (forward_check(board, next_domains, row, col, k)) {
                if (solver(board, next_domains)) {
                    return true;
                }
            }

            board[row][col] = 0;
        }
    }
    return false;
}


bool multi_solve(int board[9][9], uint16_t domains[9][9], int& num_solutions) {
    int row, col;
    if (!find_empty_mrv(board, domains, row, col)) {
        ++num_solutions;
        return num_solutions >= 2;
    }
    for (int k = 1; k <= 9; k++) {
        if (domains[row][col] & (1 << k)) {

            uint16_t next_domains[9][9];
            std::memcpy(next_domains, domains, sizeof(next_domains));

            board[row][col] = k;

            if (forward_check(board, next_domains, row, col, k)) {
                if (multi_solve(board, next_domains, num_solutions)) {
                    return true;
                }
            }

            board[row][col] = 0;
        }
    }

    return false;
}

