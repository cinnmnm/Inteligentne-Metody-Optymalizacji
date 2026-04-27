#include "lazy_priority_queue_solver.h"

#include <chrono>
#include <queue>
#include <vector>

LazyPriorityQueueLocalSearchSolver::LazyPriorityQueueLocalSearchSolver(
    const int seed,
    const int max_time_ms,
    const int candidate_k)
    : Lab3BaseSolver(seed, max_time_ms, candidate_k) {}

SolveResult LazyPriorityQueueLocalSearchSolver::solve(const Instance& instance, const int start_node) {
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

    struct MemoryMoveMinComparator {
        bool operator()(const MemoryMove& lhs, const MemoryMove& rhs) const {
            return lhs.delta > rhs.delta;
        }
    };

    const EvalConfig config{false};
    std::priority_queue<MemoryMove, std::vector<MemoryMove>, MemoryMoveMinComparator> pq;

    while (true) {
        const double elapsed_ms = std::chrono::duration<double, std::milli>(
            std::chrono::high_resolution_clock::now() - t0).count();
        if (elapsed_ms >= static_cast<double>(max_time_ms_)) {
            break;
        }

        if (pq.empty()) {
            const std::vector<MemoryMove> improving_moves = collectImprovingMoves(instance, state, config);
            for (const MemoryMove& move : improving_moves) {
                if (move.delta < 0.0) {
                    pq.push(move);
                }
            }
            if (pq.empty()) {
                break;
            }
        }

        MemoryMove move = pq.top();
        pq.pop();

        if (!isMoveApplicable(move, state)) {
            continue;
        }

        const double new_delta = evaluateMoveDeltaCurrent(instance, state, move);
        if (new_delta >= 0.0) {
            continue;
        }

        if (!pq.empty() && new_delta > pq.top().delta) {
            move.delta = new_delta;
            pq.push(move);
            continue;
        }

        move.delta = new_delta;
        applyMove(state, move);
    }

    current_distance = computeRouteDistance(instance, state.route);
    current_profit = computeRouteProfit(instance, state.route);

    result.path = state.route;
    result.phase2_distance = current_distance;
    result.phase2_profit = current_profit;
    result.final_objective = current_profit - current_distance;
    result.time_ms = std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - t0).count();

    return result;
}
