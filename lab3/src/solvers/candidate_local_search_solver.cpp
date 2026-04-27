#include "candidate_local_search_solver.h"

#include <algorithm>
#include <chrono>

CandidateLocalSearchSolver::CandidateLocalSearchSolver(const int seed, const int max_time_ms, const int candidate_k)
    : Lab3BaseSolver(seed, max_time_ms, candidate_k) {}

SolveResult CandidateLocalSearchSolver::solve(const Instance& instance, const int start_node) {
    const auto t0 = std::chrono::high_resolution_clock::now();

    initializeCandidateMatrix(instance);
    RouteState state = buildInitialState(instance, start_node);
    const std::vector<int> initial_route = state.route;

    int current_distance = computeRouteDistance(instance, state.route);
    int current_profit = computeRouteProfit(instance, state.route);

    SolveResult result;
    result.initial_path = initial_route;
    result.phase1_distance = current_distance;
    result.phase1_profit = current_profit;
    result.phase1_objective = current_profit - current_distance;

    const EvalConfig config{true};
    while (true) {
        const double elapsed_ms = std::chrono::duration<double, std::milli>(
            std::chrono::high_resolution_clock::now() - t0).count();
        if (elapsed_ms >= static_cast<double>(max_time_ms_)) {
            break;
        }

        std::vector<MemoryMove> improving_moves = collectImprovingMoves(instance, state, config);
        if (improving_moves.empty()) {
            break;
        }

        const auto best_it = std::min_element(
            improving_moves.begin(),
            improving_moves.end(),
            [](const MemoryMove& lhs, const MemoryMove& rhs) {
                return lhs.delta < rhs.delta;
            });

        if (best_it == improving_moves.end() || best_it->delta >= 0.0) {
            break;
        }

        if (!applyMove(state, *best_it)) {
            break;
        }
    }

    current_distance = computeRouteDistance(instance, state.route);
    current_profit = computeRouteProfit(instance, state.route);

    result.path = state.route;
    result.phase2_distance = current_distance;
    result.phase2_profit = current_profit;
    result.final_objective = current_profit - current_distance;
    result.time_ms = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();

    return result;
}
