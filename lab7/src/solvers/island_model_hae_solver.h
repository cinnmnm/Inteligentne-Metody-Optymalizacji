#pragma once

#include "solvers/base_solver.h"
#include "solve_result.h"

/// Island Model Hybrid Evolutionary Algorithm.
///
/// Runs multiple independent solver "islands" in parallel using OpenMP.
/// Each island employs a different search strategy (HAE with various operators,
/// LNS with different destroy fractions, ILS). After all islands finish, the
/// globally best solution is returned.
///
/// The diversity comes from:
///   1. Different RNG seeds per island
///   2. Different search strategies per island (HAE operators, LNS, ILS)
///   3. Independent population evolution across islands
///
/// All islands share the same wall-clock time budget (max_time_ms).
class IslandModelHAE_Solver : public BaseSolver {
public:
    /// @param seed          Base random seed (each island offsets from this)
    /// @param max_time_ms   Wall-clock time budget per island
    /// @param num_islands   Number of parallel islands (threads)
    /// @param use_local_search  Whether HAE/LNS islands apply local search
    IslandModelHAE_Solver(int seed, int max_time_ms, int num_islands = 4,
                          bool use_local_search = true);

    SolveResult solve(const Instance& instance, int start_node) override;

private:
    int seed_;
    int max_time_ms_;
    int num_islands_;
    bool use_local_search_;
};
