#ifndef GREEDY_WRAP_ALGORITHM_H
#define GREEDY_WRAP_ALGORITHM_H

#include "board.h"
#include <vector>
#include <queue>
#include <limits>

using namespace std;

// Dijkstra-based algorithm with radial weighting from center
// Lower weight at borders encourages pipes to wrap around the edges
std::vector<std::pair<int, int>> greedyWrapAlgorithm(const Board &board);

#endif
