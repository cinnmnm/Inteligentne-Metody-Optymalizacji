#include "instance.h"
#include "io/result_printer.h"
#include "io/solver_cli_utils.h"
#include "solution_checker.h"
#include "solvers/base_solver.h"
#include "solvers/greedy_heuristic_solver.h"
#include "solvers/hae_solver.h"

#include "hybrid_local_search_solver.h"
#include "ils_solver.h"
#include "lns_solver.h"
#include "msls_solver.h"

#include <iostream>
#include <stdexcept>
#include <string>

int main(int argc, char* argv[]) {
    try {
        const SolverCli::CommonArgs args = SolverCli::parseAndLoadCommonArgs(
            argc,
            argv,
            5,
            6,
            "./solver <path_to_instance> <solver_name> <start_node> <seed> [max_time_ms]");

        const int max_time_ms = (argc >= 6) ? SolverCli::parseIntArg(argv[5], "max_time_ms") : 1000;

        SolveResult result;
        if (args.solver_name == "greedy_baseline") {
            GreedyHeuristicSolver solver(args.seed);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "local_search_baseline") {
            HybridLocalSearchSolver solver(args.seed, max_time_ms);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "msls") {
            MSLS_Solver solver(args.seed, /*iterations=*/200);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "ils") {
            ILS_Solver solver(args.seed, max_time_ms, /*perturb_size=*/3);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "lns") {
            LNS_Solver solver(args.seed, max_time_ms, /*destroy_fraction=*/0.3, /*use_local_search=*/true);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "lns_no_ls") {
            LNS_Solver solver(args.seed, max_time_ms, /*destroy_fraction=*/0.3, /*use_local_search=*/false);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "hae_op1_ls") {
            HAE_Solver solver(args.seed, max_time_ms, HaeOperator::OP1, true);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "hae_op2_ls") {
            HAE_Solver solver(args.seed, max_time_ms, HaeOperator::OP2, true);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "hae_op3_ls") {
            HAE_Solver solver(args.seed, max_time_ms, HaeOperator::OP3, true);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "hae_random_ls") {
            HAE_Solver solver(args.seed, max_time_ms, HaeOperator::ADAPTIVE, true);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "hae_op2_no_ls") {
            HAE_Solver solver(args.seed, max_time_ms, HaeOperator::OP2, false);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "hae_op3_no_ls") {
            HAE_Solver solver(args.seed, max_time_ms, HaeOperator::OP3, false);
            result = solver.solve(args.instance, args.start_node);
        } else {
            std::cerr << "Unknown solver: " << args.solver_name << std::endl;
            return 1;
        }

        SolutionChecker::validate(args.instance, result);
        std::cout << ResultPrinter::toJson(result, args.solver_name, args.start_node, args.seed, "lab6") << std::endl;
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
}