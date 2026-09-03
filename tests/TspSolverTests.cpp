#include "TspSolver.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <fstream>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace tsp {

struct BranchBoundSolverTestAccess {
    using Edge = BranchBoundSolver::Edge;
    using Node = BranchBoundSolver::PartialSol;
    using OneTree = BranchBoundSolver::OneTree;

    struct Fixture {
        struct AlphaEntry {
            int u = -1;
            int v = -1;
            double value = std::numeric_limits<double>::infinity();
        };

        explicit Fixture(std::vector<std::vector<double>> matrix)
            : solver(std::move(matrix))
        {
            initialize();
        }

        BranchBoundSolver solver;
        Node node;
        std::vector<Edge> candidates;
        OneTree tree;

        void initialize()
        {
            const int n = solver.n_;
            const std::size_t state_size = static_cast<std::size_t>(n) * n;
            node.forced.assign(state_size, 0);
            node.forbidden.assign(state_size, 0);
            node.forced_parent.resize(static_cast<std::size_t>(n));
            std::iota(node.forced_parent.begin(), node.forced_parent.end(), 0);
            node.forced_rank.assign(static_cast<std::size_t>(n), 0);
            node.forced_comp_size.assign(static_cast<std::size_t>(n), 1);
            node.forced_degree.assign(static_cast<std::size_t>(n), 0);
            node.candidate_mask.assign(state_size, 0);

            for (int u = 0; u < n; ++u) {
                for (int v = u + 1; v < n; ++v) {
                    const double weight = solver.dist_[u][v];
                    if (!std::isfinite(weight)) continue;
                    candidates.push_back(Edge{u, v, weight});
                    node.candidate_mask[solver.edgeId(u, v)] = 1;
                }
            }
            sortEdges(candidates);

            solver.root_candidates_sorted_.clear();
            for (int v = 1; v < n; ++v) {
                if (std::isfinite(solver.dist_[0][v])) {
                    solver.root_candidates_sorted_.push_back(Edge{0, v, solver.dist_[0][v]});
                }
            }
            sortEdges(solver.root_candidates_sorted_);
            tree = solver.computeOneTree(node, candidates);
        }

        static void sortEdges(std::vector<Edge>& edges)
        {
            std::sort(edges.begin(), edges.end(), [](const Edge& a, const Edge& b) {
                if (a.w != b.w) return a.w < b.w;
                if (a.u != b.u) return a.u < b.u;
                return a.v < b.v;
            });
        }

        Edge edge(int u, int v) const
        {
            if (u > v) std::swap(u, v);
            return Edge{u, v, solver.dist_[u][v]};
        }

        bool contains(const OneTree& value, int u, int v) const
        {
            const std::size_t id = solver.edgeId(u, v);
            return std::any_of(value.edges.begin(), value.edges.end(), [&](const Edge& e) {
                return solver.edgeId(e.u, e.v) == id;
            });
        }

        OneTree forbidAndCompare(int u, int v, bool require_same_degree = true)
        {
            const Edge removed = edge(u, v);
            const std::size_t removed_id = solver.edgeId(u, v);
            node.forbidden[removed_id] = 1;

            OneTree incremental = tree;
            solver.updateOneTreeAfterForbid(node, candidates, incremental, removed);

            std::vector<Edge> allowed;
            allowed.reserve(candidates.size());
            for (const Edge& candidate : candidates) {
                if (!node.forbidden[solver.edgeId(candidate.u, candidate.v)]) {
                    allowed.push_back(candidate);
                }
            }
            const OneTree rebuilt = solver.computeOneTree(node, allowed);

            expect(incremental.feasible == rebuilt.feasible,
                   "incremental and rebuilt feasibility differ");
            if (incremental.feasible) {
                expect(std::fabs(incremental.cost - rebuilt.cost) <= 1e-9,
                       "incremental and rebuilt costs differ");
                if (require_same_degree) {
                    expect(incremental.degree == rebuilt.degree,
                           "incremental and rebuilt degrees differ");
                }
                validate(incremental);
            }

            tree = incremental;
            return rebuilt;
        }

        bool forbidLightestSelectedEdgeAndCompare(bool require_same_degree = true)
        {
            if (!tree.feasible) return false;
            const Edge* selected = nullptr;
            for (const Edge& current : tree.edges) {
                const std::size_t id = solver.edgeId(current.u, current.v);
                if (node.forced[id] || node.forbidden[id]) continue;
                if (selected == nullptr || current.w < selected->w) {
                    selected = &current;
                }
            }
            if (selected == nullptr) return false;
            const int u = selected->u;
            const int v = selected->v;
            forbidAndCompare(u, v, require_same_degree);
            return tree.feasible;
        }

        void force(int u, int v)
        {
            const Edge forced_edge = edge(u, v);
            const std::size_t id = solver.edgeId(u, v);
            expect(!node.forced[id] && !node.forbidden[id], "invalid force setup");
            node.forced[id] = 1;
            node.forced_edges.push_back(forced_edge);
            ++node.forced_degree[static_cast<std::size_t>(u)];
            ++node.forced_degree[static_cast<std::size_t>(v)];
            if (u != 0 && v != 0) {
                node.forced_mst_cost += forced_edge.w;
                ++node.forced_mst_count;
            }

            int root_u = findForced(u);
            int root_v = findForced(v);
            if (root_u != root_v) {
                node.forced_parent[static_cast<std::size_t>(root_v)] = root_u;
                node.forced_comp_size[static_cast<std::size_t>(root_u)]
                    += node.forced_comp_size[static_cast<std::size_t>(root_v)];
            }
        }

        void rebuildAfterForces()
        {
            std::vector<std::size_t> removed_ids;
            expect(solver.buildBranchCandidates(node, candidates, &removed_ids),
                   "forced test state unexpectedly infeasible");
            for (const std::size_t id : removed_ids) {
                node.candidate_mask[id] = 0;
            }
            tree = solver.computeOneTree(node, candidates);
            expect(tree.feasible, "forced test state has no 1-tree");
            validate(tree);
        }

        void verifyForcedMstCacheRestore()
        {
            node.forced_mst_cost = 1.0;
            node.forced_mst_count = 1;
            const double old_cost = node.forced_mst_cost;
            const int old_count = node.forced_mst_count;

            node.forced_mst_cost += 1e300;
            ++node.forced_mst_count;
            expect(node.forced_mst_cost - 1e300 != old_cost,
                   "test setup did not lose the small cached cost");

            solver.restoreForcedMstCache(node, old_cost, old_count);
            expect(node.forced_mst_cost == old_cost
                       && node.forced_mst_count == old_count,
                   "force rollback did not restore the exact cached MST state");
        }

        void verifyNodePotentialPrimBound()
        {
            expect(tree.feasible,
                   "cannot compare Prim potential bound on an infeasible tree");
            const double upper_bound = tree.cost
                + std::max(1.0, std::fabs(tree.cost));
            const BranchBoundSolver::NodePotentialUpdateResult update =
                solver.updateNodePotentialBound(
                    node, -std::numeric_limits<double>::infinity(),
                    upper_bound, 1);
            expect(update.feasible,
                   "Prim potential evaluator rejected a feasible forced state");
            expect(std::fabs(update.bound - tree.cost) <= 1e-8,
                   "Prim potential bound differs from constrained Kruskal");
        }

        void buildRootAlphaNearness()
        {
            expect(tree.feasible,
                   "cannot build root alpha from an infeasible 1-tree");
            solver.buildRootAlphaNearness(tree);
        }

        double rootAlpha(int u, int v) const
        {
            return solver.root_alpha_by_edge_id_[solver.edgeId(u, v)];
        }

        double currentForbidDelta(int u, int v)
        {
            solver.candidates_sorted_ = candidates;
            const std::size_t state_size =
                static_cast<std::size_t>(solver.n_) * solver.n_;
            solver.edge_rank_by_id_.assign(state_size, -1);
            solver.candidate_word_count_ =
                (solver.candidates_sorted_.size() + 63) / 64;
            solver.resetCandidateBits(
                node, solver.candidates_sorted_.size());
            solver.available_degree_.assign(
                static_cast<std::size_t>(solver.n_), 0);
            solver.insufficient_degree_count_ = 0;
            for (std::size_t index = 0;
                 index < solver.candidates_sorted_.size(); ++index) {
                const Edge& candidate = solver.candidates_sorted_[index];
                solver.edge_rank_by_id_[solver.edgeId(
                    candidate.u, candidate.v)] = static_cast<int>(index);
                ++solver.available_degree_[
                    static_cast<std::size_t>(candidate.u)];
                ++solver.available_degree_[
                    static_cast<std::size_t>(candidate.v)];
            }
            const std::size_t id = solver.edgeId(u, v);
            const auto it = std::find_if(
                tree.edges.begin(), tree.edges.end(),
                [&](const Edge& edge) {
                    return solver.edgeId(edge.u, edge.v) == id;
                });
            expect(it != tree.edges.end(),
                   "forbid-delta test edge is not in the current 1-tree");
            return solver.currentForbidReplacementDelta(node, tree, *it);
        }

        std::pair<int, int> firstBranchEdge(
            BranchEdgeOrder order,
            const std::vector<std::pair<int, int>>& tree_edges,
            const std::vector<AlphaEntry>& alpha_entries)
        {
            tree = OneTree{};
            tree.feasible = true;
            tree.degree.assign(static_cast<std::size_t>(solver.n_), 0);
            for (const auto& endpoints : tree_edges) {
                const Edge selected = edge(endpoints.first, endpoints.second);
                tree.edges.push_back(selected);
                tree.cost += selected.w;
                ++tree.degree[static_cast<std::size_t>(selected.u)];
                ++tree.degree[static_cast<std::size_t>(selected.v)];
            }
            expect(tree.edges.size() == static_cast<std::size_t>(solver.n_),
                   "branch-order fixture must contain exactly n 1-tree edges");

            const std::size_t state_size =
                static_cast<std::size_t>(solver.n_) * solver.n_;
            tree.edge_index_in_tree.assign(state_size, -1);
            for (std::size_t index = 0; index < tree.edges.size(); ++index) {
                const Edge& selected = tree.edges[index];
                tree.edge_index_in_tree[solver.edgeId(
                    selected.u, selected.v)] = static_cast<int>(index);
            }
            solver.initializeDynamicMst(tree);

            solver.candidates_sorted_ = candidates;
            solver.edge_rank_by_id_.assign(state_size, -1);
            solver.candidate_word_count_ =
                (solver.candidates_sorted_.size() + 63) / 64;
            solver.resetCandidateBits(node, solver.candidates_sorted_.size());
            solver.available_degree_.assign(
                static_cast<std::size_t>(solver.n_), 0);
            solver.insufficient_degree_count_ = 0;
            for (std::size_t index = 0;
                 index < solver.candidates_sorted_.size(); ++index) {
                const Edge& candidate = solver.candidates_sorted_[index];
                solver.edge_rank_by_id_[solver.edgeId(
                    candidate.u, candidate.v)] = static_cast<int>(index);
                ++solver.available_degree_[
                    static_cast<std::size_t>(candidate.u)];
                ++solver.available_degree_[
                    static_cast<std::size_t>(candidate.v)];
            }

            solver.root_alpha_by_edge_id_.assign(
                state_size, std::numeric_limits<double>::infinity());
            for (const AlphaEntry& entry : alpha_entries) {
                solver.root_alpha_by_edge_id_[solver.edgeId(
                    entry.u, entry.v)] = entry.value;
            }
            solver.branch_edge_order_ = order;
            solver.best_cost_ = std::numeric_limits<double>::infinity();

            const BranchBoundSolver::BranchSet branches =
                solver.bpPartition(node, tree);
            expect(!branches.empty(),
                   "branch-order fixture produced no branch edge");
            int u = branches.front().edge.u;
            int v = branches.front().edge.v;
            if (u > v) std::swap(u, v);
            return {u, v};
        }

        void validate(const OneTree& value) const
        {
            expect(value.feasible, "cannot validate infeasible 1-tree");
            expect(static_cast<int>(value.edges.size()) == solver.n_,
                   "1-tree must contain n edges");

            std::vector<int> parent(static_cast<std::size_t>(solver.n_));
            std::iota(parent.begin(), parent.end(), 0);
            auto find = [&](int vertex) {
                int root = vertex;
                while (parent[static_cast<std::size_t>(root)] != root) {
                    root = parent[static_cast<std::size_t>(root)];
                }
                return root;
            };

            int root_edges = 0;
            int mst_edges = 0;
            double cost = 0.0;
            std::vector<int> degree(static_cast<std::size_t>(solver.n_), 0);
            std::vector<unsigned char> seen(
                static_cast<std::size_t>(solver.n_) * solver.n_, 0);
            for (const Edge& current : value.edges) {
                const std::size_t id = solver.edgeId(current.u, current.v);
                expect(!seen[id], "1-tree contains a duplicate edge");
                expect(!node.forbidden[id], "1-tree contains a forbidden edge");
                seen[id] = 1;
                cost += current.w;
                ++degree[static_cast<std::size_t>(current.u)];
                ++degree[static_cast<std::size_t>(current.v)];

                if (current.u == 0 || current.v == 0) {
                    ++root_edges;
                    continue;
                }
                ++mst_edges;
                int root_u = find(current.u);
                int root_v = find(current.v);
                expect(root_u != root_v, "non-root part contains a cycle");
                parent[static_cast<std::size_t>(root_v)] = root_u;
            }

            expect(root_edges == 2, "1-tree must contain two root edges");
            expect(mst_edges == solver.n_ - 2, "non-root part has the wrong edge count");
            const int connected_root = find(1);
            for (int v = 2; v < solver.n_; ++v) {
                expect(find(v) == connected_root, "non-root part is disconnected");
            }
            expect(degree == value.degree, "cached degrees do not match edges");
            expect(std::fabs(cost - value.cost) <= 1e-9,
                   "cached cost does not match edges");
            for (const Edge& forced_edge : node.forced_edges) {
                expect(seen[solver.edgeId(forced_edge.u, forced_edge.v)],
                       "1-tree omitted a forced edge");
            }
        }

        int findForced(int vertex) const
        {
            while (node.forced_parent[static_cast<std::size_t>(vertex)] != vertex) {
                vertex = node.forced_parent[static_cast<std::size_t>(vertex)];
            }
            return vertex;
        }

        static void expect(bool condition, const std::string& message)
        {
            if (!condition) throw std::runtime_error(message);
        }
    };

    static bool usesExactIntegerCosts(std::vector<std::vector<double>> matrix)
    {
        BranchBoundSolver solver(std::move(matrix));
        return solver.exact_integer_costs_;
    }

    struct RootFrequencyStats {
        SolveResult result;
        std::uint32_t sample_count = 0;
        std::uint64_t total_selected_edge_count = 0;
    };

    static RootFrequencyStats solveWithRootFrequency(
        std::vector<std::vector<double>> matrix)
    {
        BranchBoundSolver solver(std::move(matrix));
        solver.setRootAscentStrategy(RootAscentStrategy::Polyak);
        solver.setBranchEdgeOrder(
            BranchEdgeOrder::RootOneTreeFrequencyMiddle);

        RootFrequencyStats stats;
        stats.result = solver.solve();
        stats.sample_count = solver.root_one_tree_sample_count_;
        stats.total_selected_edge_count = std::accumulate(
            solver.root_one_tree_edge_counts_.begin(),
            solver.root_one_tree_edge_counts_.end(), std::uint64_t{0});
        return stats;
    }

    struct DiversifiedTourStats {
        double initial_cost = std::numeric_limits<double>::infinity();
        double diversified_cost = std::numeric_limits<double>::infinity();
        std::size_t alternative_count = 0;
        bool improved = false;
    };

    static DiversifiedTourStats diversifiedTourStats(
        std::vector<std::vector<double>> matrix)
    {
        BranchBoundSolver solver(std::move(matrix));
        std::vector<int> tour;
        std::vector<BranchBoundSolver::TourCandidate> alternatives;
        double cost = std::numeric_limits<double>::infinity();
        if (!solver.findInitialTour(tour, cost, alternatives)) {
            return {};
        }

        DiversifiedTourStats stats;
        stats.initial_cost = solver.tourCost(tour);
        stats.alternative_count = alternatives.size();
        stats.improved = solver.improveInitialTourDiversified(
            alternatives, -std::numeric_limits<double>::infinity(),
            tour, cost);
        stats.diversified_cost = solver.tourCost(tour);
        return stats;
    }
};

} // namespace tsp

namespace {

using Fixture = tsp::BranchBoundSolverTestAccess::Fixture;

constexpr double inf = std::numeric_limits<double>::infinity();

std::vector<std::vector<double>> replacementMatrix()
{
    return {
        {0, 1, 2, 6, 7},
        {1, 0, 1, 4, 10},
        {2, 1, 0, 2, 5},
        {6, 4, 2, 0, 3},
        {7, 10, 5, 3, 0},
    };
}

void expectCost(double actual, double expected, const std::string& message)
{
    if (!std::isfinite(actual) || !std::isfinite(expected)) {
        if (actual == expected) return;
        throw std::runtime_error(message);
    }
    const double scale = std::max(std::fabs(actual), std::fabs(expected));
    const double next = std::nextafter(scale, inf);
    const double ulp = std::isfinite(next)
        ? next - scale
        : scale - std::nextafter(scale, 0.0);
    const double tolerance = 64.0 * std::max(
        std::numeric_limits<double>::epsilon() * scale, ulp);
    if (std::fabs(actual - expected) > tolerance) {
        throw std::runtime_error(message);
    }
}

void expectPotentialUpdateDecisionAccounting(
    const tsp::SolveStats& stats, const std::string& context)
{
    // 所有未触发计数均为互斥的“首个阻止原因”。该恒等式既能防止新增判断
    // 时漏记原因，也能防止同一候选节点被多个原因重复累计。
    const std::size_t skipped =
        stats.search_node_potential_updates_skipped_strategy_none
        + stats.search_node_potential_updates_skipped_update_depth_zero
        + stats.search_node_potential_updates_skipped_budget_exhausted
        + stats.search_node_potential_updates_skipped_numerically_unsafe
        + stats.search_node_potential_updates_skipped_invalid_state
        + stats.search_node_potential_updates_skipped_zero_violation
        + stats.search_node_potential_updates_skipped_zero_iteration_limit
        + stats.search_node_potential_updates_skipped_depth_interval
        + stats.search_node_potential_updates_skipped_gap_below_minimum
        + stats.search_node_potential_updates_skipped_gap_above_maximum;
    if (stats.search_node_potential_update_candidates
        != stats.search_node_potential_updates_triggered + skipped) {
        throw std::runtime_error(
            context + ": potential-update decision counters do not add up");
    }
}

double bruteForceOptimalCost(const std::vector<std::vector<double>>& matrix)
{
    const int n = static_cast<int>(matrix.size());
    std::vector<int> order(static_cast<std::size_t>(n - 1));
    std::iota(order.begin(), order.end(), 1);

    double best = inf;
    do {
        double cost = 0.0;
        int previous = 0;
        bool feasible = true;
        for (const int vertex : order) {
            const double weight = matrix[static_cast<std::size_t>(previous)]
                                        [static_cast<std::size_t>(vertex)];
            if (!std::isfinite(weight)) {
                feasible = false;
                break;
            }
            cost += weight;
            previous = vertex;
        }
        const double closing_weight = matrix[static_cast<std::size_t>(previous)][0];
        if (feasible && std::isfinite(closing_weight)) {
            best = std::min(best, cost + closing_weight);
        }
    } while (std::next_permutation(order.begin(), order.end()));
    return best;
}

void testRootAlphaNearness()
{
    const auto matrix = replacementMatrix();
    Fixture fixture(matrix);
    fixture.buildRootAlphaNearness();

    Fixture::expect(fixture.contains(fixture.tree, 1, 2),
                    "root alpha fixture omitted MST edge 1-2");
    Fixture::expect(fixture.contains(fixture.tree, 2, 3),
                    "root alpha fixture omitted MST edge 2-3");
    Fixture::expect(fixture.contains(fixture.tree, 3, 4),
                    "root alpha fixture omitted MST edge 3-4");
    expectCost(fixture.rootAlpha(1, 3), 2.0,
               "internal non-tree alpha used the wrong path maximum");
    expectCost(fixture.rootAlpha(1, 4), 7.0,
               "long internal non-tree alpha is incorrect");
    expectCost(fixture.rootAlpha(2, 4), 2.0,
               "internal non-tree replacement alpha is incorrect");
    expectCost(fixture.rootAlpha(0, 3), 4.0,
               "non-tree root-edge alpha is incorrect");
    expectCost(fixture.rootAlpha(0, 4), 5.0,
               "second non-tree root-edge alpha is incorrect");
    const double optimum = bruteForceOptimalCost(matrix);
    for (const tsp::BranchEdgeOrder order : {
             tsp::BranchEdgeOrder::RootAlphaAscending,
             tsp::BranchEdgeOrder::RootAlphaDescending,
             tsp::BranchEdgeOrder::RootAlphaGlobalAscending,
             tsp::BranchEdgeOrder::RootAlphaGlobalDescending,
             tsp::BranchEdgeOrder::AdjustedWeightDescending,
             tsp::BranchEdgeOrder::MaximumDegreeAllAdjustedWeight,
             tsp::BranchEdgeOrder::MaximumExcessCoverAdjustedWeight,
             tsp::BranchEdgeOrder::LocalExcessCoverAdjustedWeight,
             tsp::BranchEdgeOrder::MaximumDegreeExcessCoverAdjustedWeight,
             tsp::BranchEdgeOrder::PropagationPotentialAdjustedWeight,
             tsp::BranchEdgeOrder::ForcedDegreeAdjustedWeight,
             tsp::BranchEdgeOrder::MaximumDegreeMinimumUndecided,
             tsp::BranchEdgeOrder::MaximumDegreeMaximumUndecided,
             tsp::BranchEdgeOrder::CurrentForbidDeltaDescending,
             tsp::BranchEdgeOrder::CurrentForbidDeltaAscending,
             tsp::BranchEdgeOrder::CurrentForbidDeltaDegreeAware,
             tsp::BranchEdgeOrder::RootOneTreeFrequencyMiddle,
             tsp::BranchEdgeOrder::TwoSidedStrongBranchingTop2}) {
        tsp::BranchBoundSolver solver(matrix);
        solver.setRootAscentStrategy(
            order == tsp::BranchEdgeOrder::RootOneTreeFrequencyMiddle
            ? tsp::RootAscentStrategy::Polyak
            : tsp::RootAscentStrategy::None);
        solver.setBranchEdgeOrder(order);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, optimum,
                   "experimental branch order changed the exact optimum");
    }
}

void testRootAlphaGlobalPriority()
{
    const std::vector<std::vector<double>> matrix = {
        {0, 1, 2, 20, 21, 22},
        {1, 0, 3, 4, 5, 23},
        {2, 3, 0, 24, 25, 6},
        {20, 4, 24, 0, 26, 27},
        {21, 5, 25, 26, 0, 28},
        {22, 23, 6, 27, 28, 0},
    };
    // 这是合法 1-tree：顶点 1 的 excess=2，顶点 2 的 excess=1。
    // 边 (2,5) 只接触较低违规热点，但 alpha 最小，global-asc 必须选它；
    // (0,1) 的 alpha 最大，global-desc 必须选它。若 alpha 全平，则覆盖
    // excess=3 的 (1,2) 应当胜出。
    const std::vector<std::pair<int, int>> tree_edges = {
        {0, 1}, {0, 2}, {1, 2}, {1, 3}, {1, 4}, {2, 5},
    };
    const std::vector<Fixture::AlphaEntry> ordered_alpha = {
        {0, 1, 5.0}, {0, 2, 4.0}, {1, 2, 3.0},
        {1, 3, 2.0}, {1, 4, 1.0}, {2, 5, 0.0},
    };

    Fixture ascending_fixture(matrix);
    Fixture::expect(
        ascending_fixture.firstBranchEdge(
            tsp::BranchEdgeOrder::RootAlphaGlobalAscending,
            tree_edges, ordered_alpha) == std::pair<int, int>{2, 5},
        "global ascending alpha did not override degree priority");

    Fixture descending_fixture(matrix);
    Fixture::expect(
        descending_fixture.firstBranchEdge(
            tsp::BranchEdgeOrder::RootAlphaGlobalDescending,
            tree_edges, ordered_alpha) == std::pair<int, int>{0, 1},
        "global descending alpha did not override degree priority");

    std::vector<Fixture::AlphaEntry> tied_alpha = ordered_alpha;
    for (Fixture::AlphaEntry& entry : tied_alpha) entry.value = 0.0;
    Fixture tied_fixture(matrix);
    Fixture::expect(
        tied_fixture.firstBranchEdge(
            tsp::BranchEdgeOrder::RootAlphaGlobalAscending,
            tree_edges, tied_alpha) == std::pair<int, int>{1, 2},
        "global alpha tie did not prefer larger excess coverage");
}

void testRootOneTreeFrequencyCollection()
{
    const auto matrix = replacementMatrix();
    const auto stats =
        tsp::BranchBoundSolverTestAccess::solveWithRootFrequency(matrix);
    expectCost(stats.result.cost, bruteForceOptimalCost(matrix),
               "root frequency branching changed the exact optimum");
    Fixture::expect(stats.sample_count > 0,
                    "root frequency branching collected no ascent samples");
    Fixture::expect(
        stats.total_selected_edge_count
            == static_cast<std::uint64_t>(stats.sample_count) * matrix.size(),
        "each feasible root ascent sample must contribute exactly n edges");
}

void testCurrentForbidReplacementDelta()
{
    Fixture fixture(replacementMatrix());
    expectCost(fixture.currentForbidDelta(1, 2), 3.0,
               "current forbid delta used the wrong cut for edge 1-2");
    expectCost(fixture.currentForbidDelta(2, 3), 2.0,
               "current forbid delta used the wrong cut for edge 2-3");
    expectCost(fixture.currentForbidDelta(3, 4), 2.0,
               "current forbid delta used the wrong cut for edge 3-4");
    expectCost(fixture.currentForbidDelta(0, 1), 5.0,
               "current forbid delta selected the wrong root replacement");
    expectCost(fixture.currentForbidDelta(0, 2), 4.0,
               "current forbid delta reused the removed root edge");
}

tsp::SolveResult solveAndCompareWithBruteForce(
    const std::vector<std::vector<double>>& matrix,
    const std::string& case_name)
{
    const double expected = bruteForceOptimalCost(matrix);
    tsp::BranchBoundSolver solver(matrix);
    const tsp::SolveResult result = solver.solve();

    if (!std::isfinite(expected)) {
        if (result.feasible || std::isfinite(result.cost)) {
            throw std::runtime_error(case_name + ": solver accepted an infeasible instance");
        }
        return result;
    }
    if (!result.feasible) {
        throw std::runtime_error(case_name + ": solver rejected a feasible instance");
    }
    expectCost(result.cost, expected,
               case_name + ": optimal cost differs from brute force");

    const int n = static_cast<int>(matrix.size());
    if (static_cast<int>(result.tour.size()) != n) {
        throw std::runtime_error(case_name + ": returned tour has the wrong size");
    }
    std::vector<unsigned char> seen(static_cast<std::size_t>(n), 0);
    double returned_cost = 0.0;
    for (int index = 0; index < n; ++index) {
        const int u = result.tour[static_cast<std::size_t>(index)];
        const int v = result.tour[static_cast<std::size_t>((index + 1) % n)];
        if (u < 0 || u >= n || seen[static_cast<std::size_t>(u)]) {
            throw std::runtime_error(case_name + ": returned tour repeats an invalid vertex");
        }
        seen[static_cast<std::size_t>(u)] = 1;
        const double weight = matrix[static_cast<std::size_t>(u)]
                                    [static_cast<std::size_t>(v)];
        if (!std::isfinite(weight)) {
            throw std::runtime_error(case_name + ": returned tour uses a missing edge");
        }
        returned_cost += weight;
    }
    expectCost(returned_cost, result.cost,
               case_name + ": result cost is not the returned tour's real cost");
    return result;
}

void testInternalReplacement()
{
    Fixture fixture(replacementMatrix());
    expectCost(fixture.tree.cost, 9.0, "unexpected initial 1-tree cost");
    fixture.forbidAndCompare(2, 3);
    expectCost(fixture.tree.cost, 11.0, "wrong internal replacement cost");
    if (!fixture.contains(fixture.tree, 1, 3)) {
        throw std::runtime_error("internal replacement edge was not selected");
    }
}

void testRootReplacement()
{
    Fixture fixture(replacementMatrix());
    fixture.forbidAndCompare(0, 1);
    expectCost(fixture.tree.cost, 14.0, "wrong root replacement cost");
    if (!fixture.contains(fixture.tree, 0, 3)) {
        throw std::runtime_error("third-lightest root edge was not selected");
    }
}

void testNonTreeForbid()
{
    Fixture fixture(replacementMatrix());
    fixture.forbidAndCompare(1, 4);
    expectCost(fixture.tree.cost, 9.0, "forbidding a non-tree edge changed the cost");
}

void testSequentialForbids()
{
    Fixture fixture(replacementMatrix());
    fixture.forbidAndCompare(2, 3);
    fixture.forbidAndCompare(1, 3);
    expectCost(fixture.tree.cost, 12.0, "wrong sequential replacement cost");
    if (!fixture.contains(fixture.tree, 2, 4)) {
        throw std::runtime_error("second replacement edge was not selected");
    }
}

void testMissingInternalReplacement()
{
    auto matrix = replacementMatrix();
    matrix[1][3] = matrix[3][1] = inf;
    matrix[1][4] = matrix[4][1] = inf;
    matrix[2][4] = matrix[4][2] = inf;
    Fixture fixture(std::move(matrix));
    fixture.forbidAndCompare(2, 3);
    if (fixture.tree.feasible) {
        throw std::runtime_error("missing internal replacement should be infeasible");
    }
}

void testMissingRootReplacement()
{
    auto matrix = replacementMatrix();
    matrix[0][3] = matrix[3][0] = inf;
    matrix[0][4] = matrix[4][0] = inf;
    Fixture fixture(std::move(matrix));
    fixture.forbidAndCompare(0, 1);
    if (fixture.tree.feasible) {
        throw std::runtime_error("missing root replacement should be infeasible");
    }
}

void testTiedReplacement()
{
    auto matrix = replacementMatrix();
    matrix[2][4] = matrix[4][2] = 4;
    Fixture fixture(std::move(matrix));
    fixture.forbidAndCompare(2, 3, false);
    expectCost(fixture.tree.cost, 11.0, "wrong tied replacement cost");
}

void testMixedRootAndInternalForcedEdges()
{
    Fixture fixture(replacementMatrix());
    fixture.force(1, 2);
    fixture.force(0, 1);
    fixture.rebuildAfterForces();
    if (!fixture.contains(fixture.tree, 1, 2)
        || !fixture.contains(fixture.tree, 0, 1)) {
        throw std::runtime_error("forced edges are missing from rebuilt 1-tree");
    }
    fixture.verifyNodePotentialPrimBound();
    fixture.forbidAndCompare(2, 3);
}

void testOptionalRootReplacementWithForcedRootEdge()
{
    Fixture fixture(replacementMatrix());
    fixture.force(0, 1);
    fixture.rebuildAfterForces();
    if (!fixture.contains(fixture.tree, 0, 2)) {
        throw std::runtime_error("expected optional root edge is missing");
    }
    fixture.forbidAndCompare(0, 2);
    if (!fixture.contains(fixture.tree, 0, 1)
        || !fixture.contains(fixture.tree, 0, 3)) {
        throw std::runtime_error("root replacement did not preserve the forced root edge");
    }
}

void testMultipleForcedInternalEdges()
{
    Fixture fixture(replacementMatrix());
    fixture.force(1, 2);
    fixture.force(2, 3);
    fixture.force(0, 1);
    fixture.rebuildAfterForces();
    fixture.verifyNodePotentialPrimBound();
    fixture.forbidAndCompare(3, 4);
    if (fixture.tree.feasible) {
        throw std::runtime_error("degree-filtered forced state should have no replacement");
    }
}

void testRandomSequentialForbids()
{
    std::mt19937 generator(20260713);
    std::uniform_int_distribution<int> base_weight(1, 100000);

    for (int case_index = 0; case_index < 100; ++case_index) {
        const int n = 4 + case_index % 5;
        std::vector<std::vector<double>> matrix(
            static_cast<std::size_t>(n),
            std::vector<double>(static_cast<std::size_t>(n), 0.0));
        int edge_index = 0;
        for (int u = 0; u < n; ++u) {
            for (int v = u + 1; v < n; ++v) {
                const double weight = static_cast<double>(base_weight(generator))
                    + static_cast<double>(edge_index++) / 1000.0;
                matrix[static_cast<std::size_t>(u)][static_cast<std::size_t>(v)] = weight;
                matrix[static_cast<std::size_t>(v)][static_cast<std::size_t>(u)] = weight;
            }
        }

        Fixture fixture(std::move(matrix));
        for (int step = 0; step < n + 3; ++step) {
            if (!fixture.forbidLightestSelectedEdgeAndCompare()) break;
        }
    }
}

void testPackedMstComponentWordBoundaries()
{
    for (const int n : {64, 65, 128, 129}) {
        std::vector<std::vector<double>> matrix(
            static_cast<std::size_t>(n),
            std::vector<double>(static_cast<std::size_t>(n), 0.0));
        for (int u = 0; u < n; ++u) {
            for (int v = u + 1; v < n; ++v) {
                const double weight = static_cast<double>(
                    100000 + u * n + v);
                matrix[static_cast<std::size_t>(u)]
                      [static_cast<std::size_t>(v)] = weight;
                matrix[static_cast<std::size_t>(v)]
                      [static_cast<std::size_t>(u)] = weight;
            }
        }
        for (int v = 2; v < n; ++v) {
            const double weight = static_cast<double>(v);
            matrix[static_cast<std::size_t>(v - 1)]
                  [static_cast<std::size_t>(v)] = weight;
            matrix[static_cast<std::size_t>(v)]
                  [static_cast<std::size_t>(v - 1)] = weight;
        }
        matrix[0][1] = matrix[1][0] = 1.0;
        matrix[0][static_cast<std::size_t>(n - 1)] =
            matrix[static_cast<std::size_t>(n - 1)][0] = 2.0;

        Fixture fixture(std::move(matrix));
        const int left = n == 64 ? 62 : 63;
        const int right = left + 1;
        if (!fixture.contains(fixture.tree, left, right)) {
            throw std::runtime_error(
                "packed MST boundary fixture missed its path edge");
        }
        fixture.forbidAndCompare(left, right);
    }
}

void testRandomSparseTiedForbids()
{
    std::mt19937 generator(20260714);
    std::uniform_int_distribution<int> weight(1, 10);
    std::bernoulli_distribution include_extra(0.4);

    for (int case_index = 0; case_index < 50; ++case_index) {
        const int n = 4 + case_index % 5;
        std::vector<std::vector<double>> matrix(
            static_cast<std::size_t>(n),
            std::vector<double>(static_cast<std::size_t>(n), inf));
        for (int v = 0; v < n; ++v) {
            matrix[static_cast<std::size_t>(v)][static_cast<std::size_t>(v)] = 0.0;
        }

        auto add_edge = [&](int u, int v) {
            const double value = static_cast<double>(weight(generator));
            matrix[static_cast<std::size_t>(u)][static_cast<std::size_t>(v)] = value;
            matrix[static_cast<std::size_t>(v)][static_cast<std::size_t>(u)] = value;
        };
        for (int u = 0; u < n; ++u) {
            add_edge(u, (u + 1) % n);
        }
        for (int u = 0; u < n; ++u) {
            for (int v = u + 1; v < n; ++v) {
                if (!std::isfinite(matrix[static_cast<std::size_t>(u)][static_cast<std::size_t>(v)])
                    && include_extra(generator)) {
                    add_edge(u, v);
                }
            }
        }

        Fixture fixture(std::move(matrix));
        for (int step = 0; step < n + 3; ++step) {
            if (!fixture.forbidLightestSelectedEdgeAndCompare(false)) break;
        }
    }
}

void testBpPrefixTreeRegressions()
{
    // B[1+] 的 force 子节点必须继承已禁止此前 B 边后的 prefix tree。
    const std::vector<std::vector<double>> complete = {
        {0, 585, 792, 891, 348, 768},
        {585, 0, 249, 83, 294, 778},
        {792, 249, 0, 14, 340, 834},
        {891, 83, 14, 0, 530, 399},
        {348, 294, 340, 530, 0, 325},
        {768, 778, 834, 399, 325, 0},
    };
    const tsp::SolveResult complete_result = solveAndCompareWithBruteForce(
        complete, "six-node complete BP prefix regression");
    expectCost(complete_result.cost, 1920.0,
               "six-node complete BP prefix regression returned the wrong optimum");

    const std::vector<std::vector<double>> sparse = {
        {0, 585, inf, inf, 348, inf},
        {585, 0, 249, 83, 294, inf},
        {inf, 249, 0, 14, inf, inf},
        {inf, 83, 14, 0, inf, 399},
        {348, 294, inf, inf, 0, 325},
        {inf, inf, inf, 399, 325, 0},
    };
    const tsp::SolveResult sparse_result = solveAndCompareWithBruteForce(
        sparse, "six-node sparse BP prefix regression");
    expectCost(sparse_result.cost, 1920.0,
               "six-node sparse BP prefix regression returned the wrong optimum");

    // 旧的父树复用路径在该 5 点实例上返回 38，而真实最优值为 35。
    const std::vector<std::vector<double>> five_node = {
        {0, 7, 6, inf, 9},
        {7, 0, 6, inf, 6},
        {6, 6, 0, 7, 1},
        {inf, inf, 7, 0, 9},
        {9, 6, 1, 9, 0},
    };
    const tsp::SolveResult five_result = solveAndCompareWithBruteForce(
        five_node, "five-node forced-prefix regression");
    expectCost(five_result.cost, 35.0,
               "five-node forced-prefix regression returned the wrong optimum");
}

void testScaleSafeExactSearch()
{
    const std::vector<std::vector<double>> base = {
        {0, 585, 792, 891, 348, 768},
        {585, 0, 249, 83, 294, 778},
        {792, 249, 0, 14, 340, 834},
        {891, 83, 14, 0, 530, 399},
        {348, 294, 340, 530, 0, 325},
        {768, 778, 834, 399, 325, 0},
    };
    std::vector<std::vector<double>> scaled = base;
    for (std::size_t u = 0; u < scaled.size(); ++u) {
        for (std::size_t v = 0; v < scaled.size(); ++v) {
            if (u != v) scaled[u][v] *= 1e-12;
        }
    }

    const tsp::SolveResult result = solveAndCompareWithBruteForce(
        scaled, "1e-12 scaled exact-search regression");
    expectCost(result.cost, 1920.0e-12,
               "scaled exact search was incorrectly pruned by an absolute epsilon");
    if (result.stats.nodes_expanded == 0) {
        throw std::runtime_error(
            "scaled exact-search regression did not enter the branch-and-bound search");
    }

    // epsilon*scale 在 subnormal 区间会下溢为 0；比较余量必须至少为
    // 实际相邻 double 的一个 ULP，不能把 1 ULP 舍入差当成确定下界差。
    std::vector<std::vector<double>> subnormal = base;
    const double denorm = std::numeric_limits<double>::denorm_min();
    for (std::size_t u = 0; u < subnormal.size(); ++u) {
        for (std::size_t v = 0; v < subnormal.size(); ++v) {
            if (u != v) subnormal[u][v] *= denorm;
        }
    }
    const tsp::SolveResult subnormal_result = solveAndCompareWithBruteForce(
        subnormal, "subnormal exact-search regression");
    expectCost(subnormal_result.cost, 1920.0 * denorm,
               "subnormal exact search returned the wrong optimum");
    if (subnormal_result.cost == 0.0) {
        throw std::runtime_error("subnormal exact-search cost unexpectedly underflowed to zero");
    }
}

void testMixedMagnitudeForceRollback()
{
    // force 子节点回溯必须恢复 forced_mst_cost 的原值，不能用 +w/-w。
    // 小的祖先缓存与 1e100 sibling 边相加时会被舍入吞掉，减回也无法恢复。
    Fixture cache_fixture(replacementMatrix());
    cache_fixture.verifyForcedMstCacheRestore();

    const std::vector<std::vector<double>> matrix = {
        {0, 32, 27, 34, 6e100, 8e100},
        {32, 0, 4e100, 1e100, 6e100, 1e100},
        {27, 4e100, 0, 1e100, 9e100, 9e100},
        {34, 1e100, 1e100, 0, 4e100, 14},
        {6e100, 6e100, 9e100, 4e100, 0, 7e100},
        {8e100, 1e100, 9e100, 14, 7e100, 0},
    };
    const tsp::SolveResult result = solveAndCompareWithBruteForce(
        matrix, "mixed-magnitude rollback pressure");
    if (result.stats.nodes_created <= 1) {
        throw std::runtime_error(
            "mixed-magnitude rollback tests did not exercise recursive BP search");
    }

    tsp::BranchBoundSolver update_solver(matrix);
    update_solver.setRootAscentStrategy(tsp::RootAscentStrategy::None);
    update_solver.setPotentialUpdateOptions(
        tsp::PotentialUpdateStrategy::SubtreeAdaptive, 1, 16, 1.0, 100);
    const tsp::SolveResult update_result = update_solver.solve();
    expectCost(update_result.cost, result.cost,
               "mixed-magnitude potential safety changed the optimum");
    expectPotentialUpdateDecisionAccounting(
        update_result.stats, "mixed-magnitude potential safety");
    if (update_result.stats.search_node_potential_updates_triggered != 0) {
        throw std::runtime_error(
            "mixed-magnitude instance did not disable node potential updates");
    }
}

void testRandomCompleteSolveAgainstBruteForce()
{
    std::mt19937 generator(2026071501);
    std::uniform_int_distribution<int> weight(1, 50);
    bool exercised_recursive_search = false;

    for (int case_index = 0; case_index < 16; ++case_index) {
        const int n = 5 + case_index % 4;
        std::vector<std::vector<double>> matrix(
            static_cast<std::size_t>(n),
            std::vector<double>(static_cast<std::size_t>(n), 0.0));
        for (int u = 0; u < n; ++u) {
            for (int v = u + 1; v < n; ++v) {
                const double value = static_cast<double>(weight(generator));
                matrix[static_cast<std::size_t>(u)][static_cast<std::size_t>(v)] = value;
                matrix[static_cast<std::size_t>(v)][static_cast<std::size_t>(u)] = value;
            }
        }
        const tsp::SolveResult result = solveAndCompareWithBruteForce(
            matrix, "complete random solve " + std::to_string(case_index));
        exercised_recursive_search = exercised_recursive_search
            || result.stats.nodes_created > 1;
        if (case_index < 8) {
            tsp::BranchBoundSolver update_solver(matrix);
            update_solver.setRootAscentStrategy(tsp::RootAscentStrategy::None);
            update_solver.setPotentialUpdateOptions(
                case_index % 2 == 0
                    ? tsp::PotentialUpdateStrategy::SubtreeDepth
                    : tsp::PotentialUpdateStrategy::SubtreeAdaptive,
                1, 16, 1.0, 100);
            const tsp::SolveResult update_result = update_solver.solve();
            expectCost(
                update_result.cost, bruteForceOptimalCost(matrix),
                "complete random node-potential update differs from brute force");
        }
    }
    if (!exercised_recursive_search) {
        throw std::runtime_error("complete solve tests did not exercise recursive BP search");
    }
}

void testRandomSparseSolveAgainstBruteForce()
{
    std::mt19937 generator(2026071502);
    std::uniform_int_distribution<int> weight(1, 50);
    std::bernoulli_distribution include_chord(0.35);
    bool exercised_recursive_search = false;

    for (int case_index = 0; case_index < 20; ++case_index) {
        const int n = 5 + case_index % 4;
        std::vector<std::vector<double>> matrix(
            static_cast<std::size_t>(n),
            std::vector<double>(static_cast<std::size_t>(n), inf));
        for (int vertex = 0; vertex < n; ++vertex) {
            matrix[static_cast<std::size_t>(vertex)]
                  [static_cast<std::size_t>(vertex)] = 0.0;
        }
        auto add_edge = [&](int u, int v) {
            const double value = static_cast<double>(weight(generator));
            matrix[static_cast<std::size_t>(u)][static_cast<std::size_t>(v)] = value;
            matrix[static_cast<std::size_t>(v)][static_cast<std::size_t>(u)] = value;
        };
        for (int u = 0; u < n; ++u) {
            add_edge(u, (u + 1) % n);
        }
        for (int u = 0; u < n; ++u) {
            for (int v = u + 1; v < n; ++v) {
                if (!std::isfinite(matrix[static_cast<std::size_t>(u)]
                                         [static_cast<std::size_t>(v)])
                    && include_chord(generator)) {
                    add_edge(u, v);
                }
            }
        }
        const tsp::SolveResult result = solveAndCompareWithBruteForce(
            matrix, "sparse random solve " + std::to_string(case_index));
        exercised_recursive_search = exercised_recursive_search
            || result.stats.nodes_created > 1;
        if (case_index < 8) {
            tsp::BranchBoundSolver update_solver(matrix);
            update_solver.setRootAscentStrategy(tsp::RootAscentStrategy::None);
            update_solver.setPotentialUpdateOptions(
                case_index % 2 == 0
                    ? tsp::PotentialUpdateStrategy::SubtreeDepth
                    : tsp::PotentialUpdateStrategy::SubtreeAdaptive,
                1, 16, 1.0, 100);
            const tsp::SolveResult update_result = update_solver.solve();
            expectCost(
                update_result.cost, bruteForceOptimalCost(matrix),
                "sparse random node-potential update differs from brute force");
        }
    }
    if (!exercised_recursive_search) {
        throw std::runtime_error("sparse solve tests did not exercise recursive BP search");
    }
}

void testProblemParsingDoesNotWriteStdout()
{
    std::ostringstream captured;
    std::streambuf* original = std::cout.rdbuf(captured.rdbuf());
    try {
        std::istringstream plain(
            "3\n"
            "0 1 2\n"
            "1 0 3\n"
            "2 3 0\n");
        const tsp::TspProblem plain_problem = tsp::readTspProblem(plain);
        if (plain_problem.dimension() != 3 || !plain_problem.hasDenseMatrix()) {
            throw std::runtime_error("plain matrix parser returned the wrong problem");
        }

        std::istringstream tsplib(
            "NAME: tiny\n"
            "TYPE: TSP\n"
            "DIMENSION: 3\n"
            "EDGE_WEIGHT_TYPE: EXPLICIT\n"
            "EDGE_WEIGHT_FORMAT: FULL_MATRIX\n"
            "EDGE_WEIGHT_SECTION\n"
            "0 1 2\n"
            "1 0 3\n"
            "2 3 0\n"
            "EOF\n");
        const tsp::TspProblem tsplib_problem = tsp::readTspProblem(tsplib);
        if (tsplib_problem.dimension() != 3 || !tsplib_problem.hasDenseMatrix()) {
            throw std::runtime_error("TSPLIB parser returned the wrong problem");
        }
    } catch (...) {
        std::cout.rdbuf(original);
        throw;
    }
    std::cout.rdbuf(original);
    if (!captured.str().empty()) {
        throw std::runtime_error("problem parsing unexpectedly wrote debug text to stdout");
    }
}

void testDistanceMatrixSymmetryIsExact()
{
    auto expect_rejected = [](std::vector<std::vector<double>> matrix,
                              const std::string& message) {
        try {
            tsp::BranchBoundSolver solver(std::move(matrix));
            (void)solver;
        } catch (const std::runtime_error&) {
            return;
        }
        throw std::runtime_error(message);
    };

    auto asymmetric = replacementMatrix();
    asymmetric[0][1] = std::nextafter(asymmetric[1][0], inf);
    expect_rejected(std::move(asymmetric),
                    "matrix differing by one ULP was accepted as symmetric");
}

void testExactIntegerPruningDomain()
{
    if (!tsp::BranchBoundSolverTestAccess::usesExactIntegerCosts(
            replacementMatrix())) {
        throw std::runtime_error("small integer matrix missed exact-cost pruning domain");
    }

    // 3*w = 2^53+1 mathematically, but on platforms where long double is just
    // binary64 the multiplication rounds to 2^53. Integer division must reject it.
    const double boundary_weight = 3002399751580331.0;
    const std::vector<std::vector<double>> boundary = {
        {0, boundary_weight, boundary_weight},
        {boundary_weight, 0, boundary_weight},
        {boundary_weight, boundary_weight, 0},
    };
    if (tsp::BranchBoundSolverTestAccess::usesExactIntegerCosts(boundary)) {
        throw std::runtime_error("unsafe 2^53 integer-sum boundary enabled equality pruning");
    }

    auto fractional = replacementMatrix();
    fractional[0][1] = fractional[1][0] = 1.5;
    if (tsp::BranchBoundSolverTestAccess::usesExactIntegerCosts(fractional)) {
        throw std::runtime_error("fractional matrix enabled exact-integer pruning");
    }
}

void testDiversifiedInitialTourPool()
{
    std::ifstream input(
        std::string(TSP_TEST_SOURCE_DIR)
        + "/data/classic/tsplib/st70.tsp");
    if (!input) {
        throw std::runtime_error("cannot open st70 diversified-tour regression");
    }
    const tsp::TspProblem problem = tsp::readTspProblem(input);
    const auto stats = tsp::BranchBoundSolverTestAccess::diversifiedTourStats(
        problem.toDenseMatrix(70));

    expectCost(stats.initial_cost, 685.0,
               "st70 single-start LK upper bound changed unexpectedly");
    expectCost(stats.diversified_cost, 682.0,
               "st70 diversified LK did not reproduce the tighter upper bound");
    if (!stats.improved || stats.alternative_count < 12) {
        throw std::runtime_error(
            "st70 diversified-tour pool was not populated or used");
    }
}

void testRootAscentStrategies()
{
    const auto matrix = replacementMatrix();
    const double optimum = bruteForceOptimalCost(matrix);
    auto root_bound = [&](tsp::RootAscentStrategy strategy) {
        tsp::BranchBoundSolver solver(matrix);
        solver.setRootAscentStrategy(strategy);
        solver.setRootBoundOnly(true);
        const tsp::SolveResult result = solver.solve();
        if (!result.feasible) {
            throw std::runtime_error(
                "root-bound-only strategy lost the feasible incumbent");
        }
        if (!std::isfinite(result.stats.root_lower_bound)
            || result.stats.root_lower_bound > optimum + 1e-8) {
            throw std::runtime_error(
                "root ascent produced an invalid TSP lower bound");
        }
        const bool should_run = strategy != tsp::RootAscentStrategy::None;
        if ((result.stats.root_potential_iterations != 0) != should_run) {
            throw std::runtime_error(
                "root potential iteration statistics do not match the strategy");
        }
        return result.stats.root_lower_bound;
    };

    const double none = root_bound(tsp::RootAscentStrategy::None);
    const double polyak = root_bound(tsp::RootAscentStrategy::Polyak);
    const double helsgaun = root_bound(tsp::RootAscentStrategy::Helsgaun);
    const double hybrid = root_bound(tsp::RootAscentStrategy::Hybrid);
    if (polyak + 1e-8 < none || helsgaun + 1e-8 < none
        || hybrid + 1e-8 < polyak) {
        throw std::runtime_error(
            "an ascent strategy discarded its zero/warm-start lower bound");
    }

    for (const tsp::RootAscentStrategy strategy : {
             tsp::RootAscentStrategy::Helsgaun,
             tsp::RootAscentStrategy::Hybrid}) {
        tsp::BranchBoundSolver solver(matrix);
        solver.setRootAscentStrategy(strategy);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, optimum,
                   "experimental root ascent changed the exact optimum");
    }
}

void testSearchNodePotentialUpdates()
{
    std::ifstream input(
        std::string(TSP_TEST_SOURCE_DIR)
        + "/data/classic/tsplib/dantzig42.tsp");
    if (!input) {
        throw std::runtime_error(
            "cannot open dantzig42 node-potential-update regression");
    }
    const tsp::TspProblem problem = tsp::readTspProblem(input);
    const auto matrix = problem.toDenseMatrix(42);

    for (const tsp::PotentialUpdateStrategy strategy : {
             tsp::PotentialUpdateStrategy::SubtreeDepth,
             tsp::PotentialUpdateStrategy::SubtreeAdaptive}) {
        tsp::BranchBoundSolver solver(matrix);
        solver.setPotentialUpdateOptions(strategy, 1, 8, 1.0, 100);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, 699.0,
                   "node potential update changed the exact optimum");
        expectPotentialUpdateDecisionAccounting(
            result.stats, "node potential update");
        if (result.stats.search_node_potential_updates_triggered == 0
            || result.stats.potential_updates_improved == 0
            || result.stats.potential_updates_pruned == 0
            || result.stats.search_node_potential_iterations == 0) {
            throw std::runtime_error(
                "node potential update regression did not exercise its trigger");
        }
        if (result.stats.potential_updates_improved
            != result.stats.potential_updates_pruned
                + result.stats.potential_updates_rebuilt) {
            throw std::runtime_error(
                "node potential update did not persist every non-pruning improvement");
        }
    }

    // 基础轮数为 0 时，可只启用大 gap 分档；threshold=0 使本例所有命中
    // gap/深度门的尝试都走 8 轮分档。
    {
        tsp::BranchBoundSolver solver(matrix);
        solver.setPotentialUpdateOptions(
            tsp::PotentialUpdateStrategy::SubtreeAdaptive,
            1, 0, 1.0, 100);
        solver.setPotentialUpdateGapSchedule(0.0, 0.0, 8);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, 699.0,
                   "large-gap iteration tier changed the exact optimum");
        expectPotentialUpdateDecisionAccounting(
            result.stats, "large-gap iteration tier");
        if (result.stats.search_node_potential_updates_triggered == 0
            || result.stats.potential_updates_large_gap_tier
                != result.stats.search_node_potential_updates_triggered) {
            throw std::runtime_error(
                "large-gap iteration tier did not classify every update");
        }
    }

    // dantzig42 搜索中的相对 gap 远小于 100%；把最小门设为 100% 应完全
    // 阻止 Adaptive 更新，同时不改变精确结果。
    {
        tsp::BranchBoundSolver solver(matrix);
        solver.setPotentialUpdateOptions(
            tsp::PotentialUpdateStrategy::SubtreeAdaptive,
            1, 8, 1.0, 100);
        solver.setPotentialUpdateGapSchedule(1.0, 0.0, 0);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, 699.0,
                   "minimum gap gate changed the exact optimum");
        expectPotentialUpdateDecisionAccounting(
            result.stats, "minimum gap gate");
        if (result.stats.search_node_potential_updates_triggered != 0) {
            throw std::runtime_error(
                "minimum gap gate unexpectedly allowed an update");
        }
        if (result.stats.search_node_potential_updates_skipped_gap_below_minimum
            == 0) {
            throw std::runtime_error(
                "minimum gap gate did not report its skipped nodes");
        }
    }

    // 节点 Helsgaun 使用与 Polyak 相同的触发与 epoch 路径，只替换一次
    // updateNodePotentialBound 内部的步长调度。这里要求它实际进入更新循环，
    // 并保持精确最优值不变；是否改善/剪枝属于后续 A/B 的性能指标。
    {
        tsp::BranchBoundSolver solver(matrix);
        solver.setNodeAscentStrategy(tsp::NodeAscentStrategy::Helsgaun);
        solver.setPotentialUpdateOptions(
            tsp::PotentialUpdateStrategy::SubtreeAdaptive,
            1, 16, 1.0, 100);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, 699.0,
                   "node Helsgaun update changed the exact optimum");
        expectPotentialUpdateDecisionAccounting(
            result.stats, "node Helsgaun update");
        if (result.stats.search_node_potential_updates_triggered == 0
            || result.stats.search_node_potential_iterations == 0) {
            throw std::runtime_error(
                "node Helsgaun regression did not exercise its update loop");
        }
    }

    // 同时对一个独立穷举可验证的受约束搜索实例启用零根势，确保节点
    // 更新生成的证书不会越过真实最优值。
    const std::vector<std::vector<double>> small = {
        {0, 585, 792, 891, 348, 768},
        {585, 0, 249, 83, 294, 778},
        {792, 249, 0, 14, 340, 834},
        {891, 83, 14, 0, 530, 399},
        {348, 294, 340, 530, 0, 325},
        {768, 778, 834, 399, 325, 0},
    };
    const double small_optimum = bruteForceOptimalCost(small);
    for (const tsp::NodeAscentStrategy node_ascent : {
             tsp::NodeAscentStrategy::Polyak,
             tsp::NodeAscentStrategy::Helsgaun}) {
        for (const tsp::PotentialUpdateStrategy strategy : {
                 tsp::PotentialUpdateStrategy::SubtreeDepth,
                 tsp::PotentialUpdateStrategy::SubtreeAdaptive}) {
            tsp::BranchBoundSolver solver(small);
            solver.setRootAscentStrategy(tsp::RootAscentStrategy::None);
            solver.setNodeAscentStrategy(node_ascent);
            solver.setPotentialUpdateOptions(strategy, 1, 16, 1.0, 100);
            const tsp::SolveResult result = solver.solve();
            expectCost(result.cost, small_optimum,
                       "node potential epoch exceeded brute-force optimum");
        }
    }

    std::ifstream st70_input(
        std::string(TSP_TEST_SOURCE_DIR)
        + "/data/classic/tsplib/st70.tsp");
    if (!st70_input) {
        throw std::runtime_error(
            "cannot open st70 persistent-potential regression");
    }
    const tsp::TspProblem st70_problem = tsp::readTspProblem(st70_input);
    tsp::BranchBoundSolver subtree_solver(st70_problem.toDenseMatrix(70));
    subtree_solver.setPotentialUpdateOptions(
        tsp::PotentialUpdateStrategy::SubtreeAdaptive,
        4, 8, 0.01, 100);
    const tsp::SolveResult subtree_result = subtree_solver.solve();
    expectCost(subtree_result.cost, 675.0,
               "persistent potential epoch changed the st70 optimum");
    if (subtree_result.stats.potential_updates_rebuilt == 0) {
        throw std::runtime_error(
            "persistent potential regression rebuilt no subtree epoch");
    }
    if (subtree_result.stats.potential_updates_stopped_prunable == 0) {
        throw std::runtime_error(
            "persistent potential regression exercised no prunable early stop");
    }

    // 两阶段筛选在 2%--5% 的外层 gap 区间先观察两次势移动。coverage=1
    // 会拒绝所有尚未直接形成剪枝证书的 probe，用于同时覆盖“进入”和
    // “丢弃”路径；2% 内的节点仍按原策略跑满。
    tsp::BranchBoundSolver probe_solver(st70_problem.toDenseMatrix(70));
    probe_solver.setPotentialUpdateOptions(
        tsp::PotentialUpdateStrategy::SubtreeAdaptive,
        2, 16, 0.05, 5000);
    probe_solver.setPotentialUpdateProbeOptions(2, 0.02, 1.0);
    const tsp::SolveResult probe_result = probe_solver.solve();
    expectCost(probe_result.cost, 675.0,
               "potential probe changed the st70 optimum");
    if (probe_result.stats.potential_update_probes_started == 0
        || probe_result.stats.potential_update_probes_rejected == 0) {
        throw std::runtime_error(
            "potential probe regression exercised no rejected probe");
    }
    if (probe_result.stats.potential_update_probes_continued != 0) {
        throw std::runtime_error(
            "coverage=1 unexpectedly continued a non-pruning probe");
    }
}

void testRootReducedCostFixing()
{
    std::ifstream input(
        std::string(TSP_TEST_SOURCE_DIR)
        + "/data/classic/tsplib/bayg29.tsp");
    if (!input) {
        throw std::runtime_error(
            "cannot open bayg29 reduced-cost regression");
    }
    const tsp::TspProblem problem = tsp::readTspProblem(input);
    tsp::BranchBoundSolver solver(problem.toDenseMatrix(29));
    std::ostringstream debug;
    solver.setDebugOutput(debug, std::numeric_limits<std::size_t>::max());
    const tsp::SolveResult result = solver.solve();
    expectCost(result.cost, 1610.0,
               "bayg29 reduced-cost fixing changed the optimum");

    const std::string debug_output = debug.str();
    const std::string marker = "root reduced-cost fixing: tested=";
    const std::size_t marker_pos = debug_output.find(marker);
    if (marker_pos == std::string::npos) {
        throw std::runtime_error(
            "bayg29 did not exercise root reduced-cost fixing");
    }
    const std::size_t fixed_pos = debug_output.find("fixed_zero=", marker_pos);
    if (fixed_pos == std::string::npos
        || std::stoull(debug_output.substr(fixed_pos + 11)) == 0) {
        throw std::runtime_error(
            "bayg29 root reduced-cost fixing removed no edges");
    }
    const std::size_t forced_pos =
        debug_output.find("fixed_one=", fixed_pos);
    if (forced_pos == std::string::npos
        || std::stoull(debug_output.substr(forced_pos + 10)) == 0) {
        throw std::runtime_error(
            "bayg29 root reduced-cost fixing forced no tree edges");
    }
}

} // namespace

int main()
{
    try {
        testInternalReplacement();
        testRootAlphaNearness();
        testRootAlphaGlobalPriority();
        testRootOneTreeFrequencyCollection();
        testCurrentForbidReplacementDelta();
        testRootReplacement();
        testNonTreeForbid();
        testSequentialForbids();
        testMissingInternalReplacement();
        testMissingRootReplacement();
        testTiedReplacement();
        testMixedRootAndInternalForcedEdges();
        testOptionalRootReplacementWithForcedRootEdge();
        testMultipleForcedInternalEdges();
        testRandomSequentialForbids();
        testPackedMstComponentWordBoundaries();
        testRandomSparseTiedForbids();
        testBpPrefixTreeRegressions();
        testScaleSafeExactSearch();
        testMixedMagnitudeForceRollback();
        testRandomCompleteSolveAgainstBruteForce();
        testRandomSparseSolveAgainstBruteForce();
        testProblemParsingDoesNotWriteStdout();
        testDistanceMatrixSymmetryIsExact();
        testExactIntegerPruningDomain();
        testRootAscentStrategies();
        testSearchNodePotentialUpdates();
        testDiversifiedInitialTourPool();
        testRootReducedCostFixing();
    } catch (const std::exception& error) {
        std::cerr << "tsp_solver_tests failed: " << error.what() << '\n';
        return 1;
    }

    std::cout << "tsp_solver_tests passed\n";
    return 0;
}
