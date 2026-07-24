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
        testDiversifiedInitialTourPool();
        testRootReducedCostFixing();
    } catch (const std::exception& error) {
        std::cerr << "tsp_solver_tests failed: " << error.what() << '\n';
        return 1;
    }

    std::cout << "tsp_solver_tests passed\n";
    return 0;
}
