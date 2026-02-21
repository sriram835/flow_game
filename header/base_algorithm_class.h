#include <board.h>
#include <globals.h>

#ifndef BASE_ALGORITHM
#define BASE_ALGORITHM

class base_algorithm {
public:
  virtual string get_algo_name() const { return "Base algorithm class"; }
  virtual vector<pair<int, int>> algorithm(Board board);
  vector<pair<pair<int, int>, pair<int, int>>> (*getTerminals)(Board boards);
};

#endif
