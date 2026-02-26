
#ifndef ALGO_H
#define ALGO_H
#include "board.h"
#include "globals.h"

bool dfsColor(Board &board, int x, int y, int tx, int ty, int color,
              int colorIndex, vector<vector<bool>> &visited);
bool solver(Board &board, int index, vector<vector<bool>> &visited);
vector<pair<int, int>> algorithm(const Board board);
unordered_map<int, pair<pair<int, int>, pair<int, int>>>
getTerminals(const Board &board);
int distanceBetween(const pair<int, int> &a, const pair<int, int> &b);
map<int, vector<int>> getDistanceColor(
    unordered_map<int, pair<pair<int, int>, pair<int, int>>> terminals);
vector<vector<int>> getRegion(pair<int, int> start_index,
                              pair<int, int> end_index, const Board board);

bool pointInPolygon(const vector<pair<double, double>> &poly,
                    pair<double, double> p);
vector<pair<int, int>> dfsFindPath(pair<int, int> start, pair<int, int> end,
                                   const Board &board,
                                   const vector<vector<int>> &region);

bool dfsUtil(int i, int j, int ei, int ej, const Board &board,
             const vector<vector<int>> &region, vector<vector<bool>> &visited,
             vector<pair<int, int>> &path);
#endif
