#include "half_split.h"
#include "globals.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <tuple>
#include <unordered_map>

using namespace std;

unordered_map<int, pair<pair<int, int>, pair<int, int>>> terminalMap;

void fillTerminalMap(const Board& board)
{
    for (int i = 0; i < GRID; i++) {
        for (int j = 0; j < GRID; j++)
        {
            if (terminalMap.find(board.board[i][j].color) != terminalMap.end())
            {
                if (terminalMap[board.board[i][j].color].first.first == -1)
                {
                    terminalMap[board.board[i][j].color].first = {i, j};
                }
                else
                {
                    terminalMap[board.board[i][j].color].second = {i, j};
                }
            }
            else
            {
                terminalMap[board.board[i][j].color] = {{-1, -1}, {-1, -1}};
                terminalMap[board.board[i][j].color].first = {i, j};
            }
        }
    }
}

vector<pair<int,int>> half_split_algorithm(const Board& board)
{
    fillTerminalMap(board);
    for (auto cell : terminalMap)
    {
        pair<int, int> start = cell.second.first;
        pair<int, int> end = cell.second.second;

        cout << "Color: " << cell.first << " Start: (" << start.first << ", " << start.second << ") End: (" << end.first << ", " << end.second << ")" << endl;
    }

    return {};
}




typedef vector<pair<int,int>> path;
vector<vector<bool>> visited (GRID, vector<bool> (GRID, false));
vector<path> validPaths;

int counter = 0;

pair<pair<pair<int, int>, pair<int, int>>, pair<pair<int, int>, pair<int, int>>> 
findHalf(const Board& board, pair<int, int> top_left, pair<int, int> bottom_right)
{
    int a = top_left.first;
    int b = top_left.second;

    int c = bottom_right.first;
    int d = bottom_right.second;

    if (counter % 2 == 0)
    {
        return {
            {{a, b}, {(a + c) / 2, d}}, 
            {{(a + c) / 2 + 1, b}, {c, d}}
        };
    }
    else
    {
        return {
            {{a, b}, {c, (b + d) / 2}}, 
            {{a, (b + d) / 2 + 1}, {c, d}}
        };
    }
}

bool isInside(pair<int, int> point, pair<pair<int, int>, pair<int, int>> bounds) {
    int r = point.first;
    int c = point.second;
    return (r >= bounds.first.first && r <= bounds.second.first &&
            c >= bounds.first.second && c <= bounds.second.second);
}

void half_split_recursive_solver(const Board& board, pair<int, int> top_left, pair<int,int> bottom_right)
{
    auto [firstHalf, secondHalf]= findHalf(board, top_left, bottom_right);

    // base case

    //straddlgin step
    for (auto const& [color, terminals] : terminalMap)
    {
        pair<int, int> p1 = terminals.first;
        pair<int, int> p2 = terminals.second;

        bool p1_in_first = isInside(p1, firstHalf);
        bool p2_in_first = isInside(p2, firstHalf);
        
        bool p1_in_second = isInside(p2, secondHalf); 
        bool p2_in_second = isInside(p1, secondHalf);

        if ((p1_in_first && p2_in_second) || (p2_in_first && p1_in_second))
        {
            cout << "Color " << color << " straddles the current split!" << endl;
        }
    }

    //recursive case
    counter++;
    half_split_recursive_solver(board, firstHalf.first, firstHalf.second);
    half_split_recursive_solver(board, secondHalf.first, secondHalf.second);

}   

