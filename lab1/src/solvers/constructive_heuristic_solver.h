#pragma once

#include "solvers/base_solver.h"
#include <vector>

class ConstructiveHeuristicSolver : public BaseSolver {
protected:
    void runPhaseTwo(std::vector<int>& path, int& current_dist, int& current_profit, const Instance& instance);
};