#include "instance.h"
#include "io/result_printer.h"
#include "io/solver_cli_utils.h"
#include "solution_checker.h"
#include "solvers/base_solver.h"
#include "solvers/local_search_solver.h"

#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char* argv[]) {
    try {
        const SolverCli::CommonArgs args = SolverCli::parseAndLoadCommonArgs(
            argc,
            argv,
            5,
            7,
            "./solver <path_to_instance> <solver_name> <start_node> <seed> [initial_solution_type] [max_time_ms]");

        const std::string initial_solution_type = (argc >= 6) ? argv[5] : "heuristic";
        const int max_time_ms = (argc >= 7) ? SolverCli::parseIntArg(argv[6], "max_time_ms") : 1000;

        SolveResult result;
        if (args.solver_name == "steepest_node") {
            LocalSearchSolver solver(true, false, initial_solution_type, args.seed, max_time_ms);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "steepest_edge") {
            LocalSearchSolver solver(true, true, initial_solution_type, args.seed, max_time_ms);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "greedy_node") {
            LocalSearchSolver solver(false, false, initial_solution_type, args.seed, max_time_ms);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "greedy_edge") {
            LocalSearchSolver solver(false, true, initial_solution_type, args.seed, max_time_ms);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "random_walk") {
            LocalSearchSolver solver(false, false, initial_solution_type, args.seed, max_time_ms);
            result = solver.solveRandomWalk(args.instance, args.start_node);
        } else {
            std::cerr << "Unknown solver: " << args.solver_name << std::endl;
            return 1;
        }

        SolutionChecker::validate(args.instance, result);
        std::cout << ResultPrinter::toJson(result, args.solver_name, args.start_node, args.seed, "lab2") << std::endl;

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
}
