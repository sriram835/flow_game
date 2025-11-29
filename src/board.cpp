#include "board.h"

bool Board::makeMove(vector<pair<int, int>> indexes) {
  for (auto path : saved_paths) {
    for (auto index : path) {
      cout << index.first << " " << index.second << "\t";
    }
    cout << "\n";
  }
  if (!isValidPath(indexes)) {
    cout << "invalid path\n";

    return false;
  }

  cout << "valid path\n";

  saved_paths.push_back(indexes);
  int n = indexes.size();
  auto start = indexes[0];
  auto end = indexes[n - 1];

  int start_row = start.first;
  int start_col = start.second;
  int end_row = end.first;
  int end_col = end.second;

  int color = board[start_row][start_col].color; // same as end’s color

  for (int i = 0; i < n; ++i) {
    int row = indexes[i].first;
    int col = indexes[i].second;

    Cell &c = board[row][col];

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
  if (n <= 1) {
    return false;
  }

  auto start = indexes[0];
  auto end = indexes[n - 1];

  int start_row = start.first;
  int start_col = start.second;
  int end_row = end.first;
  int end_col = end.second;

  // Bounds check for first and last
  if (start_row < 0 || start_row >= GRID || start_col < 0 || start_col >= GRID)
    return false;
  if (end_row < 0 || end_row >= GRID || end_col < 0 || end_col >= GRID)
    return false;

  Cell start_cell = board[start_row][start_col];
  Cell end_cell = board[end_row][end_col];

  // Endpoints must be terminals of same color
  if (!start_cell.isTerminal || !end_cell.isTerminal) {
    cout << "Not terminal\n";
    return false;
  }
  if (start_cell.color != end_cell.color) {
    cout << "not same color\n";
    return false;
  }
  int color = start_cell.color;

  // Track visited cells to forbid revisits
  std::vector<std::vector<bool>> visited(GRID, std::vector<bool>(GRID, false));

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

    if (c.hasPipe) {
      cout << "Already filled\n";
      return false;
    }

    // Start/end already checked; intermediates must be empty
    if (i != 0 && i != n - 1) {
      if (c.color != 0 || c.isTerminal) {
        cout << "not empty in middle\n";
        return false;
      }
    }

    // Intermediate cells must not be terminals
    if (i != 0 && i != n - 1 && c.isTerminal) {

      cout << "Terminal in middle\n";
      return false;
    }

    // Check adjacency between consecutive cells
    if (i > 0) {
      int px = indexes[i - 1].first;
      int py = indexes[i - 1].second;
      int dx = abs(x - px);
      int dy = abs(y - py);

      // Must move exactly one step in manhattan distance
      if (!((dx == 1 && dy == 0) || (dx == 0 && dy == 1))) {
        cout << "Not adjancent\n";
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

  file.close();
  return true;
}

bool Board::removePath(vector<pair<int, int>> path) {
  for (int i = 0; i < path.size(); i++) {

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
