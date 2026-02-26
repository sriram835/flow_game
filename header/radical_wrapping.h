#include "base_algorithm_class.h"

#ifndef RADICAL_WRAPPING_H
#define RADICAL_WRAPPING_H

class radical_wrapping : public base_algorithm {
private:
  string algo_name = "Radical wrapping";
  double calculateRadialWeight(int row, int col, int gridSize);
  vector<pair<int, int>> dijkstraPath(const Board &board, int startRow,
                                      int startCol, int endRow, int endCol);

public:
  explicit radical_wrapping(base_get_terminals* obj)
        : base_algorithm(obj) {}
  vector<pair<int, int>> algorithm(const Board &board) override;
  string get_algo_name() override { return algo_name; }
};

#endif
