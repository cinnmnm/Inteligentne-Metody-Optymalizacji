#include "island_model_hae_solver.h"

#include "hae_solver.h"
#include "ils_solver.h"
#include "lns_solver.h"
#include "sa_lns_solver.h"

#include <algorithm>
#include <chrono>
#include <vector>

#include <omp.h>

// ──────────────────────────────────────────────────────────────
// Island strategies – defines which solver variant each island runs.
// The pool is ordered so that the best-performing strategies come
// first, which means smaller island counts still get good coverage.
// ──────────────────────────────────────────────────────────────
namespace {

enum class IslandStrategy {
    HAE_OP2,       // Best single-strategy performer from Lab 6
    SA_LNS_FAST,   // New SA-LNS hybrid with fast cooling
    HAE_OP1,
    SA_LNS_SLOW,   // New SA-LNS hybrid with slow cooling
    HAE_OP3,
    HAE_ADAPTIVE,  // Randomly picks among Op1/Op2/Op3 each iteration
    LNS_MEDIUM,    // destroy_fraction = 0.3
    ILS,
    LNS_SMALL,     // destroy_fraction = 0.2
    LNS_LARGE,     // destroy_fraction = 0.4
};

constexpr IslandStrategy STRATEGY_POOL[] = {
    IslandStrategy::HAE_OP2,
    IslandStrategy::SA_LNS_FAST,
    IslandStrategy::HAE_OP1,
    IslandStrategy::SA_LNS_SLOW,
    IslandStrategy::HAE_OP3,
    IslandStrategy::HAE_ADAPTIVE,
    IslandStrategy::LNS_MEDIUM,
    IslandStrategy::ILS,
    IslandStrategy::LNS_SMALL,
    IslandStrategy::LNS_LARGE,
};

constexpr int STRATEGY_POOL_SIZE = static_cast<int>(
    sizeof(STRATEGY_POOL) / sizeof(STRATEGY_POOL[0]));

/// Run a single island with the given strategy and return its result.
SolveResult runIsland(IslandStrategy strategy, int seed,
                      int max_time_ms, bool use_ls,
                      const Instance& instance, int start_node) {
    switch (strategy) {
    case IslandStrategy::SA_LNS_FAST: {
        // Initial Temp 100, Cooling 0.95
        SA_LNS_Solver solver(seed, max_time_ms, 0.3, 100.0, 0.95);
        return solver.solve(instance, start_node);
    }
    case IslandStrategy::SA_LNS_SLOW: {
        // Initial Temp 1000, Cooling 0.99
        SA_LNS_Solver solver(seed, max_time_ms, 0.3, 1000.0, 0.99);
        return solver.solve(instance, start_node);
    }
    case IslandStrategy::HAE_OP1: {
        HAE_Solver solver(seed, max_time_ms, HaeOperator::OP1, use_ls);
        return solver.solve(instance, start_node);
    }
    case IslandStrategy::HAE_OP2: {
        HAE_Solver solver(seed, max_time_ms, HaeOperator::OP2, use_ls);
        return solver.solve(instance, start_node);
    }
    case IslandStrategy::HAE_OP3: {
        HAE_Solver solver(seed, max_time_ms, HaeOperator::OP3, use_ls);
        return solver.solve(instance, start_node);
    }
    case IslandStrategy::HAE_ADAPTIVE: {
        HAE_Solver solver(seed, max_time_ms, HaeOperator::ADAPTIVE, use_ls);
        return solver.solve(instance, start_node);
    }
    case IslandStrategy::LNS_MEDIUM: {
        LNS_Solver solver(seed, max_time_ms, /*destroy_fraction=*/0.3, use_ls);
        return solver.solve(instance, start_node);
    }
    case IslandStrategy::LNS_SMALL: {
        LNS_Solver solver(seed, max_time_ms, /*destroy_fraction=*/0.2, use_ls);
        return solver.solve(instance, start_node);
    }
    case IslandStrategy::LNS_LARGE: {
        LNS_Solver solver(seed, max_time_ms, /*destroy_fraction=*/0.4, use_ls);
        return solver.solve(instance, start_node);
    }
    case IslandStrategy::ILS: {
        ILS_Solver solver(seed, max_time_ms, /*perturb_size=*/3);
        return solver.solve(instance, start_node);
    }
    }
    // Fallback – should never reach here.
    HAE_Solver solver(seed, max_time_ms, HaeOperator::OP2, use_ls);
    return solver.solve(instance, start_node);
}

}  // namespace

// ──────────────────────────────────────────────────────────────

IslandModelHAE_Solver::IslandModelHAE_Solver(
    const int seed, const int max_time_ms,
    const int num_islands, const bool use_local_search)
    : seed_(seed),
      max_time_ms_(max_time_ms),
      num_islands_(num_islands),
      use_local_search_(use_local_search) {}

SolveResult IslandModelHAE_Solver::solve(const Instance& instance,
                                          const int start_node) {
    const auto t0 = std::chrono::high_resolution_clock::now();

    std::vector<SolveResult> results(static_cast<size_t>(num_islands_));

    // Each island runs independently in its own OpenMP thread.
    // Islands that map to the same strategy (when num_islands > pool size)
    // still differ by RNG seed, so their search trajectories diverge.
    #pragma omp parallel for num_threads(num_islands_) schedule(static, 1)
    for (int i = 0; i < num_islands_; i++) {
        const IslandStrategy strategy = STRATEGY_POOL[i % STRATEGY_POOL_SIZE];
        // Use a large prime offset so seeds are well-separated.
        const int island_seed = seed_ + i * 7919;

        results[static_cast<size_t>(i)] = runIsland(
            strategy, island_seed, max_time_ms_,
            use_local_search_, instance, start_node);
    }

    // ── Collect global best ──────────────────────────────────
    int best_idx = 0;
    int total_iterations = 0;
    for (int i = 0; i < num_islands_; i++) {
        total_iterations += results[static_cast<size_t>(i)].iterations;
        if (results[static_cast<size_t>(i)].final_objective >
            results[static_cast<size_t>(best_idx)].final_objective) {
            best_idx = i;
        }
    }

    SolveResult best = std::move(results[static_cast<size_t>(best_idx)]);
    best.time_ms = std::chrono::duration<double, std::milli>(
        std::chrono::high_resolution_clock::now() - t0).count();
    best.iterations = total_iterations;

    return best;
}
