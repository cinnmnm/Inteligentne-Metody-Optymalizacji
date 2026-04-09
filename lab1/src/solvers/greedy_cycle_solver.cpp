#include "greedy_cycle_solver.h"

#include <chrono>
#include <limits>
#include <stdexcept>
#include <vector>

GreedyCycleSolver::GreedyCycleSolver(bool consider_profit, int /*seed*/)
    : consider_profit_(consider_profit) {}

SolveResult GreedyCycleSolver::solve(const Instance& instance, int start_node) {
    const auto start_time = std::chrono::high_resolution_clock::now();

    const int n = instance.getNumVertices();

    SolveResult result;

    std::vector<int> path;
    path.reserve(n + 1); 

    std::vector<bool> visited(n, false);
    visited[start_node] = true;

    int second_node = -1;
    int min_dist = std::numeric_limits<int>::max();

    for (int v = 0; v < n; ++v) {
        if (!visited[v]) {
            int dist = instance.getDistance(start_node, v);
            if (dist < min_dist) {
                min_dist = dist;
                second_node = v;
            }
        }
    }

    visited[second_node] = true;

    path.push_back(start_node);
    path.push_back(second_node);
    path.push_back(start_node);

    int current_dist = 2 * min_dist;
    int current_profit = instance.getProfit(start_node) + instance.getProfit(second_node);

    for (int step = 2; step < n; ++step) {
        int best_score = std::numeric_limits<int>::lowest();
        int best_node = -1;
        int best_insert_pos = -1;
        int best_delta_d = 0;

        #pragma omp parallel
        {
            int local_best_score = std::numeric_limits<int>::lowest();
            int local_best_node = -1;
            int local_best_insert_pos = -1;
            int local_best_delta_d = 0;

            #pragma omp for nowait
            for (int v = 0; v < n; ++v) {
                if (visited[v]) continue;

                int profit_v = instance.getProfit(v);

                for (size_t i = 0; i + 1 < path.size(); ++i) {
                    int u = path[i];
                    int w = path[i + 1];

                    int delta_d = instance.getDistance(u, v) + instance.getDistance(v, w) - instance.getDistance(u, w);
                    int score = consider_profit_ ? (profit_v - delta_d) : -delta_d;

                    if (score > local_best_score) {
                        local_best_score = score;
                        local_best_node = v;
                        local_best_insert_pos = static_cast<int>(i);
                        local_best_delta_d = delta_d;
                    }
                }
            }

            #pragma omp critical
            {
                if (local_best_score > best_score) {
                    best_score = local_best_score;
                    best_node = local_best_node;
                    best_insert_pos = local_best_insert_pos;
                    best_delta_d = local_best_delta_d;
                }
            }
        }

        visited[best_node] = true;
        path.insert(path.begin() + best_insert_pos + 1, best_node);
        
        current_dist += best_delta_d;
        current_profit += instance.getProfit(best_node);
    }

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