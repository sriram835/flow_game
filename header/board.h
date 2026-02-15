#ifndef BOARD_H
#define BOARD_H

#include "globals.h"
#include <string>
#include <utility>
#include <vector>

class Cell {
public:
    bool isTerminal = false;
    int color = 0;
    bool hasPipe = false;
    int id = -1;
};

struct Position {
    int row;
    int col;

    bool operator<(const Position& other) const {
        if(row == other.row) 
            return col < other.col;
        return row < other.row;
    }
};

class Board {
public:

    int N = 0;   // local copy of GRID

    std::vector<std::vector<Cell>> board;
    std::vector<std::vector<std::pair<int, int>>> saved_paths;

    // List of all terminal pairs for algorithm.cpp
    // Example: terminals[color] = { start, end }
    std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> terminals;

    // Backup for reset()
    std::vector<std::vector<Cell>> initial_board;
    std::vector<std::vector<std::pair<int, int>>> initial_paths;

    Board() = default; // do not resize here

    // Call this after GRID is known
    void init(int n) {
        N = n;
        board.assign(N, std::vector<Cell>(N));
    }
    bool isInside(int r, int c) const {
        return r >= 0 && r<N && c>=0 && c<N;
    }

    Cell& getCell(int r, int c) {
        return board[r][c];
    }

    const Cell& getCell(int r, int c) const {
        return board[r][c];
    }
    bool makeMove(const std::vector<std::pair<int,int>> &indexes);
    bool isValidPath(const std::vector<std::pair<int, int>> &path) const;
    bool removePath(const std::vector<std::pair<int,int>> &path);

    bool loadFromFile(const std::string &filename);

    void undoMove();
    void resetBoard();
};

#endif
