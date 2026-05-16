#include "instance.h"
#include "io/result_printer.h"
#include "io/solver_cli_utils.h"
#include "solution_checker.h"
#include "solvers/base_solver.h"
#include "solvers/msls_solver.h"
#include "solvers/ils_solver.h"
#include "solvers/lns_solver.h"

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
            "./solver <path_to_instance> <solver_name> <start_node> <seed> [ignored_initial_solution_type] [max_time_ms]");

        const int max_time_ms = (argc >= 6) ? SolverCli::parseIntArg(argv[5], "max_time_ms") : 1000;

        SolveResult result;
        if (args.solver_name == "msls") {
            MSLS_Solver solver(args.seed, /*iterations=*/200);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "ils") {
            ILS_Solver solver(args.seed, max_time_ms, /*perturb_size=*/3);
            result = solver.solve(args.instance, args.start_node);
        } else if (args.solver_name == "lns") {
            LNS_Solver solver(args.seed, max_time_ms, /*destroy_fraction=*/0.3);
            result = solver.solve(args.instance, args.start_node);
        } else {
            std::cerr << "Unknown solver: " << args.solver_name << std::endl;
            return 1;
        }

        SolutionChecker::validate(args.instance, result);
        std::cout << ResultPrinter::toJson(result, args.solver_name, args.start_node, args.seed, "lab4") << std::endl;
        return 0;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
}
