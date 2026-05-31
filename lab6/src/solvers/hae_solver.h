#pragma once

#include "../../../lab4/src/solvers/lns_solver.h"

#include <unordered_set>
#include <utility>

enum class HaeOperator {
    OP1,
    OP2,
    OP3,
    ADAPTIVE
};

class HAE_Solver : public LNS_Solver {
public:
    struct PairHash {
        std::size_t operator()(const std::pair<int, int>& edge) const noexcept;
    };

    HAE_Solver(int seed, int max_time_ms, HaeOperator op_type, bool use_local_search = true);

    SolveResult solve(const Instance& instance, int start_node) override;

protected:
    struct PopulationEntry {
        RouteState state;
        std::vector<int> canonical_route;
        int distance = 0;
        int profit = 0;
        int objective = 0;
    };

    using Route = std::vector<int>;

    static constexpr int population_size_ = 20;

    HaeOperator op_type_;

    std::vector<int> canonizeRoute(const std::vector<int>& route) const;
    std::unordered_set<int> computeVertices(const std::vector<int>& route) const;
    std::unordered_set<std::pair<int, int>, PairHash> computeEdges(const std::vector<int>& route) const;
    RouteState makeStateFromRoute(const Instance& instance, const Route& route) const;
    PopulationEntry makePopulationEntry(const Instance& instance, const RouteState& state) const;
    bool isDuplicate(const std::vector<PopulationEntry>& population, const PopulationEntry& candidate) const;
    bool tryInsertPopulation(std::vector<PopulationEntry>& population, const PopulationEntry& candidate) const;
    int selectOperator();

    Route recombinationOp1(const Route& p1, const Route& p2);
    Route recombinationOp2(const Route& p1, const Route& p2);
    Route recombinationOp3(const Route& p1, const Route& p2);
    Route recombine(const Route& p1, const Route& p2);

    void repairHae(const Instance& instance, RouteState& state);
    void localSearchImproveIfEnabled(const Instance& instance, RouteState& state, bool force = false);
};
