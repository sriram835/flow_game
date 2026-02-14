#include "solver.h"
#include "board.h"
#include "globals.h"

namespace fs = std::filesystem;

using namespace std;

void solver(Board board, string file_name) {

  // Base case
  if (isBoardFull(board)) {
    if (isValidBoard(board)) {
      cout << "Got a valid board";
      saveBoard(board, file_name);
    }
    return;
  }

  // 1. Find ONE empty, non-terminal cell
  int er = -1, ec = -1;
  for (int r = 0; r < GRID; r++) {
    for (int c = 0; c < GRID; c++) {
      const Cell &cell = board.board[r][c];
      if (!cell.isTerminal && !cell.hasPipe) {
        er = r;
        ec = c;
        break;
      }
    }
    if (er != -1)
      break;
  }

  // Sanity
  if (er == -1)
    return;

  // 2. Try placing each terminal color
  vector<int> colors;
  for (int r = 0; r < GRID; r++)
    for (int c = 0; c < GRID; c++)
      if (board.board[r][c].isTerminal)
        colors.push_back(board.board[r][c].color);

  sort(colors.begin(), colors.end());
  colors.erase(unique(colors.begin(), colors.end()), colors.end());

  for (int color : colors) {

    // Fill
    Board next = board;
    Cell &cell = next.board[er][ec];
    cell.hasPipe = true;
    cell.color = color;

    // Recurse
    solver(next, file_name);
  }
}
void saveBoard(const Board &board, const string &file_name) {
  // ---- 1. Create base path: project_root/solution/file_name/ ----
  fs::path base = fs::current_path() / "solution" / file_name;

  // Create folders if missing
  if (!fs::exists(base)) {
    fs::create_directories(base);
  }

  // ---- 2. Pick next available sol_X filename ----
  int index = 1;
  fs::path outfile;

  while (true) {
    outfile = base / ("sol_" + to_string(index));
    if (!fs::exists(outfile))
      break;
    index++;
  }

  // ---- 3. Open file for writing ----
  ofstream out(outfile);
  if (!out.is_open()) {
    cerr << "Error: Could not open file for writing: " << outfile << endl;
    return;
  }

  // ---- 4. Write board ----
  for (int r = 0; r < GRID; r++) {
    for (int c = 0; c < GRID; c++) {
      const Cell &cell = board.board[r][c];

      int value = 0;

      if (cell.isTerminal) {
        value = cell.color;
      } else if (cell.hasPipe) {
        value = cell.color * 2;
      }

      out << value;

      if (c != GRID - 1)
        out << ",";
    }
    if (r != GRID - 1)
      out << "\n";
  }

  out.close();
}

vector<int> getAvailableTerminals(const Board &board) {
  vector<int> colors;
  for (int row = 0; row < board.board.size(); row++) {
    for (int col = 0; col < board.board.size(); col++) {
      const Cell &cell = board.board[row][col];
      if (cell.hasPipe == false && cell.isTerminal == true) {
        colors.push_back(cell.color);
      }
    }
  }

  return colors;
}

vector<pair<int, int>> getEmptySpaces(const Board &board) {
  vector<pair<int, int>> indexes;
  for (int row = 0; row < board.board.size(); row++) {
    for (int col = 0; col < board.board.size(); col++) {
      const Cell &cell = board.board[row][col];
      if (cell.hasPipe == false) {
        indexes.push_back({row, col});
      }
    }
  }
  return indexes;
}

bool isBoardFull(const Board &board) {
  for (int row = 0; row < board.board.size(); row++) {
    for (int col = 0; col < board.board.size(); col++) {
      if (board.board[row][col].hasPipe == false) {
        return false;
      }
    }
  }
  return true;
}

bool isValidBoard(const Board &board) {
  unordered_map<int, vector<pair<int, int>>> terminals;

  // 1. Collect terminals & ensure board is full
  for (int r = 0; r < GRID; r++) {
    for (int c = 0; c < GRID; c++) {
      const Cell &cell = board.board[r][c];

      if (cell.isTerminal) {
        terminals[cell.color].push_back({r, c});
      }
      if (cell.color == 0) {
        cout << "color 0\n";
        return false;
      }
      if (cell.hasPipe == false) {
        cout << "Empty hasPipe\n";
        return false;
      }
    }
  }

  // 3. Validate each color's pipe via BFS/DFS
  const int dr[4] = {-1, 1, 0, 0};
  const int dc[4] = {0, 0, -1, 1};

  for (auto &[color, term] : terminals) {
    auto [sr, sc] = term[0];
    auto [er, ec] = term[1];

    // BFS through pipe of same color
    vector<vector<bool>> visited(GRID, vector<bool>(GRID, false));
    queue<pair<int, int>> q;

    q.push({sr, sc});
    visited[sr][sc] = true;

    int reached_terminals = 0;

    while (!q.empty()) {
      auto [r, c] = q.front();
      q.pop();
      const Cell &cell = board.board[r][c];

      if (cell.isTerminal && cell.color == color)
        reached_terminals++;

      int degree = 0; // neighbors with same color

      for (int i = 0; i < 4; i++) {
        int nr = r + dr[i];
        int nc = c + dc[i];

        if (nr < 0 || nr >= GRID || nc < 0 || nc >= GRID)
          continue;

        const Cell &ncCell = board.board[nr][nc];

        if (ncCell.color == color) {
          degree++;

          if (!visited[nr][nc]) {
            visited[nr][nc] = true;
            q.push({nr, nc});
          }
        }
      }

      // Degree rules:
      if (cell.isTerminal) {
        if (degree != 1) {
          cout << "Ternimal degree is not 1\n";
          return false; // terminals connect to 1 pipe cell
        }
      } else {
        if (degree != 2) {
          cout << "Cell degree is not 2\n";
          return false; // pipe cells must have 2 neighbors
        }
      }
    }

    // Should have exactly 2 terminals reachable
    if (reached_terminals != 2) {
      cout << "did not reach terminals\n";
      return false;
    }
  }

  return true; // All colors validated
}
