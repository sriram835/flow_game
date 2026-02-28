#include "game_algorithms.h"
#include <unistd.h>

using namespace std;
bool isCompleted(const Board &board);
vector<pair<int, int>> algorithm(const Board board) { return {}; }
unordered_map<int, vector<pair<int, int>>> paths;

bool canVisit(const Board &board, int x, int y, int color,
              vector<vector<bool>> &visited) {

  if (x < 0 || y < 0 || x >= GRID || y >= GRID)
    return false;

  if (visited[x][y])
    return false;

  // If it's a terminal of another color → block
  if (board.board.at(x).at(y).isTerminal &&
      board.board.at(x).at(y).color != color)
    return false;

  return true;
}

int dx[4] = {-1, 1, 0, 0};
int dy[4] = {0, 0, -1, 1};
bool isCompletedVisited(const Board &board, vector<vector<bool>> &visited) {

  if (isCompleted(board) == false) {
    return false;
  }

  for (int i = 0; i < GRID; i++)
    for (int j = 0; j < GRID; j++)
      if (!visited[i][j])
        return false;

  return true;
}

pair<pair<int, int>, pair<int, int>> findTerminals(const Board &board,
                                                   int color) {
  pair<int, int> first = {-1, -1}, second = {-1, -1};

  for (int i = 0; i < GRID; i++) {
    for (int j = 0; j < GRID; j++) {
      if (board.board.at(i).at(j).isTerminal &&
          board.board.at(i).at(j).color == color) {

        if (first.first == -1)
          first = {i, j};
        else
          second = {i, j};
      }
    }
  }

  return {first, second};
}
bool feasibleCheck(const Board &board, const vector<vector<bool>> &visited) {
  for (int i = 0; i < GRID; i++) {
    for (int j = 0; j < GRID; j++) {
      if (visited[i][j])
        continue;

      int freeNeighbors = 0;
      int lastNi = -1;
      int lastNj = -1;

      // Check all 4 directions
      for (int d = 0; d < 4; d++) {
        int ni = i + dx[d];
        int nj = j + dy[d];

        if (ni >= 0 && ni < GRID && nj >= 0 && nj < GRID) {
          // A neighbor is "free" if it hasn't been visited yet
          if (!visited[ni][nj]) {
            freeNeighbors++;
            lastNi = ni;
            lastNj = nj;
          }
        }
      }
      if (freeNeighbors < 1) {
        return false;
      }

      if (freeNeighbors == 1) {
        if (board.board[i][j].isTerminal &&
            board.board[lastNi][lastNj].isTerminal &&
            board.board[i][j].color != board.board[lastNi][lastNj].color) {
          return false;
        }
        if (board.board[i][j].isTerminal == false) {
          if (board.board[lastNi][lastNj].isTerminal) {
            return false;
          }
        }
      }
    }
  }
  return true;
}

bool hasDeadRegion(const Board &board, const vector<vector<bool>> &visited,
                   int currentColor) {

  vector<vector<bool>> seen(GRID, vector<bool>(GRID, false));

  for (int i = 0; i < GRID; i++) {
    for (int j = 0; j < GRID; j++) {

      if (visited[i][j] || seen[i][j])
        continue;

      // Found a new empty region
      queue<pair<int, int>> q;
      q.push({i, j});
      seen[i][j] = true;

      unordered_map<int, int> terminalCount;
      int regionSize = 0;
      bool isDeadRegion = true;

      while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();
        regionSize++;

        if (board.board[x][y].isTerminal) {
          int color = board.board[x][y].color;
          isDeadRegion = false;
          if (color != currentColor) {
            terminalCount[color]++;
          }
        }

        for (int d = 0; d < 4; d++) {
          int nx = x + dx[d];
          int ny = y + dy[d];

          if (nx >= 0 && nx < GRID && ny >= 0 && ny < GRID &&
              !visited[nx][ny] && !seen[nx][ny]) {

            seen[nx][ny] = true;
            q.push({nx, ny});
          }
        }
      }
      for (auto &[color, count] : terminalCount) {
        if (count == 1)
          return true; // one terminal trapped alone
      }
      if (isDeadRegion) {
        return true;
      }
    }
  }

  return false;
}

bool solver(Board &board, int index, vector<vector<bool>> &visited,
            vector<int> &colors) {
  //   cout << "Solver started: " << colors[index] << "\n";
  if (index == colors.size())
    return isCompletedVisited(board, visited);

  int color = colors[index];

  auto terminals = findTerminals(board, color);
  auto start = terminals.first;
  auto end = terminals.second;
  //   cout << "Find terminals ended\n";
  if (start.first == -1 || end.first == -1) {
    //     cout << "Invalid terminals for color " << color << endl;
    return false;
  }

  visited[start.first][start.second] = true;
  paths[color].push_back({start.first, start.second});

  bool result = dfsColor(board, start.first, start.second, end.first,
                         end.second, color, index, visited, colors);

  //   cout << "Solver ended: " << color << "\n";
  if (!result) {
    visited[start.first][start.second] = false;
    paths[color].pop_back();
  }

  return result;
}
bool dfsColor(Board &board, int x, int y, int tx, int ty, int color,
              int colorIndex, vector<vector<bool>> &visited,
              vector<int> &colors) {
  //   cout << "DFS called\n";
  //
  //
  //

  if (x == tx && y == ty) {

    {
      lock_guard<mutex> lock(boardMutex);
      board.makeMove(paths[color]);
    }
    //     cout << "Made one move\n";
    if (feasibleCheck(board, visited)) {
      if (solver(board, colorIndex + 1, visited, colors))
        return true;
    }

    lock_guard<mutex> lock(boardMutex);
    board.undoMove();

    return false;
  }

  for (int d = 0; d < 4; d++) {

    int nx = x + dx[d];
    int ny = y + dy[d];

    if (!canVisit(board, nx, ny, color, visited))
      continue;

    visited[nx][ny] = true;
    paths[color].push_back({nx, ny});

    if (!hasDeadRegion(board, visited, color)) {
      if (dfsColor(board, nx, ny, tx, ty, color, colorIndex, visited, colors))
        return true;
    }
    paths[color].pop_back();
    visited[nx][ny] = false;
  }

  return false;
}

bool isCompleted(const Board &board) {
  for (int i = 0; i < GRID; i++) {
    for (int j = 0; j < GRID; j++) {
      if (board.board.at(i).at(j).hasPipe == false) {
        return false;
      }
    }
  }

  for (int i = 0; i < GRID; i++) {
    for (int j = 0; j < GRID; j++) {
      if (board.board.at(i).at(j).isTerminal == true &&
          board.board.at(i).at(j).hasPipe == false) {
        return false;
      }
    }
  }

  return true;
}
