#include "board.h"

#include <cmath>
#include <fstream>
#include <iostream>
#include <sstream>

bool Board::makeMove(const std::vector<std::pair<int, int>> &indexes) {
  for (const auto &path : saved_paths) {
    for (const auto &index : path) {
      std::cout << index.first << " " << index.second << "\t";
    }
    std::cout << "\n";
  }

  if (!isValidPath(indexes)) {
    std::cout << "invalid path\n";
    return false;
  }

  std::cout << "valid path\n";

  saved_paths.push_back(indexes);
  int n = static_cast<int>(indexes.size());
  auto start = indexes[0];

  int start_row = start.first;
  int start_col = start.second;

  int color = board[start_row][start_col].color;

  for (int i = 0; i < n; ++i) {
    int row = indexes[i].first;
    int col = indexes[i].second;

    Cell &c = board[row][col];
    c.hasPipe = true;

    if (i != 0 && i != n - 1) {
      c.color = color;
    }
  }

  return true;
}

bool Board::isValidPath(const std::vector<std::pair<int, int>> &path) const {
  int n = static_cast<int>(path.size());
  if (n <= 1) {
    return false;
  }

  auto start = path[0];
  auto end = path[n - 1];

  int start_row = start.first;
  int start_col = start.second;
  int end_row = end.first;
  int end_col = end.second;

  if (start_row < 0 || start_row >= N || start_col < 0 || start_col >= N) {
    return false;
  }
  if (end_row < 0 || end_row >= N || end_col < 0 || end_col >= N) {
    return false;
  }

  Cell start_cell = board[start_row][start_col];
  Cell end_cell = board[end_row][end_col];

  if (!start_cell.isTerminal || !end_cell.isTerminal) {
    std::cout << "Not terminal\n";
    return false;
  }
  if (start_cell.color != end_cell.color) {
    std::cout << "not same color\n";
    return false;
  }

  std::vector<std::vector<bool>> visited(N, std::vector<bool>(N, false));

  for (int i = 0; i < n; ++i) {
    int x = path[i].first;
    int y = path[i].second;

    if (x < 0 || x >= N || y < 0 || y >= N) {
      return false;
    }

    if (visited[x][y]) {
      return false;
    }
    visited[x][y] = true;

    Cell c = board[x][y];

    if (c.hasPipe) {
      std::cout << "Already filled\n";
      return false;
    }

    if (i != 0 && i != n - 1) {
      if (c.color != 0 || c.isTerminal) {
        std::cout << "not empty in middle\n";
        return false;
      }
    }

    if (i > 0) {
      int px = path[i - 1].first;
      int py = path[i - 1].second;
      int dx = std::abs(x - px);
      int dy = std::abs(y - py);

      if (!((dx == 1 && dy == 0) || (dx == 0 && dy == 1))) {
        std::cout << "Not adjacent\n";
        return false;
      }
    }
  }

  return true;
}

bool Board::loadFromFile(const std::string &filename) {
  std::ifstream file(filename);
  if (!file.is_open()) {
    std::cout << "Error: Cannot open level file: " << filename << std::endl;
    return false;
  }

  std::string line;
  int row = 0;

  while (std::getline(file, line) && row < N) {
    std::stringstream ss(line);
    std::string value;
    int col = 0;

    while (std::getline(ss, value, ',') && col < N) {
      int val = std::stoi(value);

      Cell &c = board[row][col];
      c.color = 0;
      c.isTerminal = false;
      c.hasPipe = false;

      if (val != 0) {
        c.isTerminal = true;
        c.color = val;
      }

      col++;
    }

    row++;
  }

  return true;
}

bool Board::removePath(const std::vector<std::pair<int, int>> &path) {
  for (int i = 0; i < static_cast<int>(path.size()); i++) {
    int row = path[i].first;
    int col = path[i].second;
    Cell &c = board[row][col];
    if (!c.isTerminal) {
      c.color = 0;
    }
    c.hasPipe = false;
  }
  return true;
}

void Board::undoMove() {
  if (saved_paths.empty()) {
    return;
  }

  auto path = saved_paths.back();
  bool res = removePath(path);
  if (res) {
    saved_paths.pop_back();
  }
}

void Board::resetBoard() {
  while (!saved_paths.empty()) {
    undoMove();
  }
}
