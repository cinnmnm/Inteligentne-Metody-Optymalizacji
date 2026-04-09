#include "instance.h"
#include "io/result_printer.h"
#include "io/solver_cli_utils.h"
#include "solution_checker.h"
#include "solvers/base_solver.h"
#include "random_solver.h"
#include "nearest_neighbor_solver.h"
#include "greedy_cycle_solver.h"
#include "regret_solver.h"

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

int main(int argc, char* argv[]) {
    try {
        const SolverCli::CommonArgs args = SolverCli::parseAndLoadCommonArgs(
            argc,
            argv,
            5,
            5,
            "./solver <path_to_instance> <solver_name> <start_node> <seed>");

        std::unique_ptr<BaseSolver> solver;
        if (args.solver_name == "random") {
        solver = std::make_unique<RandomSolver>(args.seed);
        } 
        else if (args.solver_name == "nearest_neighbor") {
            solver = std::make_unique<NearestNeighborSolver>(false, args.seed);
        } 
        else if (args.solver_name == "nearest_neighbor_profit") {
            solver = std::make_unique<NearestNeighborSolver>(true, args.seed);
        } 
        else if (args.solver_name == "greedy_cycle") {
            solver = std::make_unique<GreedyCycleSolver>(false, args.seed);
        } 
        else if (args.solver_name == "greedy_cycle_profit") {
            solver = std::make_unique<GreedyCycleSolver>(true, args.seed);
        } 
        else if (args.solver_name == "regret") {
            solver = std::make_unique<RegretSolver>(true, 1.0, 0.0, args.seed);
        } 
        else if (args.solver_name == "weighted_regret") {
            solver = std::make_unique<RegretSolver>(true, 1.0, 1.0, args.seed);
        } 
        else {
            std::cerr << "Unknown solver: " << args.solver_name << std::endl;
            return 1; 
        }

        SolveResult result = solver->solve(args.instance, args.start_node);
        SolutionChecker::validate(args.instance, result);

        std::cout << ResultPrinter::toJson(result, args.solver_name, args.start_node, args.seed) << std::endl;

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
}
