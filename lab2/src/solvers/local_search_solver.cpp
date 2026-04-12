#include "local_search_solver.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace {

int prevIndex(const int idx, const int size) {
    return (idx - 1 + size) % size;
}

int nextIndex(const int idx, const int size) {
    return (idx + 1) % size;
}

}  // namespace

LocalSearchSolver::LocalSearchSolver(
    const bool is_steepest,
    const bool use_edge_swap,
    std::string initial_solution_type,
    const int seed,
    const int max_time_ms)
    : is_steepest_(is_steepest),
      use_edge_swap_(use_edge_swap),
      initial_solution_type_(std::move(initial_solution_type)),
      max_time_ms_(max_time_ms),
      rng_(static_cast<std::mt19937::result_type>(seed)) {}

SolveResult LocalSearchSolver::solve(const Instance& instance, const int start_node) {
    const auto t0 = std::chrono::high_resolution_clock::now();

    if (start_node < 0 || start_node >= instance.getNumVertices()) {
        throw std::runtime_error("start_node out of range");
    }

    std::vector<int> route = buildInitialRoute(instance, start_node);
    const std::vector<int> initial_route = route;
    const int unique_count = static_cast<int>(route.size()) - 1;

    std::vector<bool> is_visited(static_cast<size_t>(instance.getNumVertices()), false);
    for (int i = 0; i < unique_count; ++i) {
        is_visited[route[static_cast<size_t>(i)]] = true;
    }

    int current_distance = computeRouteDistance(instance, route);

    SolveResult result;
    result.initial_path = initial_route;
    result.path = route;
    result.phase1_distance = current_distance;
    result.phase1_profit = computeRouteProfit(instance, route);
    result.phase1_objective = -result.phase1_distance;

    while (true) {
        Move chosen{};
        if (is_steepest_) {
            std::vector<Move> neighborhood = generateNeighborhood(instance, route, is_visited);
            if (neighborhood.empty()) {
                break;
            }
            chosen = findSteepestMove(neighborhood);
        } else {
            chosen = findGreedyMoveLazy(instance, route, is_visited);
        }

        if (chosen.delta >= 0.0) {
            break;
        }

        applyMove(route, is_visited, chosen);
        current_distance += static_cast<int>(std::lround(chosen.delta));
    }

    result.path = route;
    result.phase2_distance = current_distance;
    result.phase2_profit = computeRouteProfit(instance, route);
    result.final_objective = -result.phase2_distance;

    const auto t1 = std::chrono::high_resolution_clock::now();
    result.time_ms = std::chrono::duration<double, std::milli>(t1 - t0).count();
    return result;
}

SolveResult LocalSearchSolver::solveRandomWalk(const Instance& instance, const int start_node) {
    if (max_time_ms_ <= 0) {
        throw std::runtime_error("max_time_ms must be positive for random walk");
    }

    const auto t0 = std::chrono::high_resolution_clock::now();

    std::vector<int> route = buildInitialRoute(instance, start_node);
    const std::vector<int> initial_route = route;
    const int unique_count = static_cast<int>(route.size()) - 1;

    std::vector<bool> is_visited(static_cast<size_t>(instance.getNumVertices()), false);
    for (int i = 0; i < unique_count; ++i) {
        is_visited[route[static_cast<size_t>(i)]] = true;
    }

    const int initial_distance = computeRouteDistance(instance, route);
    const int initial_profit = computeRouteProfit(instance, route);

    int current_distance = initial_distance;
    int best_distance = current_distance;
    std::vector<int> best_route = route;

    while (true) {
        const auto now = std::chrono::high_resolution_clock::now();
        const double elapsed = std::chrono::duration<double, std::milli>(now - t0).count();
        if (elapsed >= static_cast<double>(max_time_ms_)) {
            break;
        }

        const Move move = sampleRandomMove(instance, route, is_visited);
        if (move.type == MoveType::INTER_SWAP && move.index1 < 0) {
            break;
        }

        applyMove(route, is_visited, move);
        current_distance += static_cast<int>(std::lround(move.delta));

        if (current_distance < best_distance) {
            best_distance = current_distance;
            best_route = route;
        }
    }

    SolveResult result;
    result.initial_path = initial_route;
    result.path = best_route;
    result.phase1_distance = initial_distance;
    result.phase1_profit = initial_profit;
    result.phase1_objective = -result.phase1_distance;
    result.phase2_distance = best_distance;
    result.phase2_profit = computeRouteProfit(instance, best_route);
    result.final_objective = -result.phase2_distance;
    result.time_ms = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();

    return result;
}

int LocalSearchSolver::chooseTargetVertexCount(const int n) {
    return std::max(3, n / 2);
}

std::vector<int> LocalSearchSolver::buildInitialRoute(const Instance& instance, const int start_node) {
    const int n = instance.getNumVertices();
    if (n < 3) {
        throw std::runtime_error("Instance must contain at least 3 vertices");
    }

    const int target_size = std::min(chooseTargetVertexCount(n), n);
    if (initial_solution_type_ == "random") {
        return buildRandomInitialRoute(instance, start_node, target_size);
    }
    if (initial_solution_type_ == "heuristic") {
        return buildRegretInitialRoute(instance, start_node, target_size);
    }

    throw std::runtime_error("Unknown initial_solution_type: " + initial_solution_type_ + ". Expected random or heuristic");
}

std::vector<int> LocalSearchSolver::buildRandomInitialRoute(const Instance& instance, const int start_node, const int target_size) {
    std::vector<int> candidates;
    candidates.reserve(static_cast<size_t>(instance.getNumVertices() - 1));
    for (int v = 0; v < instance.getNumVertices(); ++v) {
        if (v != start_node) {
            candidates.push_back(v);
        }
    }

    std::shuffle(candidates.begin(), candidates.end(), rng_);

    std::vector<int> route;
    route.reserve(static_cast<size_t>(target_size + 1));
    route.push_back(start_node);
    for (int i = 0; i < target_size - 1; ++i) {
        route.push_back(candidates[static_cast<size_t>(i)]);
    }
    route.push_back(start_node);
    return route;
}

std::vector<int> LocalSearchSolver::buildRegretInitialRoute(const Instance& instance, const int start_node, const int target_size) const {
    const int n = instance.getNumVertices();
    std::vector<bool> visited(static_cast<size_t>(n), false);
    visited[static_cast<size_t>(start_node)] = true;

    int second = -1;
    int best_dist = std::numeric_limits<int>::max();
    for (int v = 0; v < n; ++v) {
        if (visited[static_cast<size_t>(v)]) {
            continue;
        }
        const int d = instance.getDistance(start_node, v);
        if (d < best_dist) {
            best_dist = d;
            second = v;
        }
    }

    std::vector<int> route;
    route.reserve(static_cast<size_t>(target_size + 1));
    route.push_back(start_node);
    route.push_back(second);
    route.push_back(start_node);
    visited[static_cast<size_t>(second)] = true;

    while (static_cast<int>(route.size()) - 1 < target_size) {
        int best_node = -1;
        int best_pos = -1;
        int best_insert_value_overall = std::numeric_limits<int>::lowest();
        int best_regret_overall = std::numeric_limits<int>::lowest();

        for (int v = 0; v < n; ++v) {
            if (visited[static_cast<size_t>(v)]) {
                continue;
            }

            int best_insert_value = std::numeric_limits<int>::lowest();
            int second_best_insert_value = std::numeric_limits<int>::lowest();
            int best_insert_pos = -1;

            for (int i = 0; i + 1 < static_cast<int>(route.size()); ++i) {
                const int a = route[static_cast<size_t>(i)];
                const int b = route[static_cast<size_t>(i + 1)];
                const int delta = instance.getDistance(a, v) + instance.getDistance(v, b) - instance.getDistance(a, b);

                // 2-regret without costs from Lab 1: insertion value is based only on distance delta.
                const int insertion_value = -delta;
                if (insertion_value > best_insert_value) {
                    second_best_insert_value = best_insert_value;
                    best_insert_value = insertion_value;
                    best_insert_pos = i;
                } else if (insertion_value > second_best_insert_value) {
                    second_best_insert_value = insertion_value;
                }
            }

            if (best_insert_pos < 0) {
                continue;
            }

            if (second_best_insert_value == std::numeric_limits<int>::lowest()) {
                second_best_insert_value = best_insert_value;
            }

            const int regret = best_insert_value - second_best_insert_value;
            if (regret > best_regret_overall ||
                (regret == best_regret_overall && best_insert_value > best_insert_value_overall)) {
                best_regret_overall = regret;
                best_insert_value_overall = best_insert_value;
                best_node = v;
                best_pos = best_insert_pos;
            }
        }

        if (best_node < 0) {
            break;
        }

        visited[static_cast<size_t>(best_node)] = true;
        route.insert(route.begin() + best_pos + 1, best_node);
    }

    return route;
}

int LocalSearchSolver::computeRouteDistance(const Instance& instance, const std::vector<int>& route) {
    int distance = 0;
    for (int i = 0; i + 1 < static_cast<int>(route.size()); ++i) {
        distance += instance.getDistance(route[static_cast<size_t>(i)], route[static_cast<size_t>(i + 1)]);
    }
    return distance;
}

int LocalSearchSolver::computeRouteProfit(const Instance& instance, const std::vector<int>& route) {
    int profit = 0;
    for (int i = 0; i + 1 < static_cast<int>(route.size()); ++i) {
        profit += instance.getProfit(route[static_cast<size_t>(i)]);
    }
    return profit;
}

std::vector<Move> LocalSearchSolver::generateNeighborhood(
    const Instance& instance,
    const std::vector<int>& route,
    const std::vector<bool>& is_visited) const {
    const int m = static_cast<int>(route.size()) - 1;
    std::vector<Move> moves;
    moves.reserve(static_cast<size_t>(m * instance.getNumVertices()));

    for (int i = 0; i < m; ++i) {
        for (int node = 0; node < instance.getNumVertices(); ++node) {
            if (is_visited[static_cast<size_t>(node)]) {
                continue;
            }
            moves.push_back(Move{MoveType::INTER_SWAP, i, node, calculateInterSwapDelta(instance, route, i, node)});
        }
    }

    if (use_edge_swap_) {
        for (int i = 0; i < m; ++i) {
            for (int j = i + 1; j < m; ++j) {
                if (nextIndex(i, m) == j || nextIndex(j, m) == i) {
                    continue;
                }
                moves.push_back(Move{MoveType::INTRA_EDGE_SWAP, i, j, calculateIntraEdgeSwapDelta(instance, route, i, j)});
            }
        }
    } else {
        for (int i = 0; i < m; ++i) {
            for (int j = i + 1; j < m; ++j) {
                moves.push_back(Move{MoveType::INTRA_NODE_SWAP, i, j, calculateIntraNodeSwapDelta(instance, route, i, j)});
            }
        }
    }

    return moves;
}

double LocalSearchSolver::calculateInterSwapDelta(
    const Instance& instance,
    const std::vector<int>& route,
    const int route_idx,
    const int unvisited_node) const {
    const int m = static_cast<int>(route.size()) - 1;
    const int idx_prev = prevIndex(route_idx, m);
    const int idx_next = nextIndex(route_idx, m);

    const int prev = route[static_cast<size_t>(idx_prev)];
    const int old_node = route[static_cast<size_t>(route_idx)];
    const int next = route[static_cast<size_t>(idx_next)];

    const int old_cost = instance.getDistance(prev, old_node) + instance.getDistance(old_node, next);
    const int new_cost = instance.getDistance(prev, unvisited_node) + instance.getDistance(unvisited_node, next);

    return static_cast<double>(new_cost - old_cost);
}

double LocalSearchSolver::calculateIntraNodeSwapDelta(
    const Instance& instance,
    const std::vector<int>& route,
    int idx1,
    int idx2) const {
    const int m = static_cast<int>(route.size()) - 1;
    if (idx1 == idx2) {
        return 0.0;
    }
    if (idx1 > idx2) {
        std::swap(idx1, idx2);
    }

    const int v1 = route[static_cast<size_t>(idx1)];
    const int v2 = route[static_cast<size_t>(idx2)];

    const int p1 = route[static_cast<size_t>(prevIndex(idx1, m))];
    const int n1 = route[static_cast<size_t>(nextIndex(idx1, m))];
    const int p2 = route[static_cast<size_t>(prevIndex(idx2, m))];
    const int n2 = route[static_cast<size_t>(nextIndex(idx2, m))];

    if (nextIndex(idx1, m) == idx2) {
        const int old_cost = instance.getDistance(p1, v1) + instance.getDistance(v1, v2) + instance.getDistance(v2, n2);
        const int new_cost = instance.getDistance(p1, v2) + instance.getDistance(v2, v1) + instance.getDistance(v1, n2);
        return static_cast<double>(new_cost - old_cost);
    }

    if (nextIndex(idx2, m) == idx1) {
        const int old_cost = instance.getDistance(p2, v2) + instance.getDistance(v2, v1) + instance.getDistance(v1, n1);
        const int new_cost = instance.getDistance(p2, v1) + instance.getDistance(v1, v2) + instance.getDistance(v2, n1);
        return static_cast<double>(new_cost - old_cost);
    }

    const int old_cost = instance.getDistance(p1, v1) + instance.getDistance(v1, n1) + instance.getDistance(p2, v2) + instance.getDistance(v2, n2);
    const int new_cost = instance.getDistance(p1, v2) + instance.getDistance(v2, n1) + instance.getDistance(p2, v1) + instance.getDistance(v1, n2);

    return static_cast<double>(new_cost - old_cost);
}

double LocalSearchSolver::calculateIntraEdgeSwapDelta(
    const Instance& instance,
    const std::vector<int>& route,
    int idx1,
    int idx2) const {
    const int m = static_cast<int>(route.size()) - 1;
    if (idx1 == idx2) {
        return 0.0;
    }
    if (idx1 > idx2) {
        std::swap(idx1, idx2);
    }

    if (idx1 == 0 && idx2 == m - 1) {
        return 0.0;
    }

    const int prev1 = route[static_cast<size_t>(prevIndex(idx1, m))];
    const int v1 = route[static_cast<size_t>(idx1)];
    const int v2 = route[static_cast<size_t>(idx2)];
    const int next2 = route[static_cast<size_t>(nextIndex(idx2, m))];

    const int old_cost = instance.getDistance(prev1, v1) + instance.getDistance(v2, next2);
    const int new_cost = instance.getDistance(prev1, v2) + instance.getDistance(v1, next2);

    return static_cast<double>(new_cost - old_cost);
}

void LocalSearchSolver::applyMove(std::vector<int>& route, std::vector<bool>& is_visited, const Move& move) const {
    const int m = static_cast<int>(route.size()) - 1;

    if (move.type == MoveType::INTER_SWAP) {
        const int idx = move.index1;
        const int old_node = route[static_cast<size_t>(idx)];
        const int new_node = move.index2;

        route[static_cast<size_t>(idx)] = new_node;
        is_visited[static_cast<size_t>(old_node)] = false;
        is_visited[static_cast<size_t>(new_node)] = true;
    } else if (move.type == MoveType::INTRA_NODE_SWAP) {
        std::swap(route[static_cast<size_t>(move.index1)], route[static_cast<size_t>(move.index2)]);
    } else {
        int i = move.index1;
        int j = move.index2;
        if (i > j) {
            std::swap(i, j);
        }
        std::reverse(route.begin() + i, route.begin() + j + 1);
    }

    route[static_cast<size_t>(m)] = route[0];
}

Move LocalSearchSolver::findSteepestMove(const std::vector<Move>& neighborhood) const {
    Move best{};
    best.delta = 0.0;

    for (const Move& move : neighborhood) {
        if (move.delta < best.delta) {
            best = move;
        }
    }

    return best;
}

Move LocalSearchSolver::findGreedyMoveLazy(
    const Instance& instance,
    const std::vector<int>& route,
    const std::vector<bool>& is_visited) {
    const int m = static_cast<int>(route.size()) - 1;
    const int n = instance.getNumVertices();

    if (m <= 0 || n <= 0) {
        return Move{MoveType::INTER_SWAP, -1, -1, 0.0};
    }

    std::uniform_int_distribution<int> dist_m(0, m - 1);
    std::uniform_int_distribution<int> dist_n(0, n - 1);
    std::uniform_int_distribution<int> dist_dir(0, 1);
    std::uniform_int_distribution<int> dist_order(0, 1);

    const int start_i = dist_m(rng_);
    const int dir_i = dist_dir(rng_) == 1 ? 1 : -1;
    const bool check_inter_first = dist_order(rng_) == 1;

    for (int step_i = 0; step_i < m; ++step_i) {
        const int i = ((start_i + dir_i * step_i) % m + m) % m;

        const int start_j = dist_m(rng_);
        const int start_j_inter = dist_n(rng_);
        const int dir_j = dist_dir(rng_) == 1 ? 1 : -1;

        const auto check_inter = [&]() -> Move {
            for (int step_j = 0; step_j < n; ++step_j) {
                const int j = ((start_j_inter + dir_j * step_j) % n + n) % n;
                if (!is_visited[static_cast<size_t>(j)]) {
                    const double delta = calculateInterSwapDelta(instance, route, i, j);
                    if (delta < 0.0) {
                        return Move{MoveType::INTER_SWAP, i, j, delta};
                    }
                }
            }
            return Move{MoveType::INTER_SWAP, -1, -1, 0.0};
        };

        const auto check_intra = [&]() -> Move {
            for (int step_j = 0; step_j < m; ++step_j) {
                const int j = ((start_j + dir_j * step_j) % m + m) % m;
                if (i == j) {
                    continue;
                }

                if (use_edge_swap_) {
                    if (nextIndex(i, m) == j || nextIndex(j, m) == i) {
                        continue;
                    }
                    const double delta = calculateIntraEdgeSwapDelta(instance, route, i, j);
                    if (delta < 0.0) {
                        return Move{MoveType::INTRA_EDGE_SWAP, i, j, delta};
                    }
                } else {
                    const double delta = calculateIntraNodeSwapDelta(instance, route, i, j);
                    if (delta < 0.0) {
                        return Move{MoveType::INTRA_NODE_SWAP, i, j, delta};
                    }
                }
            }

            const MoveType no_improve_type = use_edge_swap_ ? MoveType::INTRA_EDGE_SWAP : MoveType::INTRA_NODE_SWAP;
            return Move{no_improve_type, -1, -1, 0.0};
        };

        if (check_inter_first) {
            const Move inter_move = check_inter();
            if (inter_move.delta < 0.0) {
                return inter_move;
            }

            const Move intra_move = check_intra();
            if (intra_move.delta < 0.0) {
                return intra_move;
            }
        } else {
            const Move intra_move = check_intra();
            if (intra_move.delta < 0.0) {
                return intra_move;
            }

            const Move inter_move = check_inter();
            if (inter_move.delta < 0.0) {
                return inter_move;
            }
        }
    }

    return Move{MoveType::INTER_SWAP, -1, -1, 0.0};
}

Move LocalSearchSolver::sampleRandomMove(
    const Instance& instance,
    const std::vector<int>& route,
    const std::vector<bool>& is_visited) {
    std::vector<Move> neighborhood = generateNeighborhood(instance, route, is_visited);
    if (neighborhood.empty()) {
        return Move{MoveType::INTER_SWAP, -1, -1, 0.0};
    }
    std::uniform_int_distribution<int> dist(0, static_cast<int>(neighborhood.size()) - 1);
    return neighborhood[static_cast<size_t>(dist(rng_))];
}
