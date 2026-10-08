#include "sudoku_logic.h"
#include "solver_advanced.h"
#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <random>
#include <vector>

namespace sudoku {
static bool is_legal(const int board[9][9], int x, int y, int value) {
    for (int i = 0; i < 9; i++) {
        if (i != y && std::abs(board[x][i]) == value) return false;
        if (i != x && std::abs(board[i][y]) == value) return false;
    }
    int sx = (x/3)*3, sy = (y/3)*3;
    for (int i = sx; i < sx+3; i++)
        for (int j = sy; j < sy+3; j++)
            if ((i!=x||j!=y) && std::abs(board[i][j])==value) return false;
    return true;
}

static bool generate_board(int board[9][9]) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    for (int i = 0; i < 9; i++) {
        for (int j = 0; j < 9; j++) {
            if (board[i][j] == 0) {
                std::vector<int> vals = {1,2,3,4,5,6,7,8,9};
                while (!vals.empty()) {
                    std::uniform_int_distribution<> d(0,(int)vals.size()-1);
                    int idx = d(gen), v = vals[idx];
                    vals.erase(vals.begin()+idx);
                    if (is_legal(board,i,j,v)) {
                        board[i][j] = v;
                        if (generate_board(board)) return true;
                    }
                }
                board[i][j] = 0;
                return false;
            }
        }
    }
    return true;
}

static void init_domains(int board[9][9], uint16_t domains[9][9]) {
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            domains[i][j] = 0x03FE;
    for (int i = 0; i < 9; i++)
        for (int j = 0; j < 9; j++)
            if (board[i][j] != 0)
                forward_check(board, domains, i, j, std::abs(board[i][j]));
}

static bool multi_solve_fresh(int board[9][9], int &num_solutions) {
    int tmp[9][9]; std::memcpy(tmp, board, sizeof(tmp));
    uint16_t domains[9][9]; init_domains(tmp, domains);
    return multi_solve(tmp, domains, num_solutions);
}

static void generate_puzzle(int board[9][9], int numEntries) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    int remaining = 0;
    for (int i=0;i<9;i++) for(int j=0;j<9;j++) if(board[i][j]!=0) ++remaining;

    bool progress = true;
    while (remaining > numEntries && progress) {
        progress = false;
        std::vector<std::pair<int,int>> cells;
        for(int i=0;i<9;i++) for(int j=0;j<9;j++) if(board[i][j]!=0) cells.push_back({i,j});
        std::shuffle(cells.begin(), cells.end(), gen);
        for (auto [x,y] : cells) {
            if (remaining <= numEntries) return;
            int saved = board[x][y]; board[x][y] = 0;
            int n = 0;
            multi_solve_fresh(board, n);
            if (n != 1) board[x][y] = saved;
            else { --remaining; progress = true; }
        }
    }
}

static int count_empty_board(const int board[9][9]) {
    int c=0; for(int i=0;i<9;i++) for(int j=0;j<9;j++) if(board[i][j]==0) ++c; return c;
}

static bool check_solution(const int board[9][9], const int solution[9][9]) {
    for(int i=0;i<9;i++) for(int j=0;j<9;j++)
        if(std::abs(board[i][j])!=std::abs(solution[i][j])) return false;
    return true;
}

static bool reveal_hint(int board[9][9], const int solution[9][9]) {
    static std::random_device rd;
    static std::mt19937 gen(rd());
    std::vector<std::pair<int,int>> cands;
    for(int i=0;i<9;i++) for(int j=0;j<9;j++)
        if(board[i][j]>=0 && std::abs(board[i][j])!=std::abs(solution[i][j]))
            cands.push_back({i,j});
    if(cands.empty()) return false;
    std::uniform_int_distribution<> d(0,(int)cands.size()-1);
    auto [x,y] = cands[d(gen)];
    board[x][y] = std::abs(solution[x][y]);
    return true;
}


static bool valid_cell(int row, int col) {
    return row >= 0 && row < 9 && col >= 0 && col < 9;
}

bool Game::new_game(Difficulty difficulty) {
    static std::mt19937 random(std::random_device{}());
    int minimum, maximum;
    switch (difficulty) {
        case Difficulty::Easy: minimum = 33; maximum = 38; break;
        case Difficulty::Medium: minimum = 25; maximum = 30; break;
        case Difficulty::Hard: minimum = 20; maximum = 25; break;
        default: return false;
    }
    int solution[9][9] = {};
    if (!generate_board(solution)) return false;
    int puzzle[9][9];
    std::memcpy(puzzle, solution, sizeof(puzzle));
    const int target = std::uniform_int_distribution<int>(minimum, maximum)(random);
    generate_puzzle(puzzle, target);
    std::memcpy(solution_, solution, sizeof(solution_));
    std::memcpy(puzzle_, puzzle, sizeof(puzzle_));
    std::memcpy(board_, puzzle, sizeof(board_));
    difficulty_ = difficulty;
    active_ = true;
    return true;
}

bool Game::regenerate() { return new_game(difficulty_); }
bool Game::is_given(int row, int col) const {
    return active_ && valid_cell(row, col) && puzzle_[row][col] != 0;
}
bool Game::set_cell(int row, int col, int value) {
    if (!active_ || !valid_cell(row, col) || value < 1 || value > 9 || is_given(row, col))
        return false;
    board_[row][col] = value;
    return true;
}
bool Game::clear_cell(int row, int col) {
    if (!active_ || !valid_cell(row, col) || is_given(row, col)) return false;
    board_[row][col] = 0;
    return true;
}
bool Game::give_hint() { return active_ && reveal_hint(board_, solution_); }
bool Game::is_solved() const { return active_ && check_solution(board_, solution_); }
int Game::count_empty() const { return count_empty_board(board_); }
int Game::clue_count() const { return active_ ? 81 - count_empty_board(puzzle_) : 0; }
bool Game::solve() {
    if (!active_) return false;
    std::memcpy(board_, solution_, sizeof(board_));
    return true;
}
}

