#include "board.h"
#include "globals.h"

#ifndef BSE_GET_TERMINALS_H
#define BSE_GET_TERMINALS_H

class base_get_terminals {
public:

  virtual ~base_get_terminals() = default;
  virtual string get_name() { return "Base get terminals"; }
  virtual vector<pair<pair<int, int>, pair<int, int>>>
  getTerminals(Board board)=0;
};

#endif
