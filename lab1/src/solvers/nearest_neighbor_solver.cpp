#include "nearest_neighbor_solver.h"

#include <chrono>
#include <limits>
#include <stdexcept>

NearestNeighborSolver::NearestNeighborSolver(bool consider_profit, int /*seed*/) 
    : consider_profit_(consider_profit) {}

SolveResult NearestNeighborSolver::solve(const Instance& instance, int start_node) {
    const auto start_time = std::chrono::high_resolution_clock::now();

    const int n = instance.getNumVertices();

    SolveResult result;
    
    std::vector<int> path;
    path.reserve(n + 1);
    path.push_back(start_node);

    std::vector<bool> visited(n, false);
    visited[start_node] = true;

    int current_dist = 0;
    int current_profit = instance.getProfit(start_node);
    int curr_node = start_node;

    for (int step = 1; step < n; ++step) {
        int best_score = std::numeric_limits<int>::lowest(); 
        int best_node = -1;
        int best_dist = 0;

        for (int v = 0; v < n; ++v) {
            if (visited[v]) continue;

            int dist = instance.getDistance(curr_node, v);
            
            int score = consider_profit_ ? (instance.getProfit(v) - dist) : -dist;

            if (score > best_score) {
                best_score = score;
                best_node = v;
                best_dist = dist;
            }
        }

        visited[best_node] = true;
        path.push_back(best_node);
        current_dist += best_dist;
        current_profit += instance.getProfit(best_node);
        curr_node = best_node;
    }

    current_dist += instance.getDistance(curr_node, start_node);
    path.push_back(start_node);

    result.phase1_distance = current_dist;
    result.phase1_profit = current_profit;
    result.phase1_objective = current_profit - current_dist;

    runPhaseTwo(path, current_dist, current_profit, instance);

    result.path = std::move(path);
    result.phase2_distance = current_dist;
    result.phase2_profit = current_profit;
    result.final_objective = current_profit - current_dist;

    const auto end_time = std::chrono::high_resolution_clock::now();
    result.time_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();

    return result;
}