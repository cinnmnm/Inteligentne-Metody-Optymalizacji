#pragma once

#include "../../../lab3/src/solvers/lab3_base_solver.h"
#include "../../../common/include/solve_result.h"

class ILS_Solver final : public Lab3BaseSolver {
public:
    explicit ILS_Solver(int seed, int max_time_ms = 1000, int perturb_size = 3);

    SolveResult solve(const Instance& instance, int start_node) override;

private:
    int max_time_ms_;
    int perturb_size_;

    void smallPerturbation(Lab3BaseSolver::RouteState& state);
    void localSearchImprove(const Instance& instance, RouteState& state);
};
