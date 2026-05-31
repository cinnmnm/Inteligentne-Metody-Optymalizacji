#pragma once

#include "solvers/base_solver.h"
#include "solve_result.h"

class ParallelUHMASolver final : public BaseSolver {
public:
    ParallelUHMASolver(int seed, int max_time_ms, int num_threads);

    SolveResult solve(const Instance& instance, int start_node) override;

private:
    int seed_;
    int max_time_ms_;
    int num_threads_;
};
