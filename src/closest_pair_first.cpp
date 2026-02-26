#include "closest_pair_first.h"
#include "globals.h"

vector<pair<pair<int, int>, pair<int, int>>>
closest_pair_first::getTerminals(Board board) {
  unordered_map<int, pair<pair<int, int>, pair<int, int>>> color_terminals =
      get_color_terminals(board);
  map<int, vector<int>> distance_color = get_distance_color(color_terminals);

  vector<pair<pair<int, int>, pair<int, int>>> results;
  for (const auto &[distance, colors] : distance_color) {
    for (int color : colors) {
      results.push_back(color_terminals[color]);
    }
  }

  return results;
}

unordered_map<int, pair<pair<int, int>, pair<int, int>>>
closest_pair_first::get_color_terminals(const Board &board) {

  unordered_map<int, pair<pair<int, int>, pair<int, int>>> terminals;

  for (int i = 0; i < board.board.size(); i++) {
    for (int j = 0; j < board.board[i].size(); j++) {

      const auto &cell = board.board[i][j];

      if (cell.isTerminal && !cell.hasPipe) {

        auto it = terminals.find(cell.color);

        if (it == terminals.end()) {
          // first terminal of this color
          terminals.emplace(cell.color,
                            make_pair(make_pair(i, j), make_pair(-1, -1)));
        } else {
          // second terminal of this color
          it->second.second = make_pair(i, j);
        }
      }
    }
  }

  return terminals;
}

int closest_pair_first::distance_manhattan(const pair<int, int> &a,
                                           const pair<int, int> &b) {
  return abs(a.first - b.first) + abs(a.second - b.second);
}

map<int, vector<int>> closest_pair_first::get_distance_color(
    unordered_map<int, pair<pair<int, int>, pair<int, int>>> terminals) {
  map<int, vector<int>> distances_colors;

  for (const auto &[color, terminalPair] : terminals) {

    const auto &t1 = terminalPair.first;
    const auto &t2 = terminalPair.second;

    if (t2.first == -1)
      continue;

    int dist = distance_manhattan(t1, t2);

    distances_colors[dist].push_back(color);
  }

  for (const auto &[dist, colors] : distances_colors) {
    cout << "FROM DISTANCES: Distance " << dist << ": ";
    for (int c : colors) {
      cout << c << " ";
    }
    cout << '\n';
  }

  return distances_colors;
}
