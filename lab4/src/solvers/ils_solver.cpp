#include "ils_solver.h"

#include <algorithm>
#include <chrono>
#include <random>
#include <limits>

ILS_Solver::ILS_Solver(const int seed, const int max_time_ms, const int perturb_size)
    : Lab3BaseSolver(seed), max_time_ms_(max_time_ms), perturb_size_(perturb_size) {}

void ILS_Solver::smallPerturbation(Lab3BaseSolver::RouteState& state) {
    const int m = static_cast<int>(state.route.size()) - 1;
    if (m <= 4) return;

    std::vector<int> unvisited;
    for (size_t i = 0; i < state.is_visited.size(); ++i) {
        if (!state.is_visited[i]) unvisited.push_back(static_cast<int>(i));
    }
    
    int swaps = std::min(2, static_cast<int>(unvisited.size()));
    if (swaps > 0) {
        std::shuffle(unvisited.begin(), unvisited.end(), rng_);
        for (int i = 0; i < swaps; ++i) {
            std::uniform_int_distribution<int> dist(1, m - 1);
            int idx = dist(rng_);
            int old_node = state.route[static_cast<size_t>(idx)];
            int new_node = unvisited[static_cast<size_t>(i)];
            
            state.route[static_cast<size_t>(idx)] = new_node;
            state.is_visited[static_cast<size_t>(old_node)] = false;
            state.is_visited[static_cast<size_t>(new_node)] = true;
        }
    }

    std::vector<int> removed;
    int k = std::min(perturb_size_, m - 1);
    for(int i = 0; i < k; ++i) {
        int current_m = static_cast<int>(state.route.size()) - 1;
        std::uniform_int_distribution<int> dist(1, current_m - 1);
        int idx = dist(rng_);
        removed.push_back(state.route[static_cast<size_t>(idx)]);
        state.route.erase(state.route.begin() + idx);
    }
    
    for(int node : removed) {
        int current_m = static_cast<int>(state.route.size()) - 1;
        std::uniform_int_distribution<int> pos_dist(0, current_m - 1);
        int pos = pos_dist(rng_);
        state.route.insert(state.route.begin() + pos + 1, node);
    }

    state.route.back() = state.route.front();
    rebuildRouteCaches(state);
}

void ILS_Solver::localSearchImprove(const Instance& instance, RouteState& state) {
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
            if (applyMove(state, move)) {
                applied = true;
                break;
            }
        }
        if (!applied) break;
        lm = collectImprovingMoves(instance, state, config);
        if (!lm.empty()) std::sort(lm.begin(), lm.end(), [](const MemoryMove& a, const MemoryMove& b) { return a.delta < b.delta; });
    }
}

SolveResult ILS_Solver::solve(const Instance& instance, const int start_node) {
    const auto t0 = std::chrono::high_resolution_clock::now();

    Lab3BaseSolver::RouteState x = buildInitialState(instance, start_node);
    localSearchImprove(instance, x);

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
    int non_improve_count = 0;

    while (true) {
        const double elapsed_ms = std::chrono::duration<double, std::milli>(
            std::chrono::high_resolution_clock::now() - t0).count();
        if (elapsed_ms >= static_cast<double>(max_time_ms_)) break;

        Lab3BaseSolver::RouteState y = x;
        smallPerturbation(y);
        localSearchImprove(instance, y);

        int y_dist = computeRouteDistance(instance, y.route);
        int y_profit = computeRouteProfit(instance, y.route);
        int y_obj = y_profit - y_dist;

        ++perturb_count;
        
        if (y_obj > best.final_objective) {
            best.path = y.route;
            best.phase2_distance = y_dist;
            best.phase2_profit = y_profit;
            best.final_objective = y_obj;
        }

        if (y_obj > x_obj) {
            x = std::move(y);
            x_dist = y_dist;
            x_profit = y_profit;
            x_obj = y_obj;
            non_improve_count = 0;
        } else {
            non_improve_count++;
            
            if (non_improve_count >= 15) {
                if (y_obj < best.final_objective * 0.90) {
                    x.route = best.path;
                    x.is_visited.assign(instance.getNumVertices(), false);
                    for(size_t i = 0; i < x.route.size() - 1; ++i) {
                        x.is_visited[static_cast<size_t>(x.route[i])] = true;
                    }
                    rebuildRouteCaches(x);
                    x_dist = best.phase2_distance;
                    x_profit = best.phase2_profit;
                    x_obj = best.final_objective;
                } else {
                    x = std::move(y);
                    x_dist = y_dist;
                    x_profit = y_profit;
                    x_obj = y_obj;
                }
                non_improve_count = 0;
            }
        }
    }

    best.time_ms = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();
    best.iterations = perturb_count;
    return best;
}