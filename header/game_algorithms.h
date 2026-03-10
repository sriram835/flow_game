
#ifndef ALGO_H
#define ALGO_H
#include "board.h"
#include "globals.h"
extern unordered_map<int, pair<pair<int, int>, pair<int, int>>>
    color_to_terminal;

extern unordered_map<int, vector<pair<int, int>>> paths;


bool canVisit(const Board &board, int x, int y, int color,
              vector<vector<bool>> &visited);

bool isCompletedVisited(const Board &board, vector<vector<bool>> &visited);

pair<pair<int, int>, pair<int, int>> findTerminals(const Board &board,
                                                   int color);

bool solver(
    Board &board, int index, vector<vector<bool>> &visited, vector<int> &colors,
    unordered_map<int, pair<pair<int, int>, pair<int, int>>> &color_terminal);

bool single_color_dfs_check(
    Board &board, const vector<vector<bool>> &visited, int color,
    unordered_map<int, pair<pair<int, int>, pair<int, int>>> &color_terminal,
    vector<vector<bool>> &seen, int i, int j, int end_i, int end_j);

bool dfs_feasibility_check(Board &board, vector<vector<bool>> &visited,
                           vector<int> &colors, int colorIndex,unordered_map<int, pair<pair<int, int>, pair<int, int>>>
        &color_terminal);

bool dfsColor(Board &board, int x, int y, int tx, int ty, int color,
              int colorIndex, vector<vector<bool>> &visited,
              vector<int> &colors,unordered_map<int, pair<pair<int, int>, pair<int, int>>> &color_terminal);

bool isCompleted(const Board &board);
unordered_map<int, pair<pair<int, int>, pair<int, int>>>
getTerminals(const Board &board);

#endif
