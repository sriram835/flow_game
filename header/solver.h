#ifndef SOLVER_H
#define SOLVER_H

#include "board.h"
#include <unordered_map>
#include <set>
#include <vector>

class Solver {
public:
    bool solve(Board& board);

private:
    bool isColorFinished(const Board& board, int color);

    std::unordered_map<int, std::set<Position>>
    createInitialRectangles(const Board& board);

    std::set<Position>
    expandRegion(const Board& board, const std::set<Position>& seedRegion);

    std::vector<std::vector<Position>>
    generateCandidatePaths(
        const Board& board,
        int color,
        const std::set<Position>& allowedRegion,
        int maxPaths = 5
    );

    void lockPath(Board& board, int color, const std::vector<Position>& path);

    int computeConflictScore(const std::vector<Position>& path, const Board& board, int pathColor);

    bool forwardCheck(Board& board, const std::set<int>& finishedColors, int excludedColor);
};

#endif
