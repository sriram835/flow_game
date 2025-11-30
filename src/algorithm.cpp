#include "board.h"
#include "game_algorithms.h"
#include "globals.h"

using namespace std;

vector<pair<int, int>> bfs(Board board) {
  vector<pair<int, int>> path;
  vector<vector<int>> terminals = getTerminals(board);
  if (terminals.empty()) {
    return {};
  }

  int terminal_choice = randomInt(terminals.size());
  int color = terminals[terminal_choice][0];
  int start_row = terminals[terminal_choice][1];
  int start_col = terminals[terminal_choice][2];

  queue<pair<pair<int, int>, pair<int, int>>> que;
  pair<int, int> index;
  index.first = start_row;
  index.second = start_col;
  que.push({index, {-1, -1}});
  std::vector<std::vector<bool>> visited(GRID, std::vector<bool>(GRID, false));

  vector<vector<pair<int, int>>> parents(
      GRID, vector<pair<int, int>>(GRID, {-5, -5}));

  int end_row = -1;
  int end_col = -1;

  int terminal_count = 0;

  while (!que.empty()) {
    auto index_par = que.front();
    que.pop();
    auto index = index_par.first;
    auto par = index_par.second;

    visited[index.first][index.second] = true;
    parents[index.first][index.second] = par;

    if (board.board[index.first][index.second].isTerminal) {
      terminal_count++;
    }

    if (terminal_count == 2) {
      end_row = index.first;
      end_col = index.second;
      break;
    }

    vector<pair<int, int>> neighbors =
        getNeighbors(color, index.first, index.second, board, visited);
    for (int i = 0; i < neighbors.size(); i++) {
      que.push({neighbors[i], index});
    }
  }

  if (end_row == -1 || end_col == -1) {
    return {};
  }

  path = reconstructPath(end_row, end_col, parents);
  return path;
}

vector<pair<int, int>>
reconstructPath(int end_row, int end_col,
                const vector<vector<pair<int, int>>> &parents) {
  vector<pair<int, int>> path;

  int r = end_row;
  int c = end_col;

  while (r != -1 && c != -1) {
    path.push_back({r, c});
    auto p = parents[r][c];
    r = p.first;
    c = p.second;
  }

  reverse(path.begin(), path.end());
  return path;
}

vector<pair<int, int>> getNeighbors(int color, int row, int col,
                                    const Board &board,
                                    const vector<vector<bool>> &visited) {
  vector<pair<int, int>> valid_neighbors;
  if (row - 1 > -1 && visited[row - 1][col] == false &&
      board.board[row - 1][col].hasPipe == false &&
      !(board.board[row - 1][col].isTerminal == true &&
        board.board[row - 1][col].color != color)) {
    valid_neighbors.push_back({row - 1, col});
  }

  if (col - 1 > -1 && visited[row][col - 1] == false &&
      board.board[row][col - 1].hasPipe == false &&
      !(board.board[row][col - 1].isTerminal == true &&
        board.board[row][col - 1].color != color)) {
    valid_neighbors.push_back({row, col - 1});
  }

  if (col + 1 < GRID && visited[row][col + 1] == false &&
      board.board[row][col + 1].hasPipe == false &&
      !(board.board[row][col + 1].isTerminal == true &&
        board.board[row][col + 1].color != color)) {
    valid_neighbors.push_back({row, col + 1});
  }

  if (row + 1 < GRID && visited[row + 1][col] == false &&
      board.board[row + 1][col].hasPipe == false &&
      !(board.board[row + 1][col].isTerminal == true &&
        board.board[row + 1][col].color != color)) {
    valid_neighbors.push_back({row + 1, col});
  }

  return valid_neighbors;
}

int randomInt(int upper_bound) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(0, upper_bound - 1);
  return dist(gen);
}

/*
vector<vector<int>> getTerminals(Board board) {
  vector<vector<int>> colors;

  for (int row = 0; row < GRID; row++) {
    for (int col = 0; col < GRID; col++) {
      int color = board.board[row][col].color;
      if (color != 0 && !colorAlreadyAdded(color, colors)) {
        colors.push_back(vector{color, row, col});
      }
    }
  }

  return colors;
}
*/

bool terminalsReachable(const Board &board, int color, int sr, int sc, int er,
                        int ec, vector<vector<bool>> visited) {

  if (visited[sr][sc] == true) {
    return false;
  }
	if (sr == er && sc == ec){
		return true;
	}
  visited[sr][sc] = true;
  vector<pair<int, int>> neighbors =
      getNeighbors(color, sr, sc, board, visited);

	for (int i = 0; i < neighbors.size();i++){
		auto index = neighbors[i];
		if (terminalsReachable(board,color,index.first,index.second,er,ec,visited)){
			return true;
		}
	}
	return false;
}

vector<vector<int>> getTerminals(Board board) {
  // Map from color → list of terminal coordinates
  unordered_map<int, vector<pair<int, int>>> colorGroups;

  // Gather all terminals
  for (int r = 0; r < GRID; r++) {
    for (int c = 0; c < GRID; c++) {
      int col = board.board[r][c].color;
      if (col != 0 && board.board[r][c].isTerminal &&
          board.board[r][c].hasPipe == false) {
        colorGroups[col].push_back({r, c});
      }
    }
  }

  vector<vector<int>> result;

  // For each color, check if they are reachable
  for (auto &[color, terms] : colorGroups) {

    if (terms.size() != 2)
      continue; // malformed board

    auto [r1, c1] = terms[0];
    auto [r2, c2] = terms[1];
    std::vector<std::vector<bool>> visited(GRID,
                                           std::vector<bool>(GRID, false));

    // 2. Check if reachable by BFS
    if (terminalsReachable(board, color, r1, c1, r2, c2, visited)) {
      result.push_back({color, r1, c1}); // same format as before
    }
  }

  return result;
}

bool colorAlreadyAdded(int color, vector<vector<int>> colors) {
  for (int i = 0; i < colors.size(); i++) {
    if (color == colors[i][0]) {
      return true;
    }
  }
  return false;
}
