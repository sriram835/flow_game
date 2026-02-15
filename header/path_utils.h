#ifndef PATH_UTILS_H
#define PATH_UTILS_H

#include "board.h"
#include <vector>
#include <set>

/// Find single shortest path from start to end using BFS constrained to allowedRegion.
/// Returns empty vector if no path.
/// parent map returns parent positions to reconstruct path.
std::vector<Position> bfsShortestPath(
    const Board& board,
    const Position& start,
    const Position& goal,
    const std::set<Position>& allowedRegion
);

#endif // PATH_UTILS_H
