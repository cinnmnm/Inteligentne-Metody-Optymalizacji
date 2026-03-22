#pragma once

#include "instance.h"
#include "solve_result.h"

class SolutionChecker {
public:
    static void validate(const Instance& instance, SolveResult& result);
};
