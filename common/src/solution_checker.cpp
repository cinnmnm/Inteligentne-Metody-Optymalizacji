#include "solution_checker.h"

#include <sstream>
#include <unordered_set>

void SolutionChecker::validate(const Instance& instance, SolveResult& result) {
    result.is_valid = false;
    result.error_message.clear();

    const std::vector<int>& path = result.path;
    if (path.size() < 4) {
        result.error_message = "Path must contain at least 3 unique vertices and return to start";
        return;
    }

    if (path.front() != path.back()) {
        result.error_message = "Cycle is not closed";
        return;
    }

    std::unordered_set<int> visited;
    visited.reserve(path.size());
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        const int v = path[i];
        if (v < 0 || v >= instance.getNumVertices()) {
            std::ostringstream oss;
            oss << "Vertex out of range: " << v;
            result.error_message = oss.str();
            return;
        }
        if (!visited.insert(v).second) {
            std::ostringstream oss;
            oss << "Duplicated vertex " << v;
            result.error_message = oss.str();
            return;
        }
    }

    int actual_distance = 0;
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        actual_distance += instance.getDistance(path[i], path[i + 1]);
    }

    int actual_profit = 0;
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        actual_profit += instance.getProfit(path[i]);
    }

    int expected_distance = (result.phase2_distance != 0 || result.phase2_profit != 0) ? result.phase2_distance : result.phase1_distance;
    int expected_profit = (result.phase2_distance != 0 || result.phase2_profit != 0) ? result.phase2_profit : result.phase1_profit;

    if (actual_distance != expected_distance) {
        std::ostringstream oss;
        oss << "Reported distance " << expected_distance << " != actual " << actual_distance;
        result.error_message = oss.str();
        return;
    }

    if (actual_profit != expected_profit) {
        std::ostringstream oss;
        oss << "Reported profit " << expected_profit << " != actual " << actual_profit;
        result.error_message = oss.str();
        return;
    }

    result.is_valid = true;
}
