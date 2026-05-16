#pragma once

#include "../../../lab3/src/solvers/lab3_base_solver.h"
#include "../../../common/include/solve_result.h"

class LNS_Solver final : public Lab3BaseSolver {
public:
    explicit LNS_Solver(int seed, int max_time_ms = 1000, double destroy_fraction = 0.3);

    SolveResult solve(const Instance& instance, int start_node) override;

private:
    int max_time_ms_;
    double destroy_fraction_;

    void destroy(Lab3BaseSolver::RouteState& state, std::vector<int>& removed_nodes);
    void repair(const Instance& instance, Lab3BaseSolver::RouteState& state, const std::vector<int>& removed_nodes);
    void localSearchImprove(const Instance& instance, RouteState& state);
};
