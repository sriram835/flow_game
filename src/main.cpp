#include "board.h"
#include "raylib.h"
#include <vector>

std::vector<std::string> getLevelFiles(const std::string &folderPath) {
  std::vector<std::string> files;

  for (const auto &entry : std::filesystem::directory_iterator(folderPath)) {
    if (entry.is_regular_file()) {
      files.push_back(entry.path().string());
    }
  }

  return files;
}

// ----------------------------------------------
static const int CELL_SIZE = 80;
static const int PADDING = 20;

Board board;

// Game state
enum GameState { HUMAN_TURN, AI_TURN };

GameState state = HUMAN_TURN;

// Dragging
bool isDragging = false;
std::vector<std::pair<int, int>> dragPath;
int startX = -1, startY = -1;

// ----------------------------------------------
// Convert mouse position → grid coordinates
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

// ----------------------------------------------
// Draw the board
void drawBoard(Board &b) {
  for (int x = 0; x < GRID; x++) {
    for (int y = 0; y < GRID; y++) {

      Cell &c = b.board[x][y];

      Color col = LIGHTGRAY;

      if (c.isTerminal) {
        if (c.color == 1)
          col = RED;
        if (c.color == 2)
          col = GREEN;
        if (c.color == 3)
          col = BLUE;
        if (c.color == 4)
          col = ORANGE;
      } else if (c.hasPipe) {
        if (c.color == 1)
          col = Color{230, 80, 80, 255};
        if (c.color == 2)
          col = Color{80, 230, 80, 255};
        if (c.color == 3)
          col = Color{80, 80, 230, 255};
        if (c.color == 4)
          col = Color{230, 180, 80, 255};
      }

      DrawRectangle(PADDING + x * CELL_SIZE, PADDING + y * CELL_SIZE,
                    CELL_SIZE - 2, CELL_SIZE - 2, col);
    }
  }
}

// ----------------------------------------------
// Draw current human drag path (ghost)
void drawDragPath() {
  for (auto &p : dragPath) {
    int x = p.first;
    int y = p.second;

    DrawRectangle(PADDING + x * CELL_SIZE, PADDING + y * CELL_SIZE,
                  CELL_SIZE - 2, CELL_SIZE - 2, Color{220, 220, 220, 120});
  }
}

// ----------------------------------------------
// Example algorithm() function signature
std::vector<std::pair<int, int>> algorithm() {
  vector<pair<int, int>> path;
  path.push_back(pair(3, 0));
  path.push_back(pair(2, 0));
  path.push_back(pair(1, 0));
  path.push_back(pair(1, 1));

  return path;
}

// ----------------------------------------------
// MAIN LOOP
int main() {
  // Ask user which level to load
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

  InitWindow(600, 600, "Flow Game - Raylib");
  SetTargetFPS(60);

  while (!WindowShouldClose()) {

    // -----------------------------
    // HUMAN TURN LOGIC
    // -----------------------------
    if (state == HUMAN_TURN) {

      int mx = GetMouseX();
      int my = GetMouseY();
      int gx = mouseToGridX(mx);
      int gy = mouseToGridY(my);

      // Start drag
      if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        if (gx != -1 && gy != -1) {
          Cell &c = board.board[gx][gy];
          if (c.isTerminal) {
            isDragging = true;
            dragPath.clear();
            dragPath.push_back({gx, gy});
            startX = gx;
            startY = gy;
          }
        }
      }

      // Continue drag
      if (isDragging && IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
        if (gx != -1 && gy != -1) {

          auto last = dragPath.back();
          bool isNew = !(last.first == gx && last.second == gy);

          bool adjacent = (abs(last.first - gx) == 1 && last.second == gy) ||
                          (abs(last.second - gy) == 1 && last.first == gx);

          if (isNew && adjacent) {
            dragPath.push_back({gx, gy});
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

    drawBoard(board);
    drawDragPath(); // draw ghost path only during human drag

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
