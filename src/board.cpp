#include "board.h"

bool Board::makeMove(vector<pair<int, int>> indexes) {
  if (!isValidPath(indexes))
    return false;

  int n = indexes.size();
  auto start = indexes[0];
  auto end = indexes[n - 1];

  int start_x = start.first;
  int start_y = start.second;
  int end_x = end.first;
  int end_y = end.second;

  int color = board[start_x][start_y].color; // same as end’s color

  for (int i = 0; i < n; ++i) {
    int x = indexes[i].first;
    int y = indexes[i].second;

    Cell &c = board[x][y];

    // Mark that this cell now has a pipe
    c.hasPipe = true;

    // For intermediate cells, set the color to the path color
    if (i != 0 && i != n - 1) {
      c.color = color;
    }
  }

  return true;
}

bool Board::isValidPath(vector<pair<int, int>> indexes) {
  int n = indexes.size();
  if (n <= 1)
    return false;

  auto start = indexes[0];
  auto end = indexes[n - 1];

  int start_x = start.first;
  int start_y = start.second;
  int end_x = end.first;
  int end_y = end.second;

  // Bounds check for first and last
  if (start_x < 0 || start_x >= GRID || start_y < 0 || start_y >= GRID)
    return false;
  if (end_x < 0 || end_x >= GRID || end_y < 0 || end_y >= GRID)
    return false;

  Cell start_cell = board[start_x][start_y];
  Cell end_cell = board[end_x][end_y];

  // Endpoints must be terminals of same color
  if (!start_cell.isTerminal || !end_cell.isTerminal)
    return false;
  if (start_cell.color != end_cell.color)
    return false;
  int color = start_cell.color;

  // Track visited cells to forbid revisits
  bool visited[GRID][GRID] = {false};

  for (int i = 0; i < n; ++i) {
    int x = indexes[i].first;
    int y = indexes[i].second;

    // Bounds check
    if (x < 0 || x >= GRID || y < 0 || y >= GRID)
      return false;

    // No revisits
    if (visited[x][y])
      return false;
    visited[x][y] = true;

    Cell c = board[x][y];

    // Start/end already checked; intermediates must be empty
    if (i != 0 && i != n - 1) {
      if (c.color != 0 || c.isTerminal)
        return false;
    }

    // Intermediate cells must not be terminals
    if (i != 0 && i != n - 1 && c.isTerminal)
      return false;

    // Check adjacency between consecutive cells
    if (i > 0) {
      int px = indexes[i - 1].first;
      int py = indexes[i - 1].second;
      int dx = abs(x - px);
      int dy = abs(y - py);

      // Must move exactly one step in manhattan distance
      if (!((dx == 1 && dy == 0) || (dx == 0 && dy == 1))) {
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

    while (std::getline(file, line) && row < GRID) {
        std::stringstream ss(line);
        std::string value;
        int col = 0;

        while (std::getline(ss, value, ',') && col < GRID) {
            int val = std::stoi(value);

            Cell &c = board[col][row];

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

    file.close();
    return true;
}
