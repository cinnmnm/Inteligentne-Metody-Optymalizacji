#include "io/result_printer.h"

#include <iomanip>
#include <ios>
#include <sstream>
#include <string>
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

std::string formatDouble(const double value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(6) << value;
    return oss.str();
}

void appendJsonFieldPrefix(std::ostringstream& out, bool& is_first) {
    if (!is_first) {
        out << ",";
    }
    is_first = false;
}

void appendStringField(std::ostringstream& out, bool& is_first, const std::string& key, const std::string& value) {
    appendJsonFieldPrefix(out, is_first);
    out << "\"" << key << "\":\"" << jsonEscape(value) << "\"";
}

void appendIntField(std::ostringstream& out, bool& is_first, const std::string& key, const int value) {
    appendJsonFieldPrefix(out, is_first);
    out << "\"" << key << "\":" << value;
}

void appendDoubleField(std::ostringstream& out, bool& is_first, const std::string& key, const double value) {
    appendJsonFieldPrefix(out, is_first);
    out << "\"" << key << "\":" << formatDouble(value);
}

void appendBoolField(std::ostringstream& out, bool& is_first, const std::string& key, const bool value) {
    appendJsonFieldPrefix(out, is_first);
    out << "\"" << key << "\":" << (value ? "true" : "false");
}

void appendRawField(std::ostringstream& out, bool& is_first, const std::string& key, const std::string& raw_json_value) {
    appendJsonFieldPrefix(out, is_first);
    out << "\"" << key << "\":" << raw_json_value;
}

std::string initialFinalPairToJson(const int initial_value, const int final_value) {
    std::ostringstream out;
    out << "{"
        << "\"initial\":" << initial_value << ","
        << "\"final\":" << final_value
        << "}";
    return out.str();
}

std::string metricsToJson(const SolveResult& result, const std::string& lab_name) {
    std::ostringstream out;
    bool is_first = true;
    out << "{";

    // Keep metrics explicit per-lab while retaining common objective and distance pair semantics.
    appendRawField(out, is_first, "distance", initialFinalPairToJson(result.phase1_distance, result.phase2_distance));
    appendRawField(out, is_first, "objective", initialFinalPairToJson(result.phase1_objective, result.final_objective));

    if (lab_name == "lab1") {
        appendRawField(out, is_first, "profit", initialFinalPairToJson(result.phase1_profit, result.phase2_profit));
    }

    out << "}";
    return out.str();
}

}  // namespace

namespace ResultPrinter {

std::string toJson(
    const SolveResult& result,
    const std::string& solver_name,
    const int start_node,
    const int seed,
    const std::string& lab_name) {
    std::ostringstream out;
    bool is_first = true;

    out << "{";

    // Common metadata.
    appendStringField(out, is_first, "lab", lab_name);
    appendStringField(out, is_first, "solver", solver_name);
    appendIntField(out, is_first, "start_node", start_node);
    appendIntField(out, is_first, "seed", seed);
    appendDoubleField(out, is_first, "time_ms", result.time_ms);
    appendBoolField(out, is_first, "is_valid", result.is_valid);
    appendStringField(out, is_first, "error_message", result.error_message);
    appendRawField(out, is_first, "initial_path", pathToJson(result.initial_path));
    appendRawField(out, is_first, "path", pathToJson(result.path));

    // Structured metrics by lab.
    appendRawField(out, is_first, "metrics", metricsToJson(result, lab_name));

    // Stable aliases used by Python scripts and existing reports.
    appendIntField(out, is_first, "initial_distance", result.phase1_distance);
    appendIntField(out, is_first, "final_distance", result.phase2_distance);
    appendIntField(out, is_first, "initial_objective", result.phase1_objective);
    appendIntField(out, is_first, "final_objective", result.final_objective);
    appendIntField(out, is_first, "initial_profit", result.phase1_profit);
    appendIntField(out, is_first, "final_profit", result.phase2_profit);

    // Legacy flat fields kept for backward compatibility.
    appendIntField(out, is_first, "phase1_distance", result.phase1_distance);
    appendIntField(out, is_first, "phase1_profit", result.phase1_profit);
    appendIntField(out, is_first, "phase1_objective", result.phase1_objective);
    appendIntField(out, is_first, "phase2_distance", result.phase2_distance);
    appendIntField(out, is_first, "phase2_profit", result.phase2_profit);
    out << "}";

    return out.str();
}

}  // namespace ResultPrinter
