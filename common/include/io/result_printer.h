#pragma once

#include "solve_result.h"

#include <string>

namespace ResultPrinter {

std::string toJson(const SolveResult& result, const std::string& solver_name, int start_node, int seed);

}  // namespace ResultPrinter
