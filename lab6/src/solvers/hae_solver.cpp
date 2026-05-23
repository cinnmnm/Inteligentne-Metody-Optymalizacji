#include "hae_solver.h"

#include <algorithm>
#include <chrono>
#include <iostream>
#include <limits>
#include <random>
#include <stdexcept>
#include <unordered_map>

namespace {

using Edge = std::pair<int, int>;

Edge normalizeEdge(const int u, const int v) {
    return {std::min(u, v), std::max(u, v)};
}

std::vector<int> uniqueRoute(const std::vector<int>& route) {
    if (route.empty()) {
        return {};
    }
    std::vector<int> unique;
    unique.reserve(route.size());

    std::unordered_set<int> seen;
    const size_t limit = (route.size() > 1 && route.front() == route.back()) ? route.size() - 1 : route.size();
    for (size_t i = 0; i < limit; ++i) {
        const int node = route[i];
        if (seen.insert(node).second) {
            unique.push_back(node);
        }
    }
    return unique;
}

std::vector<int> closeRoute(std::vector<int> route) {
    if (!route.empty() && route.front() != route.back()) {
        route.push_back(route.front());
    }
    return route;
}

std::vector<std::vector<int>> extractCommonSubpathsFromGraph(
    const std::unordered_set<int>& vertices,
    const std::unordered_set<Edge, HAE_Solver::PairHash>& edges) {
    std::unordered_map<int, std::vector<int>> adjacency;
    for (const int vertex : vertices) {
        adjacency[vertex];
    }
    for (const auto& edge : edges) {
        adjacency[edge.first].push_back(edge.second);
        adjacency[edge.second].push_back(edge.first);
    }

    std::unordered_set<int> seen;
    std::vector<std::vector<int>> segments;
    for (const int start_vertex : vertices) {
        if (seen.count(start_vertex) != 0U) {
            continue;
        }

        std::vector<int> component;
        std::vector<int> stack{start_vertex};
        seen.insert(start_vertex);
        while (!stack.empty()) {
            const int current = stack.back();
            stack.pop_back();
            component.push_back(current);
            for (const int next : adjacency[current]) {
                if (seen.insert(next).second) {
                    stack.push_back(next);
                }
            }
        }

        if (component.empty()) {
            continue;
        }

        int start = component.front();
        for (const int vertex : component) {
            if (adjacency[vertex].size() == 1U) {
                start = vertex;
                break;
            }
        }

        std::vector<int> path;
        path.push_back(start);
        int prev = -1;
        int current = start;
        while (true) {
            int next = -1;
            for (const int candidate : adjacency[current]) {
                if (candidate != prev) {
                    next = candidate;
                    break;
                }
            }
            if (next < 0 || next == start) {
                break;
            }
            path.push_back(next);
            prev = current;
            current = next;
        }

        segments.push_back(std::move(path));
    }

    return segments;
}

std::vector<std::vector<int>> splitByMissingEdges(
    std::vector<int> route,
    const std::unordered_set<Edge, HAE_Solver::PairHash>& parent_edges) {
    route = uniqueRoute(route);
    if (route.size() <= 1) {
        return {route};
    }

    const int m = static_cast<int>(route.size());
    int cut_index = -1;
    for (int i = 0; i < m; ++i) {
        if (parent_edges.count(normalizeEdge(route[static_cast<size_t>(i)], route[static_cast<size_t>((i + 1) % m)])) == 0U) {
            cut_index = i;
            break;
        }
    }

    if (cut_index < 0) {
        return {route};
    }

    std::rotate(route.begin(), route.begin() + ((cut_index + 1) % m), route.end());

    std::vector<std::vector<int>> segments;
    std::vector<int> current{route.front()};
    for (int i = 0; i + 1 < m; ++i) {
        current.push_back(route[static_cast<size_t>(i + 1)]);
        if (parent_edges.count(normalizeEdge(route[static_cast<size_t>(i)], route[static_cast<size_t>(i + 1)])) == 0U) {
            if (current.size() > 1) {
                segments.push_back(current);
            }
            current.clear();
            current.push_back(route[static_cast<size_t>(i + 1)]);
        }
    }

    if (current.size() > 1) {
        segments.push_back(current);
    }

    return segments;
}

}  // namespace

std::size_t HAE_Solver::PairHash::operator()(const std::pair<int, int>& edge) const noexcept {
    return std::hash<int>{}(edge.first) ^ (std::hash<int>{}(edge.second) << 1U);
}

HAE_Solver::HAE_Solver(const int seed, const int max_time_ms, const HaeOperator op_type, const bool use_local_search)
    : LNS_Solver(seed, max_time_ms, 0.3, use_local_search), op_type_(op_type) {}

std::vector<int> HAE_Solver::canonizeRoute(const std::vector<int>& route) const {
    std::vector<int> canonical = uniqueRoute(route);
    if (canonical.empty()) {
        return canonical;
    }

    const auto min_it = std::min_element(canonical.begin(), canonical.end());
    const int min_index = static_cast<int>(std::distance(canonical.begin(), min_it));
    const int n = static_cast<int>(canonical.size());
    const int left = canonical[static_cast<size_t>((min_index - 1 + n) % n)];
    const int right = canonical[static_cast<size_t>((min_index + 1) % n)];

    if (right > left) {
        std::reverse(canonical.begin(), canonical.end());
    }

    const auto rotated_min = std::min_element(canonical.begin(), canonical.end());
    std::rotate(canonical.begin(), rotated_min, canonical.end());
    return canonical;
}

std::unordered_set<int> HAE_Solver::computeVertices(const std::vector<int>& route) const {
    std::unordered_set<int> vertices;
    for (const int node : uniqueRoute(route)) {
        vertices.insert(node);
    }
    return vertices;
}

std::unordered_set<std::pair<int, int>, HAE_Solver::PairHash> HAE_Solver::computeEdges(const std::vector<int>& route) const {
    std::unordered_set<std::pair<int, int>, PairHash> edges;
    const std::vector<int> unique = uniqueRoute(route);
    if (unique.size() < 2) {
        return edges;
    }

    for (size_t i = 0; i < unique.size(); ++i) {
        edges.insert(normalizeEdge(unique[i], unique[(i + 1) % unique.size()]));
    }
    return edges;
}

HAE_Solver::RouteState HAE_Solver::makeStateFromRoute(const Instance& instance, const Route& route) const {
    RouteState state;
    state.route = closeRoute(uniqueRoute(route.empty() ? Route{0} : route));
    state.is_visited.assign(static_cast<size_t>(instance.getNumVertices()), false);
    state.position_by_id.assign(static_cast<size_t>(instance.getNumVertices()), -1);
    state.next_by_id.assign(static_cast<size_t>(instance.getNumVertices()), -1);

    for (const int node : uniqueRoute(state.route)) {
        state.is_visited[static_cast<size_t>(node)] = true;
    }

    rebuildRouteCaches(state);
    return state;
}

HAE_Solver::PopulationEntry HAE_Solver::makePopulationEntry(const Instance& instance, const RouteState& state) const {
    PopulationEntry entry;
    entry.state = state;
    entry.canonical_route = canonizeRoute(state.route);
    entry.distance = computeRouteDistance(instance, state.route);
    entry.profit = computeRouteProfit(instance, state.route);
    entry.objective = entry.profit - entry.distance;
    return entry;
}

bool HAE_Solver::isDuplicate(const std::vector<PopulationEntry>& population, const PopulationEntry& candidate) const {
    for (const auto& item : population) {
        if (item.objective == candidate.objective && item.canonical_route == candidate.canonical_route) {
            return true;
        }
    }
    return false;
}

bool HAE_Solver::tryInsertPopulation(std::vector<PopulationEntry>& population, const PopulationEntry& candidate) const {
    if (isDuplicate(population, candidate)) {
        return false;
    }

    const auto better = [](const PopulationEntry& lhs, const PopulationEntry& rhs) {
        if (lhs.objective != rhs.objective) {
            return lhs.objective > rhs.objective;
        }
        return lhs.canonical_route < rhs.canonical_route;
    };

    if (population.size() < population_size_) {
        population.push_back(candidate);
        std::sort(population.begin(), population.end(), better);
        return true;
    }

    if (candidate.objective < population.back().objective) {
        return false;
    }

    population.back() = candidate;
    std::sort(population.begin(), population.end(), better);
    return true;
}

int HAE_Solver::selectOperator() {
    if (op_type_ != HaeOperator::ADAPTIVE) {
        return static_cast<int>(op_type_);
    }

    std::uniform_int_distribution<int> dist(0, 2);
    return dist(rng_);
}

HAE_Solver::Route HAE_Solver::recombinationOp1(const Route& p1, const Route& p2) {
    const auto vertices1 = computeVertices(p1);
    const auto vertices2 = computeVertices(p2);
    const auto edges1 = computeEdges(p1);
    const auto edges2 = computeEdges(p2);

    std::unordered_set<int> common_vertices;
    for (const int vertex : vertices1) {
        if (vertices2.count(vertex) != 0U) {
            common_vertices.insert(vertex);
        }
    }

    std::unordered_set<std::pair<int, int>, PairHash> common_edges;
    for (const auto& edge : edges1) {
        if (edges2.count(edge) != 0U) {
            common_edges.insert(edge);
        }
    }

    std::vector<std::vector<int>> segments;
    std::unordered_set<int> used_in_segments;

    std::vector<int> u_p1 = uniqueRoute(p1);
    if (u_p1.size() > 1) {
        std::vector<int> current_segment;
        for (size_t i = 0; i < u_p1.size(); ++i) {
            int u = u_p1[i];
            int v = u_p1[(i + 1) % u_p1.size()];
            
            if (common_edges.count(normalizeEdge(u, v)) != 0U) {
                if (current_segment.empty()) {
                    current_segment.push_back(u);
                    used_in_segments.insert(u);
                }
                if (used_in_segments.insert(v).second) {
                    current_segment.push_back(v);
                }
            } else {
                if (!current_segment.empty()) {
                    segments.push_back(std::move(current_segment));
                    current_segment.clear();
                }
            }
        }
        if (!current_segment.empty()) {
            if (segments.size() > 0 && segments.front().front() == current_segment.back()) {
                current_segment.pop_back();
                segments.front().insert(segments.front().begin(), current_segment.begin(), current_segment.end());
            } else {
                segments.push_back(std::move(current_segment));
            }
        }
    }

    for (const int v : common_vertices) {
        if (used_in_segments.count(v) == 0U) {
            segments.push_back(std::vector<int>{v});
            used_in_segments.insert(v);
        }
    }

    if (segments.empty()) {
        return uniqueRoute(p1);
    }

    std::shuffle(segments.begin(), segments.end(), rng_);
    for (auto& segment : segments) {
        if (segment.size() > 1) {
            std::bernoulli_distribution reverse_dist(0.5);
            if (reverse_dist(rng_)) {
                std::reverse(segment.begin(), segment.end());
            }
        }
    }

    Route child;
    for (const auto& segment : segments) {
        child.insert(child.end(), segment.begin(), segment.end());
    }

    if (child.empty()) {
        return uniqueRoute(p1);
    }

    return closeRoute(std::move(child));
}

HAE_Solver::Route HAE_Solver::recombinationOp2(const Route& p1, const Route& p2) {
    std::vector<int> u_p1 = uniqueRoute(p1);
    const auto p2_vertices = computeVertices(p2);
    const auto p2_edges = computeEdges(p2);

    if (u_p1.empty()) {
        return uniqueRoute(p1);
    }

    std::vector<int> filtered_vertices;
    for (int v : u_p1) {
        if (p2_vertices.count(v) != 0U) {
            filtered_vertices.push_back(v);
        }
    }

    if (filtered_vertices.empty()) {
        return uniqueRoute(p1);
    }

    std::vector<std::vector<int>> segments;
    std::vector<int> current_segment;
    const size_t m = filtered_vertices.size();

    if (m == 1) {
        segments.push_back(filtered_vertices);
    } else {
        int start_idx = 0;
        bool found_cut = false;
        for (size_t i = 0; i < m; ++i) {
            int u = filtered_vertices[i];
            int v = filtered_vertices[(i + 1) % m];
            if (p2_edges.count(normalizeEdge(u, v)) == 0U) {
                start_idx = static_cast<int>((i + 1) % m);
                found_cut = true;
                break;
            }
        }

        if (found_cut) {
            std::rotate(filtered_vertices.begin(), filtered_vertices.begin() + start_idx, filtered_vertices.end());
        }

        current_segment.push_back(filtered_vertices[0]);
        for (size_t i = 0; i < m; ++i) {
            int u = filtered_vertices[i];
            if (i + 1 < m) {
                int v = filtered_vertices[i + 1];
                if (p2_edges.count(normalizeEdge(u, v)) != 0U) {
                    current_segment.push_back(v);
                } else {
                    segments.push_back(std::move(current_segment));
                    current_segment.clear();
                    current_segment.push_back(v);
                }
            } else {
                segments.push_back(std::move(current_segment));
            }
        }
    }

    std::vector<std::vector<int>> filtered_segments;
    std::unordered_set<int> global_used;
    for (auto& seg : segments) {
        if (seg.size() > 1) {
            std::vector<int> clean_seg;
            for (int v : seg) {
                if (global_used.insert(v).second) {
                    clean_seg.push_back(v);
                }
            }
            if (!clean_seg.empty()) {
                filtered_segments.push_back(std::move(clean_seg));
            }
        }
    }

    if (filtered_segments.empty()) {
        global_used.clear();
        for (auto& seg : segments) {
            std::vector<int> clean_seg;
            for (int v : seg) {
                if (global_used.insert(v).second) {
                    clean_seg.push_back(v);
                }
            }
            if (!clean_seg.empty()) {
                filtered_segments.push_back(std::move(clean_seg));
            }
        }
    }

    if (filtered_segments.empty()) {
        return uniqueRoute(p1);
    }

    std::shuffle(filtered_segments.begin(), filtered_segments.end(), rng_);
    for (auto& segment : filtered_segments) {
        if (segment.size() > 1) {
            std::bernoulli_distribution reverse_dist(0.5);
            if (reverse_dist(rng_)) {
                std::reverse(segment.begin(), segment.end());
            }
        }
    }

    Route child;
    for (const auto& segment : filtered_segments) {
        child.insert(child.end(), segment.begin(), segment.end());
    }

    if (child.empty()) {
        return uniqueRoute(p1);
    }

    return closeRoute(std::move(child));
}

HAE_Solver::Route HAE_Solver::recombinationOp3(const Route& p1, const Route& p2) {
    std::vector<int> u_p1 = uniqueRoute(p1);
    const auto p2_vertices = computeVertices(p2);

    if (u_p1.empty()) {
        return uniqueRoute(p1);
    }

    Route child;
    std::unordered_set<int> global_used;
    
    for (int v : u_p1) {
        if (p2_vertices.count(v) != 0U) {
            if (global_used.insert(v).second) {
                child.push_back(v);
            }
        }
    }

    if (child.empty()) {
        return uniqueRoute(p1);
    }

    return closeRoute(std::move(child));
}

HAE_Solver::Route HAE_Solver::recombine(const Route& p1, const Route& p2) {
    switch (selectOperator()) {
    case 0:
        return recombinationOp1(p1, p2);
    case 1:
        return recombinationOp2(p1, p2);
    case 2:
    default:
        return recombinationOp3(p1, p2);
    }
}

void HAE_Solver::repairHae(const Instance& instance, RouteState& state) {
    std::vector<int> removed;
    destroy(state, removed);
    if (!removed.empty()) {
        repair(instance, state, removed);
    }

    std::bernoulli_distribution mutation_dist(0.35);
    if (mutation_dist(rng_) && state.route.size() > 6) {
        std::vector<int> current_nodes = uniqueRoute(state.route);
        
        if (current_nodes.size() > 5) {
            for (int k = 0; k < 3; ++k) {
                std::uniform_int_distribution<size_t> idx_dist(1, current_nodes.size() - 1);
                size_t erase_idx = idx_dist(rng_);
                current_nodes.erase(current_nodes.begin() + erase_idx);
            }
        }

        std::vector<int> fresh_unvisited;
        std::unordered_set<int> current_set(current_nodes.begin(), current_nodes.end());
        for (int v = 0; v < instance.getNumVertices(); ++v) {
            if (current_set.count(v) == 0) {
                fresh_unvisited.push_back(v);
            }
        }

        if (!fresh_unvisited.empty()) {
            std::shuffle(fresh_unvisited.begin(), fresh_unvisited.end(), rng_);
            size_t to_add = std::min(size_t(3), fresh_unvisited.size());
            std::vector<int> mutate_insert(fresh_unvisited.begin(), fresh_unvisited.begin() + to_add);

            state.route = closeRoute(current_nodes);
            std::fill(state.is_visited.begin(), state.is_visited.end(), false);
            for (int node : current_nodes) {
                state.is_visited[static_cast<size_t>(node)] = true;
            }
            rebuildRouteCaches(state);

            repair(instance, state, mutate_insert);
        }
    }

    std::fill(state.is_visited.begin(), state.is_visited.end(), false);
    for (size_t i = 0; i < state.route.size() - 1; ++i) {
        state.is_visited[static_cast<size_t>(state.route[i])] = true;
    }

    rebuildRouteCaches(state);
}

void HAE_Solver::localSearchImproveIfEnabled(const Instance& instance, RouteState& state, bool force) {
    if (!use_local_search_ && !force) {
        return;
    }

    const EvalConfig config{true};
    std::vector<MemoryMove> lm = collectImprovingMoves(instance, state, config);
    if (!lm.empty()) {
        std::sort(lm.begin(), lm.end(), [](const MemoryMove& lhs, const MemoryMove& rhs) {
            return lhs.delta < rhs.delta;
        });
    }

    while (!lm.empty()) {
        bool applied = false;
        size_t idx = 0;
        while (idx < lm.size()) {
            MemoryMove move = lm[idx++];
            if (!isMoveApplicable(move, state)) {
                continue;
            }

            const double current_delta = evaluateMoveDeltaCurrent(instance, state, move);
            if (current_delta >= 0.0) {
                continue;
            }

            move.delta = current_delta;
            if (applyMove(state, move)) {
                applied = true;
            }
        }

        if (!applied) {
            break;
        }

        lm = collectImprovingMoves(instance, state, config);
        if (!lm.empty()) {
            std::sort(lm.begin(), lm.end(), [](const MemoryMove& lhs, const MemoryMove& rhs) {
                return lhs.delta < rhs.delta;
            });
        }
    }

    state.route = closeRoute(uniqueRoute(state.route));
    rebuildRouteCaches(state);
}

SolveResult HAE_Solver::solve(const Instance& instance, const int start_node) {
    const auto t0 = std::chrono::high_resolution_clock::now();

    initializeCandidateMatrix(instance);

    std::uniform_int_distribution<int> start_dist(0, std::max(0, instance.getNumVertices() - 1));

    std::vector<PopulationEntry> population;
    population.reserve(population_size_);

    PopulationEntry initial_seed_entry;
    bool has_initial_seed_entry = false;

    int population_attempts = 0;
    while (static_cast<int>(population.size()) < population_size_ && population_attempts < population_size_ * 200) {
        const double elapsed_ms = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();
        if (elapsed_ms >= static_cast<double>(max_time_ms_)) {
            break;
        }

        ++population_attempts;
        const int init_start = (population.empty() ? start_node : start_dist(rng_));
        RouteState state = buildInitialState(instance, init_start);

        if (!has_initial_seed_entry) {
            initial_seed_entry = makePopulationEntry(instance, state);
            has_initial_seed_entry = true;
        }

        localSearchImproveIfEnabled(instance, state, true);
        const PopulationEntry candidate = makePopulationEntry(instance, state);
        tryInsertPopulation(population, candidate);
    }

    if (population.empty()) {
        throw std::runtime_error("HAE population could not be initialized");
    }

    const PopulationEntry initial_best = population.front();
    PopulationEntry best = initial_best;

    int iterations = 0;
    while (true) {
        const double elapsed_ms = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();
        if (elapsed_ms >= static_cast<double>(max_time_ms_)) {
            break;
        }

        std::uniform_int_distribution<int> index_dist(0, static_cast<int>(population.size()) - 1);
        const int parent1_idx = index_dist(rng_);
        int parent2_idx = index_dist(rng_);
        while (parent2_idx == parent1_idx) {
            parent2_idx = index_dist(rng_);
        }

        const Route& parent1 = population[static_cast<size_t>(parent1_idx)].state.route;
        const Route& parent2 = population[static_cast<size_t>(parent2_idx)].state.route;

        Route child_route = recombine(parent1, parent2);
        RouteState child_state = makeStateFromRoute(instance, child_route);
        repairHae(instance, child_state);
        localSearchImproveIfEnabled(instance, child_state);

        const PopulationEntry child = makePopulationEntry(instance, child_state);
        ++iterations;

        if (child.objective > best.objective ||
            (child.objective == best.objective && child.canonical_route < best.canonical_route)) {
            best = child;
        }

        tryInsertPopulation(population, child);
    }

    SolveResult result;
    result.initial_path = has_initial_seed_entry ? initial_seed_entry.state.route : initial_best.state.route;
    result.path = best.state.route;
    result.phase1_distance = has_initial_seed_entry ? initial_seed_entry.distance : initial_best.distance;
    result.phase1_profit = has_initial_seed_entry ? initial_seed_entry.profit : initial_best.profit;
    result.phase1_objective = has_initial_seed_entry ? initial_seed_entry.objective : initial_best.objective;
    result.phase2_distance = best.distance;
    result.phase2_profit = best.profit;
    result.final_objective = best.objective;
    result.time_ms = std::chrono::duration<double, std::milli>(std::chrono::high_resolution_clock::now() - t0).count();
    result.iterations = iterations;
    return result;
}