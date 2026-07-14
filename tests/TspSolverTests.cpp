#include "TspSolver.hpp"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <iostream>
#include <limits>
#include <numeric>
#include <random>
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
            node.forced_members.resize(static_cast<std::size_t>(n));
            for (int v = 0; v < n; ++v) {
                node.forced_members[static_cast<std::size_t>(v)].push_back(v);
            }
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

            node.candidate_bits.assign((candidates.size() + 63) / 64, 0);
            node.candidate_degree.assign(static_cast<std::size_t>(n), 0);
            solver.candidate_position_by_id_.assign(
                state_size, std::numeric_limits<std::size_t>::max());
            for (std::size_t index = 0; index < candidates.size(); ++index) {
                const Edge& candidate = candidates[index];
                const std::size_t id = solver.edgeId(candidate.u, candidate.v);
                node.candidate_bits[index / 64]
                    |= std::uint64_t{1} << (index % 64);
                solver.candidate_position_by_id_[id] = index;
                ++node.candidate_degree[static_cast<std::size_t>(candidate.u)];
                ++node.candidate_degree[static_cast<std::size_t>(candidate.v)];
            }

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

            const OneTree rebuilt = solver.computeOneTree(node, candidates);

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

        bool deactivateBatchAndCompare(
            const std::vector<std::pair<int, int>>& removed_edges)
        {
            std::vector<std::size_t> removed_ids;
            removed_ids.reserve(removed_edges.size());
            for (const auto& [u, v] : removed_edges) {
                solver.deactivateCandidate(
                    node, solver.edgeId(u, v), removed_ids);
            }

            OneTree incremental = tree;
            const bool updated = solver.updateOneTreeAfterCandidateRemoval(
                node, candidates, incremental, removed_ids);
            const OneTree rebuilt = solver.computeOneTree(node, candidates);
            expect(updated == rebuilt.feasible,
                   "batch dynamic MST and rebuilt feasibility differ");
            if (updated) {
                expect(std::fabs(incremental.cost - rebuilt.cost) <= 1e-9,
                       "batch dynamic MST and rebuilt costs differ");
                validate(incremental);
            }
            tree = std::move(incremental);
            return updated;
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
            std::vector<Edge> rebuilt_candidates = candidates;
            std::vector<std::size_t> removed_ids;
            expect(solver.buildBranchCandidates(node, rebuilt_candidates, &removed_ids),
                   "forced test state unexpectedly infeasible");
            std::vector<std::size_t> deactivated_ids;
            for (const std::size_t id : removed_ids) {
                solver.deactivateCandidate(node, id, deactivated_ids);
            }
            tree = solver.computeOneTree(node, candidates);
            expect(tree.feasible, "forced test state has no 1-tree");
            validate(tree);
        }

        void expectSameTree(
            const OneTree& actual, const OneTree& expected,
            const std::string& message) const
        {
            bool same_cost = false;
            if (std::isfinite(actual.cost) && std::isfinite(expected.cost)) {
                same_cost = std::fabs(actual.cost - expected.cost) <= 1e-9;
            } else {
                same_cost = !std::isfinite(actual.cost) && !std::isfinite(expected.cost);
            }

            bool same_edges = actual.edges.size() == expected.edges.size();
            if (same_edges) {
                for (std::size_t index = 0; index < actual.edges.size(); ++index) {
                    const Edge& a = actual.edges[index];
                    const Edge& b = expected.edges[index];
                    if (a.u != b.u || a.v != b.v || std::fabs(a.w - b.w) > 1e-9) {
                        same_edges = false;
                        break;
                    }
                }
            }

            expect(actual.feasible == expected.feasible
                       && same_cost
                       && same_edges
                       && actual.degree == expected.degree
                       && actual.mst_adjacency_bits == expected.mst_adjacency_bits
                       && actual.root_candidate_cursor == expected.root_candidate_cursor,
                   message);
        }

        void validate(const OneTree& value) const
        {
            expect(value.feasible, "cannot validate infeasible 1-tree");
            expect(static_cast<int>(value.edges.size()) == solver.n_,
                   "1-tree must contain n edges");
            expect(solver.dynamicMstMatchesEdges(value),
                   "dynamic MST adjacency differs from 1-tree edges");

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

std::vector<std::vector<double>> earlierSkippedReplacementMatrix()
{
    return {
        {0, 20, 21, 22, 23},
        {20, 0, 1, 3, 10},
        {21, 1, 0, 2, 11},
        {22, 3, 2, 0, 4},
        {23, 10, 11, 4, 0},
    };
}

std::vector<std::vector<double>> sparseInternalPathMatrix()
{
    return {
        {0, 10, 11, 12, 13},
        {10, 0, 1, inf, inf},
        {11, 1, 0, 2, inf},
        {12, inf, 2, 0, 3},
        {13, inf, inf, 3, 0},
    };
}

void expectCost(double actual, double expected, const std::string& message)
{
    if (std::fabs(actual - expected) > 1e-9) {
        throw std::runtime_error(message);
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

tsp::SolveResult solveAndCompareWithBruteForce(
    const std::vector<std::vector<double>>& matrix,
    const std::string& case_name)
{
    const double expected = bruteForceOptimalCost(matrix);
    tsp::BranchBoundSolver solver(matrix);
    tsp::SolveResult result = solver.solve();

    if (!std::isfinite(expected)) {
        if (result.feasible || std::isfinite(result.cost)) {
            throw std::runtime_error(case_name + ": solver accepted an infeasible instance");
        }
        return result;
    }

    if (!result.feasible) {
        throw std::runtime_error(case_name + ": solver rejected a feasible instance");
    }
    expectCost(result.cost, expected, case_name + ": optimal cost differs from brute force");

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
               case_name + ": returned tour cost differs from the result cost");
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
    const auto mst_adjacency_before = fixture.tree.mst_adjacency_bits;
    fixture.forbidAndCompare(0, 1);
    expectCost(fixture.tree.cost, 14.0, "wrong root replacement cost");
    if (!fixture.contains(fixture.tree, 0, 3)) {
        throw std::runtime_error("third-lightest root edge was not selected");
    }
    if (fixture.tree.mst_adjacency_bits != mst_adjacency_before) {
        throw std::runtime_error("root replacement changed dynamic MST adjacency");
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

void testEarlierSkippedReplacementAndParentIsolation()
{
    Fixture fixture(earlierSkippedReplacementMatrix());
    expectCost(fixture.tree.cost, 48.0, "unexpected earlier-replacement initial cost");

    const auto parent_node = fixture.node;
    const auto parent_tree = fixture.tree;
    const auto expected_parent_tree = fixture.tree;

    fixture.forbidAndCompare(2, 3);
    expectCost(fixture.tree.cost, 49.0,
               "replacement before the old Kruskal cursor had the wrong cost");
    if (!fixture.contains(fixture.tree, 1, 3)
        || fixture.contains(fixture.tree, 2, 3)) {
        throw std::runtime_error(
            "replacement before the old Kruskal cursor was not selected");
    }
    fixture.expectSameTree(
        parent_tree, expected_parent_tree,
        "first dynamic MST replacement mutated its parent OneTree copy");

    fixture.forbidAndCompare(1, 3);
    expectCost(fixture.tree.cost, 56.0,
               "second earlier-replacement update had the wrong cost");
    if (!fixture.contains(fixture.tree, 1, 4)
        || fixture.contains(fixture.tree, 1, 3)) {
        throw std::runtime_error("second dynamic MST replacement selected the wrong edge");
    }
    fixture.expectSameTree(
        parent_tree, expected_parent_tree,
        "sequential dynamic MST replacements mutated their parent OneTree copy");

    // 模拟 DFS 回滚：恢复父节点约束和父树后，完整结构及动态邻接必须仍有效。
    fixture.node = parent_node;
    fixture.tree = parent_tree;
    fixture.validate(fixture.tree);
    expectCost(fixture.tree.cost, 48.0, "rollback did not restore the parent 1-tree");
}

void testForcedEdgeSurvivesBatchDynamicReplacement()
{
    Fixture fixture(earlierSkippedReplacementMatrix());
    const auto parent_tree = fixture.tree;
    const auto expected_parent_tree = fixture.tree;

    // 模拟 force 子节点：23 已在 prefix MST 中，变为 mandatory 后会从 candidate
    // mask 移除；同批另外两条树边失效。动态更新必须跳过 forced 23，只替换 12/34。
    fixture.force(2, 3);
    if (!fixture.deactivateBatchAndCompare({{2, 3}, {1, 2}, {3, 4}})) {
        throw std::runtime_error("forced batch replacement unexpectedly disconnected MST");
    }
    if (!fixture.contains(fixture.tree, 2, 3)
        || !fixture.contains(fixture.tree, 1, 3)
        || !fixture.contains(fixture.tree, 1, 4)) {
        throw std::runtime_error(
            "batch dynamic replacement removed the forced edge or chose wrong replacements");
    }
    expectCost(fixture.tree.cost, 56.0, "batch dynamic replacement cost is wrong");
    fixture.expectSameTree(
        parent_tree, expected_parent_tree,
        "batch dynamic replacement mutated its parent OneTree copy");
}

void testSparseDisconnectionLeavesParentUnchanged()
{
    Fixture fixture(sparseInternalPathMatrix());
    expectCost(fixture.tree.cost, 27.0, "unexpected sparse-path initial cost");

    const auto parent_node = fixture.node;
    const auto parent_tree = fixture.tree;
    const auto expected_parent_tree = fixture.tree;

    fixture.forbidAndCompare(2, 3);
    if (fixture.tree.feasible) {
        throw std::runtime_error("deleting the sparse path bridge should disconnect the MST");
    }
    fixture.expectSameTree(
        parent_tree, expected_parent_tree,
        "failed sparse replacement mutated its parent OneTree copy");

    fixture.node = parent_node;
    fixture.tree = parent_tree;
    fixture.validate(fixture.tree);
    expectCost(fixture.tree.cost, 27.0,
               "rollback after sparse disconnection did not restore the parent tree");
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
    const auto mst_adjacency_before = fixture.tree.mst_adjacency_bits;
    fixture.forbidAndCompare(0, 2);
    if (!fixture.contains(fixture.tree, 0, 1)
        || !fixture.contains(fixture.tree, 0, 3)) {
        throw std::runtime_error("root replacement did not preserve the forced root edge");
    }
    if (fixture.tree.mst_adjacency_bits != mst_adjacency_before) {
        throw std::runtime_error(
            "root replacement with a forced root edge changed MST adjacency");
    }
}

void testMultipleForcedInternalEdges()
{
    Fixture fixture(replacementMatrix());
    fixture.force(1, 2);
    fixture.force(2, 3);
    fixture.force(0, 1);
    fixture.rebuildAfterForces();
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

void testBpPrefixTreeRegression()
{
    // 旧实现对 B[1+] force 子节点错误复用原 current_tree，会遗漏该 B 边。
    // 完整图因此把 1920 错报为 2115，删边版本甚至会误报无解。
    const std::vector<std::vector<double>> complete = {
        {0, 585, 792, 891, 348, 768},
        {585, 0, 249, 83, 294, 778},
        {792, 249, 0, 14, 340, 834},
        {891, 83, 14, 0, 530, 399},
        {348, 294, 340, 530, 0, 325},
        {768, 778, 834, 399, 325, 0},
    };
    const tsp::SolveResult complete_result = solveAndCompareWithBruteForce(
        complete, "BP prefix-tree complete regression");
    expectCost(complete_result.cost, 1920.0,
               "BP prefix-tree complete regression returned the wrong optimum");

    const std::vector<std::vector<double>> sparse = {
        {0, 585, inf, inf, 348, inf},
        {585, 0, 249, 83, 294, inf},
        {inf, 249, 0, 14, inf, inf},
        {inf, 83, 14, 0, inf, 399},
        {348, 294, inf, inf, 0, 325},
        {inf, inf, inf, 399, 325, 0},
    };
    const tsp::SolveResult sparse_result = solveAndCompareWithBruteForce(
        sparse, "BP prefix-tree sparse regression");
    expectCost(sparse_result.cost, 1920.0,
               "BP prefix-tree sparse regression returned the wrong optimum");
}

void testRandomCompleteSolveAgainstBruteForce()
{
    std::mt19937 generator(2026071401);
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
            matrix, "complete random case " + std::to_string(case_index));
        exercised_recursive_search = exercised_recursive_search
            || result.stats.nodes_created > 1;
    }

    if (!exercised_recursive_search) {
        throw std::runtime_error("complete random cases did not exercise recursive BP search");
    }
}

void testRandomSparseSolveAgainstBruteForce()
{
    std::mt19937 generator(2026071402);
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
            matrix, "sparse random case " + std::to_string(case_index));
        exercised_recursive_search = exercised_recursive_search
            || result.stats.nodes_created > 1;
    }

    if (!exercised_recursive_search) {
        throw std::runtime_error("sparse random cases did not exercise recursive BP search");
    }
}

} // namespace

int main()
{
    try {
        testInternalReplacement();
        testRootReplacement();
        testNonTreeForbid();
        testSequentialForbids();
        testEarlierSkippedReplacementAndParentIsolation();
        testForcedEdgeSurvivesBatchDynamicReplacement();
        testSparseDisconnectionLeavesParentUnchanged();
        testMissingInternalReplacement();
        testMissingRootReplacement();
        testTiedReplacement();
        testMixedRootAndInternalForcedEdges();
        testOptionalRootReplacementWithForcedRootEdge();
        testMultipleForcedInternalEdges();
        testRandomSequentialForbids();
        testRandomSparseTiedForbids();
        testBpPrefixTreeRegression();
        testRandomCompleteSolveAgainstBruteForce();
        testRandomSparseSolveAgainstBruteForce();
    } catch (const std::exception& error) {
        std::cerr << "tsp_solver_tests failed: " << error.what() << '\n';
        return 1;
    }

    std::cout << "tsp_solver_tests passed\n";
    return 0;
}
