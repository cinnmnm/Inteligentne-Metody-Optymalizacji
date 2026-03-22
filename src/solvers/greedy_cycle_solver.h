#pragma once

#include "solvers/constructive_heuristic_solver.h"

class GreedyCycleSolver final : public ConstructiveHeuristicSolver {
public:
    explicit GreedyCycleSolver(bool consider_profit, int seed = 0);
    
    SolveResult solve(const Instance& instance, int start_node) override;

private:
    bool consider_profit_;
};