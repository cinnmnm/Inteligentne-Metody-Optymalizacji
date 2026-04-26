#pragma once

#include "instance.h"
#include "memory_move.h"
#include "solvers/base_solver.h"

#include <random>
#include <vector>

class Lab3BaseSolver : public BaseSolver {
public:
    explicit Lab3BaseSolver(int seed, int max_time_ms = 1000, int candidate_k = 10);

protected:
    struct RouteState {
        std::vector<int> route;
        std::vector<bool> is_visited;
        std::vector<int> position_by_id;
        std::vector<int> next_by_id;
    };

    struct EvalConfig {
        bool use_candidate_filter = false;
    };

    std::mt19937 rng_;
    int max_time_ms_;

    RouteState buildInitialState(const Instance& instance, int start_node);
    void rebuildRouteCaches(RouteState& state) const;

    static int chooseTargetVertexCount(int n);
    static int computeRouteDistance(const Instance& instance, const std::vector<int>& route);
    static int computeRouteProfit(const Instance& instance, const std::vector<int>& route);

    void initializeCandidateMatrix(const Instance& instance);
    bool isCandidateNeighbor(int from, int to) const;

    std::vector<MemoryMove> collectImprovingMoves(
        const Instance& instance,
        const RouteState& state,
        const EvalConfig& config) const;

    bool isMoveApplicable(const MemoryMove& move, const RouteState& state) const;
    double evaluateMoveDeltaCurrent(const Instance& instance, const RouteState& state, const MemoryMove& move) const;
    bool applyMove(RouteState& state, const MemoryMove& move) const;

private:
    int candidate_k_;
    std::vector<std::vector<unsigned char>> candidate_matrix_;

    static int nextIndex(int idx, int size);
    static int prevIndex(int idx, int size);

    std::vector<int> buildRandomInitialRoute(const Instance& instance, int start_node, int target_size);

    static double calculateAddNodeDelta(const Instance& instance, const std::vector<int>& route, int edge_idx, int new_node);
    static double calculateRemoveNodeDelta(const Instance& instance, const std::vector<int>& route, int route_idx);
    static double calculateIntraEdgeSwapDelta(const Instance& instance, const std::vector<int>& route, int idx1, int idx2);
};
