#include "lab3_base_solver.h"

#include <algorithm>
#include <limits>
#include <numeric>
#include <stdexcept>

Lab3BaseSolver::Lab3BaseSolver(const int seed, const int max_time_ms, const int candidate_k)
    : rng_(static_cast<std::mt19937::result_type>(seed)),
      max_time_ms_(max_time_ms),
      candidate_k_(candidate_k) {}

int Lab3BaseSolver::nextIndex(const int idx, const int size) {
    return (idx + 1) % size;
}

int Lab3BaseSolver::prevIndex(const int idx, const int size) {
    return (idx - 1 + size) % size;
}

int Lab3BaseSolver::chooseTargetVertexCount(const int n) {
    return std::max(3, n / 2);
}

std::vector<int> Lab3BaseSolver::buildRandomInitialRoute(
    const Instance& instance,
    const int start_node,
    const int target_size) {
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

Lab3BaseSolver::RouteState Lab3BaseSolver::buildInitialState(const Instance& instance, const int start_node) {
    const int n = instance.getNumVertices();
    if (n < 3) {
        throw std::runtime_error("Instance must contain at least 3 vertices");
    }
    if (start_node < 0 || start_node >= n) {
        throw std::runtime_error("start_node out of range");
    }

    const int target_size = std::min(chooseTargetVertexCount(n), n);
    RouteState state;
    state.route = buildRandomInitialRoute(instance, start_node, target_size);
    state.is_visited.assign(static_cast<size_t>(n), false);

    const int m = static_cast<int>(state.route.size()) - 1;
    for (int i = 0; i < m; ++i) {
        state.is_visited[static_cast<size_t>(state.route[static_cast<size_t>(i)])] = true;
    }

    state.position_by_id.assign(static_cast<size_t>(n), -1);
    state.next_by_id.assign(static_cast<size_t>(n), -1);
    rebuildRouteCaches(state);
    return state;
}

void Lab3BaseSolver::rebuildRouteCaches(RouteState& state) const {
    const int n = static_cast<int>(state.is_visited.size());
    std::fill(state.position_by_id.begin(), state.position_by_id.end(), -1);
    std::fill(state.next_by_id.begin(), state.next_by_id.end(), -1);

    const int m = static_cast<int>(state.route.size()) - 1;
    for (int i = 0; i < m; ++i) {
        const int node = state.route[static_cast<size_t>(i)];
        const int next = state.route[static_cast<size_t>(i + 1)];
        state.position_by_id[static_cast<size_t>(node)] = i;
        if (node >= 0 && node < n) {
            state.next_by_id[static_cast<size_t>(node)] = next;
        }
    }
    state.route.back() = state.route.front();
}

int Lab3BaseSolver::computeRouteDistance(const Instance& instance, const std::vector<int>& route) {
    int distance = 0;
    for (int i = 0; i + 1 < static_cast<int>(route.size()); ++i) {
        distance += instance.getDistance(route[static_cast<size_t>(i)], route[static_cast<size_t>(i + 1)]);
    }
    return distance;
}

int Lab3BaseSolver::computeRouteProfit(const Instance& instance, const std::vector<int>& route) {
    int profit = 0;
    for (int i = 0; i + 1 < static_cast<int>(route.size()); ++i) {
        profit += instance.getProfit(route[static_cast<size_t>(i)]);
    }
    return profit;
}

void Lab3BaseSolver::initializeCandidateMatrix(const Instance& instance) {
    const int n = instance.getNumVertices();
    candidate_matrix_.assign(static_cast<size_t>(n), std::vector<unsigned char>(static_cast<size_t>(n), 0));
    candidate_neighbors_.assign(static_cast<size_t>(n), std::vector<int>());
    const int effective_k = std::max(0, std::min(candidate_k_, n - 1));

    std::vector<int> vertices(static_cast<size_t>(n));
    std::iota(vertices.begin(), vertices.end(), 0);

    for (int u = 0; u < n; ++u) {
        std::vector<int> order;
        order.reserve(static_cast<size_t>(n - 1));
        for (int v : vertices) {
            if (v != u) {
                order.push_back(v);
            }
        }

        std::sort(order.begin(), order.end(), [&](const int lhs, const int rhs) {
            const int dl = instance.getDistance(u, lhs);
            const int dr = instance.getDistance(u, rhs);
            if (dl != dr) {
                return dl < dr;
            }
            return lhs < rhs;
        });

        for (int i = 0; i < effective_k; ++i) {
            int neighbor = order[static_cast<size_t>(i)];
            candidate_matrix_[static_cast<size_t>(u)][static_cast<size_t>(neighbor)] = 1;
        }
    }

    for (int u = 0; u < n; ++u) {
        for (int v = 0; v < n; ++v) {
            if (candidate_matrix_[static_cast<size_t>(u)][static_cast<size_t>(v)]) {
                candidate_neighbors_[static_cast<size_t>(u)].push_back(v);
            }
        }
    }
}

bool Lab3BaseSolver::isCandidateNeighbor(const int from, const int to) const {
    if (from < 0 || to < 0 ||
        from >= static_cast<int>(candidate_matrix_.size()) ||
        to >= static_cast<int>(candidate_matrix_[static_cast<size_t>(from)].size())) {
        return false;
    }
    return candidate_matrix_[static_cast<size_t>(from)][static_cast<size_t>(to)] != 0;
}

double Lab3BaseSolver::calculateAddNodeDelta(
    const Instance& instance,
    const std::vector<int>& route,
    const int edge_idx,
    const int new_node) {
    const int m = static_cast<int>(route.size()) - 1;
    const int idx_next = nextIndex(edge_idx, m);

    const int u = route[static_cast<size_t>(edge_idx)];
    const int w = route[static_cast<size_t>(idx_next)];

    const int added_dist = instance.getDistance(u, new_node) + instance.getDistance(new_node, w) - instance.getDistance(u, w);
    const int added_profit = instance.getProfit(new_node);
    return static_cast<double>(added_dist - added_profit);
}

double Lab3BaseSolver::calculateRemoveNodeDelta(
    const Instance& instance,
    const std::vector<int>& route,
    const int route_idx) {
    const int m = static_cast<int>(route.size()) - 1;
    if (route_idx <= 0 || route_idx >= m) {
        return 0.0;
    }

    const int idx_prev = prevIndex(route_idx, m);
    const int idx_next = nextIndex(route_idx, m);

    const int prev = route[static_cast<size_t>(idx_prev)];
    const int removed = route[static_cast<size_t>(route_idx)];
    const int next = route[static_cast<size_t>(idx_next)];

    const int removed_dist = instance.getDistance(prev, removed) + instance.getDistance(removed, next) - instance.getDistance(prev, next);
    const int removed_profit = instance.getProfit(removed);
    return static_cast<double>(removed_profit - removed_dist);
}

double Lab3BaseSolver::calculateIntraEdgeSwapDelta(
    const Instance& instance,
    const std::vector<int>& route,
    int idx1,
    int idx2) {
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

    const int a = route[static_cast<size_t>(idx1)];
    const int b = route[static_cast<size_t>(idx1 + 1)];
    const int c = route[static_cast<size_t>(idx2)];
    const int d = route[static_cast<size_t>(idx2 + 1)];

    const int old_cost = instance.getDistance(a, b) + instance.getDistance(c, d);
    const int new_cost = instance.getDistance(a, c) + instance.getDistance(b, d);
    return static_cast<double>(new_cost - old_cost);
}

std::vector<MemoryMove> Lab3BaseSolver::collectImprovingMoves(
    const Instance& instance,
    const RouteState& state,
    const EvalConfig& config) const {
    std::vector<MemoryMove> moves;
    const int n = instance.getNumVertices();
    const int m = static_cast<int>(state.route.size()) - 1;

    if (config.use_candidate_filter) {
        std::vector<int> unvisited;
        const int remaining_slots = std::max(0, n - m);
        unvisited.reserve(static_cast<size_t>(remaining_slots));
        for (int i = 0; i < n; ++i) {
            if (!state.is_visited[static_cast<size_t>(i)]) {
                unvisited.push_back(i);
            }
        }

        std::vector<int> edge_checked(static_cast<size_t>(std::max(0, m)), -1);
        for (int v : unvisited) {
            for (int c : candidate_neighbors_[static_cast<size_t>(v)]) {
                if (!state.is_visited[static_cast<size_t>(c)]) {
                    continue;
                }
                const int pos_c = state.position_by_id[static_cast<size_t>(c)];

                if (edge_checked[static_cast<size_t>(pos_c)] != v) {
                    edge_checked[static_cast<size_t>(pos_c)] = v;
                    const double delta1 = calculateAddNodeDelta(instance, state.route, pos_c, v);
                    if (delta1 < 0.0) {
                        const int w = state.route[static_cast<size_t>(pos_c + 1)];
                        moves.push_back(MemoryMove{MemoryMoveType::ADD_NODE, c, v, w, -1, delta1});
                    }
                }

                const int pos_prev = prevIndex(pos_c, m);
                if (edge_checked[static_cast<size_t>(pos_prev)] != v) {
                    edge_checked[static_cast<size_t>(pos_prev)] = v;
                    const double delta2 = calculateAddNodeDelta(instance, state.route, pos_prev, v);
                    if (delta2 < 0.0) {
                        const int u = state.route[static_cast<size_t>(pos_prev)];
                        moves.push_back(MemoryMove{MemoryMoveType::ADD_NODE, u, v, c, -1, delta2});
                    }
                }
            }
        }
    } else {
        for (int i = 0; i < m; ++i) {
            const int u = state.route[static_cast<size_t>(i)];
            const int w = state.route[static_cast<size_t>(i + 1)];

            for (int v = 0; v < n; ++v) {
                if (state.is_visited[static_cast<size_t>(v)]) {
                    continue;
                }

                const double delta = calculateAddNodeDelta(instance, state.route, i, v);
                if (delta < 0.0) {
                    moves.push_back(MemoryMove{MemoryMoveType::ADD_NODE, u, v, w, -1, delta});
                }
            }
        }
    }

    if (m > 3) {
        for (int i = 1; i < m; ++i) {
            const int prev = state.route[static_cast<size_t>(prevIndex(i, m))];
            const int node = state.route[static_cast<size_t>(i)];
            const int next = state.route[static_cast<size_t>(nextIndex(i, m))];
            const double delta = calculateRemoveNodeDelta(instance, state.route, i);
            if (delta < 0.0) {
                moves.push_back(MemoryMove{MemoryMoveType::REMOVE_NODE, prev, node, next, -1, delta});
            }
        }
    }

    if (config.use_candidate_filter) {
        for (int i = 0; i < m; ++i) {
            const int a = state.route[static_cast<size_t>(i)];
            const int b = state.route[static_cast<size_t>(i + 1)];

            for (int c : candidate_neighbors_[static_cast<size_t>(a)]) {
                if (!state.is_visited[static_cast<size_t>(c)]) {
                    continue;
                }
                const int j = state.position_by_id[static_cast<size_t>(c)];
                if (j <= i + 1 || (i == 0 && j == m - 1)) {
                    continue;
                }

                const double delta = calculateIntraEdgeSwapDelta(instance, state.route, i, j);
                if (delta < 0.0) {
                    const int d = state.route[static_cast<size_t>(j + 1)];
                    moves.push_back(MemoryMove{MemoryMoveType::EDGE_SWAP, a, b, c, d, delta});
                }
            }

            for (int d : candidate_neighbors_[static_cast<size_t>(b)]) {
                if (!state.is_visited[static_cast<size_t>(d)]) {
                    continue;
                }
                int j = state.position_by_id[static_cast<size_t>(d)] - 1;
                if (j < 0) {
                    j = m - 1;
                }
                if (j <= i + 1 || (i == 0 && j == m - 1)) {
                    continue;
                }

                const int c = state.route[static_cast<size_t>(j)];
                if (!isCandidateNeighbor(a, c)) {
                    const double delta = calculateIntraEdgeSwapDelta(instance, state.route, i, j);
                    if (delta < 0.0) {
                        moves.push_back(MemoryMove{MemoryMoveType::EDGE_SWAP, a, b, c, d, delta});
                    }
                }
            }
        }
    } else {
        for (int i = 0; i < m; ++i) {
            for (int j = i + 1; j < m; ++j) {
                if (nextIndex(i, m) == j || nextIndex(j, m) == i) {
                    continue;
                }

                const int a = state.route[static_cast<size_t>(i)];
                const int b = state.route[static_cast<size_t>(i + 1)];
                const int c = state.route[static_cast<size_t>(j)];
                const int d = state.route[static_cast<size_t>(j + 1)];

                const double delta = calculateIntraEdgeSwapDelta(instance, state.route, i, j);
                if (delta < 0.0) {
                    moves.push_back(MemoryMove{MemoryMoveType::EDGE_SWAP, a, b, c, d, delta});
                }
            }
        }
    }

    return moves;
}

bool Lab3BaseSolver::isMoveApplicable(const MemoryMove& move, const RouteState& state) const {
    const auto next_of = [&](const int node_id) {
        if (node_id < 0 || node_id >= static_cast<int>(state.next_by_id.size())) {
            return -1;
        }
        return state.next_by_id[static_cast<size_t>(node_id)];
    };

    switch (move.type) {
    case MemoryMoveType::ADD_NODE:
        if (move.id_b < 0 || move.id_b >= static_cast<int>(state.is_visited.size())) {
            return false;
        }
        return !state.is_visited[static_cast<size_t>(move.id_b)] && next_of(move.id_a) == move.id_c;
    case MemoryMoveType::REMOVE_NODE:
        if (move.id_b < 0 || move.id_b >= static_cast<int>(state.is_visited.size())) {
            return false;
        }
        if (move.id_b == state.route.front()) {
            return false;
        }
        return state.is_visited[static_cast<size_t>(move.id_b)] &&
            next_of(move.id_a) == move.id_b &&
            next_of(move.id_b) == move.id_c;
    case MemoryMoveType::EDGE_SWAP:
        if (move.id_a == move.id_c || move.id_b == move.id_d ||
            move.id_a == move.id_d || move.id_b == move.id_c) {
            return false;
        }
        return next_of(move.id_a) == move.id_b && next_of(move.id_c) == move.id_d;
    }

    return false;
}

double Lab3BaseSolver::evaluateMoveDeltaCurrent(
    const Instance& instance,
    const RouteState& state,
    const MemoryMove& move) const {
    if (!isMoveApplicable(move, state)) {
        return std::numeric_limits<double>::infinity();
    }

    switch (move.type) {
    case MemoryMoveType::ADD_NODE: {
        const int idx_u = state.position_by_id[static_cast<size_t>(move.id_a)];
        if (idx_u < 0) {
            return std::numeric_limits<double>::infinity();
        }
        return calculateAddNodeDelta(instance, state.route, idx_u, move.id_b);
    }
    case MemoryMoveType::REMOVE_NODE: {
        const int idx_b = state.position_by_id[static_cast<size_t>(move.id_b)];
        if (idx_b <= 0) {
            return std::numeric_limits<double>::infinity();
        }
        return calculateRemoveNodeDelta(instance, state.route, idx_b);
    }
    case MemoryMoveType::EDGE_SWAP: {
        const int idx_a = state.position_by_id[static_cast<size_t>(move.id_a)];
        const int idx_c = state.position_by_id[static_cast<size_t>(move.id_c)];
        if (idx_a < 0 || idx_c < 0 || idx_a >= idx_c) {
            return std::numeric_limits<double>::infinity();
        }
        return calculateIntraEdgeSwapDelta(instance, state.route, idx_a, idx_c);
    }
    }

    return std::numeric_limits<double>::infinity();
}

bool Lab3BaseSolver::applyMove(RouteState& state, const MemoryMove& move) const {
    if (!isMoveApplicable(move, state)) {
        return false;
    }

    switch (move.type) {
    case MemoryMoveType::ADD_NODE: {
        const int idx_u = state.position_by_id[static_cast<size_t>(move.id_a)];
        if (idx_u < 0) {
            return false;
        }
        state.route.insert(state.route.begin() + idx_u + 1, move.id_b);
        state.is_visited[static_cast<size_t>(move.id_b)] = true;
        break;
    }
    case MemoryMoveType::REMOVE_NODE: {
        const int idx_b = state.position_by_id[static_cast<size_t>(move.id_b)];
        if (idx_b <= 0 || idx_b >= static_cast<int>(state.route.size()) - 1) {
            return false;
        }
        state.route.erase(state.route.begin() + idx_b);
        state.is_visited[static_cast<size_t>(move.id_b)] = false;
        break;
    }
    case MemoryMoveType::EDGE_SWAP: {
        const int idx_a = state.position_by_id[static_cast<size_t>(move.id_a)];
        const int idx_c = state.position_by_id[static_cast<size_t>(move.id_c)];
        if (idx_a < 0 || idx_c < 0 || idx_a >= idx_c) {
            return false;
        }
        std::reverse(state.route.begin() + idx_a + 1, state.route.begin() + idx_c + 1);
        break;
    }
    }

    state.route.back() = state.route.front();
    rebuildRouteCaches(state);
    return true;
}
