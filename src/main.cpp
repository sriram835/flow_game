// main.cpp - AI now uses Divide & Conquer 
#include "board.h"
#include "globals.h"
#include "raylib.h"
#include "solver.h" // new solver (d&c)

#include <algorithm>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <unordered_map>
#include <utility>
#include <vector>

bool pathLocked = false;
bool isDragging = false;
int GRID_OFFSET_X = 0;
int GRID_OFFSET_Y = 0;

std::unordered_map<int, Color> color_map = {
    {1, Color{255, 0, 0, 255}},   // Red
    {2, Color{0, 255, 0, 255}},   // Green
    {3, Color{0, 0, 255, 255}},   // Blue
    {4, Color{255, 165, 0, 255}}, // Orange
    {5, Color{255, 0, 255, 255}}, // Magenta / Pink
    {6, Color{0, 255, 255, 255}}, // Cyan / Aqua
    {7, Color{255, 255, 0, 255}}, // Yellow
    {8, Color{112, 55, 67, 255}},
    {9, Color{228, 123, 126, 255}},
    {10, Color{230, 143, 174, 255}}};

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

int CELL_SIZE = 80; 
int PADDING = 60;

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

void drawPath(const std::vector<std::pair<int, int>> &path, const Color col) {
  if (path.size() <= 1) {
    return;
  }
  float thickness = CELL_SIZE * 0.35f;

  for (size_t i = 0; i + 1 < path.size(); i++) {
    int r1 = path[i].first;
    int c1 = path[i].second;
    int r2 = path[i + 1].first;
    int c2 = path[i + 1].second;
    Vector2 p1 = {GRID_OFFSET_X + c1 * CELL_SIZE + CELL_SIZE * 0.5f,
                  GRID_OFFSET_Y + r1 * CELL_SIZE + CELL_SIZE * 0.5f};

    Vector2 p2 = {GRID_OFFSET_X + c2 * CELL_SIZE + CELL_SIZE * 0.5f,
                  GRID_OFFSET_Y + r2 * CELL_SIZE + CELL_SIZE * 0.5f};

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

      DrawRectangle(GRID_OFFSET_X + y * CELL_SIZE, GRID_OFFSET_Y + x * CELL_SIZE,
                    CELL_SIZE - 2, CELL_SIZE - 2, col);

      if (c.isTerminal) {
        col = color_map[b.board[x][y].color];
        Vector2 point = {GRID_OFFSET_X + y * CELL_SIZE + CELL_SIZE * 0.5f,
                         GRID_OFFSET_Y + x * CELL_SIZE + CELL_SIZE * 0.5f};
        DrawCircleV(point, thickness * 0.8f, col);
      }
    }
  }
  for (const auto &path : b.saved_paths) {
    if (path.empty()) {
      continue;
    }
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
    return -1;
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
  for (size_t i = 0; i < files.size(); i++) {
    std::cout << i << ": " << files[i] << "\n";
  }

  int choice;
  std::cout << "Enter number: ";
  std::cin >> choice;

  if (choice < 0 || static_cast<size_t>(choice) >= files.size()) {
    std::cout << "Invalid selection." << std::endl;
    return 1;
  }

  GRID = countLines(files[choice]);
  if (GRID == -1) {
    return 1;
  }

  int screenW = GetMonitorWidth(0);
  int screenH = GetMonitorHeight(0);

  int MAX_UI_SPACE = 300;

  if (GRID <= 0) {
    GRID = 1;
  }

  int availableW = screenW - 2 * PADDING;
  int availableH = screenH - 2 * PADDING - MAX_UI_SPACE;

  availableW = std::max(availableW, GRID);
  availableH = std::max(availableH, GRID);

  int cellW = availableW / GRID;
  int cellH = availableH / GRID;

  CELL_SIZE = std::min(cellW, cellH);

  if (CELL_SIZE < 60) {
    CELL_SIZE = 60;
  }
  if (CELL_SIZE > 80) {
    CELL_SIZE = 80;
  }

  Board board;
  board.init(GRID);
  board.loadFromFile(files[choice]);

  for (int row = 0; row < GRID; row++) {
    for (int col = 0; col < GRID; col++) {
      std::cout << board.board[row][col].color << " " << row << " " << col
                << "\t";
    }
    std::cout << "\n";
  }

  int windowW = 2 * PADDING + GRID * CELL_SIZE;
  int windowH = 2 * PADDING + GRID * CELL_SIZE + 300;

  if (windowW < 600) {
    windowW = 600;
  }
  if (windowH < 500) {
    windowH = 500;
  }

  if (windowW > 1800) {
    windowW = 1800;
  }
  if (windowH > 1000) {
    windowH = 1000;
  }

  SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_WINDOW_MAXIMIZED);

  InitWindow(windowW, windowH, "Flow Game - Raylib");
  SetTargetFPS(60);

  int monitor = GetCurrentMonitor();
  int screen_Width = GetMonitorWidth(monitor);
  int screen_Height = GetMonitorHeight(monitor);

  int posX = (screen_Width - windowW) / 2;
  int posY = (screen_Height - windowH) / 2;

  if (posX < 0) {
    posX = 0;
  }
  if (posY < 0) {
    posY = 0;
  }
  SetWindowPosition(posX, posY);

  int gridPixelSize = GRID * CELL_SIZE;

  GRID_OFFSET_X = (GetScreenWidth() - gridPixelSize) / 2;
  GRID_OFFSET_Y = (GetScreenHeight() - gridPixelSize - 300) / 2;

  if (GRID_OFFSET_Y < 40) {
    GRID_OFFSET_Y = 40;
  }

  Rectangle undo_button = {(float)((GetScreenWidth() - 200) / 2),
                           (float)(GRID_OFFSET_Y + gridPixelSize + 40),
                           (float)200.0f, (float)60.0f};

  Rectangle reset_button = {(float)((GetScreenWidth() - 200) / 2),
                            (float)(GRID_OFFSET_Y + gridPixelSize + 120),
                            (float)200.0f, (float)60.0f};

  Font roboto_font =
      LoadFontEx("./resources/fonts/Roboto-Black.ttf", 64, NULL, 250);

  SetTextureFilter(roboto_font.texture, TEXTURE_FILTER_TRILINEAR);

  Solver solver;

  while (!WindowShouldClose()) {
    Vector2 mouse_pos = GetMousePosition();
    bool undo_hover = CheckCollisionPointRec(mouse_pos, undo_button);
    bool undo_clicked = undo_hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    bool reset_hover = CheckCollisionPointRec(mouse_pos, reset_button);
    bool reset_clicked = reset_hover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON);

    if (undo_clicked && !board.saved_paths.empty()) {
      std::cout << "Undo clicked\n";
      board.undoMove();
    }

    if (reset_clicked) {
      std::cout << "Reset clicked\n";
      board.resetBoard();
    }

    if (state == HUMAN_TURN) {
      int mx = GetMouseX();
      int my = GetMouseY();
      int row = mouseToGridY(my);
      int col = mouseToGridX(mx);

      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (row != -1 && col != -1) {
          Cell &c = board.board[row][col];
          if (c.isTerminal) {
            isDragging = true;
            dragPath.clear();
            dragPath.push_back({row, col});
            start_row = row;
            start_col = col;
          }
        }
      }

      if (isDragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON) && !pathLocked) {
        if (row == -1 || col == -1)
          goto END_DRAG;

        auto last = dragPath.back();
        int dx = row - last.first;
        int dy = col - last.second;

        if (!((std::abs(dx) == 1 && dy == 0) ||
              (std::abs(dy) == 1 && dx == 0))) {
          goto END_DRAG;
        }

        for (auto &p : dragPath) {
          if (p.first == row && p.second == col) {
            goto END_DRAG;
          }
        }

        Cell &next = board.board[row][col];
        Cell &startCell = board.board[start_row][start_col];

        if (next.hasPipe) {
          goto END_DRAG;
        }

        if (next.isTerminal) {
          if (next.color != startCell.color) {
            goto END_DRAG;
          }

          // Correct destination -> lock.
          dragPath.push_back({row, col});
          pathLocked = true;
          goto END_DRAG;
        }

        dragPath.push_back({row, col});
      }

    END_DRAG:
      if (isDragging && IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) {
        isDragging = false;

        if (pathLocked) {
          board.makeMove(dragPath);
          state = AI_TURN;
        }

        dragPath.clear();
        pathLocked = false;
      }
    } else if (state == AI_TURN) {
      bool solved = solver.solve(board);

      if (!solved) {
        std::cout
            << "D&C solver failed (no backtracking). Returning control to human.\n";
      }

      state = HUMAN_TURN;
    }

    BeginDrawing();
    ClearBackground(RAYWHITE);

    if (CheckCollisionPointRec(GetMousePosition(), undo_button)) {
      DrawRectangleRec(undo_button, LIGHTGRAY);
    } else {
      DrawRectangleRec(undo_button, GRAY);
    }
    DrawRectangleLines(undo_button.x, undo_button.y, undo_button.width,
                       undo_button.height, BLACK);
    DrawTextEx(roboto_font, "Undo", (Vector2){undo_button.x + 60, undo_button.y + 15},
               32, 2, BLACK);

    if (CheckCollisionPointRec(GetMousePosition(), reset_button)) {
      DrawRectangleRec(reset_button, LIGHTGRAY);
    } else {
      DrawRectangleRec(reset_button, GRAY);
    }
    DrawRectangleLines(reset_button.x, reset_button.y, reset_button.width,
                       reset_button.height, BLACK);
    DrawTextEx(roboto_font, "Reset",
               (Vector2){reset_button.x + 60, reset_button.y + 15}, 32, 2, BLACK);

    drawBoard(board);
    drawDragPath(board);

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
