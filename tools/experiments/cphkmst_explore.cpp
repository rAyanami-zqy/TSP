// Standalone research driver; Concorde remains an optional research dependency.
// Build/run instructions and experiment boundaries are in README.md beside this file.
#include "TspSolver.hpp"
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>
extern "C" {
#include "machdefs.h"
#include "util.h"
#include "edgegen.h"
#include "linkern.h"
#include "kdtree.h"
}

namespace tsp {
struct BranchBoundSolverTestAccess {
    using Solver = BranchBoundSolver;
    using Edge = Solver::Edge;
    using Node = Solver::PartialSol;
    using Tree = Solver::OneTree;
    using Graph = std::vector<std::vector<int>>;
    using Clock = std::chrono::steady_clock;
    static double seconds(Clock::time_point t) {
        return std::chrono::duration<double>(Clock::now() - t).count();
    }
    static Graph nearest(Solver& s, int k) {
        Graph g(s.n_);
        for (int u = 0; u < s.n_; ++u) {
            for (int v = 0; v < s.n_; ++v)
                if (u != v && std::isfinite(s.dist_[u][v])) g[u].push_back(v);
            std::sort(g[u].begin(), g[u].end(), [&](int a, int b) {
                return s.dist_[u][a] != s.dist_[u][b]
                    ? s.dist_[u][a] < s.dist_[u][b] : a < b;
            });
            if (g[u].size() > static_cast<std::size_t>(k)) g[u].resize(k);
        }
        return g;
    }
    static void normalize(Solver& s, Graph& g) {
        for (int u = 0; u < s.n_; ++u) {
            auto& row = g[u];
            std::sort(row.begin(), row.end());
            row.erase(std::unique(row.begin(), row.end()), row.end());
            std::sort(row.begin(), row.end(), [&](int a, int b) {
                return s.dist_[u][a] != s.dist_[u][b]
                    ? s.dist_[u][a] < s.dist_[u][b] : a < b;
            });
        }
    }
    static void merge(Solver& s, Graph& a, const Graph& b) {
        for (int u = 0; u < s.n_; ++u) a[u].insert(a[u].end(), b[u].begin(), b[u].end());
        normalize(s, a);
    }
    static std::vector<unsigned char> mask(Solver& s, const Graph& g) {
        std::vector<unsigned char> bits(s.n_ * s.n_, 0);
        for (int u = 0; u < s.n_; ++u)
            for (int v : g[u]) bits[s.edgeId(u, v)] = 1;
        return bits;
    }
    static Graph generated(Solver& s, CCdatagroup& dat, CCrandstate& rng,
                           const std::string& mode) {
        CCedgegengroup plan;
        CCedgegen_init_edgegengroup(&plan);
        if (mode == "quad2") plan.quadnearest = 2;
        else if (mode == "delaunay") plan.delaunay = 1;
        else if (mode == "union") {
            plan.linkern.count = 10;
            plan.linkern.quadnearest = 2;
            plan.linkern.nkicks = s.n_ / 100 + 1;
        } else if (mode == "kd8") plan.nearest = 8;
        else throw std::invalid_argument("unknown generation mode");
        int m = 0;
        int* edges = nullptr;
        if (CCedgegen_edges(&plan, s.n_, &dat, nullptr, &m, &edges, 1, &rng)) {
            std::free(edges);
            throw std::runtime_error("Concorde edge generation failed");
        }
        Graph g(s.n_);
        for (int i = 0; i < m; ++i) {
            g[edges[2*i]].push_back(edges[2*i+1]);
            g[edges[2*i+1]].push_back(edges[2*i]);
        }
        std::free(edges);
        normalize(s, g);
        return g;
    }
    static std::vector<int> linkern(Solver& s, CCdatagroup& dat, Graph& g,
                                   CCrandstate& rng, int starts, int kicks, int kicktype) {
        auto bits = mask(s, g);
        std::vector<int> edges;
        for (int u = 0; u < s.n_; ++u)
            for (int v = u+1; v < s.n_; ++v)
                if (bits[s.edgeId(u,v)]) { edges.push_back(u); edges.push_back(v); }
        std::vector<int> best, trial(s.n_);
        double best_cost = INFINITY;
        for (int r = 0; r < starts; ++r) {
            double cost = 0;
            if (CClinkern_tour(s.n_, &dat, static_cast<int>(edges.size()/2),
                edges.data(), 100000000, kicks, nullptr, trial.data(), &cost, 1,
                -1.0, -1.0, nullptr, kicktype, &rng))
                throw std::runtime_error("Linkern failed");
            // Never trust a foreign library's claimed cost/permutation.
            s.setInitialTour(trial);
            const double checked = s.tourCost(trial);
            if (checked != cost) throw std::runtime_error("Linkern distance mismatch");
            if (checked < best_cost) { best = trial; best_cost = checked; }
        }
        s.setInitialTour(best);
        return best;
    }
    static Node emptyNode(Solver& s) {
        Node node;
        node.forced.assign(s.n_*s.n_, 0);
        node.forbidden.assign(s.n_*s.n_, 0);
        node.forced_degree.assign(s.n_, 0);
        node.forced_parent.resize(s.n_);
        std::iota(node.forced_parent.begin(), node.forced_parent.end(), 0);
        node.forced_rank.assign(s.n_, 0);
        node.forced_comp_size.assign(s.n_, 1);
        return node;
    }
    static bool edgeLess(const Edge& a, const Edge& b) {
        if (a.w != b.w) return a.w < b.w;
        if (a.u != b.u) return a.u < b.u;
        return a.v < b.v;
    }
    static std::vector<Edge> allEdges(Solver& s) {
        std::vector<Edge> edges;
        s.root_candidates_sorted_.clear();
        for (int u = 0; u < s.n_; ++u) for (int v = u+1; v < s.n_; ++v) {
            if (!std::isfinite(s.dist_[u][v])) continue;
            Edge e{u,v,s.adjustedEdgeWeight(u,v)};
            edges.push_back(e);
            if (u==0) s.root_candidates_sorted_.push_back(e);
        }
        std::sort(edges.begin(), edges.end(), edgeLess);
        std::sort(s.root_candidates_sorted_.begin(), s.root_candidates_sorted_.end(), edgeLess);
        return edges;
    }
    // Signed exchange deltas, unlike alpha-nearness which clamps negatives.
    // A negative delta is a pricing violation for the *current* fixed potentials.
    static std::vector<double> support(Solver& s, const Node& node, const Tree& tree) {
        std::vector<std::vector<std::pair<int,double>>> adj(s.n_);
        std::vector<double> beta(s.n_*s.n_, -INFINITY);
        double root_max = -INFINITY;
        for (const auto& e : tree.edges) {
            double w = node.forced[s.edgeId(e.u,e.v)] ? -INFINITY : e.w;
            if (e.u==0) { root_max = std::max(root_max,w); continue; }
            adj[e.u].push_back({e.v,w}); adj[e.v].push_back({e.u,w});
        }
        for (int u=1; u<s.n_; ++u) {
            std::vector<int> parent(s.n_,-1), stack{u}; parent[u]=u;
            while (!stack.empty()) {
                int v=stack.back(); stack.pop_back();
                for (auto [w,c] : adj[v]) if (parent[w]<0) {
                    parent[w]=v; stack.push_back(w);
                    beta[u*s.n_+w]=std::max(beta[u*s.n_+v],c);
                }
            }
            beta[u]=root_max;
        }
        return beta;
    }
    static void pricingAudit(Solver& s, const Graph& graph, const std::vector<int>& tour,
                             const std::string& instance, int trials) {
        std::vector<Solver::TourCandidate> alternatives;
        std::vector<int> ubtour = tour;
        double ub = INFINITY;
        if (ubtour.empty()) s.findInitialTour(ubtour,ub,alternatives);
        ub = s.tourCost(ubtour);
        s.optimizeRootPotentials(ub);
        auto rootpi = s.vertex_potential_;
        std::mt19937 rng(91723);
        int added=0, rounds=0, fallbacks=0, bad_sparse=0, forced_tests=0;
        double full_seconds=0, sparse_seconds=0;
        for (int t=0; t<trials; ++t) {
            s.vertex_potential_=rootpi;
            if (t) for (auto& p : s.vertex_potential_) p += (int(rng()%201)-100)*0.03;
            s.potential_correction_=2*std::accumulate(s.vertex_potential_.begin(),s.vertex_potential_.end(),0.0);
            auto all=allEdges(s);
            auto node=emptyNode(s);
            auto allowed=std::vector<unsigned char>(s.n_*s.n_,0);
            for (const auto& e : all) allowed[s.edgeId(e.u,e.v)]=1;
            // Non-root tour path edges provide acyclic forced subsets; forbid
            // only non-tour edges, so the generated constraints stay feasible.
            std::vector<unsigned char> tourmask(s.n_*s.n_,0);
            for (int i=0;i<s.n_;++i) tourmask[s.edgeId(ubtour[i],ubtour[(i+1)%s.n_])]=1;
            for (const auto& e : all) {
                auto id=s.edgeId(e.u,e.v);
                if (t && tourmask[id] && e.u!=0 && rng()%5==0) {
                    node.forced[id]=1; node.forced_edges.push_back(e);
                    node.forced_mst_cost+=e.w; ++node.forced_mst_count; ++forced_tests;
                    ++node.forced_degree[e.u]; ++node.forced_degree[e.v];
                } else if (t && !tourmask[id] && rng()%7==0) {
                    node.forbidden[id]=1; allowed[id]=0;
                }
            }
            node.candidate_mask=allowed;
            auto start=Clock::now();
            auto exact=s.computeOneTree(node,all);
            full_seconds+=seconds(start);
            if (!exact.feasible) throw std::runtime_error("audit full tree infeasible");
            auto active=mask(s,graph);
            for (int i=0;i<s.n_;++i) active[s.edgeId(ubtour[i],ubtour[(i+1)%s.n_])]=1;
            for (std::size_t i=0;i<active.size();++i) active[i] &= allowed[i];
            start=Clock::now();
            Tree tree;
            for (int r=0;;++r) {
                ++rounds;
                std::vector<Edge> sparse;
                for (const auto& e : all) if (active[s.edgeId(e.u,e.v)]) sparse.push_back(e);
                node.candidate_mask=active;
                tree=s.computeOneTree(node,sparse);
                if (!tree.feasible) { active=allowed; ++fallbacks; continue; }
                if (r==0 && tree.cost>exact.cost+1e-7) ++bad_sparse;
                auto beta=support(s,node,tree);
                int violations=0;
                for (const auto& e : all) {
                    auto id=s.edgeId(e.u,e.v);
                    if (!allowed[id] || active[id] || node.forced[id]) continue;
                    // Equal-weight edges are also admitted: they can change the
                    // canonical tree / subgradient even when the cost is equal.
                    if (e.w <= beta[e.u*s.n_+e.v]) { active[id]=1; ++violations; }
                }
                added+=violations;
                if (!violations) break;
            }
            sparse_seconds+=seconds(start);
            if (std::fabs(tree.cost-exact.cost)>1e-7 || tree.degree!=exact.degree)
                throw std::runtime_error("pricing certificate differs from full rebuild");
        }
        std::cout<<"RESULT {\"kind\":\"pricing\",\"instance\":"<<std::quoted(instance)
            <<",\"trials\":"<<trials<<",\"rounds\":"<<rounds<<",\"added\":"<<added
            <<",\"fallbacks\":"<<fallbacks<<",\"uncertified_overestimates\":"<<bad_sparse
            <<",\"forced_tests\":"<<forced_tests<<",\"full_seconds\":"<<full_seconds
            <<",\"pricing_seconds\":"<<sparse_seconds<<"}\n";
    }
    static void run(const std::string& instance, const std::string& mode, bool rootonly, int seed) {
        std::ifstream input(instance);
        if (!input) throw std::runtime_error("cannot open instance");
        auto problem=readTspProblem(input);
        Solver s(problem.toDenseMatrix(2000));
        s.setPotentialUpdateOptions(PotentialUpdateStrategy::SubtreeAdaptive,1,32,0);
        s.setRootBoundOnly(rootonly);
        CCdatagroup dat;
        CCutil_init_datagroup(&dat);
        struct Cleanup { CCdatagroup* p; ~Cleanup(){CCutil_freedatagroup(p);} } cleanup{&dat};
        CCrandstate rng; CCutil_sprand(seed,&rng);
        int distance_mismatches=0;
        if (mode=="delaunay" && problem.edge_weight_type!="EUC_2D") {
            std::cout<<"RESULT {\"kind\":\"solve\",\"mode\":\"delaunay\",\"instance\":"
                <<std::quoted(instance)<<",\"status\":\"not_applicable\"}\n"; return;
        }
        const bool external=mode=="quad2" || mode=="delaunay" || mode=="union"
            || mode=="kd8" || mode.rfind("clk",0)==0;
        if (external) {
            int n=0;
            if (CCutil_gettsplib(const_cast<char*>(instance.c_str()),&n,&dat) || n!=s.n_)
                throw std::runtime_error("Concorde input failed");
            for (int u=0;u<s.n_;++u) for (int v=u+1;v<s.n_;++v)
                if (CCutil_dat_edgelen(u,v,&dat)!=s.dist_[u][v])
                    ++distance_mismatches;
            if (distance_mismatches) {
                CCutil_freedatagroup(&dat);
                CCutil_init_datagroup(&dat);
                if (CCutil_dat_setnorm(&dat,CC_MATRIXNORM)) throw std::runtime_error("matrix norm failed");
                dat.adj=CC_SAFE_MALLOC(s.n_,int*);
                dat.adjspace=CC_SAFE_MALLOC(s.n_*(s.n_+1)/2,int);
                if (!dat.adj || !dat.adjspace) throw std::bad_alloc();
                int offset=0;
                for (int u=0;u<s.n_;++u) {
                    dat.adj[u]=dat.adjspace+offset; offset+=u+1;
                    for (int v=0;v<=u;++v) {
                        if (s.dist_[u][v]<0 || s.dist_[u][v]>10000000 || std::floor(s.dist_[u][v])!=s.dist_[u][v])
                            throw std::runtime_error("matrix outside research Linkern integer range");
                        dat.adj[u][v]=static_cast<int>(s.dist_[u][v]);
                    }
                }
            }
        }
        auto start=Clock::now();
        Graph g=nearest(s,8);
        std::vector<int> supplied;
        if (mode=="knn12" || mode=="knn20") g=nearest(s,mode=="knn12"?12:20);
        else if (mode=="quad2" || mode=="delaunay" || mode=="union" || mode=="kd8") {
            auto extra=generated(s,dat,rng,mode);
            if (mode=="kd8") g=extra;
            else merge(s,g,extra);
        } else if (mode.rfind("clk",0)==0) {
            auto cg=mode=="clk-knn" ? nearest(s,8) : generated(s,dat,rng,"quad2");
            int starts=mode=="clk-multi" ? 3 : 1;
            int kicktype=mode=="clk-walk" ? CC_LK_WALK_KICK : CC_LK_GEOMETRIC_KICK;
            int kicks=mode=="clk-zero" ? 0 : 10;
            supplied=linkern(s,dat,cg,rng,starts,kicks,kicktype);
        } else if (mode!="baseline" && mode!="pricing") throw std::invalid_argument("unknown mode");
        if (mode=="pricing") { pricingAudit(s,g,{},instance,24); return; }
        if (mode!="baseline" && mode.rfind("clk",0)!=0) {
            s.buildCandidateSets();
            s.candidate_set_=g;
            for (int u=0;u<s.n_;++u) for (int v:g[u]) {
                auto id=s.edgeId(u,v);
                if (!s.candidate_hint_edges_[id]) {
                    s.candidate_hint_edges_[id]=1;
                    s.candidate_hint_neighbors_[u].push_back(v);
                    s.candidate_hint_neighbors_[v].push_back(u);
                }
            }
        }
        double generation=seconds(start);
        auto result=s.solve();
        double total=seconds(start);
        if (!result.feasible || result.cost!=s.tourCost(result.tour))
            throw std::runtime_error("invalid returned tour");
        auto bits=mask(s,g);
        int missing=0;
        for (int i=0;i<s.n_;++i) if (!bits[s.edgeId(result.tour[i],result.tour[(i+1)%s.n_])]) ++missing;
        std::cout<<"RESULT {\"kind\":\"solve\",\"mode\":"<<std::quoted(mode)
            <<",\"instance\":"<<std::quoted(instance)<<",\"status\":\"ok\",\"seed\":"<<seed
            <<",\"n\":"<<s.n_<<",\"root_only\":"<<(rootonly?"true":"false")
            <<",\"native_distance_mismatches\":"<<distance_mismatches
            <<",\"cost\":"<<result.cost<<",\"ub\":"<<result.stats.initial_upper_bound
            <<",\"lb\":"<<result.stats.root_lower_bound<<",\"nodes\":"<<result.stats.nodes_created
            <<",\"expanded\":"<<result.stats.nodes_expanded
            <<",\"iterations\":"<<result.stats.search_node_potential_iterations
            <<",\"fixed_zero\":"<<result.stats.root_fixing_fixed_zero
            <<",\"fixed_one\":"<<result.stats.root_fixing_fixed_one
            <<",\"active\":"<<result.stats.root_fixing_active_after
            <<",\"graph_edges\":"<<std::count(bits.begin(),bits.end(),1)
            <<",\"returned_tour_edges_outside_graph\":"<<missing
            <<",\"generation_seconds\":"<<generation<<",\"seconds\":"<<total
            <<",\"tour\":[";
        for (int i=0;i<s.n_;++i) { if(i) std::cout<<','; std::cout<<result.tour[i]; }
        std::cout<<"]}\n";
    }
};
}

int main(int argc,char** argv) {
    try {
        if(argc<3) throw std::invalid_argument("usage: cphkmst_explore MODE INSTANCE [root] [seed]");
        std::cout<<std::setprecision(17);
        tsp::BranchBoundSolverTestAccess::run(argv[2],argv[1],argc>3 && std::string(argv[3])=="root",
                                            argc>4?std::stoi(argv[4]):1);
        return 0;
    } catch(const std::exception& e) { std::cerr<<e.what()<<'\n'; return 1; }
}
