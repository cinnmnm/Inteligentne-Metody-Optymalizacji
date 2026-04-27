#include "list_memory_local_search_solver.h"

#include <algorithm>
#include <chrono>

ListMemoryLocalSearchSolver::ListMemoryLocalSearchSolver(const int seed, const int max_time_ms, const int candidate_k)
    : Lab3BaseSolver(seed, max_time_ms, candidate_k) {}

SolveResult ListMemoryLocalSearchSolver::solve(const Instance& instance, const int start_node) {
    const auto t0 = std::chrono::high_resolution_clock::now();

    RouteState state = buildInitialState(instance, start_node);
    const std::vector<int> initial_route = state.route;

    int current_distance = computeRouteDistance(instance, state.route);
    int current_profit = computeRouteProfit(instance, state.route);

    SolveResult result;
    result.initial_path = initial_route;
    result.phase1_distance = current_distance;
    result.phase1_profit = current_profit;
    result.phase1_objective = current_profit - current_distance;

    const EvalConfig config{false};
    std::vector<MemoryMove> lm;

    while (true) {
        const double elapsed_ms = std::chrono::duration<double, std::milli>(
            std::chrono::high_resolution_clock::now() - t0).count();
        if (elapsed_ms >= static_cast<double>(max_time_ms_)) {
            break;
        }

        if (lm.empty()) {
            lm = collectImprovingMoves(instance, state, config);
            if (lm.empty()) {
                break;
            }
            std::sort(lm.begin(), lm.end(), [](const MemoryMove& lhs, const MemoryMove& rhs) {
                return lhs.delta < rhs.delta;
            });
        }

        bool applied = false;
        size_t lm_idx = 0;
        while (lm_idx < lm.size()) {
            MemoryMove move = lm[lm_idx++];

            if (!isMoveApplicable(move, state)) {
                continue;
            }

            const double current_delta = evaluateMoveDeltaCurrent(instance, state, move);
            if (current_delta >= 0.0) {
                continue;
            }
            move.delta = current_delta;

            if (applyMove(state, move)) {
                applied = true;
                break;
            }
        }
        
        if (lm_idx >= lm.size()) {
            lm.clear();
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
