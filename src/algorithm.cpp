#include "algorithm.h"
#include <queue>
#include <limits>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <cmath>
#include <random>
#include <iostream>

using namespace std;

int manhattan(int x1, int y1, int x2, int y2) {
  return abs(x1 - x2) + abs(y1 - y2);
}

int randomInt(int upper_bound) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::uniform_int_distribution<> dist(0, upper_bound - 1);
  return dist(gen);
}

bool colorAlreadyAdded(int color, vector<vector<int>> colors) {
  for (int i = 0; i < (int)colors.size(); i++) {
    if (color == colors[i][0]) {
      return true;
    }
  }
  return false;
}

bool terminalsReachable(const Board &board, int sr, int sc, int er, int ec) {
  int N = GRID;
  vector<vector<bool>> vis(N, vector<bool>(N, false));
  queue<pair<int,int>> q;
  q.push({sr, sc});
  vis[sr][sc] = true;

  while (!q.empty()) {
    auto cur = q.front(); q.pop();
    int r = cur.first, c = cur.second;
    if (r == er && c == ec) return true;

    int dr[4] = {-1, 0, 0, 1};
    int dc[4] = {0, -1, 1, 0};
    for (int k=0;k<4;k++){
      int nr = r + dr[k], nc = c + dc[k];
      if (nr < 0 || nc < 0 || nr >= N || nc >= N) continue;
      if (vis[nr][nc]) continue;
      if (board.board[nr][nc].hasPipe) continue;
      vis[nr][nc] = true;
      q.push({nr, nc});
    }
  }
  return false;
}

vector<vector<int>> getTerminals(Board board) {
  unordered_map<int, vector<pair<int,int>>> colorGroups;
  for (int r = 0; r < GRID; ++r) {
    for (int c = 0; c < GRID; ++c) {
      int colr = board.board[r][c].color;
      if (colr != 0 && board.board[r][c].isTerminal && !board.board[r][c].hasPipe) {
        colorGroups[colr].push_back({r,c});
      }
    }
  }

  vector<vector<int>> result;
  for (auto &kv : colorGroups) {
    int color = kv.first;
    auto &terms = kv.second;
    if (terms.size() != 2) continue;
    auto [r1,c1] = terms[0];
    auto [r2,c2] = terms[1];

    if (terminalsReachable(board, r1, c1, r2, c2)) {
      result.push_back({color, r1, c1});
      result.push_back({color, r2, c2});
    }
  }

  return result;
}

vector<pair<int, int>> getNeighbors(int color, int row, int col,
                                    const Board &board,
                                    const vector<vector<bool>> &visited) {
  vector<pair<int,int>> valid;
  int N = GRID;
  if (row-1 >= 0 && !visited[row-1][col] &&
      !board.board[row-1][col].hasPipe &&
      !(board.board[row-1][col].isTerminal && board.board[row-1][col].color != color)) {
    valid.push_back({row-1, col});
  }
  if (col-1 >= 0 && !visited[row][col-1] &&
      !board.board[row][col-1].hasPipe &&
      !(board.board[row][col-1].isTerminal && board.board[row][col-1].color != color)) {
    valid.push_back({row, col-1});
  }
  if (col+1 < N && !visited[row][col+1] &&
      !board.board[row][col+1].hasPipe &&
      !(board.board[row][col+1].isTerminal && board.board[row][col+1].color != color)) {
    valid.push_back({row, col+1});
  }
  if (row+1 < N && !visited[row+1][col] &&
      !board.board[row+1][col].hasPipe &&
      !(board.board[row+1][col].isTerminal && board.board[row+1][col].color != color)) {
    valid.push_back({row+1, col});
  }
  return valid;
}

vector<pair<int, int>>
reconstructPath(int end_row, int end_col,
                const vector<vector<pair<int, int>>> &parents) {
  vector<pair<int,int>> path;
  int r = end_row, c = end_col;
  while (r != -1 && c != -1) {
    path.push_back({r,c});
    auto p = parents[r][c];
    r = p.first; c = p.second;
  }
  reverse(path.begin(), path.end());
  return path;
}

bool hasDeadSpace(const Board &b) {
  int N = GRID;
  vector<vector<bool>> seen(N, vector<bool>(N,false));
  int dr[4] = {-1,0,0,1};
  int dc[4] = {0,-1,1,0};

  for (int r=0;r<N;r++){
    for (int c=0;c<N;c++){
      if (seen[r][c]) continue;
      if (b.board[r][c].hasPipe) { seen[r][c]=true; continue; }

      int freeCount = 0;
      int termCount = 0;
      queue<pair<int,int>> q; q.push({r,c}); seen[r][c]=true;
      while (!q.empty()) {
        auto cur = q.front(); q.pop();
        int cr = cur.first, cc = cur.second;
        freeCount++;
        if (b.board[cr][cc].isTerminal && !b.board[cr][cc].hasPipe) termCount++;
        for (int k=0;k<4;k++){
          int nr = cr + dr[k], nc = cc + dc[k];
          if (nr<0||nc<0||nr>=N||nc>=N) continue;
          if (seen[nr][nc]) continue;
          if (b.board[nr][nc].hasPipe) { seen[nr][nc]=true; continue; }
          seen[nr][nc] = true;
          q.push({nr,nc});
        }
      }
      if (freeCount > 0 && termCount == 0) {
        return true;
      }
    }
  }
  return false;
}

struct ANode {
  int r,c;
  int g;    
  int h;    
  int f;     
  int pr, pc; 
  int dirFromParent;
};

struct AComp {
  bool operator()(const ANode& a, const ANode& b) const {
    if (a.f != b.f) return a.f > b.f;
    return a.g < b.g;
  }
};

bool creates1x1Hole(const Board &board, int nr, int nc) {
  int N = GRID;
  auto isFree = [&](int r,int c)->bool {
    if (r<0||c<0||r>=N||c>=N) return false;
    if (r==nr && c==nc) return false; 
    return !board.board[r][c].hasPipe;
  };
  int dr[4]={-1,0,0,1};
  int dc[4]={0,-1,1,0};
  for (int k=0;k<4;k++){
    int ar = nr + dr[k], ac = nc + dc[k];
    if (ar<0||ac<0||ar>=N||ac>=N) continue;
    if (!isFree(ar,ac)) continue;
    int freeAdj = 0;
    for (int t=0;t<4;t++){
      int br = ar + dr[t], bc = ac + dc[t];
      if (br<0||bc<0||br>=N||bc>=N) continue;
      if (isFree(br,bc)) freeAdj++;
    }
    if (freeAdj == 0) return true;
  }
  return false;
}

vector<pair<int,int>> AStarForPair(const Board &board, pair<int,int> start, pair<int,int> goal) {
  int N = GRID;
  int sr = start.first, sc = start.second;
  int tr = goal.first, tc = goal.second;
  int color = board.board[sr][sc].color;

  priority_queue<ANode, vector<ANode>, AComp> pq;
  const int SCALE = 100;

  vector<vector<int>> bestG(N, vector<int>(N, numeric_limits<int>::max()));
  vector<vector<pair<int,int>>> parent(N, vector<pair<int,int>>(N, {-1,-1}));
  vector<vector<int>> parentDir(N, vector<int>(N, 0));

  ANode startNode;
  startNode.r = sr; startNode.c = sc;
  startNode.g = 0;
  startNode.h = manhattan(sr, sc, tr, tc) * SCALE;
  startNode.f = startNode.g + startNode.h;
  startNode.pr = -1; startNode.pc = -1; startNode.dirFromParent = 0;

  pq.push(startNode);
  bestG[sr][sc] = 0;

  int dr[4] = {-1,0,0,1};
  int dc[4] = {0,-1,1,0};
  int dirId[4] = {1,2,3,4};

  while (!pq.empty()) {
    ANode cur = pq.top(); pq.pop();
    int r = cur.r, c = cur.c;
    if (cur.g != bestG[r][c]) continue;

    if (r == tr && c == tc) {
      vector<pair<int,int>> path;
      int cr = r, cc = c;
      while (!(cr == -1 && cc == -1)) {
        path.push_back({cr,cc});
        auto p = parent[cr][cc];
        int prr = p.first, pcc = p.second;
        if (prr == -1 && pcc == -1) break;
        cr = prr; cc = pcc;
      }
      reverse(path.begin(), path.end());
      return path;
    }

    for (int k=0;k<4;k++){
      int nr = r + dr[k], nc = c + dc[k];
      if (nr < 0 || nc < 0 || nr >= N || nc >= N) continue;
      if (board.board[nr][nc].hasPipe && !(nr==tr && nc==tc)) continue;
      if (board.board[nr][nc].isTerminal && board.board[nr][nc].color != color && !(nr==tr && nc==tc)) continue;

      int newG = cur.g + 1 * SCALE;
      int newH = manhattan(nr, nc, tr, tc) * SCALE;

      int bias = 0;

      if (creates1x1Hole(board, nr, nc)) {
        bias += 2000 * SCALE / 100;
      }

      Board tmp = board;
      tmp.board[nr][nc].hasPipe = true;
      tmp.board[nr][nc].color = color;
      if (hasDeadSpace(tmp)) {
        bias += 1500 * SCALE / 100;
      }

      int dir = dirId[k];
      if (cur.dirFromParent != 0 && cur.dirFromParent == dir) {
        bias -= 20;
      }

      if (nr == 0 || nc == 0 || nr == N-1 || nc == N-1) {
        bias -= 10;
      }

      int candidateF = newG + newH + bias;
      if (newG < bestG[nr][nc]) {
        bestG[nr][nc] = newG;
        parent[nr][nc] = {r,c};
        parentDir[nr][nc] = dir;
        ANode next;
        next.r = nr; next.c = nc;
        next.g = newG; next.h = newH; next.f = candidateF;
        next.pr = r; next.pc = c; next.dirFromParent = dir;
        pq.push(next);
      }
    }
  }

  return {};
}

int pairDifficulty(const Board &board, pair<int,int> a, pair<int,int> b) {
  int base = manhattan(a.first,a.second,b.first,b.second) * 100;
  int freer = 0;
  int dr[4] = {-1,0,0,1};
  int dc[4] = {0,-1,1,0};
  for (int i=0;i<4;i++){
    int r = a.first + dr[i], c = a.second + dc[i];
    if (r>=0 && c>=0 && r<GRID && c<GRID && !board.board[r][c].hasPipe) freer++;
    r = b.first + dr[i]; c = b.second + dc[i];
    if (r>=0 && c>=0 && r<GRID && c<GRID && !board.board[r][c].hasPipe) freer++;
  }
  return base - freer*10;
}

vector<pair<int,int>> algorithm(const Board &board) {
  vector<vector<int>> terms = getTerminals(board);
  if (terms.empty()) return {};

  unordered_map<int, vector<pair<int,int>>> groups;
  for (auto &t : terms) {
    int color = t[0], r = t[1], c = t[2];
    groups[color].push_back({r,c});
  }

  struct Candidate { int color; pair<int,int> a,b; int diff; };
  vector<Candidate> cand;
  for (auto &kv : groups) {
    if (kv.second.size() != 2) continue;
    Candidate cc;
    cc.color = kv.first;
    cc.a = kv.second[0];
    cc.b = kv.second[1];
    cc.diff = pairDifficulty(board, cc.a, cc.b);
    cand.push_back(cc);
  }

  if (cand.empty()) return {};

  sort(cand.begin(), cand.end(), [](const Candidate &x, const Candidate &y){
    return x.diff < y.diff;
  });

  for (auto &c : cand) {
    cout << "AI: trying color " << c.color
         << " endpoints (" << c.a.first << "," << c.a.second << ") <- -> ("
         << c.b.first << "," << c.b.second << "), difficulty=" << c.diff << "\n";
    auto p1 = AStarForPair(board, c.a, c.b);
    auto p2 = AStarForPair(board, c.b, c.a);

    cout << "AI: A* results for color " << c.color
         << " p1.size=" << p1.size() << " p2.size=" << p2.size() << "\n";

    vector<pair<int,int>> chosen;
    if (!p1.empty() && !p2.empty()) {
      if (p1.size() <= p2.size()) chosen = p1; else chosen = p2;
    } else if (!p1.empty()) chosen = p1;
    else if (!p2.empty()) chosen = p2;

    if (!chosen.empty()) {
      return chosen;
    }
  }

  return {};
}
