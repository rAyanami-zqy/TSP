#include "TspSolver.hpp"

#include <algorithm>
#include <array>
#include <chrono>
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
#include <utility>

#if defined(_MSC_VER)
#include <intrin.h>
#endif

namespace tsp {
namespace {

// 精确求解内部的 tour 构造/LK 只接受超过该阈值的启发式改善；
// 正确性相关的下界剪枝使用后面的尺度化舍入保护，不使用此常量。
constexpr double kHeuristicEps = 1e-9;
constexpr double kPi = 3.141592653589793238462643383279502884;

// 统一判断边权是否可用。缺边在输入中会被解析成 infinity。
bool isFinite(double value)
{
    return std::isfinite(value);
}

// value 必须非 0。优先使用编译器位扫描指令；未知编译器保留可移植回退。
unsigned trailingZeroCount(std::uint64_t value)
{
#if defined(_MSC_VER) && (defined(_M_X64) || defined(_M_ARM64))
    unsigned long bit_index = 0;
    _BitScanForward64(&bit_index, value);
    return static_cast<unsigned>(bit_index);
#elif defined(_MSC_VER)
    unsigned long bit_index = 0;
    if (_BitScanForward(&bit_index, static_cast<unsigned long>(value))) {
        return static_cast<unsigned>(bit_index);
    }
    _BitScanForward(&bit_index, static_cast<unsigned long>(value >> 32));
    return static_cast<unsigned>(bit_index + 32);
#elif defined(__clang__) || defined(__GNUC__)
    return static_cast<unsigned>(__builtin_ctzll(value));
#else
    unsigned bit_index = 0;
    while ((value & std::uint64_t{1}) == 0) {
        value >>= 1;
        ++bit_index;
    }
    return bit_index;
#endif
}

unsigned populationCount(std::uint64_t value)
{
#if defined(__clang__) || defined(__GNUC__)
    return static_cast<unsigned>(__builtin_popcountll(value));
#else
    unsigned count = 0;
    while (value != 0) {
        value &= value - 1;
        ++count;
    }
    return count;
#endif
}

// n 项非负 double 求和的舍入误差随数值尺度和项数增长。该容差没有固定
// 绝对下限，因此把所有边权整体缩放后，比较结论不会被 1e-9 一类常量改变。
double scaledRoundoffTolerance(double a, double b, std::size_t term_count)
{
    if (!isFinite(a) || !isFinite(b)) {
        return 0.0;
    }
    const double scale = std::max(std::fabs(a), std::fabs(b));
    const double factor = 32.0 * static_cast<double>(std::max<std::size_t>(term_count, 1));
    const double relative_unit = std::numeric_limits<double>::epsilon() * scale;
    const double next = std::nextafter(scale, std::numeric_limits<double>::infinity());
    const double ulp = isFinite(next)
        ? next - scale
        : scale - std::nextafter(scale, 0.0);
    // epsilon*scale 会在 subnormal 区间下溢为 0；至少保留一个实际 ULP。
    return factor * std::max(relative_unit, ulp);
}

bool costsNumericallyEqual(double a, double b, std::size_t term_count)
{
    if (!isFinite(a) || !isFinite(b)) {
        return a == b;
    }
    return std::fabs(a - b) <= scaledRoundoffTolerance(a, b, term_count);
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

#include "TspProblemText.ipp"

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

#include "TspProblemWeight.ipp"

// 近似比较两个边权是否相等，考虑到输入中的微小误差和计算中的浮点误差。
bool sameWeight(double a, double b)
{
    // 输入矩阵是算法约束本身，不是内部求和缓存；两方向必须是完全相同的
    // double。否则分支边和 tourCost 可能读取不同权重，破坏对称下界语义。
    return a == b;
}

#include "TspProblemCoordinate.ipp"

} // namespace

#include "TspProblemModel.ipp"

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

    // 非负整数 double 在 2^53 以内可精确表示；若任意 n 条边之和仍在
    // 此范围，1-tree 与 tour 的每一步求和也都是精确整数，可放心剪等值界。
    exact_integer_costs_ = true;
    const std::uint64_t exact_integer_limit = std::uint64_t{1} << 53;
    const double max_safe_edge = static_cast<double>(
        exact_integer_limit / static_cast<std::uint64_t>(n_));
    for (int i = 0; i < n_ && exact_integer_costs_; ++i) {
        for (int j = i + 1; j < n_; ++j) {
            const double weight = dist_[i][j];
            if (isFinite(weight)
                && (std::floor(weight) != weight
                    || weight > max_safe_edge)) {
                exact_integer_costs_ = false;
                break;
            }
        }
    }

    double smallest_positive = std::numeric_limits<double>::infinity();
    double largest_weight = 0.0;
    for (int i = 0; i < n_; ++i) {
        for (int j = i + 1; j < n_; ++j) {
            const double weight = dist_[i][j];
            if (!isFinite(weight)) continue;
            if (weight > 0.0) {
                smallest_positive = std::min(smallest_positive, weight);
            }
            largest_weight = std::max(largest_weight, weight);
        }
    }
    potential_ascent_numerically_safe_ =
        !isFinite(smallest_positive)
        || largest_weight / smallest_positive <= 1e12;
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

void BranchBoundSolver::setRootAscentStrategy(RootAscentStrategy strategy)
{
    root_ascent_strategy_ = strategy;
}

void BranchBoundSolver::setNodeAscentStrategy(NodeAscentStrategy strategy)
{
    node_ascent_strategy_ = strategy;
}

void BranchBoundSolver::setBranchEdgeOrder(BranchEdgeOrder order)
{
    // 此处只记录策略。root alpha 依赖最终根势和根 1-tree；root frequency
    // 依赖根势上升各轮 1-tree。两者都必须延迟到每个根搜索 epoch 构建。
    branch_edge_order_ = order;
}

void BranchBoundSolver::setPotentialUpdateOptions(
    PotentialUpdateStrategy strategy, std::size_t depth,
    std::size_t iterations, double gap_ratio, std::size_t budget)
{
    if (strategy != PotentialUpdateStrategy::None
        && (depth == 0 || iterations == 0 || budget == 0)) {
        throw std::invalid_argument(
            "potential update depth, iterations, and budget must be positive");
    }
    if (!isFinite(gap_ratio) || gap_ratio < 0.0) {
        throw std::invalid_argument(
            "potential update gap ratio must be finite and non-negative");
    }
    potential_update_strategy_ = strategy;
    potential_update_depth_ = depth;
    potential_update_iterations_ = iterations;
    potential_update_gap_ratio_ = gap_ratio;
    potential_update_budget_ = budget;
}

void BranchBoundSolver::setRootBoundOnly(bool enabled)
{
    root_bound_only_ = enabled;
}

double BranchBoundSolver::adjustedEdgeWeight(int u, int v) const
{
    const double original = dist_[u][v];
    if (!isFinite(original)
        || vertex_potential_.size() != static_cast<std::size_t>(n_)) {
        return original;
    }
    return original
        + vertex_potential_[static_cast<std::size_t>(u)]
        + vertex_potential_[static_cast<std::size_t>(v)];
}

void BranchBoundSolver::buildRootAlphaNearness(const OneTree& root_tree)
{
    // edgeId 使用 u*n+v 的稠密下标，因此所有按边索引的状态统一分配 n²
    // 个槽位；虽然只使用无向边的上三角位置，但可避免额外哈希和分支。
    const std::size_t state_size = static_cast<std::size_t>(n_)
        * static_cast<std::size_t>(n_);
    const double infinity = std::numeric_limits<double>::infinity();
    // 表使用稳定的 edgeId 索引。根 1-tree 中的边 alpha=0；缺边或无法
    // 计算替换路径的边保持 infinity，使比较器仍能通过后备规则稳定排序。
    root_alpha_by_edge_id_.assign(state_size, infinity);

    struct TreeArc {
        // 内部根 MST 邻接表的一条有向弧；每条无向树边存为两条弧。
        int to = -1;
        // 根势下的调整权重，不是原始距离。
        double weight = 0.0;
    };
    // tree_adjacency 排除顶点 0，只描述顶点 1..n-1 上的 MST。
    std::vector<std::vector<TreeArc>> tree_adjacency(
        static_cast<std::size_t>(n_));
    // 以 edgeId 为下标，标记边是否属于当前根 1-tree。
    std::vector<unsigned char> in_root_tree(state_size, 0);
    // 根 1-tree 已选两条根边中的较大调整权重；用于计算第三条根边的
    // 强制加入代价。若根树不完整则保持 -infinity。
    double largest_selected_root_weight =
        -std::numeric_limits<double>::infinity();

    for (const Edge& edge : root_tree.edges) {
        const std::size_t id = edgeId(edge.u, edge.v);
        in_root_tree[id] = 1;
        root_alpha_by_edge_id_[id] = 0.0;
        if (edge.u == 0 || edge.v == 0) {
            largest_selected_root_weight = std::max(
                largest_selected_root_weight, edge.w);
            continue;
        }
        tree_adjacency[static_cast<std::size_t>(edge.u)].push_back(
            TreeArc{edge.v, edge.w});
        tree_adjacency[static_cast<std::size_t>(edge.v)].push_back(
            TreeArc{edge.u, edge.w});
    }

    // parent 同时保存 DFS 父节点并充当 visited 标记；path_max[v] 是从本轮
    // source 到 v 的唯一树路径上最大调整边权；stack 是无递归 DFS 栈。
    std::vector<int> parent(static_cast<std::size_t>(n_), -1);
    std::vector<double> path_max(
        static_cast<std::size_t>(n_),
        -std::numeric_limits<double>::infinity());
    std::vector<int> stack;
    stack.reserve(static_cast<std::size_t>(n_));

    // 顶点 1..n-1 上的根 MST 是一棵树。从每个源点遍历一次，在 O(n^2)
    // 内得到所有树路径最大边；该一次性预处理不会进入 DFS 热路径。
    for (int source = 1; source < n_; ++source) {
        std::fill(parent.begin(), parent.end(), -1);
        std::fill(path_max.begin(), path_max.end(),
                  -std::numeric_limits<double>::infinity());
        stack.clear();
        parent[static_cast<std::size_t>(source)] = source;
        stack.push_back(source);
        while (!stack.empty()) {
            const int u = stack.back();
            stack.pop_back();
            for (const TreeArc& arc :
                 tree_adjacency[static_cast<std::size_t>(u)]) {
                if (parent[static_cast<std::size_t>(arc.to)] >= 0) continue;
                parent[static_cast<std::size_t>(arc.to)] = u;
                path_max[static_cast<std::size_t>(arc.to)] = std::max(
                    path_max[static_cast<std::size_t>(u)], arc.weight);
                stack.push_back(arc.to);
            }
        }

        for (int target = source + 1; target < n_; ++target) {
            const std::size_t id = edgeId(source, target);
            if (in_root_tree[id] || !isFinite(dist_[source][target])) continue;
            // beta 是加入该非树边后形成环中可删除的最重树边权。
            const double beta = path_max[static_cast<std::size_t>(target)];
            if (!isFinite(beta)) continue;
            // 强制内部非树边 (source,target) 后，删去根 MST 路径上的最大边
            // 即可恢复生成树；两者调整权重之差就是该边的 alpha-nearness。
            const double alpha = std::max(
                0.0, adjustedEdgeWeight(source, target) - beta);
            root_alpha_by_edge_id_[id] = alpha;
        }
    }

    if (isFinite(largest_selected_root_weight)) {
        for (int vertex = 1; vertex < n_; ++vertex) {
            const std::size_t id = edgeId(0, vertex);
            if (!isFinite(dist_[0][vertex]) || in_root_tree[id]) continue;
            // 1-tree 必须保留两条根边。强制一条未选根边时，最优交换是替换
            // 当前两条根边中较重的一条，因此代价增量可直接由两者之差得到。
            const double alpha = std::max(
                0.0, adjustedEdgeWeight(0, vertex)
                    - largest_selected_root_weight);
            root_alpha_by_edge_id_[id] = alpha;
        }
    }
}

void BranchBoundSolver::optimizeRootPotentials(double upper_bound)
{
    // 用于统计整个根势优化阶段（包括所有候选策略）的墙钟时间。
    const auto started_at = std::chrono::steady_clock::now();
    // 频率属于一次根势 epoch；即使本次上升提前退出，也不能沿用上次根
    // 搜索或上一个 solve() 的样本。
    root_one_tree_edge_counts_.clear();
    root_one_tree_sample_count_ = 0;
    const bool collect_root_one_tree_frequency = branch_edge_order_
        == BranchEdgeOrder::RootOneTreeFrequencyMiddle;
    const std::size_t edge_state_size = static_cast<std::size_t>(n_)
        * static_cast<std::size_t>(n_);
    vertex_potential_.assign(static_cast<std::size_t>(n_), 0.0);
    potential_correction_ = 0.0;
    potential_roundoff_guard_ = 0.0;

    auto strategy_name = [](RootAscentStrategy strategy) {
        switch (strategy) {
        case RootAscentStrategy::None: return "none";
        case RootAscentStrategy::Polyak: return "polyak";
        case RootAscentStrategy::Helsgaun: return "helsgaun";
        case RootAscentStrategy::Hybrid: return "hybrid";
        }
        return "unknown";
    };
    auto write_summary = [&](const std::string& selected,
                             int polyak_iterations,
                             int helsgaun_iterations,
                             double polyak_bound,
                             double helsgaun_bound) {
        if (debug_.output == nullptr) return;
        const double seconds = std::chrono::duration<double>(
            std::chrono::steady_clock::now() - started_at).count();
        std::ostringstream line;
        line << "root ascent: strategy=" << strategy_name(root_ascent_strategy_)
             << " selected=" << selected
             << " polyak_iterations=" << polyak_iterations
             << " helsgaun_iterations=" << helsgaun_iterations
             << " polyak_bound=" << formatDebugDouble(polyak_bound)
             << " helsgaun_bound=" << formatDebugDouble(helsgaun_bound)
             << " seconds=" << formatDebugDouble(seconds);
        if (collect_root_one_tree_frequency) {
            line << " frequency_samples=" << root_one_tree_sample_count_;
        }
        writeDebugLine(debug_, line.str());
    };

    if (root_ascent_strategy_ == RootAscentStrategy::None
        || !isFinite(upper_bound)) {
        write_summary("none", 0, 0,
                      -std::numeric_limits<double>::infinity(),
                      -std::numeric_limits<double>::infinity());
        return;
    }

    // 极端动态范围会让修改权重与势修正发生灾难性消减；这类输入保留
    // 原始 1-tree，既避免不可靠下界，也维持混合数量级回归路径。
    if (!potential_ascent_numerically_safe_) {
        write_summary("none-dynamic-range", 0, 0,
                      -std::numeric_limits<double>::infinity(),
                      -std::numeric_limits<double>::infinity());
        return;
    }

    struct OneTreeEvaluation {
        // false 表示调整权重图无法形成完整 1-tree。
        bool feasible = false;
        // 已减去 2*sum(pi) 的原问题下界。
        double bound = -std::numeric_limits<double>::infinity();
        // 本次 1-tree 的顶点度数；degree[v]-2 是势的次梯度分量。
        std::vector<int> degree;
        // 仅 middle-frequency 实验使用：本轮恰好 n 条 1-tree 边的稳定
        // edgeId。默认策略不填充，避免改变根势基线的分配与热路径。
        std::vector<std::size_t> selected_edge_ids;
    };

    // evaluate 多次复用的内部边缓冲区，只包含顶点 1..n-1 的边。
    std::vector<Edge> internal_edges;
    internal_edges.reserve(
        static_cast<std::size_t>(n_ - 1) * static_cast<std::size_t>(n_ - 2) / 2);
    auto evaluate = [&](const std::vector<double>& potentials) {
        OneTreeEvaluation evaluation;
        evaluation.degree.assign(static_cast<std::size_t>(n_), 0);
        if (collect_root_one_tree_frequency) {
            evaluation.selected_edge_ids.reserve(static_cast<std::size_t>(n_));
        }
        internal_edges.clear();
        for (int u = 1; u < n_; ++u) {
            for (int v = u + 1; v < n_; ++v) {
                if (!isFinite(dist_[u][v])) continue;
                internal_edges.push_back(Edge{
                    u, v,
                    dist_[u][v]
                        + potentials[static_cast<std::size_t>(u)]
                        + potentials[static_cast<std::size_t>(v)]});
            }
        }
        std::sort(internal_edges.begin(), internal_edges.end(),
                  [](const Edge& a, const Edge& b) { return a.w < b.w; });

        // components 用于本轮 Kruskal；mst_edge_count 的目标值是 n-2。
        // modified_cost 累加调整权重，最后还需减去势修正量。
        DisjointSet components(n_);
        int mst_edge_count = 0;
        double modified_cost = 0.0;
        for (const Edge& edge : internal_edges) {
            if (!components.unite(edge.u, edge.v)) continue;
            modified_cost += edge.w;
            ++evaluation.degree[static_cast<std::size_t>(edge.u)];
            ++evaluation.degree[static_cast<std::size_t>(edge.v)];
            if (collect_root_one_tree_frequency) {
                evaluation.selected_edge_ids.push_back(
                    edgeId(edge.u, edge.v));
            }
            if (++mst_edge_count == n_ - 2) break;
        }
        if (mst_edge_count != n_ - 2) return evaluation;

        // root_edges 始终保存目前扫描到的两条最轻根边，按权重升序排列。
        std::array<Edge, 2> root_edges{};
        std::size_t root_edge_count = 0;
        for (int vertex = 1; vertex < n_; ++vertex) {
            if (!isFinite(dist_[0][vertex])) continue;
            const Edge edge{
                0, vertex,
                dist_[0][vertex]
                    + potentials[0]
                    + potentials[static_cast<std::size_t>(vertex)]};
            if (root_edge_count < root_edges.size()) {
                root_edges[root_edge_count++] = edge;
                if (root_edge_count == root_edges.size()
                    && root_edges[1].w < root_edges[0].w) {
                    std::swap(root_edges[0], root_edges[1]);
                }
            } else if (edge.w < root_edges[1].w) {
                root_edges[1] = edge;
                if (root_edges[1].w < root_edges[0].w) {
                    std::swap(root_edges[0], root_edges[1]);
                }
            }
        }
        if (root_edge_count != root_edges.size()) return evaluation;
        for (const Edge& edge : root_edges) {
            modified_cost += edge.w;
            ++evaluation.degree[static_cast<std::size_t>(edge.u)];
            ++evaluation.degree[static_cast<std::size_t>(edge.v)];
            if (collect_root_one_tree_frequency) {
                evaluation.selected_edge_ids.push_back(
                    edgeId(edge.u, edge.v));
            }
        }

        const double correction = 2.0
            * std::accumulate(potentials.begin(), potentials.end(), 0.0);
        evaluation.feasible = true;
        evaluation.bound = modified_cost - correction;
        return evaluation;
    };

    struct AscentCandidate {
        // 某一上升策略迄今取得最强根下界时对应的势，而不是最后一轮势。
        std::vector<double> potentials;
        double bound = -std::numeric_limits<double>::infinity();
        // 实际完成的 1-tree 评估次数，用于实验统计。
        int iterations = 0;
        // 与本策略所有可行评估同步累计的选边次数；只在 frequency-middle
        // 模式分配，选中 Polyak/Helsgaun 后直接安装对应一份统计。
        std::vector<std::uint32_t> edge_counts;
        std::uint32_t frequency_samples = 0;
    };

    auto record_frequency = [&](AscentCandidate& candidate,
                                const OneTreeEvaluation& evaluation) {
        if (!collect_root_one_tree_frequency || !evaluation.feasible) return;
        if (candidate.edge_counts.empty()) {
            candidate.edge_counts.assign(edge_state_size, 0);
        }
        ++candidate.frequency_samples;
        for (const std::size_t id : evaluation.selected_edge_ids) {
            ++candidate.edge_counts[id];
        }
    };

    auto save_if_better = [&](AscentCandidate& best,
                              const std::vector<double>& potentials,
                              double bound) {
        const double tolerance = scaledRoundoffTolerance(
            bound, upper_bound, static_cast<std::size_t>(n_));
        if (!isFinite(best.bound) || bound > best.bound + tolerance) {
            best.bound = bound;
            best.potentials = potentials;
            return true;
        }
        return false;
    };

    constexpr int kMaxIterations = 400;
    auto run_polyak = [&](const std::vector<double>& initial) {
        // potentials 是正在迭代的工作势；best 单独保存历史最好证书。
        std::vector<double> potentials = initial;
        AscentCandidate best;
        best.potentials = initial;
        // step_scale 是 Polyak 步长乘子；连续停滞后折半。
        double step_scale = 2.0;
        // 自上次改善 best.bound 起连续无改善的轮数。
        int no_improvement = 0;
        constexpr int kStagnationIterations = 12;
        constexpr int kMinIterationsBeforeGapStop = 100;

        for (int iteration = 0; iteration < kMaxIterations; ++iteration) {
            const OneTreeEvaluation evaluation = evaluate(potentials);
            ++best.iterations;
            if (!evaluation.feasible) break;
            record_frequency(best, evaluation);
            const double bound = evaluation.bound;
            if (save_if_better(best, potentials, bound)) {
                no_improvement = 0;
            } else {
                ++no_improvement;
            }

            double subgradient_norm = 0.0;
            for (const int value : evaluation.degree) {
                const double deviation = static_cast<double>(value - 2);
                subgradient_norm += deviation * deviation;
            }
            const double tolerance = scaledRoundoffTolerance(
                bound, upper_bound, static_cast<std::size_t>(n_));
            if (subgradient_norm == 0.0
                || bound >= upper_bound - tolerance) {
                break;
            }
            if (iteration + 1 >= kMinIterationsBeforeGapStop
                && upper_bound - best.bound
                    <= 0.01 * std::fabs(upper_bound)) {
                break;
            }

            const double gap = upper_bound - bound;
            const double step = step_scale * gap / subgradient_norm;
            if (!isFinite(step) || step <= 0.0) break;
            for (int vertex = 0; vertex < n_; ++vertex) {
                potentials[static_cast<std::size_t>(vertex)] += step
                    * static_cast<double>(
                        evaluation.degree[static_cast<std::size_t>(vertex)] - 2);
            }

            if (no_improvement >= kStagnationIterations) {
                step_scale *= 0.5;
                no_improvement = 0;
                if (step_scale < 1e-5) break;
            }
        }
        return best;
    };

    auto run_helsgaun = [&](const std::vector<double>& initial) {
        std::vector<double> potentials = initial;
        // 上一轮次梯度用于 0.7/0.3 平滑，降低方向振荡。
        std::vector<double> previous_subgradient(
            static_cast<std::size_t>(n_), 0.0);
        bool have_previous = false;
        double previous_bound = -std::numeric_limits<double>::infinity();
        // step 是当前 period 的基础步长；period 是该步长持续的轮数。
        double step = 1.0;
        int period = std::max(1, n_ / 2);
        // position_in_period 从 0 计数；结束一个 period 后步长和周期更新。
        int position_in_period = 0;
        bool first_period = true;
        bool doubling_step = true;
        AscentCandidate best;
        best.potentials = initial;

        while (best.iterations < kMaxIterations
               && period > 0 && step > 0.0 && isFinite(step)) {
            const OneTreeEvaluation evaluation = evaluate(potentials);
            ++best.iterations;
            if (!evaluation.feasible) break;
            record_frequency(best, evaluation);

            const double bound = evaluation.bound;
            const bool improved_best = save_if_better(best, potentials, bound);
            double subgradient_norm = 0.0;
            std::vector<double> subgradient(static_cast<std::size_t>(n_), 0.0);
            for (int vertex = 0; vertex < n_; ++vertex) {
                const double value = static_cast<double>(
                    evaluation.degree[static_cast<std::size_t>(vertex)] - 2);
                subgradient[static_cast<std::size_t>(vertex)] = value;
                subgradient_norm += value * value;
            }
            const double tolerance = scaledRoundoffTolerance(
                bound, upper_bound, static_cast<std::size_t>(n_));
            if (subgradient_norm == 0.0
                || bound >= upper_bound - tolerance) {
                break;
            }

            // Helsgaun 第一个 period 中从 t=1 开始，只要 W 继续增长就
            // 把步长翻倍；第一次不增长后，本 period 剩余迭代保持不变。
            if (first_period && have_previous && doubling_step) {
                const double previous_tolerance = scaledRoundoffTolerance(
                    bound, previous_bound, static_cast<std::size_t>(n_));
                if (bound > previous_bound + previous_tolerance
                    && isFinite(step * 2.0)) {
                    step *= 2.0;
                } else {
                    doubling_step = false;
                }
            }

            for (int vertex = 0; vertex < n_; ++vertex) {
                const std::size_t index = static_cast<std::size_t>(vertex);
                const double direction = have_previous
                    ? 0.7 * subgradient[index]
                        + 0.3 * previous_subgradient[index]
                    : subgradient[index];
                potentials[index] += step * direction;
            }

            previous_subgradient = std::move(subgradient);
            previous_bound = bound;
            have_previous = true;
            ++position_in_period;
            if (position_in_period >= period) {
                int next_period = period / 2;
                step *= 0.5;
                // 论文在 period 最后一次迭代仍提高 W 时把 period 加倍；
                // 与常规减半合并后，相当于保留当前长度而不是立即收缩。
                if (improved_best && next_period > 0) {
                    next_period *= 2;
                }
                period = next_period;
                position_in_period = 0;
                first_period = false;
            }
        }
        return best;
    };

    const std::vector<double> zero_potentials(static_cast<std::size_t>(n_), 0.0);
    AscentCandidate polyak;
    AscentCandidate helsgaun;
    AscentCandidate selected;
    std::string selected_name;
    if (root_ascent_strategy_ == RootAscentStrategy::Polyak) {
        polyak = run_polyak(zero_potentials);
        selected = polyak;
        selected_name = "polyak";
    } else if (root_ascent_strategy_ == RootAscentStrategy::Helsgaun) {
        helsgaun = run_helsgaun(zero_potentials);
        selected = helsgaun;
        selected_name = "helsgaun";
    } else {
        polyak = run_polyak(zero_potentials);
        // 组合策略不是简单平均两组势。它从当前 Polyak 最优解继续执行
        // 论文的平滑 period 上升，并始终保留固定根下界更强的一组势。
        helsgaun = run_helsgaun(polyak.potentials);
        const double tolerance = scaledRoundoffTolerance(
            helsgaun.bound, polyak.bound, static_cast<std::size_t>(n_));
        if (helsgaun.bound > polyak.bound + tolerance) {
            selected = helsgaun;
            selected_name = "polyak+helsgaun";
        } else {
            selected = polyak;
            selected_name = "polyak";
        }
    }

    if (!isFinite(selected.bound)
        || selected.potentials.size() != static_cast<std::size_t>(n_)) {
        write_summary("none-infeasible",
                      polyak.iterations, helsgaun.iterations,
                      polyak.bound, helsgaun.bound);
        return;
    }

    vertex_potential_ = std::move(selected.potentials);
    if (collect_root_one_tree_frequency
        && selected.frequency_samples != 0) {
        root_one_tree_edge_counts_ = std::move(selected.edge_counts);
        root_one_tree_sample_count_ = selected.frequency_samples;
    }
    potential_correction_ = 2.0
        * std::accumulate(vertex_potential_.begin(), vertex_potential_.end(), 0.0);
    const bool has_nonzero_potential = std::any_of(
        vertex_potential_.begin(), vertex_potential_.end(),
        [](double value) { return value != 0.0; });
    if (has_nonzero_potential) {
        double largest_adjusted_weight = 0.0;
        for (int u = 0; u < n_; ++u) {
            for (int v = u + 1; v < n_; ++v) {
                if (!isFinite(dist_[u][v])) continue;
                largest_adjusted_weight = std::max(
                    largest_adjusted_weight,
                    std::fabs(adjustedEdgeWeight(u, v)));
            }
        }
        const double modified_sum_scale =
            static_cast<double>(n_) * largest_adjusted_weight;
        potential_roundoff_guard_ = scaledRoundoffTolerance(
            modified_sum_scale, potential_correction_,
            static_cast<std::size_t>(n_));
    }
    write_summary(selected_name,
                  polyak.iterations, helsgaun.iterations,
                  polyak.bound, helsgaun.bound);
}

bool BranchBoundSolver::usesPersistentPotentialUpdates() const
{
    return potential_update_strategy_ == PotentialUpdateStrategy::SubtreeDepth
        || potential_update_strategy_ == PotentialUpdateStrategy::SubtreeAdaptive;
}

bool BranchBoundSolver::shouldUpdatePotentials(
    const OneTree& tree, int depth, double bound, double upper_bound) const
{
    // 初始精确搜索同时承担 diversified-LK 的困难度探测；若稍后改善
    // incumbent，该轮会整体回滚。先给它一个保守预算，探测完成或重启后
    // 再开放完整用户预算，避免在注定丢弃的轮次做大量局部上升。
    // active_budget 是当前根搜索轮次实际可用的尝试上限，不是剩余次数。
    const std::size_t active_budget =
        !diversified_tour_attempted_ && !initial_tour_alternatives_.empty()
        ? std::min<std::size_t>(potential_update_budget_, 1000)
        : potential_update_budget_;
    if (potential_update_strategy_ == PotentialUpdateStrategy::None
        || depth <= 0 || potential_update_depth_ == 0
        || potential_update_iterations_ == 0
        || potential_updates_in_round_ >= active_budget
        || !potential_ascent_numerically_safe_
        || !tree.feasible || !isFinite(bound) || !isFinite(upper_bound)) {
        return false;
    }

    // violation_norm = Σ(degree[v]-2)²；为 0 表示 1-tree 已满足 tour 度约束，
    // 此时没有可用于更新势的非零次梯度。
    double violation_norm = 0.0;
    for (const int degree : tree.degree) {
        const double deviation = static_cast<double>(degree - 2);
        violation_norm += deviation * deviation;
    }
    if (violation_norm == 0.0) return false;

    if (potential_update_strategy_ == PotentialUpdateStrategy::Depth) {
        return static_cast<std::size_t>(depth) % potential_update_depth_ == 0;
    }

    if (usesPersistentPotentialUpdates()) {
        // depth_since_epoch 防止在刚安装新势的相邻层立即再次重建整个子树状态。
        const int depth_since_epoch = depth - current_potential_epoch_depth_;
        if (depth_since_epoch < 0
            || static_cast<std::size_t>(depth_since_epoch)
                < potential_update_depth_) {
            return false;
        }
        if (potential_update_strategy_ == PotentialUpdateStrategy::SubtreeDepth) {
            return true;
        }
    }

    // scale 避免 UB 接近 0 时除数退化；relative_gap 是无量纲触发指标。
    const double scale = std::max(1.0, std::fabs(upper_bound));
    const double relative_gap = std::max(0.0, upper_bound - bound) / scale;
    return static_cast<std::size_t>(depth) >= potential_update_depth_
        && relative_gap <= potential_update_gap_ratio_;
}

BranchBoundSolver::NodePotentialUpdateResult
BranchBoundSolver::updateNodePotentialBound(
    const PartialSol& node, double current_bound,
    double upper_bound, std::size_t max_iterations) const
{
    struct Evaluation {
        // 单组临时势在当前 forced/forbidden 约束下得到的 1-tree 证书。
        bool feasible = false;
        double bound = -std::numeric_limits<double>::infinity();
        // degree[v]-2 在外层被用作本轮次梯度。
        std::vector<int> degree;
    };

    // 节点约束在一次势上升期间保持不变。预先收缩所有非根 forced 边，
    // 后续每轮只需在当前有效边构成的分量图上运行 Prim；这避免了为每组势
    // 重新收集 O(n^2) 条 Edge 并执行 O(n^2 log n) 的 Kruskal 排序。
    // forced_components 只收缩非根 forced 边；根边不属于内部 MST。
    DisjointSet forced_components(n_);
    // forced_structure_feasible 在势迭代开始前一次性验证强制结构。
    bool forced_structure_feasible = true;
    // 根 forced 边最多 2 条，内部 forced 边最多 n-2 条。
    int forced_root_count = 0;
    int forced_internal_count = 0;
    for (const Edge& edge : node.forced_edges) {
        if (edge.u == 0 || edge.v == 0) {
            ++forced_root_count;
            continue;
        }
        if (!forced_components.unite(edge.u, edge.v)) {
            forced_structure_feasible = false;
            break;
        }
        ++forced_internal_count;
    }
    if (forced_root_count > 2 || forced_internal_count > n_ - 2) {
        forced_structure_feasible = false;
    }

    // component_by_vertex[v] 是 v 所属的紧凑 Prim 分量编号；
    // component_by_root 将 DSU 根映射为 0..component_count-1。
    std::vector<int> component_by_vertex(static_cast<std::size_t>(n_), -1);
    std::vector<int> component_by_root(static_cast<std::size_t>(n_), -1);
    int component_count = 0;
    if (forced_structure_feasible) {
        for (int vertex = 1; vertex < n_; ++vertex) {
            const int root = forced_components.find(vertex);
            int& component = component_by_root[static_cast<std::size_t>(root)];
            if (component < 0) component = component_count++;
            component_by_vertex[static_cast<std::size_t>(vertex)] = component;
        }
    }
    // 每个 forced 分量的顶点以单链表存储：head[c] 指向首顶点，
    // next_vertex_in_component[v] 指向同分量下一顶点，避免 vector-of-vector。
    std::vector<int> component_vertex_head(
        static_cast<std::size_t>(component_count), -1);
    std::vector<int> next_vertex_in_component(
        static_cast<std::size_t>(n_), -1);
    for (int vertex = 1; vertex < n_; ++vertex) {
        const int component =
            component_by_vertex[static_cast<std::size_t>(vertex)];
        if (component < 0) continue;
        next_vertex_in_component[static_cast<std::size_t>(vertex)] =
            component_vertex_head[static_cast<std::size_t>(component)];
        component_vertex_head[static_cast<std::size_t>(component)] = vertex;
    }

    struct PrimArc {
        // 目标顶点；弧的源顶点由它所在的 prim_head 邻接链确定。
        int to = -1;
        // 同一源顶点邻接链中的下一条弧下标，-1 表示链尾。
        int next = -1;
        // 原始距离；每轮使用时再加临时势 pi[u]+pi[v]。
        double original_weight = 0.0;
    };
    // prim_head[u] 是 u 的第一条有效内部弧下标；每条无向边存为两条弧。
    std::vector<int> prim_head(static_cast<std::size_t>(n_), -1);
    std::vector<PrimArc> prim_arcs;
    prim_arcs.reserve(
        static_cast<std::size_t>(n_ - 1)
        * static_cast<std::size_t>(n_ - 2));
    if (forced_structure_feasible) {
        // active/forced/forbidden 状态在本次有限轮上升中不变，只过滤一次。
        // 双向邻接使每轮 Prim 仅访问当前节点仍有效的边，而不是完整距离矩阵。
        for (int u = 1; u < n_; ++u) {
            for (int v = u + 1; v < n_; ++v) {
                if (!isFinite(dist_[u][v])) continue;
                if (component_by_vertex[static_cast<std::size_t>(u)]
                    == component_by_vertex[static_cast<std::size_t>(v)]) {
                    continue;
                }
                const std::size_t id = edgeId(u, v);
                if (node.forced[id] || node.forbidden[id]
                    || !isCandidateActive(node, id)) {
                    continue;
                }
                prim_arcs.push_back(
                    PrimArc{v, prim_head[static_cast<std::size_t>(u)],
                            dist_[u][v]});
                prim_head[static_cast<std::size_t>(u)] =
                    static_cast<int>(prim_arcs.size() - 1);
                prim_arcs.push_back(
                    PrimArc{u, prim_head[static_cast<std::size_t>(v)],
                            dist_[u][v]});
                prim_head[static_cast<std::size_t>(v)] =
                    static_cast<int>(prim_arcs.size() - 1);
            }
        }
    }

    const double infinity = std::numeric_limits<double>::infinity();
    // 对每个尚未入树的 forced 分量，以下三个数组共同保存当前最便宜的
    // 跨割边：(best_component_u[c], best_component_v[c], weight[c])。
    std::vector<double> best_component_weight(
        static_cast<std::size_t>(component_count), infinity);
    std::vector<int> best_component_u(
        static_cast<std::size_t>(component_count), -1);
    std::vector<int> best_component_v(
        static_cast<std::size_t>(component_count), -1);
    // component_in_tree[c] 标记紧凑分量 c 是否已经被 Prim 选中。
    std::vector<unsigned char> component_in_tree(
        static_cast<std::size_t>(component_count), 0);
    // 根边数量只有 O(n)，每轮重新收集后强制边优先、其余按调整权重排序。
    std::vector<Edge> root_edges;
    root_edges.reserve(static_cast<std::size_t>(n_ - 1));

    auto edge_key_less = [](double left_weight, int left_u, int left_v,
                            double right_weight, int right_u, int right_v) {
        if (left_weight != right_weight) return left_weight < right_weight;
        if (left_u > left_v) std::swap(left_u, left_v);
        if (right_u > right_v) std::swap(right_u, right_v);
        if (left_u != right_u) return left_u < right_u;
        return left_v < right_v;
    };

    auto evaluate = [&](const std::vector<double>& potentials) {
        Evaluation evaluation;
        evaluation.degree.assign(static_cast<std::size_t>(n_), 0);
        if (!forced_structure_feasible || component_count == 0) {
            return evaluation;
        }

        // modified_cost 是临时势下已选边的调整权重和；mst_edge_count 包含
        // 预先收缩的 forced 内部边，最终必须恰好达到 n-2。
        double modified_cost = 0.0;
        int mst_edge_count = forced_internal_count;

        // 强制边必须先进入受约束 1-tree。边中保存的是根势权重，因此这里
        // 用临时势重新计算权重，不能直接使用 Edge::w。
        for (const Edge& edge : node.forced_edges) {
            const double weight = dist_[edge.u][edge.v]
                + potentials[static_cast<std::size_t>(edge.u)]
                + potentials[static_cast<std::size_t>(edge.v)];
            if (!isFinite(weight)) return evaluation;
            if (edge.u == 0 || edge.v == 0) {
                continue;
            }
            modified_cost += weight;
            ++evaluation.degree[static_cast<std::size_t>(edge.u)];
            ++evaluation.degree[static_cast<std::size_t>(edge.v)];
        }

        std::fill(best_component_weight.begin(), best_component_weight.end(),
                  infinity);
        std::fill(best_component_u.begin(), best_component_u.end(), -1);
        std::fill(best_component_v.begin(), best_component_v.end(), -1);
        std::fill(component_in_tree.begin(), component_in_tree.end(), 0);

        // 在 forced 分量图上执行 Prim。候选邻接已在本次上升开始时过滤，
        // 每轮只需扫描 O(m_active) 条 arc，并用 O(component_count^2) 选择
        // 下一个分量；不再构造和排序 Edge 数组。
        auto relax_component = [&](int selected_component) {
            for (int u = component_vertex_head[
                     static_cast<std::size_t>(selected_component)];
                 u >= 0;
                 u = next_vertex_in_component[static_cast<std::size_t>(u)]) {
                for (int arc_index = prim_head[static_cast<std::size_t>(u)];
                     arc_index >= 0;
                     arc_index = prim_arcs[
                         static_cast<std::size_t>(arc_index)].next) {
                    const PrimArc& arc =
                        prim_arcs[static_cast<std::size_t>(arc_index)];
                    const int v = arc.to;
                    const int target_component =
                        component_by_vertex[static_cast<std::size_t>(v)];
                    if (target_component == selected_component
                        || component_in_tree[
                            static_cast<std::size_t>(target_component)]) {
                        continue;
                    }
                    const double weight = arc.original_weight
                        + potentials[static_cast<std::size_t>(u)]
                        + potentials[static_cast<std::size_t>(v)];
                    if (!isFinite(weight)) continue;
                    const std::size_t target =
                        static_cast<std::size_t>(target_component);
                    if (best_component_u[target] < 0
                        || edge_key_less(
                            weight, u, v,
                            best_component_weight[target],
                            best_component_u[target],
                            best_component_v[target])) {
                        best_component_weight[target] = weight;
                        best_component_u[target] = u;
                        best_component_v[target] = v;
                    }
                }
            }
        };

        component_in_tree[0] = 1;
        relax_component(0);
        for (int selected_count = 1;
             selected_count < component_count; ++selected_count) {
            int next_component = -1;
            for (int component = 0; component < component_count; ++component) {
                const std::size_t index = static_cast<std::size_t>(component);
                if (component_in_tree[index] || best_component_u[index] < 0) {
                    continue;
                }
                if (next_component < 0) {
                    next_component = component;
                    continue;
                }
                const std::size_t next =
                    static_cast<std::size_t>(next_component);
                if (edge_key_less(
                        best_component_weight[index],
                        best_component_u[index], best_component_v[index],
                        best_component_weight[next],
                        best_component_u[next], best_component_v[next])) {
                    next_component = component;
                }
            }
            if (next_component < 0) return evaluation;

            const std::size_t next = static_cast<std::size_t>(next_component);
            modified_cost += best_component_weight[next];
            ++evaluation.degree[
                static_cast<std::size_t>(best_component_u[next])];
            ++evaluation.degree[
                static_cast<std::size_t>(best_component_v[next])];
            ++mst_edge_count;
            component_in_tree[next] = 1;
            relax_component(next_component);
        }
        if (mst_edge_count != n_ - 2) return evaluation;

        root_edges.clear();
        for (const Edge& edge : node.forced_edges) {
            if (edge.u != 0 && edge.v != 0) continue;
            root_edges.push_back(Edge{
                edge.u, edge.v,
                dist_[edge.u][edge.v]
                    + potentials[static_cast<std::size_t>(edge.u)]
                    + potentials[static_cast<std::size_t>(edge.v)]});
        }
        for (int vertex = 1; vertex < n_; ++vertex) {
            const std::size_t id = edgeId(0, vertex);
            if (!isFinite(dist_[0][vertex]) || node.forced[id]
                || node.forbidden[id] || !isCandidateActive(node, id)) {
                continue;
            }
            root_edges.push_back(Edge{
                0, vertex,
                dist_[0][vertex] + potentials[0]
                    + potentials[static_cast<std::size_t>(vertex)]});
        }
        if (root_edges.size() < 2) return evaluation;
        std::sort(root_edges.begin(), root_edges.end(),
                  [&](const Edge& a, const Edge& b) {
                      const bool a_forced = node.forced[edgeId(a.u, a.v)] != 0;
                      const bool b_forced = node.forced[edgeId(b.u, b.v)] != 0;
                      if (a_forced != b_forced) return a_forced;
                      if (a.w != b.w) return a.w < b.w;
                      return a.v < b.v;
                  });
        // forced 根边已经排在最前；其余位置由修改权重最小的可用根边补齐。
        for (std::size_t index = 0; index < 2; ++index) {
            const Edge& edge = root_edges[index];
            modified_cost += edge.w;
            ++evaluation.degree[static_cast<std::size_t>(edge.u)];
            ++evaluation.degree[static_cast<std::size_t>(edge.v)];
        }

        // 每个合法 tour 中每个顶点出现两次，因此从调整权重和中减去
        // correction=2*Σpi[v] 后，得到原问题坐标系中的下界。
        const double correction = 2.0
            * std::accumulate(potentials.begin(), potentials.end(), 0.0);
        // 与安装 subtree epoch 时使用同一舍入保护口径。保护量取决于
        // 整个修改权重图，而不是恰好被本次 1-tree 选中的边；这样临时
        // 证书和随后重建出的 epoch 下界可逐项核对。
        double largest_adjusted_weight = 0.0;
        const bool has_nonzero_potential = std::any_of(
            potentials.begin(), potentials.end(),
            [](double value) { return value != 0.0; });
        if (has_nonzero_potential) {
            for (int u = 0; u < n_; ++u) {
                for (int v = u + 1; v < n_; ++v) {
                    if (!isFinite(dist_[u][v])) continue;
                    const double weight = dist_[u][v]
                        + potentials[static_cast<std::size_t>(u)]
                        + potentials[static_cast<std::size_t>(v)];
                    largest_adjusted_weight = std::max(
                        largest_adjusted_weight, std::fabs(weight));
                }
            }
        }
        // guard 只向下放松证书，用于抵消调整权重求和与势修正相减的舍入误差。
        const double guard = scaledRoundoffTolerance(
            static_cast<double>(n_) * largest_adjusted_weight,
            correction, static_cast<std::size_t>(n_));
        evaluation.feasible = true;
        evaluation.bound = modified_cost - correction - guard;
        return evaluation;
    };

    // result 始终保存整个尝试期间取得的最强证书和对应势；potentials 则是
    // 下一轮要评估的工作副本，两者可能来自不同迭代。
    NodePotentialUpdateResult result;
    result.feasible = true;
    result.bound = current_bound;
    std::vector<double> potentials = vertex_potential_;
    if (potentials.size() != static_cast<std::size_t>(n_)) {
        potentials.assign(static_cast<std::size_t>(n_), 0.0);
    }
    result.potentials = potentials;

    if (node_ascent_strategy_ == NodeAscentStrategy::Helsgaun) {
        // 与根节点的 Helsgaun 实验使用同一套适配调度：固定特殊节点 0，
        // period 内保持步长，使用 0.7/0.3 平滑次梯度。这里仍从父 epoch 势
        // warm start，并在当前 forced/forbidden/active 约束图上评估。
        std::vector<double> previous_subgradient(
            static_cast<std::size_t>(n_), 0.0);
        bool have_previous = false;
        double previous_bound = -std::numeric_limits<double>::infinity();
        double step = 1.0;
        std::size_t period = std::max<std::size_t>(
            1, static_cast<std::size_t>(n_) / 2);
        std::size_t position_in_period = 0;
        bool first_period = true;
        bool doubling_step = true;

        for (std::size_t iteration = 0;
             iteration < max_iterations && period > 0
                 && step > 0.0 && isFinite(step);
             ++iteration) {
            const Evaluation evaluation = evaluate(potentials);
            ++result.iterations;
            if (!evaluation.feasible) {
                result.feasible = false;
                break;
            }

            const double tolerance = scaledRoundoffTolerance(
                evaluation.bound, upper_bound, static_cast<std::size_t>(n_));
            const bool improved_best =
                evaluation.bound > result.bound + tolerance;
            if (improved_best) {
                result.bound = evaluation.bound;
                result.potentials = potentials;
            }

            std::vector<double> subgradient(
                static_cast<std::size_t>(n_), 0.0);
            double subgradient_norm = 0.0;
            for (int vertex = 0; vertex < n_; ++vertex) {
                const std::size_t index = static_cast<std::size_t>(vertex);
                const double deviation =
                    static_cast<double>(evaluation.degree[index] - 2);
                subgradient[index] = deviation;
                subgradient_norm += deviation * deviation;
            }

            const bool prunable = shouldPrune(result.bound, upper_bound);
            if (subgradient_norm == 0.0 || prunable
                || evaluation.bound >= upper_bound - tolerance) {
                result.stopped_prunable =
                    prunable && iteration + 1 < max_iterations;
                break;
            }

            // 论文式首 period 探索：只要本轮下界继续增长就将步长翻倍；
            // 第一次不增长后，本 period 的剩余迭代保持当前步长。
            if (first_period && have_previous && doubling_step) {
                const double previous_tolerance = scaledRoundoffTolerance(
                    evaluation.bound, previous_bound,
                    static_cast<std::size_t>(n_));
                if (evaluation.bound > previous_bound + previous_tolerance
                    && isFinite(step * 2.0)) {
                    step *= 2.0;
                } else {
                    doubling_step = false;
                }
            }

            bool finite_update = true;
            for (int vertex = 0; vertex < n_; ++vertex) {
                const std::size_t index = static_cast<std::size_t>(vertex);
                const double direction = have_previous
                    ? 0.7 * subgradient[index]
                        + 0.3 * previous_subgradient[index]
                    : subgradient[index];
                potentials[index] += step * direction;
                finite_update = finite_update && isFinite(potentials[index]);
            }
            if (!finite_update) break;

            previous_subgradient = std::move(subgradient);
            previous_bound = evaluation.bound;
            have_previous = true;
            ++position_in_period;
            if (position_in_period >= period) {
                std::size_t next_period = period / 2;
                step *= 0.5;
                if (improved_best && next_period > 0) {
                    next_period *= 2;
                }
                period = next_period;
                position_in_period = 0;
                first_period = false;
            }
        }
        return result;
    }

    // step_scale 控制 Polyak 步长，no_improvement 达到阈值后折半抑制振荡。
    double step_scale = 2.0;
    std::size_t no_improvement = 0;
    for (std::size_t iteration = 0; iteration < max_iterations; ++iteration) {
        const Evaluation evaluation = evaluate(potentials);
        ++result.iterations;
        if (!evaluation.feasible) {
            result.feasible = false;
            break;
        }

        // tolerance 是比较两个浮点下界时的尺度相关容差，不是算法 gap。
        const double tolerance = scaledRoundoffTolerance(
            evaluation.bound, upper_bound, static_cast<std::size_t>(n_));
        if (evaluation.bound > result.bound + tolerance) {
            result.bound = evaluation.bound;
            result.potentials = potentials;
            no_improvement = 0;
        } else {
            ++no_improvement;
        }

        double subgradient_norm = 0.0;
        for (const int degree : evaluation.degree) {
            const double deviation = static_cast<double>(degree - 2);
            subgradient_norm += deviation * deviation;
        }
        // 与 DFS 共用正式剪枝谓词。整数 TSP 中 ceil(bound-tolerance) >= UB
        // 已足以证明该节点不能改进 incumbent；继续迭代只会加固同一证明，
        // 不会改变搜索决策，所以可安全省掉本轮剩余的势更新。
        const bool prunable = shouldPrune(result.bound, upper_bound);
        if (subgradient_norm == 0.0 || prunable
            || evaluation.bound >= upper_bound - tolerance) {
            // 仅在确实省下至少一次迭代时计数，避免把自然到达轮次上限记成收益。
            result.stopped_prunable =
                prunable && iteration + 1 < max_iterations;
            break;
        }

        // gap 使用当前轮证书而非历史最优 result.bound；step 是标准 Polyak
        // 步长 scale*(UB-LB)/||g||²。
        const double gap = upper_bound - evaluation.bound;
        const double step = step_scale * gap / subgradient_norm;
        if (!isFinite(step) || step <= 0.0) break;
        bool finite_update = true;
        for (int vertex = 0; vertex < n_; ++vertex) {
            const std::size_t index = static_cast<std::size_t>(vertex);
            potentials[index] += step
                * static_cast<double>(evaluation.degree[index] - 2);
            finite_update = finite_update && isFinite(potentials[index]);
        }
        if (!finite_update) break;

        if (no_improvement >= 4) {
            step_scale *= 0.5;
            no_improvement = 0;
        }
    }
    return result;
}

BranchBoundSolver::OneTree BranchBoundSolver::rebuildPotentialEpoch(
    PartialSol& node, const std::vector<double>& potentials,
    const std::vector<unsigned char>& active_by_edge_id)
{
    // potentials 成为整个新子树 epoch 的当前势；所有依赖边权顺序的缓存
    // 都必须在本函数中同步重建，不能与父 epoch 的候选下标混用。
    vertex_potential_ = potentials;
    potential_correction_ = 2.0
        * std::accumulate(
            vertex_potential_.begin(), vertex_potential_.end(), 0.0);
    potential_roundoff_guard_ = 0.0;
    const bool has_nonzero_potential = std::any_of(
        vertex_potential_.begin(), vertex_potential_.end(),
        [](double value) { return value != 0.0; });
    if (has_nonzero_potential) {
        double largest_adjusted_weight = 0.0;
        for (int u = 0; u < n_; ++u) {
            for (int v = u + 1; v < n_; ++v) {
                if (!isFinite(dist_[u][v])) continue;
                largest_adjusted_weight = std::max(
                    largest_adjusted_weight,
                    std::fabs(adjustedEdgeWeight(u, v)));
            }
        }
        potential_roundoff_guard_ = scaledRoundoffTolerance(
            static_cast<double>(n_) * largest_adjusted_weight,
            potential_correction_, static_cast<std::size_t>(n_));
    }

    // forced_edges 属于节点副本，可以安全地换成新 epoch 的权重。
    node.forced_mst_cost = 0.0;
    node.forced_mst_count = 0;
    for (Edge& edge : node.forced_edges) {
        edge.w = adjustedEdgeWeight(edge.u, edge.v);
        if (edge.u != 0 && edge.v != 0) {
            node.forced_mst_cost += edge.w;
            ++node.forced_mst_count;
        }
    }

    // active_by_edge_id 使用旧 epoch 的稳定 edgeId；重排后借它把相同的
    // 活跃边集合投影到新 candidates_sorted_ 的 bitset 下标。
    const std::size_t edge_state_size = static_cast<std::size_t>(n_)
        * static_cast<std::size_t>(n_);
    candidates_sorted_.clear();
    candidates_sorted_.reserve(edge_state_size / 2);
    for (int u = 0; u < n_; ++u) {
        for (int v = u + 1; v < n_; ++v) {
            if (!isFinite(dist_[u][v])) continue;
            candidates_sorted_.push_back(
                Edge{u, v, adjustedEdgeWeight(u, v)});
        }
    }
    std::sort(candidates_sorted_.begin(), candidates_sorted_.end(),
              [](const Edge& a, const Edge& b) {
                  if (a.w != b.w) return a.w < b.w;
                  if (a.u != b.u) return a.u < b.u;
                  return a.v < b.v;
              });

    edge_rank_by_id_.assign(edge_state_size, -1);
    candidate_word_count_ = (candidates_sorted_.size() + 63) / 64;
    candidate_incident_bits_.assign(
        static_cast<std::size_t>(n_) * candidate_word_count_, 0);
    internal_candidate_bits_.assign(candidate_word_count_, 0);
    node.candidate_mask.clear();
    node.candidate_bit_count = candidates_sorted_.size();
    node.candidate_bits.assign(candidate_word_count_, 0);
    available_degree_ = node.forced_degree;

    for (std::size_t index = 0; index < candidates_sorted_.size(); ++index) {
        const Edge& edge = candidates_sorted_[index];
        const std::size_t id = edgeId(edge.u, edge.v);
        edge_rank_by_id_[id] = static_cast<int>(index);
        const std::size_t word_index = index / 64;
        const std::uint64_t bit = std::uint64_t{1} << (index % 64);
        candidate_incident_bits_[
            static_cast<std::size_t>(edge.u) * candidate_word_count_ + word_index]
            |= bit;
        candidate_incident_bits_[
            static_cast<std::size_t>(edge.v) * candidate_word_count_ + word_index]
            |= bit;
        if (edge.u != 0 && edge.v != 0) {
            internal_candidate_bits_[word_index] |= bit;
        }
        if (id < active_by_edge_id.size() && active_by_edge_id[id]) {
            node.candidate_bits[word_index] |= bit;
            if (!node.forced[id]) {
                ++available_degree_[static_cast<std::size_t>(edge.u)];
                ++available_degree_[static_cast<std::size_t>(edge.v)];
            }
        }
    }
    insufficient_degree_count_ = 0;
    for (const int degree : available_degree_) {
        if (degree < 2) ++insufficient_degree_count_;
    }

    root_candidates_sorted_.clear();
    root_candidates_sorted_.reserve(static_cast<std::size_t>(n_ - 1));
    for (int vertex = 1; vertex < n_; ++vertex) {
        if (!isFinite(dist_[0][vertex])) continue;
        root_candidates_sorted_.push_back(
            Edge{0, vertex, adjustedEdgeWeight(0, vertex)});
    }
    std::sort(root_candidates_sorted_.begin(), root_candidates_sorted_.end(),
              [](const Edge& a, const Edge& b) {
                  if (a.w != b.w) return a.w < b.w;
                  return a.v < b.v;
              });

    candidate_undo_.clear();
    candidate_undo_.reserve(candidates_sorted_.size());
    tree_undo_.clear();
    tree_undo_.reserve(candidates_sorted_.size());
    tree_snapshot_undo_.clear();
    tree_snapshot_undo_.reserve(static_cast<std::size_t>(n_));
    removed_candidate_scratch_.clear();
    mst_cut_candidate_bits_.clear();
    return computeOneTree(node, candidates_sorted_);
}

bool BranchBoundSolver::searchSubtreeWithUpdatedPotentials(
    PartialSol& node, const OneTree& current_tree,
    int depth, bool count_node)
{
    // update_started_at 只统计次梯度上升；后面的 epoch 重建单独计时。
    const auto update_started_at = std::chrono::steady_clock::now();
    ++result_.stats.potential_updates_attempted;
    ++potential_updates_in_round_;
    const NodePotentialUpdateResult update = updateNodePotentialBound(
        node, current_tree.cost, best_cost_, potential_update_iterations_);
    result_.stats.potential_update_iterations += update.iterations;
    if (update.stopped_prunable) {
        ++result_.stats.potential_updates_stopped_prunable;
    }
    result_.stats.potential_update_seconds += std::chrono::duration<double>(
        std::chrono::steady_clock::now() - update_started_at).count();

    // 只有新证书严格强于当前树下界（超过数值容差）才值得安装昂贵的新 epoch。
    const double tolerance = scaledRoundoffTolerance(
        update.bound, current_tree.cost, static_cast<std::size_t>(n_));
    if (!update.feasible
        || update.potentials.size() != static_cast<std::size_t>(n_)
        || update.bound <= current_tree.cost + tolerance) {
        return false;
    }

    const double gain = update.bound - current_tree.cost;
    ++result_.stats.potential_updates_improved;
    result_.stats.potential_update_total_gain += gain;
    result_.stats.potential_update_max_gain = std::max(
        result_.stats.potential_update_max_gain, gain);
    if (shouldPrune(update.bound, best_cost_)) {
        ++result_.stats.potential_updates_pruned;
        ++result_.stats.nodes_pruned_by_bound;
        return true;
    }

    // 在候选边重排前按 edgeId 保存 active 集合；edgeId 不依赖权重顺序。
    const std::size_t edge_state_size = static_cast<std::size_t>(n_)
        * static_cast<std::size_t>(n_);
    std::vector<unsigned char> active_by_edge_id(edge_state_size, 0);
    for (const Edge& edge : candidates_sorted_) {
        const std::size_t id = edgeId(edge.u, edge.v);
        if (isCandidateActive(node, id)) active_by_edge_id[id] = 1;
    }

    // EpochSnapshot 是父势环境的完整可移动快照。这里不仅保存 pi，还必须
    // 保存所有由 pi 派生的排序、位图、度数缓存和 undo 栈。
    struct EpochSnapshot {
        std::vector<double> vertex_potential;
        double potential_correction = 0.0;
        double potential_roundoff_guard = 0.0;
        std::vector<Edge> root_candidates;
        std::vector<Edge> candidates;
        std::vector<int> edge_rank;
        std::vector<CandidateUndo> candidate_undo;
        std::vector<std::size_t> removed_candidate_scratch;
        std::vector<std::uint64_t> candidate_incident_bits;
        std::vector<std::uint64_t> internal_candidate_bits;
        std::size_t candidate_word_count = 0;
        std::vector<int> available_degree;
        int insufficient_degree_count = 0;
        std::vector<TreeUndo> tree_undo;
        std::vector<OneTree> tree_snapshot_undo;
        std::vector<std::uint64_t> mst_cut_candidate_bits;
        int epoch_depth = 0;
    } snapshot;

    snapshot.vertex_potential = std::move(vertex_potential_);
    snapshot.potential_correction = potential_correction_;
    snapshot.potential_roundoff_guard = potential_roundoff_guard_;
    snapshot.root_candidates = std::move(root_candidates_sorted_);
    snapshot.candidates = std::move(candidates_sorted_);
    snapshot.edge_rank = std::move(edge_rank_by_id_);
    snapshot.candidate_undo = std::move(candidate_undo_);
    snapshot.removed_candidate_scratch =
        std::move(removed_candidate_scratch_);
    snapshot.candidate_incident_bits = std::move(candidate_incident_bits_);
    snapshot.internal_candidate_bits = std::move(internal_candidate_bits_);
    snapshot.candidate_word_count = candidate_word_count_;
    snapshot.available_degree = std::move(available_degree_);
    snapshot.insufficient_degree_count = insufficient_degree_count_;
    snapshot.tree_undo = std::move(tree_undo_);
    snapshot.tree_snapshot_undo = std::move(tree_snapshot_undo_);
    snapshot.mst_cut_candidate_bits = std::move(mst_cut_candidate_bits_);
    snapshot.epoch_depth = current_potential_epoch_depth_;

    auto restore_epoch = [&]() {
        vertex_potential_ = std::move(snapshot.vertex_potential);
        potential_correction_ = snapshot.potential_correction;
        potential_roundoff_guard_ = snapshot.potential_roundoff_guard;
        root_candidates_sorted_ = std::move(snapshot.root_candidates);
        candidates_sorted_ = std::move(snapshot.candidates);
        edge_rank_by_id_ = std::move(snapshot.edge_rank);
        candidate_undo_ = std::move(snapshot.candidate_undo);
        removed_candidate_scratch_ =
            std::move(snapshot.removed_candidate_scratch);
        candidate_incident_bits_ =
            std::move(snapshot.candidate_incident_bits);
        internal_candidate_bits_ =
            std::move(snapshot.internal_candidate_bits);
        candidate_word_count_ = snapshot.candidate_word_count;
        available_degree_ = std::move(snapshot.available_degree);
        insufficient_degree_count_ = snapshot.insufficient_degree_count;
        tree_undo_ = std::move(snapshot.tree_undo);
        tree_snapshot_undo_ = std::move(snapshot.tree_snapshot_undo);
        mst_cut_candidate_bits_ =
            std::move(snapshot.mst_cut_candidate_bits);
        current_potential_epoch_depth_ = snapshot.epoch_depth;
    };

    // epoch_node 是当前约束节点的独立副本；新 epoch 内的 candidate_bits
    // 使用新排序，不能直接写回父节点。
    const auto rebuild_started_at = std::chrono::steady_clock::now();
    PartialSol epoch_node = node;
    current_potential_epoch_depth_ = depth;
    OneTree epoch_tree = rebuildPotentialEpoch(
        epoch_node, update.potentials, active_by_edge_id);
    result_.stats.potential_update_rebuild_seconds
        += std::chrono::duration<double>(
            std::chrono::steady_clock::now() - rebuild_started_at).count();

    if (!epoch_tree.feasible) {
        restore_epoch();
        return false;
    }
#ifdef TSP_VERIFY_INCREMENTAL_STATE
    if (!costsNumericallyEqual(
            epoch_tree.cost, update.bound,
            static_cast<std::size_t>(n_))) {
        std::ostringstream message;
        message << "rebuilt potential epoch differs from its ascent certificate"
                << " depth=" << depth
                << " rebuilt=" << formatDebugDouble(epoch_tree.cost)
                << " certificate=" << formatDebugDouble(update.bound)
                << " active=" << epoch_node.candidate_bit_count
                << " forced=" << epoch_node.forced_edges.size();
        restore_epoch();
        throw std::runtime_error(message.str());
    }
#endif
    ++result_.stats.potential_updates_rebuilt;

    try {
        // 本锚点已更新，首次进入时禁止再次触发；其后代仍可按间隔创建
        // 下一层势 epoch。
        search(epoch_node, epoch_tree, depth, count_node, false);
        if (!candidate_undo_.empty() || !tree_undo_.empty()
            || !tree_snapshot_undo_.empty()) {
            throw std::logic_error(
                "potential epoch DFS did not restore its anchor state");
        }
    } catch (...) {
        restore_epoch();
        throw;
    }
    restore_epoch();
    return true;
}

SolveResult BranchBoundSolver::solve()
{
    // forced/forbidden/root-alpha 等按 edgeId 访问的稠密状态容量。edgeId 只
    // 使用上三角槽位，但分配 n² 可保持 O(1) 映射并简化回滚。
    const std::size_t edge_state_size = static_cast<std::size_t>(n_) * static_cast<std::size_t>(n_);

    // 重置搜索状态。
    result_ = SolveResult{};
    best_cost_ = std::numeric_limits<double>::infinity();
    best_tour_.clear();
    initial_tour_alternatives_.clear();
    diversified_tour_attempted_ = false;
    restart_search_requested_ = false;
    candidate_undo_.clear();
    tree_undo_.clear();
    tree_snapshot_undo_.clear();
    writeDebugLine(debug_, "exact solve started: vertices=" + std::to_string(n_));

    // 先用启发式得到一个上界。
    if (findInitialTour(
            best_tour_, best_cost_, initial_tour_alternatives_)) {
        // 启发式内部会做多次增量 delta 更新。精确搜索的 incumbent 必须以
        // 返回回路逐边重算的真实成本为准，不能沿用可能漂移的缓存值。
        best_cost_ = tourCost(best_tour_);
        result_.stats.initial_upper_bound = best_cost_;
        writeDebugLine(debug_, "initial incumbent: cost=" + formatDebugDouble(best_cost_));
    } else {
        result_.stats.initial_upper_bound = std::numeric_limits<double>::infinity();
        writeDebugLine(debug_, "initial incumbent: unavailable");
    }
    // 自适应多启动 LK 若在探测搜索后改善 incumbent，会完整回退当前
    // DFS，再用更紧上界重新优化根势并重启。subtree 模式在单轮内部还可
    // 创建局部势 epoch，但每个 epoch 都在退出其子树时完整恢复。
    while (true) {
        // 两个计数器都以本次根搜索为生命周期；若改善 incumbent 后重启，
        // 势更新预算和最近 epoch 深度从根重新计算。
        potential_updates_in_round_ = 0;
        current_potential_epoch_depth_ = 0;
        optimizeRootPotentials(best_cost_);
        // root 是深度 0 的空约束部分解；之后 reduced-cost fixing 可能在它
        // 上永久加入 forced/forbidden 基线。
        PartialSol root;
        root.depth = 0;
        root.forced.assign(edge_state_size, 0);
        root.forbidden.assign(edge_state_size, 0);
        // 初始化 forced 边的并查集和度数统计。
        root.forced_parent.resize(static_cast<std::size_t>(n_));
        std::iota(root.forced_parent.begin(), root.forced_parent.end(), 0);
        root.forced_rank.assign(static_cast<std::size_t>(n_), 0);
        root.forced_comp_size.assign(static_cast<std::size_t>(n_), 1);
        if (n_ <= 64) {
            root.forced_member_bits.resize(static_cast<std::size_t>(n_));
            for (int vertex = 0; vertex < n_; ++vertex) {
                root.forced_member_bits[static_cast<std::size_t>(vertex)] =
                    std::uint64_t{1} << static_cast<unsigned>(vertex);
            }
        } else {
            root.forced_members.resize(static_cast<std::size_t>(n_));
            for (int vertex = 0; vertex < n_; ++vertex) {
                root.forced_members[static_cast<std::size_t>(vertex)].push_back(vertex);
            }
        }
        root.forced_degree.assign(static_cast<std::size_t>(n_), 0);
        root.forced_edges.reserve(static_cast<std::size_t>(n_));
        // 边候选集初始化：全局只保留这一份按权重排序的不可变边表。
        // DFS 通过 active bitset 原地删除，并用 undo 恢复。
        candidates_sorted_.clear();
        candidates_sorted_.reserve(edge_state_size / 2);
        edge_rank_by_id_.assign(edge_state_size, -1);
        for (int u = 0; u < n_; ++u) {
            for (int v = u + 1; v < n_; ++v) {
                if (!isFinite(dist_[u][v])) {
                    continue;
                }
                candidates_sorted_.push_back(Edge{u, v, adjustedEdgeWeight(u, v)});
            }
        }
        std::sort(candidates_sorted_.begin(), candidates_sorted_.end(),
                  [](const Edge& a, const Edge& b) { return a.w < b.w; });
        candidate_undo_.reserve(candidates_sorted_.size());
        tree_undo_.reserve(candidates_sorted_.size());
        tree_snapshot_undo_.reserve(static_cast<std::size_t>(n_));
        candidate_word_count_ = (candidates_sorted_.size() + 63) / 64;
        candidate_incident_bits_.assign(
            static_cast<std::size_t>(n_) * candidate_word_count_, 0);
        internal_candidate_bits_.assign(candidate_word_count_, 0);
        available_degree_.assign(static_cast<std::size_t>(n_), 0);
        insufficient_degree_count_ = 0;
        // 生产搜索仅维护 active bitset；candidate_mask 只留给局部候选兼容路径。
        root.candidate_mask.clear();
        resetCandidateBits(root, candidates_sorted_.size());
        for (std::size_t index = 0; index < candidates_sorted_.size(); ++index) {
            const Edge& e = candidates_sorted_[index];
            // eid 是与排序无关的边状态下标；index 是当前 epoch 内与权重顺序
            // 绑定的候选下标。两者通过 edge_rank_by_id_ 双向关联。
            const std::size_t eid = edgeId(e.u, e.v);
            edge_rank_by_id_[eid] = static_cast<int>(index);
            // 每个 uint64_t 保存连续 64 个候选的 active/incident 状态。
            const std::size_t word_index = index / 64;
            const std::uint64_t bit = std::uint64_t{1} << (index % 64);
            candidate_incident_bits_[
                static_cast<std::size_t>(e.u) * candidate_word_count_ + word_index]
                |= bit;
            candidate_incident_bits_[
                static_cast<std::size_t>(e.v) * candidate_word_count_ + word_index]
                |= bit;
            if (e.u != 0 && e.v != 0) {
                internal_candidate_bits_[word_index] |= bit;
            }
            ++available_degree_[static_cast<std::size_t>(e.u)];
            ++available_degree_[static_cast<std::size_t>(e.v)];
        }
        for (const int degree : available_degree_) {
            if (degree < 2) ++insufficient_degree_count_;
        }

        // 预排序与顶点 0 相连的边，computeOneTree 中直接取前两条合法边。
        root_candidates_sorted_.clear();
        root_candidates_sorted_.reserve(static_cast<std::size_t>(n_ - 1));
        for (int v = 1; v < n_; ++v) {
            if (isFinite(dist_[0][v])) {
                root_candidates_sorted_.push_back(
                    Edge{0, v, adjustedEdgeWeight(0, v)});
            }
        }
        std::sort(root_candidates_sorted_.begin(), root_candidates_sorted_.end(),
                  [](const Edge& a, const Edge& b) { return a.w < b.w; });

        // root_tree 同时是根下界证书和 DFS 首棵可变工作树。
        OneTree root_tree = computeOneTree(root, candidates_sorted_);
        if (!root_tree.feasible) {
            result_.feasible = false;
            result_.cost = std::numeric_limits<double>::infinity();
            result_.stats.root_lower_bound = std::numeric_limits<double>::infinity();
            writeDebugLine(debug_, "root 1-tree infeasible; exact solve stopped");
            return result_;
        }
        result_.stats.root_lower_bound = root_tree.cost;
        root.bound = root_tree.cost;
        ++result_.stats.nodes_created;
        if (root_bound_only_) {
            result_.feasible = isFinite(best_cost_) && !best_tour_.empty();
            result_.cost = result_.feasible
                ? best_cost_ : std::numeric_limits<double>::infinity();
            result_.tour = best_tour_;
            writeDebugLine(
                debug_,
                "root-bound-only finished: lower_bound="
                    + formatDebugDouble(root.bound)
                    + " upper_bound=" + formatDebugDouble(best_cost_));
            return result_;
        }
        if (branch_edge_order_ == BranchEdgeOrder::RootAlphaAscending
            || branch_edge_order_ == BranchEdgeOrder::RootAlphaDescending) {
            // 根势优化已经结束，此时的调整权重和根 1-tree 定义了本搜索
            // epoch 的静态 alpha 先验。若 diversified incumbent 触发根重启，
            // 新 epoch 会重新计算；默认 weight 路径则完全跳过这项 O(n^2) 预处理。
            buildRootAlphaNearness(root_tree);
        } else {
            root_alpha_by_edge_id_.clear();
        }
        // root_pruned 表示根下界已经证明不存在严格优于当前 incumbent 的 tour。
        const bool root_pruned = shouldPrune(root.bound, best_cost_);
        // fixing 汇总根 reduced-cost 测试；其结果会成为本轮搜索永久约束。
        RootReducedCostStats fixing;
        if (root_pruned) {
            // 根下界已经证明初始 incumbent 最优，候选池不再有使用价值。
            initial_tour_alternatives_.clear();
        } else {
            fixing = applyRootReducedCostFixing(root, root_tree);
            // 根 fixing 是本轮搜索的永久基线，不应被 DFS rollback 恢复。
            candidate_undo_.clear();
            if (debug_.output != nullptr) {
                std::ostringstream line;
                line << "root reduced-cost fixing: tested=" << fixing.tested
                     << " fixed_zero=" << fixing.fixed_zero
                     << " tree_tested=" << fixing.tree_tested
                     << " fixed_one=" << fixing.fixed_one
                     << " active=" << fixing.active_after;
                if (fixing.proves_no_improvement) {
                    line << " proves_no_improvement=yes";
                }
                writeDebugLine(debug_, line.str());
            }
        }
        {
            std::ostringstream line;
            line << "root: lower_bound=" << formatDebugDouble(root.bound)
                 << " search=bp-chain";
            writeDebugLine(debug_, line.str());
        }

        restart_search_requested_ = false;
        if (!root_pruned
            && (fixing.proves_no_improvement
                || insufficient_degree_count_ != 0)) {
            // 所有被删边都已分别证明不可能进入改进 tour；剩余图若有顶点
            // 度数不足 2，则不存在只由未删边组成的改进 Hamilton 回路。
            ++result_.stats.nodes_pruned_by_bound;
        } else {
            search(root, root_tree, 0);
        }
        if (!candidate_undo_.empty() || !tree_undo_.empty()
            || !tree_snapshot_undo_.empty()) {
            throw std::logic_error("DFS rollback did not restore the root state");
        }
        if (restart_search_requested_) {
            writeDebugLine(
                debug_,
                "restarting exact search with diversified incumbent: cost="
                    + formatDebugDouble(best_cost_));
            continue;
        }
        break;
    }

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
             << " pruned_infeasible=" << result_.stats.nodes_pruned_infeasible
             << " potential_updates="
             << result_.stats.potential_updates_attempted
             << " potential_improved="
             << result_.stats.potential_updates_improved
             << " potential_pruned="
             << result_.stats.potential_updates_pruned
             << " potential_rebuilt="
             << result_.stats.potential_updates_rebuilt
             << " potential_stopped_prunable="
             << result_.stats.potential_updates_stopped_prunable
             << " potential_iterations="
             << result_.stats.potential_update_iterations
             << " potential_seconds="
             << formatDebugDouble(result_.stats.potential_update_seconds)
             << " potential_rebuild_seconds="
             << formatDebugDouble(
                    result_.stats.potential_update_rebuild_seconds);
        writeDebugLine(debug_, line.str());
    }
    return result_;
}

// ── BP (Branch Partitioning) 实现 ────────────────────────────────

double BranchBoundSolver::currentForbidReplacementDelta(
    PartialSol& node, const OneTree& tree,
    const Edge& tree_edge) const
{
    const std::size_t removed_id = edgeId(tree_edge.u, tree_edge.v);
    const Edge* replacement = nullptr;

    // 评分时让被删边在权威 active bitset 中临时失活，使现有 replacement
    // 查询走与真正 forbid 完全相同的候选集合。checkpoint 会立即恢复候选
    // 位、available_degree_ 和 undo 栈；工作 1-tree 本身从未修改。
    const std::size_t candidate_checkpoint = candidate_undo_.size();
    deactivateCandidate(node, removed_id);

    try {
        if (tree_edge.u == 0 || tree_edge.v == 0) {
            // 根边不属于内部 MST；禁止其中一条后，只需寻找当前约束下
            // 下一条未被 1-tree 选中的合法根边。
            for (const Edge& candidate : root_candidates_sorted_) {
                const std::size_t candidate_id = edgeId(candidate.u, candidate.v);
                if (node.forbidden[candidate_id]) continue;
                if (!node.forced[candidate_id]
                    && !isCandidateActive(node, candidate_id)) {
                    continue;
                }
                const bool already_selected = !tree.edge_index_in_tree.empty()
                    ? tree.edge_index_in_tree[candidate_id] >= 0
                    : std::any_of(
                        tree.edges.begin(), tree.edges.end(),
                        [&](const Edge& edge) {
                            return edgeId(edge.u, edge.v) == candidate_id;
                        });
                if (already_selected) continue;
                replacement = &candidate;
                break;
            }
        } else {
            // 默认增量路径已有动态 MST 邻接位图。关闭增量的实验变体仅在
            // 显式选择本策略时用局部副本补建，以支持相同 CLI 实验。
            const OneTree* query_tree = &tree;
            OneTree tree_with_dynamic_mst;
            if (tree.mst_adjacency_bits.empty()) {
                tree_with_dynamic_mst = tree;
                initializeDynamicMst(tree_with_dynamic_mst);
                query_tree = &tree_with_dynamic_mst;
            }
            replacement = findMstReplacement(
                node, candidates_sorted_, *query_tree, tree_edge);
        }
    } catch (...) {
        rollbackCandidates(node, candidate_checkpoint);
        throw;
    }

    const double delta = replacement == nullptr
        ? std::numeric_limits<double>::infinity()
        : std::max(0.0, replacement->w - tree_edge.w);
    rollbackCandidates(node, candidate_checkpoint);
    // MST 交换性质下 replacement 不轻于被删树边；max(0,...) 只防御
    // 极小浮点舍入或兼容测试状态，不改变正常排序。
    return delta;
}

double BranchBoundSolver::currentForceBranchBound(
    const PartialSol& node, const OneTree& current_tree,
    const Edge& tree_edge) const
{
    // strong-branching 的试算必须与正式 force 分支具有相同约束语义，但不能
    // 触碰 DFS 的候选位图、available_degree_ 或回滚栈。因此复制节点，并在
    // 局部候选表上完整重建一次受约束 1-tree。
    const std::size_t forced_id = edgeId(tree_edge.u, tree_edge.v);
    if (node.forced[forced_id] || node.forbidden[forced_id]
        || !isFinite(dist_[tree_edge.u][tree_edge.v])
        || node.forced_degree[static_cast<std::size_t>(tree_edge.u)] >= 2
        || node.forced_degree[static_cast<std::size_t>(tree_edge.v)] >= 2) {
        return std::numeric_limits<double>::infinity();
    }

    auto forced_find = [&](int vertex) {
        // 与可回滚正式 DSU 一样不做路径压缩。
        while (node.forced_parent[static_cast<std::size_t>(vertex)] != vertex) {
            vertex = node.forced_parent[static_cast<std::size_t>(vertex)];
        }
        return vertex;
    };

    int root_u = forced_find(tree_edge.u);
    int root_v = forced_find(tree_edge.v);
    if (root_u == root_v
        && node.forced_comp_size[static_cast<std::size_t>(root_u)] < n_) {
        return std::numeric_limits<double>::infinity();
    }

    // 两端点都是 forced 单点时，加入一条已经位于 current_tree 中的边：
    // 不会让端点达到强制度 2，也不会闭合已有 forced 分量。当前 1-tree
    // 因而仍满足新约束，其最优下界不可能改变，可省掉节点复制后的全量扫描。
    if (node.forced_degree[static_cast<std::size_t>(tree_edge.u)] == 0
        && node.forced_degree[static_cast<std::size_t>(tree_edge.v)] == 0) {
        return current_tree.cost;
    }

    PartialSol trial = node;

    trial.forced[forced_id] = 1;
    ++trial.forced_degree[static_cast<std::size_t>(tree_edge.u)];
    ++trial.forced_degree[static_cast<std::size_t>(tree_edge.v)];
    trial.forced_edges.push_back(tree_edge);
    if (tree_edge.u != 0 && tree_edge.v != 0) {
        trial.forced_mst_cost += tree_edge.w;
        ++trial.forced_mst_count;
    }

    if (root_u != root_v) {
        if (trial.forced_rank[static_cast<std::size_t>(root_u)]
            < trial.forced_rank[static_cast<std::size_t>(root_v)]) {
            std::swap(root_u, root_v);
        }
        if (!trial.forced_member_bits.empty()) {
            trial.forced_member_bits[static_cast<std::size_t>(root_u)] |=
                trial.forced_member_bits[static_cast<std::size_t>(root_v)];
        } else {
            auto& members_u =
                trial.forced_members[static_cast<std::size_t>(root_u)];
            const auto& members_v =
                trial.forced_members[static_cast<std::size_t>(root_v)];
            members_u.insert(
                members_u.end(), members_v.begin(), members_v.end());
        }
        trial.forced_parent[static_cast<std::size_t>(root_v)] = root_u;
        trial.forced_comp_size[static_cast<std::size_t>(root_u)] +=
            trial.forced_comp_size[static_cast<std::size_t>(root_v)];
        if (trial.forced_rank[static_cast<std::size_t>(root_u)]
            == trial.forced_rank[static_cast<std::size_t>(root_v)]) {
            ++trial.forced_rank[static_cast<std::size_t>(root_u)];
        }
    }

    // 先投影当前节点的 active 集合，再复用完整候选过滤逻辑处理新 force
    // 导致的度满和子回路删除。局部 vector 仍保持全局 adjusted-weight 顺序。
    std::vector<Edge> trial_candidates;
    trial_candidates.reserve(candidates_sorted_.size());
    for (const Edge& candidate : candidates_sorted_) {
        if (isCandidateActive(node, edgeId(candidate.u, candidate.v))) {
            trial_candidates.push_back(candidate);
        }
    }
    if (!buildBranchCandidates(trial, trial_candidates)) {
        return std::numeric_limits<double>::infinity();
    }

    // 局部 vector 的下标不再与全局 candidate_bits 对齐。改用 edgeId 掩码，
    // 这样 computeOneTree 的根边选择也只能看到过滤后仍合法的候选。
    trial.candidate_bits.clear();
    trial.candidate_bit_count = 0;
    trial.candidate_mask.assign(
        static_cast<std::size_t>(n_) * static_cast<std::size_t>(n_), 0);
    for (const Edge& candidate : trial_candidates) {
        trial.candidate_mask[edgeId(candidate.u, candidate.v)] = 1;
    }

    const OneTree forced_tree = computeOneTree(trial, trial_candidates);
    return forced_tree.feasible
        ? forced_tree.cost
        : std::numeric_limits<double>::infinity();
}

BranchBoundSolver::BranchSet BranchBoundSolver::bpPartition(
    PartialSol& node, OneTree& work_tree)
{
    // B_set 保存依次发现的关键边，以及禁止该边后 work_tree 的重放信息。
    BranchSet B_set;

    // bpPartition 会临时累积 forbid 前缀；两个 checkpoint 保证函数返回前
    // 只恢复树和 active 候选，forbidden 标记则在末尾按 B_set 显式清除。
    const std::size_t tree_checkpoint = tree_undo_.size();
    const std::size_t candidate_checkpoint = candidate_undo_.size();

    while (true) {
        // deg_best 指向本轮最终选中的未决 1-tree 边；指针只引用 work_tree，
        // 在调用 test() 可能替换树边之前使用。
        const Edge* deg_best = nullptr;
        double deg_best_forbid_delta =
            -std::numeric_limits<double>::infinity();
        int deg_best_excess_cover = -1;
        int deg_best_forced_degree = -1;
        int deg_best_candidate_slack = std::numeric_limits<int>::max();
        std::uint64_t deg_best_frequency_distance =
            std::numeric_limits<std::uint64_t>::max();
        std::array<const Edge*, 2> strong_candidates{{nullptr, nullptr}};

        // 该比较器只决定 BP 已选违规顶点上的分支边，不重排
        // candidates_sorted_：后者仍必须维持调整权重顺序，供 Kruskal 和
        // 1-tree 增量替换使用。root alpha 因而只改变搜索树遍历顺序，
        // 不改变节点下界的正确性。
        auto branch_edge_better = [&](const Edge& candidate,
                                      const Edge* incumbent) {
            if (incumbent == nullptr) return true;
            const bool use_root_alpha = branch_edge_order_
                    == BranchEdgeOrder::RootAlphaAscending
                || branch_edge_order_
                    == BranchEdgeOrder::RootAlphaDescending;
            if (use_root_alpha
                && root_alpha_by_edge_id_.size()
                    == static_cast<std::size_t>(n_)
                        * static_cast<std::size_t>(n_)) {
                const double candidate_alpha = root_alpha_by_edge_id_[
                    edgeId(candidate.u, candidate.v)];
                const double incumbent_alpha = root_alpha_by_edge_id_[
                    edgeId(incumbent->u, incumbent->v)];
                if (candidate_alpha != incumbent_alpha) {
                    if (branch_edge_order_
                        == BranchEdgeOrder::RootAlphaAscending) {
                        return candidate_alpha < incumbent_alpha;
                    }
                    return candidate_alpha > incumbent_alpha;
                }
            }
            // 根树边的 alpha 都为 0，平局很常见；按调整权重和端点继续比较，
            // 保证相同输入下分支顺序确定，并保持默认策略原有的局部偏好。
            if (candidate.w != incumbent->w) {
                if (branch_edge_order_
                    == BranchEdgeOrder::AdjustedWeightDescending) {
                    return candidate.w > incumbent->w;
                }
                return candidate.w < incumbent->w;
            }
            const int candidate_u = std::min(candidate.u, candidate.v);
            const int candidate_v = std::max(candidate.u, candidate.v);
            const int incumbent_u = std::min(incumbent->u, incumbent->v);
            const int incumbent_v = std::max(incumbent->u, incumbent->v);
            if (candidate_u != incumbent_u) return candidate_u < incumbent_u;
            return candidate_v < incumbent_v;
        };

        // 单侧策略对每条合格边查询 forbid replacement；双侧策略先按默认
        // adjusted-weight 顺序保留两条，再在循环之后执行昂贵的 force 试算。
        auto consider_branch_edge = [&](const Edge& candidate) {
            if (branch_edge_order_
                == BranchEdgeOrder::TwoSidedStrongBranchingTop2) {
                if (branch_edge_better(candidate, strong_candidates[0])) {
                    strong_candidates[1] = strong_candidates[0];
                    strong_candidates[0] = &candidate;
                } else if (branch_edge_better(
                               candidate, strong_candidates[1])) {
                    strong_candidates[1] = &candidate;
                }
                return;
            }
            if (branch_edge_order_
                    == BranchEdgeOrder::CurrentForbidDeltaDescending
                || branch_edge_order_
                    == BranchEdgeOrder::CurrentForbidDeltaAscending) {
                const double candidate_delta =
                    currentForbidReplacementDelta(node, work_tree, candidate);
                if (deg_best != nullptr) {
                    const bool descending = branch_edge_order_
                        == BranchEdgeOrder::CurrentForbidDeltaDescending;
                    if ((descending
                            && candidate_delta < deg_best_forbid_delta)
                        || (!descending
                            && candidate_delta > deg_best_forbid_delta)) {
                        return;
                    }
                    if (candidate_delta == deg_best_forbid_delta
                        && !branch_edge_better(candidate, deg_best)) {
                        return;
                    }
                }
                deg_best = &candidate;
                deg_best_forbid_delta = candidate_delta;
                return;
            }
            if (branch_edge_better(candidate, deg_best)) {
                deg_best = &candidate;
            }
        };

#ifdef TSP_BRANCH_STRATEGY_MIN_EDGE
        constexpr bool kUseMinEdgeFromOneTree = true;
#else
        constexpr bool kUseMinEdgeFromOneTree = false;
#endif

        const bool use_root_frequency_middle = branch_edge_order_
                == BranchEdgeOrder::RootOneTreeFrequencyMiddle
            && root_one_tree_sample_count_ != 0
            && root_one_tree_edge_counts_.size()
                == static_cast<std::size_t>(n_)
                    * static_cast<std::size_t>(n_);
        if (!kUseMinEdgeFromOneTree
            && branch_edge_order_ == BranchEdgeOrder::AdjustedWeight) {
            // 默认热路径保持原实现：先取编号最小的最大度违规顶点，再取
            // 该顶点最轻的未决树边。实验策略的额外计数不进入此循环。
            int branch_vertex = -1;
            int max_degree = 2;
            for (int vertex = 0; vertex < n_; ++vertex) {
                const int degree =
                    work_tree.degree[static_cast<std::size_t>(vertex)];
                if (degree > max_degree) {
                    max_degree = degree;
                    branch_vertex = vertex;
                }
            }
            if (branch_vertex >= 0) {
                for (const Edge& edge : work_tree.edges) {
                    if (edge.u != branch_vertex && edge.v != branch_vertex) {
                        continue;
                    }
                    const std::size_t edge_id = edgeId(edge.u, edge.v);
                    if (node.forced[edge_id] || node.forbidden[edge_id]) {
                        continue;
                    }
                    if (branch_edge_better(edge, deg_best)) deg_best = &edge;
                }
            }
        } else if (branch_edge_order_
                   == BranchEdgeOrder::MaximumDegreeAllAdjustedWeight) {
            // 默认路径只取编号最小的一个最大度顶点。此策略把所有并列
            // 最大度热点合并为候选集，其余仍使用默认 adjusted-weight 顺序。
            int maximum_violating_degree = 2;
            for (const int degree : work_tree.degree) {
                maximum_violating_degree = std::max(
                    maximum_violating_degree, degree);
            }
            if (maximum_violating_degree > 2) {
                for (const Edge& edge : work_tree.edges) {
                    const int endpoint_max_degree = std::max(
                        work_tree.degree[static_cast<std::size_t>(edge.u)],
                        work_tree.degree[static_cast<std::size_t>(edge.v)]);
                    if (endpoint_max_degree != maximum_violating_degree) {
                        continue;
                    }
                    const std::size_t edge_id = edgeId(edge.u, edge.v);
                    if (node.forced[edge_id] || node.forbidden[edge_id]) {
                        continue;
                    }
                    if (branch_edge_better(edge, deg_best)) {
                        deg_best = &edge;
                    }
                }
            }
        } else if (branch_edge_order_
                   == BranchEdgeOrder::MaximumExcessCoverAdjustedWeight) {
            // 一条边若同时连接两个超度顶点，可覆盖更多当前度违规。覆盖量
            // 是首要离散分数，adjusted weight 只负责平局，不混合不同单位。
            for (const Edge& edge : work_tree.edges) {
                const int excess_cover =
                    std::max(
                        0,
                        work_tree.degree[static_cast<std::size_t>(edge.u)] - 2)
                    + std::max(
                        0,
                        work_tree.degree[static_cast<std::size_t>(edge.v)] - 2);
                if (excess_cover == 0) continue;
                const std::size_t edge_id = edgeId(edge.u, edge.v);
                if (node.forced[edge_id] || node.forbidden[edge_id]) continue;
                if (deg_best != nullptr
                    && (excess_cover < deg_best_excess_cover
                        || (excess_cover == deg_best_excess_cover
                            && !branch_edge_better(edge, deg_best)))) {
                    continue;
                }
                deg_best = &edge;
                deg_best_excess_cover = excess_cover;
            }
        } else if (branch_edge_order_ ==
                   BranchEdgeOrder::MaximumDegreeExcessCoverAdjustedWeight) {
            // 候选顶点仍限定为最大度热点，防止全局 cover 策略跳到较低度
            // 顶点；随后用两端超度覆盖消除并列热点和关联边的平局。
            int maximum_violating_degree = 2;
            for (const int degree : work_tree.degree) {
                maximum_violating_degree = std::max(
                    maximum_violating_degree, degree);
            }
            if (maximum_violating_degree > 2) {
                for (const Edge& edge : work_tree.edges) {
                    const int endpoint_max_degree = std::max(
                        work_tree.degree[static_cast<std::size_t>(edge.u)],
                        work_tree.degree[static_cast<std::size_t>(edge.v)]);
                    if (endpoint_max_degree != maximum_violating_degree) {
                        continue;
                    }
                    const std::size_t edge_id = edgeId(edge.u, edge.v);
                    if (node.forced[edge_id] || node.forbidden[edge_id]) {
                        continue;
                    }
                    const int excess_cover =
                        std::max(
                            0,
                            work_tree.degree[static_cast<std::size_t>(edge.u)]
                                - 2)
                        + std::max(
                            0,
                            work_tree.degree[static_cast<std::size_t>(edge.v)]
                                - 2);
                    if (deg_best != nullptr
                        && (excess_cover < deg_best_excess_cover
                            || (excess_cover == deg_best_excess_cover
                                && !branch_edge_better(edge, deg_best)))) {
                        continue;
                    }
                    deg_best = &edge;
                    deg_best_excess_cover = excess_cover;
                }
            }
        } else if (use_root_frequency_middle) {
            // 根上升中的 1-tree 样本近似 LP 分数：count/samples 越接近
            // 1/2，force/forbid 两侧越可能都保留有效搜索空间。这里仍限制
            // 在当前最大度热点相邻树边内，避免为“中间性”放弃度违规结构。
            int maximum_violating_degree = 2;
            for (const int degree : work_tree.degree) {
                maximum_violating_degree = std::max(
                    maximum_violating_degree, degree);
            }
            if (maximum_violating_degree > 2) {
                const std::uint64_t sample_count =
                    root_one_tree_sample_count_;
                for (const Edge& edge : work_tree.edges) {
                    const int endpoint_max_degree = std::max(
                        work_tree.degree[static_cast<std::size_t>(edge.u)],
                        work_tree.degree[static_cast<std::size_t>(edge.v)]);
                    if (endpoint_max_degree != maximum_violating_degree) {
                        continue;
                    }
                    const std::size_t edge_id = edgeId(edge.u, edge.v);
                    if (node.forced[edge_id] || node.forbidden[edge_id]) {
                        continue;
                    }

                    // 比较 |count/samples - 1/2| 时统一乘 2*samples，
                    // 只需整数距离 |2*count-samples|，没有除法和舍入。
                    const std::uint64_t doubled_count = 2ULL
                        * root_one_tree_edge_counts_[edge_id];
                    const std::uint64_t frequency_distance =
                        doubled_count >= sample_count
                        ? doubled_count - sample_count
                        : sample_count - doubled_count;
                    const int excess_cover =
                        std::max(
                            0,
                            work_tree.degree[static_cast<std::size_t>(edge.u)]
                                - 2)
                        + std::max(
                            0,
                            work_tree.degree[static_cast<std::size_t>(edge.v)]
                                - 2);
                    if (deg_best != nullptr
                        && (frequency_distance > deg_best_frequency_distance
                            || (frequency_distance
                                    == deg_best_frequency_distance
                                && (excess_cover < deg_best_excess_cover
                                    || (excess_cover
                                            == deg_best_excess_cover
                                        && !branch_edge_better(
                                            edge, deg_best)))))) {
                        continue;
                    }
                    deg_best = &edge;
                    deg_best_frequency_distance = frequency_distance;
                    deg_best_excess_cover = excess_cover;
                }
            }
        } else if (branch_edge_order_
                   == BranchEdgeOrder::CurrentForbidDeltaDegreeAware) {
            // 与原路径只取“编号最小的一个最大度顶点”不同，本实验先找出
            // 全局最大违规度，再考察所有至少连接一个该度数顶点的树边。
            int maximum_violating_degree = 2;
            for (const int degree : work_tree.degree) {
                maximum_violating_degree = std::max(
                    maximum_violating_degree, degree);
            }
            if (maximum_violating_degree > 2) {
                for (const Edge& edge : work_tree.edges) {
                    const int endpoint_max_degree = std::max(
                        work_tree.degree[static_cast<std::size_t>(edge.u)],
                        work_tree.degree[static_cast<std::size_t>(edge.v)]);
                    if (endpoint_max_degree != maximum_violating_degree) {
                        continue;
                    }
                    const std::size_t edge_id = edgeId(edge.u, edge.v);
                    if (node.forced[edge_id] || node.forbidden[edge_id]) {
                        continue;
                    }

                    const double forbid_delta =
                        currentForbidReplacementDelta(node, work_tree, edge);
                    const int excess_cover =
                        std::max(
                            0,
                            work_tree.degree[static_cast<std::size_t>(edge.u)]
                                - 2)
                        + std::max(
                            0,
                            work_tree.degree[static_cast<std::size_t>(edge.v)]
                                - 2);
                    if (deg_best != nullptr
                        && (forbid_delta < deg_best_forbid_delta
                            || (forbid_delta == deg_best_forbid_delta
                                && (excess_cover < deg_best_excess_cover
                                    || (excess_cover
                                            == deg_best_excess_cover
                                        && !branch_edge_better(
                                            edge, deg_best)))))) {
                        continue;
                    }
                    deg_best = &edge;
                    deg_best_forbid_delta = forbid_delta;
                    deg_best_excess_cover = excess_cover;
                }
            }
        } else if (kUseMinEdgeFromOneTree) {
            // ── 新选边策略：从 1-tree 所有未决边中直接选最小权重边 ──
            for (const Edge& e : work_tree.edges) {
                const std::size_t eid = edgeId(e.u, e.v);
                if (node.forced[eid]) continue;
                if (node.forbidden[eid]) continue;
                consider_branch_edge(e);
            }
        } else {
            // ── 原选边策略：从度数违规顶点取最小权重未决边 ──
            // 若没有度数 > 2 的顶点，则当前可行 1-tree 已由 isTour() 处理。
            // branch_vertex 是当前最大超度顶点；max_deg 从合法目标度 2 开始，
            // 因此只有 degree>2 的顶点能够被选中。
            int branch_vertex = -1;
            int max_deg = 2;
            int selected_undecided_count = 0;
            const bool use_undecided_tie = branch_edge_order_
                    == BranchEdgeOrder::MaximumDegreeMinimumUndecided
                || branch_edge_order_
                    == BranchEdgeOrder::MaximumDegreeMaximumUndecided;
            auto count_undecided_tree_edges = [&](int vertex) {
                int count = 0;
                for (const Edge& edge : work_tree.edges) {
                    if (edge.u != vertex && edge.v != vertex) continue;
                    const std::size_t id = edgeId(edge.u, edge.v);
                    if (!node.forced[id] && !node.forbidden[id]) ++count;
                }
                return count;
            };
            for (int v = 0; v < n_; ++v) {
                const int vertex_degree =
                    work_tree.degree[static_cast<std::size_t>(v)];
                if (vertex_degree < max_deg || vertex_degree <= 2) continue;

                if (vertex_degree > max_deg) {
                    max_deg = vertex_degree;
                    branch_vertex = v;
                    selected_undecided_count = use_undecided_tie
                        ? count_undecided_tree_edges(v) : 0;
                    continue;
                }

                if (use_undecided_tie) {
                    const int undecided_count =
                        count_undecided_tree_edges(v);
                    const bool prefer_minimum = branch_edge_order_
                        == BranchEdgeOrder::MaximumDegreeMinimumUndecided;
                    if ((prefer_minimum
                            && undecided_count < selected_undecided_count)
                        || (!prefer_minimum
                            && undecided_count > selected_undecided_count)) {
                        branch_vertex = v;
                        selected_undecided_count = undecided_count;
                    }
                } else if (branch_vertex < 0) {
                    // 仅防御 max_deg 初值与未来目标度修改；当前 degree>2 且
                    // vertex_degree==max_deg 的首个顶点也必须可被选中。
                    branch_vertex = v;
                }
            }
            if (branch_vertex >= 0) {
                for (const Edge& e : work_tree.edges) {
                    if (e.u != branch_vertex && e.v != branch_vertex) continue;
                    const std::size_t eid = edgeId(e.u, e.v);
                    if (node.forced[eid]) continue;
                    if (node.forbidden[eid]) continue;
                    if (branch_edge_order_
                        == BranchEdgeOrder::LocalExcessCoverAdjustedWeight) {
                        const int other_vertex = e.u == branch_vertex
                            ? e.v : e.u;
                        const int other_excess = std::max(
                            0,
                            work_tree.degree[
                                static_cast<std::size_t>(other_vertex)] - 2);
                        if (deg_best != nullptr
                            && (other_excess < deg_best_excess_cover
                                || (other_excess == deg_best_excess_cover
                                    && !branch_edge_better(e, deg_best)))) {
                            continue;
                        }
                        deg_best = &e;
                        deg_best_excess_cover = other_excess;
                        continue;
                    }
                    if (branch_edge_order_ ==
                        BranchEdgeOrder::PropagationPotentialAdjustedWeight) {
                        const int forced_degree =
                            node.forced_degree[static_cast<std::size_t>(e.u)]
                            + node.forced_degree[static_cast<std::size_t>(e.v)];
                        const int candidate_slack =
                            available_degree_[static_cast<std::size_t>(e.u)]
                            + available_degree_[static_cast<std::size_t>(e.v)]
                            - forced_degree;
                        if (deg_best != nullptr
                            && (forced_degree < deg_best_forced_degree
                                || (forced_degree == deg_best_forced_degree
                                    && (candidate_slack
                                            > deg_best_candidate_slack
                                        || (candidate_slack
                                                == deg_best_candidate_slack
                                            && !branch_edge_better(
                                                e, deg_best)))))) {
                            continue;
                        }
                        deg_best = &e;
                        deg_best_forced_degree = forced_degree;
                        deg_best_candidate_slack = candidate_slack;
                        continue;
                    }
                    if (branch_edge_order_
                        == BranchEdgeOrder::ForcedDegreeAdjustedWeight) {
                        const int forced_degree =
                            node.forced_degree[static_cast<std::size_t>(e.u)]
                            + node.forced_degree[static_cast<std::size_t>(e.v)];
                        if (deg_best != nullptr
                            && (forced_degree < deg_best_forced_degree
                                || (forced_degree == deg_best_forced_degree
                                    && !branch_edge_better(e, deg_best)))) {
                            continue;
                        }
                        deg_best = &e;
                        deg_best_forced_degree = forced_degree;
                        continue;
                    }
                    consider_branch_edge(e);
                }
            }
        }

        if (branch_edge_order_
            == BranchEdgeOrder::TwoSidedStrongBranchingTop2) {
            double best_weak_gain =
                -std::numeric_limits<double>::infinity();
            double best_product_score =
                -std::numeric_limits<double>::infinity();
            for (const Edge* candidate : strong_candidates) {
                if (candidate == nullptr) continue;
                const double forbid_gain = currentForbidReplacementDelta(
                    node, work_tree, *candidate);
                const double force_bound =
                    currentForceBranchBound(node, work_tree, *candidate);
                const double force_gain = isFinite(force_bound)
                    ? std::max(0.0, force_bound - work_tree.cost)
                    : std::numeric_limits<double>::infinity();
                const double weak_gain = std::min(force_gain, forbid_gain);
                // 若任一侧为零，乘积就是零；先处理该情况可避免 0*inf=NaN。
                const double product_score =
                    force_gain == 0.0 || forbid_gain == 0.0
                    ? 0.0 : force_gain * forbid_gain;
                if (deg_best != nullptr
                    && (weak_gain < best_weak_gain
                        || (weak_gain == best_weak_gain
                            && (product_score < best_product_score
                                || (product_score == best_product_score
                                    && !branch_edge_better(
                                        *candidate, deg_best)))))) {
                    continue;
                }
                deg_best = candidate;
                best_weak_gain = weak_gain;
                best_product_score = product_score;
            }
        }

        if (!deg_best) break;

        // 选中的边先进入 B；若 forbid 后仍可行且未被 bound 剪枝，则继续划分。
        auto test = [&](Edge e) -> bool {
            // eid 在本次 lambda 内统一访问 forced、forbidden 和 candidate 状态。
            const std::size_t eid = edgeId(e.u, e.v);

            // 记录 forbid 当前边在 work_tree 中要替换的位置；search 会在
            // 兄弟分支间顺序重放该 delta，而不是复制 |B| 棵完整 1-tree。
            // O(1) 查表替代 find_if 扫描。
            // tree_edge_idx 是被禁边在当前 work_tree.edges 中的位置；后续
            // search 可在同一位置直接重放替换，避免再次搜索旧边。
            std::size_t tree_edge_idx;
            if (!work_tree.edge_index_in_tree.empty()) {
                tree_edge_idx = static_cast<std::size_t>(work_tree.edge_index_in_tree[eid]);
            } else {
                const auto removed = std::find_if(
                    work_tree.edges.begin(), work_tree.edges.end(),
                    [&](const Edge& edge) {
                        return edgeId(edge.u, edge.v) == eid;
                    });
                tree_edge_idx = static_cast<std::size_t>(
                    std::distance(work_tree.edges.begin(), removed));
            }
            // choice 先记录被禁边；成功增量替换后再补充 replacement delta。
            BranchChoice choice;
            choice.edge = e;
            choice.tree_edge_index = tree_edge_idx;
            B_set.push_back(choice);

            node.forbidden[eid] = 1;
            deactivateCandidate(node, eid);

            // replacement_found 表示 forbid 后仍存在受约束 1-tree；
            // incremental_valid 还表示当前 work_tree 已由增量路径更新完成。
            bool replacement_found = false;
#ifndef TSP_DISABLE_INCREMENTAL_ONETREE
            // 本步自己的 checkpoint 用于增量失败时仅撤销此次替换，再走重建。
            const std::size_t step_tree_checkpoint = tree_undo_.size();
            replacement_found = updateOneTreeAfterForbid(
                node, candidates_sorted_, work_tree, e, true);
            const bool incremental_valid = replacement_found;
                // && oneTreeSatisfiesConstraints(node, work_tree);  // 已关闭

            if (incremental_valid
                && B_set.back().tree_edge_index < work_tree.edges.size()) {
                B_set.back().has_forbid_replacement = true;
                B_set.back().forbid_replacement =
                    work_tree.edges[B_set.back().tree_edge_index];
            } else if (replacement_found) {
                B_set.back().replay_requires_rebuild = true;
            }

#ifdef TSP_VERIFY_INCREMENTAL_STATE
            const OneTree rebuilt = computeOneTree(node, candidates_sorted_);
            if (incremental_valid != rebuilt.feasible
                || (incremental_valid
                    && !costsNumericallyEqual(
                        work_tree.cost, rebuilt.cost, static_cast<std::size_t>(n_)))) {
                throw std::runtime_error(
                    "incremental BP prefix 1-tree differs from a complete rebuild");
            }
#endif

            // 生产路径同样检查所有 forced/forbidden/结构不变量。增量状态
            // 一旦可疑就完整重建，绝不把不受约束的树作为下界。
            if (!incremental_valid) {
                rollbackOneTree(work_tree, step_tree_checkpoint);
                OneTree rebuilt = computeOneTree(node, candidates_sorted_);
                replacement_found = rebuilt.feasible;
                    // && oneTreeSatisfiesConstraints(node, work_tree);  // 已关闭
                if (replacement_found) {
                    replaceOneTreeWithRebuild(work_tree, std::move(rebuilt));
                    B_set.back().replay_requires_rebuild = true;
                }
            }
#else
            OneTree rebuilt = computeOneTree(node, candidates_sorted_);
            replacement_found = rebuilt.feasible;
                // && oneTreeSatisfiesConstraints(node, work_tree);  // 已关闭
            if (replacement_found) {
                replaceOneTreeWithRebuild(work_tree, std::move(rebuilt));
                B_set.back().replay_requires_rebuild = true;
            }
#endif

            if (!replacement_found) {
                return false;
            }
            if (isTour(work_tree)) {
                std::vector<int> candidate = buildTour(work_tree.edges);
                const double candidate_cost = tourCost(candidate);
                if (!candidate.empty() && candidate_cost < best_cost_) {
                    best_cost_ = candidate_cost;
                    best_tour_ = std::move(candidate);
                    writeDebugLine(debug_,
                                   "new incumbent: cost=" + formatDebugDouble(best_cost_)
                                       + " source=bp-partition");
                }
                return false;
            }
            if (shouldPrune(work_tree.cost, best_cost_)) {
                return false;
            }

            // forbid 后仍可能包含改进解：保留 forbidden，基于增量更新后的树继续划分。
            return true;
        };

        if (test(*deg_best)) continue;
        break;
    }

    rollbackOneTree(work_tree, tree_checkpoint);
    rollbackCandidates(node, candidate_checkpoint);
    for (std::size_t index = 0; index < B_set.size(); ++index) {
        const Edge& edge = B_set[index].edge;
        node.forbidden[edgeId(edge.u, edge.v)] = 0;
    }

    return B_set;
}

void BranchBoundSolver::search(
    PartialSol& node, OneTree& current_tree, int depth, bool count_node,
    bool allow_potential_anchor)
{
    if (!current_tree.feasible) {
        ++result_.stats.nodes_pruned_infeasible;
        return;
    }

    maybeImproveIncumbentDiversified();
    if (restart_search_requested_) {
        return;
    }

    node.bound = current_tree.cost;
    node.depth = depth;

    // Step 2: 下界剪枝。
    if (shouldPrune(node.bound, best_cost_)) {
        ++result_.stats.nodes_pruned_by_bound;
        return;
    }

    // update_triggered 只表示通过 ROI/深度/预算门槛；persistent 模式能否
    // 成功安装 epoch 还取决于新下界是否有实质提升。
    const bool update_triggered = count_node && allow_potential_anchor
        && shouldUpdatePotentials(
            current_tree, depth, node.bound, best_cost_);
    if (update_triggered && usesPersistentPotentialUpdates()
        && searchSubtreeWithUpdatedPotentials(
            node, current_tree, depth, count_node)) {
        return;
    }

    // 证书模式只短暂上升并使用额外下界，不替换 current_tree；subtree
    // 模式则已在上面建立新 epoch 并完整处理子树。
    if (update_triggered && !usesPersistentPotentialUpdates()) {
        const auto update_started_at = std::chrono::steady_clock::now();
        ++result_.stats.potential_updates_attempted;
        ++potential_updates_in_round_;
        // update 保存本次有限轮上升中的最佳证书，不保证来自最后一轮。
        const NodePotentialUpdateResult update = updateNodePotentialBound(
            node, node.bound, best_cost_, potential_update_iterations_);
        result_.stats.potential_update_iterations += update.iterations;
        if (update.stopped_prunable) {
            ++result_.stats.potential_updates_stopped_prunable;
        }
        result_.stats.potential_update_seconds += std::chrono::duration<double>(
            std::chrono::steady_clock::now() - update_started_at).count();

        // tolerance 用于判断新证书是否实质强于 current_tree，而非剪枝阈值。
        const double tolerance = scaledRoundoffTolerance(
            update.bound, node.bound, static_cast<std::size_t>(n_));
        if (update.feasible && update.bound > node.bound + tolerance) {
            const double gain = update.bound - node.bound;
            node.bound = update.bound;
            ++result_.stats.potential_updates_improved;
            result_.stats.potential_update_total_gain += gain;
            result_.stats.potential_update_max_gain = std::max(
                result_.stats.potential_update_max_gain, gain);
            if (shouldPrune(node.bound, best_cost_)) {
                ++result_.stats.potential_updates_pruned;
                ++result_.stats.nodes_pruned_by_bound;
                return;
            }
        }
    }
    if (count_node) {
        ++result_.stats.nodes_expanded;
    }

    // 调试输出。
    const std::size_t debug_interval = normalizedDebugInterval(debug_);
    if (count_node && debug_.output != nullptr
        && result_.stats.nodes_expanded % debug_interval == 0) {
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
        const double candidate_cost = tourCost(candidate);
        if (!candidate.empty() && candidate_cost < best_cost_) {
            best_cost_ = candidate_cost;
            best_tour_ = std::move(candidate);
            writeDebugLine(debug_,
                           "new incumbent: cost=" + formatDebugDouble(best_cost_)
                               + " source=bp-node depth=" + std::to_string(depth));
        }
        return;
    }

    // Step 4: BP 划分 —— 获取 B 集（关键边）。
    BranchSet B_set = bpPartition(node, current_tree);

    // B 为空：禁止任意 1-tree 边均超上界，等价于 bound 剪枝。
    if (B_set.empty()) {
        ++result_.stats.nodes_pruned_by_bound;
        return;
    }

    // Step 5: B 集多路分支。
    // ForceChanges 是一次 apply_force 的可逆日志：
    // - forced_id/fu/fv：本次强制边及端点；
    // - root/rank/size/member：forced DSU 合并前快照；
    // - merged_scan_*：较小旧分量，供候选过滤只扫描必要顶点；
    // - old_forced_mst_*：调整权重和及边数的精确快照。
    struct ForceChanges {
        // edgeId 槽位；无有效修改时保持 -1。
        std::size_t forced_id = static_cast<std::size_t>(-1);
        // 强制边的两个原始顶点编号。
        int fu = -1, fv = -1;
        // 合并前两个端点所在的 DSU 根。
        int root_u = -1, root_v = -1;
        // 合并前根的 rank 和分量大小，用于无路径压缩 DSU 回滚。
        int old_rank_u = -1, old_rank_v = -1;
        int old_size_u = -1, old_size_v = -1;
        // 按 rank 合并后的胜方根；ru==rv 未发生合并时保持 -1。
        int merge_root = -1;
        // n>64 路径下，胜方成员 vector 合并前的长度。
        std::size_t old_merge_member_count = 0;
        // n>64 时较小旧分量的根和成员数；用于局部候选过滤。
        int merged_scan_root = -1;
        std::size_t merged_scan_count = 0;
        // n<=64 时较小旧分量的成员位图；与上面两个字段互斥使用。
        std::uint64_t merged_scan_bits = 0;
        // n<=64 时胜方根合并前的完整成员位图。
        std::uint64_t old_merge_member_bits = 0;
        // forced 内部 MST 缓存的旧值，直接恢复可避免浮点加减漂移。
        double old_forced_mst_cost = 0.0;
        int old_forced_mst_count = 0;
    };

    auto forced_find = [&](int x) -> int {
        // 不做路径压缩：父指针必须能由 ForceChanges 精确恢复。
        while (node.forced_parent[static_cast<std::size_t>(x)] != x) {
            x = node.forced_parent[static_cast<std::size_t>(x)];
        }
        return x;
    };

    auto apply_force = [&](const Edge& e, ForceChanges& changes) -> bool {
        // id 是本次唯一会从 0 改为 1 的 forced 状态槽位。
        const std::size_t id = edgeId(e.u, e.v);
        if (node.forced[id] || node.forbidden[id]
            || !isFinite(dist_[e.u][e.v])) {
            return false;
        }
        if (node.forced_degree[static_cast<std::size_t>(e.u)] >= 2
            || node.forced_degree[static_cast<std::size_t>(e.v)] >= 2) {
            return false;
        }
        // ru/rv 是修改前的分量根；相等通常意味着强制出子回路。
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
        changes.old_forced_mst_cost = node.forced_mst_cost;
        changes.old_forced_mst_count = node.forced_mst_count;

        if (ru != rv) {
            if (!node.forced_member_bits.empty()) {
                changes.merged_scan_bits =
                    changes.old_size_u <= changes.old_size_v
                    ? node.forced_member_bits[static_cast<std::size_t>(ru)]
                    : node.forced_member_bits[static_cast<std::size_t>(rv)];
            } else {
                const auto& members_u =
                    node.forced_members[static_cast<std::size_t>(ru)];
                const auto& members_v =
                    node.forced_members[static_cast<std::size_t>(rv)];
                changes.merged_scan_root = members_u.size() <= members_v.size()
                    ? ru : rv;
                changes.merged_scan_count =
                    std::min(members_u.size(), members_v.size());
            }
        }

        // ru == rv is only allowed when this edge closes a spanning forced
        // path into a Hamiltonian cycle. It does not merge two components;
        // doubling comp_size/rank here would make the cached DSU state false.
        if (ru != rv) {
            int ra = ru, rb = rv;
            if (node.forced_rank[static_cast<std::size_t>(ra)]
                < node.forced_rank[static_cast<std::size_t>(rb)]) {
                std::swap(ra, rb);
            }
            changes.merge_root = ra;
            if (!node.forced_member_bits.empty()) {
                auto& merge_bits =
                    node.forced_member_bits[static_cast<std::size_t>(ra)];
                changes.old_merge_member_bits = merge_bits;
                merge_bits |=
                    node.forced_member_bits[static_cast<std::size_t>(rb)];
            } else {
                auto& merge_members =
                    node.forced_members[static_cast<std::size_t>(ra)];
                const auto& absorbed_members =
                    node.forced_members[static_cast<std::size_t>(rb)];
                changes.old_merge_member_count = merge_members.size();
                merge_members.insert(
                    merge_members.end(),
                    absorbed_members.begin(), absorbed_members.end());
            }
            node.forced_parent[static_cast<std::size_t>(rb)] = ra;
            node.forced_comp_size[static_cast<std::size_t>(ra)]
                += node.forced_comp_size[static_cast<std::size_t>(rb)];
            if (node.forced_rank[static_cast<std::size_t>(ra)]
                == node.forced_rank[static_cast<std::size_t>(rb)]) {
                ++node.forced_rank[static_cast<std::size_t>(ra)];
            }
        }

        ++node.forced_degree[static_cast<std::size_t>(e.u)];
        ++node.forced_degree[static_cast<std::size_t>(e.v)];
        adjustAvailableDegree(e.u, 1);
        adjustAvailableDegree(e.v, 1);

        node.forced_edges.push_back(e);
        if (e.u != 0 && e.v != 0) {
            node.forced_mst_cost += e.w;
            ++node.forced_mst_count;
        }
        return true;
    };

    auto revert_force = [&](const ForceChanges& changes) {
        node.forced[changes.forced_id] = 0;

        node.forced_parent[static_cast<std::size_t>(changes.root_u)] = changes.root_u;
        node.forced_parent[static_cast<std::size_t>(changes.root_v)] = changes.root_v;
        node.forced_rank[static_cast<std::size_t>(changes.root_u)] = changes.old_rank_u;
        node.forced_rank[static_cast<std::size_t>(changes.root_v)] = changes.old_rank_v;
        node.forced_comp_size[static_cast<std::size_t>(changes.root_u)] = changes.old_size_u;
        node.forced_comp_size[static_cast<std::size_t>(changes.root_v)] = changes.old_size_v;
        if (changes.merge_root >= 0) {
            if (!node.forced_member_bits.empty()) {
                node.forced_member_bits[
                    static_cast<std::size_t>(changes.merge_root)] =
                    changes.old_merge_member_bits;
            } else {
                node.forced_members[
                    static_cast<std::size_t>(changes.merge_root)].resize(
                    changes.old_merge_member_count);
            }
        }

        --node.forced_degree[static_cast<std::size_t>(changes.fu)];
        --node.forced_degree[static_cast<std::size_t>(changes.fv)];
        adjustAvailableDegree(changes.fu, -1);
        adjustAvailableDegree(changes.fv, -1);

        // 直接恢复快照，避免大量 sibling 的 +w/-w 让缓存权值逐步漂移。
        restoreForcedMstCache(
            node, changes.old_forced_mst_cost, changes.old_forced_mst_count);
        node.forced_edges.pop_back();
    };

    // |B|=1 时，BP 已严格证明任何改进 tour 都必须包含这条边。
    // 直接在当前逻辑节点传播 x_e=1，避免创建只有一个孩子的 BP 节点。
    if (B_set.size() == 1) {
        ForceChanges force_changes;
        if (!apply_force(B_set.front().edge, force_changes)) {
            ++result_.stats.nodes_pruned_infeasible;
            return;
        }

        const std::size_t candidate_checkpoint = candidate_undo_.size();
        const std::size_t tree_checkpoint = tree_undo_.size();
        const bool candidates_feasible = filterActiveCandidates(
            node, force_changes.merged_scan_root,
            force_changes.merged_scan_count,
            force_changes.merged_scan_bits);
        if (candidates_feasible) {
            bool tree_valid = false;
#ifndef TSP_DISABLE_INCREMENTAL_ONETREE
            tree_valid = true;
            if (candidate_undo_.size() != candidate_checkpoint) {
                tree_valid = updateOneTreeAfterActiveRemoval(
                    node, current_tree, candidate_checkpoint, true);
            }

#ifdef TSP_VERIFY_INCREMENTAL_STATE
            const OneTree rebuilt = computeOneTree(node, candidates_sorted_);
            if (tree_valid != rebuilt.feasible
                || (tree_valid
                    && !costsNumericallyEqual(
                        current_tree.cost, rebuilt.cost,
                        static_cast<std::size_t>(n_)))) {
                throw std::runtime_error(
                    "propagated singleton 1-tree differs from a complete rebuild");
            }
#endif

            if (!tree_valid) {
                rollbackOneTree(current_tree, tree_checkpoint);
                OneTree rebuilt = computeOneTree(node, candidates_sorted_);
                tree_valid = rebuilt.feasible;
                if (tree_valid) {
                    replaceOneTreeWithRebuild(
                        current_tree, std::move(rebuilt));
                }
            }
#else
            OneTree rebuilt = computeOneTree(node, candidates_sorted_);
            tree_valid = rebuilt.feasible;
            if (tree_valid) {
                replaceOneTreeWithRebuild(current_tree, std::move(rebuilt));
            }
#endif

            if (tree_valid) {
                search(node, current_tree, depth, false);
            } else {
                ++result_.stats.nodes_pruned_infeasible;
            }
        } else {
            ++result_.stats.nodes_pruned_infeasible;
        }

        rollbackOneTree(current_tree, tree_checkpoint);
        rollbackCandidates(node, candidate_checkpoint);
        revert_force(force_changes);
        return;
    }

    // Step 5: 直接枚举互斥的 force 子节点：
    //   force B[0]
    //   forbid B[0] + force B[1]
    //   ...
    //   forbid B[0..k-2] + force B[k-1]
    // 全部 B 边均 forbidden 的剩余分支已由 bpPartition 的终止测试证明不可改进。
    // parent checkpoint 包围整个 B 集枚举；child checkpoint 只包围当前
    // force 子节点。这样此前 B 边的 forbid 前缀可跨兄弟节点保留。
    const std::size_t parent_candidate_checkpoint = candidate_undo_.size();
    const std::size_t parent_tree_checkpoint = tree_undo_.size();

    for (std::size_t idx = 0; idx < B_set.size(); ++idx) {
        const BranchChoice& choice = B_set[idx];
        const Edge& branch_edge = choice.edge;
        ForceChanges force_changes;
        if (apply_force(branch_edge, force_changes)) {
            const std::size_t child_candidate_checkpoint = candidate_undo_.size();
            const std::size_t child_tree_checkpoint = tree_undo_.size();

            const bool ok = filterActiveCandidates(
                node, force_changes.merged_scan_root,
                force_changes.merged_scan_count,
                force_changes.merged_scan_bits);
            if (ok) {
                // B[i] 已经位于当前 prefix tree 中。force 后只需增量
                // 替换因度数/子回路过滤而失效的其他树边。
                bool tree_valid = false;
#ifndef TSP_DISABLE_INCREMENTAL_ONETREE
                tree_valid = true;
                if (candidate_undo_.size() != child_candidate_checkpoint) {
                    tree_valid = updateOneTreeAfterActiveRemoval(
                        node, current_tree,
                        child_candidate_checkpoint, true);
                }

#ifdef TSP_VERIFY_INCREMENTAL_STATE
                const OneTree rebuilt = computeOneTree(node, candidates_sorted_);
                if (tree_valid != rebuilt.feasible
                    || (tree_valid
                        && !costsNumericallyEqual(
                            current_tree.cost, rebuilt.cost,
                            static_cast<std::size_t>(n_)))) {
                    throw std::runtime_error(
                        "incremental child 1-tree differs from a complete rebuild");
                }
#endif

                if (!tree_valid) {
                    rollbackOneTree(current_tree, child_tree_checkpoint);
                    OneTree rebuilt = computeOneTree(node, candidates_sorted_);
                    tree_valid = rebuilt.feasible;
                    if (tree_valid) {
                        replaceOneTreeWithRebuild(current_tree, std::move(rebuilt));
                    }
                }
#else
                OneTree rebuilt = computeOneTree(node, candidates_sorted_);
                tree_valid = rebuilt.feasible;
                if (tree_valid) {
                    replaceOneTreeWithRebuild(current_tree, std::move(rebuilt));
                }
#endif

                ++result_.stats.nodes_created;
                if (tree_valid) {
                    search(node, current_tree, depth + 1);
                } else {
                    ++result_.stats.nodes_pruned_infeasible;
                }
            } else {
                ++result_.stats.nodes_pruned_infeasible;
            }
            rollbackOneTree(current_tree, child_tree_checkpoint);
            rollbackCandidates(node, child_candidate_checkpoint);
            revert_force(force_changes);
        } else {
            ++result_.stats.nodes_pruned_infeasible;
        }

        if (restart_search_requested_) {
            break;
        }

        if (idx + 1 == B_set.size()) {
            break;
        }

        // 当前 force 子节点完成后，仅把该边加入后续子节点的前缀禁止约束。
        const std::size_t eid = edgeId(branch_edge.u, branch_edge.v);
        node.forbidden[eid] = 1;
        deactivateCandidate(node, eid);

        // 将当前树从“此前 B 边被禁止”的状态推进到“连同当前
        // branch_edge 也被禁止”的状态，供 B[idx+1] 的 force 子节点使用。
        bool prefix_valid = false;
#ifndef TSP_DISABLE_INCREMENTAL_ONETREE
        if (!choice.replay_requires_rebuild
            && choice.has_forbid_replacement
            && choice.tree_edge_index < current_tree.edges.size()
            && edgeId(current_tree.edges[choice.tree_edge_index].u,
                      current_tree.edges[choice.tree_edge_index].v) == eid) {
            const Edge& replacement = choice.forbid_replacement;
            replaceOneTreeEdge(
                current_tree, choice.tree_edge_index, replacement, true);
            prefix_valid = true;
        }

#ifdef TSP_VERIFY_INCREMENTAL_STATE
        if (!choice.replay_requires_rebuild) {
            const OneTree rebuilt_prefix = computeOneTree(node, candidates_sorted_);
            if (prefix_valid != rebuilt_prefix.feasible
                || (prefix_valid
                    && !costsNumericallyEqual(
                        current_tree.cost, rebuilt_prefix.cost,
                        static_cast<std::size_t>(n_)))) {
                throw std::runtime_error(
                    "replayed BP prefix 1-tree differs from a complete rebuild");
            }
        }
#endif
#endif

        if (!prefix_valid) {
            OneTree rebuilt = computeOneTree(node, candidates_sorted_);
            prefix_valid = rebuilt.feasible;
            if (prefix_valid) {
                replaceOneTreeWithRebuild(current_tree, std::move(rebuilt));
            }
        }

        // bpPartition 只有在该前缀仍可行时才会生成下一项；若生产校验仍
        // 失败，下一子节点会通过完整重建判为 infeasible，不使用坏状态。
        (void)prefix_valid;
    }

    rollbackOneTree(current_tree, parent_tree_checkpoint);
    rollbackCandidates(node, parent_candidate_checkpoint);
    for (std::size_t index = 0; index + 1 < B_set.size(); ++index) {
        const Edge& edge = B_set[index].edge;
        node.forbidden[edgeId(edge.u, edge.v)] = 0;
    }
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
    const PartialSol& node_, const std::vector<Edge>& branch_candidates) const
{
    // result.cost 以 infinity 初始化；任何结构不足的提前返回自然表示不可行。
    OneTree result;
    result.cost = std::numeric_limits<double>::infinity();
    result.degree.assign(static_cast<std::size_t>(n_), 0);

    // 顶点 0 不参与 MST；从独立集合开始显式合并所有非根 forced 边。
    // 不能直接复用包含根边的 forced DSU，否则拆开 0 所在分量时会丢失其中的
    // 非根 forced 连通关系。
    DisjointSet tree_components(n_);

    // mst_edges/cost 只累计顶点 1..n-1 的内部 MST，直到加入根边阶段；
    // cost 始终位于当前势的调整权重坐标系，返回前再减势修正和舍入保护。
    int mst_edges = 0;
    double cost = 0.0;
    // edges 最终恰好包含 n 条 1-tree 边：n-2 条内部边和 2 条根边。
    std::vector<Edge> edges;
    edges.reserve(static_cast<std::size_t>(n_));

    // 1-tree 的第一部分：从部分解中直接取用 forced 边（O(#forced) 而非 O(n²)）。
    // 强制边已在 buildBranchCandidates 中验证无环、无度超。
    cost += node_.forced_mst_cost;
    mst_edges = node_.forced_mst_count;
    for (const Edge& e : node_.forced_edges) {
        if (e.u == 0 || e.v == 0) continue;
        tree_components.unite(e.u, e.v);
        edges.push_back(e);
    }
    // branch_candidates 由调用者保证已按权重升序排列，无需再次排序。

    // Kruskal：按权重从小到大加入不会成环的边，直到 MST 有 n-2 条边。
    // 跳过与顶点 0 相连的边，它们留给 1-tree 的第二部分处理。
    for (std::size_t candidate_index = nextActiveCandidate(
             node_, 0, branch_candidates.size());
         candidate_index < branch_candidates.size();
         candidate_index = nextActiveCandidate(
             node_, candidate_index + 1, branch_candidates.size())) {
        const Edge& edge = branch_candidates[candidate_index];
        if (mst_edges == n_ - 2) {
            break;
        }
        if (edge.u == 0 || edge.v == 0) {
            continue;
        }
        const std::size_t eid = edgeId(edge.u, edge.v);
        if (node_.forced[eid] || node_.forbidden[eid]) {
            continue;
        }
        if (!node_.candidate_mask.empty() && !node_.candidate_mask[eid]) {
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
    // root_edges 前 root_edge_count 个槽位有效；forced 根边先占位，剩余
    // 槽位由 root_candidates_sorted_ 中最轻的 active 边补齐。
    std::array<Edge, 2> root_edges{};
    std::size_t root_edge_count = 0;
    auto add_root_edge = [&](const Edge& e) {
        if (root_edge_count < root_edges.size()) {
            root_edges[root_edge_count++] = e;
        }
    };

    // 先从 forced 边列表中收集与顶点 0 相连的（≤2 条）。
    for (const Edge& e : node_.forced_edges) {
        if (e.u == 0 || e.v == 0) {
            add_root_edge(e);
        }
    }

    // 从预排序列表中补充可用的候选边，直至凑满 2 条。
    for (const Edge& e : root_candidates_sorted_) {
        if (root_edge_count == root_edges.size()) break;
        const std::size_t eid = edgeId(e.u, e.v);
        if (node_.forced[eid]) continue;       // 已由 forced 列表计入
        if (node_.forbidden[eid]) continue;    // 被当前节点禁止
        if (!isCandidateActive(node_, eid)) continue;
        add_root_edge(e);
    }
    if (root_edge_count != root_edges.size()) {
        return result;
    }

    for (std::size_t i = 0; i < root_edge_count; ++i) {
        edges.push_back(root_edges[i]);
        cost += root_edges[i].w;
    }

    // 统计 1-tree 中每个顶点的度数，用于判断是否已经得到一条 TSP 回路。
    for (const Edge& edge : edges) {
        ++result.degree[static_cast<std::size_t>(edge.u)];
        ++result.degree[static_cast<std::size_t>(edge.v)];
    }

    result.feasible = true;
    result.cost = cost - potential_correction_ - potential_roundoff_guard_;
    result.edges = std::move(edges);
#ifndef TSP_DISABLE_INCREMENTAL_ONETREE
    // 填充 O(1) 边→索引映射，替代增量路径中的 find_if 扫描。
    {
        const std::size_t edge_state_size = static_cast<std::size_t>(n_) * static_cast<std::size_t>(n_);
        result.edge_index_in_tree.assign(edge_state_size, -1);
        for (std::size_t i = 0; i < result.edges.size(); ++i) {
            const std::size_t eid = edgeId(result.edges[i].u, result.edges[i].v);
            result.edge_index_in_tree[eid] = static_cast<int>(i);
        }
    }
    initializeDynamicMst(result);
#endif
    return result;
}

bool BranchBoundSolver::updateOneTreeAfterForbid(
    const PartialSol& node,
    const std::vector<Edge>& branch_candidates,
    OneTree& tree,
    const Edge& forbidden_edge,
    bool record_undo) const
{
    // forbidden_id 是与候选排序无关的稳定边状态下标。
    const std::size_t forbidden_id = edgeId(forbidden_edge.u, forbidden_edge.v);
    if (!tree.feasible || !node.forbidden[forbidden_id] || node.forced[forbidden_id]) {
        return false;
    }

    // O(1) 查表替代 find_if 扫描。
    // removed_index 是禁边在 tree.edges 中的位置；若禁边不是树边，则当前
    // 最小 1-tree 仍然有效，无需寻找 replacement。
    std::size_t removed_index;
    if (!tree.edge_index_in_tree.empty()) {
        const int idx = tree.edge_index_in_tree[forbidden_id];
        if (idx < 0) {
            // 禁用非树边不会改变当前最小 1-tree。
            return true;
        }
        removed_index = static_cast<std::size_t>(idx);
    } else {
        auto removed = std::find_if(tree.edges.begin(), tree.edges.end(),
            [&](const Edge& edge) {
                return edgeId(edge.u, edge.v) == forbidden_id;
            });
        if (removed == tree.edges.end()) {
            return true;
        }
        removed_index = static_cast<std::size_t>(
            std::distance(tree.edges.begin(), removed));
    }

    // old_edge 是割边；replacement 最终指向预排序候选表中的稳定 Edge 对象。
    const Edge old_edge = tree.edges[removed_index];
    const Edge* replacement = nullptr;

    if (old_edge.u == 0 || old_edge.v == 0) {
        // 0 号顶点的两条边不属于 MST。删除其中一条后，只需补入下一条最轻合法根边。
        for (const Edge& candidate : root_candidates_sorted_) {
            const std::size_t candidate_id = edgeId(candidate.u, candidate.v);
            if (node.forbidden[candidate_id]) continue;
            if (!node.forced[candidate_id]
                && !isCandidateActive(node, candidate_id)) {
                continue;
            }

            const bool already_selected = std::any_of(
                tree.edges.begin(), tree.edges.end(), [&](const Edge& edge) {
                    return edgeId(edge.u, edge.v) == candidate_id;
                });
            if (already_selected) continue;

            replacement = &candidate;
            break;
        }
    } else {
        if (tree.mst_adjacency_bits.empty()) {
            initializeDynamicMst(tree);
        }
        replacement = findMstReplacement(node, branch_candidates, tree, old_edge);
    }

    if (replacement == nullptr) {
        markOneTreeInfeasible(tree, record_undo);
        return false;
    }

    replaceOneTreeEdge(tree, removed_index, *replacement, record_undo);
    return true;
}

bool BranchBoundSolver::updateOneTreeAfterCandidateRemoval(
    const PartialSol& node,
    const std::vector<Edge>& branch_candidates,
    OneTree& tree,
    const std::vector<std::size_t>& removed_edge_ids,
    bool record_undo) const
{
    for (const std::size_t removed_id : removed_edge_ids) {
        if (!updateOneTreeAfterRemovedCandidate(
                node, branch_candidates, tree, removed_id, record_undo)) {
            return false;
        }
    }
    return true;
}

bool BranchBoundSolver::updateOneTreeAfterActiveRemoval(
    const PartialSol& node, OneTree& tree,
    std::size_t candidate_checkpoint, bool record_undo) const
{
    // scratch 只收集“本轮刚失活且恰好仍在 1-tree 中”的非 forced 边，
    // 避免遍历和修复所有被候选过滤删除的边。
    removed_candidate_scratch_.clear();
    removed_candidate_scratch_.reserve(tree.edges.size());
    // removed_count 是 candidate_checkpoint 之后写入 undo 日志的失活边数。
    const std::size_t removed_count =
        candidate_undo_.size() - candidate_checkpoint;
    if (!tree.edge_index_in_tree.empty()
        && removed_count < tree.edges.size()) {
        for (std::size_t undo_index = candidate_checkpoint;
             undo_index < candidate_undo_.size(); ++undo_index) {
            const Edge& removed =
                candidates_sorted_[candidate_undo_[undo_index].index];
            const std::size_t edge_id = edgeId(removed.u, removed.v);
            if (!node.forced[edge_id]
                && tree.edge_index_in_tree[edge_id] >= 0) {
                removed_candidate_scratch_.push_back(edge_id);
            }
        }
    } else {
        for (const Edge& edge : tree.edges) {
            const std::size_t edge_id = edgeId(edge.u, edge.v);
            if (!node.forced[edge_id]
                && !isCandidateActive(node, edge_id)) {
                removed_candidate_scratch_.push_back(edge_id);
            }
        }
    }
    std::sort(
        removed_candidate_scratch_.begin(), removed_candidate_scratch_.end(),
        [&](std::size_t left, std::size_t right) {
            return edge_rank_by_id_[left] < edge_rank_by_id_[right];
        });
    for (const std::size_t removed_id : removed_candidate_scratch_) {
        if (!updateOneTreeAfterRemovedCandidate(
                node, candidates_sorted_, tree, removed_id, record_undo)) {
            return false;
        }
    }
    return true;
}

bool BranchBoundSolver::updateOneTreeAfterRemovedCandidate(
    const PartialSol& node, const std::vector<Edge>& branch_candidates,
    OneTree& tree, std::size_t removed_id, bool record_undo) const
{
    if (node.forced[removed_id]) return true;

    std::size_t removed_index;
    if (!tree.edge_index_in_tree.empty()) {
        const int idx = tree.edge_index_in_tree[removed_id];
        if (idx < 0) return true;
        removed_index = static_cast<std::size_t>(idx);
    } else {
        const auto it = std::find_if(tree.edges.begin(), tree.edges.end(),
            [&](const Edge& e) {
                return edgeId(e.u, e.v) == removed_id;
            });
        if (it == tree.edges.end()) return true;
        removed_index = static_cast<std::size_t>(
            std::distance(tree.edges.begin(), it));
    }

    const Edge old_edge = tree.edges[removed_index];
    const Edge* replacement = nullptr;
    if (old_edge.u == 0 || old_edge.v == 0) {
        for (const Edge& candidate : root_candidates_sorted_) {
            const std::size_t cid = edgeId(candidate.u, candidate.v);
            if (node.forbidden[cid] || node.forced[cid]) continue;
            if (!isCandidateActive(node, cid)) continue;
            if (!tree.edge_index_in_tree.empty()
                && tree.edge_index_in_tree[cid] >= 0) {
                continue;
            }
            if (tree.edge_index_in_tree.empty()) {
                const bool already_in_tree = std::any_of(
                    tree.edges.begin(), tree.edges.end(),
                    [&](const Edge& e) {
                        return edgeId(e.u, e.v) == cid;
                    });
                if (already_in_tree) continue;
            }
            replacement = &candidate;
            break;
        }
    } else {
        if (tree.mst_adjacency_bits.empty()) {
            initializeDynamicMst(tree);
        }
        replacement = findMstReplacement(node, branch_candidates, tree, old_edge);
    }

    if (replacement == nullptr) return false;
    replaceOneTreeEdge(tree, removed_index, *replacement, record_undo);
    return true;
}

const BranchBoundSolver::Edge* BranchBoundSolver::findMstReplacement(
    const PartialSol& node, const std::vector<Edge>& branch_candidates,
    const OneTree& tree, const Edge& removed_edge) const
{
    if (!markMstComponentWithoutEdge(tree, removed_edge)) {
        throw std::logic_error("dynamic MST state is missing or inconsistent");
    }

    // MST 交换性质保证合法 replacement 不会轻于被删树边，因此从
    // lower_bound(removed_edge.w) 开始扫描即可跳过不可能候选。
    const auto first = std::lower_bound(
        branch_candidates.begin(), branch_candidates.end(), removed_edge.w,
        [](const Edge& edge, double weight) { return edge.w < weight; });
    const std::size_t first_index = static_cast<std::size_t>(
        std::distance(branch_candidates.begin(), first));

    // 只有生产路径传入全局候选表时，稳定下标、active bitset 和 incident
    // 位图才属于同一坐标系；局部测试候选必须走兼容扫描路径。
    const bool use_global_candidates = &branch_candidates == &candidates_sorted_;
    if (use_global_candidates
        && candidate_word_count_ != 0
        && node.candidate_bits.size() == candidate_word_count_
        && candidate_incident_bits_.size()
            == static_cast<std::size_t>(n_) * candidate_word_count_
        && internal_candidate_bits_.size() == candidate_word_count_) {
        // 65..128 顶点已有固定双 word component mask。多数 replacement
        // 在 removed edge 的排序位置附近即可命中；先做有预算的顺序扫描，
        // 预算耗尽才支付完整 fundamental-cut XOR 的成本。
        if (mst_selected_component_word_count_ != 0) {
            // estimated_cut_words 估计构造完整 fundamental-cut 位图的工作量；
            // scan_budget 在顺序扫描和位图构造之间做自适应切换。
            const std::size_t component_size =
                populationCount(mst_selected_component_words_[0])
                + populationCount(mst_selected_component_words_[1]);
            const std::size_t estimated_cut_words =
                component_size * candidate_word_count_;
            const std::size_t scan_budget = std::max<std::size_t>(
                32, std::min<std::size_t>(256, estimated_cut_words / 8));
            std::size_t candidate_index = nextActiveCandidate(
                node, first_index, branch_candidates.size());
            std::size_t inspected = 0;
            while (candidate_index < branch_candidates.size()
                   && inspected < scan_budget) {
                const Edge& candidate = branch_candidates[candidate_index];
                ++inspected;
                if (candidate.u != 0 && candidate.v != 0) {
                    // nextActiveCandidate 已由权威 active bitset 排除了
                    // forced/forbidden/filtered 边；candidate_mask 与该位图
                    // 同步维护，无需再次访问三个 edge-id 状态数组。
                    auto in_component = [&](int vertex) {
                        const std::size_t word_index =
                            static_cast<std::size_t>(vertex) / 64;
                        return (mst_selected_component_words_[word_index]
                                & (std::uint64_t{1}
                                   << (static_cast<unsigned>(vertex)
                                       % 64))) != 0;
                    };
                    if (in_component(candidate.u)
                        != in_component(candidate.v)) {
                        return &candidate;
                    }
                }
                candidate_index = nextActiveCandidate(
                    node, candidate_index + 1,
                    branch_candidates.size());
            }
            if (candidate_index >= branch_candidates.size()) {
                return nullptr;
            }
        }

        mst_cut_candidate_bits_.assign(candidate_word_count_, 0);
        auto add_component_vertex = [&](int vertex) {
            const std::size_t row_begin =
                static_cast<std::size_t>(vertex) * candidate_word_count_;
            for (std::size_t word_index = 0;
                 word_index < candidate_word_count_; ++word_index) {
                // 分量内部边在两个端点行中各出现一次并相互抵消；跨割边
                // 只出现一次，因此异或结果恰好是 fundamental cut。
                mst_cut_candidate_bits_[word_index] ^=
                    candidate_incident_bits_[row_begin + word_index];
            }
        };
        if (mst_selected_component_bits_ != 0) {
            std::uint64_t component = mst_selected_component_bits_;
            while (component != 0) {
                const unsigned vertex = trailingZeroCount(component);
                component &= component - 1;
                add_component_vertex(static_cast<int>(vertex));
            }
        } else if (mst_selected_component_word_count_ != 0) {
            for (std::size_t component_word = 0;
                 component_word < mst_selected_component_word_count_;
                 ++component_word) {
                std::uint64_t component =
                    mst_selected_component_words_[component_word];
                while (component != 0) {
                    const std::size_t vertex = component_word * 64
                        + trailingZeroCount(component);
                    component &= component - 1;
                    if (vertex < static_cast<std::size_t>(n_)) {
                        add_component_vertex(static_cast<int>(vertex));
                    }
                }
            }
        } else {
            const std::vector<int>& component = mst_selected_component_is_left_
                ? mst_component_left_ : mst_component_right_;
            for (const int vertex : component) {
                add_component_vertex(vertex);
            }
        }

        std::size_t word_index = first_index / 64;
        const std::size_t bit_offset = first_index % 64;
        while (word_index < candidate_word_count_) {
            std::uint64_t pending = mst_cut_candidate_bits_[word_index]
                & node.candidate_bits[word_index]
                & internal_candidate_bits_[word_index];
            if (word_index == first_index / 64) {
                pending &= ~std::uint64_t{0} << bit_offset;
            }
            if (pending != 0) {
                const std::size_t candidate_index = word_index * 64
                    + trailingZeroCount(pending);
                if (candidate_index < branch_candidates.size()) {
                    return &branch_candidates[candidate_index];
                }
                return nullptr;
            }
            ++word_index;
        }
        return nullptr;
    }

    for (std::size_t candidate_index = nextActiveCandidate(
             node, first_index, branch_candidates.size());
         candidate_index < branch_candidates.size();
         candidate_index = nextActiveCandidate(
             node, candidate_index + 1, branch_candidates.size())) {
        const Edge& candidate = branch_candidates[candidate_index];
        if (candidate.u == 0 || candidate.v == 0) continue;
        const std::size_t candidate_id = edgeId(candidate.u, candidate.v);
        if (node.forced[candidate_id] || node.forbidden[candidate_id]) continue;
        if (!node.candidate_mask.empty() && !node.candidate_mask[candidate_id]) continue;

        auto in_selected_component = [&](int vertex) {
            if (mst_selected_component_bits_ != 0) {
                return (mst_selected_component_bits_
                        & (std::uint64_t{1}
                           << static_cast<unsigned>(vertex))) != 0;
            }
            if (mst_selected_component_word_count_ != 0) {
                const std::size_t word_index =
                    static_cast<std::size_t>(vertex) / 64;
                return (mst_selected_component_words_[word_index]
                        & (std::uint64_t{1}
                           << (static_cast<unsigned>(vertex) % 64))) != 0;
            }
            return mst_component_mark_[static_cast<std::size_t>(vertex)]
                == mst_selected_component_epoch_;
        };
        const bool u_in_component = in_selected_component(candidate.u);
        const bool v_in_component = in_selected_component(candidate.v);
        if (u_in_component != v_in_component) {
            return &candidate;
        }
    }
    return nullptr;
}

void BranchBoundSolver::initializeDynamicMst(OneTree& tree) const
{
    // 每个顶点一行、每行 ceil(n/64) 个 word；仅记录内部 MST 邻接关系。
    const std::size_t words_per_row = (static_cast<std::size_t>(n_) + 63) / 64;
    tree.mst_adjacency_bits.assign(
        static_cast<std::size_t>(n_) * words_per_row, 0);
    for (const Edge& edge : tree.edges) {
        if (edge.u == 0 || edge.v == 0) continue;
        setDynamicMstEdge(tree, edge.u, edge.v, true);
    }
#ifdef TSP_VERIFY_INCREMENTAL_STATE
    if (tree.feasible && !dynamicMstMatchesEdges(tree)) {
        throw std::logic_error("new dynamic MST state is inconsistent");
    }
#endif
}

void BranchBoundSolver::setDynamicMstEdge(
    OneTree& tree, int u, int v, bool present) const
{
    if (u == 0 || v == 0) return;
    const std::size_t words_per_row = (static_cast<std::size_t>(n_) + 63) / 64;
    const std::size_t expected_size = static_cast<std::size_t>(n_) * words_per_row;
    if (tree.mst_adjacency_bits.size() != expected_size) {
        throw std::logic_error("dynamic MST adjacency has the wrong size");
    }

    // update 修改一个方向的邻接位；无向边由下面两次调用保持对称。
    auto update = [&](int from, int to) {
        std::uint64_t& word = tree.mst_adjacency_bits[
            static_cast<std::size_t>(from) * words_per_row
            + static_cast<std::size_t>(to) / 64];
        const std::uint64_t bit =
            std::uint64_t{1} << (static_cast<std::size_t>(to) % 64);
        if (present) {
            word |= bit;
        } else {
            word &= ~bit;
        }
    };
    update(u, v);
    update(v, u);
}

void BranchBoundSolver::replaceOneTreeEdge(
    OneTree& tree, std::size_t edge_index, const Edge& replacement,
    bool record_undo) const
{
    if (edge_index >= tree.edges.size()) {
        throw std::logic_error("1-tree replacement index is out of range");
    }
    if (tree.mst_adjacency_bits.empty()) {
        initializeDynamicMst(tree);
    }

    // old_edge 与 replacement 必须同属根边区或内部 MST 区；一次交换同步
    // 更新边集、度数、成本、动态邻接和 edge_index_in_tree。
    const Edge old_edge = tree.edges[edge_index];
    if (record_undo) {
        TreeUndo undo;
        undo.edge_index = edge_index;
        undo.old_edge = old_edge;
        undo.new_edge = replacement;
        undo.old_cost = tree.cost;
        undo.old_feasible = tree.feasible;
        tree_undo_.push_back(undo);
    }
    const bool old_is_internal = old_edge.u != 0 && old_edge.v != 0;
    const bool replacement_is_internal = replacement.u != 0 && replacement.v != 0;
    if (old_is_internal != replacement_is_internal) {
        throw std::logic_error("1-tree replacement crossed the root/MST boundary");
    }
    if (old_is_internal) {
        setDynamicMstEdge(tree, old_edge.u, old_edge.v, false);
        setDynamicMstEdge(tree, replacement.u, replacement.v, true);
    }

    --tree.degree[static_cast<std::size_t>(old_edge.u)];
    --tree.degree[static_cast<std::size_t>(old_edge.v)];
    ++tree.degree[static_cast<std::size_t>(replacement.u)];
    ++tree.degree[static_cast<std::size_t>(replacement.v)];
    tree.cost += replacement.w - old_edge.w;
    tree.edges[edge_index] = replacement;
#ifndef TSP_DISABLE_INCREMENTAL_ONETREE
    // 维护 O(1) 边→索引映射。
    if (!tree.edge_index_in_tree.empty()) {
        tree.edge_index_in_tree[edgeId(old_edge.u, old_edge.v)] = -1;
        tree.edge_index_in_tree[edgeId(replacement.u, replacement.v)] = static_cast<int>(edge_index);
    }
#endif
#ifdef TSP_VERIFY_INCREMENTAL_STATE
    if (tree.feasible && !dynamicMstMatchesEdges(tree)) {
        throw std::logic_error("dynamic MST cut/link produced an invalid tree");
    }
#endif
}

void BranchBoundSolver::markOneTreeInfeasible(
    OneTree& tree, bool record_undo) const
{
    if (record_undo) {
        TreeUndo undo;
        undo.old_cost = tree.cost;
        undo.old_feasible = tree.feasible;
        undo.state_only = true;
        tree_undo_.push_back(undo);
    }
    tree.feasible = false;
    tree.cost = std::numeric_limits<double>::infinity();
}

void BranchBoundSolver::replaceOneTreeWithRebuild(
    OneTree& tree, OneTree rebuilt) const
{
    tree_snapshot_undo_.push_back(std::move(tree));
    tree = std::move(rebuilt);
    TreeUndo undo;
    undo.full_snapshot = true;
    tree_undo_.push_back(undo);
}

void BranchBoundSolver::rollbackOneTree(
    OneTree& tree, std::size_t checkpoint) const
{
    // checkpoint 是进入当前逻辑作用域前 tree_undo_.size()；逆序弹出可确保
    // 多次边交换、状态变化和完整快照按严格相反顺序恢复。
    while (tree_undo_.size() > checkpoint) {
        const TreeUndo undo = tree_undo_.back();
        tree_undo_.pop_back();

        if (undo.full_snapshot) {
            tree = std::move(tree_snapshot_undo_.back());
            tree_snapshot_undo_.pop_back();
            continue;
        }
        if (undo.state_only) {
            tree.cost = undo.old_cost;
            tree.feasible = undo.old_feasible;
            continue;
        }

        const bool old_is_internal = undo.old_edge.u != 0 && undo.old_edge.v != 0;
        if (old_is_internal) {
            setDynamicMstEdge(tree, undo.new_edge.u, undo.new_edge.v, false);
            setDynamicMstEdge(tree, undo.old_edge.u, undo.old_edge.v, true);
        }
        --tree.degree[static_cast<std::size_t>(undo.new_edge.u)];
        --tree.degree[static_cast<std::size_t>(undo.new_edge.v)];
        ++tree.degree[static_cast<std::size_t>(undo.old_edge.u)];
        ++tree.degree[static_cast<std::size_t>(undo.old_edge.v)];
        if (!tree.edge_index_in_tree.empty()) {
            tree.edge_index_in_tree[edgeId(undo.new_edge.u, undo.new_edge.v)] = -1;
            tree.edge_index_in_tree[edgeId(undo.old_edge.u, undo.old_edge.v)] =
                static_cast<int>(undo.edge_index);
        }
        tree.edges[undo.edge_index] = undo.old_edge;
        tree.cost = undo.old_cost;
        tree.feasible = undo.old_feasible;
    }
}

bool BranchBoundSolver::markMstComponentWithoutEdge(
    const OneTree& tree, const Edge& removed_edge) const
{
    // words_per_row 决定采用单 word、双 word 或通用 epoch 标记路径。
    const std::size_t words_per_row = (static_cast<std::size_t>(n_) + 63) / 64;
    const std::size_t expected_size = static_cast<std::size_t>(n_) * words_per_row;
    if (removed_edge.u == 0 || removed_edge.v == 0
        || tree.mst_adjacency_bits.size() != expected_size) {
        return false;
    }

    auto has_edge = [&](int from, int to) {
        return (tree.mst_adjacency_bits[
                    static_cast<std::size_t>(from) * words_per_row
                    + static_cast<std::size_t>(to) / 64]
                & (std::uint64_t{1}
                   << (static_cast<std::size_t>(to) % 64)))
            != 0;
    };
    if (!has_edge(removed_edge.u, removed_edge.v)
        || !has_edge(removed_edge.v, removed_edge.u)) {
        return false;
    }

    // n <= 64 时每个邻接行只有一个 machine word。直接用 seen/frontier
    // bitset 完成双向扩展，避免通用路径为每个访问顶点写 epoch、
    // push 两个动态 vector，并在只有一个 word 时仍执行内层 word 循环。
    if (words_per_row == 1) {
        mst_selected_component_words_ = {};
        mst_selected_component_word_count_ = 0;
        std::uint64_t left_seen =
            std::uint64_t{1} << static_cast<unsigned>(removed_edge.u);
        std::uint64_t right_seen =
            std::uint64_t{1} << static_cast<unsigned>(removed_edge.v);
        std::uint64_t left_frontier = left_seen;
        std::uint64_t right_frontier = right_seen;

        auto expand_fast = [&](std::uint64_t& frontier,
                               std::uint64_t& own_seen,
                               std::uint64_t other_seen) {
            const int vertex =
                static_cast<int>(trailingZeroCount(frontier));
            frontier &= frontier - 1;
            std::uint64_t neighbors =
                tree.mst_adjacency_bits[static_cast<std::size_t>(vertex)];
            if (vertex == removed_edge.u) {
                neighbors &= ~(
                    std::uint64_t{1}
                    << static_cast<unsigned>(removed_edge.v));
            } else if (vertex == removed_edge.v) {
                neighbors &= ~(
                    std::uint64_t{1}
                    << static_cast<unsigned>(removed_edge.u));
            }
            if ((neighbors & other_seen) != 0) {
                return false;
            }
            neighbors &= ~own_seen;
            own_seen |= neighbors;
            frontier |= neighbors;
            return true;
        };

        bool selected_left = true;
        while (true) {
            if (!expand_fast(
                    left_frontier,
                    left_seen, right_seen)) {
                return false;
            }
            if (left_frontier == 0) {
                mst_selected_component_bits_ = left_seen;
                break;
            }

            if (!expand_fast(
                    right_frontier,
                    right_seen, left_seen)) {
                return false;
            }
            if (right_frontier == 0) {
                mst_selected_component_bits_ = right_seen;
                selected_left = false;
                break;
            }
        }

        mst_selected_component_is_left_ = selected_left;
        return true;
    }

    // 65..128 顶点时邻接矩阵每行恰好两个 word。使用固定大小
    // seen/frontier 保留双向“小分量优先”语义，同时避免 epoch 写入和
    // vector push。
    if (words_per_row == 2) {
        mst_selected_component_bits_ = 0;
        std::array<std::uint64_t, 2> left_seen{};
        std::array<std::uint64_t, 2> right_seen{};
        std::array<std::uint64_t, 2> left_frontier{};
        std::array<std::uint64_t, 2> right_frontier{};

        auto set_vertex = [](std::array<std::uint64_t, 2>& words,
                             int vertex) {
            words[static_cast<std::size_t>(vertex) / 64] |=
                std::uint64_t{1}
                << (static_cast<unsigned>(vertex) % 64);
        };
        set_vertex(left_seen, removed_edge.u);
        set_vertex(right_seen, removed_edge.v);
        left_frontier = left_seen;
        right_frontier = right_seen;

        auto frontier_empty =
            [](const std::array<std::uint64_t, 2>& frontier) {
                return (frontier[0] | frontier[1]) == 0;
            };
        auto expand_fast =
            [&](std::array<std::uint64_t, 2>& frontier,
                std::array<std::uint64_t, 2>& own_seen,
                const std::array<std::uint64_t, 2>& other_seen) {
                const std::size_t frontier_word =
                    frontier[0] != 0 ? 0 : 1;
                const unsigned bit_index =
                    trailingZeroCount(frontier[frontier_word]);
                frontier[frontier_word] &=
                    frontier[frontier_word] - 1;
                const int vertex = static_cast<int>(
                    frontier_word * 64 + bit_index);
                const std::size_t row_begin =
                    static_cast<std::size_t>(vertex) * 2;
                std::array<std::uint64_t, 2> neighbors{
                    tree.mst_adjacency_bits[row_begin],
                    tree.mst_adjacency_bits[row_begin + 1]};
                if (vertex == removed_edge.u) {
                    neighbors[
                        static_cast<std::size_t>(removed_edge.v) / 64]
                        &= ~(std::uint64_t{1}
                             << (static_cast<unsigned>(removed_edge.v) % 64));
                } else if (vertex == removed_edge.v) {
                    neighbors[
                        static_cast<std::size_t>(removed_edge.u) / 64]
                        &= ~(std::uint64_t{1}
                             << (static_cast<unsigned>(removed_edge.u) % 64));
                }
                for (std::size_t word_index = 0;
                     word_index < 2; ++word_index) {
                    if ((neighbors[word_index]
                         & other_seen[word_index]) != 0) {
                        return false;
                    }
                    neighbors[word_index] &= ~own_seen[word_index];
                    own_seen[word_index] |= neighbors[word_index];
                    frontier[word_index] |= neighbors[word_index];
                }
                return true;
            };

        bool selected_left = true;
        while (true) {
            if (!expand_fast(left_frontier, left_seen, right_seen)) {
                return false;
            }
            if (frontier_empty(left_frontier)) {
                mst_selected_component_words_ = left_seen;
                break;
            }

            if (!expand_fast(right_frontier, right_seen, left_seen)) {
                return false;
            }
            if (frontier_empty(right_frontier)) {
                mst_selected_component_words_ = right_seen;
                selected_left = false;
                break;
            }
        }

        mst_selected_component_word_count_ = 2;
        mst_selected_component_is_left_ = selected_left;
        return true;
    }

    mst_selected_component_bits_ = 0;
    mst_selected_component_words_ = {};
    mst_selected_component_word_count_ = 0;
    if (mst_component_mark_.size() != static_cast<std::size_t>(n_)) {
        mst_component_mark_.assign(static_cast<std::size_t>(n_), 0);
        mst_component_epoch_ = 0;
        mst_component_left_.reserve(static_cast<std::size_t>(n_));
        mst_component_right_.reserve(static_cast<std::size_t>(n_));
    }
    if (mst_component_epoch_
        > std::numeric_limits<std::uint32_t>::max() - 2) {
        std::fill(mst_component_mark_.begin(), mst_component_mark_.end(), 0);
        mst_component_epoch_ = 0;
    }
    const std::uint32_t left_epoch = ++mst_component_epoch_;
    const std::uint32_t right_epoch = ++mst_component_epoch_;

    mst_component_left_.clear();
    mst_component_right_.clear();
    mst_component_left_.push_back(removed_edge.u);
    mst_component_right_.push_back(removed_edge.v);
    mst_component_mark_[static_cast<std::size_t>(removed_edge.u)] =
        left_epoch;
    mst_component_mark_[static_cast<std::size_t>(removed_edge.v)] =
        right_epoch;

    auto expand = [&](std::vector<int>& component, std::size_t& cursor,
                      std::uint32_t own_epoch,
                      std::uint32_t other_epoch) -> bool {
        const int vertex = component[cursor++];
        for (std::size_t word_index = 0; word_index < words_per_row; ++word_index) {
            std::uint64_t neighbors = tree.mst_adjacency_bits[
                static_cast<std::size_t>(vertex) * words_per_row + word_index];
            if (vertex == removed_edge.u
                && word_index == static_cast<std::size_t>(removed_edge.v) / 64) {
                neighbors &= ~(
                    std::uint64_t{1}
                    << (static_cast<std::size_t>(removed_edge.v) % 64));
            } else if (vertex == removed_edge.v
                       && word_index == static_cast<std::size_t>(removed_edge.u) / 64) {
                neighbors &= ~(
                    std::uint64_t{1}
                    << (static_cast<std::size_t>(removed_edge.u) % 64));
            }

            while (neighbors != 0) {
                const unsigned bit_index = trailingZeroCount(neighbors);
                neighbors &= neighbors - 1;
                const std::size_t neighbor_index = word_index * 64 + bit_index;
                if (neighbor_index >= static_cast<std::size_t>(n_)) continue;
                const std::uint32_t mark = mst_component_mark_[neighbor_index];
                if (mark == own_epoch) continue;
                if (mark == other_epoch) return false;
                mst_component_mark_[neighbor_index] = own_epoch;
                component.push_back(static_cast<int>(neighbor_index));
            }
        }
        return true;
    };

    std::size_t left_cursor = 0;
    std::size_t right_cursor = 0;
    while (true) {
        if (!expand(mst_component_left_, left_cursor, left_epoch, right_epoch)) {
            return false;
        }
        if (left_cursor == mst_component_left_.size()) {
            mst_selected_component_is_left_ = true;
            mst_selected_component_epoch_ = left_epoch;
            return true;
        }

        if (!expand(mst_component_right_, right_cursor, right_epoch, left_epoch)) {
            return false;
        }
        if (right_cursor == mst_component_right_.size()) {
            mst_selected_component_is_left_ = false;
            mst_selected_component_epoch_ = right_epoch;
            return true;
        }
    }
}

bool BranchBoundSolver::dynamicMstMatchesEdges(const OneTree& tree) const
{
    const std::size_t words_per_row = (static_cast<std::size_t>(n_) + 63) / 64;
    const std::size_t expected_size = static_cast<std::size_t>(n_) * words_per_row;
    if (!tree.feasible || tree.mst_adjacency_bits.size() != expected_size) return false;
    if (tree.edges.size() != static_cast<std::size_t>(n_)) return false;

    std::vector<std::uint64_t> expected(expected_size, 0);
    DisjointSet components(n_);
    int root_edge_count = 0;
    int internal_edge_count = 0;
    auto add_expected = [&](int from, int to) -> bool {
        if (from <= 0 || from >= n_ || to <= 0 || to >= n_ || from == to) {
            return false;
        }
        std::uint64_t& word = expected[
            static_cast<std::size_t>(from) * words_per_row
            + static_cast<std::size_t>(to) / 64];
        const std::uint64_t bit =
            std::uint64_t{1} << (static_cast<std::size_t>(to) % 64);
        if ((word & bit) != 0) return false;
        word |= bit;
        return true;
    };

    for (const Edge& edge : tree.edges) {
        if (edge.u < 0 || edge.u >= n_ || edge.v < 0 || edge.v >= n_
            || edge.u == edge.v) {
            return false;
        }
        if (edge.u == 0 || edge.v == 0) {
            ++root_edge_count;
            continue;
        }
        ++internal_edge_count;
        if (!components.unite(edge.u, edge.v)
            || !add_expected(edge.u, edge.v)
            || !add_expected(edge.v, edge.u)) {
            return false;
        }
    }
    if (root_edge_count != 2 || internal_edge_count != n_ - 2) return false;
    if (n_ > 1) {
        const int component = components.find(1);
        for (int vertex = 2; vertex < n_; ++vertex) {
            if (components.find(vertex) != component) return false;
        }
    }
    return expected == tree.mst_adjacency_bits;
}

bool BranchBoundSolver::oneTreeSatisfiesConstraints(
    const PartialSol& node, const OneTree& tree) const
{
    const std::size_t state_size = static_cast<std::size_t>(n_)
        * static_cast<std::size_t>(n_);
    if (!tree.feasible
        || static_cast<int>(tree.edges.size()) != n_
        || static_cast<int>(tree.degree.size()) != n_
        || node.forced.size() < state_size
        || node.forbidden.size() < state_size
        || (!node.candidate_mask.empty()
            && node.candidate_mask.size() < state_size)) {
        return false;
    }

    std::vector<std::size_t> selected_ids;
    selected_ids.reserve(static_cast<std::size_t>(n_));
    std::vector<int> degree(static_cast<std::size_t>(n_), 0);
    DisjointSet components(n_);
    int root_edge_count = 0;
    int internal_edge_count = 0;
    double recomputed_cost = 0.0;

    for (const Edge& edge : tree.edges) {
        if (edge.u < 0 || edge.u >= n_ || edge.v < 0 || edge.v >= n_
            || edge.u == edge.v || !isFinite(edge.w)
            || !costsNumericallyEqual(
                edge.w, adjustedEdgeWeight(edge.u, edge.v), 1)) {
            return false;
        }

        const std::size_t id = edgeId(edge.u, edge.v);
        if (node.forbidden[id]) {
            return false;
        }
        if (!node.forced[id] && !isCandidateActive(node, id)) {
            return false;
        }
        selected_ids.push_back(id);
        recomputed_cost += edge.w;
        ++degree[static_cast<std::size_t>(edge.u)];
        ++degree[static_cast<std::size_t>(edge.v)];

        if (edge.u == 0 || edge.v == 0) {
            ++root_edge_count;
        } else {
            ++internal_edge_count;
            if (!components.unite(edge.u, edge.v)) {
                return false;
            }
        }
    }

    std::sort(selected_ids.begin(), selected_ids.end());
    if (std::adjacent_find(selected_ids.begin(), selected_ids.end())
        != selected_ids.end()) {
        return false;
    }

    if (root_edge_count != 2 || internal_edge_count != n_ - 2
        || degree != tree.degree
        || !costsNumericallyEqual(
            recomputed_cost - potential_correction_ - potential_roundoff_guard_,
            tree.cost,
            static_cast<std::size_t>(n_))) {
        return false;
    }

    const int internal_root = components.find(1);
    for (int vertex = 2; vertex < n_; ++vertex) {
        if (components.find(vertex) != internal_root) {
            return false;
        }
    }

    // forced_edges 是强制状态的稀疏权威列表；逐项确认它们均出现在树中。
    for (const Edge& forced_edge : node.forced_edges) {
        const std::size_t id = edgeId(forced_edge.u, forced_edge.v);
        if (!node.forced[id] || node.forbidden[id]
            || !std::binary_search(selected_ids.begin(), selected_ids.end(), id)) {
            return false;
        }
    }
    return true;
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

bool BranchBoundSolver::buildBranchCandidates(
    const PartialSol& node,
    std::vector<Edge>& branch_candidates,
    std::vector<std::size_t>* removed_edge_ids) const
{
    // 利用节点中缓存的 forced_degree 和 forced 并查集，避免 O(n²) 扫描。
    // apply_force 已保证：无 forced/forbidden 冲突、度数 ≤ 2、无子回路，
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
        } else if (removed_edge_ids != nullptr) {
            removed_edge_ids->push_back(eid);
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

bool BranchBoundSolver::filterActiveCandidates(
    PartialSol& node, int merged_scan_root,
    std::size_t merged_scan_count,
    std::uint64_t merged_scan_bits,
    std::vector<std::size_t>* removed_edge_ids) const
{
    const std::size_t undo_begin = candidate_undo_.size();
    const Edge& forced_edge = node.forced_edges.back();
    deactivateCandidate(node, edgeId(forced_edge.u, forced_edge.v));

    auto for_each_active_incident = [&](int vertex, const auto& visit) {
        const std::size_t row_begin =
            static_cast<std::size_t>(vertex) * candidate_word_count_;
        for (std::size_t word_index = 0;
             word_index < candidate_word_count_; ++word_index) {
            std::uint64_t pending = node.candidate_bits[word_index]
                & candidate_incident_bits_[row_begin + word_index];
            while (pending != 0) {
                const std::size_t index = word_index * 64
                    + trailingZeroCount(pending);
                pending &= pending - 1;
                visit(index);
            }
        }
    };
    if (node.forced_degree[static_cast<std::size_t>(forced_edge.u)] >= 2) {
        deactivateIncidentCandidates(node, forced_edge.u);
    }
    if (forced_edge.v != forced_edge.u
        && node.forced_degree[static_cast<std::size_t>(forced_edge.v)] >= 2) {
        deactivateIncidentCandidates(node, forced_edge.v);
    }

    if (merged_scan_bits != 0 && !node.forced_member_bits.empty()) {
        auto forced_root = [&](int vertex) {
            while (node.forced_parent[static_cast<std::size_t>(vertex)] != vertex) {
                vertex = node.forced_parent[static_cast<std::size_t>(vertex)];
            }
            return vertex;
        };
        const int combined_root = forced_root(forced_edge.u);
        if (node.forced_comp_size[static_cast<std::size_t>(combined_root)] < n_) {
            const std::uint64_t combined_bits =
                node.forced_member_bits[static_cast<std::size_t>(combined_root)];
            while (merged_scan_bits != 0) {
                const unsigned vertex_index =
                    trailingZeroCount(merged_scan_bits);
                merged_scan_bits &= merged_scan_bits - 1;
                const int vertex = static_cast<int>(vertex_index);
                for_each_active_incident(vertex, [&](std::size_t index) {
                    const Edge& edge = candidates_sorted_[index];
                    if (edge.u == 0 || edge.v == 0) return;
                    const int other = edge.u == vertex ? edge.v : edge.u;
                    if ((combined_bits
                         & (std::uint64_t{1}
                            << static_cast<unsigned>(other))) != 0) {
                        deactivateCandidateByIndex(node, index);
                    }
                });
            }
        }
    } else if (merged_scan_root >= 0 && merged_scan_count != 0) {
        const auto& members =
            node.forced_members[static_cast<std::size_t>(merged_scan_root)];
        auto forced_root = [&](int vertex) {
            while (node.forced_parent[static_cast<std::size_t>(vertex)] != vertex) {
                vertex = node.forced_parent[static_cast<std::size_t>(vertex)];
            }
            return vertex;
        };
        const int combined_root = forced_root(members.front());
        std::uint64_t combined_member_bits = 0;
        if (n_ <= 64) {
            const auto& combined_members =
                node.forced_members[static_cast<std::size_t>(combined_root)];
            for (const int vertex : combined_members) {
                combined_member_bits |=
                    std::uint64_t{1} << static_cast<unsigned>(vertex);
            }
        }
        if (node.forced_comp_size[static_cast<std::size_t>(combined_root)] < n_) {
            for (std::size_t member_index = 0;
                 member_index < merged_scan_count; ++member_index) {
                const int vertex = members[member_index];
                for_each_active_incident(vertex, [&](std::size_t index) {
                    const Edge& edge = candidates_sorted_[index];
                    if (edge.u == 0 || edge.v == 0) return;
                    const int other = edge.u == vertex ? edge.v : edge.u;
                    const bool other_in_component = n_ <= 64
                        ? (combined_member_bits
                           & (std::uint64_t{1}
                              << static_cast<unsigned>(other))) != 0
                        : forced_root(other) == combined_root;
                    if (other_in_component) {
                        deactivateCandidateByIndex(node, index);
                    }
                });
            }
        }
    }

    if (removed_edge_ids != nullptr) {
        removed_edge_ids->reserve(
            removed_edge_ids->size() + candidate_undo_.size() - undo_begin);
        for (std::size_t i = undo_begin; i < candidate_undo_.size(); ++i) {
            const Edge& removed = candidates_sorted_[candidate_undo_[i].index];
            removed_edge_ids->push_back(edgeId(removed.u, removed.v));
        }
    }
#ifdef TSP_VERIFY_INCREMENTAL_STATE
    std::vector<int> verified_degree = node.forced_degree;
    auto verified_root = [&](int vertex) {
        while (node.forced_parent[static_cast<std::size_t>(vertex)] != vertex) {
            vertex = node.forced_parent[static_cast<std::size_t>(vertex)];
        }
        return vertex;
    };
    for (std::size_t index = 0; index < candidates_sorted_.size(); ++index) {
        const Edge& edge = candidates_sorted_[index];
        const std::size_t eid = edgeId(edge.u, edge.v);
        if (!isCandidateActive(node, eid)) continue;
        bool invalid = node.forced[eid] || node.forbidden[eid]
            || node.forced_degree[static_cast<std::size_t>(edge.u)] >= 2
            || node.forced_degree[static_cast<std::size_t>(edge.v)] >= 2;
        if (!invalid && edge.u > 0 && edge.v > 0) {
            const int root_u = verified_root(edge.u);
            invalid = root_u == verified_root(edge.v)
                && node.forced_comp_size[static_cast<std::size_t>(root_u)] < n_;
        }
        if (invalid) {
            throw std::logic_error("local candidate filtering missed an invalid edge");
        }
        ++verified_degree[static_cast<std::size_t>(edge.u)];
        ++verified_degree[static_cast<std::size_t>(edge.v)];
    }
    int verified_insufficient = 0;
    for (int vertex = 0; vertex < n_; ++vertex) {
        if (verified_degree[static_cast<std::size_t>(vertex)]
            != available_degree_[static_cast<std::size_t>(vertex)]) {
            throw std::logic_error("incremental available degree is inconsistent");
        }
        if (verified_degree[static_cast<std::size_t>(vertex)] < 2) {
            ++verified_insufficient;
        }
    }
    if (verified_insufficient != insufficient_degree_count_) {
        throw std::logic_error("incremental insufficient degree count is inconsistent");
    }
#endif
    return insufficient_degree_count_ == 0;
}

void BranchBoundSolver::deactivateCandidate(
    PartialSol& node, std::size_t edge_id) const
{
    if (edge_id >= edge_rank_by_id_.size()) {
        throw std::logic_error("candidate edge id is out of range");
    }
    const int rank = edge_rank_by_id_[edge_id];
    if (rank < 0) {
        throw std::logic_error("active candidate has no stable rank");
    }
    deactivateCandidateByIndex(node, static_cast<std::size_t>(rank));
}

void BranchBoundSolver::deactivateCandidateByIndex(
    PartialSol& node, std::size_t index) const
{
    if (index >= candidates_sorted_.size()) {
        throw std::logic_error("candidate index is out of range");
    }
    const std::size_t word_index = index / 64;
    const std::uint64_t bit = std::uint64_t{1} << (index % 64);
    if (word_index >= node.candidate_bits.size()
        || (node.candidate_bits[word_index] & bit) == 0) {
        return;
    }
    deactivateCandidateBits(node, word_index, bit);
}

void BranchBoundSolver::deactivateCandidateBits(
    PartialSol& node, std::size_t word_index,
    std::uint64_t candidate_bits) const
{
    if (word_index >= node.candidate_bits.size()) {
        throw std::logic_error("candidate word index is out of range");
    }
    candidate_bits &= node.candidate_bits[word_index];
    node.candidate_bits[word_index] &= ~candidate_bits;
    while (candidate_bits != 0) {
        const unsigned bit_index = trailingZeroCount(candidate_bits);
        candidate_bits &= candidate_bits - 1;
        const std::size_t index = word_index * 64 + bit_index;
        if (index >= candidates_sorted_.size()) {
            throw std::logic_error("active candidate bit is out of range");
        }
        const Edge& edge = candidates_sorted_[index];
        if (!node.candidate_mask.empty()) {
            const std::size_t edge_id = edgeId(edge.u, edge.v);
            if (!node.candidate_mask[edge_id]) {
                throw std::logic_error("candidate active bit and mask are inconsistent");
            }
            node.candidate_mask[edge_id] = 0;
        }
        adjustAvailableDegree(edge.u, -1);
        adjustAvailableDegree(edge.v, -1);
        candidate_undo_.push_back(CandidateUndo{index});
    }
}

void BranchBoundSolver::deactivateIncidentCandidates(
    PartialSol& node, int vertex) const
{
    const std::size_t row_begin =
        static_cast<std::size_t>(vertex) * candidate_word_count_;
    int removed_count = 0;
    for (std::size_t word_index = 0;
         word_index < candidate_word_count_; ++word_index) {
        std::uint64_t removed_bits = node.candidate_bits[word_index]
            & candidate_incident_bits_[row_begin + word_index];
        node.candidate_bits[word_index] &= ~removed_bits;
        while (removed_bits != 0) {
            const unsigned bit_index = trailingZeroCount(removed_bits);
            removed_bits &= removed_bits - 1;
            const std::size_t index = word_index * 64 + bit_index;
            const Edge& edge = candidates_sorted_[index];
            if (!node.candidate_mask.empty()) {
                const std::size_t edge_id = edgeId(edge.u, edge.v);
                if (!node.candidate_mask[edge_id]) {
                    throw std::logic_error(
                        "candidate active bit and mask are inconsistent");
                }
                node.candidate_mask[edge_id] = 0;
            }
            const int other = edge.u == vertex ? edge.v : edge.u;
            adjustAvailableDegree(other, -1);
            candidate_undo_.push_back(CandidateUndo{index});
            ++removed_count;
        }
    }
    if (removed_count != 0) {
        adjustAvailableDegree(vertex, -removed_count);
    }
}

void BranchBoundSolver::rollbackCandidates(
    PartialSol& node, std::size_t checkpoint) const
{
    while (candidate_undo_.size() > checkpoint) {
        const CandidateUndo undo = candidate_undo_.back();
        candidate_undo_.pop_back();
        const Edge& edge = candidates_sorted_[undo.index];
        if (!node.candidate_mask.empty()) {
            node.candidate_mask[edgeId(edge.u, edge.v)] = 1;
        }
        node.candidate_bits[undo.index / 64] |=
            std::uint64_t{1} << (undo.index % 64);
        adjustAvailableDegree(edge.u, 1);
        adjustAvailableDegree(edge.v, 1);
    }
}

void BranchBoundSolver::adjustAvailableDegree(int vertex, int delta) const
{
    // available_degree_ 同时计入 forced 边和仍 active 的可选边；
    // insufficient_degree_count_ 缓存 degree<2 的顶点个数，避免全图重扫。
    int& degree = available_degree_[static_cast<std::size_t>(vertex)];
    const bool was_insufficient = degree < 2;
    degree += delta;
    const bool is_insufficient = degree < 2;
    if (was_insufficient != is_insufficient) {
        insufficient_degree_count_ += is_insufficient ? 1 : -1;
    }
}

bool BranchBoundSolver::isCandidateActive(
    const PartialSol& node, std::size_t edge_id) const
{
    // has_global_bits 表示 node 的位图与当前 epoch 的全局排序完全对齐；
    // 否则局部测试可退回 edgeId 兼容掩码。
    const bool has_global_bits =
        node.candidate_bit_count != 0
        && node.candidate_bit_count == candidates_sorted_.size()
        && node.candidate_bits.size() == candidate_word_count_;
    if (has_global_bits) {
        if (edge_id >= edge_rank_by_id_.size()) return false;
        const int rank = edge_rank_by_id_[edge_id];
        if (rank < 0) return false;
        const std::size_t index = static_cast<std::size_t>(rank);
        return (node.candidate_bits[index / 64]
                & (std::uint64_t{1} << (index % 64))) != 0;
    }
    if (!node.candidate_mask.empty()) {
        return edge_id < node.candidate_mask.size()
            && node.candidate_mask[edge_id] != 0;
    }
    return true;
}

void BranchBoundSolver::resetCandidateBits(
    PartialSol& node, std::size_t candidate_count) const
{
    node.candidate_bit_count = candidate_count;
    node.candidate_bits.assign((candidate_count + 63) / 64, ~std::uint64_t{0});
    // 最后一个 word 超出 candidate_count 的高位必须清零，否则枚举时会把
    // 不存在的候选下标误判为 active。
    const std::size_t extra_bits = node.candidate_bits.size() * 64 - candidate_count;
    if (!node.candidate_bits.empty() && extra_bits > 0) {
        node.candidate_bits.back() >>= extra_bits;
    }
}

std::size_t BranchBoundSolver::nextActiveCandidate(
    const PartialSol& node, std::size_t begin, std::size_t candidate_count) const
{
    if (begin >= candidate_count) return candidate_count;
    if (node.candidate_bit_count != candidate_count
        || node.candidate_bits.size() != (candidate_count + 63) / 64) {
        return begin;
    }

    // word_index/bit_offset 定位 begin；word 会先屏蔽 begin 之前的低位。
    std::size_t word_index = begin / 64;
    const std::size_t bit_offset = begin % 64;
    std::uint64_t word = node.candidate_bits[word_index]
        & (~std::uint64_t{0} << bit_offset);

    while (true) {
        if (word != 0) {
            const std::size_t result = word_index * 64
                + trailingZeroCount(word);
            return result < candidate_count ? result : candidate_count;
        }
        ++word_index;
        if (word_index >= node.candidate_bits.size()) return candidate_count;
        word = node.candidate_bits[word_index];
    }
}

void BranchBoundSolver::restoreForcedMstCache(
    PartialSol& node, double old_cost, int old_count) const
{
    node.forced_mst_cost = old_cost;
    node.forced_mst_count = old_count;
}


bool BranchBoundSolver::shouldPrune(double bound, double best_cost) const
{
    // 固定绝对 epsilon 会在 1e-12 等小尺度权重上吞掉整个有效差值。
    // 2^53 安全域内的整数 tour 可用保守 ceil 下界剪枝；普通 double
    // 下界略高时必须超过 n 项求和的尺度化舍入余量，避免误剪更优回路。
    if (!isFinite(best_cost)) {
        return false;
    }
    if (!isFinite(bound)) {
        return true;
    }
    if (exact_integer_costs_) {
        // 原问题的所有 tour 成本都是整数。固定 Held-Karp 势会让下界
        // 变为浮点数，但 ceil(lower_bound) 仍是有效整数下界；先减去
        // 求和舍入余量，避免数值误差跨过整数边界。
        const double tolerance = scaledRoundoffTolerance(
            bound, best_cost, static_cast<std::size_t>(n_));
        return std::ceil(bound - tolerance) >= best_cost;
    }
    // 普通 double 的不同加法顺序可能把略有差异的真实和舍入成相同值；
    // 因而非整数安全域内，等值和相差少量 ULP 都保守地继续搜索。
    if (bound <= best_cost) {
        return false;
    }
    return bound - best_cost
        > scaledRoundoffTolerance(bound, best_cost, static_cast<std::size_t>(n_));
}

BranchBoundSolver::RootReducedCostStats
BranchBoundSolver::applyRootReducedCostFixing(
    PartialSol& root, OneTree& root_tree) const
{
    RootReducedCostStats stats;
    if (!root_tree.feasible || !isFinite(best_cost_)
        || root_tree.edge_index_in_tree.size()
            != static_cast<std::size_t>(n_) * static_cast<std::size_t>(n_)) {
        return stats;
    }

    // 顶点 1..n-1 上的内部部分是一棵 MST。对每个起点做一次树 DFS，
    // 预计算路径最大边；根节点规模下这是 O(n^2)，之后每条非树边 O(1)。
    // negative_infinity 表示对应树路径或根支持边尚不存在。
    const double negative_infinity =
        -std::numeric_limits<double>::infinity();
    // adjacency 只包含内部 MST；heaviest_root_edge 是非树根边的交换支持边。
    std::vector<std::vector<std::pair<int, double>>> adjacency(
        static_cast<std::size_t>(n_));
    double heaviest_root_edge = negative_infinity;
    for (const Edge& edge : root_tree.edges) {
        if (edge.u == 0 || edge.v == 0) {
            heaviest_root_edge = std::max(heaviest_root_edge, edge.w);
            continue;
        }
        adjacency[static_cast<std::size_t>(edge.u)].push_back(
            {edge.v, edge.w});
        adjacency[static_cast<std::size_t>(edge.v)].push_back(
            {edge.u, edge.w});
    }

    // path_max[start*n+v] 保存 start 到 v 的内部树路径最大调整边权。
    const std::size_t state_size =
        static_cast<std::size_t>(n_) * static_cast<std::size_t>(n_);
    std::vector<double> path_max(state_size, negative_infinity);
    std::vector<int> parent(static_cast<std::size_t>(n_), -1);
    std::vector<int> stack;
    stack.reserve(static_cast<std::size_t>(n_));
    for (int start = 1; start < n_; ++start) {
        std::fill(parent.begin(), parent.end(), -1);
        stack.clear();
        parent[static_cast<std::size_t>(start)] = start;
        stack.push_back(start);
        while (!stack.empty()) {
            const int vertex = stack.back();
            stack.pop_back();
            const double prefix_max = path_max[
                static_cast<std::size_t>(start) * static_cast<std::size_t>(n_)
                + static_cast<std::size_t>(vertex)];
            for (const auto& [next, weight] :
                 adjacency[static_cast<std::size_t>(vertex)]) {
                if (parent[static_cast<std::size_t>(next)] >= 0) continue;
                parent[static_cast<std::size_t>(next)] = vertex;
                path_max[
                    static_cast<std::size_t>(start)
                        * static_cast<std::size_t>(n_)
                    + static_cast<std::size_t>(next)] =
                    std::max(prefix_max, weight);
                stack.push_back(next);
            }
        }
    }

    // fixed_indices 收集可以证明 x_e=0 的非树候选下标；先完成全部证明，
    // 再统一失活，避免遍历过程中修改正在扫描的 active bitset。
    std::vector<std::size_t> fixed_indices;
    fixed_indices.reserve(candidates_sorted_.size());
    for (std::size_t index = nextActiveCandidate(
             root, 0, candidates_sorted_.size());
         index < candidates_sorted_.size();
         index = nextActiveCandidate(
             root, index + 1, candidates_sorted_.size())) {
        const Edge& edge = candidates_sorted_[index];
        const std::size_t eid = edgeId(edge.u, edge.v);
        if (root_tree.edge_index_in_tree[eid] >= 0) continue;

        ++stats.tested;
        // support_weight 是强制加入 edge 时可删除的最重树边权；两者差值
        // 给出包含该边的 1-tree 相对根下界至少增加多少。
        const double support_weight = edge.u == 0 || edge.v == 0
            ? heaviest_root_edge
            : path_max[
                static_cast<std::size_t>(edge.u)
                    * static_cast<std::size_t>(n_)
                + static_cast<std::size_t>(edge.v)];
        if (!isFinite(support_weight)) continue;

        // MST 性质保证 delta >= 0。减去一次运算尺度误差，使构造出的
        // forced-edge 下界只可能偏低，绝不因相消误差错误固定边。
        const double delta_guard = scaledRoundoffTolerance(
            edge.w, support_weight, static_cast<std::size_t>(n_));
        const double safe_delta = std::max(
            0.0, edge.w - support_weight - delta_guard);
        if (shouldPrune(root_tree.cost + safe_delta, best_cost_)) {
            fixed_indices.push_back(index);
        }
    }

    for (const std::size_t index : fixed_indices) {
        deactivateCandidateByIndex(root, index);
    }
    stats.fixed_zero = fixed_indices.size();
    if (insufficient_degree_count_ != 0) {
        stats.proves_no_improvement = true;
        stats.active_after = candidates_sorted_.size() - candidate_undo_.size();
        return stats;
    }
    // 对根 1-tree 的每条边做一次单边 forbid sensitivity。若禁止该边后
    // 最小受约束 1-tree 已无法改善 incumbent，则所有改进 tour 都必须
    // 包含它，可安全固定为 x_e=1。
    // mandatory_edges 收集可以证明 x_e=1 的根树边，随后一次性写入 root。
    std::vector<Edge> mandatory_edges;
    mandatory_edges.reserve(root_tree.edges.size());
    for (const Edge& edge : root_tree.edges) {
        ++stats.tree_tested;
        const std::size_t eid = edgeId(edge.u, edge.v);
        // checkpoint 包围单条树边的临时 forbid sensitivity 测试。
        const std::size_t candidate_checkpoint = candidate_undo_.size();
        root.forbidden[eid] = 1;
        deactivateCandidate(root, eid);
        // forbid 后若某顶点可用度数不足 2，无需寻找替换边即可证明该边必选。
        const bool structurally_mandatory = insufficient_degree_count_ != 0;

        const Edge* replacement = nullptr;
        if (!structurally_mandatory) {
            if (edge.u == 0 || edge.v == 0) {
                for (const Edge& candidate : root_candidates_sorted_) {
                    const std::size_t candidate_id =
                        edgeId(candidate.u, candidate.v);
                    if (!isCandidateActive(root, candidate_id)
                        || root_tree.edge_index_in_tree[candidate_id] >= 0) {
                        continue;
                    }
                    replacement = &candidate;
                    break;
                }
            } else {
                replacement = findMstReplacement(
                    root, candidates_sorted_, root_tree, edge);
            }
        }
        const bool replacement_found = replacement != nullptr;
        // replacement_bound 是禁止当前树边后的保守下界；无 replacement 时
        // 保持 infinity，从而自然满足 mandatory 判定。
        double replacement_bound =
            std::numeric_limits<double>::infinity();
#ifdef TSP_VERIFY_INCREMENTAL_STATE
        double raw_replacement_bound = replacement_bound;
#endif
        if (replacement_found) {
#ifdef TSP_VERIFY_INCREMENTAL_STATE
            raw_replacement_bound =
                root_tree.cost + replacement->w - edge.w;
#endif
            const double delta_guard = scaledRoundoffTolerance(
                replacement->w, edge.w, static_cast<std::size_t>(n_));
            const double safe_delta = std::max(
                0.0, replacement->w - edge.w - delta_guard);
            replacement_bound = root_tree.cost + safe_delta;
        }

#ifdef TSP_VERIFY_INCREMENTAL_STATE
        if (!structurally_mandatory) {
            const OneTree rebuilt = computeOneTree(root, candidates_sorted_);
            if (replacement_found != rebuilt.feasible
                || (replacement_found
                    && !costsNumericallyEqual(
                        raw_replacement_bound, rebuilt.cost,
                        static_cast<std::size_t>(n_)))) {
                throw std::runtime_error(
                    "root tree-edge fixing replacement differs from rebuild");
            }
        }
#endif

        const bool mandatory = structurally_mandatory
            || !replacement_found
            || shouldPrune(replacement_bound, best_cost_);
        rollbackCandidates(root, candidate_checkpoint);
        root.forbidden[eid] = 0;
        if (mandatory) mandatory_edges.push_back(edge);
    }
    stats.fixed_one = mandatory_edges.size();

    auto forced_find = [&](int vertex) {
        while (root.forced_parent[static_cast<std::size_t>(vertex)] != vertex) {
            vertex = root.forced_parent[static_cast<std::size_t>(vertex)];
        }
        return vertex;
    };
    for (const Edge& edge : mandatory_edges) {
        if (root.forced_degree[static_cast<std::size_t>(edge.u)] >= 2
            || root.forced_degree[static_cast<std::size_t>(edge.v)] >= 2) {
            stats.proves_no_improvement = true;
            break;
        }

        int root_u = forced_find(edge.u);
        int root_v = forced_find(edge.v);
        if (root_u == root_v
            && root.forced_comp_size[static_cast<std::size_t>(root_u)] < n_) {
            stats.proves_no_improvement = true;
            break;
        }

        const std::size_t eid = edgeId(edge.u, edge.v);
        root.forced[eid] = 1;
        root.forced_edges.push_back(edge);
        ++root.forced_degree[static_cast<std::size_t>(edge.u)];
        ++root.forced_degree[static_cast<std::size_t>(edge.v)];
        // available_degree_ 统计 forced + active optional；随后移除候选中的
        // forced edge 会抵消这次 +1，使该强制关联仍计入可用度数。
        adjustAvailableDegree(edge.u, 1);
        adjustAvailableDegree(edge.v, 1);
        if (edge.u != 0 && edge.v != 0) {
            root.forced_mst_cost += edge.w;
            ++root.forced_mst_count;
        }

        if (root_u != root_v) {
            if (root.forced_rank[static_cast<std::size_t>(root_u)]
                < root.forced_rank[static_cast<std::size_t>(root_v)]) {
                std::swap(root_u, root_v);
            }
            if (!root.forced_member_bits.empty()) {
                root.forced_member_bits[static_cast<std::size_t>(root_u)] |=
                    root.forced_member_bits[static_cast<std::size_t>(root_v)];
            } else {
                auto& members_u =
                    root.forced_members[static_cast<std::size_t>(root_u)];
                const auto& members_v =
                    root.forced_members[static_cast<std::size_t>(root_v)];
                members_u.insert(
                    members_u.end(), members_v.begin(), members_v.end());
            }
            root.forced_parent[static_cast<std::size_t>(root_v)] = root_u;
            root.forced_comp_size[static_cast<std::size_t>(root_u)]
                += root.forced_comp_size[static_cast<std::size_t>(root_v)];
            if (root.forced_rank[static_cast<std::size_t>(root_u)]
                == root.forced_rank[static_cast<std::size_t>(root_v)]) {
                ++root.forced_rank[static_cast<std::size_t>(root_u)];
            }
        }
    }

    if (!stats.proves_no_improvement && !mandatory_edges.empty()) {
        // 根 fixing 不需要增量回滚。一次全量扫描删除 forced、度满关联边
        // 和会形成非完整子回路的边，然后完整重建 constrained 1-tree。
        std::vector<std::size_t> filtered_indices;
        filtered_indices.reserve(candidates_sorted_.size());
        for (std::size_t index = nextActiveCandidate(
                 root, 0, candidates_sorted_.size());
             index < candidates_sorted_.size();
             index = nextActiveCandidate(
                 root, index + 1, candidates_sorted_.size())) {
            const Edge& edge = candidates_sorted_[index];
            const std::size_t eid = edgeId(edge.u, edge.v);
            bool remove = root.forced[eid] != 0
                || root.forced_degree[static_cast<std::size_t>(edge.u)] >= 2
                || root.forced_degree[static_cast<std::size_t>(edge.v)] >= 2;
            if (!remove && edge.u != 0 && edge.v != 0) {
                const int component_u = forced_find(edge.u);
                const int component_v = forced_find(edge.v);
                remove = component_u == component_v
                    && root.forced_comp_size[
                        static_cast<std::size_t>(component_u)] < n_;
            }
            if (remove) filtered_indices.push_back(index);
        }
        for (const std::size_t index : filtered_indices) {
            deactivateCandidateByIndex(root, index);
        }

        if (insufficient_degree_count_ != 0) {
            stats.proves_no_improvement = true;
        } else {
            OneTree rebuilt = computeOneTree(root, candidates_sorted_);
            if (!rebuilt.feasible) {
                stats.proves_no_improvement = true;
            } else {
                root_tree = std::move(rebuilt);
            }
        }
    }

    stats.active_after = candidates_sorted_.size() - candidate_undo_.size();
    return stats;
}

#include "TspInitialTour.ipp"

void BranchBoundSolver::maybeImproveIncumbentDiversified()
{
    // 先让便宜的精确搜索处理小实例；只有已扩展足够多节点、可确认当前
    // 子树确实困难时，才支付多启动 LK 成本。
    constexpr std::size_t kSmallInstanceNodeBudget = 500000;
    constexpr std::size_t kLargeInstanceNodeBudget = 100000;
    constexpr int kLargeInstanceDimension = 64;
    // node_budget 是“当前精确搜索确实困难”的触发阈值；大实例更早启动
    // diversified LK，以便用更紧 UB 抵消后续庞大搜索树。
    const std::size_t node_budget = n_ >= kLargeInstanceDimension
        ? kLargeInstanceNodeBudget
        : kSmallInstanceNodeBudget;
    if (diversified_tour_attempted_
        || initial_tour_alternatives_.empty()
        || result_.stats.nodes_expanded < node_budget) {
        return;
    }

    diversified_tour_attempted_ = true;
    const double old_cost = best_cost_;
    std::vector<int> old_tour = best_tour_;
    if (improveInitialTourDiversified(
            initial_tour_alternatives_, result_.stats.root_lower_bound,
            best_tour_, best_cost_)) {
        const double recomputed_cost = tourCost(best_tour_);
        if (recomputed_cost < old_cost) {
            best_cost_ = recomputed_cost;
            writeDebugLine(
                debug_,
                "diversified incumbent: cost="
                    + formatDebugDouble(best_cost_)
                    + " previous=" + formatDebugDouble(old_cost)
                    + " expanded="
                    + std::to_string(result_.stats.nodes_expanded));
            restart_search_requested_ = true;
        } else {
            best_cost_ = old_cost;
            best_tour_ = std::move(old_tour);
        }
    }
    initial_tour_alternatives_.clear();
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

#include "TspLinKernighan.ipp"

#include "TspProblemIO.ipp"
#include "TspHeuristicSolver.ipp"

} // namespace tsp
