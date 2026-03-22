#pragma once

#include "solvers/constructive_heuristic_solver.h"

class RegretSolver final : public ConstructiveHeuristicSolver {
public:
    explicit RegretSolver(bool consider_profit, double weight_regret, double weight_greedy, int seed = 0);
    
    SolveResult solve(const Instance& instance, int start_node) override;

private:
    bool consider_profit_;
    double weight_regret_;
    double weight_greedy_;
};