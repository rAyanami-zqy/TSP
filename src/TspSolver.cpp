#include "TspSolver.hpp"

#include <algorithm>
#include <cmath>
#include <cctype>
#include <iomanip>
#include <limits>
#include <numeric>
#include <queue>
#include <sstream>
#include <stdexcept>
#include <string>

namespace tsp {
namespace {

// 浮点数比较容差，用于处理 double 计算中的微小误差。
constexpr double kEps = 1e-9;

// 统一判断边权是否可用。缺边在输入中会被解析成 infinity。
bool isFinite(double value)
{
    return std::isfinite(value);
}

// 并查集用于 Kruskal 构造 MST，同时检测强制边是否已形成非法环。
class DisjointSet {
public:
    explicit DisjointSet(int n)
        : parent_(n), rank_(n, 0)
    {
        // 初始时每个顶点各自属于一个集合。
        std::iota(parent_.begin(), parent_.end(), 0);
    }

    int find(int x)
    {
        // 路径压缩：查找根节点时顺便把路径上的节点直接挂到根上。
        if (parent_[x] != x) {
            parent_[x] = find(parent_[x]);
        }
        return parent_[x];
    }

    bool unite(int a, int b)
    {
        int root_a = find(a);
        int root_b = find(b);
        // 两个端点已经在同一连通分量中，再加入这条边会形成环。
        if (root_a == root_b) {
            return false;
        }
        // 按秩合并，避免并查集退化成链。
        if (rank_[root_a] < rank_[root_b]) {
            std::swap(root_a, root_b);
        }
        parent_[root_b] = root_a;
        if (rank_[root_a] == rank_[root_b]) {
            ++rank_[root_a];
        }
        return true;
    }

private:
    std::vector<int> parent_;
    std::vector<int> rank_;
};

double parseWeight(const std::string& token)
{
    // 先转小写，兼容 INF / Infinity 等大小写写法。
    std::string lowered;
    lowered.reserve(token.size());
    for (char ch : token) {
        lowered.push_back(static_cast<char>(std::tolower(static_cast<unsigned char>(ch))));
    }

    if (lowered == "inf" || lowered == "infinity" || lowered == "-" || lowered == "x") {
        return std::numeric_limits<double>::infinity();
    }

    // stod 允许部分解析，因此需要检查 parsed 是否刚好到字符串末尾。
    std::size_t parsed = 0;
    double value = std::stod(token, &parsed);
    if (parsed != token.size()) {
        throw std::runtime_error("invalid numeric token: " + token);
    }
    return value;
}

bool sameWeight(double a, double b)
{
    // infinity 只能和 infinity 视为相等；有限值用容差比较。
    if (!isFinite(a) || !isFinite(b)) {
        return !isFinite(a) && !isFinite(b);
    }
    return std::fabs(a - b) <= kEps;
}

} // namespace

BranchBoundSolver::BranchBoundSolver(std::vector<std::vector<double>> distance)
    : n_(static_cast<int>(distance.size())), dist_(std::move(distance))
{
    // TSP 回路至少需要 3 个顶点。
    if (n_ < 3) {
        throw std::runtime_error("TSP instance must contain at least 3 vertices");
    }
    for (int i = 0; i < n_; ++i) {
        // 距离矩阵必须是 n*n 方阵。
        if (static_cast<int>(dist_[i].size()) != n_) {
            throw std::runtime_error("distance matrix must be square");
        }
        for (int j = 0; j < n_; ++j) {
            // 对角线元素不参与求解，允许输入中写 0。
            if (i == j) {
                continue;
            }
            // 当前算法假设非负边权，负边会破坏下界语义。
            if (isFinite(dist_[i][j]) && dist_[i][j] < 0.0) {
                throw std::runtime_error("distances must be non-negative");
            }
        }
    }

    // 当前求解器按无向边分支，因此要求输入为对称 TSP。
    for (int i = 0; i < n_; ++i) {
        for (int j = i + 1; j < n_; ++j) {
            if (!sameWeight(dist_[i][j], dist_[j][i])) {
                throw std::runtime_error("this solver expects a symmetric distance matrix");
            }
        }
    }
}

SolveResult BranchBoundSolver::solve()
{
    // 为了用 O(1) 标记任意无向边状态，直接使用 n*n 大小的一维数组。
    const std::size_t edge_state_size = static_cast<std::size_t>(n_) * static_cast<std::size_t>(n_);

    SolveResult result;
    std::vector<int> best_tour;
    double best_cost = std::numeric_limits<double>::infinity();

    // 先用启发式得到一个上界。上界越小，后续 bound 剪枝越有效。
    if (findInitialTour(best_tour, best_cost)) {
        result.stats.initial_upper_bound = best_cost;
    } else {
        result.stats.initial_upper_bound = std::numeric_limits<double>::infinity();
    }

    Node root;
    // 根节点没有任何强制边或禁止边。
    root.forced.assign(edge_state_size, 0);
    root.forbidden.assign(edge_state_size, 0);

    OneTree root_tree = computeOneTree(root.forced, root.forbidden);
    if (!root_tree.feasible) {
        // 根节点都无法构造 1-tree，说明实例无法形成 Hamilton 回路。
        result.feasible = false;
        result.cost = std::numeric_limits<double>::infinity();
        result.stats.root_lower_bound = std::numeric_limits<double>::infinity();
        return result;
    }

    result.stats.root_lower_bound = root_tree.cost;
    root.bound = root_tree.cost;
    root.one_tree_edges = std::move(root_tree.edges);
    root.degree = std::move(root_tree.degree);
    result.stats.nodes_created = 1;

    struct QueueItem {
        double bound = 0.0;
        std::size_t serial = 0;
        Node node;
    };

    struct CompareQueueItem {
        bool operator()(const QueueItem& lhs, const QueueItem& rhs) const
        {
            // priority_queue 默认取“最大”，这里反向比较以实现最小下界优先。
            if (std::fabs(lhs.bound - rhs.bound) > kEps) {
                return lhs.bound > rhs.bound;
            }
            return lhs.serial > rhs.serial;
        }
    };

    std::priority_queue<QueueItem, std::vector<QueueItem>, CompareQueueItem> queue;
    std::size_t serial = 0;
    queue.push(QueueItem{root.bound, serial++, std::move(root)});

    // Best-first 分支定界：每次扩展当前下界最小的节点。
    while (!queue.empty()) {
        Node node = std::move(queue.top().node);
        queue.pop();

        // 当前节点的下界已经不优于已知最优可行解，可以直接剪枝。
        if (shouldPrune(node.bound, best_cost)) {
            ++result.stats.nodes_pruned_by_bound;
            continue;
        }

        ++result.stats.nodes_expanded;

        OneTree current_tree;
        current_tree.feasible = true;
        current_tree.cost = node.bound;
        current_tree.edges = node.one_tree_edges;
        current_tree.degree = node.degree;

        // 1-tree 若所有顶点度数都是 2，就已经是一条合法 TSP 回路。
        if (isTour(current_tree)) {
            // 将边集合转成按访问顺序排列的顶点序列。
            std::vector<int> candidate = buildTour(current_tree.edges);
            if (!candidate.empty() && current_tree.cost + kEps < best_cost) {
                best_cost = current_tree.cost;
                best_tour = std::move(candidate);
            }
            continue;
        }

        Edge branch_edge;
        // 选择一条 1-tree 中的边 e，生成两个子问题：e 必选 / e 禁用。
        if (!chooseBranchEdge(node, branch_edge)) {
            continue;
        }

        for (bool include_edge : {true, false}) {
            Node child;
            child.depth = node.depth + 1;
            // 子节点继承父节点已有约束，再叠加本次分支产生的新约束。
            child.forced = node.forced;
            child.forbidden = node.forbidden;

            const std::size_t id = edgeId(branch_edge.u, branch_edge.v);
            if (include_edge) {
                // 左分支：要求解必须包含 branch_edge。
                child.forced[id] = 1;
            } else {
                // 右分支：要求解不能包含 branch_edge。
                child.forbidden[id] = 1;
            }

            // 子节点重新计算受约束 1-tree；构造失败表示该分支不可行。
            OneTree child_tree = computeOneTree(child.forced, child.forbidden);
            if (!child_tree.feasible) {
                ++result.stats.nodes_pruned_infeasible;
                continue;
            }

            child.bound = child_tree.cost;
            child.one_tree_edges = std::move(child_tree.edges);
            child.degree = std::move(child_tree.degree);
            ++result.stats.nodes_created;

            // 如果子节点的下界已经不可能改进当前最优解，立即剪掉。
            if (shouldPrune(child.bound, best_cost)) {
                ++result.stats.nodes_pruned_by_bound;
                continue;
            }

            OneTree check_tree;
            check_tree.feasible = true;
            check_tree.cost = child.bound;
            check_tree.edges = child.one_tree_edges;
            check_tree.degree = child.degree;
            // 子节点刚算出的 1-tree 可能已经是一条更好的完整回路。
            if (isTour(check_tree)) {
                std::vector<int> candidate = buildTour(check_tree.edges);
                if (!candidate.empty() && child.bound + kEps < best_cost) {
                    best_cost = child.bound;
                    best_tour = std::move(candidate);
                }
                continue;
            }

            // 既没有被剪枝，也还不是完整回路，进入优先队列等待后续扩展。
            queue.push(QueueItem{child.bound, serial++, std::move(child)});
        }
    }

    // 搜索结束后，best_cost 有限且 best_tour 非空才算找到可行最优解。
    if (isFinite(best_cost) && !best_tour.empty()) {
        result.feasible = true;
        result.cost = best_cost;
        result.tour = std::move(best_tour);
    } else {
        result.feasible = false;
        result.cost = std::numeric_limits<double>::infinity();
    }
    return result;
}

std::size_t BranchBoundSolver::edgeId(int u, int v) const
{
    // 无向边 (u,v) 和 (v,u) 应映射到同一个状态位置。
    if (u > v) {
        std::swap(u, v);
    }
    return static_cast<std::size_t>(u) * static_cast<std::size_t>(n_) + static_cast<std::size_t>(v);
}

bool BranchBoundSolver::isForced(const std::vector<unsigned char>& forced, int u, int v) const
{
    // forced 数组中非 0 表示该边在当前分支节点中必须被选择。
    return forced[edgeId(u, v)] != 0;
}

bool BranchBoundSolver::isForbidden(const std::vector<unsigned char>& forbidden, int u, int v) const
{
    // forbidden 数组中非 0 表示该边在当前分支节点中禁止被选择。
    return forbidden[edgeId(u, v)] != 0;
}

BranchBoundSolver::OneTree BranchBoundSolver::computeOneTree(
    const std::vector<unsigned char>& forced,
    const std::vector<unsigned char>& forbidden) const
{
    OneTree result;
    result.degree.assign(n_, 0);

    std::vector<int> forced_degree(n_, 0);
    DisjointSet forced_components(n_);
    std::vector<int> component_vertices(n_, 1);
    std::vector<int> component_edges(n_, 0);

    // 先检查 forced / forbidden 是否互相冲突，以及强制边是否让某点度数超过 2。
    for (int u = 0; u < n_; ++u) {
        for (int v = u + 1; v < n_; ++v) {
            const std::size_t id = edgeId(u, v);
            // 同一条边不能既强制选择又禁止选择。
            if (forced[id] && forbidden[id]) {
                return result;
            }
            if (!forced[id]) {
                continue;
            }
            // 强制选择一条缺边，当前分支必然不可行。
            if (!isFinite(dist_[u][v])) {
                return result;
            }
            ++forced_degree[u];
            ++forced_degree[v];
            if (forced_degree[u] > 2 || forced_degree[v] > 2) {
                return result;
            }
        }
    }

    // 强制边如果提前形成不包含全部顶点的环，则不可能扩展成 Hamilton 回路。
    for (int u = 0; u < n_; ++u) {
        for (int v = u + 1; v < n_; ++v) {
            if (!isForced(forced, u, v)) {
                continue;
            }
            // 这里只建立强制边形成的连通分量，后续统计每个分量的边数和点数。
            int root_u = forced_components.find(u);
            int root_v = forced_components.find(v);
            if (root_u != root_v) {
                forced_components.unite(root_u, root_v);
            }
        }
    }

    std::fill(component_vertices.begin(), component_vertices.end(), 0);
    std::fill(component_edges.begin(), component_edges.end(), 0);
    // component_vertices[c] 表示强制边分量 c 中有多少个顶点。
    for (int v = 0; v < n_; ++v) {
        ++component_vertices[forced_components.find(v)];
    }
    // component_edges[c] 表示强制边分量 c 中已经有多少条强制边。
    for (int u = 0; u < n_; ++u) {
        for (int v = u + 1; v < n_; ++v) {
            if (isForced(forced, u, v)) {
                ++component_edges[forced_components.find(u)];
            }
        }
    }
    for (int c = 0; c < n_; ++c) {
        // 在一个真子集上边数等于点数，说明强制边已经闭成子回路。
        if (component_vertices[c] > 0
            && component_edges[c] == component_vertices[c]
            && component_vertices[c] < n_) {
            return result;
        }
    }

    DisjointSet tree_components(n_);
    int mst_edges = 0;
    double cost = 0.0;
    std::vector<Edge> edges;
    edges.reserve(static_cast<std::size_t>(n_));

    // 1-tree 的第一部分：在顶点 1..n-1 上构造受约束 MST。
    // 先放入非 0 顶点之间的强制边，再用 Kruskal 补齐。
    for (int u = 1; u < n_; ++u) {
        for (int v = u + 1; v < n_; ++v) {
            if (!isForced(forced, u, v)) {
                continue;
            }
            // MST 部分不能含环；强制边若已经成环，此分支不可行。
            if (!tree_components.unite(u, v)) {
                return result;
            }
            edges.push_back(Edge{u, v, dist_[u][v]});
            cost += dist_[u][v];
            ++mst_edges;
        }
    }

    std::vector<Edge> mst_candidates;
    // 收集所有可用于 MST 补齐的候选边，排除已强制、已禁止和缺边。
    for (int u = 1; u < n_; ++u) {
        for (int v = u + 1; v < n_; ++v) {
            if (isForced(forced, u, v) || isForbidden(forbidden, u, v) || !isFinite(dist_[u][v])) {
                continue;
            }
            mst_candidates.push_back(Edge{u, v, dist_[u][v]});
        }
    }
    std::sort(mst_candidates.begin(), mst_candidates.end(), [](const Edge& lhs, const Edge& rhs) {
        return lhs.w < rhs.w;
    });

    // Kruskal：按权重从小到大加入不会成环的边，直到 MST 有 n-2 条边。
    for (const Edge& edge : mst_candidates) {
        if (mst_edges == n_ - 2) {
            break;
        }
        if (tree_components.unite(edge.u, edge.v)) {
            edges.push_back(edge);
            cost += edge.w;
            ++mst_edges;
        }
    }

    if (mst_edges != n_ - 2) {
        // 无法把顶点 1..n-1 连成树，说明这个分支不可行。
        return result;
    }

    // 1-tree 的第二部分：给 0 号顶点选择两条受约束的最小 incident edges。
    std::vector<Edge> root_edges;
    for (int v = 1; v < n_; ++v) {
        if (isForced(forced, 0, v)) {
            // 与 0 相连的强制边必须先加入 root_edges。
            if (isForbidden(forbidden, 0, v) || !isFinite(dist_[0][v])) {
                return result;
            }
            root_edges.push_back(Edge{0, v, dist_[0][v]});
        }
    }
    if (root_edges.size() > 2) {
        // TSP 回路中每个顶点度数必须为 2，0 号点也不能超过 2 条边。
        return result;
    }

    std::vector<Edge> root_candidates;
    // 收集 0 号顶点还可以选择的边，用于补足两条 incident edges。
    for (int v = 1; v < n_; ++v) {
        if (isForced(forced, 0, v) || isForbidden(forbidden, 0, v) || !isFinite(dist_[0][v])) {
            continue;
        }
        root_candidates.push_back(Edge{0, v, dist_[0][v]});
    }
    std::sort(root_candidates.begin(), root_candidates.end(), [](const Edge& lhs, const Edge& rhs) {
        return lhs.w < rhs.w;
    });

    // 按权重选择最便宜的可用边，直到 0 号点度数达到 2。
    for (const Edge& edge : root_candidates) {
        if (root_edges.size() == 2) {
            break;
        }
        root_edges.push_back(edge);
    }
    if (root_edges.size() != 2) {
        // 0 号顶点无法选到两条合法边，分支不可行。
        return result;
    }

    for (const Edge& edge : root_edges) {
        edges.push_back(edge);
        cost += edge.w;
    }

    // 统计 1-tree 中每个顶点的度数，用于判断是否已经得到一条 TSP 回路。
    for (const Edge& edge : edges) {
        ++result.degree[edge.u];
        ++result.degree[edge.v];
    }

    result.feasible = true;
    result.cost = cost;
    result.edges = std::move(edges);
    return result;
}

bool BranchBoundSolver::isTour(const OneTree& one_tree) const
{
    // 1-tree 必须有 n 条边；若每个顶点度数都是 2，则它就是单个 Hamilton 回路。
    if (!one_tree.feasible || static_cast<int>(one_tree.edges.size()) != n_) {
        return false;
    }
    return std::all_of(one_tree.degree.begin(), one_tree.degree.end(), [](int degree) {
        return degree == 2;
    });
}

std::vector<int> BranchBoundSolver::buildTour(const std::vector<Edge>& edges) const
{
    // 先把边集合转成邻接表，便于从任意起点沿回路走一圈。
    std::vector<std::vector<int>> adj(static_cast<std::size_t>(n_));
    for (const Edge& edge : edges) {
        adj[edge.u].push_back(edge.v);
        adj[edge.v].push_back(edge.u);
    }
    // 合法回路中每个顶点必须恰好有两个邻居。
    for (const auto& neighbors : adj) {
        if (neighbors.size() != 2) {
            return {};
        }
    }

    std::vector<int> tour;
    tour.reserve(static_cast<std::size_t>(n_));
    int previous = -1;
    int current = 0;
    for (int step = 0; step < n_; ++step) {
        tour.push_back(current);
        // 选择不是来路 previous 的那个邻居作为下一步。
        int next = adj[current][0] == previous ? adj[current][1] : adj[current][0];
        previous = current;
        current = next;
    }

    // 走完 n 步必须回到起点，否则说明边集合不是单个闭合回路。
    if (current != 0) {
        return {};
    }
    return tour;
}

bool BranchBoundSolver::chooseBranchEdge(const Node& node, Edge& edge) const
{
    int branch_vertex = -1;
    int branch_degree = 2;
    // 优先处理度数超过 2 的顶点；这类顶点必须删掉至少一条 incident edge。
    for (int v = 0; v < n_; ++v) {
        if (node.degree[v] > branch_degree) {
            branch_degree = node.degree[v];
            branch_vertex = v;
        }
    }

    auto usable = [&](const Edge& candidate) {
        // 已经被强制或禁止的边不再作为新的分支变量。
        return !isForced(node.forced, candidate.u, candidate.v)
            && !isForbidden(node.forbidden, candidate.u, candidate.v);
    };

    if (branch_vertex >= 0) {
        bool has_chosen = false;
        Edge chosen;
        for (const Edge& candidate : node.one_tree_edges) {
            if (!usable(candidate)) {
                continue;
            }
            if (candidate.u != branch_vertex && candidate.v != branch_vertex) {
                continue;
            }
            // 在该顶点 incident edges 中选较重的一条分支，通常更快排除差边。
            if (!has_chosen || candidate.w > chosen.w) {
                chosen = candidate;
                has_chosen = true;
            }
        }
        if (has_chosen) {
            edge = chosen;
            return true;
        }
    }

    for (const Edge& candidate : node.one_tree_edges) {
        // 若没有度数超过 2 的顶点，就退化为选择任意尚未定性的 1-tree 边。
        if (usable(candidate)) {
            edge = candidate;
            return true;
        }
    }
    return false;
}

bool BranchBoundSolver::shouldPrune(double bound, double best_cost) const
{
    // 只有已经有可行上界时才能做 bound 剪枝。
    return isFinite(best_cost) && bound >= best_cost - kEps;
}

bool BranchBoundSolver::findInitialTour(std::vector<int>& tour, double& cost) const
{
    bool found = false;
    double best_cost = std::numeric_limits<double>::infinity();
    std::vector<int> best_tour;

    // 从每个起点跑一次最近邻，再用 2-opt 局部改进，取最好的可行回路作为初始上界。
    for (int start = 0; start < n_; ++start) {
        std::vector<int> candidate;
        candidate.reserve(static_cast<std::size_t>(n_));
        std::vector<unsigned char> used(static_cast<std::size_t>(n_), 0);

        int current = start;
        candidate.push_back(current);
        used[current] = 1;
        double candidate_cost = 0.0;
        bool ok = true;

        for (int step = 1; step < n_; ++step) {
            int next = -1;
            double best_edge = std::numeric_limits<double>::infinity();
            // 最近邻：从当前顶点出发，找尚未访问且距离最短的下一个顶点。
            for (int v = 0; v < n_; ++v) {
                if (used[v] || !isFinite(dist_[current][v])) {
                    continue;
                }
                if (dist_[current][v] < best_edge) {
                    best_edge = dist_[current][v];
                    next = v;
                }
            }
            if (next < 0) {
                // 当前起点的最近邻路径断开，换下一个起点尝试。
                ok = false;
                break;
            }
            candidate_cost += best_edge;
            current = next;
            candidate.push_back(current);
            used[current] = 1;
        }

        if (!ok || !isFinite(dist_[current][start])) {
            // 不能回到起点就不是 Hamilton 回路。
            continue;
        }
        candidate_cost += dist_[current][start];
        // 最近邻只给出初始回路，再用 2-opt 做局部改进。
        twoOpt(candidate, candidate_cost);

        if (candidate_cost < best_cost) {
            best_cost = candidate_cost;
            best_tour = std::move(candidate);
            found = true;
        }
    }

    if (!found) {
        return false;
    }
    tour = std::move(best_tour);
    cost = best_cost;
    return true;
}

double BranchBoundSolver::tourCost(const std::vector<int>& tour) const
{
    // tour 只保存 n 个顶点，不重复保存末尾回到起点的那个顶点。
    if (static_cast<int>(tour.size()) != n_) {
        return std::numeric_limits<double>::infinity();
    }

    double cost = 0.0;
    for (int i = 0; i < n_; ++i) {
        int u = tour[i];
        int v = tour[(i + 1) % n_];
        // 回路中任何一条缺边都会让该 tour 无效。
        if (!isFinite(dist_[u][v])) {
            return std::numeric_limits<double>::infinity();
        }
        cost += dist_[u][v];
    }
    return cost;
}

void BranchBoundSolver::twoOpt(std::vector<int>& tour, double& cost) const
{
    bool improved = true;
    // 不断寻找能缩短回路的 2-opt 交换，直到没有改进为止。
    while (improved) {
        improved = false;
        for (int i = 1; i < n_ - 1 && !improved; ++i) {
            for (int k = i + 1; k < n_ && !improved; ++k) {
                // 固定 0 号顶点所在位置，从 i..k 翻转中间段。
                int a = tour[i - 1];
                int b = tour[i];
                int c = tour[k];
                int d = tour[(k + 1) % n_];
                // 新边不存在时，不能执行这次 2-opt 交换。
                if (!isFinite(dist_[a][c]) || !isFinite(dist_[b][d])) {
                    continue;
                }
                // 2-opt：用 (a,c)、(b,d) 替换 (a,b)、(c,d)，若总长下降则翻转中间段。
                double delta = dist_[a][c] + dist_[b][d] - dist_[a][b] - dist_[c][d];
                if (delta < -kEps) {
                    std::reverse(tour.begin() + i, tour.begin() + k + 1);
                    cost += delta;
                    improved = true;
                }
            }
        }
    }

    // 最后重新计算一次成本，避免多次浮点增量更新累积误差。
    cost = tourCost(tour);
}

std::vector<std::vector<double>> readDistanceMatrix(std::istream& input)
{
    int n = 0;
    // 输入第一项必须是顶点数。
    if (!(input >> n)) {
        throw std::runtime_error("input must start with the number of vertices");
    }
    if (n < 3) {
        throw std::runtime_error("TSP instance must contain at least 3 vertices");
    }

    std::vector<std::vector<double>> distance(static_cast<std::size_t>(n),
                                              std::vector<double>(static_cast<std::size_t>(n), 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            std::string token;
            // 矩阵必须完整给出 n*n 个 token。
            if (!(input >> token)) {
                throw std::runtime_error("distance matrix has fewer entries than expected");
            }
            // parseWeight 负责把 x / inf 等缺边符号转换为 infinity。
            distance[i][j] = parseWeight(token);
        }
    }
    return distance;
}

std::string formatTour(const std::vector<int>& tour)
{
    std::ostringstream out;
    // 输出形如 0 -> 1 -> 2 -> 0，末尾补回起点表示闭合回路。
    for (std::size_t i = 0; i < tour.size(); ++i) {
        if (i != 0) {
            out << " -> ";
        }
        out << tour[i];
    }
    if (!tour.empty()) {
        out << " -> " << tour.front();
    }
    return out.str();
}

} // namespace tsp
