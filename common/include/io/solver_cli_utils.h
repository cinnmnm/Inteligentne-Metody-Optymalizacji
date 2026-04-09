#pragma once

#include "instance.h"
#include "io/instance_reader.h"

#include <stdexcept>
#include <string>

namespace SolverCli {

struct CommonArgs {
    std::string instance_path;
    std::string solver_name;
    int start_node = 0;
    int seed = 0;
    Instance instance;
};

inline int parseIntArg(const char* value, const std::string& name) {
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

inline void validateStartNode(const Instance& instance, const int start_node) {
    if (start_node < 0 || start_node >= instance.getNumVertices()) {
        throw std::runtime_error(
            "start_node out of range: " + std::to_string(start_node) +
            ", expected [0, " + std::to_string(instance.getNumVertices() - 1) + "]");
    }
}

inline CommonArgs parseAndLoadCommonArgs(
    const int argc,
    char* argv[],
    const int min_argc,
    const int max_argc,
    const std::string& usage) {
    if (argc < min_argc || argc > max_argc) {
        throw std::runtime_error("Usage: " + usage);
    }

    CommonArgs args;
    args.instance_path = argv[1];
    args.solver_name = argv[2];
    args.start_node = parseIntArg(argv[3], "start_node");
    args.seed = parseIntArg(argv[4], "seed");
    args.instance = InstanceReader::loadFromCsv(args.instance_path);

    validateStartNode(args.instance, args.start_node);
    return args;
}

}  // namespace SolverCli
