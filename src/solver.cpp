#include "solver.h"
#include "path_utils.h"

#include <queue>
#include <limits>
#include <algorithm>
#include <iostream>
#include <map>
#include <climits>

using namespace std;

// ------------------------------------------------------------
// Ensure board.terminals is populated from grid (if empty).
// Color index i corresponds to i-th distinct terminal color (sorted).
// ------------------------------------------------------------
static void ensureTerminalsFromBoard(Board& board)
{
    if (!board.terminals.empty())
        return;

    map<int, vector<pair<int,int>>> colorToPositions;
    for (int r = 0; r < board.N; r++)
        for (int c = 0; c < board.N; c++)
        {
            const Cell& cell = board.getCell(r, c);
            if (cell.isTerminal && cell.color != 0)
                colorToPositions[cell.color].push_back({r, c});
        }

    for (const auto& entry : colorToPositions)
    {
        const auto& positions = entry.second;
        if (positions.size() >= 2)
            board.terminals.push_back({ positions[0], positions[1] });
    }
}

// ------------------------------------------------------------
// Check if color (by index) is already solved via saved_paths.
// ------------------------------------------------------------
bool Solver::isColorFinished(const Board& board, int colorIndex)
{
    if (colorIndex < 0 || colorIndex >= (int)board.terminals.size())
        return true;

    int actualColor = board.getCell(
        board.terminals[colorIndex].first.first,
        board.terminals[colorIndex].first.second
    ).color;

    for (const auto& path : board.saved_paths)
    {
        if (path.empty()) continue;

        int r = path.front().first;
        int c = path.front().second;
        if (board.getCell(r, c).color == actualColor)
            return true;
    }
    return false;
}

// ------------------------------------------------------------
// Rectangle seeds (minimal bounding rect between terminals).
// ------------------------------------------------------------
unordered_map<int, set<Position>>
Solver::createInitialRectangles(const Board& board)
{
    unordered_map<int, set<Position>> regions;
    int totalColors = (int)board.terminals.size();

    for (int color = 0; color < totalColors; color++)
    {
        auto s = board.terminals[color].first;
        auto e = board.terminals[color].second;

        int r1 = min(s.first, e.first);
        int r2 = max(s.first, e.first);
        int c1 = min(s.second, e.second);
        int c2 = max(s.second, e.second);

        for (int r = r1; r <= r2; r++)
            for (int c = c1; c <= c2; c++)
                regions[color].insert({r, c});
    }

    return regions;
}

// ------------------------------------------------------------
// Remove from seed: other colors' pipes and other terminals.
// ------------------------------------------------------------
static void filterSeed(const Board& board, int color, set<Position>& seed)
{
    int ourColor = board.getCell(
        board.terminals[color].first.first,
        board.terminals[color].first.second
    ).color;

    vector<Position> removeList;
    for (auto& p : seed)
    {
        const Cell& cell = board.getCell(p.row, p.col);
        if (cell.hasPipe && cell.color != ourColor)
            removeList.push_back(p);
        else if (cell.isTerminal && cell.color != ourColor)
            removeList.push_back(p);
    }
    for (auto& p : removeList)
        seed.erase(p);
}

// ------------------------------------------------------------
// Expansion: flood fill into reachable blank cells only.
// ------------------------------------------------------------
set<Position>
Solver::expandRegion(const Board& board, const set<Position>& seed)
{
    set<Position> expanded = seed;
    queue<Position> q;

    for (auto& p : seed)
        q.push(p);

    int dr[4] = {1, -1, 0, 0};
    int dc[4] = {0, 0, 1, -1};

    while (!q.empty())
    {
        Position cur = q.front(); q.pop();

        for (int i = 0; i < 4; i++)
        {
            int nr = cur.row + dr[i];
            int nc = cur.col + dc[i];

            if (!board.isInside(nr, nc)) continue;

            Position nxt{nr, nc};
            if (expanded.count(nxt)) continue;

            const Cell& cell = board.getCell(nr, nc);
            if (!cell.hasPipe && !cell.isTerminal)
            {
                expanded.insert(nxt);
                q.push(nxt);
            }
        }
    }

    return expanded;
}

// ------------------------------------------------------------
// Lock chosen path and push to board.saved_paths.
// ------------------------------------------------------------
void Solver::lockPath(Board& board, int colorIndex, const vector<Position>& path)
{
    int actualColor = board.getCell(
        board.terminals[colorIndex].first.first,
        board.terminals[colorIndex].first.second
    ).color;

    for (auto& p : path)
    {
        Cell& cell = board.getCell(p.row, p.col);
        cell.hasPipe = true;
        cell.color = actualColor;
    }

    vector<pair<int,int>> saved;
    for (auto& p : path)
        saved.emplace_back(p.row, p.col);

    board.saved_paths.push_back(saved);
}

// ------------------------------------------------------------
// Conflict score if we place this path (path not on board yet).
// pathColor is the terminal color for this path.
// ------------------------------------------------------------
int Solver::computeConflictScore(const vector<Position>& path, const Board& board, int pathColor)
{
    int score = 0;
    int dr[4] = {1, -1, 0, 0};
    int dc[4] = {0, 0, 1, -1};

    for (auto& p : path)
    {
        for (int i = 0; i < 4; i++)
        {
            int nr = p.row + dr[i];
            int nc = p.col + dc[i];

            if (!board.isInside(nr, nc)) continue;

            const Cell& ncell = board.getCell(nr, nc);
            if (ncell.hasPipe && ncell.color != pathColor)
                score++;
        }
    }
    return score;
}

// ------------------------------------------------------------
// Candidate paths: BFS inside allowedRegion only.
// allowedRegion must contain only traversable cells (blanks + our terminals).
// ------------------------------------------------------------
vector<vector<Position>>
Solver::generateCandidatePaths(
    const Board& board,
    int colorIndex,
    const set<Position>& allowedRegion,
    int /* maxPaths */
)
{
    vector<vector<Position>> result;

    auto s_pair = board.terminals[colorIndex].first;
    auto e_pair = board.terminals[colorIndex].second;

    Position start{s_pair.first, s_pair.second};
    Position goal{e_pair.first, e_pair.second};

    set<Position> region = allowedRegion;
    region.insert(start);
    region.insert(goal);

    vector<Position> basePath = bfsShortestPath(board, start, goal, region);

    if (basePath.empty())
        return result;

    result.push_back(basePath);
    return result;
}

// ------------------------------------------------------------
// Forward check: after simulating a lock, every other unfinished
// color must still have at least one candidate path (rectangle or expanded).
// ------------------------------------------------------------
bool Solver::forwardCheck(Board& board, const set<int>& finishedColors, int excludedColor)
{
    int totalColors = (int)board.terminals.size();
    auto regions = createInitialRectangles(board);

    for (int color = 0; color < totalColors; color++)
    {
        if (color == excludedColor) continue;
        if (finishedColors.count(color)) continue;
        if (isColorFinished(board, color)) continue;

        set<Position> seed = regions[color];
        filterSeed(board, color, seed);

        vector<vector<Position>> paths = generateCandidatePaths(board, color, seed, 5);
        if (paths.empty())
        {
            set<Position> expanded = expandRegion(board, seed);
            paths = generateCandidatePaths(board, color, expanded, 5);
        }
        if (paths.empty())
            return false;
    }
    return true;
}

// ------------------------------------------------------------
// SINGLE AI MOVE: rectangle-first, full expansion if needed, MRV, forward check.
// ------------------------------------------------------------
bool Solver::solve(Board& board)
{
    ensureTerminalsFromBoard(board);

    int totalColors = (int)board.terminals.size();
    if (totalColors == 0)
        return false;

    set<int> finishedColors;
    for (int c = 0; c < totalColors; c++)
        if (isColorFinished(board, c))
            finishedColors.insert(c);

    struct ColorInfo {
        int color;
        int rectArea;
        int manhattan;
        vector<vector<Position>> paths;
    };

    vector<ColorInfo> colorInfos;
    vector<int> invalidColors;
    auto regions = createInitialRectangles(board);

    // ---- STEP 1: For each unfinished color ----
    for (int color = 0; color < totalColors; color++)
    {
        if (finishedColors.count(color)) continue;

        auto start = board.terminals[color].first;
        auto end   = board.terminals[color].second;

        set<Position> rectangle = regions[color];
        filterSeed(board, color, rectangle);

        // Try rectangle-only paths first
        vector<vector<Position>> paths = generateCandidatePaths(board, color, rectangle, 5);
        bool expandedUsed = false;
        size_t segmentSize = rectangle.size();

        // If none → full expansion
        if (paths.empty())
        {
            expandedUsed = true;
            set<Position> expanded = expandRegion(board, rectangle);
            segmentSize = expanded.size();
            paths = generateCandidatePaths(board, color, expanded, 5);

            if (paths.empty())
            {
                invalidColors.push_back(color);
                cout << "Color: " << color
                     << " | Segment size: " << segmentSize
                     << " | Expansion: YES | Paths: 0\n";
                continue;
            }
        }

        cout << "Color: " << color
             << " | Segment size: " << segmentSize
             << " | Expansion: " << (expandedUsed ? "YES" : "NO")
             << " | Paths: " << paths.size() << "\n";

        int r1 = min(start.first, end.first), r2 = max(start.first, end.first);
        int c1 = min(start.second, end.second), c2 = max(start.second, end.second);
        int rectArea = (r2 - r1 + 1) * (c2 - c1 + 1);
        int manhattan = abs(start.first - end.first) + abs(start.second - end.second);

        colorInfos.push_back({color, rectArea, manhattan, paths});
    }

    // Only invalid when NO color can move
    if (colorInfos.empty())
    {
        if (!invalidColors.empty())
            cout << "INVALID PATH\n";
        return false;
    }

    // ---- STEP 2: MRV + tie-break ----
    sort(colorInfos.begin(), colorInfos.end(),
        [](const ColorInfo& a, const ColorInfo& b) {
            if (a.paths.size() != b.paths.size())
                return a.paths.size() < b.paths.size();
            if (a.rectArea != b.rectArea)
                return a.rectArea < b.rectArea;
            if (a.manhattan != b.manhattan)
                return a.manhattan > b.manhattan;
            return a.color < b.color;
        });

    ColorInfo chosen = colorInfos.front();
    cout << "Chosen Color: " << chosen.color << "\n";

    int pathColor = board.getCell(
        board.terminals[chosen.color].first.first,
        board.terminals[chosen.color].first.second
    ).color;

    // ---- STEP 3: Path selection with forward check ----
    vector<Position> bestPath;
    int bestScore = INT_MAX;

    for (auto& candidate : chosen.paths)
    {
        Board temp = board;
        lockPath(temp, chosen.color, candidate);

        if (!forwardCheck(temp, finishedColors, chosen.color))
            continue;

        int score = computeConflictScore(candidate, board, pathColor);

        if (score < bestScore ||
            (score == bestScore && (bestPath.empty() || candidate.size() < bestPath.size())))
        {
            bestScore = score;
            bestPath = candidate;
        }
    }

    if (bestPath.empty())
        return false;

    lockPath(board, chosen.color, bestPath);
    cout << "AI locked color " << chosen.color << "\n\n";

    return true;
}
