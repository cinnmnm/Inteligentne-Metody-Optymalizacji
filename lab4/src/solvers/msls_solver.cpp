#include "msls_solver.h"

#include <algorithm>
#include <chrono>
#include <limits>

MSLS_Solver::MSLS_Solver(const int seed, const int iterations)
    : Lab3BaseSolver(seed), iterations_(iterations) {}

void MSLS_Solver::performLocalSearchImprove(const Instance& instance, RouteState& state, const EvalConfig& config) {
    std::vector<MemoryMove> lm = collectImprovingMoves(instance, state, config);
    if (!lm.empty()) {
        std::sort(lm.begin(), lm.end(), [](const MemoryMove& a, const MemoryMove& b) { return a.delta < b.delta; });
    }

    while (!lm.empty()) {
        bool applied = false;
        size_t idx = 0;
        while (idx < lm.size()) {
            MemoryMove move = lm[idx++];
            if (!isMoveApplicable(move, state)) continue;
            double cur_delta = evaluateMoveDeltaCurrent(instance, state, move);
            if (cur_delta >= 0.0) continue;
            move.delta = cur_delta;
            if (applyMove(state, move)) {
                applied = true;
            }
        }
        if (!applied) break;
        lm = collectImprovingMoves(instance, state, config);
        if (!lm.empty()) std::sort(lm.begin(), lm.end(), [](const MemoryMove& a, const MemoryMove& b) { return a.delta < b.delta; });
    }
}

SolveResult MSLS_Solver::solve(const Instance& instance, const int start_node) {
    const auto t0 = std::chrono::high_resolution_clock::now();
    (void)start_node;

    initializeCandidateMatrix(instance);

    SolveResult best_result;
    best_result.final_objective = std::numeric_limits<int>::min();

    const int vertex_count = instance.getNumVertices();
    std::uniform_int_distribution<int> start_dist(0, std::max(0, vertex_count - 1));

    for (int it = 0; it < iterations_; ++it) {
        const int restart_start_node = start_dist(rng_);
        Lab3BaseSolver::RouteState state = buildInitialState(instance, restart_start_node);
        const std::vector<int> initial_route = state.route;

        int current_distance = computeRouteDistance(instance, state.route);
        int current_profit = computeRouteProfit(instance, state.route);

        const EvalConfig config{true};
        performLocalSearchImprove(instance, state, config);

        current_distance = computeRouteDistance(instance, state.route);
        current_profit = computeRouteProfit(instance, state.route);

        const int objective = current_profit - current_distance;
        if (objective > best_result.final_objective) {
            best_result.initial_path = initial_route;
            best_result.path = state.route;
            best_result.phase1_distance = current_distance;
            best_result.phase1_profit = current_profit;
            best_result.phase2_distance = current_distance;
            best_result.phase2_profit = current_profit;
            best_result.final_objective = objective;
        }
    }

    best_result.time_ms = std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - t0).count();
    best_result.iterations = iterations_;
    return best_result;
}
