/**
 * Test harness: compare C++ computeOneTree / buildBranchCandidates output
 * with Python simulation at a specific state.
 *
 * Build: cd build && g++ -std=c++17 -O2 -I../include ../tools/test_state.cpp ../src/TspSolver.cpp -o test_state
 */
#include "TspSolver.hpp"
#include <iostream>
#include <set>

int main() {
    // Load bayg29
    std::ifstream input("/home/wj/code/TSP/data/classic/tsplib/bayg29.tsp");
    auto problem = tsp::readTspProblem(input);
    int n = problem.dimension();
    auto dist = problem.toDenseMatrix();

    tsp::BranchBoundSolver solver(dist);
    solver.setDebugOutput(std::cout, 1);

    // Common edges between Concorde and B&B tours
    int common_pairs[][2] = {
        {0,27},{27,5},{5,11},{11,8},{25,2},{2,28},{20,1},{1,19},{19,9},{9,3},
        {3,14},{14,17},{17,13},{13,16},{16,21},{21,10},{10,18},{18,24},{24,6},{6,22},
        {22,7},{7,26},{26,15},{15,12},{12,23},{23,0}
    };
    int n_common = 26;

    // Build a custom Node with all 26 common edges forced + (4,8) forbidden
    std::size_t n2 = static_cast<std::size_t>(n) * static_cast<std::size_t>(n);
    std::vector<unsigned char> forced(n2, 0), forbidden(n2, 0);

    for (int i = 0; i < n_common; ++i) {
        int u = common_pairs[i][0], v = common_pairs[i][1];
        if (u > v) std::swap(u, v);
        forced[u * n + v] = 1;
    }
    // Forbid (4,8)
    forbidden[4 * n + 8] = 1;

    // Build all candidate edges
    std::vector<tsp::BranchBoundSolver::Edge> all_edges;
    for (int u = 0; u < n; ++u) {
        for (int v = u + 1; v < n; ++v) {
            if (std::isfinite(dist[u][v])) {
                all_edges.push_back({u, v, dist[u][v]});
            }
        }
    }
    std::cout << "Total edges: " << all_edges.size() << std::endl;

    // We need to call buildBranchCandidates directly.
    // We need a Node object and call the private methods.
    // Since they're private, we'll construct a Node manually and use
    // the public solve function with a modified approach.
    //
    // Instead, let's directly test computeOneTree by constructing
    // a Node and branch_candidates manually.

    // Build filtered candidates (simulating buildBranchCandidates output)
    // First compute forced_degree and components
    std::vector<int> forced_deg(n, 0);
    // DSU
    std::vector<int> parent(n), rank(n, 0);
    for (int i = 0; i < n; ++i) parent[i] = i;
    auto find = [&](int x) {
        while (parent[x] != x) { parent[x] = parent[parent[x]]; x = parent[x]; }
        return x;
    };
    auto unite = [&](int a, int b) -> bool {
        int ra = find(a), rb = find(b);
        if (ra == rb) return false;
        if (rank[ra] < rank[rb]) std::swap(ra, rb);
        parent[rb] = ra;
        if (rank[ra] == rank[rb]) ++rank[ra];
        return true;
    };

    for (int u = 0; u < n; ++u) {
        for (int v = u + 1; v < n; ++v) {
            if (forced[u * n + v]) {
                unite(u, v);
                forced_deg[u]++;
                forced_deg[v]++;
            }
        }
    }
    std::cout << "Forced edges: " << n_common << std::endl;

    // Count component sizes
    std::vector<int> comp_size(n, 0);
    for (int v = 0; v < n; ++v) comp_size[find(v)]++;

    // Filter candidates
    std::vector<tsp::BranchBoundSolver::Edge> kept, removed;
    for (const auto& e : all_edges) {
        int idx = e.u < e.v ? e.u * n + e.v : e.v * n + e.u;
        if (forced[idx] || forbidden[idx]) { removed.push_back(e); continue; }
        if (forced_deg[e.u] >= 2 || forced_deg[e.v] >= 2) { removed.push_back(e); continue; }
        if (e.u > 0 && e.v > 0) {
            int ru = find(e.u), rv = find(e.v);
            if (ru == rv && comp_size[ru] < n) { removed.push_back(e); continue; }
        }
        kept.push_back(e);
    }

    // Check available degree
    auto avail_deg = forced_deg;
    for (const auto& e : kept) { avail_deg[e.u]++; avail_deg[e.v]++; }
    bool feasible = true;
    for (int v = 0; v < n; ++v) {
        if (avail_deg[v] < 2) { feasible = false; break; }
    }

    std::cout << "Feasible: " << (feasible ? "yes" : "no") << std::endl;
    std::cout << "Candidates kept: " << kept.size() << ", removed: " << removed.size() << std::endl;

    // Print the Concorde/B&B edges status
    int key_edges[][2] = {{4,8},{4,25},{4,20},{4,28},{8,25},{20,28}};
    std::cout << "\nKey edge candidates:" << std::endl;
    for (auto [a,b] : key_edges) {
        bool in_kept = false;
        for (const auto& e : kept) {
            if ((e.u == a && e.v == b) || (e.u == b && e.v == a)) { in_kept = true; break; }
        }
        bool in_removed = false;
        for (const auto& e : removed) {
            if ((e.u == a && e.v == b) || (e.u == b && e.v == a)) { in_removed = true; break; }
        }
        std::cout << "  (" << a << "," << b << ") w=" << dist[a][b]
                  << " kept=" << (in_kept ? "yes" : "no")
                  << " removed=" << (in_removed ? "yes" : "no")
                  << std::endl;
    }

    // Now simulate computeOneTree
    if (!feasible || kept.empty()) {
        std::cout << "\nINFEA SIBLE — cannot compute 1-tree" << std::endl;
        return 0;
    }

    // Build MST on {1..n-1}
    // Reset DSU
    for (int i = 0; i < n; ++i) { parent[i] = i; rank[i] = 0; }
    double mst_cost = 0;
    int mst_edges = 0;

    for (int u = 1; u < n; ++u) {
        for (int v = u + 1; v < n; ++v) {
            if (forced[u * n + v]) {
                unite(u, v);
                mst_cost += dist[u][v];
                mst_edges++;
            }
        }
    }
    std::cout << "\nMST forced edges (1..n-1): " << mst_edges << std::endl;

    // Sort candidates by weight
    std::sort(kept.begin(), kept.end(), [](const auto& a, const auto& b) { return a.w < b.w; });

    // Kruskal
    for (const auto& e : kept) {
        if (mst_edges == n - 2) break;
        if (e.u == 0 || e.v == 0) continue;
        if (unite(e.u, e.v)) {
            mst_cost += e.w;
            mst_edges++;
        }
    }
    std::cout << "MST total edges: " << mst_edges << " (need " << (n-2) << ")" << std::endl;
    std::cout << "MST cost: " << mst_cost << std::endl;

    if (mst_edges != n - 2) {
        std::cout << "\n1-TREE INFEA SIBLE (MST incomplete)" << std::endl;
        return 0;
    }

    // Root edges
    double root_cost = 0;
    int root_count = 0;
    for (int v = 1; v < n; ++v) {
        if (forced[0 * n + v]) {
            root_cost += dist[0][v];
            root_count++;
        }
    }
    // Add cheapest candidates to vertex 0
    std::vector<tsp::BranchBoundSolver::Edge> root_cands;
    for (const auto& e : kept) {
        if (e.u == 0 || e.v == 0) root_cands.push_back(e);
    }
    std::sort(root_cands.begin(), root_cands.end(), [](const auto& a, const auto& b) { return a.w < b.w; });
    for (const auto& e : root_cands) {
        if (root_count == 2) break;
        root_cost += e.w;
        root_count++;
    }
    std::cout << "Root edges: " << root_count << " cost=" << root_cost << std::endl;

    if (root_count != 2) {
        std::cout << "\n1-TREE INFEA SIBLE (not enough root edges)" << std::endl;
        return 0;
    }

    double total = mst_cost + root_cost;
    std::cout << "\n1-TREE BOUND: " << total << std::endl;
    std::cout << "Expected (Python): 1592" << std::endl;

    return 0;
}
