
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <math.h>
#include <queue>
#include <random>
#include <raylib.h>
#include <sstream>
#include <stdbool.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>
#include <unordered_set>
#include <mutex>
#ifndef GLOBAL_H
#define GLOBAL_H

using namespace std;
extern mutex boardMutex;
extern int GRID;
extern unordered_map<int, Color> color_map;
const double REGION_THRESHOLD = 0.5;
#endif
