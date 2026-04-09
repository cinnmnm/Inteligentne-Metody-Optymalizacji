#pragma once

#include "constructive_heuristic_solver.h"

class NearestNeighborSolver final : public ConstructiveHeuristicSolver {
public:
    explicit NearestNeighborSolver(bool consider_profit, int seed = 0);
    
    SolveResult solve(const Instance& instance, int start_node) override;

private:
    bool consider_profit_;
};