#include "greedy_wrap_algorithm.h"
#include "globals.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <tuple>

using namespace std;

double calculateRadialWeight(int row, int col, int gridSize) {
  int distanceToTop = row;
  int distanceToBottom = gridSize - 1 - row;
  int distanceToLeft = col;
  int distanceToRight = gridSize - 1 - col;

  int distanceToBorder = min({distanceToTop, distanceToBottom, distanceToLeft, distanceToRight});
  
  // Set outermost ring weight to 0
  if (distanceToBorder == 0) {
    return 0.0;
  }
  
  double weight = pow(10.0*GRID, distanceToBorder);

  return weight;
}

vector<pair<int, int>> dijkstraPath(const Board &board, int startRow, int startCol,
                                     int endRow, int endCol) {
  int gridSize = GRID;

  vector<vector<double>> dist(gridSize, vector<double>(gridSize, numeric_limits<double>::max()));
  vector<vector<pair<int, int>>> parent(gridSize, vector<pair<int, int>>(gridSize, {-1, -1}));
  vector<vector<bool>> visited(gridSize, vector<bool>(gridSize, false));

  priority_queue<tuple<double, int, int>, vector<tuple<double, int, int>>, greater<tuple<double, int, int>>> pq;

  dist[startRow][startCol] = 0;
  pq.push({0, startRow, startCol});

  int dr[] = {-1, 1, 0, 0};
  int dc[] = {0, 0, -1, 1};

  while (!pq.empty()) {
    auto [d, row, col] = pq.top();
    pq.pop();

    if (visited[row][col]) continue;
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

      if (nr < 0 || nr >= gridSize || nc < 0 || nc >= gridSize) continue;
      if (visited[nr][nc]) continue;
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

std::vector<std::pair<int, int>> greedyWrapAlgorithm(const Board &board) {
  int gridSize = GRID;
  vector<pair<int, int>> terminals[11]; // Support up to 10 colors

  for (int i = 0; i < gridSize; i++) {
    for (int j = 0; j < gridSize; j++) {
      if (board.board[i][j].isTerminal) {
        int color = board.board[i][j].color;
        terminals[color].push_back({i, j});
      }
    }
  }

  vector<tuple<double, int, int, int, int, int>> unsolved_pairs;

  for (int color = 1; color <= 10; color++) {
    if (terminals[color].size() != 2) continue; 

    int r1 = terminals[color][0].first;
    int c1 = terminals[color][0].second;
    int r2 = terminals[color][1].first;
    int c2 = terminals[color][1].second;

    if (board.board[r1][c1].hasPipe && board.board[r2][c2].hasPipe) {
      continue;
    }
    double dr = r1 - r2;
    double dc = c1 - c2;
    double distance = sqrt(dr * dr + dc * dc);

    unsolved_pairs.push_back({distance, color, r1, c1, r2, c2});
  }

  //sort(unsolved_pairs.begin(), unsolved_pairs.end());
  for (const auto &pair_info : unsolved_pairs) {
    int r1 = get<2>(pair_info);
    int c1 = get<3>(pair_info);
    int r2 = get<4>(pair_info);
    int c2 = get<5>(pair_info);
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
