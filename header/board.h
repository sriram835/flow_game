

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
  Cell board[GRID][GRID];
  vector<vector<pair<int, int>>> saved_paths;
  bool makeMove(vector<pair<int, int>>);
  bool isValidPath(vector<pair<int, int>>);
  bool loadFromFile(const std::string &filename);
};

#endif
