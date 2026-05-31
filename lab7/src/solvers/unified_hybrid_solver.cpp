#include "unified_hybrid_solver.h"

#include <algorithm>
#include <chrono>
#include <random>
#include <cmath>
#include <stdexcept>

UnifiedHybridSolver::UnifiedHybridSolver(const int seed, const int max_time_ms, const double initial_temperature, const double cooling_rate)
    : HAE_Solver(seed, max_time_ms, HaeOperator::ADAPTIVE, true), // We use ADAPTIVE HAE as base
      initial_temperature_(initial_temperature),
      cooling_rate_(cooling_rate) {}

void UnifiedHybridSolver::tryInsertWithSA(std::vector<PopulationEntry>& population, const PopulationEntry& candidate, double current_temp) {
    if (isDuplicate(population, candidate)) {
        return;
    }

    const auto better = [](const PopulationEntry& lhs, const PopulationEntry& rhs) {
        if (lhs.objective != rhs.objective) {
            return lhs.objective > rhs.objective;
        }
        return lhs.canonical_route < rhs.canonical_route;
    };

    if (static_cast<int>(population.size()) < population_size_) {
        population.push_back(candidate);
        std::sort(population.begin(), population.end(), better);
        return;
    }

    // Candidate is better than the worst in population
    if (better(candidate, population.back())) {
        population.back() = candidate;
        std::sort(population.begin(), population.end(), better);
        return;
    }

    // Simulated Annealing logic: candidate is WORSE than the worst in the population.
    // Calculate delta (negative) between candidate and the worst
    double delta = static_cast<double>(candidate.objective - population.back().objective);
    
    double prob = 0.0;
    if (current_temp > 1e-5) {
        prob = std::exp(delta / current_temp);
    }
    
    std::uniform_real_distribution<double> dist(0.0, 1.0);
    // rng_ is protected from Lab3BaseSolver
    if (dist(rng_) < prob) {
        // Accept the worse solution by replacing the worst in the population
        population.back() = candidate;
        std::sort(population.begin(), population.end(), better);
    }
}

SolveResult UnifiedHybridSolver::solve(const Instance& instance, const int start_node) {
    const auto t0 = std::chrono::high_resolution_clock::now();

    initializeCandidateMatrix(instance);

    std::uniform_int_distribution<int> start_dist(0, std::max(0, instance.getNumVertices() - 1));

    std::vector<PopulationEntry> population;
    population.reserve(population_size_);

    PopulationEntry initial_seed_entry;
    bool has_initial_seed_entry = false;

    // 1. INITIALIZE POPULATION
    int population_attempts = 0;
    while (static_cast<int>(population.size()) < population_size_ && population_attempts < population_size_ * 200) {
        const double elapsed_ms = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();
        if (elapsed_ms >= static_cast<double>(max_time_ms_)) {
            break;
        }

        ++population_attempts;
        const int init_start = (population.empty() ? start_node : start_dist(rng_));
        RouteState state = buildInitialState(instance, init_start);

        if (!has_initial_seed_entry) {
            initial_seed_entry = makePopulationEntry(instance, state);
            has_initial_seed_entry = true;
        }

        localSearchImproveIfEnabled(instance, state, true);
        const PopulationEntry candidate = makePopulationEntry(instance, state);
        tryInsertPopulation(population, candidate);
    }

    if (population.empty()) {
        throw std::runtime_error("UHMA population could not be initialized");
    }

    const PopulationEntry initial_best = population.front();
    PopulationEntry best = initial_best;

    int iterations = 0;
    double current_temp = initial_temperature_;
    
    // Dist for LNS Mutation probability
    std::uniform_real_distribution<double> mutation_prob_dist(0.0, 1.0);

    // 2. MAIN EVOLUTIONARY LOOP (HAE + LNS + SA + LS)
    while (true) {
        const double elapsed_ms = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();
        if (elapsed_ms >= static_cast<double>(max_time_ms_)) {
            break;
        }

        // Parent Selection (HAE)
        std::uniform_int_distribution<int> index_dist(0, static_cast<int>(population.size()) - 1);
        const int parent1_idx = index_dist(rng_);
        int parent2_idx = index_dist(rng_);
        while (parent2_idx == parent1_idx) {
            parent2_idx = index_dist(rng_);
        }

        const Route& parent1 = population[static_cast<size_t>(parent1_idx)].state.route;
        const Route& parent2 = population[static_cast<size_t>(parent2_idx)].state.route;

        // Recombination (HAE Adaptive)
        Route child_route = recombine(parent1, parent2);
        RouteState child_state = makeStateFromRoute(instance, child_route);
        
        // HAE Repair
        repairHae(instance, child_state);
        
        // --- THE UNIFIED HYBRID STEP ---
        // LNS MUTATION (30% chance to deeply destroy and repair the child)
        if (mutation_prob_dist(rng_) < 0.3) {
            std::vector<int> removed;
            // destroy() comes from LNS_Solver base class! (uses destroy_fraction_=0.3)
            destroy(child_state, removed);
            if (!removed.empty()) {
                repair(instance, child_state, removed);
            }
        }
        
        // Local Search (Memetic)
        localSearchImproveIfEnabled(instance, child_state);

        const PopulationEntry child = makePopulationEntry(instance, child_state);
        ++iterations;

        if (child.objective > best.objective ||
            (child.objective == best.objective && child.canonical_route < best.canonical_route)) {
            best = child;
        }

        // SA Acceptance into Population
        tryInsertWithSA(population, child, current_temp);
        
        current_temp *= cooling_rate_;
    }

    SolveResult result;
    result.initial_path = has_initial_seed_entry ? initial_seed_entry.state.route : initial_best.state.route;
    result.path = best.state.route;
    result.phase1_distance = has_initial_seed_entry ? initial_seed_entry.distance : initial_best.distance;
    result.phase1_profit = has_initial_seed_entry ? initial_seed_entry.profit : initial_best.profit;
    result.phase1_objective = has_initial_seed_entry ? initial_seed_entry.objective : initial_best.objective;
    result.phase2_distance = best.distance;
    result.phase2_profit = best.profit;
    result.final_objective = best.objective;
    result.time_ms = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();
    result.iterations = iterations;
    return result;
}
