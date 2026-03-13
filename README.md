# 23CSE211 - Design and Analysis of Algorithms - Lab Evaluation

This repository, and the attached repository contains the implementation of various strategies demonstarted for the 3-phase lab evaluation of the course 23CSE211 by Dr. Vidya Balasubramainian.

## Flow Free
Flow-free is a grid based game, where in an n by n grid, there are k pairs of colours (or
terminals), which has to be connected by drawing pipes on the grid. No two pipes must
intersect, and they should fill the grid as much as possible. 

## Team Members
- Akilan S S (CB.SC.U4CSE24707)
- Govind Nair (CB.SC.U4CSE24720)
- M V Sai Kartik (CB.SC.U4CSE24744)
- Sriram Tatikonda (CB.SC.U4CSE24753)

## Strategies Implemented
### Phase 1
1. Naive BFS
2. Sorted Euclidean Distances
3. A* Heuristics
### Phase 2
1. Radial Weighted Greedy
2. Naive Divide and Conquer
3. Uneven Region with Path Finding Algorithm
4. BFS with Bending through Region Finding
5. Dynamic Uneven Region Segmentation Algorithm
### Phase 3
1. Dynamic Programming - Bellman Ford + Bitmasking
2. Dynamic Programming with A* + Bitmasking
3. Backtracking
4. Backtracking (Maximum Bends Variant) Access here: [Zig-Solver](https://github.com/AkilanSS/zig-flow-free-solver)


## Documentation and Testing
The entire project is documented as report, and can be accessed from the root directory of the main branch

To aid us in testing our puzzle, a small python test case generator was developed to generate various levels and can be accessed from this repository -> [Flow-Free-Test-Suite-Gen](https://github.com/AkilanSS/flow-game-test-suite)

## How to Use
The master branch simply holds the template of the raylib project. To access implementations of different strategies, switch between branches. The variant solver is in a seperate repository whose link is provided above.