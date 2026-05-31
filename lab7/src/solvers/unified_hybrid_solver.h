#pragma once

#include "../../../lab6/src/solvers/hae_solver.h"

class UnifiedHybridSolver final : public HAE_Solver {
public:
    UnifiedHybridSolver(int seed, int max_time_ms, double initial_temperature, double cooling_rate);

    SolveResult solve(const Instance& instance, int start_node) override;

private:
    double initial_temperature_;
    double cooling_rate_;
    
    // Tries to insert into population. If candidate is worse than the worst,
    // it uses Simulated Annealing acceptance to possibly replace the worst.
    void tryInsertWithSA(std::vector<PopulationEntry>& population, const PopulationEntry& candidate, double current_temp);
};
