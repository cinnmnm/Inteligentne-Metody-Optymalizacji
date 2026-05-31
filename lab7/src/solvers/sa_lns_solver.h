#pragma once

#include "../../../lab4/src/solvers/lns_solver.h"

class SA_LNS_Solver final : public LNS_Solver {
public:
    SA_LNS_Solver(int seed, int max_time_ms, double destroy_fraction, 
                  double initial_temperature, double cooling_rate);

    SolveResult solve(const Instance& instance, int start_node) override;

private:
    double initial_temperature_;
    double cooling_rate_;
};
