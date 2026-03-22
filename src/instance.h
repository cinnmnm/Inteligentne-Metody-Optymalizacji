#pragma once

#include <cstddef>
#include <vector>

struct Instance {
    int num_vertices = 0;
    std::vector<int> profits;
    std::vector<int> distance_matrix;

    int getNumVertices() const {
        return num_vertices;
    }

    int getProfit(int i) const {
        return profits.at(static_cast<std::size_t>(i));
    }

    inline int getDistance(int i, int j) const {
        return distance_matrix[static_cast<std::size_t>(i) * static_cast<std::size_t>(num_vertices) + static_cast<std::size_t>(j)];
    }
};
