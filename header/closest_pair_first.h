#include "base_get_terminals.h"
#include "globals.h"

#ifndef CLOSEST_PAIR_FIRST_H
#define CLOSEST_PAIR_FIRST_H

class closest_pair_first :public base_get_terminals {
private:
  string name = "closest pair first";

public:
  string get_name() override { return name; }
  vector<pair<pair<int, int>, pair<int, int>>>
  getTerminals(Board board) override;

private:
  unordered_map<int, pair<pair<int, int>, pair<int, int>>>
  get_color_terminals(const Board &board);
  int distance_manhattan(const pair<int, int> &a, const pair<int, int> &b);

  map<int, vector<int>> get_distance_color(
      unordered_map<int, pair<pair<int, int>, pair<int, int>>> terminals);
};

#endif
