#include "regret_solver.h"

#include <chrono>
#include <limits>
#include <stdexcept>
#include <vector>

RegretSolver::RegretSolver(bool consider_profit, double weight_regret, double weight_greedy, int /*seed*/)
    : consider_profit_(consider_profit), weight_regret_(weight_regret), weight_greedy_(weight_greedy) {}

SolveResult RegretSolver::solve(const Instance& instance, int start_node) {
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
        double best_overall_score = std::numeric_limits<double>::lowest();
        int best_node = -1;
        int best_insert_pos = -1;
        int best_delta_d = 0;

        for (int v = 0; v < n; ++v) {
            if (visited[v]) continue;

            int profit_v = instance.getProfit(v);
            int best_value = std::numeric_limits<int>::lowest();
            int second_best_value = std::numeric_limits<int>::lowest();
            int pos1 = -1;
            int delta_d1 = 0;

            for (size_t i = 0; i + 1 < path.size(); ++i) {
                int u = path[i];
                int w = path[i + 1];
                int delta_d = instance.getDistance(u, v) + instance.getDistance(v, w) - instance.getDistance(u, w);
                int f = consider_profit_ ? (profit_v - delta_d) : -delta_d;

                if (f > best_value) {
                    second_best_value = best_value; 
                    best_value = f;
                    pos1 = static_cast<int>(i);
                    delta_d1 = delta_d;
                } else if (f > second_best_value) {
                    second_best_value = f;
                }
            }

            int regret = best_value - second_best_value;
            double score = weight_regret_ * regret + weight_greedy_ * best_value;

            if (score > best_overall_score) {
                best_overall_score = score;
                best_node = v;
                best_insert_pos = pos1;
                best_delta_d = delta_d1;
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