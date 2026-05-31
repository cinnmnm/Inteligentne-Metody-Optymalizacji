#include "instance.h"
#include "io/result_printer.h"
#include "io/solver_cli_utils.h"
#include "solution_checker.h"
#include "solvers/base_solver.h"
#include "solvers/island_model_hae_solver.h"

#include "hae_solver.h"
#include "msls_solver.h"
#include "solvers/sa_lns_solver.h"
#include "solvers/unified_hybrid_solver.h"
#include "solvers/parallel_uhma_solver.h"

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
        if (args.solver_name == "msls") {
            // Used for baseline display (same as lab4/lab6)
            MSLS_Solver solver(args.seed, /*iterations=*/200);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "msls_4800") {
            // Used for timing calibration to match Lab 6 2500ms budget
            MSLS_Solver solver(args.seed, /*iterations=*/4800);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "hae_op2_ls") {
            // Baseline: best single-population HAE from Lab 6
            HAE_Solver solver(args.seed, max_time_ms, HaeOperator::OP2, true);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "gulasz") {
            // New Single-Thread Własna Metoda (Unified Hybrid Memetic Algorithm)
            UnifiedHybridSolver solver(args.seed, max_time_ms, 100.0, 0.99);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "sa_lns_fast") {
            SA_LNS_Solver solver(args.seed, max_time_ms, 0.3, 100.0, 0.95);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "sa_lns_slow") {
            SA_LNS_Solver solver(args.seed, max_time_ms, 0.3, 1000.0, 0.99);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "imhae_2") {
            IslandModelHAE_Solver solver(args.seed, max_time_ms, /*num_islands=*/2);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "imhae_4") {
            IslandModelHAE_Solver solver(args.seed, max_time_ms, /*num_islands=*/4);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "imhae_6") {
            IslandModelHAE_Solver solver(args.seed, max_time_ms, /*num_islands=*/6);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "imhae_8") {
            IslandModelHAE_Solver solver(args.seed, max_time_ms, /*num_islands=*/8);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "imhae_12") {
            IslandModelHAE_Solver solver(args.seed, max_time_ms, /*num_islands=*/12);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "p_gulasz_2") {
            ParallelUHMASolver solver(args.seed, max_time_ms, /*num_threads=*/2);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "p_gulasz_4") {
            ParallelUHMASolver solver(args.seed, max_time_ms, /*num_threads=*/4);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "p_gulasz_8") {
            ParallelUHMASolver solver(args.seed, max_time_ms, /*num_threads=*/8);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "p_gulasz_12") {
            ParallelUHMASolver solver(args.seed, max_time_ms, /*num_threads=*/12);
            result = solver.solve(args.instance, args.start_node);
        } else {
            std::cerr << "Unknown solver: " << args.solver_name << std::endl;
            return 1;
        }

        SolutionChecker::validate(args.instance, result);
        std::cout << ResultPrinter::toJson(result, args.solver_name, args.start_node, args.seed, "lab7") << std::endl;
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
}
