#ifndef BOARD_H
#define BOARD_H

#include "globals.h"

using namespace std;

class Cell {
public:
  bool isTerminal = false;
  int color = 0;
  bool hasPipe = false;
  int id;
};

class Board {
public:
  Board() {
    board.resize(GRID, vector<Cell>(GRID));
  }
  Board(const Board &other) {
    N = other.N;
    board = other.board;
    saved_paths = other.saved_paths;
    terminals = other.terminals;
}

Board& operator=(const Board &other) {
    N = other.N;
    board = other.board;
    saved_paths = other.saved_paths;
    terminals = other.terminals;
    return *this;
} 
  vector<vector<Cell>> board;
  vector<vector<pair<int, int>>> saved_paths;
  vector<pair<pair<int,int>, pair<int,int>>> terminals;
  int N=0;
  void init(int n){
    N=n;
    board.assign(N,vector<Cell>(N));
  }
  bool makeMove(vector<pair<int, int>>);
  bool isValidPath(vector<pair<int, int>>);
  void applyPath(vector<pair<int,int>> path);
  bool loadFromFile(const std::string &filename);
  bool removePath(vector<pair<int, int>>);
  void undoMove();
  void resetBoard();
};

#endif
