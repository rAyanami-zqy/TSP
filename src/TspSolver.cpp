#include "TspSolver.hpp"

#include <algorithm>
#include <array>
#include <cmath>
#include <cctype>
#include <cstdint>
#include <functional>
#include <iomanip>
#include <iterator>
#include <limits>
#include <map>
#include <numeric>
#include <ostream>
#include <random>
#include <sstream>
#include <stdexcept>
#include <string>
#include <iostream>

namespace tsp {
namespace {

// 浮点数比较容差，用于处理 double 计算中的微小误差。
constexpr double kEps = 1e-9;
constexpr double kPi = 3.141592653589793238462643383279502884;

// 统一判断边权是否可用。缺边在输入中会被解析成 infinity。
bool isFinite(double value)
{
    return std::isfinite(value);
}

// 将输入中的调试输出间隔规范化为至少 1，避免除以 0 导致的错误。
std::size_t normalizedDebugInterval(const DebugOptions& debug)
{
    return debug.interval == 0 ? 1 : debug.interval;
}

// 如果调试输出流有效，则写一行调试信息并立即 flush，确保输出及时可见。
void writeDebugLine(const DebugOptions& debug, const std::string& message)
{
    if (debug.output == nullptr) {
        return;
    }
    (*debug.output) << "[tsp-debug] " << message << '\n';
    debug.output->flush();
}

// 格式化 double 数值用于调试输出，特殊处理 infinity 以避免输出过长的数字。
std::string formatDebugDouble(double value)
{
    if (!isFinite(value)) {
        return "inf";
    }
    std::ostringstream out;
    out << std::setprecision(10) << value;
    return out.str();
}

// 去除字符串首尾的空白字符，返回新的字符串副本。输入中的 section marker 可能带有多余空格。
std::string trimCopy(const std::string& text)
{
    const std::string whitespace = " \t\r\n";
    const std::size_t begin = text.find_first_not_of(whitespace);
    if (begin == std::string::npos) {
        return {};
    }
    const std::size_t end = text.find_last_not_of(whitespace);
    return text.substr(begin, end - begin + 1);
}

// 将字符串转换为全大写形式，返回新的字符串副本。输入中的 section marker 可能大小写不一致。
std::string upperCopy(std::string text)
{
    for (char& ch : text) {
        ch = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }
    return text;
}

// 判断一行文本是否是输入文件中的 section marker，标记了不同类型数据的开始。输入中可能有多余空格和大小写不一致。
bool isSectionMarker(const std::string& line)
{
    const std::string upper = upperCopy(trimCopy(line));
    return upper == "NODE_COORD_SECTION"
        || upper == "EDGE_WEIGHT_SECTION"
        || upper == "DISPLAY_DATA_SECTION"
        || upper == "TOUR_SECTION"
        || upper == "DEPOT_SECTION"
        || upper == "DEMAND_SECTION"
        || upper == "EOF";
}

// 将 double 类型的距离值四舍五入到最近的整数，符合 TSPLIB 中某些 EDGE_WEIGHT_TYPE 的定义。
int roundedDistance(double value)
{
    return static_cast<int>(std::floor(value + 0.5));
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

    // 从已有的 parent/rank 数组构造（用于复用部分解的 forced 并查集）。
    explicit DisjointSet(std::vector<int> parent, std::vector<int> rank)
        : parent_(std::move(parent)), rank_(std::move(rank)) {}

    // 查找元素 x 所属集合的代表元（根节点），路径压缩优化。
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
    // parent_[i] 是元素 i 的父节点，根节点的 parent_[i] == i。
    std::vector<int> parent_;
    // rank_[i] 是以 i 为根的树的秩（近似高度），用于优化 unite 操作。
    std::vector<int> rank_;
};

// 解析一个距离矩阵元素，支持数字、INF、-、X 等表示缺边的写法。
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

// 近似比较两个边权是否相等，考虑到输入中的微小误差和计算中的浮点误差。
bool sameWeight(double a, double b)
{
    // infinity 只能和 infinity 视为相等；有限值用容差比较。
    if (!isFinite(a) || !isFinite(b)) {
        return !isFinite(a) && !isFinite(b);
    }
    return std::fabs(a - b) <= kEps;
}

// 将 GEO 坐标格式的经纬度转换为弧度，符合 TSPLIB 中 GEO 坐标的定义。
double geoCoordinateToRadians(double value)
{
    const int degrees = static_cast<int>(value);
    const double minutes = value - static_cast<double>(degrees);
    return kPi * (static_cast<double>(degrees) + 5.0 * minutes / 3.0) / 180.0;
}

// 根据 EDGE_WEIGHT_TYPE 定义的距离计算方法，计算两个点之间的距离。支持多种距离类型，符合 TSPLIB 中的定义。
double coordinateDistance(const Point& a, const Point& b, const std::string& edge_weight_type)
{
    const double dx = a.x - b.x;
    const double dy = a.y - b.y;
    const double dz = a.z - b.z;
    const std::string type = upperCopy(edge_weight_type);

    if (type == "EUC_2D") {
        return roundedDistance(std::sqrt(dx * dx + dy * dy));
    }
    if (type == "CEIL_2D") {
        return std::ceil(std::sqrt(dx * dx + dy * dy));
    }
    if (type == "FLOOR_2D") {
        return std::floor(std::sqrt(dx * dx + dy * dy));
    }
    if (type == "MAN_2D") {
        return roundedDistance(std::fabs(dx) + std::fabs(dy));
    }
    if (type == "MAX_2D") {
        return roundedDistance(std::max(std::fabs(dx), std::fabs(dy)));
    }
    if (type == "EUC_3D") {
        return roundedDistance(std::sqrt(dx * dx + dy * dy + dz * dz));
    }
    if (type == "CEIL_3D") {
        return std::ceil(std::sqrt(dx * dx + dy * dy + dz * dz));
    }
    if (type == "MAN_3D") {
        return roundedDistance(std::fabs(dx) + std::fabs(dy) + std::fabs(dz));
    }
    if (type == "MAX_3D") {
        return roundedDistance(std::max({std::fabs(dx), std::fabs(dy), std::fabs(dz)}));
    }
    if (type == "ATT") {
        const double rij = std::sqrt((dx * dx + dy * dy) / 10.0);
        const int tij = roundedDistance(rij);
        return tij < rij ? tij + 1 : tij;
    }
    if (type == "GEO") {
        const double lat_a = geoCoordinateToRadians(a.x);
        const double lon_a = geoCoordinateToRadians(a.y);
        const double lat_b = geoCoordinateToRadians(b.x);
        const double lon_b = geoCoordinateToRadians(b.y);
        const double q1 = std::cos(lon_a - lon_b);
        const double q2 = std::cos(lat_a - lat_b);
        const double q3 = std::cos(lat_a + lat_b);
        const double argument = 0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3);
        const double clamped = std::max(-1.0, std::min(1.0, argument));
        return static_cast<int>(6378.388 * std::acos(clamped) + 1.0);
    }

    throw std::runtime_error("unsupported coordinate EDGE_WEIGHT_TYPE: " + edge_weight_type);
}

} // namespace

// TspProblem 成员函数实现。
int TspProblem::dimension() const
{
    if (!matrix.empty()) {
        return static_cast<int>(matrix.size());
    }
    return static_cast<int>(coordinates.size());
}

bool TspProblem::hasCoordinates() const
{
    return !coordinates.empty();
}

bool TspProblem::hasDenseMatrix() const
{
    return !matrix.empty();
}

double TspProblem::distance(int u, int v) const
{
    if (u == v) {
        return 0.0;
    }
    if (!matrix.empty()) {
        return matrix[static_cast<std::size_t>(u)][static_cast<std::size_t>(v)];
    }
    if (!coordinates.empty()) {
        return coordinateDistance(
            coordinates[static_cast<std::size_t>(u)],
            coordinates[static_cast<std::size_t>(v)],
            edge_weight_type.empty() ? "EUC_2D" : edge_weight_type);
    }
    throw std::runtime_error("problem has neither a matrix nor coordinates");
}

std::vector<std::vector<double>> TspProblem::toDenseMatrix(std::size_t max_dimension) const
{
    const int n = dimension();
    if (n <= 0) {
        throw std::runtime_error("TSP problem has no vertices");
    }
    if (max_dimension != 0 && static_cast<std::size_t>(n) > max_dimension) {
        throw std::runtime_error("instance dimension exceeds dense exact limit");
    }
    if (!matrix.empty()) {
        return matrix;
    }

    std::vector<std::vector<double>> dense(static_cast<std::size_t>(n),
                                           std::vector<double>(static_cast<std::size_t>(n), 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            const double value = distance(i, j);
            dense[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = value;
            dense[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = value;
        }
    }
    return dense;
}

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

void BranchBoundSolver::setDebugOutput(std::ostream& output, std::size_t progress_interval)
{
    debug_.output = &output;
    debug_.interval = progress_interval == 0 ? 1 : progress_interval;
}

void BranchBoundSolver::disableDebugOutput()
{
    debug_ = {};
}

SolveResult BranchBoundSolver::solve()
{
    const std::size_t edge_state_size = static_cast<std::size_t>(n_) * static_cast<std::size_t>(n_);

    // 重置搜索状态。
    result_ = SolveResult{};
    best_cost_ = std::numeric_limits<double>::infinity();
    best_tour_.clear();

    writeDebugLine(debug_, "exact solve started: vertices=" + std::to_string(n_));

    // 先用启发式得到一个上界。
    if (findInitialTour(best_tour_, best_cost_)) {
        result_.stats.initial_upper_bound = best_cost_;
        writeDebugLine(debug_, "initial incumbent: cost=" + formatDebugDouble(best_cost_));
    } else {
        result_.stats.initial_upper_bound = std::numeric_limits<double>::infinity();
        writeDebugLine(debug_, "initial incumbent: unavailable");
    }
    PartialSol root;
    root.depth = 0;
    root.forced.assign(edge_state_size, 0);
    root.forbidden.assign(edge_state_size, 0);
    // 初始化 forced 边的并查集和度数统计。
    root.forced_parent.resize(static_cast<std::size_t>(n_));
    std::iota(root.forced_parent.begin(), root.forced_parent.end(), 0);
    root.forced_rank.assign(static_cast<std::size_t>(n_), 0);
    root.forced_comp_size.assign(static_cast<std::size_t>(n_), 1);
    root.forced_degree.assign(static_cast<std::size_t>(n_), 0);
    // 边候选集初始化：收集所有有限权重的无向边，按权重排序。
    // 后续 buildBranchCandidates 通过 compaction 过滤，保持排序不变，
    // computeOneTree 可假定输入已按权重升序并直接用于 Kruskal。
    std::vector<Edge> branch_candidates;
    for (int u = 0; u < n_; ++u) {
        for (int v = u + 1; v < n_; ++v) {
            if (!isFinite(dist_[u][v])) {
                continue;
            }
            branch_candidates.push_back(Edge{u, v, dist_[u][v]});
        }
    }
    std::sort(branch_candidates.begin(), branch_candidates.end(),
              [](const Edge& a, const Edge& b) { return a.w < b.w; });

    // 初始化候选集掩码，供 chooseBranchEdge O(1) 查询。
    root.candidate_mask.assign(edge_state_size, 0);
    for (const Edge& e : branch_candidates) {
        root.candidate_mask[edgeId(e.u, e.v)] = 1;
    }

    // 预排序与顶点 0 相连的边，computeOneTree 中直接取前两条合法边。
    for (int v = 1; v < n_; ++v) {
        if (isFinite(dist_[0][v])) {
            root_candidates_sorted_.push_back(Edge{0, v, dist_[0][v]});
        }
    }
    std::sort(root_candidates_sorted_.begin(), root_candidates_sorted_.end(),
              [](const Edge& a, const Edge& b) { return a.w < b.w; });

    OneTree root_tree = computeOneTree(root, branch_candidates);
    if (!root_tree.feasible) {
        result_.feasible = false;
        result_.cost = std::numeric_limits<double>::infinity();
        result_.stats.root_lower_bound = std::numeric_limits<double>::infinity();
        writeDebugLine(debug_, "root 1-tree infeasible; exact solve stopped");
        return result_;
    }
    result_.stats.root_lower_bound = root_tree.cost;
    root.bound = root_tree.cost;
    result_.stats.nodes_created = 1;
    {
        std::ostringstream line;
        line << "root: lower_bound=" << formatDebugDouble(root.bound)
             << " search=bp-chain";
        writeDebugLine(debug_, line.str());
    }

    search(root, branch_candidates, 0);

    // 搜索结束后，best_cost_ 有限且 best_tour_ 非空才算找到可行最优解。
    if (isFinite(best_cost_) && !best_tour_.empty()) {
        result_.feasible = true;
        result_.cost = best_cost_;
        result_.tour = std::move(best_tour_);
    } else {
        result_.feasible = false;
        result_.cost = std::numeric_limits<double>::infinity();
    }
    {
        std::ostringstream line;
        line << "exact solve finished: feasible=" << (result_.feasible ? "yes" : "no")
             << " cost=" << formatDebugDouble(result_.cost)
             << " expanded=" << result_.stats.nodes_expanded
             << " created=" << result_.stats.nodes_created
             << " pruned_bound=" << result_.stats.nodes_pruned_by_bound
             << " pruned_infeasible=" << result_.stats.nodes_pruned_infeasible;
        writeDebugLine(debug_, line.str());
    }
    return result_;
}

// ── BP (Branch Partitioning) 实现 ────────────────────────────────

std::vector<BranchBoundSolver::Edge> BranchBoundSolver::bpPartition(
    const PartialSol& node,
    const std::vector<Edge>& branch_candidates,
    const OneTree& current_tree,
    std::vector<OneTree>& forbid_trees)
{
    std::vector<Edge> B_set;
    B_set.reserve(current_tree.edges.size());
    forbid_trees.clear();
    forbid_trees.reserve(current_tree.edges.size());

    PartialSol work_node = node;
    std::vector<Edge> work_candidates = branch_candidates;
    const OneTree* work_tree = &current_tree;

    while (true) {
        const Edge* deg_best = nullptr;

        constexpr bool kUseMinEdgeFromOneTree = true;

        if (kUseMinEdgeFromOneTree) {
            // ── 新选边策略：从 1-tree 所有未决边中直接选最小权重边 ──
            for (const Edge& e : work_tree->edges) {
                const std::size_t eid = edgeId(e.u, e.v);
                if (work_node.forced[eid]) continue;
                if (work_node.forbidden[eid]) continue;
                if (!deg_best || e.w < deg_best->w) deg_best = &e;
            }
        } else {
            // ── 原选边策略：从度数违规顶点取最小权重未决边 ──
            // 若没有度数 > 2 的顶点，则当前可行 1-tree 已由 isTour() 处理。
            int branch_vertex = -1;
            int max_deg = 2;
            for (int v = 0; v < n_; ++v) {
                if (work_tree->degree[static_cast<std::size_t>(v)] > max_deg) {
                    max_deg = work_tree->degree[static_cast<std::size_t>(v)];
                    branch_vertex = v;
                }
            }
            if (branch_vertex >= 0) {
                for (const Edge& e : work_tree->edges) {
                    if (e.u != branch_vertex && e.v != branch_vertex) continue;
                    const std::size_t eid = edgeId(e.u, e.v);
                    if (work_node.forced[eid]) continue;
                    if (work_node.forbidden[eid]) continue;
                    if (!deg_best || e.w < deg_best->w) deg_best = &e;
                }
            }
        }

        if (!deg_best) break;

        // 测试函数：选中的边先进入 B；若 forbid 后仍可行且未被 bound 剪枝，则继续划分。
        auto test = [&](Edge e) -> bool {
            const std::size_t eid = edgeId(e.u, e.v);
            work_node.forbidden[eid] = 1;

            auto it = std::find_if(work_candidates.begin(), work_candidates.end(),
                [&](const Edge& x) {
                    return (x.u == e.u && x.v == e.v)
                        || (x.u == e.v && x.v == e.u);
                });
            if (it != work_candidates.end()) work_candidates.erase(it);

            unsigned char sm = 0;
            if (!work_node.candidate_mask.empty()) {
                sm = work_node.candidate_mask[eid];
                work_node.candidate_mask[eid] = 0;
            }

            OneTree t = computeOneTree(work_node, work_candidates);

            if (!work_node.candidate_mask.empty()) {
                work_node.candidate_mask[eid] = sm;
            }

            B_set.push_back(e);
            forbid_trees.push_back(std::move(t));
            const OneTree& saved_tree = forbid_trees.back();
            if (!saved_tree.feasible) {
                return false;
            }
            if (isTour(saved_tree)) {
                std::vector<int> candidate = buildTour(saved_tree.edges);
                if (!candidate.empty() && saved_tree.cost + kEps < best_cost_) {
                    best_cost_ = saved_tree.cost;
                    best_tour_ = std::move(candidate);
                    writeDebugLine(debug_,
                                   "new incumbent: cost=" + formatDebugDouble(best_cost_)
                                       + " source=bp-partition");
                }
                return false;
            }
            if (shouldPrune(saved_tree.cost, best_cost_)) {
                return false;
            }

            // forbid 后仍可能包含改进解：保留 forbidden，更新持久状态后继续划分。
            work_tree = &saved_tree;
            return true;
        };

        if (test(*deg_best)) continue;
        break;
    }

    return B_set;
}

void BranchBoundSolver::search(PartialSol& node, std::vector<Edge>& branch_candidates, int depth)
{
    // Step 1: 计算 1-tree 下界。
    OneTree current_tree = computeOneTree(node, branch_candidates);
    node.bound = current_tree.cost;
    node.depth = depth;

    // Step 2: 下界剪枝。
    if (shouldPrune(node.bound, best_cost_)) {
        ++result_.stats.nodes_pruned_by_bound;
        return;
    }
    ++result_.stats.nodes_expanded;

    // 调试输出。
    const std::size_t debug_interval = normalizedDebugInterval(debug_);
    if (debug_.output != nullptr && result_.stats.nodes_expanded % debug_interval == 0) {
        std::ostringstream line;
        line << "progress: expanded=" << result_.stats.nodes_expanded
             << " created=" << result_.stats.nodes_created
             << " depth=" << depth
             << " bound=" << formatDebugDouble(node.bound)
             << " best=" << formatDebugDouble(best_cost_)
             << " pruned_bound=" << result_.stats.nodes_pruned_by_bound
             << " pruned_infeasible=" << result_.stats.nodes_pruned_infeasible;
        writeDebugLine(debug_, line.str());
    }

    // Step 3: 检查是否已是合法 TSP 回路。
    if (isTour(current_tree)) {
        std::vector<int> candidate = buildTour(current_tree.edges);
        if (!candidate.empty() && current_tree.cost + kEps < best_cost_) {
            best_cost_ = current_tree.cost;
            best_tour_ = std::move(candidate);
            writeDebugLine(debug_,
                           "new incumbent: cost=" + formatDebugDouble(best_cost_)
                               + " source=bp-node depth=" + std::to_string(depth));
        }
        return;
    }

    // Step 4: BP 划分 —— 获取 B 集（关键边）。
    std::vector<OneTree> forbid_trees;
    std::vector<Edge> B_set = bpPartition(node, branch_candidates, current_tree, forbid_trees);

    // B 为空：禁止任意 1-tree 边均超上界，等价于 bound 剪枝。
    if (B_set.empty()) {
        ++result_.stats.nodes_pruned_by_bound;
        return;
    }

    // Step 5: 链式分支。
    struct LevelChanges {
        std::size_t forced_id = static_cast<std::size_t>(-1);
        std::size_t forbidden_id = static_cast<std::size_t>(-1);
        int fu = -1, fv = -1;
        int root_u = -1, root_v = -1;
        int old_rank_u = -1, old_rank_v = -1;
        int old_size_u = -1, old_size_v = -1;
    };

    auto forced_find = [&](int x) -> int {
        while (node.forced_parent[static_cast<std::size_t>(x)] != x) {
            x = node.forced_parent[static_cast<std::size_t>(x)];
        }
        return x;
    };

    auto apply_flag = [&](const Edge& e, bool force, LevelChanges& changes) -> bool {
        const std::size_t id = edgeId(e.u, e.v);
        if (force) {
            if (node.forbidden[id] || !isFinite(dist_[e.u][e.v])) return false;
            if (node.forced_degree[static_cast<std::size_t>(e.u)] >= 2
                || node.forced_degree[static_cast<std::size_t>(e.v)] >= 2) {
                return false;
            }
            const int ru = forced_find(e.u);
            const int rv = forced_find(e.v);
            if (ru == rv) {
                if (node.forced_comp_size[static_cast<std::size_t>(ru)] < n_) {
                    return false;
                }
            }
            node.forced[id] = 1;
            changes.forced_id = id;

            changes.fu = e.u;
            changes.fv = e.v;
            changes.root_u = ru;
            changes.root_v = rv;
            changes.old_rank_u = node.forced_rank[static_cast<std::size_t>(ru)];
            changes.old_rank_v = node.forced_rank[static_cast<std::size_t>(rv)];
            changes.old_size_u = node.forced_comp_size[static_cast<std::size_t>(ru)];
            changes.old_size_v = node.forced_comp_size[static_cast<std::size_t>(rv)];

            int ra = ru, rb = rv;
            if (node.forced_rank[static_cast<std::size_t>(ra)]
                < node.forced_rank[static_cast<std::size_t>(rb)]) {
                std::swap(ra, rb);
            }
            node.forced_parent[static_cast<std::size_t>(rb)] = ra;
            node.forced_comp_size[static_cast<std::size_t>(ra)]
                += node.forced_comp_size[static_cast<std::size_t>(rb)];
            if (node.forced_rank[static_cast<std::size_t>(ra)]
                == node.forced_rank[static_cast<std::size_t>(rb)]) {
                ++node.forced_rank[static_cast<std::size_t>(ra)];
            }

            ++node.forced_degree[static_cast<std::size_t>(e.u)];
            ++node.forced_degree[static_cast<std::size_t>(e.v)];

            node.forced_edges.push_back(e);
            if (e.u != 0 && e.v != 0) {
                node.forced_mst_cost += dist_[e.u][e.v];
                ++node.forced_mst_count;
            }
        } else {
            if (node.forced[id]) return false;
            node.forbidden[id] = 1;
            changes.forbidden_id = id;
        }
        return true;
    };

    auto revert_flag = [&](const LevelChanges& changes) {
        if (changes.forced_id != static_cast<std::size_t>(-1)) {
            node.forced[changes.forced_id] = 0;

            node.forced_parent[static_cast<std::size_t>(changes.root_u)] = changes.root_u;
            node.forced_parent[static_cast<std::size_t>(changes.root_v)] = changes.root_v;
            node.forced_rank[static_cast<std::size_t>(changes.root_u)] = changes.old_rank_u;
            node.forced_rank[static_cast<std::size_t>(changes.root_v)] = changes.old_rank_v;
            node.forced_comp_size[static_cast<std::size_t>(changes.root_u)] = changes.old_size_u;
            node.forced_comp_size[static_cast<std::size_t>(changes.root_v)] = changes.old_size_v;

            --node.forced_degree[static_cast<std::size_t>(changes.fu)];
            --node.forced_degree[static_cast<std::size_t>(changes.fv)];

            const Edge& last = node.forced_edges.back();
            if (last.u != 0 && last.v != 0) {
                node.forced_mst_cost -= dist_[last.u][last.v];
                --node.forced_mst_count;
            }
            node.forced_edges.pop_back();
        }
        if (changes.forbidden_id != static_cast<std::size_t>(-1)) {
            node.forbidden[changes.forbidden_id] = 0;
        }
    };

    auto rebuild_mask = [&](const std::vector<Edge>& cands) {
        if (!node.candidate_mask.empty()) {
            std::fill(node.candidate_mask.begin(), node.candidate_mask.end(),
                      static_cast<unsigned char>(0));
            for (const Edge& e : cands) {
                node.candidate_mask[edgeId(e.u, e.v)] = 1;
            }
        }
    };

    // Step 5: 链式分支 —— force 节点递归（重做 BP 划分），forbid 节点沿链走（免重算）。
    //   完备性：BP 保证至少一条 B 集边属于最优解。
    //     Force  B[i] → 覆盖 B[i] ∈ opt
    //     Forbid B[i] → 覆盖 B[i] ∉ opt，但 B[i+1..k] 中至少一条 ∈ opt
    //   Forbid 链到底（所有 B 边均被禁止）⇒ 与 BP 保证矛盾 ⇒ 死胡同。
    //   forbid 节点不调用 search()，使用 bpPartition 预计算的 forbid_trees[i] 做剪枝。
    std::function<void(int)> chain_branch = [&](int idx) {
        if (idx >= static_cast<int>(B_set.size())) {
            return;  // 所有 B 边均被禁止，死胡同
        }

        const Edge branch_edge = B_set[static_cast<std::size_t>(idx)];

        // ── Force B[i]：强制选择，进入完整 BP 递归 ──
        {
            LevelChanges flag_changes;
            if (apply_flag(branch_edge, true, flag_changes)) {
                std::vector<Edge> original_candidates = branch_candidates;
                std::vector<unsigned char> original_mask = node.candidate_mask;

                bool ok = buildBranchCandidates(node, branch_candidates);
                if (ok) {
                    rebuild_mask(branch_candidates);
                    ++result_.stats.nodes_created;
                    search(node, branch_candidates, depth + 1);
                } else {
                    ++result_.stats.nodes_pruned_infeasible;
                }

                branch_candidates = std::move(original_candidates);
                node.candidate_mask = std::move(original_mask);
                revert_flag(flag_changes);
            } else {
                ++result_.stats.nodes_pruned_infeasible;
            }
        }

        // ── Forbid B[i]：禁止，沿链继续（不重新 BP 划分）──
        {
            LevelChanges flag_changes;
            if (apply_flag(branch_edge, false, flag_changes)) {
                std::vector<Edge> original_candidates = branch_candidates;
                std::vector<unsigned char> original_mask = node.candidate_mask;

                auto it = std::find_if(branch_candidates.begin(), branch_candidates.end(),
                    [&](const Edge& e) {
                        return (e.u == branch_edge.u && e.v == branch_edge.v)
                            || (e.u == branch_edge.v && e.v == branch_edge.u);
                    });
                if (it != branch_candidates.end()) {
                    if (!node.candidate_mask.empty()) {
                        node.candidate_mask[edgeId(it->u, it->v)] = 0;
                    }
                    branch_candidates.erase(it);
                }

                // 轻量可行性检查。
                std::vector<int> avail = node.forced_degree;
                for (const Edge& e : branch_candidates) {
                    ++avail[static_cast<std::size_t>(e.u)];
                    ++avail[static_cast<std::size_t>(e.v)];
                }
                bool feasible = true;
                for (int v = 0; v < n_ && feasible; ++v) {
                    if (avail[static_cast<std::size_t>(v)] < 2) {
                        feasible = false;
                    }
                }

                if (feasible) {
                    ++result_.stats.nodes_created;

                    // 用 bpPartition 预计算的 forbid_trees[idx] 做下界剪枝。
                    const OneTree& forbid_tree = forbid_trees[static_cast<std::size_t>(idx)];
                    if (!forbid_tree.feasible) {
                        branch_candidates = std::move(original_candidates);
                        node.candidate_mask = std::move(original_mask);
                        revert_flag(flag_changes);
                        ++result_.stats.nodes_pruned_infeasible;
                        return;
                    }
                    if (isTour(forbid_tree)) {
                        std::vector<int> candidate = buildTour(forbid_tree.edges);
                        if (!candidate.empty() && forbid_tree.cost + kEps < best_cost_) {
                            best_cost_ = forbid_tree.cost;
                            best_tour_ = std::move(candidate);
                            writeDebugLine(debug_,
                                           "new incumbent: cost=" + formatDebugDouble(best_cost_)
                                               + " source=bp-forbid depth="
                                               + std::to_string(depth + 1));
                        }
                        branch_candidates = std::move(original_candidates);
                        node.candidate_mask = std::move(original_mask);
                        revert_flag(flag_changes);
                        return;
                    }
                    if (shouldPrune(forbid_tree.cost, best_cost_)) {
                        branch_candidates = std::move(original_candidates);
                        node.candidate_mask = std::move(original_mask);
                        revert_flag(flag_changes);
                        ++result_.stats.nodes_pruned_by_bound;
                        return;
                    }

                    // 沿链继续，不调用 search()（forbid 节点不做 BP 划分）。
                    chain_branch(idx + 1);
                } else {
                    ++result_.stats.nodes_pruned_infeasible;
                }

                branch_candidates = std::move(original_candidates);
                node.candidate_mask = std::move(original_mask);
                revert_flag(flag_changes);
            } else {
                ++result_.stats.nodes_pruned_infeasible;
            }
        }
    };

    chain_branch(0);
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

// 在当前分支节点的强制边和禁止边约束下，构造受约束的 1-tree 以计算下界。
// 候选集已在 buildBranchCandidates 中过滤，此处不再重复可行性检查。
BranchBoundSolver::OneTree BranchBoundSolver::computeOneTree(
    const PartialSol &node_,std::vector<Edge>&branch_candidates) const
{
    OneTree result;
    result.degree.assign(static_cast<std::size_t>(n_), 0);

    // 复用部分解的 forced 并查集初始化 MST 的 DisjointSet（O(n) 拷贝），
    // 避免 O(n²) 扫描中逐条 unite。顶点 0 不参与 MST，需将其所在分量中的
    // 其余顶点脱离为独立分量。
    std::vector<int> mst_parent = node_.forced_parent;
    std::vector<int> mst_rank = node_.forced_rank;
    // 找到顶点 0 的根，将分量内的其他顶点重置为独立根。
    int root0 = 0;
    while (mst_parent[static_cast<std::size_t>(root0)] != root0) {
        root0 = mst_parent[static_cast<std::size_t>(root0)];
    }
    for (int v = 1; v < n_; ++v) {
        int rv = v;
        while (mst_parent[static_cast<std::size_t>(rv)] != rv) {
            rv = mst_parent[static_cast<std::size_t>(rv)];
        }
        if (rv == root0) {
            mst_parent[static_cast<std::size_t>(v)] = v;
            mst_rank[static_cast<std::size_t>(v)] = 0;
        }
    }
    DisjointSet tree_components(std::move(mst_parent), std::move(mst_rank));

    int mst_edges = 0;
    double cost = 0.0;
    std::vector<Edge> edges;
    edges.reserve(static_cast<std::size_t>(n_));

    // 1-tree 的第一部分：从部分解中直接取用 forced 边（O(#forced) 而非 O(n²)）。
    // 强制边已在 buildBranchCandidates 中验证无环、无度超。
    cost += node_.forced_mst_cost;
    mst_edges = node_.forced_mst_count;
    for (const Edge& e : node_.forced_edges) {
        if (e.u == 0 || e.v == 0) continue;
        edges.push_back(e);
    }
    // branch_candidates 由调用者保证已按权重升序排列，无需再次排序。

    // Kruskal：按权重从小到大加入不会成环的边，直到 MST 有 n-2 条边。
    // 跳过与顶点 0 相连的边，它们留给 1-tree 的第二部分处理。
    for (const Edge& edge : branch_candidates) {
        if (mst_edges == n_ - 2) {
            break;
        }
        if (edge.u == 0 || edge.v == 0) {
            continue;
        }
        if (tree_components.unite(edge.u, edge.v)) {
            edges.push_back(edge);
            cost += edge.w;
            ++mst_edges;
        }
    }

    if (mst_edges != n_ - 2) {
        return result;
    }

    // 1-tree 的第二部分：给 0 号顶点选择两条最小 incident edges。
    // 使用预排序的 root_candidates_sorted_，跳过不在候选集中的边。
    std::vector<Edge> root_edges;
    // 先从 forced 边列表中收集与顶点 0 相连的（≤2 条）。
    for (const Edge& e : node_.forced_edges) {
        if (e.u == 0 || e.v == 0) {
            root_edges.push_back(e);
        }
    }

    // 从预排序列表中补充可用的候选边，直至凑满 2 条。
    // 必须检查 candidate_mask，因为某些边可能已被过滤（度数、子回路等）。
    const bool has_mask = !node_.candidate_mask.empty();
    for (const Edge& e : root_candidates_sorted_) {
        if (root_edges.size() == 2) break;
        const std::size_t eid = edgeId(e.u, e.v);
        if (node_.forced[eid]) continue;       // 已由 forced 列表计入
        if (node_.forbidden[eid]) continue;    // 被当前节点禁止
        if (has_mask && !node_.candidate_mask[eid]) continue;  // 不在候选集中
        root_edges.push_back(e);
    }
    if (root_edges.size() != 2) {
        return result;
    }

    for (const Edge& edge : root_edges) {
        edges.push_back(edge);
        cost += edge.w;
    }

    // 统计 1-tree 中每个顶点的度数，用于判断是否已经得到一条 TSP 回路。
    for (const Edge& edge : edges) {
        ++result.degree[static_cast<std::size_t>(edge.u)];
        ++result.degree[static_cast<std::size_t>(edge.v)];
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

bool BranchBoundSolver::buildBranchCandidates(const PartialSol& node,std::vector<Edge>& branch_candidates) const
{
    // 利用节点中缓存的 forced_degree 和 forced 并查集，避免 O(n²) 扫描。
    // apply_flag 已保证：无 forced/forbidden 冲突、度数 ≤ 2、无子回路，
    // 因此此处只需按 forced_degree 和 forced 分量做候选集过滤。
    const std::vector<int>& forced_degree = node.forced_degree;

    // 预计算所有顶点的 DSU 根（O(n)），避免每条边 O(tree_height) 的 while 查找。
    std::vector<int> dsu_root(static_cast<std::size_t>(n_));
    for (int v = 0; v < n_; ++v) {
        int r = v;
        while (node.forced_parent[static_cast<std::size_t>(r)] != r) {
            r = node.forced_parent[static_cast<std::size_t>(r)];
        }
        dsu_root[static_cast<std::size_t>(v)] = r;
    }

    // 过滤候选集：将保留的边压缩到 vector 前端。
    std::size_t write = 0;
    for (std::size_t read = 0; read < branch_candidates.size(); ++read) {
        const Edge& e = branch_candidates[read];
        const std::size_t eid = edgeId(e.u, e.v);

        bool remove = false;
        // 已决定的边（强制或禁止）不再作为候选。
        if (node.forced[eid] || node.forbidden[eid]) {
            remove = true;
        }
        // 端点已由强制边满足度数 2，不能再加入新边。
        else if (forced_degree[e.u] >= 2 || forced_degree[e.v] >= 2) {
            remove = true;
        }
        // 两端点已在同一强制分量中且该分量未包含全部顶点，加入此边会形成子回路。
        else if (e.u > 0 && e.v > 0) {
            const int ru = dsu_root[static_cast<std::size_t>(e.u)];
            const int rv = dsu_root[static_cast<std::size_t>(e.v)];
            if (ru == rv
                && node.forced_comp_size[static_cast<std::size_t>(ru)] < n_) {
                remove = true;
            }
        }

        if (!remove) {
            if (write != read) {
                branch_candidates[write] = branch_candidates[read];
            }
            ++write;
        }
    }
    branch_candidates.resize(write);

    // 验证每个顶点仍有足够候选边以达到度数 2。
    std::vector<int> available_degree = forced_degree;
    for (const Edge& e : branch_candidates) {
        ++available_degree[e.u];
        ++available_degree[e.v];
    }
    for (int v = 0; v < n_; ++v) {
        if (available_degree[v] < 2) {
            return false;
        }
    }

    return true;
}


bool BranchBoundSolver::shouldPrune(double bound, double best_cost) const
{
    // 只有已经有可行上界时才能做 bound 剪枝。
    return isFinite(best_cost) && bound >= best_cost - kEps;
}

bool BranchBoundSolver::findInitialTour(std::vector<int>& tour, double& cost)
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

    // 用 Lin-Kernighan 对最佳 NN+2opt 结果做精细优化，获取更紧上界。
    linKernighan(best_tour, best_cost);

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

// ── Lin-Kernighan 启发式 ──────────────────────────────────────────

std::vector<int> BranchBoundSolver::buildPositionMap(const std::vector<int>& tour) const
{
    std::vector<int> pos(static_cast<std::size_t>(n_), -1);
    for (int i = 0; i < static_cast<int>(tour.size()); ++i) {
        pos[static_cast<std::size_t>(tour[i])] = i;
    }
    return pos;
}

void BranchBoundSolver::buildCandidateSets() const
{
    if (candidate_set_built_) {
        return;
    }
    candidate_set_.resize(static_cast<std::size_t>(n_));
    for (int i = 0; i < n_; ++i) {
        std::vector<std::pair<double, int>> nb;
        nb.reserve(static_cast<std::size_t>(n_) - 1);
        for (int j = 0; j < n_; ++j) {
            if (i == j || !isFinite(dist_[i][j])) {
                continue;
            }
            nb.emplace_back(dist_[i][j], j);
        }
        const int keep = std::min(kLkCandidateSize, static_cast<int>(nb.size()));
        std::nth_element(nb.begin(), nb.begin() + keep, nb.end(),
                         [](const auto& a, const auto& b) { return a.first < b.first; });
        nb.resize(static_cast<std::size_t>(keep));
        std::sort(nb.begin(), nb.end(),
                  [](const auto& a, const auto& b) { return a.first < b.first; });
        candidate_set_[static_cast<std::size_t>(i)].reserve(static_cast<std::size_t>(keep));
        for (const auto& p : nb) {
            candidate_set_[static_cast<std::size_t>(i)].push_back(p.second);
        }
    }
    candidate_set_built_ = true;
}

bool BranchBoundSolver::lkSearch(int t1, int t2,
                                  std::vector<int>& next,
                                  std::vector<int>& prev,
                                  std::vector<bool>& active) const
{
    // LK 交替链中的一步：添加 (from,to)，删除 (to,rem)。
    struct LKStep {
        int from = -1;
        int to = -1;
        int rem = -1;
    };

    double best_gain = kEps;
    std::vector<LKStep> best_seq;
    std::vector<LKStep> cur_seq;
    std::vector<unsigned char> used(static_cast<std::size_t>(n_), 0);

    used[static_cast<std::size_t>(t1)] = 1;
    used[static_cast<std::size_t>(t2)] = 1;

    // Phase 1: DFS 搜索最佳 k-opt 序列（固定 t4 = next[t3]，与 LK 原论文一致）。
    std::function<void(int, double, int)> dfs = [&](int from, double cum_gain, int depth) {
        if (depth >= kLkMaxDepth) {
            return;
        }
        for (int x : candidate_set_[static_cast<std::size_t>(from)]) {
            // 尝试 close-up：添加 (from, t1) 闭合回路。
            if (x == t1) {
                if (!isFinite(dist_[from][t1])) {
                    continue;
                }
                // 闭合边 (from, t1) 不能是当前 tour 中的边。
                if (next[static_cast<std::size_t>(from)] == t1
                    || prev[static_cast<std::size_t>(from)] == t1) {
                    continue;
                }
                double total = cum_gain - dist_[from][t1];
                if (total > best_gain) {
                    best_gain = total;
                    best_seq = cur_seq;
                }
                continue;
            }
            if (used[static_cast<std::size_t>(x)]) {
                continue;
            }
            if (!isFinite(dist_[from][x])) {
                continue;
            }
            // 增益准则：被加入边必须比累积 gain 小（维持正 gain）。
            if (dist_[from][x] >= cum_gain - kEps) {
                continue;
            }

            // 按照 LK 原论文：t4 固定为 next[t3]（一致的前进方向）。
            int y = next[static_cast<std::size_t>(x)];
            if (y == from || used[static_cast<std::size_t>(y)]) {
                continue;
            }

            double new_gain = cum_gain - dist_[from][x] + dist_[x][y];
            if (new_gain <= kEps) {
                continue;
            }

            used[static_cast<std::size_t>(y)] = 1;
            cur_seq.push_back({from, x, y});
            dfs(y, new_gain, depth + 1);
            cur_seq.pop_back();
            used[static_cast<std::size_t>(y)] = 0;
        }
    };

    dfs(t2, dist_[t1][t2], 1);

    if (best_seq.empty()) {
        return false;
    }

    // Phase 2: 应用 k-opt 交换。使用邻接表重建 tour。
    std::vector<std::vector<int>> adj(static_cast<std::size_t>(n_));
    for (int v = 0; v < n_; ++v) {
        adj[static_cast<std::size_t>(v)].push_back(next[static_cast<std::size_t>(v)]);
        adj[static_cast<std::size_t>(v)].push_back(prev[static_cast<std::size_t>(v)]);
    }

    auto rm_edge = [&](int u, int v) {
        auto& nu = adj[static_cast<std::size_t>(u)];
        for (auto it = nu.begin(); it != nu.end(); ++it) {
            if (*it == v) { nu.erase(it); break; }
        }
        auto& nv = adj[static_cast<std::size_t>(v)];
        for (auto it = nv.begin(); it != nv.end(); ++it) {
            if (*it == u) { nv.erase(it); break; }
        }
    };

    // 删除第一条边 (t1, t2)。
    rm_edge(t1, t2);

    // 按序列依次添加/删除边。
    int last = t2;
    for (const auto& s : best_seq) {
        adj[static_cast<std::size_t>(s.from)].push_back(s.to);
        adj[static_cast<std::size_t>(s.to)].push_back(s.from);
        rm_edge(s.to, s.rem);
        last = s.rem;
    }

    // 闭合边 (last, t1)。
    adj[static_cast<std::size_t>(last)].push_back(t1);
    adj[static_cast<std::size_t>(t1)].push_back(last);

    // 验证所有顶点度数 = 2 且两个邻居不同（无重复边）。
    for (int v = 0; v < n_; ++v) {
        const auto& nb = adj[static_cast<std::size_t>(v)];
        if (nb.size() != 2) {
            return false;
        }
        if (nb[0] == nb[1]) {
            return false;  // 两个邻居相同，回路无效。
        }
    }

    // 从顶点 0 开始沿邻接表走，验证是否构成单一 Hamilton 回路。
    // 必须恰好 n 步回到 0，且覆盖所有顶点。
    std::vector<int> new_tour(static_cast<std::size_t>(n_));
    std::vector<unsigned char> seen(static_cast<std::size_t>(n_), 0);
    int cur = 0;
    int pv = -1;
    for (int i = 0; i < n_; ++i) {
        new_tour[static_cast<std::size_t>(i)] = cur;
        seen[static_cast<std::size_t>(cur)] = 1;
        const auto& nb = adj[static_cast<std::size_t>(cur)];
        // 选不是来路的邻居作为下一步。
        int nxt = (nb[0] != pv) ? nb[0] : nb[1];
        pv = cur;
        cur = nxt;
    }
    // 必须回到起点且覆盖全部顶点。
    if (cur != 0) {
        return false;
    }
    for (int v = 0; v < n_; ++v) {
        if (!seen[static_cast<std::size_t>(v)]) {
            return false;
        }
    }

    // 重建 next / prev 双向链表。
    for (int i = 0; i < n_; ++i) {
        int u = new_tour[static_cast<std::size_t>(i)];
        int v = new_tour[static_cast<std::size_t>((i + 1) % n_)];
        next[static_cast<std::size_t>(u)] = v;
        prev[static_cast<std::size_t>(v)] = u;
    }

    // 重新激活被改动边的顶点，以便下一轮继续尝试改进。
    active[static_cast<std::size_t>(t1)] = true;
    active[static_cast<std::size_t>(t2)] = true;
    for (const auto& s : best_seq) {
        active[static_cast<std::size_t>(s.from)] = true;
        active[static_cast<std::size_t>(s.to)] = true;
        active[static_cast<std::size_t>(s.rem)] = true;
    }

    return true;
}

bool BranchBoundSolver::linKernighanImprove(std::vector<int>& tour, double& cost) const
{
    // 构建双向链表表示，O(1) 查找前后邻居。
    std::vector<int> next(static_cast<std::size_t>(n_));
    std::vector<int> prev(static_cast<std::size_t>(n_));
    for (int i = 0; i < n_; ++i) {
        int u = tour[static_cast<std::size_t>(i)];
        int v = tour[static_cast<std::size_t>((i + 1) % n_)];
        next[static_cast<std::size_t>(u)] = v;
        prev[static_cast<std::size_t>(v)] = u;
    }

    std::vector<bool> active(static_cast<std::size_t>(n_), true);
    bool improved = false;

    for (int pass = 0; pass < n_; ++pass) {
        bool pass_improved = false;
        for (int idx = 0; idx < n_; ++idx) {
            int t1 = tour[static_cast<std::size_t>(idx)];
            if (!active[static_cast<std::size_t>(t1)]) {
                continue;
            }

            bool found = false;
            // 两个方向都尝试：next[t1] 和 prev[t1] 作为 t2。
            int ngb[2] = {next[static_cast<std::size_t>(t1)],
                          prev[static_cast<std::size_t>(t1)]};
            for (int t2 : ngb) {
                if (lkSearch(t1, t2, next, prev, active)) {
                    improved = true;
                    pass_improved = true;
                    found = true;
                    // 从链表重建 tour。
                    int cur = 0;
                    for (int i = 0; i < n_; ++i) {
                        tour[static_cast<std::size_t>(i)] = cur;
                        cur = next[static_cast<std::size_t>(cur)];
                    }
                    cost = tourCost(tour);
                    break;
                }
            }

            if (!found) {
                active[static_cast<std::size_t>(t1)] = false;
            }
            // 找到改进则重置 don't-look 并重新扫描。
            if (found) {
                // active 已在 lkSearch 中部分重置，此处重置全部以安全。
                std::fill(active.begin(), active.end(), true);
                break;  // 跳出 idx 循环，从头开始本次 pass。
            }
        }
        if (!pass_improved) {
            break;  // 本轮无改进，LK 已收敛。
        }
    }

    return improved;
}

void BranchBoundSolver::doubleBridgeKick(std::vector<int>& tour) const
{
    if (n_ < 8) {
        return;
    }

    // 用基于 n 的确定性断点，避免引入随机数，确保可复现。
    const int a = 1 + (n_ / 7) % std::max(1, n_ / 4 - 1);
    const int b = a + 1 + (n_ / 5) % std::max(1, n_ / 4);
    const int c = b + 1 + (n_ / 3) % std::max(1, n_ / 4);
    if (c >= n_ - 1) {
        return;
    }

    // 四段：A=[0..a], B=[a+1..b], C=[b+1..c], D=[c+1..n_-1]。
    // Double-bridge 重排为 A + C + B + D。
    std::vector<int> kicked;
    kicked.reserve(static_cast<std::size_t>(n_));

    for (int i = 0; i <= a; ++i) {
        kicked.push_back(tour[static_cast<std::size_t>(i)]);
    }
    for (int i = b + 1; i <= c; ++i) {
        kicked.push_back(tour[static_cast<std::size_t>(i)]);
    }
    for (int i = a + 1; i <= b; ++i) {
        kicked.push_back(tour[static_cast<std::size_t>(i)]);
    }
    for (int i = c + 1; i < n_; ++i) {
        kicked.push_back(tour[static_cast<std::size_t>(i)]);
    }

    tour = std::move(kicked);
}

void BranchBoundSolver::linKernighan(std::vector<int>& tour, double& cost) const
{
    buildCandidateSets();

    std::vector<int> best_tour = tour;
    double best_cost = cost;

    for (int kick = 0; kick < kLkMaxKicks; ++kick) {
        // 连续 LK 改进直到局部最优。
        while (linKernighanImprove(tour, cost)) {
            if (cost + kEps < best_cost) {
                best_tour = tour;
                best_cost = cost;
            }
        }

        // 更新当前最优。
        if (cost + kEps < best_cost) {
            best_tour = tour;
            best_cost = cost;
        }

        if (kick == kLkMaxKicks - 1) {
            break;
        }

        // Double-bridge kick 跳出局部最优。
        doubleBridgeKick(tour);
        cost = tourCost(tour);

        if (cost + kEps < best_cost) {
            best_tour = tour;
            best_cost = cost;
        }
    }

    tour = std::move(best_tour);
    cost = best_cost;
}

namespace {

std::vector<std::string> tokenizeSectionLine(const std::string& line)
{
    std::istringstream in(line);
    std::vector<std::string> tokens;
    std::string token;
    while (in >> token) {
        if (upperCopy(token) == "EOF") {
            break;
        }
        tokens.push_back(token);
    }
    return tokens;
}

// TSPLIB 的 header line 可能是 "KEY: VALUE" 格式，也可能是 "KEY VALUE" 格式。这个函数解析出 KEY 和 VALUE，并把 KEY 转成大写、VALUE 去掉首尾空白。
std::pair<std::string, std::string> parseTsplibHeaderLine(const std::string& line)
{
    const std::size_t colon = line.find(':');
    if (colon != std::string::npos) {
        return {upperCopy(trimCopy(line.substr(0, colon))), trimCopy(line.substr(colon + 1))};
    }
    // 没有冒号时，按空白分隔 KEY 和 VALUE。
    std::istringstream in(line);
    std::string key;
    in >> key;
    std::string value;
    std::getline(in, value);
    return {upperCopy(key), trimCopy(value)};
}


std::vector<std::vector<double>> buildExplicitMatrix(
    int n,
    const std::string& format,
    const std::vector<double>& values)
{
    std::vector<std::vector<double>> matrix(static_cast<std::size_t>(n),
                                            std::vector<double>(static_cast<std::size_t>(n), 0.0));
    const std::string upper_format = upperCopy(format.empty() ? "FULL_MATRIX" : format);
    std::size_t cursor = 0;

    auto next_value = [&]() {
        if (cursor >= values.size()) {
            throw std::runtime_error("EDGE_WEIGHT_SECTION has fewer values than expected");
        }
        return values[cursor++];
    };

    if (upper_format == "FULL_MATRIX") {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j) {
                matrix[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = next_value();
            }
        }
    } else if (upper_format == "UPPER_ROW") {
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                const double value = next_value();
                matrix[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = value;
                matrix[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = value;
            }
        }
    } else if (upper_format == "UPPER_DIAG_ROW") {
        for (int i = 0; i < n; ++i) {
            for (int j = i; j < n; ++j) {
                const double value = next_value();
                matrix[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = value;
                matrix[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = value;
            }
        }
    } else if (upper_format == "LOWER_ROW") {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < i; ++j) {
                const double value = next_value();
                matrix[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = value;
                matrix[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = value;
            }
        }
    } else if (upper_format == "LOWER_DIAG_ROW") {
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j <= i; ++j) {
                const double value = next_value();
                matrix[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = value;
                matrix[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = value;
            }
        }
    } else if (upper_format == "UPPER_COL") {
        for (int j = 0; j < n; ++j) {
            for (int i = 0; i < j; ++i) {
                const double value = next_value();
                matrix[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = value;
                matrix[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = value;
            }
        }
    } else if (upper_format == "UPPER_DIAG_COL") {
        for (int j = 0; j < n; ++j) {
            for (int i = 0; i <= j; ++i) {
                const double value = next_value();
                matrix[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = value;
                matrix[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = value;
            }
        }
    } else if (upper_format == "LOWER_COL") {
        for (int j = 0; j < n; ++j) {
            for (int i = j + 1; i < n; ++i) {
                const double value = next_value();
                matrix[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = value;
                matrix[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = value;
            }
        }
    } else if (upper_format == "LOWER_DIAG_COL") {
        for (int j = 0; j < n; ++j) {
            for (int i = j; i < n; ++i) {
                const double value = next_value();
                matrix[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = value;
                matrix[static_cast<std::size_t>(j)][static_cast<std::size_t>(i)] = value;
            }
        }
    } else {
        throw std::runtime_error("unsupported EDGE_WEIGHT_FORMAT: " + format);
    }

    return matrix;
}

// 判断输入内容是否看起来像一个纯粹的距离矩阵（没有 TSPLIB 格式的 header 和 section 标记）。
bool looksLikePlainMatrix(const std::string& content)
{
    const std::string trimmed = trimCopy(content);
    if (trimmed.empty()) {
        return false;
    }
    const char first = trimmed.front();
    if (!std::isdigit(static_cast<unsigned char>(first)) && first != '+' && first != '-') {
        return false;
    }
    const std::size_t first_line_end = trimmed.find('\n');
    const std::string first_line = first_line_end == std::string::npos
        ? trimmed
        : trimmed.substr(0, first_line_end);
    
    return first_line.find(':') == std::string::npos;
}

} // namespace

TspProblem readTspProblem(std::istream& input)
{
    const std::string content((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
    if (looksLikePlainMatrix(content)) {
        std::istringstream matrix_input(content);
        TspProblem problem;
        problem.name = "matrix";
        problem.type = "TSP";
        problem.edge_weight_type = "EXPLICIT";
        problem.edge_weight_format = "FULL_MATRIX";
        problem.matrix = readDistanceMatrix(matrix_input);
         std::cout <<"debug:" << problem.matrix.size() << std::endl;
        return problem;
    }

    TspProblem problem;
    std::map<std::string, std::string> header;
    std::vector<double> edge_values;
    enum class Section {
        Header,
        NodeCoord,
        EdgeWeight,
        Ignored
    };
    Section section = Section::Header;

    std::istringstream lines(content);
    std::string line;
    while (std::getline(lines, line)) {
        
        line = trimCopy(line);
        if (line.empty()) {
            continue;
        }

        const std::string upper = upperCopy(line);
        std::cout <<"debug:" << upper << std::endl;
        if (upper == "EOF") {
            break;
        }
        if (upper == "NODE_COORD_SECTION") {
            section = Section::NodeCoord;
            continue;
        }
        if (upper == "EDGE_WEIGHT_SECTION") {
            section = Section::EdgeWeight;
            continue;
        }
        if (isSectionMarker(line)) {
            section = Section::Ignored;
            continue;
        }

        if (section == Section::Header) {
            auto [key, value] = parseTsplibHeaderLine(line);
            if (!key.empty()) {
                header[key] = value;
            }
        } else if (section == Section::NodeCoord) {
            const std::vector<std::string> tokens = tokenizeSectionLine(line);
            if (tokens.size() < 3) {
                continue;
            }
            const int id = std::stoi(tokens[0]);
            const int n = header.count("DIMENSION") ? std::stoi(header["DIMENSION"]) : id;
            if (static_cast<int>(problem.coordinates.size()) < n) {
                problem.coordinates.resize(static_cast<std::size_t>(n));
            }
            if (id < 1 || id > n) {
                throw std::runtime_error("NODE_COORD_SECTION contains an out-of-range node id");
            }
            Point point;
            point.x = std::stod(tokens[1]);
            point.y = std::stod(tokens[2]);
            if (tokens.size() >= 4) {
                point.z = std::stod(tokens[3]);
            }
            problem.coordinates[static_cast<std::size_t>(id - 1)] = point;
        } else if (section == Section::EdgeWeight) {
            std::cout <<"debug:" <<"EdgeWeight" << std::endl;
            const std::vector<std::string> tokens = tokenizeSectionLine(line);
            for (const std::string& token : tokens) {
                edge_values.push_back(parseWeight(token));
            }
        }
    }

    problem.name = header.count("NAME") ? header["NAME"] : "tsplib";
    problem.type = header.count("TYPE") ? upperCopy(header["TYPE"]) : "TSP";
    problem.edge_weight_type = header.count("EDGE_WEIGHT_TYPE")
        ? upperCopy(header["EDGE_WEIGHT_TYPE"])
        : (problem.coordinates.empty() ? "EXPLICIT" : "EUC_2D");
    problem.edge_weight_format = header.count("EDGE_WEIGHT_FORMAT")
        ? upperCopy(header["EDGE_WEIGHT_FORMAT"])
        : (problem.edge_weight_type == "EXPLICIT" ? "FULL_MATRIX" : "");

    const int dimension = header.count("DIMENSION") ? std::stoi(header["DIMENSION"]) : problem.dimension();
    if (dimension < 3) {
        throw std::runtime_error("TSP instance must contain at least 3 vertices");
    }

    if (problem.edge_weight_type == "EXPLICIT") {
        problem.matrix = buildExplicitMatrix(dimension, problem.edge_weight_format, edge_values);
        problem.coordinates.clear();
    } else if (static_cast<int>(problem.coordinates.size()) != dimension) {
        throw std::runtime_error("NODE_COORD_SECTION does not match DIMENSION");
    }

    return problem;
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
    //nxn的的距离矩阵，初始化为0.0
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

// 计算给定 TSP 回路的总成本，若回路无效（长度不为 n 或存在缺边）则返回 infinity。
namespace {

double problemTourCost(const TspProblem& problem, const std::vector<int>& tour)
{
    const int n = problem.dimension();
    if (static_cast<int>(tour.size()) != n) {
        return std::numeric_limits<double>::infinity();
    }

    double cost = 0.0;
    for (int i = 0; i < n; ++i) {
        const double edge = problem.distance(tour[static_cast<std::size_t>(i)],
                                             tour[static_cast<std::size_t>((i + 1) % n)]);
        if (!isFinite(edge)) {
            return std::numeric_limits<double>::infinity();
        }
        cost += edge;
    }
    return cost;
}

std::vector<int> sequentialTour(int n)
{
    std::vector<int> tour(static_cast<std::size_t>(n));
    std::iota(tour.begin(), tour.end(), 0);
    return tour;
}

// 从指定起点出发，按照最近邻策略构造一个 TSP 回路。起点不同可能得到不同的回路，返回所有起点中成本最低的那个回路。
std::vector<int> nearestNeighborTour(const TspProblem& problem, int start)
{
    const int n = problem.dimension();
    std::vector<int> tour;
    tour.reserve(static_cast<std::size_t>(n));
    std::vector<unsigned char> used(static_cast<std::size_t>(n), 0);

    int current = start;
    tour.push_back(current);
    used[static_cast<std::size_t>(current)] = 1;

    for (int step = 1; step < n; ++step) {
        int next = -1;
        double best = std::numeric_limits<double>::infinity();
        for (int v = 0; v < n; ++v) {
            if (used[static_cast<std::size_t>(v)]) {
                continue;
            }
            const double value = problem.distance(current, v);
            if (value < best) {
                best = value;
                next = v;
            }
        }
        if (next < 0 || !isFinite(best)) {
            return {};
        }
        current = next;
        used[static_cast<std::size_t>(current)] = 1;
        tour.push_back(current);
    }

    if (!isFinite(problem.distance(current, start))) {
        return {};
    }
    return tour;
}

std::uint64_t partBy1(std::uint32_t value)
{
    std::uint64_t result = value;
    result = (result | (result << 16U)) & 0x0000FFFF0000FFFFULL;
    result = (result | (result << 8U)) & 0x00FF00FF00FF00FFULL;
    result = (result | (result << 4U)) & 0x0F0F0F0F0F0F0F0FULL;
    result = (result | (result << 2U)) & 0x3333333333333333ULL;
    result = (result | (result << 1U)) & 0x5555555555555555ULL;
    return result;
}

std::vector<int> mortonTour(const TspProblem& problem)
{
    const int n = problem.dimension();
    if (!problem.hasCoordinates()) {
        return sequentialTour(n);
    }

    double min_x = problem.coordinates.front().x;
    double max_x = problem.coordinates.front().x;
    double min_y = problem.coordinates.front().y;
    double max_y = problem.coordinates.front().y;
    for (const Point& point : problem.coordinates) {
        min_x = std::min(min_x, point.x);
        max_x = std::max(max_x, point.x);
        min_y = std::min(min_y, point.y);
        max_y = std::max(max_y, point.y);
    }
    const double range_x = std::max(kEps, max_x - min_x);
    const double range_y = std::max(kEps, max_y - min_y);

    std::vector<std::pair<std::uint64_t, int>> keyed;
    keyed.reserve(static_cast<std::size_t>(n));
    for (int i = 0; i < n; ++i) {
        const Point& point = problem.coordinates[static_cast<std::size_t>(i)];
        const auto sx = static_cast<std::uint32_t>(
            std::max(0.0, std::min(65535.0, 65535.0 * (point.x - min_x) / range_x)));
        const auto sy = static_cast<std::uint32_t>(
            std::max(0.0, std::min(65535.0, 65535.0 * (point.y - min_y) / range_y)));
        const std::uint64_t key = partBy1(sx) | (partBy1(sy) << 1U);
        keyed.push_back({key, i});
    }
    std::sort(keyed.begin(), keyed.end(), [](const auto& lhs, const auto& rhs) {
        if (lhs.first != rhs.first) {
            return lhs.first < rhs.first;
        }
        return lhs.second < rhs.second;
    });

    std::vector<int> tour;
    tour.reserve(static_cast<std::size_t>(n));
    for (const auto& item : keyed) {
        tour.push_back(item.second);
    }
    return tour;
}

std::vector<int> initialHeuristicTour(const TspProblem& problem, const HeuristicOptions& options)
{
    const int n = problem.dimension();
    if (static_cast<std::size_t>(n) > options.nearest_scan_limit) {
        writeDebugLine(options.debug,
                       "initial tour: morton order because dimension exceeds nearest-scan limit");
        return mortonTour(problem);
    }

    std::vector<int> starts;
    starts.push_back(0);
    if (n > 4) {
        starts.push_back(n / 2);
        starts.push_back(n / 4);
        starts.push_back((3 * n) / 4);
    }

    std::mt19937 rng(options.seed);
    std::uniform_int_distribution<int> distribution(0, n - 1);
    while (starts.size() < options.starts) {
        starts.push_back(distribution(rng));
    }
    std::sort(starts.begin(), starts.end());
    starts.erase(std::unique(starts.begin(), starts.end()), starts.end());
    writeDebugLine(options.debug,
                   "initial tour: nearest-neighbor starts=" + std::to_string(starts.size()));

    double best_cost = std::numeric_limits<double>::infinity();
    std::vector<int> best_tour;
    const std::size_t debug_interval = normalizedDebugInterval(options.debug);
    std::size_t starts_done = 0;
    for (int start : starts) {
        std::vector<int> candidate = nearestNeighborTour(problem, start);
        ++starts_done;
        if (candidate.empty()) {
            if (options.debug.output != nullptr && starts_done % debug_interval == 0) {
                writeDebugLine(options.debug,
                               "initial tour progress: tried=" + std::to_string(starts_done)
                                   + " best=" + formatDebugDouble(best_cost));
            }
            continue;
        }
        const double cost = problemTourCost(problem, candidate);
        if (cost < best_cost) {
            best_cost = cost;
            best_tour = std::move(candidate);
            writeDebugLine(options.debug,
                           "initial tour improved: start=" + std::to_string(start)
                               + " cost=" + formatDebugDouble(best_cost));
        } else if (options.debug.output != nullptr && starts_done % debug_interval == 0) {
            writeDebugLine(options.debug,
                           "initial tour progress: tried=" + std::to_string(starts_done)
                               + " best=" + formatDebugDouble(best_cost));
        }
    }
    if (!best_tour.empty()) {
        return best_tour;
    }
    writeDebugLine(options.debug, "initial tour: nearest-neighbor failed; using sequential order");
    return sequentialTour(n);
}

void fullTwoOpt(const TspProblem& problem,
                std::vector<int>& tour,
                double& cost,
                std::size_t passes,
                const DebugOptions& debug)
{
    const int n = problem.dimension();
    for (std::size_t pass = 0; pass < passes; ++pass) {
        const double before = cost;
        bool improved = false;
        for (int i = 1; i < n - 1 && !improved; ++i) {
            for (int k = i + 1; k < n && !improved; ++k) {
                const int a = tour[static_cast<std::size_t>(i - 1)];
                const int b = tour[static_cast<std::size_t>(i)];
                const int c = tour[static_cast<std::size_t>(k)];
                const int d = tour[static_cast<std::size_t>((k + 1) % n)];
                const double ac = problem.distance(a, c);
                const double bd = problem.distance(b, d);
                if (!isFinite(ac) || !isFinite(bd)) {
                    continue;
                }
                const double delta = ac + bd - problem.distance(a, b) - problem.distance(c, d);
                if (delta < -kEps) {
                    std::reverse(tour.begin() + i, tour.begin() + k + 1);
                    cost += delta;
                    improved = true;
                }
            }
        }
        std::ostringstream line;
        line << "2-opt full pass=" << (pass + 1)
             << " improved=" << (improved ? "yes" : "no")
             << " cost=" << formatDebugDouble(cost)
             << " delta=" << formatDebugDouble(cost - before);
        writeDebugLine(debug, line.str());
        if (!improved) {
            break;
        }
    }
    cost = problemTourCost(problem, tour);
}

void windowTwoOpt(const TspProblem& problem,
                  std::vector<int>& tour,
                  double& cost,
                  std::size_t window,
                  std::size_t passes,
                  const DebugOptions& debug)
{
    const int n = problem.dimension();
    if (window == 0) {
        writeDebugLine(debug, "2-opt window skipped: window=0");
        return;
    }
    for (std::size_t pass = 0; pass < passes; ++pass) {
        const double before = cost;
        bool improved = false;
        for (int i = 1; i < n - 1; ++i) {
            const int upper = std::min(n - 1, i + static_cast<int>(window));
            for (int k = i + 1; k <= upper; ++k) {
                const int a = tour[static_cast<std::size_t>(i - 1)];
                const int b = tour[static_cast<std::size_t>(i)];
                const int c = tour[static_cast<std::size_t>(k)];
                const int d = tour[static_cast<std::size_t>((k + 1) % n)];
                const double ac = problem.distance(a, c);
                const double bd = problem.distance(b, d);
                if (!isFinite(ac) || !isFinite(bd)) {
                    continue;
                }
                const double delta = ac + bd - problem.distance(a, b) - problem.distance(c, d);
                if (delta < -kEps) {
                    std::reverse(tour.begin() + i, tour.begin() + k + 1);
                    cost += delta;
                    improved = true;
                    break;
                }
            }
        }
        std::ostringstream line;
        line << "2-opt window pass=" << (pass + 1)
             << " improved=" << (improved ? "yes" : "no")
             << " cost=" << formatDebugDouble(cost)
             << " delta=" << formatDebugDouble(cost - before);
        writeDebugLine(debug, line.str());
        if (!improved) {
            break;
        }
    }
    cost = problemTourCost(problem, tour);
}

} // namespace

SolveResult solveHeuristic(const TspProblem& problem, const HeuristicOptions& options)
{
    SolveResult result;
    const int n = problem.dimension();
    writeDebugLine(options.debug, "heuristic solve started: vertices=" + std::to_string(n));
    if (n < 3) {
        result.cost = std::numeric_limits<double>::infinity();
        writeDebugLine(options.debug, "heuristic solve stopped: fewer than 3 vertices");
        return result;
    }

    std::vector<int> tour = initialHeuristicTour(problem, options);
    double cost = problemTourCost(problem, tour);
    if (!isFinite(cost)) {
        result.cost = std::numeric_limits<double>::infinity();
        writeDebugLine(options.debug, "heuristic solve stopped: initial tour is infeasible");
        return result;
    }
    writeDebugLine(options.debug, "heuristic initial cost=" + formatDebugDouble(cost));

    if (static_cast<std::size_t>(n) <= options.full_two_opt_limit) {
        writeDebugLine(options.debug,
                       "2-opt mode: full passes=" + std::to_string(options.two_opt_passes));
        fullTwoOpt(problem, tour, cost, options.two_opt_passes, options.debug);
    } else {
        writeDebugLine(options.debug,
                       "2-opt mode: window size=" + std::to_string(options.two_opt_window)
                           + " passes=" + std::to_string(options.two_opt_passes));
        windowTwoOpt(problem, tour, cost, options.two_opt_window, options.two_opt_passes, options.debug);
    }

    result.feasible = isFinite(cost);
    result.cost = cost;
    result.tour = std::move(tour);
    writeDebugLine(options.debug,
                   "heuristic solve finished: feasible=" + std::string(result.feasible ? "yes" : "no")
                       + " cost=" + formatDebugDouble(result.cost));
    return result;
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
