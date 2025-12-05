#include "board.h"
#include "globals.h"
#include "raylib.h"
#include <vector>
#include <queue>
#include <algorithm>

std::unordered_map<int, Color> color_map = {
    {1, Color{255, 0, 0, 255}},   // Red
    {2, Color{0, 255, 0, 255}},   // Green
    {3, Color{0, 0, 255, 255}},   // Blue
    {4, Color{255, 165, 0, 255}}, // Orange
    {5, Color{255, 0, 255, 255}}, // Magenta / Pink
    {6, Color{0, 255, 255, 255}}, // Cyan / Aqua
    {7, Color{255, 255, 0, 255}},  // Yellow
    {8, Color{112, 55, 67, 255}}
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

int GRID = -1;

static const int CELL_SIZE = 80;
static const int PADDING = 100;

enum GameState { HUMAN_TURN, AI_TURN };

GameState state = HUMAN_TURN;

bool isDragging = false;
std::vector<std::pair<int, int>> dragPath;
int start_row = -1, start_col = -1;

int mouseToGridX(int mx) {
  mx -= PADDING;
  int gx = mx / CELL_SIZE;
  if (gx < 0 || gx >= GRID)
    return -1;
  return gx;
}

int mouseToGridY(int my) {
  my -= PADDING;
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
    Vector2 p1 = {c1 * CELL_SIZE + CELL_SIZE * 0.5f + PADDING,
                  r1 * CELL_SIZE + CELL_SIZE * 0.5f + PADDING};
    Vector2 p2 = {c2 * CELL_SIZE + CELL_SIZE * 0.5f + PADDING,
                  r2 * CELL_SIZE + CELL_SIZE * 0.5f + PADDING};
    DrawCircleV(p1, thickness * 0.5f, col);
    DrawCircleV(p2, thickness * 0.5f, col);
    DrawLineEx(p1, p2, thickness, col);
  }
}

void drawBoard(const Board &b) {
  float thickness = CELL_SIZE * 0.35f;

  for (int x = 0; x < GRID; x++) {
    for (int y = 0; y < GRID; y++) {

      Cell c = b.board[x][y];

      Color col = Color{0, 0, 0, 255};

      DrawRectangle(PADDING + y * CELL_SIZE, PADDING + x * CELL_SIZE,

                    CELL_SIZE - 2, CELL_SIZE - 2, col);

      if (c.isTerminal) {
        col = color_map[b.board[x][y].color];
        Vector2 point = {y * CELL_SIZE + CELL_SIZE * 0.5f + PADDING,
                         x * CELL_SIZE + CELL_SIZE * 0.5f + PADDING};
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

void drawDragPath(Board board) {
  if (dragPath.empty()) {
    return;
  }
  auto index = dragPath[0];
  int row = index.first;
  int col = index.second;
  int color_int = board.board[row][col].color;
  drawPath(dragPath, color_map[color_int]);
}

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

            // Forbidden:
            // - other pipes
            // - terminals of other colors
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

vector<pair<int,int>> algorithm(Board &board)
{
    // -------------------------------------------------------------------
    // 1. Collect all terminal pairs by color
    // -------------------------------------------------------------------
    unordered_map<int, vector<pair<int,int>>> terminals;

    for (int r = 0; r < GRID; r++) {
        for (int c = 0; c < GRID; c++) {
            const Cell &cell = board.board[r][c];
            if (cell.isTerminal)
                terminals[cell.color].push_back({r,c});
        }
    }

    // Build a list of pairs
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

    // -------------------------------------------------------------------
    // 2. Sort terminal pairs by Euclidean distance (smallest first)
    // -------------------------------------------------------------------
    std::sort(pairs.begin(), pairs.end(),
              [](const TerminalPair &x, const TerminalPair &y){
                  return x.dist < y.dist;
              });

    // -------------------------------------------------------------------
    // 3. Remove already-solved terminal pairs (solved by human or AI)
    // -------------------------------------------------------------------
    pairs.erase(
        remove_if(pairs.begin(), pairs.end(),
                  [&](const TerminalPair &tp)
                  {
                      return isAlreadySolved(board, tp.a, tp.b);
                  }),
        pairs.end());

    // -------------------------------------------------------------------
    // 4. Try solving pairs in sorted order until a solvable one is found
    // -------------------------------------------------------------------
    for (const auto &tp : pairs)
    {
        vector<pair<int,int>> path = bfsPath(board, tp.a, tp.b);

        if (!path.empty()) {
            // BFS succeeded → this pair is solvable
            return path;
        }

        // BFS failed → try next pair
    }

    // -------------------------------------------------------------------
    // 5. Nothing solvable
    // -------------------------------------------------------------------
    return {};  
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

  Rectangle undo_button = {PADDING + (int)(CELL_SIZE * GRID / 2) - 100,
                           PADDING + CELL_SIZE * GRID + 50, 200, 60};

  Rectangle reset_button = {PADDING + (int)(CELL_SIZE * GRID / 2) - 100,
                            PADDING + CELL_SIZE * GRID + 150, 200, 60};

  Board board;

  board.loadFromFile(files[choice]);

  for (int row = 0; row < GRID; row++) {
    for (int col = 0; col < GRID; col++) {
      cout << board.board[row][col].color << " " << row << " " << col << "\t";
    }
    cout << "\n";
  }

  InitWindow(2 * PADDING + GRID * CELL_SIZE,
             PADDING * 2 + CELL_SIZE * GRID + 300, "Flow Game - Raylib");
  SetTargetFPS(60);

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
        cout << row << " " << col << "\n";
        if (row != -1 && col != -1) {
          Cell &c = board.board[row][col];

          if (c.isTerminal) {
            isDragging = true;
            dragPath.clear();
            dragPath.push_back({row, col});
            start_col = col;
            start_row = row;
          }
        }
      }

      // Continue drag
      if (isDragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (row != -1 && col != -1) {

          auto last = dragPath.back();
          bool isNew = !(last.first == row && last.second == col);
          bool adjacent = (abs(last.first - row) == 1 && last.second == col) ||
                          (abs(last.second - col) == 1 && last.first == row);

          if (isNew && adjacent) {
            dragPath.push_back({row, col});
          }
        }
      }

      // End drag → human move complete
      if (isDragging && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        isDragging = false;

        if (!dragPath.empty()) {
          // Attempt move
          bool ok = board.makeMove(dragPath);

          if (ok) {
            // Move accepted → AI turn begins
            state = AI_TURN;
          }
        }

        dragPath.clear();
      }
    }

    // -----------------------------
    // AI TURN LOGIC
    // -----------------------------
    else if (state == AI_TURN) {

      // Call your algorithm
      auto ai_path = algorithm(board);
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
