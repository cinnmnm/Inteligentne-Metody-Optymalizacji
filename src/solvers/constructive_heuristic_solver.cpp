#include "solvers/constructive_heuristic_solver.h"

void ConstructiveHeuristicSolver::runPhaseTwo(std::vector<int>& path, int& current_dist, int& current_profit, const Instance& instance) {
    bool improvement = true;
    
    while (improvement && path.size() > 4) { 
        improvement = false;
        int best_delta = 0;
        size_t best_index = 0;

        for (size_t i = 0; i + 1 < path.size(); ++i) {
            int u = (i == 0) ? path[path.size() - 2] : path[i - 1];
            int v = path[i];
            int w = path[i + 1];

            int dist_uv = instance.getDistance(u, v);
            int dist_vw = instance.getDistance(v, w);
            int dist_uw = instance.getDistance(u, w);
            int profit_v = instance.getProfit(v);

            int delta_f = dist_uv + dist_vw - dist_uw - profit_v;

            if (delta_f > best_delta) {
                best_delta = delta_f;
                best_index = i;
            }
        }

        if (best_delta > 0) {
            improvement = true;
            
            int u = (best_index == 0) ? path[path.size() - 2] : path[best_index - 1];
            int v = path[best_index];
            int w = path[best_index + 1];

            current_dist -= (instance.getDistance(u, v) + instance.getDistance(v, w) - instance.getDistance(u, w));
            current_profit -= instance.getProfit(v);

            if (best_index == 0) {
                path.erase(path.begin());
                path.back() = path.front();
            } else {
                path.erase(path.begin() + best_index);
            }
        }
    }
}