#include "game_algorithms.h"

using namespace std;

vector<pair<int, int>> algorithm(const Board board) {
  unordered_map<int, pair<pair<int, int>, pair<int, int>>> terminals =
      getTerminals(board);

  map<int, vector<int>> distances_colors = getDistanceColor(terminals);

  // For terminals
  /*
for (const auto &entry : terminals) {
int color = entry.first;

auto first = entry.second.first;
auto second = entry.second.second;

cout << "Color " << color << ": ";
cout << "(" << first.first << ", " << first.second << ") ";
cout << "(" << second.first << ", " << second.second << ")";
cout << '\n';
}
  */
  for (const auto &entry : distances_colors) {
    int distance = entry.first;
    const vector<int> &colors = entry.second;

    cout << "Distance " << distance << ": ";
    for (int c : colors) {
      cout << c << " ";
    }
    cout << '\n';

    for (int c : colors) {
      auto start_index = terminals[c].first;
      auto end_index = terminals[c].second;
      vector<vector<int>> region = getRegion(start_index, end_index, board);

      vector<pair<int, int>> path =
          dfsFindPath(start_index, end_index, board, region);

      if (!path.empty()) {

        cout << "PATH\n";
        for (int i = 0; i < path.size(); i++) {
          cout << path[i].first << " " << path[i].second;
          cout << "\n";
        }
        return path;
      }
    }
  }

  return {};
}
bool pointOnSegment(double xi, double yi, double xj, double yj, double px,
                    double py) {
  const double EPS = 1e-9;

  // cross product == 0 → collinear
  double cross = (px - xi) * (yj - yi) - (py - yi) * (xj - xi);
  if (fabs(cross) > EPS)
    return false;

  // check bounding box
  if (px < min(xi, xj) - EPS || px > max(xi, xj) + EPS)
    return false;
  if (py < min(yi, yj) - EPS || py > max(yi, yj) + EPS)
    return false;

  return true;
}
unordered_map<int, pair<pair<int, int>, pair<int, int>>>
getTerminals(const Board &board) {

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

int distanceBetween(const pair<int, int> &a, const pair<int, int> &b) {
  return abs(a.first - b.first) + abs(a.second - b.second);
}

map<int, vector<int>> getDistanceColor(
    unordered_map<int, pair<pair<int, int>, pair<int, int>>> terminals) {
  map<int, vector<int>> distances_colors;

  for (const auto &[color, terminalPair] : terminals) {

    const auto &t1 = terminalPair.first;
    const auto &t2 = terminalPair.second;

    // Safety: ensure both terminals exist
    if (t2.first == -1)
      continue;

    int dist = distanceBetween(t1, t2);

    distances_colors[dist].push_back(color);
  }

  // Debug print (optional)
  for (const auto &[dist, colors] : distances_colors) {
    cout << "FROM DISTANCES: Distance " << dist << ": ";
    for (int c : colors) {
      cout << c << " ";
    }
    cout << '\n';
  }

  return distances_colors;
}

vector<vector<int>> getRegion(pair<int, int> start_index,
                              pair<int, int> end_index, const Board board) {
  int n = board.board.size();
  if (n == 0) {
    return {};
  }

  int m = board.board[0].size();

  vector<vector<int>> region = vector(n, vector(m, 0));
  int orn_x1 = start_index.second;
  int orn_y1 = n - start_index.first - 1;

  int orn_x2 = end_index.second;
  int orn_y2 = n - end_index.first - 1;

  cout << "ORN X1: " << orn_x1 << "\n";
  cout << "ORN Y1: " << orn_y1 << "\n";
  cout << "ORN X2: " << orn_x2 << "\n";
  cout << "ORN Y2: " << orn_y2 << "\n";

  // Center point
  double cx = (orn_x1 + orn_x2) / 2.0;
  double cy = (orn_y1 + orn_y2) / 2.0;

  cout << "CX: " << cx << "\n";
  cout << "CY: " << cy << "\n";

  // Direction of original line
  double dx = orn_x1 - orn_x2;
  double dy = orn_y1 - orn_y2;

  cout << "DX: " << dx << "\n";
  cout << "DY: " << dy << "\n";

  // Length of original segment
  double len = sqrt(dx * dx + dy * dy);
  cout << "LEN: " << len << "\n";

  if (len == 0)
    return region;

  // Perpendicular unit vector
  double px = -dy / len;
  double py = dx / len;

  cout << "PX: " << px << "\n";
  cout << "PY: " << py << "\n";

  // Half-length of perpendicular segment
  double halfLen = len / 2.0;

  // Endpoints of perpendicular line
  double x1 = cx + px * halfLen;
  double y1 = cy + py * halfLen;
  double x2 = cx - px * halfLen;
  double y2 = cy - py * halfLen;
  cout << x1 << " " << y1 << "\n";
  cout << x2 << " " << y2 << "\n";
  int temp;

  vector<pair<double, double>> polypoints;
  polypoints.push_back({x1, y1});
  polypoints.push_back(
      {static_cast<double>(orn_x1), static_cast<double>(orn_y1)});
  polypoints.push_back({x2, y2});

  polypoints.push_back(
      {static_cast<double>(orn_x2), static_cast<double>(orn_y2)});

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m; j++) {

      double x = static_cast<double>(j);
      double y = static_cast<double>(n - i - 1);

      vector<pair<double, double>> samples = {
          {x, y},
          {x - REGION_THRESHOLD, y},
          {x + REGION_THRESHOLD, y},
          {x, y - REGION_THRESHOLD},
          {x, y + REGION_THRESHOLD},
          {x - REGION_THRESHOLD, y - REGION_THRESHOLD},
          {x - REGION_THRESHOLD, y + REGION_THRESHOLD},
          {x + REGION_THRESHOLD, y - REGION_THRESHOLD},
          {x + REGION_THRESHOLD, y + REGION_THRESHOLD}};

      for (const auto &pt : samples) {
        if (pointInPolygon(polypoints, pt)) {
          region[i][j] = 1;
          break;
        }
      }
    }
  }

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < m; j++) {
      cout << region[i][j] << " ";
    }
    cout << "\n";
  }

  return region;
}

bool pointInPolygon(const vector<pair<double, double>> &poly,
                    pair<double, double> p) {
  bool inside = false;
  int n = poly.size();
  for (int i = 0, j = n - 1; i < n; j = i++) {
    if (pointOnSegment(poly[i].first, poly[i].second, poly[j].first,
                       poly[j].second, p.first, p.second))
      return true; // or classify as "boundary"
  }
  for (int i = 0, j = n - 1; i < n; j = i++) {
    double xi = poly[i].first, yi = poly[i].second;
    double xj = poly[j].first, yj = poly[j].second;

    double px = p.first, py = p.second;

    if ((yi > py) != (yj > py)) {
      double xIntersect = (xj - xi) * (py - yi) / (yj - yi) + xi;

      if (px < xIntersect)
        inside = !inside;
    }
  }
  return inside;
}

bool dfsUtil(int i, int j, int ei, int ej, const Board &board,
             const vector<vector<int>> &region, vector<vector<bool>> &visited,
             vector<pair<int, int>> &path, int start_i, int start_j) {

  // Out of bounds
  if (i < 0 || j < 0 || i >= board.board.size() || j >= board.board[0].size())
    return false;

  // Not allowed by region
  if (region[i][j] == 0)
    return false;

  // Already visited
  if (visited[i][j])
    return false;

  // Blocked cell (adjust if needed)
  if (board.board[i][j].hasPipe)
    return false;
  if (board.board[i][j].isTerminal && !(i == start_i && j == start_j) &&
      !(i == ei && j == ej))
    return false;

  visited[i][j] = true;
  path.push_back({i, j});

  // Reached destination
  if (i == ei && j == ej)
    return true;

  // 4-directional movement
  static int dx[4] = {1, -1, 0, 0};
  static int dy[4] = {0, 0, 1, -1};

  for (int d = 0; d < 4; d++) {
    if (dfsUtil(i + dx[d], j + dy[d], ei, ej, board, region, visited, path,
                start_i, start_j))
      return true;
  }

  // Backtrack
  path.pop_back();
  return false;
}

vector<pair<int, int>> dfsFindPath(pair<int, int> start, pair<int, int> end,
                                   const Board &board,
                                   const vector<vector<int>> &region) {

  int n = board.board.size();
  int m = board.board[0].size();

  vector<vector<bool>> visited(n, vector<bool>(m, false));
  vector<pair<int, int>> path;

  if (dfsUtil(start.first, start.second, end.first, end.second, board, region,
              visited, path, start.first, start.second)) {
    return path;
  }

  return {};
}
