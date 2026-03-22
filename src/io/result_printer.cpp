#include "io/result_printer.h"

#include <iomanip>
#include <ios>
#include <sstream>
#include <vector>

namespace {

std::string jsonEscape(const std::string& input) {
    std::ostringstream oss;
    for (const char c : input) {
        switch (c) {
            case '"':
                oss << "\\\"";
                break;
            case '\\':
                oss << "\\\\";
                break;
            case '\b':
                oss << "\\b";
                break;
            case '\f':
                oss << "\\f";
                break;
            case '\n':
                oss << "\\n";
                break;
            case '\r':
                oss << "\\r";
                break;
            case '\t':
                oss << "\\t";
                break;
            default:
                oss << c;
                break;
        }
    }
    return oss.str();
}

std::string pathToJson(const std::vector<int>& path) {
    std::ostringstream oss;
    oss << "[";
    for (size_t i = 0; i < path.size(); ++i) {
        if (i > 0) {
            oss << ",";
        }
        oss << path[i];
    }
    oss << "]";
    return oss.str();
}

}  // namespace

namespace ResultPrinter {

std::string toJson(const SolveResult& result, const std::string& solver_name, int start_node, int seed) {
    std::ostringstream out;

    out << "{";
    out << "\"solver\":\"" << jsonEscape(solver_name) << "\",";
    out << "\"start_node\":" << start_node << ",";
    out << "\"seed\":" << seed << ",";
    out << "\"phase1_distance\":" << result.phase1_distance << ",";
    out << "\"phase1_profit\":" << result.phase1_profit << ",";
    out << "\"phase1_objective\":" << result.phase1_objective << ",";
    out << "\"phase2_distance\":" << result.phase2_distance << ",";
    out << "\"phase2_profit\":" << result.phase2_profit << ",";
    out << "\"final_objective\":" << result.final_objective << ",";
    out << std::fixed << std::setprecision(6);
    out << "\"time_ms\":" << result.time_ms << ",";
    out.unsetf(std::ios::floatfield);
    out << "\"is_valid\":" << (result.is_valid ? "true" : "false") << ",";
    out << "\"error_message\":\"" << jsonEscape(result.error_message) << "\",";
    out << "\"path\":" << pathToJson(result.path);
    out << "}";

    return out.str();
}

}  // namespace ResultPrinter
