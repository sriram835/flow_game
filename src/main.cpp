#include "board.h"
#include "globals.h"
#include "raylib.h"
#include <utility>
#include <vector>
#include <queue>
#include <algorithm>
#include <functional>

std::unordered_map<int, Color> color_map = {
    {1, Color{255, 0, 0, 255}},     // Red
    {2, Color{0, 255, 0, 255}},     // Green
    {3, Color{0, 0, 255, 255}},     // Blue
    {4, Color{255, 165, 0, 255}},   // Orange
    {5, Color{255, 0, 255, 255}},   // Magenta / Pink
    {6, Color{0, 255, 255, 255}},   // Cyan / Aqua
    {7, Color{255, 255, 0, 255}},   // Yellow
    {8, Color{112, 55, 67, 255}},   // Brown
    {9, Color{127, 234, 112, 255}}, // Light Green
    {10, Color{63, 67, 123, 255}}   // Grey
};

std::vector<std::string> getLevelFiles(const std::string &folderPath) {
  std::vector<std::string> files;

  for (const auto &entry : std::filesystem::directory_iterator(folderPath)) {
    if (entry.is_regular_file()) {
      files.push_back(entry.path().string());
    }
  }

  return files;
}

int dir_dx = 0, dir_dy = 0;
bool directionLocked = false;
bool pathLocked = false;
bool isDragging = false;
int GRID_OFFSET_X = 0;
int GRID_OFFSET_Y = 0;
int GRID=-1;
static int CELL_SIZE = 80;
static int PADDING = 100;

enum GameState { HUMAN_TURN, AI_TURN };

GameState state = HUMAN_TURN;

std::vector<std::pair<int, int>> dragPath;
int start_row = -1, start_col = -1;

int mouseToGridX(int mx) {
  mx -= GRID_OFFSET_X;
  int gx = mx / CELL_SIZE;
  if (gx < 0 || gx >= GRID)
    return -1;
  return gx;
}

int mouseToGridY(int my) {
  my -= GRID_OFFSET_Y;
  int gy = my / CELL_SIZE;
  if (gy < 0 || gy >= GRID)
    return -1;
  return gy;
}

void drawPath(const vector<pair<int, int>> &path, const Color col) {
  if (path.empty())
    return;
  float thickness = CELL_SIZE * 0.35f;

  if (path.size() == 1) {
    return;
  }

  for (int i = 0; i < path.size() - 1; i++) {
    int r1 = path[i].first, c1 = path[i].second;
    int r2 = path[i + 1].first, c2 = path[i + 1].second;
    Vector2 p1 = {c1 * CELL_SIZE + CELL_SIZE * 0.5f + GRID_OFFSET_X,
                  r1 * CELL_SIZE + CELL_SIZE * 0.5f + GRID_OFFSET_Y};
    Vector2 p2 = {c2 * CELL_SIZE + CELL_SIZE * 0.5f + GRID_OFFSET_X,
                  r2 * CELL_SIZE + CELL_SIZE * 0.5f + GRID_OFFSET_Y};
    DrawCircleV(p1, thickness * 0.5f, col);
    DrawCircleV(p2, thickness * 0.5f, col);
    DrawLineEx(p1, p2, thickness, col);
  }
}

void drawBoard(const Board &b) {
  float thickness = CELL_SIZE * 0.35f;

  for (int x = 0; x < GRID; x++) {
    for (int y = 0; y < GRID; y++) {

      const Cell &c = b.board[x][y];

      Color col = Color{0, 0, 0, 255};

      DrawRectangle(GRID_OFFSET_X+ y * CELL_SIZE, GRID_OFFSET_Y + x * CELL_SIZE,

                    CELL_SIZE - 2, CELL_SIZE - 2, col);

      if (c.isTerminal) {
        col = color_map[b.board[x][y].color];
        Vector2 point = {y * CELL_SIZE + CELL_SIZE * 0.5f + GRID_OFFSET_X,
                         x * CELL_SIZE + CELL_SIZE * 0.5f + GRID_OFFSET_Y};
        DrawCircleV(point, thickness * 0.8f, col);
      }
    }
  }
  for (vector<pair<int, int>> path : b.saved_paths) {
    auto cell_index = path[0];
    int row = cell_index.first;
    int col = cell_index.second;

    Color color = color_map[b.board[row][col].color];
    drawPath(path, color);
  }
}

void drawDragPath(const Board &board) {
  if (dragPath.empty()) {
    return;
  }
  auto index = dragPath[0];
  int row = index.first;
  int col = index.second;
  int color_int = board.board[row][col].color;
  drawPath(dragPath, color_map[color_int]);
}

int countLines(const std::string &filePath) {
  std::ifstream file(filePath);

  if (!file.is_open()) {
    return -1; // Could not open file
  }

  int count = 0;
  std::string line;

  while (std::getline(file, line)) {
    count++;
  }

  return count;
}

/* ===============================================================
                           OLD LOGIC
   =============================================================== */

struct TerminalPair {
    int color;
    pair<int,int> a;
    pair<int,int> b;
    double dist;
};

double distanceEuclid(pair<int,int> p1, pair<int,int> p2) {
    double dx = p1.first - p2.first;
    double dy = p1.second - p2.second;
    return sqrt(dx*dx + dy*dy);
}

void shell_sort(vector<TerminalPair>&);

void shell_sort(std::vector<TerminalPair>& pairs)
{
    int hseq[] = {31, 15, 7,3,1};  //all praise hibbard sequence
    int n = pairs.size();

    for (int k = 0; k <= 4; k++)
    {
        int gap = hseq[k];
        if (gap >= n)
        {
          continue;
        }
        for (int i = gap; i < n; i++)  //We can let the for loop take care of gap >= n, but one less assignment
        {
            TerminalPair temp = pairs[i]; 
            int j;
            for (j = i; j >= gap && pairs[j - gap].dist > temp.dist; j -= gap)
            {
                pairs[j] = pairs[j - gap]; //shifttinhggkhubkhbl
            }
            pairs[j] = temp;
        }
    }
}

vector<pair<int,int>> bfsPath(const Board &board, pair<int,int> start, pair<int,int> goal)
{
    vector<vector<bool>> visited(GRID, vector<bool>(GRID,false));
    queue<pair<int,int>> q;
    unordered_map<int, pair<int,int>> parent;

    auto encode = [&](int r, int c){ return r * GRID + c; };

    q.push(start);
    visited[start.first][start.second] = true;

    int dr[4] = {1,-1,0,0};
    int dc[4] = {0,0,1,-1};

    while (!q.empty()) {
        auto [r, c] = q.front();
        q.pop();

        if (r == goal.first && c == goal.second)
            break;

        for (int k = 0; k < 4; k++) {
            int nr = r + dr[k];
            int nc = c + dc[k];

            // Bounds
            if (nr < 0 || nr >= GRID || nc < 0 || nc >= GRID)
                continue;

            const Cell &cell = board.board[nr][nc];

            if (cell.hasPipe)
                continue;
            if (cell.isTerminal && !(nr == goal.first && nc == goal.second))
                continue;

            if (!visited[nr][nc]) {
                visited[nr][nc] = true;
                parent[encode(nr,nc)] = std::make_pair(r, c);
                q.push({nr,nc});
            }
        }
    }

    // Reconstruct path
    vector<pair<int,int>> path;

    if (!visited[goal.first][goal.second])
        return path;  // no path, return empty

    pair<int,int> cur = goal;
    while (!(cur.first == start.first && cur.second == start.second)) {
        path.push_back(cur);
        cur = parent[encode(cur.first, cur.second)];
    }
    path.push_back(start);
    reverse(path.begin(), path.end());
    return path;
}

bool isAlreadySolved(const Board &board, pair<int,int> a, pair<int,int> b)
{
    for (const auto &path : board.saved_paths)
    {
        if (path.size() < 2) continue;

        auto start = path.front();
        auto end   = path.back();

        if ((start == a && end == b) ||
            (start == b && end == a))
        {
            return true;
        }
    }
    return false;
}

/* ===============================================================
                           NEW LOGIC
   =============================================================== */

vector<vector<pair<int,int>>> findEmptyRegions(const Board &board) {
    vector<vector<bool>> visited(GRID, vector<bool>(GRID, false));
    vector<vector<pair<int,int>>> regions;
    
    //flood fill
    function<void(int, int, vector<pair<int,int>>&)> flood = 
        [&](int r, int c, vector<pair<int,int>> &region) {

        //grid border reached
        if (r < 0 || r >= GRID || c < 0 || c >= GRID) return;

        //cell already visited
        if (visited[r][c]) return;
        
        //used cell
        const Cell &cell = board.board[r][c];
        if (cell.hasPipe || cell.isTerminal) return;
        
        visited[r][c] = true;
        region.push_back({r, c});
        
        //flood fill recursion
        flood(r+1, c, region);
        flood(r-1, c, region);
        flood(r, c+1, region);
        flood(r, c-1, region);
    };
    
    for (int r = 0; r < GRID; r++) {
        for (int c = 0; c < GRID; c++) {
            const Cell &cell = board.board[r][c];
            if (!visited[r][c] && !cell.hasPipe && !cell.isTerminal) {
                vector<pair<int,int>> region;
                flood(r, c, region);
                if (!region.empty()) {
                    regions.push_back(region);
                }
            }
        }
    }
    
    return regions;
}

vector<pair<int,int>> regionPath(const Board &board, 
                                        pair<int,int> start, 
                                        pair<int,int> goal) {
    cout << "\n=== regionPath called ===\n";
    
    //finding shortest path
    vector<pair<int,int>> shortestPath = bfsPath(board, start, goal);
    if (shortestPath.empty()) return {};
    int shortestLen = shortestPath.size();
    cout << "Shortest path length: " << shortestLen << "\n";
    
    //maximum difference between new and shortest path
    int maxExtra = 25;
    
    //region vector and region values for each cell
    auto emptyRegions = findEmptyRegions(board);
    vector<vector<int>> regionValue(GRID, vector<int>(GRID, 0));
    
    for (const auto &region : emptyRegions) {
        if (region.size() >= 2) {
            for (const auto &[r, c] : region) {
                regionValue[r][c] = region.size();
            }
        }
    }
    
    vector<vector<int>> bestScore(GRID, vector<int>(GRID, INT_MAX));
    vector<vector<int>> distance(GRID, vector<int>(GRID, INT_MAX));
    vector<vector<pair<int,int>>> parent(GRID, vector<pair<int,int>>(GRID, {-1,-1}));
    vector<vector<bool>> finalized(GRID, vector<bool>(GRID, false)); //tracking finalised cells
    
    //modified dijkstra's algorithm

    priority_queue<pair<int, pair<int,int>>, 
                   vector<pair<int, pair<int,int>>>,
                   greater<pair<int, pair<int,int>>>> pq;
    
    pq.push({0, start});
    bestScore[start.first][start.second] = 0;
    distance[start.first][start.second] = 0;
    
    int dr[4] = {1,-1,0,0};
    int dc[4] = {0,0,1,-1};
    
    while (!pq.empty()) {
        auto [currentScore, pos] = pq.top();
        pq.pop();
        
        auto [r, c] = pos;
        
        //skip if finalised
        if (finalized[r][c]) continue;
        
        finalized[r][c] = true;
        
        if (r == goal.first && c == goal.second) {
            cout << "Reached goal!\n";
            break;
        }
        
        if (distance[r][c] >= shortestLen + maxExtra) continue;
        
        //checking neighbours
        for (int k = 0; k < 4; k++) {
            int nr = r + dr[k];
            int nc = c + dc[k];
            
            if (nr < 0 || nr >= GRID || nc < 0 || nc >= GRID) continue;
            
            const Cell &cell = board.board[nr][nc];
            if (cell.hasPipe) continue;
            if (cell.isTerminal && !(nr == goal.first && nc == goal.second)) continue;
            
            //skip finalised cells
            if (finalized[nr][nc]) continue;
            
            int newDist = distance[r][c] + 1;

            /*calculating priority based on region value
            -->larger regions have lower region values => higher priority*/
            int newScore = currentScore + 1 - (regionValue[nr][nc] * 2);
            
            if (newScore < bestScore[nr][nc]) {
                bestScore[nr][nc] = newScore;
                distance[nr][nc] = newDist;
                parent[nr][nc] = {r, c};
                pq.push({newScore, {nr, nc}});
            }
        }
    }
    
    if (!finalized[goal.first][goal.second]) {
        cout << "Could not reach goal\n";
        return shortestPath;
    }
    
    //reconstruct path
    vector<pair<int,int>> path;
    pair<int,int> cur = goal;
    
    while (!(cur == start)) {
        path.push_back(cur);
        cur = parent[cur.first][cur.second];
    }
    path.push_back(start);
    reverse(path.begin(), path.end());
    
    cout << "SUCCESS! Shortest: " << shortestLen << " cells, Region-aware: " << path.size() << " cells\n";
    
    //count region cells
    int regionCells = 0;
    for (const auto &[r, c] : path) {
        if (regionValue[r][c] > 0) regionCells++;
    }
    cout << "Path goes through " << regionCells << " region cells\n";
    
    //choose path
    return (path.size() <= shortestLen + maxExtra) ? path : shortestPath;
}

/* ===============================================================
                            ALGORITHMS
   =============================================================== */

vector<pair<int,int>> algorithm(Board &board)
{
    // collecting terminal pairs by colour
    unordered_map<int, vector<pair<int,int>>> terminals;

    for (int r = 0; r < GRID; r++) {
        for (int c = 0; c < GRID; c++) {
            const Cell &cell = board.board[r][c];
            if (cell.isTerminal)
                terminals[cell.color].push_back({r,c});
        }
    }

    // building a list of pairs
    vector<TerminalPair> pairs;

    for (auto &kv : terminals) {
        auto &vec = kv.second;
        if (vec.size() != 2)
            continue;

        TerminalPair tp;
        tp.color = kv.first;
        tp.a = vec[0];
        tp.b = vec[1];
        tp.dist = distanceEuclid(tp.a, tp.b);

        pairs.push_back(tp);
    }

    // sorting terminal pairs by euclidean distance
    shell_sort(pairs);

    /// removing solved pairs
    pairs.erase(
        remove_if(pairs.begin(), pairs.end(),
                  [&](const TerminalPair &tp)
                  {
                      return isAlreadySolved(board, tp.a, tp.b);
                  }),
        pairs.end());

    // solving pairs in order
    for (const auto &tp : pairs)
    {
        vector<pair<int,int>> path = bfsPath(board, tp.a, tp.b);

        if (!path.empty()) {
            return path;
        }
        // bfs failed-> next pair
    }

    // unsolvable path
    return {};  
}

vector<pair<int,int>> algorithm2(Board &board) {
    unordered_map<int, vector<pair<int,int>>> terminals;
    
    //collect terminal pairs
    for (int r = 0; r < GRID; r++) {
        for (int c = 0; c < GRID; c++) {
            const Cell &cell = board.board[r][c];
            if (cell.isTerminal)
                terminals[cell.color].push_back({r,c});
        }
    }
    
    vector<TerminalPair> pairs;
    
    for (auto &kv : terminals) {
        auto &vec = kv.second;
        if (vec.size() != 2) continue;
        
        TerminalPair tp;
        tp.color = kv.first;
        tp.a = vec[0];
        tp.b = vec[1];
        tp.dist = distanceEuclid(tp.a, tp.b);
        
        pairs.push_back(tp);
    }
    
    //sort by euclidean distance
    shell_sort(pairs);
    reverse(pairs.begin(), pairs.end());
    
    //remove used pairs
    pairs.erase(
        remove_if(pairs.begin(), pairs.end(),
                  [&](const TerminalPair &tp) {
                      return isAlreadySolved(board, tp.a, tp.b);
                  }),
        pairs.end());
    
    //flood fill
    auto emptyRegions = findEmptyRegions(board);
    
    cout << "\n=== AI TURN ===\n";
    cout << "Found " << emptyRegions.size() << " regions\n";
    for (size_t i = 0; i < emptyRegions.size(); i++) {
        cout << "Region " << i << ": " << emptyRegions[i].size() << " cells\n";
    }

    //scoring pairs
    vector<pair<double, int>> scoredPairs;
    
    for (size_t i = 0; i < pairs.size(); i++) {
        const auto &tp = pairs[i];
        double score = -tp.dist;
        
        //checking proximity to emoty regions
        for (const auto &region : emptyRegions) {
            if (region.size() < 2) continue;
            
            //checking proximity to terminals
            bool nearStart = false, nearEnd = false;
            for (const auto &[r, c] : region) {
                if (abs(r - tp.a.first) + abs(c - tp.a.second) <= 4)
                    nearStart = true;
                if (abs(r - tp.b.first) + abs(c - tp.b.second) <= 4)
                    nearEnd = true;
            }
            
            //higher priority if both terminals are near the region
            if (nearStart && nearEnd) {
                score -= region.size() * 3.0;
            }
        }
        
        scoredPairs.push_back({score, i});
    }
    
    //sorting scored pairs
    sort(scoredPairs.begin(), scoredPairs.end());
    
    //choosing best pair
    for (const auto &[score, idx] : scoredPairs) {
        const auto &tp = pairs[idx];
        
        vector<pair<int,int>> path = regionPath(board, tp.a, tp.b);
        
        if (!path.empty()) {
            return path;
        }
    }
    
    return {};
}

/* ===============================================================
                               MAIN
   =============================================================== */

int main() {
  auto files = getLevelFiles("levels");
  if (files.empty()) {
    std::cout << "No level files found in /levels" << std::endl;
    return 1;
  }

  std::cout << "Select a level:\n";
  for (int i = 0; i < files.size(); i++) {
    std::cout << i << ": " << files[i] << "\n";
  }

  int choice;
  std::cout << "Enter number: ";
  std::cin >> choice;

  if (choice < 0 || choice >= files.size()) {
    std::cout << "Invalid selection." << std::endl;
    return 1;
  }

  GRID = countLines(files[choice]);
  if (GRID == -1) {
    exit(EXIT_FAILURE);
  }
  // -------------------------------
  // Dynamic scaling (SAFE VERSION)
  // -------------------------------
  int screenW = GetMonitorWidth(0);
  int screenH = GetMonitorHeight(0);

  int MAX_UI_SPACE = 300;

  // Prevent division issues
  if (GRID <= 0) GRID = 1;

  int availableW = screenW - 2 * PADDING;
  int availableH = screenH - 2 * PADDING - MAX_UI_SPACE;

  // Safety clamp
  availableW = std::max(availableW, GRID);
  availableH = std::max(availableH, GRID);

  int cellW = availableW / GRID;
  int cellH = availableH / GRID;

  CELL_SIZE = std::min(cellW, cellH);

  // HARD safety limits (important)
  if (CELL_SIZE < 60) CELL_SIZE = 60;
  if (CELL_SIZE > 80) CELL_SIZE = 80;

  Board board;
  board.init(GRID); 
  board.loadFromFile(files[choice]);

  auto testRegions = findEmptyRegions(board);
    cout << "Found " << testRegions.size() << " empty regions\n";
    for (size_t i = 0; i < testRegions.size(); i++) {
        cout << "Region " << i << ": " << testRegions[i].size() << " cells\n";
    }

  for (int row = 0; row < GRID; row++) {
    for (int col = 0; col < GRID; col++) {
      cout << board.board[row][col].color << " " << row << " " << col << "\t";
    }
    cout << "\n";
  }

  int windowW = 2 * PADDING + GRID * CELL_SIZE;
  int windowH = 2 * PADDING + GRID * CELL_SIZE + 300;

  // Hard minimums (GLFW requires positive size)
  if (windowW < 600) windowW = 600;
  if (windowH < 500) windowH = 500;

  // Hard maximums WITHOUT monitor query
  if (windowW > 1800) windowW = 1800;
  if (windowH > 1000)  windowH = 1000;

  //SetConfigFlags(FLAG_WINDOW_TOPMOST);
  InitWindow(windowW, windowH, "Flow Game - Raylib");
  SetTargetFPS(60);

  // --------------------------------
  // Center the window
  // --------------------------------

  int Monitor = GetCurrentMonitor();
  int screen_Width = GetMonitorWidth(Monitor);
  int screen_Height = GetMonitorHeight(Monitor);
  int window_Width = GetScreenWidth();
  int window_Height = GetScreenHeight();

  int posX = (screen_Width-window_Width)/2;
  int posY = (screen_Height-window_Height)/2;
  
  if(posX<0) posX = 0;
  if(posY<0) posY = 0;
  SetWindowPosition(posX,posY);

  SetWindowState(FLAG_WINDOW_TOPMOST);
  BeginDrawing();
  ClearBackground(RAYWHITE);
  EndDrawing();
  ClearWindowState(FLAG_WINDOW_TOPMOST);

  // --------------------------------
  // Center the grid inside the window
  // --------------------------------
  int gridPixelSize = GRID * CELL_SIZE;

  GRID_OFFSET_X = (GetScreenWidth()  - gridPixelSize) / 2;
  GRID_OFFSET_Y = (GetScreenHeight() - gridPixelSize - 300) / 2;

  // Keep some space at the top for aesthetics
  if (GRID_OFFSET_Y < 40) GRID_OFFSET_Y = 40;

  Rectangle undo_button = {
    (GetScreenWidth() - 200) / 2,
    GRID_OFFSET_Y + gridPixelSize + 40,
    200, 60
  };

  Rectangle reset_button = {
    (GetScreenWidth() - 200) / 2,
    GRID_OFFSET_Y + gridPixelSize + 120,
    200, 60
  };

  Font roboto_font =
      LoadFontEx("./resources/fonts/Roboto-Black.ttf", 64, NULL, 250);

  SetTextureFilter(roboto_font.texture, TEXTURE_FILTER_TRILINEAR);

  while (!WindowShouldClose()) {
    Vector2 mouse_pos = GetMousePosition();
    bool undo_hover = CheckCollisionPointRec(mouse_pos, undo_button);
    bool undo_clicked = undo_hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    bool reset_hover = CheckCollisionPointRec(mouse_pos, reset_button);
    bool reset_clicked = reset_hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    if (undo_clicked == true && !board.saved_paths.empty()) {
      cout << "Undo clicked\n";
      board.undoMove();
    }

    if (reset_clicked) {
      cout << "Reset clicked\n";
      board.resetBoard();
    }

    // -----------------------------
    // HUMAN TURN LOGIC
    // -----------------------------
    if (state == HUMAN_TURN) {

        int mx = GetMouseX();
        int my = GetMouseY();
        int row = mouseToGridY(my);
        int col = mouseToGridX(mx);

        // Start drag
        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
            if (row != -1 && col != -1) {
                Cell &c = board.board[row][col];
                if (c.isTerminal) {
                    isDragging = true;
                    // directionLocked = false;
                    dragPath.clear();
                    dragPath.push_back({row, col});
                    start_row = row;
                    start_col = col;
                }
            }
        }

        // Continue drag
        if (isDragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON) && !pathLocked) {

            if (row == -1 || col == -1)
                goto END_DRAG;

            auto last = dragPath.back();
            int dx = row - last.first;
            int dy = col - last.second;

            // Must be exactly 1 step
            if (!((abs(dx) == 1 && dy == 0) || (abs(dy) == 1 && dx == 0)))
                goto END_DRAG;

            // No self overlap
            for (auto &p : dragPath)
                if (p.first == row && p.second == col)
                    goto END_DRAG;

            Cell &next = board.board[row][col];
            Cell &startCell = board.board[start_row][start_col];

            // No overlapping other pipes
            if (next.hasPipe)
                goto END_DRAG;

            // Terminal rules
            if (next.isTerminal) {
                if (next.color != startCell.color)
                    goto END_DRAG;

                // Correct destination → lock
                dragPath.push_back({row, col});
                pathLocked = true;
                goto END_DRAG;
            }

            // Normal move
            dragPath.push_back({row, col});
        }

    END_DRAG:

        // End drag
        if (isDragging && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
            isDragging = false;

            // Commit ONLY if destination reached
            if (pathLocked) {
                board.makeMove(dragPath);
                state = AI_TURN;
            }

            dragPath.clear();
            pathLocked = false;
            directionLocked = false;
        }
    }

    // -----------------------------
    // AI TURN LOGIC
    // -----------------------------
    else if (state == AI_TURN) {

      auto ai_path = algorithm2(board);
      if (!ai_path.empty())
          board.makeMove(ai_path);

      state = HUMAN_TURN;

    }

    // -----------------------------
    // DRAWING
    // -----------------------------
    BeginDrawing();
    ClearBackground(RAYWHITE);

    if (undo_hover) {
      DrawRectangleRec(undo_button, LIGHTGRAY);

    } else {
      DrawRectangleRec(undo_button, GRAY);
    }
    DrawRectangleLines(undo_button.x, undo_button.y, undo_button.width,
                       undo_button.height, BLACK);

    DrawTextEx(roboto_font, "Undo",
               (Vector2){undo_button.x + 60, undo_button.y + 15}, 32, 2, BLACK);

    if (reset_hover) {
      DrawRectangleRec(reset_button, LIGHTGRAY);

    } else {
      DrawRectangleRec(reset_button, GRAY);
    }
    DrawRectangleLines(reset_button.x, reset_button.y, reset_button.width,
                       reset_button.height, BLACK);

    DrawTextEx(roboto_font, "Reset",
               (Vector2){reset_button.x + 60, reset_button.y + 15}, 32, 2,
               BLACK);

    drawBoard(board);
    drawDragPath(board);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}