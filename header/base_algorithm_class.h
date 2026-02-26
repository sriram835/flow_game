#include "board.h"
#include "globals.h"
#include "base_get_terminals.h"

#ifndef BASE_ALGORITHM_H
#define BASE_ALGORITHM_H

class base_algorithm {
protected:
    base_get_terminals* get_terminals_obj;

public:
    explicit base_algorithm(base_get_terminals* obj)
        : get_terminals_obj(obj)
    {
        if (!get_terminals_obj) {
            throw std::invalid_argument("get_terminals_obj cannot be null");
        }
    }

    virtual ~base_algorithm() = default;

    virtual string get_algo_name() {
        return "Base algorithm class";
    }

    virtual vector<pair<int, int>> algorithm(const Board& board) = 0;
};

/*
class base_algorithm {
public:
  using GetTerminalsFn =
      std::vector<std::pair<std::pair<int, int>, std::pair<int, int>>> (*)(
          Board);

  base_algorithm(GetTerminalsFn fn) : getTerminals(fn) {
    if (!getTerminals) {
      throw std::invalid_argument("getTerminals function cannot be null");
    }
  }
  virtual string get_algo_name() { return "Base algorithm class"; }
  virtual vector<pair<int, int>> algorithm(const Board &board);
  vector<pair<pair<int, int>, pair<int, int>>> (*getTerminals)(Board boards);

};
*/

#endif
