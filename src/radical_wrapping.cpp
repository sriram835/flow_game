#include "radical_wrapping.h"
#include "board.h"
#include "globals.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <tuple>

using namespace std;

double radical_wrapping::calculateRadialWeight(int row, int col, int gridSize) {
  int distanceToTop = row;
  int distanceToBottom = gridSize - 1 - row;
  int distanceToLeft = col;
  int distanceToRight = gridSize - 1 - col;

  int distanceToBorder =
      min({distanceToTop, distanceToBottom, distanceToLeft, distanceToRight});

  //  outermost ring weight to 0
  if (distanceToBorder == 0) {
    return 0.0;
  }

  double weight = pow(10.0 * GRID, distanceToBorder);

  return weight;
}

vector<pair<int, int>> radical_wrapping::dijkstraPath(const Board &board,
                                                      int startRow,
                                                      int startCol, int endRow,
                                                      int endCol) {
  int gridSize = GRID;

  vector<vector<double>> dist(
      gridSize, vector<double>(gridSize, numeric_limits<double>::max()));
  vector<vector<pair<int, int>>> parent(
      gridSize, vector<pair<int, int>>(gridSize, {-1, -1}));
  vector<vector<bool>> visited(gridSize, vector<bool>(gridSize, false));

  priority_queue<tuple<double, int, int>, vector<tuple<double, int, int>>,
                 greater<tuple<double, int, int>>>
      pq;

  dist[startRow][startCol] = 0;
  pq.push({0, startRow, startCol});

  int dr[] = {-1, 1, 0, 0};
  int dc[] = {0, 0, -1, 1};

  while (!pq.empty()) {
    auto [d, row, col] = pq.top();
    pq.pop();

    if (visited[row][col])
      continue;
    visited[row][col] = true;

    if (row == endRow && col == endCol) {
      vector<pair<int, int>> path;
      int r = endRow, c = endCol;

      while (r != -1 && c != -1) {
        path.push_back({r, c});
        auto p = parent[r][c];
        r = p.first;
        c = p.second;
      }

      reverse(path.begin(), path.end());

      cout << "Path found with total weight: " << dist[endRow][endCol] << endl;

      return path;
    }

    for (int i = 0; i < 4; i++) {
      int nr = row + dr[i];
      int nc = col + dc[i];

      if (nr < 0 || nr >= gridSize || nc < 0 || nc >= gridSize)
        continue;
      if (visited[nr][nc])
        continue;
      Cell cell = board.board[nr][nc];
      if (cell.hasPipe && !(nr == endRow && nc == endCol)) {
        continue;
      }

      if (cell.isTerminal && (nr != endRow || nc != endCol)) {
        continue;
      }

      double weight = calculateRadialWeight(nr, nc, gridSize);
      // Terminals have 0 cost to enter since they're endpoints
      if (cell.isTerminal) {
        weight = 0;
      }
      double newDist = dist[row][col] + weight;

      if (newDist < dist[nr][nc]) {
        dist[nr][nc] = newDist;
        parent[nr][nc] = {row, col};
        pq.push({newDist, nr, nc});
      }
    }
  }

  return vector<pair<int, int>>();
}

std::vector<std::pair<int, int>>
radical_wrapping::algorithm(const Board &board) {
  int gridSize = GRID;

  vector<pair<pair<int, int>, pair<int, int>>> terminals =
      get_terminals_obj->getTerminals(board);

  // sort(unsolved_pairs.begin(), unsolved_pairs.end());
  for (const auto &pair_info : terminals) {
    auto start_index = pair_info.first;
    auto end_index = pair_info.second;

    int r1 = start_index.first;
    int c1 = start_index.second;

    int r2 = end_index.first;
    int c2 = end_index.second;
    vector<pair<int, int>> path = dijkstraPath(board, r1, c1, r2, c2);

    if (!path.empty()) {
      return path;
    }
    path = dijkstraPath(board, r2, c2, r1, c1);
    if (!path.empty()) {
      return path;
    }
  }
  return vector<pair<int, int>>();
}
