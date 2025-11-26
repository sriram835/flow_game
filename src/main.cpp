#include "board.h"
#include "raylib.h"
#include <vector>

std::unordered_map<int, Color> color_map = {
    {1, Color{255, 0, 0, 255}}, // red
    {2, Color{0, 255, 0, 255}}, // green
    {3, Color{0, 0, 255, 255}}  // blue
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

static const int CELL_SIZE = 80;
static const int PADDING = 100;

Board board;

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

void drawDragPath() {
  if (dragPath.empty()) {
    return;
  }
  auto index = dragPath[0];
  int row = index.first;
  int col = index.second;
  int color_int = board.board[row][col].color;
  drawPath(dragPath, color_map[color_int]);
}

std::vector<std::pair<int, int>> algorithm() {
  vector<pair<int, int>> path;
  path.push_back(pair(3, 0));
  path.push_back(pair(2, 0));
  path.push_back(pair(1, 0));
  path.push_back(pair(1, 1));

  return path;
}

Rectangle undo_button = {PADDING + (int)(CELL_SIZE * GRID / 2) - 100,
                         PADDING + CELL_SIZE *GRID + 50, 200, 60};

Rectangle reset_button = {PADDING + (int)(CELL_SIZE * GRID / 2) - 100,
                          PADDING + CELL_SIZE *GRID + 150, 200, 60};

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

  board.loadFromFile(files[choice]);

  for (int row = 0; row < GRID; row++) {
    for (int col = 0; col < GRID; col++) {
      cout << board.board[row][col].color << " " << row << " " << col << "\t";
    }
    cout << "\n";
  }

  InitWindow(600, 800, "Flow Game - Raylib");
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
      board.makeMove(algorithm());
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
    drawDragPath();

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
