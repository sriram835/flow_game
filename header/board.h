#ifndef BOARD_H
#define BOARD_H

#include "globals.h"
#include <vector>
#include <string>

using namespace std;

class Cell {
public:
    bool isTerminal = false;
    int color = 0;
    bool hasPipe = false;
    int id = -1;
};

class Board {
public:

    int N = 0;   // local copy of GRID

    vector<vector<Cell>> board;
    vector<vector<pair<int,int>>> saved_paths;

    // List of all terminal pairs for algorithm.cpp
    // Example: terminals[color] = { start, end }
    vector<pair<pair<int,int>, pair<int,int>>> terminals;

    // Backup for reset()
    vector<vector<Cell>> initial_board;
    vector<vector<pair<int,int>>> initial_paths;

    Board() {} // <-- do NOT resize here

    // Call this after GRID is known
    void init(int n) {
        N = n;
        board.assign(N, vector<Cell>(N));
    }
    bool makeMove(const std::vector<std::pair<int,int>> &indexes);
    bool isValidPath(const vector<pair<int,int>> &path) const;
    bool removePath(const std::vector<std::pair<int,int>> &path);

    bool loadFromFile(const std::string &filename);

    void undoMove();
    void resetBoard();
};

#endif
