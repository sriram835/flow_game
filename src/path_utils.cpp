#include "path_utils.h"
#include <algorithm>
#include <queue>
#include <string>
#include <unordered_map>
#include <unordered_set>

static inline std::string posKey(const Position &p) {
    return std::to_string(p.row) + ":" + std::to_string(p.col);
}

std::vector<Position> bfsShortestPath(
    const Board& board,
    const Position& start,
    const Position& goal,
    const std::set<Position>& allowedRegion
) {
    std::vector<Position> emptyResult;
    if (start.row == goal.row && start.col == goal.col) return {start};

    // Quick reject if start or goal not in allowedRegion
    if (allowedRegion.count(start) == 0 || allowedRegion.count(goal) == 0) {
        return emptyResult;
    }

    // Convert allowedRegion to an unordered_set of keys for O(1) membership tests
    std::unordered_set<std::string> allowedKeys;
    allowedKeys.reserve(allowedRegion.size() * 2 + 1);
    for (const auto &p : allowedRegion) {
        allowedKeys.insert(posKey(p));
    }

    std::queue<Position> q;
    std::unordered_map<std::string, Position> parent;
    parent.reserve(allowedRegion.size() * 2 + 1);
    std::unordered_set<std::string> visited;
    visited.reserve(allowedRegion.size() * 2 + 1);

    q.push(start);
    visited.insert(posKey(start));

    int dr[4] = {1,-1,0,0};
    int dc[4] = {0,0,1,-1};

    bool found = false;
    while (!q.empty()) {
        Position cur = q.front(); q.pop();

        for (int i=0;i<4;i++) {
            int nr = cur.row + dr[i];
            int nc = cur.col + dc[i];
            Position nxt{nr,nc};

            if (!board.isInside(nr,nc)) continue;

            std::string nextKey = posKey(nxt);
            // membership check in allowed region using unordered_set
            if (allowedKeys.find(nextKey) == allowedKeys.end()) continue;

            if (visited.find(nextKey) != visited.end()) continue;

            visited.insert(nextKey);
            parent[nextKey] = cur;

            if (nxt.row == goal.row && nxt.col == goal.col) {
                found = true;
                break;
            }
            q.push(nxt);
        }
        if (found) break;
    }

    if (!found) return emptyResult;

    // reconstruct path
    std::vector<Position> path;
    Position cur = goal;
    while (!(cur.row == start.row && cur.col == start.col)) {
        path.push_back(cur);
        cur = parent[posKey(cur)];
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}
