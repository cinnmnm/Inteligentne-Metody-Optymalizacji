#pragma once

#include "solvers/base_solver.h"

#include <random>

class RandomSolver final : public BaseSolver {
public:
    explicit RandomSolver(int seed);
    SolveResult solve(const Instance& instance, int start_node) override;

private:
    std::mt19937 rng_;
};
