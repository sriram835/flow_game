#include "half_split.h"
#include "board.h"
#include "globals.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <queue>
#include <tuple>
#include <unordered_map>
#include <vector>

using namespace std;

unordered_map<int, pair<pair<int, int>, pair<int, int>>> terminalMap;
typedef vector<pair<int,int>> path;
vector<vector<bool>> visited;
vector<path> validPaths;
int counter = 0;

vector<path> solvedPaths; 
int currentPathIndex = -1;

void fillTerminalMap(const Board& board)
{
    terminalMap.clear();
    for (int i = 0; i < GRID; i++) {
        for (int j = 0; j < GRID; j++)
        {
            if (!board.board[i][j].isTerminal || board.board[i][j].color == 0) {
                continue;
            }
            
            if (terminalMap.find(board.board[i][j].color) != terminalMap.end())
            {
                if (terminalMap[board.board[i][j].color].first.first == -1)
                {
                    terminalMap[board.board[i][j].color].first = {i, j};
                }
                else
                {
                    terminalMap[board.board[i][j].color].second = {i, j};
                }
            }
            else
            {
                terminalMap[board.board[i][j].color] = {{-1, -1}, {-1, -1}};
                terminalMap[board.board[i][j].color].first = {i, j};
            }
        }
    }
}

pair<pair<pair<int, int>, pair<int, int>>, pair<pair<int, int>, pair<int, int>>> 
findHalf(const Board& board, pair<int, int> top_left, pair<int, int> bottom_right)
{
    int a = top_left.first;
    int b = top_left.second;

    int c = bottom_right.first;
    int d = bottom_right.second;

    int height = c - a;
    int width = d - b;

    if (height > width)
    {
        return {
            {{a, b}, {(a + c) / 2, d}}, 
            {{(a + c) / 2 + 1, b}, {c, d}}
        };
    }
    else
    {
        return {
            {{a, b}, {c, (b + d) / 2}}, 
            {{a, (b + d) / 2 + 1}, {c, d}}
        };
    }
}

bool isInside(pair<int, int> point, pair<pair<int, int>, pair<int, int>> bounds) {
    int r = point.first;
    int c = point.second;
    return (r >= bounds.first.first && r <= bounds.second.first &&
            c >= bounds.first.second && c <= bounds.second.second);
}

double getDistance(pair<int, int> p1, pair<int, int> p2) {
    return sqrt(pow(p1.first - p2.first, 2) + pow(p1.second - p2.second, 2));
}

path findPathDijkstra(pair<int, int> start, pair<int, int> target, pair<pair<int, int>, pair<int, int>> bounds) {
    priority_queue<pair<int, pair<int, int>>, vector<pair<int, pair<int, int>>>, greater<>> pq;
    
    vector<vector<pair<int, int>>> parent(GRID, vector<pair<int, int>>(GRID));
    vector<vector<int>> dist(GRID, vector<int>(GRID, 1e9));

    pq.push({0, start});
    dist[start.first][start.second] = 0;

    int dr[] = {-1, 1, 0, 0};
    int dc[] = {0, 0, -1, 1};

    while (!pq.empty()) {
        auto [d, curr] = pq.top();
        pq.pop();

        if (curr == target) {
            path p;
            while (curr != start) {
                p.push_back(curr);
                curr = parent[curr.first][curr.second];
            }
            p.push_back(start);
            reverse(p.begin(), p.end());
            return p;
        }

        for (int i = 0; i < 4; i++) {
            pair<int, int> next = {curr.first + dr[i], curr.second + dc[i]};
    
            if (next.first >= 0 && next.first < GRID && next.second >= 0 && next.second < GRID &&
                isInside(next, bounds)) {
            
                if (!visited[next.first][next.second] || next == target) {
                    int newDist = d + 1;
                    if (newDist < dist[next.first][next.second]) {
                        dist[next.first][next.second] = newDist;
                        parent[next.first][next.second] = curr;
                        pq.push({newDist, next});
                    }
                }
            }
        }
    }
    return {}; 
}

void half_split_recursive_solver(const Board& board, pair<int, int> top_left, pair<int,int> bottom_right)
{
    if (top_left.first > bottom_right.first || top_left.second > bottom_right.second) return;
    if (top_left == bottom_right) return; 

    auto [firstHalf, secondHalf] = findHalf(board, top_left, bottom_right);

    // straddling step
    vector<pair<double, int>> straddlingColor;
    for (auto const& [color, terminals] : terminalMap) {
        if ((isInside(terminals.first, firstHalf) && isInside(terminals.second, secondHalf)) ||
            (isInside(terminals.second, firstHalf) && isInside(terminals.first, secondHalf))) {

            if (!visited[terminals.first.first][terminals.first.second]) {
                double dist = getDistance(terminals.first, terminals.second);
                straddlingColor.push_back({dist, color});
            }
        }
    }
    sort(straddlingColor.begin(), straddlingColor.end());

    for (auto& item : straddlingColor) {
        int color = item.second;
        path p = findPathDijkstra(terminalMap[color].first, terminalMap[color].second, {top_left, bottom_right});
        
        if (!p.empty()) {
            validPaths.push_back(p);
            for (auto pos : p) {
                visited[pos.first][pos.second] = true;
            }
        }
    }

    // recursive case
    counter++;
    half_split_recursive_solver(board, firstHalf.first, firstHalf.second);
    half_split_recursive_solver(board, secondHalf.first, secondHalf.second);
}   

vector<pair<int,int>> half_split_algorithm(const Board& board)
{
    if (solvedPaths.empty() && currentPathIndex == -1) {
        
        visited.assign(GRID, vector<bool>(GRID, false));
        validPaths.clear();
        terminalMap.clear();
        counter = 0;
        
        fillTerminalMap(board);
        half_split_recursive_solver(board, {0, 0}, {GRID-1, GRID-1});
        
        solvedPaths = validPaths;
        currentPathIndex = 0;
    }
    
    if (currentPathIndex < solvedPaths.size()) {
        return solvedPaths[currentPathIndex++];
    }
    
    return {};
}