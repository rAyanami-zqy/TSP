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
    if (std::fabs(actual - expected) > 1e-9) {
        throw std::runtime_error(message);
    }
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
        testRandomSparseTiedForbids();
    } catch (const std::exception& error) {
        std::cerr << "tsp_solver_tests failed: " << error.what() << '\n';
        return 1;
    }

    std::cout << "tsp_solver_tests passed\n";
    return 0;
}
