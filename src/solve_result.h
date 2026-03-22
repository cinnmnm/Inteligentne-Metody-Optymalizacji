#pragma once

#include <string>
#include <vector>

struct SolveResult {
    std::vector<int> path;
    int phase1_distance = 0;
    int phase1_profit = 0;
    int phase1_objective = 0;
    int phase2_distance = 0;
    int phase2_profit = 0;
    int final_objective = 0;
    double time_ms = 0.0;
    bool is_valid = true;
    std::string error_message;
};
