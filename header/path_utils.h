#ifndef PATH_UTILS_H
#define PATH_UTILS_H

#include "board.h"
#include <vector>
#include <set>

std::vector<Position> bfsShortestPath(
    const Board& board,
    const Position& start,
    const Position& goal,
    const std::set<Position>& allowedRegion,
    int pathColor
);

#endif
