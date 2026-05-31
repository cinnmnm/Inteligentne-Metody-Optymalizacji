#include "sa_lns_solver.h"

#include <algorithm>
#include <chrono>
#include <random>
#include <cmath>

SA_LNS_Solver::SA_LNS_Solver(const int seed, const int max_time_ms, const double destroy_fraction,
                             const double initial_temperature, const double cooling_rate)
    : LNS_Solver(seed, max_time_ms, destroy_fraction, true),
      initial_temperature_(initial_temperature),
      cooling_rate_(cooling_rate) {}

SolveResult SA_LNS_Solver::solve(const Instance& instance, const int start_node) {
    const auto t0 = std::chrono::high_resolution_clock::now();

    initializeCandidateMatrix(instance);

    // Initial solution
    Lab3BaseSolver::RouteState x = buildInitialState(instance, start_node);
    if (use_local_search_) {
        // Run initial local search to start from a local optimum
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
    double current_temp = initial_temperature_;

    while (true) {
        const double elapsed_ms = std::chrono::duration<double, std::milli>(
            std::chrono::high_resolution_clock::now() - t0).count();
        if (elapsed_ms >= static_cast<double>(max_time_ms_)) break;

        Lab3BaseSolver::RouteState y = x;
        std::vector<int> removed;
        
        // Use LNS_Solver's destroy and repair
        destroy(y, removed);
        if (removed.empty()) break;
        repair(instance, y, removed);

        if (use_local_search_) {
            localSearchImprove(instance, y);
        }

        int y_dist = computeRouteDistance(instance, y.route);
        int y_profit = computeRouteProfit(instance, y.route);
        int y_obj = y_profit - y_dist;

        ++perturb_count;
        
        // Simulated Annealing Acceptance Criterion
        if (y_obj > x_obj) {
            // Strictly better: accept
            x = std::move(y);
            x_dist = y_dist;
            x_profit = y_profit;
            x_obj = y_obj;

            // Update global best
            if (x_obj > best.final_objective) {
                best.path = x.route;
                best.phase2_distance = x_dist;
                best.phase2_profit = x_profit;
                best.final_objective = x_obj;
            }
        } else {
            // Worse: accept with probability P = exp(delta / T)
            double delta = static_cast<double>(y_obj - x_obj); // delta is negative
            std::uniform_real_distribution<double> dist(0.0, 1.0);
            
            double prob = 0.0;
            if (current_temp > 1e-5) {
                prob = std::exp(delta / current_temp);
            }
            
            if (dist(rng_) < prob) {
                x = std::move(y);
                x_dist = y_dist;
                x_profit = y_profit;
                x_obj = y_obj;
            }
        }
        
        // Cooling schedule
        current_temp *= cooling_rate_;
    }

    best.time_ms = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();
    best.iterations = perturb_count;
    return best;
}
