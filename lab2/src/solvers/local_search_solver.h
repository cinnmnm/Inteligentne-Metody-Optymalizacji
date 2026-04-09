#pragma once

#include "instance.h"
#include "local_search_structures.h"
#include "solvers/base_solver.h"

#include <random>
#include <string>
#include <vector>

class LocalSearchSolver final : public BaseSolver {
public:
    LocalSearchSolver(bool is_steepest, bool use_edge_swap, std::string initial_solution_type, int seed, int max_time_ms = 1000);

    SolveResult solve(const Instance& instance, int start_node) override;
    SolveResult solveRandomWalk(const Instance& instance, int start_node);

private:
    bool is_steepest_;
    bool use_edge_swap_;
    std::string initial_solution_type_;
    int max_time_ms_;
    std::mt19937 rng_;

    std::vector<int> buildInitialRoute(const Instance& instance, int start_node);
    std::vector<int> buildRandomInitialRoute(const Instance& instance, int start_node, int target_size);
    std::vector<int> buildGreedyInitialRoute(const Instance& instance, int start_node, int target_size) const;

    static int chooseTargetVertexCount(int n);
    static int computeRouteDistance(const Instance& instance, const std::vector<int>& route);
    static int computeRouteProfit(const Instance& instance, const std::vector<int>& route);

    std::vector<Move> generateNeighborhood(const Instance& instance, const std::vector<int>& route, const std::vector<bool>& is_visited) const;

    double calculateInterSwapDelta(const Instance& instance, const std::vector<int>& route, int route_idx, int unvisited_node) const;
    double calculateIntraNodeSwapDelta(const Instance& instance, const std::vector<int>& route, int idx1, int idx2) const;
    double calculateIntraEdgeSwapDelta(const Instance& instance, const std::vector<int>& route, int idx1, int idx2) const;

    void applyMove(std::vector<int>& route, std::vector<bool>& is_visited, const Move& move) const;

    Move findSteepestMove(const std::vector<Move>& neighborhood) const;
    Move findGreedyMoveLazy(const Instance& instance, const std::vector<int>& route, const std::vector<bool>& is_visited);
    Move sampleRandomMove(const Instance& instance, const std::vector<int>& route, const std::vector<bool>& is_visited);
};
