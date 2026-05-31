#include "parallel_uhma_solver.h"
#include "unified_hybrid_solver.h"

#include <chrono>
#include <vector>

#include <omp.h>

ParallelUHMASolver::ParallelUHMASolver(const int seed, const int max_time_ms, const int num_threads)
    : seed_(seed), max_time_ms_(max_time_ms), num_threads_(num_threads) {}

SolveResult ParallelUHMASolver::solve(const Instance& instance, const int start_node) {
    const auto t0 = std::chrono::high_resolution_clock::now();

    std::vector<SolveResult> results(static_cast<size_t>(num_threads_));

    #pragma omp parallel for num_threads(num_threads_) schedule(static, 1)
    for (int i = 0; i < num_threads_; i++) {
        const int thread_seed = seed_ + i * 7919;
        UnifiedHybridSolver solver(thread_seed, max_time_ms_, 100.0, 0.99);
        results[static_cast<size_t>(i)] = solver.solve(instance, start_node);
    }

    int best_idx = 0;
    int total_iterations = 0;
    for (int i = 0; i < num_threads_; i++) {
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
