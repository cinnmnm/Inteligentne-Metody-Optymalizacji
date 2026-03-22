#include "solvers/random_solver.h"

#include <algorithm>
#include <chrono>
#include <numeric>
#include <stdexcept>
#include <vector>

RandomSolver::RandomSolver(int seed) : rng_(static_cast<std::mt19937::result_type>(seed)) {}

SolveResult RandomSolver::solve(const Instance& instance, int start_node) {
    const auto start_time = std::chrono::high_resolution_clock::now();

    const int n = instance.getNumVertices();

    std::vector<int> candidates;
    candidates.reserve(static_cast<size_t>(n - 1));
    for (int v = 0; v < n; ++v) {
        if (v != start_node) {
            candidates.push_back(v);
        }
    }

    std::uniform_int_distribution<int> count_dist(2, static_cast<int>(candidates.size()));
    const int extra_count = count_dist(rng_);

    std::shuffle(candidates.begin(), candidates.end(), rng_);
    candidates.resize(static_cast<size_t>(extra_count));

    SolveResult result;
    result.path.reserve(static_cast<size_t>(extra_count + 2));
    result.path.push_back(start_node);
    result.path.insert(result.path.end(), candidates.begin(), candidates.end());
    result.path.push_back(start_node);

    for (size_t i = 0; i + 1 < result.path.size(); ++i) {
        const int u = result.path[i];
        const int v = result.path[i + 1];
        result.phase1_distance += instance.getDistance(u, v);
    }

    for (size_t i = 0; i + 1 < result.path.size(); ++i) {
        result.phase1_profit += instance.getProfit(result.path[i]);
    }

    result.phase1_objective = result.phase1_profit - result.phase1_distance;
    result.final_objective = result.phase1_objective;

    const auto end_time = std::chrono::high_resolution_clock::now();
    result.time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    return result;
}
