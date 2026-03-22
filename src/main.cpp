#include "instance.h"
#include "io/instance_reader.h"
#include "io/result_printer.h"
#include "solution_checker.h"
#include "solvers/base_solver.h"
#include "solvers/random_solver.h"
#include "solvers/nearest_neighbor_solver.h"
#include "solvers/greedy_cycle_solver.h"
#include "solvers/regret_solver.h"

#include <cstdlib>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

namespace {

int parseIntArg(const char* value, const std::string& name) {
    try {
        size_t pos = 0;
        const std::string as_string(value);
        const int parsed = std::stoi(as_string, &pos);
        if (pos != as_string.size()) {
            throw std::invalid_argument("trailing characters");
        }
        return parsed;
    } catch (const std::exception&) {
        throw std::runtime_error("Invalid integer for " + name + ": " + value);
    }
}

}  // namespace

int main(int argc, char* argv[]) {
    try {
        if (argc != 5) {
            std::cerr << "Usage: ./solver <path_to_instance> <solver_name> <start_node> <seed>" << std::endl;
            return 1;
        }

        const std::string instance_path = argv[1];
        const std::string solver_name = argv[2];
        const int start_node = parseIntArg(argv[3], "start_node");
        const int seed = parseIntArg(argv[4], "seed");

        const Instance instance = InstanceReader::loadFromCsv(instance_path);

        if (start_node < 0 || start_node >= instance.getNumVertices()) {
            std::cerr << "start_node out of range: " << start_node << ", expected [0, " << (instance.getNumVertices() - 1) << "]" << std::endl;
            return 1;
        }

        std::unique_ptr<BaseSolver> solver;
        if (solver_name == "random") {
        solver = std::make_unique<RandomSolver>(seed);
        } 
        else if (solver_name == "nearest_neighbor") {
            solver = std::make_unique<NearestNeighborSolver>(false, seed);
        } 
        else if (solver_name == "nearest_neighbor_profit") {
            solver = std::make_unique<NearestNeighborSolver>(true, seed);
        } 
        else if (solver_name == "greedy_cycle") {
            solver = std::make_unique<GreedyCycleSolver>(false, seed);
        } 
        else if (solver_name == "greedy_cycle_profit") {
            solver = std::make_unique<GreedyCycleSolver>(true, seed);
        } 
        else if (solver_name == "regret") {
            solver = std::make_unique<RegretSolver>(true, 1.0, 0.0, seed);
        } 
        else if (solver_name == "weighted_regret") {
            solver = std::make_unique<RegretSolver>(true, 1.0, 1.0, seed);
        } 
        else {
            std::cerr << "Unknown solver: " << solver_name << std::endl;
            return 1; 
        }

        SolveResult result = solver->solve(instance, start_node);
        SolutionChecker::validate(instance, result);

        std::cout << ResultPrinter::toJson(result, solver_name, start_node, seed) << std::endl;

        return 0;
    } catch (const std::exception& ex) {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
}
