#pragma once

#include "../../../lab3/src/solvers/lab3_base_solver.h"
#include "../../../common/include/solve_result.h"

class MSLS_Solver final : public Lab3BaseSolver {
public:
    explicit MSLS_Solver(int seed, int iterations = 200);

    SolveResult solve(const Instance& instance, int start_node) override;

private:
    int iterations_;
    void performLocalSearchImprove(const Instance& instance, RouteState& state, const EvalConfig& config);
};
