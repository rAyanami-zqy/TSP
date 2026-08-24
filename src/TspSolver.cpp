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

// 仅用于启发式改进的停滞阈值；精确搜索不能使用固定绝对容差剪枝。
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
    // 输入矩阵是算法约束本身，不是内部求和缓存；两方向必须是完全相同的
    // double。否则分支边和 tourCost 可能读取不同权重，破坏对称下界语义。
    return a == b;
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

void BranchBoundSolver::optimizeRootPotentials(double upper_bound)
{
    const auto started_at = std::chrono::steady_clock::now();
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
        bool feasible = false;
        double bound = -std::numeric_limits<double>::infinity();
        std::vector<int> degree;
    };

    std::vector<Edge> internal_edges;
    internal_edges.reserve(
        static_cast<std::size_t>(n_ - 1) * static_cast<std::size_t>(n_ - 2) / 2);
    auto evaluate = [&](const std::vector<double>& potentials) {
        OneTreeEvaluation evaluation;
        evaluation.degree.assign(static_cast<std::size_t>(n_), 0);
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

        DisjointSet components(n_);
        int mst_edge_count = 0;
        double modified_cost = 0.0;
        for (const Edge& edge : internal_edges) {
            if (!components.unite(edge.u, edge.v)) continue;
            modified_cost += edge.w;
            ++evaluation.degree[static_cast<std::size_t>(edge.u)];
            ++evaluation.degree[static_cast<std::size_t>(edge.v)];
            if (++mst_edge_count == n_ - 2) break;
        }
        if (mst_edge_count != n_ - 2) return evaluation;

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
        }

        const double correction = 2.0
            * std::accumulate(potentials.begin(), potentials.end(), 0.0);
        evaluation.feasible = true;
        evaluation.bound = modified_cost - correction;
        return evaluation;
    };

    struct AscentCandidate {
        std::vector<double> potentials;
        double bound = -std::numeric_limits<double>::infinity();
        int iterations = 0;
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
        std::vector<double> potentials = initial;
        AscentCandidate best;
        best.potentials = initial;
        double step_scale = 2.0;
        int no_improvement = 0;
        constexpr int kStagnationIterations = 12;
        constexpr int kMinIterationsBeforeGapStop = 100;

        for (int iteration = 0; iteration < kMaxIterations; ++iteration) {
            const OneTreeEvaluation evaluation = evaluate(potentials);
            ++best.iterations;
            if (!evaluation.feasible) break;
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
        std::vector<double> previous_subgradient(
            static_cast<std::size_t>(n_), 0.0);
        bool have_previous = false;
        double previous_bound = -std::numeric_limits<double>::infinity();
        double step = 1.0;
        int period = std::max(1, n_ / 2);
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
        bool feasible = false;
        double bound = -std::numeric_limits<double>::infinity();
        std::vector<int> degree;
    };

    // 节点约束在一次势上升期间保持不变。预先收缩所有非根 forced 边，
    // 后续每轮只需在当前有效边构成的分量图上运行 Prim；这避免了为每组势
    // 重新收集 O(n^2) 条 Edge 并执行 O(n^2 log n) 的 Kruskal 排序。
    DisjointSet forced_components(n_);
    bool forced_structure_feasible = true;
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
        int to = -1;
        int next = -1;
        double original_weight = 0.0;
    };
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
    std::vector<double> best_component_weight(
        static_cast<std::size_t>(component_count), infinity);
    std::vector<int> best_component_u(
        static_cast<std::size_t>(component_count), -1);
    std::vector<int> best_component_v(
        static_cast<std::size_t>(component_count), -1);
    std::vector<unsigned char> component_in_tree(
        static_cast<std::size_t>(component_count), 0);
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
        const double guard = scaledRoundoffTolerance(
            static_cast<double>(n_) * largest_adjusted_weight,
            correction, static_cast<std::size_t>(n_));
        evaluation.feasible = true;
        evaluation.bound = modified_cost - correction - guard;
        return evaluation;
    };

    NodePotentialUpdateResult result;
    result.feasible = true;
    result.bound = current_bound;
    std::vector<double> potentials = vertex_potential_;
    if (potentials.size() != static_cast<std::size_t>(n_)) {
        potentials.assign(static_cast<std::size_t>(n_), 0.0);
    }
    result.potentials = potentials;

    double step_scale = 2.0;
    std::size_t no_improvement = 0;
    for (std::size_t iteration = 0; iteration < max_iterations; ++iteration) {
        const Evaluation evaluation = evaluate(potentials);
        ++result.iterations;
        if (!evaluation.feasible) {
            result.feasible = false;
            break;
        }

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
        if (subgradient_norm == 0.0
            || evaluation.bound >= upper_bound - tolerance) {
            break;
        }

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
    const auto update_started_at = std::chrono::steady_clock::now();
    ++result_.stats.potential_updates_attempted;
    ++potential_updates_in_round_;
    const NodePotentialUpdateResult update = updateNodePotentialBound(
        node, current_tree.cost, best_cost_, potential_update_iterations_);
    result_.stats.potential_update_iterations += update.iterations;
    result_.stats.potential_update_seconds += std::chrono::duration<double>(
        std::chrono::steady_clock::now() - update_started_at).count();

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

    const std::size_t edge_state_size = static_cast<std::size_t>(n_)
        * static_cast<std::size_t>(n_);
    std::vector<unsigned char> active_by_edge_id(edge_state_size, 0);
    for (const Edge& edge : candidates_sorted_) {
        const std::size_t id = edgeId(edge.u, edge.v);
        if (isCandidateActive(node, id)) active_by_edge_id[id] = 1;
    }

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
        potential_updates_in_round_ = 0;
        current_potential_epoch_depth_ = 0;
        optimizeRootPotentials(best_cost_);
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
            const std::size_t eid = edgeId(e.u, e.v);
            edge_rank_by_id_[eid] = static_cast<int>(index);
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
        const bool root_pruned = shouldPrune(root.bound, best_cost_);
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

BranchBoundSolver::BranchSet BranchBoundSolver::bpPartition(
    PartialSol& node, OneTree& work_tree)
{
    BranchSet B_set;

    const std::size_t tree_checkpoint = tree_undo_.size();
    const std::size_t candidate_checkpoint = candidate_undo_.size();

    while (true) {
        const Edge* deg_best = nullptr;

#ifdef TSP_BRANCH_STRATEGY_MIN_EDGE
        constexpr bool kUseMinEdgeFromOneTree = true;
#else
        constexpr bool kUseMinEdgeFromOneTree = false;
#endif

        if (kUseMinEdgeFromOneTree) {
            // ── 新选边策略：从 1-tree 所有未决边中直接选最小权重边 ──
            for (const Edge& e : work_tree.edges) {
                const std::size_t eid = edgeId(e.u, e.v);
                if (node.forced[eid]) continue;
                if (node.forbidden[eid]) continue;
                if (!deg_best || e.w < deg_best->w) deg_best = &e;
            }
        } else {
            // ── 原选边策略：从度数违规顶点取最小权重未决边 ──
            // 若没有度数 > 2 的顶点，则当前可行 1-tree 已由 isTour() 处理。
            int branch_vertex = -1;
            int max_deg = 2;
            for (int v = 0; v < n_; ++v) {
                if (work_tree.degree[static_cast<std::size_t>(v)] > max_deg) {
                    max_deg = work_tree.degree[static_cast<std::size_t>(v)];
                    branch_vertex = v;
                }
            }
            if (branch_vertex >= 0) {
                for (const Edge& e : work_tree.edges) {
                    if (e.u != branch_vertex && e.v != branch_vertex) continue;
                    const std::size_t eid = edgeId(e.u, e.v);
                    if (node.forced[eid]) continue;
                    if (node.forbidden[eid]) continue;
                    if (!deg_best || e.w < deg_best->w) deg_best = &e;
                }
            }
        }

        if (!deg_best) break;

        // 选中的边先进入 B；若 forbid 后仍可行且未被 bound 剪枝，则继续划分。
        auto test = [&](Edge e) -> bool {
            const std::size_t eid = edgeId(e.u, e.v);

            // 记录 forbid 当前边在 work_tree 中要替换的位置；search 会在
            // 兄弟分支间顺序重放该 delta，而不是复制 |B| 棵完整 1-tree。
            // O(1) 查表替代 find_if 扫描。
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
            BranchChoice choice;
            choice.edge = e;
            choice.tree_edge_index = tree_edge_idx;
            B_set.push_back(choice);

            node.forbidden[eid] = 1;
            deactivateCandidate(node, eid);

            bool replacement_found = false;
#ifndef TSP_DISABLE_INCREMENTAL_ONETREE
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
        const NodePotentialUpdateResult update = updateNodePotentialBound(
            node, node.bound, best_cost_, potential_update_iterations_);
        result_.stats.potential_update_iterations += update.iterations;
        result_.stats.potential_update_seconds += std::chrono::duration<double>(
            std::chrono::steady_clock::now() - update_started_at).count();

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
    struct ForceChanges {
        std::size_t forced_id = static_cast<std::size_t>(-1);
        int fu = -1, fv = -1;
        int root_u = -1, root_v = -1;
        int old_rank_u = -1, old_rank_v = -1;
        int old_size_u = -1, old_size_v = -1;
        int merge_root = -1;
        std::size_t old_merge_member_count = 0;
        int merged_scan_root = -1;
        std::size_t merged_scan_count = 0;
        std::uint64_t merged_scan_bits = 0;
        std::uint64_t old_merge_member_bits = 0;
        double old_forced_mst_cost = 0.0;
        int old_forced_mst_count = 0;
    };

    auto forced_find = [&](int x) -> int {
        while (node.forced_parent[static_cast<std::size_t>(x)] != x) {
            x = node.forced_parent[static_cast<std::size_t>(x)];
        }
        return x;
    };

    auto apply_force = [&](const Edge& e, ForceChanges& changes) -> bool {
        const std::size_t id = edgeId(e.u, e.v);
        if (node.forced[id] || node.forbidden[id]
            || !isFinite(dist_[e.u][e.v])) {
            return false;
        }
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
    OneTree result;
    result.cost = std::numeric_limits<double>::infinity();
    result.degree.assign(static_cast<std::size_t>(n_), 0);

    // 顶点 0 不参与 MST；从独立集合开始显式合并所有非根 forced 边。
    // 不能直接复用包含根边的 forced DSU，否则拆开 0 所在分量时会丢失其中的
    // 非根 forced 连通关系。
    DisjointSet tree_components(n_);

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
    const std::size_t forbidden_id = edgeId(forbidden_edge.u, forbidden_edge.v);
    if (!tree.feasible || !node.forbidden[forbidden_id] || node.forced[forbidden_id]) {
        return false;
    }

    // O(1) 查表替代 find_if 扫描。
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
    removed_candidate_scratch_.clear();
    removed_candidate_scratch_.reserve(tree.edges.size());
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

    const auto first = std::lower_bound(
        branch_candidates.begin(), branch_candidates.end(), removed_edge.w,
        [](const Edge& edge, double weight) { return edge.w < weight; });
    const std::size_t first_index = static_cast<std::size_t>(
        std::distance(branch_candidates.begin(), first));

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
    const double negative_infinity =
        -std::numeric_limits<double>::infinity();
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
    std::vector<Edge> mandatory_edges;
    mandatory_edges.reserve(root_tree.edges.size());
    for (const Edge& edge : root_tree.edges) {
        ++stats.tree_tested;
        const std::size_t eid = edgeId(edge.u, edge.v);
        const std::size_t candidate_checkpoint = candidate_undo_.size();
        root.forbidden[eid] = 1;
        deactivateCandidate(root, eid);
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

bool BranchBoundSolver::findInitialTour(
    std::vector<int>& tour, double& cost,
    std::vector<TourCandidate>& alternatives)
{
    alternatives.clear();
    std::map<std::vector<int>, TourCandidate> distinct_tours;
    double best_cost = std::numeric_limits<double>::infinity();
    std::vector<int> best_tour;

    auto canonicalize = [&](const std::vector<int>& candidate) {
        const auto zero = std::find(candidate.begin(), candidate.end(), 0);
        if (zero == candidate.end()) return std::vector<int>{};
        const int zero_index = static_cast<int>(
            std::distance(candidate.begin(), zero));
        std::vector<int> forward(static_cast<std::size_t>(n_));
        std::vector<int> reverse(static_cast<std::size_t>(n_));
        for (int offset = 0; offset < n_; ++offset) {
            forward[static_cast<std::size_t>(offset)] = candidate[
                static_cast<std::size_t>((zero_index + offset) % n_)];
            reverse[static_cast<std::size_t>(offset)] = candidate[
                static_cast<std::size_t>((zero_index - offset + n_) % n_)];
        }
        return reverse < forward ? reverse : forward;
    };

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

        // 保持原实现的严格小于与首次命中语义，确保便宜的首个 LK 起点
        // 不因候选池的规范化或排序而改变。
        if (candidate_cost < best_cost) {
            best_cost = candidate_cost;
            best_tour = candidate;
        }

        std::vector<int> canonical = canonicalize(candidate);
        if (!canonical.empty()) {
            distinct_tours.try_emplace(
                std::move(canonical),
                TourCandidate{candidate_cost, std::move(candidate)});
        }
    }

    if (best_tour.empty()) {
        return false;
    }

    const std::size_t distinct_tour_count = distinct_tours.size();
    distinct_tours.erase(canonicalize(best_tour));

    alternatives.reserve(distinct_tours.size());
    for (auto& [canonical, candidate] : distinct_tours) {
        (void)canonical;
        alternatives.push_back(std::move(candidate));
    }
    std::sort(
        alternatives.begin(), alternatives.end(),
        [](const TourCandidate& left, const TourCandidate& right) {
            if (left.cost != right.cost) return left.cost < right.cost;
            return left.tour < right.tour;
        });

    // 自适应阶段只需保留最有希望的一小组不同局部最优，避免大实例把
    // 所有 n 个 tour 带入多启动 LK。
    constexpr std::size_t kInitialCandidatePoolSize = 24;
    if (alternatives.size() > kInitialCandidatePoolSize) {
        alternatives.resize(kInitialCandidatePoolSize);
    }

    if (debug_.output != nullptr) {
        std::ostringstream line;
        line << "initial tour pool: distinct=" << distinct_tour_count
             << " retained=" << alternatives.size()
             << " best_2opt=" << formatDebugDouble(best_cost);
        if (!alternatives.empty()) {
            line << " next_2opt="
                 << formatDebugDouble(alternatives.front().cost);
        }
        writeDebugLine(debug_, line.str());
    }

    // 用 Lin-Kernighan 对最佳 NN+2opt 结果做精细优化，获取更紧上界。
    linKernighan(best_tour, best_cost);

    tour = std::move(best_tour);
    cost = best_cost;
    return true;
}

bool BranchBoundSolver::improveInitialTourDiversified(
    std::vector<TourCandidate>& alternatives,
    double root_lower_bound,
    std::vector<int>& tour, double& cost)
{
    constexpr std::size_t kDiversifiedLkStarts = 12;
    const std::size_t start_count = std::min(
        alternatives.size(), kDiversifiedLkStarts);
    bool improved = false;
    std::size_t starts_run = 0;

    for (std::size_t index = 0; index < start_count; ++index) {
        std::vector<int> candidate = alternatives[index].tour;
        double candidate_cost = alternatives[index].cost;
        linKernighan(candidate, candidate_cost);
        candidate_cost = tourCost(candidate);
        ++starts_run;

        if (candidate_cost < cost) {
            tour = std::move(candidate);
            cost = candidate_cost;
            improved = true;
        }
        if (isFinite(root_lower_bound)
            && shouldPrune(root_lower_bound, cost)) {
            break;
        }
    }

    writeDebugLine(
        debug_,
        "diversified LK completed: starts=" + std::to_string(starts_run)
            + " best=" + formatDebugDouble(cost));
    return improved;
}

void BranchBoundSolver::maybeImproveIncumbentDiversified()
{
    // 先让便宜的精确搜索处理小实例；只有已扩展足够多节点、可确认当前
    // 子树确实困难时，才支付多启动 LK 成本。
    constexpr std::size_t kSmallInstanceNodeBudget = 500000;
    constexpr std::size_t kLargeInstanceNodeBudget = 100000;
    constexpr int kLargeInstanceDimension = 64;
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
                if (delta < -kHeuristicEps) {
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
        const auto neighbor_less = [](const auto& left, const auto& right) {
            if (left.first != right.first) return left.first < right.first;
            return left.second < right.second;
        };
        if (keep < static_cast<int>(nb.size())) {
            std::nth_element(
                nb.begin(), nb.begin() + keep, nb.end(), neighbor_less);
        }
        nb.resize(static_cast<std::size_t>(keep));
        std::sort(nb.begin(), nb.end(), neighbor_less);
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

    double best_gain = kHeuristicEps;
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
            if (dist_[from][x] >= cum_gain - kHeuristicEps) {
                continue;
            }

            // 按照 LK 原论文：t4 固定为 next[t3]（一致的前进方向）。
            int y = next[static_cast<std::size_t>(x)];
            if (y == from || used[static_cast<std::size_t>(y)]) {
                continue;
            }

            double new_gain = cum_gain - dist_[from][x] + dist_[x][y];
            if (new_gain <= kHeuristicEps) {
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
            if (cost + kHeuristicEps < best_cost) {
                best_tour = tour;
                best_cost = cost;
            }
        }

        // 更新当前最优。
        if (cost + kHeuristicEps < best_cost) {
            best_tour = tour;
            best_cost = cost;
        }

        if (kick == kLkMaxKicks - 1) {
            break;
        }

        // Double-bridge kick 跳出局部最优。
        doubleBridgeKick(tour);
        cost = tourCost(tour);

        if (cost + kHeuristicEps < best_cost) {
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
    const double range_x = std::max(kHeuristicEps, max_x - min_x);
    const double range_y = std::max(kHeuristicEps, max_y - min_y);

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
                if (delta < -kHeuristicEps) {
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
                if (delta < -kHeuristicEps) {
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
