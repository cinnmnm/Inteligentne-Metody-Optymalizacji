#pragma once

#include "instance.h"
#include "solve_result.h"

class BaseSolver {
public:
    virtual ~BaseSolver() = default;
    virtual SolveResult solve(const Instance& instance, int start_node) = 0;
};
