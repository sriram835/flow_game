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
    const std::set<Position>& allowedRegion,
    int pathColor
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

    struct Node {
        Position pos;
        int conflict;
        int length;
    };

    auto cmp = [](const Node& a, const Node& b) {
        if (a.conflict != b.conflict) return a.conflict > b.conflict;
        if (a.length != b.length) return a.length > b.length;
        if (a.pos.row != b.pos.row) return a.pos.row > b.pos.row;
        return a.pos.col > b.pos.col;
    };

    std::priority_queue<Node, std::vector<Node>, decltype(cmp)> pq(cmp);
    std::unordered_map<std::string, std::pair<int,int>> bestCost; // key -> (conflict, length)
    std::unordered_map<std::string, Position> parent;
    parent.reserve(allowedRegion.size() * 2 + 1);

    auto startKey = posKey(start);
    bestCost[startKey] = {0,0};
    pq.push({start, 0, 0});

    int dr[4] = {1,-1,0,0};
    int dc[4] = {0,0,1,-1};

    bool found = false;
    Position finalPos = start;

    while (!pq.empty()) {
        Node curNode = pq.top(); pq.pop();
        Position cur = curNode.pos;
        std::string curKey = posKey(cur);

        auto itBest = bestCost.find(curKey);
        if (itBest == bestCost.end()) continue;
        if (curNode.conflict > itBest->second.first) continue;
        if (curNode.conflict == itBest->second.first &&
            curNode.length > itBest->second.second) continue;

        if (cur.row == goal.row && cur.col == goal.col) {
            found = true;
            finalPos = cur;
            break;
        }

        for (int i = 0; i < 4; i++) {
            int nr = cur.row + dr[i];
            int nc = cur.col + dc[i];
            Position nxt{nr,nc};

            if (!board.isInside(nr,nc)) continue;

            std::string nextKey = posKey(nxt);
            if (allowedKeys.find(nextKey) == allowedKeys.end()) continue;

            // incremental conflict: neighbors of nxt that are foreign-colored pipes
            int addConflict = 0;
            for (int k = 0; k < 4; k++) {
                int ar = nr + dr[k];
                int ac = nc + dc[k];
                if (!board.isInside(ar,ac)) continue;
                const Cell& ncell = board.getCell(ar,ac);
                if (ncell.hasPipe && ncell.color != pathColor)
                    addConflict++;
            }

            int newConflict = curNode.conflict + addConflict;
            int newLength   = curNode.length + 1;

            auto it = bestCost.find(nextKey);
            if (it == bestCost.end() ||
                newConflict < it->second.first ||
                (newConflict == it->second.first && newLength < it->second.second)) {
                bestCost[nextKey] = {newConflict, newLength};
                parent[nextKey] = cur;
                pq.push({nxt, newConflict, newLength});
            }
        }
    }

    if (!found) return emptyResult;

    // reconstruct minimal-conflict path
    std::vector<Position> path;
    Position cur = finalPos;
    while (!(cur.row == start.row && cur.col == start.col)) {
        path.push_back(cur);
        cur = parent[posKey(cur)];
    }
    path.push_back(start);
    std::reverse(path.begin(), path.end());
    return path;
}
