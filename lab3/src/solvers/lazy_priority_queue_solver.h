#pragma once

#include "lab3_base_solver.h"

class LazyPriorityQueueLocalSearchSolver final : public Lab3BaseSolver {
public:
    explicit LazyPriorityQueueLocalSearchSolver(int seed, int max_time_ms = 1000, int candidate_k = 10);

    SolveResult solve(const Instance& instance, int start_node) override;
};
