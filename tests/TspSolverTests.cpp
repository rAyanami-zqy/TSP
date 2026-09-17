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

        void verifyCompactedEpoch(const std::vector<double>& potentials)
        {
            const auto active = node.candidate_mask;
            Node compact_node = node;
            const auto compact = solver.rebuildPotentialEpoch(compact_node, potentials, active);
            Node full_node = compact_node;
            full_node.candidate_bits.clear();
            full_node.candidate_mask = active;
            auto full_candidates = candidates;
            std::size_t retained = 0;
            for (auto& e : full_candidates) {
                e.w = solver.adjustedEdgeWeight(e.u, e.v);
                const auto id = solver.edgeId(e.u, e.v);
                if (active[id] || node.forced[id]) ++retained;
                expect(solver.isCandidateActive(compact_node, id) == (active[id] != 0),
                       "epoch compaction changed edge activity");
            }
            sortEdges(full_candidates);
            const auto full = solver.computeOneTree(full_node, full_candidates);
            expect(compact.feasible && full.feasible, "epoch test lost its feasible cycle");
            expect(std::fabs(compact.cost - full.cost) <= 1e-9 && compact.degree == full.degree,
                   "compacted epoch differs from full-edge constrained tree");
            const auto compact_update = solver.updateNodePotentialBound(
                compact_node, -std::numeric_limits<double>::infinity(),
                compact.cost + std::max(1.0, std::fabs(compact.cost)), 1);
            expect(compact_update.feasible,
                   "compact-candidate CSR potential evaluator rejected a feasible state");
            expect(std::fabs(compact_update.bound - compact.cost) <= 1e-8,
                   "compact-candidate CSR potential bound differs from rebuilt tree");
#ifndef TSP_DISABLE_EPOCH_COMPACTION
            expect(solver.candidates_sorted_.size() == retained,
                   "epoch retained an inactive unforced edge or lost a forced edge");
#endif
        }

        void verifyUnlimitedPotentialUpdateBudget()
        {
            solver.initial_tour_alternatives_.push_back(
                BranchBoundSolver::TourCandidate{});
            solver.setPotentialUpdateOptions(
                PotentialUpdateStrategy::SubtreeDepth,
                1, 1, 5000);
            solver.potential_updates_in_round_ = 1000;
            const auto limited = solver.classifyPotentialUpdate(
                tree, 1, tree.cost,
                tree.cost + std::max(1.0, std::fabs(tree.cost)),
                node.forced_edges.size());
            expect(
                limited
                    == BranchBoundSolver::PotentialUpdateDecision::BudgetExhausted,
                   "positive node-potential budget was not enforced");

            solver.setPotentialUpdateOptions(
                PotentialUpdateStrategy::SubtreeDepth,
                1, 1, 0);
            const auto unlimited = solver.classifyPotentialUpdate(
                tree, 1, tree.cost,
                tree.cost + std::max(1.0, std::fabs(tree.cost)),
                node.forced_edges.size());
            expect(
                unlimited
                    != BranchBoundSolver::PotentialUpdateDecision::BudgetExhausted,
                "zero node-potential budget did not disable the limit");
        }

        void verifyLargeAbsoluteGapDoesNotBlockPotentialUpdate()
        {
            solver.setPotentialUpdateOptions(
                PotentialUpdateStrategy::SubtreeAdaptive,
                1, 1, 0);
            solver.setPotentialUpdateGapChangeThreshold(0.0);
            const double scale = std::max(1.0, std::fabs(tree.cost));
            const auto decision = solver.classifyPotentialUpdate(
                tree, 1, tree.cost, tree.cost + 100.0 * scale,
                node.forced_edges.size());
            expect(
                decision == BranchBoundSolver::PotentialUpdateDecision::Trigger,
                "large absolute gap unexpectedly blocked a shallow potential update");
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
        // 本测试专门覆盖历史固定三起点路径；Adaptive 的根 gap 触发在
        // solve() 集成测试中验证，不能由只调用 findInitialTour 的助手触发。
        solver.setInitialClkStrategy(InitialClkStrategy::Triple);
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
        + stats.search_node_potential_updates_skipped_max_depth
        + stats.search_node_potential_updates_skipped_near_leaf
        + stats.search_node_potential_updates_skipped_depth_interval
        + stats
            .search_node_potential_updates_skipped_gap_change_below_minimum;
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
             tsp::BranchEdgeOrder::TwoSidedStrongBranchingTop2,
             tsp::BranchEdgeOrder::AscentStrongBranchingTop2}) {
        tsp::BranchBoundSolver solver(matrix);
        solver.setRootAscentStrategy(
            order == tsp::BranchEdgeOrder::RootOneTreeFrequencyMiddle
            ? tsp::RootAscentStrategy::Polyak
            : tsp::RootAscentStrategy::None);
        solver.setBranchEdgeOrder(order);
        if (order == tsp::BranchEdgeOrder::AscentStrongBranchingTop2) {
            solver.setPotentialUpdateOptions(
                tsp::PotentialUpdateStrategy::SubtreeAdaptive, 1, 8, 100);
        }
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, optimum,
                   "experimental branch order changed the exact optimum");
        if (order == tsp::BranchEdgeOrder::AscentStrongBranchingTop2) {
            Fixture::expect(
                result.stats.branch_ascent_strong_probes > 0,
                "ascent strong branching did not probe the root/shallow tree");
            Fixture::expect(
                result.stats.branch_ascent_strong_seconds >= 0.0,
                "ascent strong branching reported a negative duration");
        }
    }

    {
        tsp::BranchBoundSolver solver(matrix);
        solver.setPotentialUpdateOptions(
            tsp::PotentialUpdateStrategy::SubtreeAdaptive, 1, 8, 100);
        solver.setBranchLiftFirstDepth(2);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, optimum,
                   "lift-first branch order changed the exact optimum");
        expectPotentialUpdateDecisionAccounting(
            result.stats, "lift-first branch order");
    }

    {
        tsp::BranchBoundSolver solver(matrix);
        solver.setPotentialUpdateOptions(
            tsp::PotentialUpdateStrategy::SubtreeAdaptive, 1, 8, 100);
        solver.setBranchSplitZeroGainDepth(2);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, optimum,
                   "zero-gain split changed the exact optimum");
        expectPotentialUpdateDecisionAccounting(
            result.stats, "zero-gain split");
    }

    {
        tsp::BranchBoundSolver solver(matrix);
        solver.setPotentialUpdateOptions(
            tsp::PotentialUpdateStrategy::SubtreeAdaptive, 1, 8, 100);
        solver.setBranchEdgeOrder(
            tsp::BranchEdgeOrder::AscentStrongBranchingTop2);
        solver.setBranchLiftFirstDepth(2);
        solver.setBranchSplitZeroGainDepth(2);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, optimum,
                   "combined branch experiment changed the exact optimum");
        expectPotentialUpdateDecisionAccounting(
            result.stats, "combined branch experiment");
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
        tsp::PotentialUpdateStrategy::SubtreeAdaptive, 1, 16, 100);
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

void testUnlimitedPotentialUpdateBudget()
{
    Fixture fixture(replacementMatrix());
    fixture.verifyUnlimitedPotentialUpdateBudget();
}

void testLargeAbsoluteGapDoesNotBlockPotentialUpdate()
{
    Fixture fixture(replacementMatrix());
    fixture.verifyLargeAbsoluteGapDoesNotBlockPotentialUpdate();
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
                1, 16, 100);
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
                1, 16, 100);
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

    // 最优三个 NN+2-opt 起点会在根搜索前立即做 chained LK；st70 因而
    // 在原来的延迟 diversified 阶段之前就得到 682 上界。
    expectCost(stats.initial_cost, 682.0,
               "st70 immediate multi-start LK did not tighten the upper bound");
    if (!stats.improved || stats.alternative_count < 10
        || stats.diversified_cost > stats.initial_cost + 1e-9) {
        throw std::runtime_error(
            "st70 diversified-tour pool was not populated or used");
    }

    tsp::BranchBoundSolver adaptive(problem.toDenseMatrix(70));
    adaptive.setRootBoundOnly(true);
    adaptive.setInitialClkStrategy(
        tsp::InitialClkStrategy::Adaptive, 0.0, 2);
    const tsp::SolveResult adaptive_result = adaptive.solve();
    expectCost(adaptive_result.cost, 682.0,
               "adaptive CLK did not tighten the st70 upper bound");
    if (adaptive_result.stats.adaptive_clk_triggers != 1
        || adaptive_result.stats.adaptive_clk_improvements != 1
        || adaptive_result.stats.initial_clk_starts != 3
        || adaptive_result.stats.root_ascent_seconds <= 0.0
        || adaptive_result.stats.initial_tour_seconds <= 0.0) {
        throw std::runtime_error(
            "adaptive CLK or phase timing statistics were not recorded");
    }

    bool invalid_gap_rejected = false;
    try {
        adaptive.setInitialClkStrategy(
            tsp::InitialClkStrategy::Adaptive, -0.1, 2);
    } catch (const std::invalid_argument&) {
        invalid_gap_rejected = true;
    }
    if (!invalid_gap_rejected) {
        throw std::runtime_error("negative adaptive CLK gap was accepted");
    }
}

void testRootGuidedAlphaLk()
{
    std::ifstream input(
        std::string(TSP_TEST_SOURCE_DIR)
        + "/data/classic/tsplib/gr48.tsp");
    if (!input) {
        throw std::runtime_error("cannot open gr48 root-guided LK regression");
    }
    const tsp::TspProblem problem = tsp::readTspProblem(input);
    const auto matrix = problem.toDenseMatrix(48);

    tsp::BranchBoundSolver baseline(matrix);
    baseline.setRootAscentStrategy(tsp::RootAscentStrategy::HybridReverse);
    baseline.setInitialClkStrategy(tsp::InitialClkStrategy::Single);
    baseline.setRootBoundOnly(true);
    const tsp::SolveResult baseline_result = baseline.solve();

    tsp::BranchBoundSolver guided(matrix);
    guided.setRootAscentStrategy(tsp::RootAscentStrategy::HybridReverse);
    guided.setInitialClkStrategy(tsp::InitialClkStrategy::Single);
    guided.setLkCandidateSetOptions(
        tsp::LkCandidateSetStrategy::Hybrid, 8);
    guided.setRootGuidedLk(true, false);
    guided.setRootBoundOnly(true);
    const tsp::SolveResult guided_result = guided.solve();
    expectCost(guided_result.cost, 5055.0,
               "root alpha/hybrid LK did not improve gr48 incumbent");
    if (baseline_result.cost != 5093.0
        || guided_result.stats.root_guided_lk_calls != 1
        || guided_result.stats.root_guided_lk_improvements != 1
        || guided_result.stats.root_guided_lk_reascents != 0
        || guided_result.stats.root_guided_lk_total_gain != 38.0
        || guided_result.stats.root_guided_lk_seconds <= 0.0
        || guided_result.stats.initial_clk_starts != 2
        || guided_result.stats.root_potential_iterations
            != baseline_result.stats.root_potential_iterations) {
        throw std::runtime_error(
            "root-guided LK statistics or no-reascent semantics changed");
    }

    tsp::BranchBoundSolver reascended(matrix);
    reascended.setRootAscentStrategy(tsp::RootAscentStrategy::HybridReverse);
    reascended.setInitialClkStrategy(tsp::InitialClkStrategy::Single);
    reascended.setLkCandidateSetOptions(
        tsp::LkCandidateSetStrategy::Hybrid, 8);
    reascended.setRootGuidedLk(true, true);
    reascended.setRootBoundOnly(true);
    const tsp::SolveResult reascended_result = reascended.solve();
    expectCost(reascended_result.cost, guided_result.cost,
               "root re-ascent changed the root-guided incumbent");
    if (reascended_result.stats.root_guided_lk_reascents != 1
        || reascended_result.stats.root_potential_iterations
            <= guided_result.stats.root_potential_iterations) {
        throw std::runtime_error(
            "root-guided LK re-ascent switch did not rerun root ascent");
    }

    bool zero_candidates_rejected = false;
    try {
        guided.setLkCandidateSetOptions(
            tsp::LkCandidateSetStrategy::Alpha, 0);
    } catch (const std::invalid_argument&) {
        zero_candidates_rejected = true;
    }
    if (!zero_candidates_rejected) {
        throw std::runtime_error("zero LK candidate count was accepted");
    }

    // Alpha 候选只改变 incumbent 启发式；用小规模随机完全图确认它不改变
    // 精确搜索的最优性证书或可行域。
    std::mt19937 rng(209914);
    for (int trial = 0; trial < 12; ++trial) {
        const int n = 6 + trial % 3;
        std::vector<std::vector<double>> random_matrix(
            n, std::vector<double>(n, 0.0));
        for (int u = 0; u < n; ++u) {
            for (int v = u + 1; v < n; ++v) {
                random_matrix[u][v] = random_matrix[v][u]
                    = 1.0 + static_cast<double>(rng() % 100);
            }
        }
        const double optimum = bruteForceOptimalCost(random_matrix);
        tsp::BranchBoundSolver exact(std::move(random_matrix));
        exact.setInitialClkStrategy(tsp::InitialClkStrategy::Single);
        exact.setLkCandidateSetOptions(
            tsp::LkCandidateSetStrategy::Alpha, 5);
        exact.setRootGuidedLk(true, false);
        expectCost(
            exact.solve().cost, optimum,
            "root-guided alpha LK changed an exact optimum");
    }
}

void testExternalRootPotentialSeed()
{
    const auto matrix = replacementMatrix();
    const double optimum = bruteForceOptimalCost(matrix);
    std::vector<double> seed(matrix.size());
    for (std::size_t index = 0; index < seed.size(); ++index) {
        seed[index] = 0.75 * static_cast<double>(index)
            - static_cast<double>(index % 2);
    }

    tsp::BranchBoundSolver replaced(matrix);
    replaced.setRootPotentialSeed(
        seed, tsp::RootPotentialSeedStrategy::Replace);
    replaced.setRootBoundOnly(true);
    const tsp::SolveResult replaced_result = replaced.solve();
    if (replaced_result.stats.root_external_potential_replacements != 1
        || replaced_result.stats.root_external_potential_warm_starts != 0
        || replaced_result.stats.root_potential_iterations != 0
        || replaced_result.stats.root_lower_bound > optimum + 1e-8) {
        throw std::runtime_error(
            "external root potential replacement was not a valid certificate");
    }

    tsp::BranchBoundSolver warmed(matrix);
    warmed.setRootAscentStrategy(tsp::RootAscentStrategy::Polyak);
    warmed.setRootAscentIterationLimit(8);
    warmed.setRootPotentialSeed(
        seed, tsp::RootPotentialSeedStrategy::WarmStart);
    warmed.setRootBoundOnly(true);
    const tsp::SolveResult warmed_result = warmed.solve();
    if (warmed_result.stats.root_external_potential_replacements != 0
        || warmed_result.stats.root_external_potential_warm_starts != 1
        || warmed_result.stats.root_potential_iterations == 0
        || warmed_result.stats.root_lower_bound > optimum + 1e-8) {
        throw std::runtime_error(
            "external root potential warm start was not a valid certificate");
    }

    tsp::BranchBoundSolver exact(matrix);
    exact.setRootPotentialSeed(
        seed, tsp::RootPotentialSeedStrategy::Replace);
    expectCost(exact.solve().cost, optimum,
               "external root potentials changed the exact optimum");

    bool wrong_size_rejected = false;
    try {
        exact.setRootPotentialSeed(
            {1.0}, tsp::RootPotentialSeedStrategy::Replace);
    } catch (const std::invalid_argument&) {
        wrong_size_rejected = true;
    }
    if (!wrong_size_rejected) {
        throw std::runtime_error("wrong-sized root potential seed was accepted");
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
    const double hybrid_reverse = root_bound(
        tsp::RootAscentStrategy::HybridReverse);
    const double polyak_smoothed = root_bound(
        tsp::RootAscentStrategy::PolyakSmoothed);
    const double polyak_smoothed_dynamic = root_bound(
        tsp::RootAscentStrategy::PolyakSmoothedDynamic);
    if (polyak + 1e-8 < none || helsgaun + 1e-8 < none
        || hybrid + 1e-8 < polyak
        || hybrid_reverse + 1e-8 < helsgaun
        || polyak_smoothed + 1e-8 < none
        || polyak_smoothed_dynamic + 1e-8 < none) {
        throw std::runtime_error(
            "an ascent strategy discarded its zero/warm-start lower bound");
    }

    for (const tsp::RootAscentStrategy strategy : {
             tsp::RootAscentStrategy::Helsgaun,
             tsp::RootAscentStrategy::Hybrid,
             tsp::RootAscentStrategy::HybridReverse,
             tsp::RootAscentStrategy::PolyakSmoothed,
             tsp::RootAscentStrategy::PolyakSmoothedDynamic}) {
        tsp::BranchBoundSolver solver(matrix);
        solver.setRootAscentStrategy(strategy);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, optimum,
                   "experimental root ascent changed the exact optimum");
    }
}

void testRootAscentTraceAndIterationLimit()
{
    const auto matrix = replacementMatrix();
    tsp::BranchBoundSolver solver(matrix);
    solver.setRootAscentStrategy(tsp::RootAscentStrategy::Polyak);
    solver.setRootAscentIterationLimit(3);
    solver.setRootBoundOnly(true);
    std::ostringstream trace;
    solver.setRootAscentTraceOutput(trace);
    const tsp::SolveResult result = solver.solve();

    if (!result.feasible || result.stats.nodes_expanded != 0) {
        throw std::runtime_error(
            "root ascent trace did not stay in root-bound-only mode");
    }
    std::istringstream rows(trace.str());
    std::string row;
    std::size_t row_count = 0;
    double previous_best = -std::numeric_limits<double>::infinity();
    while (std::getline(rows, row)) {
        if (row.empty()) continue;
        std::istringstream fields(row);
        std::vector<std::string> values;
        std::string value;
        while (std::getline(fields, value, ',')) values.push_back(value);
        if (values.size() != 6 || values[0] != "polyak"
            || values[2] != "polyak") {
            throw std::runtime_error(
                "root ascent trace row has an unexpected schema");
        }
        ++row_count;
        if (std::stoull(values[1]) != row_count
            || std::stoull(values[3]) != row_count) {
            throw std::runtime_error(
                "root ascent trace iterations are not contiguous");
        }
        const double best = std::stod(values[5]);
        if (!std::isfinite(std::stod(values[4]))
            || !std::isfinite(best) || best + 1e-9 < previous_best) {
            throw std::runtime_error(
                "root ascent trace contains an invalid lower bound");
        }
        previous_best = best;
    }
    if (row_count == 0 || row_count > 3
        || row_count != result.stats.root_potential_iterations) {
        throw std::runtime_error(
            "root ascent trace does not match the configured iteration limit");
    }

    bool rejected_zero_limit = false;
    try {
        solver.setRootAscentIterationLimit(0);
    } catch (const std::invalid_argument&) {
        rejected_zero_limit = true;
    }
    if (!rejected_zero_limit) {
        throw std::runtime_error("zero root ascent iteration limit was accepted");
    }

    for (const tsp::RootAscentStrategy strategy : {
             tsp::RootAscentStrategy::PolyakSmoothed,
             tsp::RootAscentStrategy::PolyakSmoothedDynamic}) {
        tsp::BranchBoundSolver configured_solver(matrix);
        configured_solver.setRootAscentStrategy(strategy);
        configured_solver.setRootAscentIterationLimit(20);
        configured_solver.setRootAscentDirectionSmoothing(
            0.65, 0.15, 0.4, 0.85);
        configured_solver.setRootBoundOnly(true);
        const tsp::SolveResult configured_result = configured_solver.solve();
        if (!configured_result.feasible
            || !std::isfinite(configured_result.stats.root_lower_bound)) {
            throw std::runtime_error(
                "configured root direction smoothing produced an invalid bound");
        }
    }

    auto rejects_direction_weights = [&](double fixed_weight,
                                         double cosine_scale,
                                         double minimum_weight,
                                         double maximum_weight) {
        try {
            solver.setRootAscentDirectionSmoothing(
                fixed_weight, cosine_scale, minimum_weight, maximum_weight);
        } catch (const std::invalid_argument&) {
            return true;
        }
        return false;
    };
    if (!rejects_direction_weights(0.4, 0.2, 0.5, 0.9)
        || !rejects_direction_weights(0.8, 0.2, 0.5, 0.7)
        || !rejects_direction_weights(0.7, -0.1, 0.5, 0.9)
        || !rejects_direction_weights(
            std::numeric_limits<double>::quiet_NaN(), 0.2, 0.5, 0.9)) {
        throw std::runtime_error(
            "invalid root direction smoothing weights were accepted");
    }

    auto phase_order = [&](tsp::RootAscentStrategy strategy,
                           const std::string& expected_strategy) {
        tsp::BranchBoundSolver phase_solver(matrix);
        phase_solver.setRootAscentStrategy(strategy);
        phase_solver.setRootAscentIterationLimit(3);
        phase_solver.setRootBoundOnly(true);
        std::ostringstream phase_trace;
        phase_solver.setRootAscentTraceOutput(phase_trace);
        const tsp::SolveResult phase_result = phase_solver.solve();
        if (!phase_result.feasible) {
            throw std::runtime_error(
                "root ascent phase-order strategy lost its incumbent");
        }
        std::vector<std::string> phases;
        std::istringstream phase_rows(phase_trace.str());
        std::string phase_row;
        while (std::getline(phase_rows, phase_row)) {
            if (phase_row.empty()) continue;
            std::istringstream fields(phase_row);
            std::vector<std::string> values;
            std::string value;
            while (std::getline(fields, value, ',')) values.push_back(value);
            if (values.size() != 6 || values[0] != expected_strategy) {
                throw std::runtime_error(
                    "root ascent phase-order trace has an unexpected schema");
            }
            phases.push_back(values[2]);
        }
        return phases;
    };

    const std::vector<std::string> forward_phases = phase_order(
        tsp::RootAscentStrategy::Hybrid, "hybrid");
    const auto forward_switch = std::find(
        forward_phases.begin(), forward_phases.end(), "helsgaun");
    if (forward_phases.empty() || forward_phases.front() != "polyak"
        || forward_switch == forward_phases.end()) {
        throw std::runtime_error("hybrid trace is not ordered Polyak then Helsgaun");
    }

    const std::vector<std::string> reverse_phases = phase_order(
        tsp::RootAscentStrategy::HybridReverse, "hybrid-reverse");
    const auto reverse_switch = std::find(
        reverse_phases.begin(), reverse_phases.end(), "polyak");
    if (reverse_phases.empty() || reverse_phases.front() != "helsgaun"
        || reverse_switch == reverse_phases.end()) {
        throw std::runtime_error("reverse hybrid trace is not ordered Helsgaun then Polyak");
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
        solver.setPotentialUpdateOptions(strategy, 1, 8, 100);
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
            1, 0, 100);
        solver.setPotentialUpdateLargeGapTier(0.0, 8);
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

    // 基础轮数为 0 时，浅层档也能独立启用节点势上升。
    {
        // 测试实例的 DFS 深度远小于 100，因此每次触发都必须归入浅层档。
        tsp::BranchBoundSolver solver(matrix);
        solver.setPotentialUpdateOptions(
            tsp::PotentialUpdateStrategy::SubtreeAdaptive,
            1, 0, 100);
        solver.setPotentialUpdateShallowDepthTier(100, 8);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, 699.0,
                   "shallow-depth iteration tier changed the exact optimum");
        expectPotentialUpdateDecisionAccounting(
            result.stats, "shallow-depth iteration tier");
        if (result.stats.search_node_potential_updates_triggered == 0
            || result.stats.potential_updates_shallow_depth_tier
                != result.stats.search_node_potential_updates_triggered) {
            throw std::runtime_error(
                "shallow-depth iteration tier did not classify every update");
        }
    }

    // 浅层慢热延长默认关闭；启用后不得改变本例最优值。
    {
        tsp::BranchBoundSolver solver(matrix);
        solver.setPotentialUpdateOptions(
            tsp::PotentialUpdateStrategy::SubtreeAdaptive,
            1, 8, 100);
        solver.setPotentialUpdateSlowWarmExtend(4, 16);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, 699.0,
                   "slow-warm extend changed the exact optimum");
        expectPotentialUpdateDecisionAccounting(
            result.stats, "slow-warm extend");
        if (result.stats.search_node_potential_updates_triggered == 0) {
            throw std::runtime_error(
                "slow-warm extend regression did not trigger an update");
        }
    }

    // epoch-relative gap-change 门槛衡量自最近一次成功势上升以来的下界
    // 增量。100% 对本正权实例不可达，应阻止全部更新并保持精确性。
    {
        tsp::BranchBoundSolver solver(matrix);
        solver.setPotentialUpdateOptions(
            tsp::PotentialUpdateStrategy::SubtreeAdaptive,
            1, 8, 100);
        solver.setPotentialUpdateGapChangeThreshold(1.0);
        solver.setPotentialUpdateGapChangeStartDepth(0);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, 699.0,
                   "gap-change gate changed the exact optimum");
        expectPotentialUpdateDecisionAccounting(
            result.stats, "gap-change gate");
        if (result.stats.search_node_potential_updates_triggered != 0) {
            throw std::runtime_error(
                "gap-change gate unexpectedly allowed an update");
        }
        if (result.stats
                .search_node_potential_updates_skipped_gap_change_below_minimum
            == 0) {
            throw std::runtime_error(
                "gap-change gate did not report its skipped nodes");
        }
    }

    // 绝对最大深度门槛只允许浅层节点上升，深层节点直接展开。
    {
        tsp::BranchBoundSolver solver(matrix);
        solver.setPotentialUpdateOptions(
            tsp::PotentialUpdateStrategy::SubtreeAdaptive,
            1, 8, 100);
        solver.setPotentialUpdateMaxDepth(1);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, 699.0,
                   "maximum update depth changed the exact optimum");
        expectPotentialUpdateDecisionAccounting(
            result.stats, "maximum update depth");
        if (result.stats.search_node_potential_updates_skipped_max_depth == 0) {
            throw std::runtime_error(
                "maximum update depth did not report its skipped nodes");
        }
    }

    // 倒置深度门槛按剩余 tour 边而不是绝对 DFS depth 判断。阈值 n 会让
    // 所有非根候选节点直接展开，用于验证统计与精确性。
    {
        tsp::BranchBoundSolver solver(matrix);
        solver.setPotentialUpdateOptions(
            tsp::PotentialUpdateStrategy::SubtreeAdaptive,
            1, 8, 100);
        solver.setPotentialUpdateSkipLastEdges(42);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, 699.0,
                   "near-leaf update gate changed the exact optimum");
        expectPotentialUpdateDecisionAccounting(
            result.stats, "near-leaf update gate");
        if (result.stats.search_node_potential_updates_skipped_near_leaf == 0) {
            throw std::runtime_error(
                "near-leaf update gate did not report its skipped nodes");
        }
    }

    // 即使 gap-change 门槛本身不可达，浅层保护区仍应允许 depth<=1 的
    // 节点按原条件触发势上升，并保持决策统计闭合。
    {
        tsp::BranchBoundSolver solver(matrix);
        solver.setPotentialUpdateOptions(
            tsp::PotentialUpdateStrategy::SubtreeAdaptive,
            1, 8, 100);
        solver.setPotentialUpdateGapChangeThreshold(1.0);
        solver.setPotentialUpdateGapChangeStartDepth(1);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, 699.0,
                   "gap-change shallow protection changed the exact optimum");
        expectPotentialUpdateDecisionAccounting(
            result.stats, "gap-change shallow protection");
        if (result.stats.potential_update_gap_change_shallow_bypasses == 0
            || result.stats.search_node_potential_updates_triggered == 0) {
            throw std::runtime_error(
                "gap-change shallow protection did not bypass the gate");
        }
    }

    // 三个实验节点策略与 Polyak 使用相同的触发与 epoch 路径，只替换一次
    // updateNodePotentialBound 内部的调度或方向。这里要求它们实际进入更新
    // 循环并保持精确最优值不变；是否改善/剪枝属于后续 A/B 的性能指标。
    for (const tsp::NodeAscentStrategy node_ascent : {
             tsp::NodeAscentStrategy::Helsgaun,
             tsp::NodeAscentStrategy::PolyakSmoothed,
             tsp::NodeAscentStrategy::PolyakSmoothedDynamic}) {
        tsp::BranchBoundSolver solver(matrix);
        solver.setNodeAscentStrategy(node_ascent);
        solver.setPotentialUpdateOptions(
            tsp::PotentialUpdateStrategy::SubtreeAdaptive,
            1, 16, 100);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, 699.0,
                   "experimental node ascent changed the exact optimum");
        expectPotentialUpdateDecisionAccounting(
            result.stats, "experimental node ascent update");
        if (result.stats.search_node_potential_updates_triggered == 0
            || result.stats.search_node_potential_iterations == 0) {
            throw std::runtime_error(
                "experimental node ascent did not exercise its update loop");
        }
    }

    {
        tsp::BranchBoundSolver solver(matrix);
        solver.setNodeAscentStrategy(
            tsp::NodeAscentStrategy::PolyakSmoothedDynamic);
        solver.setNodeAscentDirectionSmoothing(0.65, 0.15, 0.4, 0.85);
        solver.setPotentialUpdateOptions(
            tsp::PotentialUpdateStrategy::SubtreeAdaptive,
            1, 16, 100);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, 699.0,
                   "custom node smoothing changed the exact optimum");
    }

    auto rejects_node_direction_weights = [&](double fixed_weight,
                                               double cosine_scale,
                                               double minimum_weight,
                                               double maximum_weight) {
        tsp::BranchBoundSolver solver(matrix);
        try {
            solver.setNodeAscentDirectionSmoothing(
                fixed_weight, cosine_scale, minimum_weight, maximum_weight);
        } catch (const std::invalid_argument&) {
            return true;
        }
        return false;
    };
    if (!rejects_node_direction_weights(0.4, 0.2, 0.5, 0.9)
        || !rejects_node_direction_weights(0.8, 0.2, 0.5, 0.7)
        || !rejects_node_direction_weights(0.7, -0.1, 0.5, 0.9)) {
        throw std::runtime_error(
            "invalid node direction smoothing weights were accepted");
    }

    auto rejects_sibling_warm_weight = [&](double warm_weight) {
        tsp::BranchBoundSolver solver(matrix);
        try {
            solver.setNodeAscentSiblingWarmWeight(warm_weight);
        } catch (const std::invalid_argument&) {
            return true;
        }
        return false;
    };
    if (!rejects_sibling_warm_weight(-0.1)
        || !rejects_sibling_warm_weight(1.1)
        || !rejects_sibling_warm_weight(
            std::numeric_limits<double>::infinity())) {
        throw std::runtime_error(
            "invalid node sibling warm weights were accepted");
    }
    {
        tsp::BranchBoundSolver solver(matrix);
        solver.setNodeAscentSiblingWarmWeight(0.0);
        solver.setPotentialUpdateOptions(
            tsp::PotentialUpdateStrategy::SubtreeAdaptive,
            1, 16, 100);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, 699.0,
                   "disabling sibling warm start changed the exact optimum");
    }
    {
        tsp::BranchBoundSolver solver(matrix);
        solver.setNodeAscentSiblingWarmWeight(0.25);
        solver.setNodeAscentSiblingWarmStartStrategy(
            tsp::SiblingWarmStartStrategy::Guarded);
        solver.setPotentialUpdateOptions(
            tsp::PotentialUpdateStrategy::SubtreeAdaptive,
            1, 16, 100);
        const tsp::SolveResult result = solver.solve();
        expectCost(result.cost, 699.0,
                   "guarded sibling warm start changed the exact optimum");
        if (result.stats.sibling_warm_probes == 0
            || result.stats.sibling_warm_accepted
                    + result.stats.sibling_warm_rejected
                != result.stats.sibling_warm_probes) {
            throw std::runtime_error(
                "guarded sibling warm decisions were not fully accounted");
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
             tsp::NodeAscentStrategy::Helsgaun,
             tsp::NodeAscentStrategy::PolyakSmoothed,
             tsp::NodeAscentStrategy::PolyakSmoothedDynamic}) {
        for (const tsp::PotentialUpdateStrategy strategy : {
                 tsp::PotentialUpdateStrategy::SubtreeDepth,
                 tsp::PotentialUpdateStrategy::SubtreeAdaptive}) {
            tsp::BranchBoundSolver solver(small);
            solver.setRootAscentStrategy(tsp::RootAscentStrategy::None);
            solver.setNodeAscentStrategy(node_ascent);
            solver.setPotentialUpdateOptions(strategy, 1, 16, 100);
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
        4, 8, 100);
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

    // 更紧的多启动 LK 初始上界使 st70 的相关节点 gap 落到 1%--5%；
    // 两阶段筛选在该外层区间先观察两次势移动。coverage=1
    // 会拒绝所有尚未直接形成剪枝证书的 probe，用于同时覆盖“进入”和
    // “丢弃”路径；1% 内的节点仍按原策略跑满。
    tsp::BranchBoundSolver probe_solver(st70_problem.toDenseMatrix(70));
    probe_solver.setPotentialUpdateOptions(
        tsp::PotentialUpdateStrategy::SubtreeAdaptive,
        2, 16, 5000);
    probe_solver.setPotentialUpdateProbeOptions(2, 0.01, 1.0);
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
    if (result.stats.root_fixing_calls == 0
        || result.stats.root_fixing_tested == 0
        || result.stats.root_fixing_fixed_zero == 0
        || result.stats.root_fixing_tree_tested == 0
        || result.stats.root_fixing_fixed_one == 0
        || result.stats.root_fixing_active_after == 0
        || result.stats.root_fixing_seconds < 0.0
        || result.stats.root_candidate_compaction_calls == 0
        || result.stats.root_candidate_edges_before
            <= result.stats.root_candidate_edges_after
        || result.stats.root_candidate_edges_after
            != result.stats.root_fixing_active_after
                + result.stats.root_fixing_fixed_one
        || result.stats.root_candidate_compaction_seconds < 0.0) {
        throw std::runtime_error(
            "bayg29 fixing/compaction statistics were not exported");
    }

    tsp::BranchBoundSolver baseline(problem.toDenseMatrix(29));
    baseline.setRootCandidateCompaction(false);
    const tsp::SolveResult baseline_result = baseline.solve();
    expectCost(baseline_result.cost, result.cost,
               "root candidate compaction changed the optimum");
    if (baseline_result.stats.root_candidate_compaction_calls != 0
        || baseline_result.stats.nodes_created != result.stats.nodes_created
        || baseline_result.stats.nodes_expanded != result.stats.nodes_expanded
        || baseline_result.stats.root_fixing_fixed_zero
            != result.stats.root_fixing_fixed_zero
        || baseline_result.stats.root_fixing_fixed_one
            != result.stats.root_fixing_fixed_one) {
        throw std::runtime_error(
            "root candidate compaction changed the bayg29 search tree");
    }

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
    if (debug_output.find("root candidate compaction: before=")
        == std::string::npos) {
        throw std::runtime_error(
            "bayg29 did not report root candidate compaction");
    }
}

void testCompactedEpoch()
{
    std::mt19937 rng(82731);
    const double inf = std::numeric_limits<double>::infinity();
    for (int trial = 0; trial < 96; ++trial) {
        const int n = 6 + trial % 13;
        std::vector<std::vector<double>> matrix(n, std::vector<double>(n, 0));
        for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v) {
            const bool cycle = v == u + 1 || (u == 0 && v == n - 1);
            const double value = !cycle && rng() % 5 == 0 ? inf : (1 + rng() % 9) * 0.25;
            matrix[u][v] = matrix[v][u] = value;
        }
        tsp::BranchBoundSolverTestAccess::Fixture f(matrix);
        if (trial % 3) f.force(0, 1);
        if (trial % 3 == 2) f.force(0, n - 1);
        f.force(1, 2);
        // Keep an entire feasible cycle; forced edges need not be active.
        for (const auto& e : f.candidates) {
            const auto id = static_cast<std::size_t>(e.u) * n + e.v;
            const bool cycle = e.v == e.u + 1 || (e.u == 0 && e.v == n - 1);
            if (f.node.forced[id] || (!cycle && rng() % 2 == 0)) f.node.candidate_mask[id] = 0;
        }
        std::vector<double> potentials(n);
        for (auto& value : potentials) value = (static_cast<int>(rng() % 41) - 20) * 0.125;
        f.verifyCompactedEpoch(potentials);
    }
}

// An external tour is only an incumbent, never an optimality certificate.
void testSuppliedInitialTour()
{
    std::mt19937 rng(91923);
    for (int trial = 0; trial < 32; ++trial) {
        const int n = 5 + trial % 4;
        std::vector<std::vector<double>> matrix(n, std::vector<double>(n, 0));
        for (int u = 0; u < n; ++u) for (int v = u + 1; v < n; ++v)
            matrix[u][v] = matrix[v][u] = 1 + rng() % 100;
        std::vector<int> tour(n);
        std::iota(tour.begin(), tour.end(), 0);
        std::shuffle(tour.begin(), tour.end(), rng);
        tsp::BranchBoundSolver solver(matrix);
        solver.setInitialTour(tour);
        solver.setPotentialUpdateOptions(tsp::PotentialUpdateStrategy::SubtreeDepth, 1, 8, 0);
        expectCost(solver.solve().cost, bruteForceOptimalCost(matrix), "supplied incumbent changed optimum");
        solver.setInitialTour({});
        expectCost(solver.solve().cost, bruteForceOptimalCost(matrix), "cleared incumbent changed optimum");
        for (const auto& bad : std::vector<std::vector<int>>{{0}, {0,0,1,2,3}, {-1,0,1,2,3}}) {
            bool rejected = false;
            try { solver.setInitialTour(bad); } catch (const std::invalid_argument&) { rejected = true; }
            if (!rejected) throw std::runtime_error("invalid external tour accepted");
        }
    }
    const double inf = std::numeric_limits<double>::infinity();
    tsp::BranchBoundSolver sparse({{0,1,inf,1},{1,0,1,inf},{inf,1,0,1},{1,inf,1,0}});
    sparse.setInitialTour({0,1,2,3});
    bool rejected = false;
    try { sparse.setInitialTour({0,2,1,3}); } catch (const std::invalid_argument&) { rejected = true; }
    if (!rejected) throw std::runtime_error("external tour with missing edge accepted");
    expectCost(sparse.solve().cost, 4.0, "invalid setter damaged previous incumbent");
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
        testUnlimitedPotentialUpdateBudget();
        testLargeAbsoluteGapDoesNotBlockPotentialUpdate();
        testRandomCompleteSolveAgainstBruteForce();
        testRandomSparseSolveAgainstBruteForce();
        testProblemParsingDoesNotWriteStdout();
        testDistanceMatrixSymmetryIsExact();
        testExactIntegerPruningDomain();
        testRootAscentStrategies();
        testRootAscentTraceAndIterationLimit();
        testSearchNodePotentialUpdates();
        testDiversifiedInitialTourPool();
        testRootGuidedAlphaLk();
        testExternalRootPotentialSeed();
        testRootReducedCostFixing();
        testCompactedEpoch();
        testSuppliedInitialTour();
    } catch (const std::exception& error) {
        std::cerr << "tsp_solver_tests failed: " << error.what() << '\n';
        return 1;
    }

    std::cout << "tsp_solver_tests passed\n";
    return 0;
}
