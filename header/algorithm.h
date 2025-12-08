#ifndef ALGO_H
#define ALGO_H

#include "board.h"
#include "globals.h"

#include <vector>
#include <utility>

// Top-level called by main.cpp
std::vector<std::pair<int, int>> algorithm(const Board &board);

// Utilities (kept for compatibility & testing)
std::vector<std::vector<int>> getTerminals(Board board);
int randomInt(int upper_bound);
bool colorAlreadyAdded(int color, std::vector<std::vector<int>> colors);

// Original-like helpers (reused/ported)
std::vector<std::pair<int, int>> getNeighbors(int color, int row, int col,
                                              const Board &board,
                                              const std::vector<std::vector<bool>> &visited);

std::vector<std::pair<int, int>>
reconstructPath(int end_row, int end_col,
                const std::vector<std::vector<std::pair<int, int>>> &parents);

#endif
