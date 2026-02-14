#include "board.h"

#ifndef SOLVER_H
#define SOLVER_H
void solver(Board board, string file_name);
vector<int> getAvailableTerminals(const Board &board);
vector<pair<int, int>> getEmptySpaces(const Board &board);
bool isBoardFull(const Board &board);
bool isValidBoard(const Board &board);
void saveBoard(const Board &board, const string &file_name);

#endif
