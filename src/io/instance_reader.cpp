#include "io/instance_reader.h"

#include <cctype>
#include <cmath>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

std::string trim(const std::string& text) {
    size_t left = 0;
    size_t right = text.size();
    while (left < right && std::isspace(static_cast<unsigned char>(text[left])) != 0) {
        ++left;
    }
    while (right > left && std::isspace(static_cast<unsigned char>(text[right - 1])) != 0) {
        --right;
    }
    return text.substr(left, right - left);
}

std::vector<std::string> splitSemicolonLine(const std::string& line) {
    std::vector<std::string> tokens;
    std::stringstream ss(line);
    std::string item;
    while (std::getline(ss, item, ';')) {
        tokens.push_back(trim(item));
    }

    if (!line.empty() && line.back() == ';') {
        tokens.emplace_back();
    }

    return tokens;
}

int parseIntStrict(const std::string& value) {
    size_t pos = 0;
    const int parsed = std::stoi(value, &pos);
    if (pos != value.size()) {
        throw std::invalid_argument("trailing characters");
    }
    return parsed;
}

std::runtime_error formatError(const std::string& file_path, int line_number) {
    return std::runtime_error(
        "Invalid line format at line " + std::to_string(line_number) +
        " in file: " + file_path +
        ". Expected: x;y;profit or id;x;y;profit");
}

std::vector<int> buildDistanceMatrix(const std::vector<int>& x, const std::vector<int>& y) {
    const size_t n = x.size();
    std::vector<int> distance_matrix(n * n, 0);

    for (size_t i = 0; i < n; ++i) {
        for (size_t j = 0; j < n; ++j) {
            const double dx = static_cast<double>(x[i] - x[j]);
            const double dy = static_cast<double>(y[i] - y[j]);
            const int dist = static_cast<int>(std::round(std::sqrt(dx * dx + dy * dy)));
            distance_matrix[i * n + j] = dist;
        }
    }

    return distance_matrix;
}

}

namespace InstanceReader {

Instance loadFromCsv(const std::string& file_path) {
    std::ifstream input(file_path);
    if (!input) {
        throw std::runtime_error("Cannot open instance file: " + file_path);
    }

    std::vector<int> x;
    std::vector<int> y;
    std::vector<int> profits;

    std::string line;
    int line_number = 0;
    while (std::getline(input, line)) {
        ++line_number;
        if (line.empty()) {
            continue;
        }

        const std::vector<std::string> tokens = splitSemicolonLine(line);
        if (tokens.size() != 3 && tokens.size() != 4) {
            throw formatError(file_path, line_number);
        }

        try {
            x.push_back(parseIntStrict(tokens[0]));
            y.push_back(parseIntStrict(tokens[1]));
            profits.push_back(parseIntStrict(tokens[2]));
        } catch (const std::exception&) {
            throw formatError(file_path, line_number);
        }
    }

    Instance instance;
    instance.num_vertices = static_cast<int>(profits.size());
    instance.profits = std::move(profits);
    instance.distance_matrix = buildDistanceMatrix(x, y);
    return instance;
}

}
