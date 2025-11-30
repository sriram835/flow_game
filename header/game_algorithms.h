
#ifndef ALGO_H
#define ALGO_H
#include "board.h"
#include "globals.h"

vector<vector<int>> getTerminals(Board board);

int randomInt(int upper_bound);
bool colorAlreadyAdded(int color, vector<vector<int>> colors);

vector<pair<int, int>> getNeighbors(int color, int row, int col,
                                    const Board &board,
                                    const vector<vector<bool>> &visited);

vector<pair<int, int>>
reconstructPath(int end_row, int end_col,
                const vector<vector<pair<int, int>>> &parents);
vector<pair<int, int>> bfs(Board board);
#endif
