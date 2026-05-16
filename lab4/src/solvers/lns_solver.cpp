#include "lns_solver.h"

#include <algorithm>
#include <chrono>
#include <random>
#include <limits>

LNS_Solver::LNS_Solver(const int seed, const int max_time_ms, const double destroy_fraction)
    : Lab3BaseSolver(seed), max_time_ms_(max_time_ms), destroy_fraction_(destroy_fraction) {}

void LNS_Solver::destroy(Lab3BaseSolver::RouteState& state, std::vector<int>& removed_nodes) {
    removed_nodes.clear();
    const int m = static_cast<int>(state.route.size()) - 1;
    if (m <= 3) return;

    int k = std::max(1, static_cast<int>(std::round((m - 1) * destroy_fraction_)));

    int num_chunks = 3; 
    int chunk_size = std::max(1, k / num_chunks);

    for (int chunk = 0; chunk < num_chunks && !state.route.empty(); ++chunk) {
        int current_m = static_cast<int>(state.route.size()) - 1;
        if (current_m <= 1) break;
        
        std::uniform_int_distribution<int> start_dist(1, current_m - 1);
        int start_pos = start_dist(rng_);

        int to_remove_here = std::min(chunk_size, current_m - 1);
        for (int i = 0; i < to_remove_here; ++i) {
            int current_idx = start_pos;
            if (current_idx >= static_cast<int>(state.route.size()) - 1) {
                current_idx = 1; 
            }
            removed_nodes.push_back(state.route[static_cast<size_t>(current_idx)]);
            state.route.erase(state.route.begin() + current_idx);
        }
    }
    
    state.route.back() = state.route.front();
    rebuildRouteCaches(state);
}

void LNS_Solver::repair(const Instance& instance, Lab3BaseSolver::RouteState& state, const std::vector<int>& removed_nodes) {
    std::vector<int> unplaced = removed_nodes;

    while (!unplaced.empty()) {
        const int m = static_cast<int>(state.route.size()) - 1;
        double max_regret = -1.0;
        int best_node_idx = -1;
        int best_pos = -1;

        for (size_t i = 0; i < unplaced.size(); ++i) {
            int node = unplaced[i];
            double best_delta = std::numeric_limits<double>::infinity();
            double second_best_delta = std::numeric_limits<double>::infinity();
            int local_best_pos = 0;

            for (int edge = 0; edge < m; ++edge) {
                const int u = state.route[static_cast<size_t>(edge)];
                const int w = state.route[static_cast<size_t>((edge + 1) % m)];
                const int added_dist = instance.getDistance(u, node) + instance.getDistance(node, w) - instance.getDistance(u, w);
                const int added_profit = instance.getProfit(node);
                const double delta = static_cast<double>(added_dist - added_profit);

                if (delta < best_delta) {
                    second_best_delta = best_delta;
                    best_delta = delta;
                    local_best_pos = edge;
                } else if (delta < second_best_delta) {
                    second_best_delta = delta;
                }
            }

            double regret = second_best_delta - best_delta;
            if (regret > max_regret) {
                max_regret = regret;
                best_node_idx = static_cast<int>(i);
                best_pos = local_best_pos;
            }
        }

        state.route.insert(state.route.begin() + best_pos + 1, unplaced[best_node_idx]);
        unplaced.erase(unplaced.begin() + best_node_idx);
    }
    state.route.back() = state.route.front();
    rebuildRouteCaches(state);
}

void LNS_Solver::localSearchImprove(const Instance& instance, RouteState& state) {
    const EvalConfig config{false};
    std::vector<MemoryMove> lm = collectImprovingMoves(instance, state, config);
    if (!lm.empty()) std::sort(lm.begin(), lm.end(), [](const MemoryMove& a, const MemoryMove& b) { return a.delta < b.delta; });

    while (!lm.empty()) {
        bool applied = false;
        size_t idx = 0;
        while (idx < lm.size()) {
            MemoryMove move = lm[idx++];
            if (!isMoveApplicable(move, state)) continue;
            double cur_delta = evaluateMoveDeltaCurrent(instance, state, move);
            if (cur_delta >= 0.0) continue;
            move.delta = cur_delta;
            if (applyMove(state, move)) { applied = true; break; }
        }
        if (!applied) break;
        lm = collectImprovingMoves(instance, state, config);
        if (!lm.empty()) std::sort(lm.begin(), lm.end(), [](const MemoryMove& a, const MemoryMove& b) { return a.delta < b.delta; });
    }
}

SolveResult LNS_Solver::solve(const Instance& instance, const int start_node) {
    const auto t0 = std::chrono::high_resolution_clock::now();

    Lab3BaseSolver::RouteState x = buildInitialState(instance, start_node);
    {
        const EvalConfig config{false};
        std::vector<MemoryMove> lm = collectImprovingMoves(instance, x, config);
        if (!lm.empty()) std::sort(lm.begin(), lm.end(), [](const MemoryMove& a, const MemoryMove& b) { return a.delta < b.delta; });
        while (!lm.empty()) {
            bool applied = false;
            size_t idx = 0;
            while (idx < lm.size()) {
                MemoryMove move = lm[idx++];
                if (!isMoveApplicable(move, x)) continue;
                double cur_delta = evaluateMoveDeltaCurrent(instance, x, move);
                if (cur_delta >= 0.0) continue;
                move.delta = cur_delta;
                if (applyMove(x, move)) { applied = true; break; }
            }
            if (!applied) break;
            lm = collectImprovingMoves(instance, x, config);
            if (!lm.empty()) std::sort(lm.begin(), lm.end(), [](const MemoryMove& a, const MemoryMove& b) { return a.delta < b.delta; });
        }
    }

    int x_dist = computeRouteDistance(instance, x.route);
    int x_profit = computeRouteProfit(instance, x.route);
    int x_obj = x_profit - x_dist;

    SolveResult best;
    best.initial_path = x.route;
    best.path = x.route;
    best.phase1_distance = x_dist;
    best.phase1_profit = x_profit;
    best.phase2_distance = x_dist;
    best.phase2_profit = x_profit;
    best.final_objective = x_obj;

    int perturb_count = 0;
    while (true) {
        const double elapsed_ms = std::chrono::duration<double, std::milli>(
            std::chrono::high_resolution_clock::now() - t0).count();
        if (elapsed_ms >= static_cast<double>(max_time_ms_)) break;

        Lab3BaseSolver::RouteState y = x;
        std::vector<int> removed;
        destroy(y, removed);
        if (removed.empty()) break;
        repair(instance, y, removed);
        localSearchImprove(instance, y);

        int y_dist = computeRouteDistance(instance, y.route);
        int y_profit = computeRouteProfit(instance, y.route);
        int y_obj = y_profit - y_dist;

        ++perturb_count;
        
        x = std::move(y);
        x_dist = y_dist;
        x_profit = y_profit;
        x_obj = y_obj;

        if (x_obj > best.final_objective) {
            best.path = x.route;
            best.phase2_distance = x_dist;
            best.phase2_profit = x_profit;
            best.final_objective = x_obj;
        }
    }

    best.time_ms = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();
    best.iterations = perturb_count;
    return best;
}
