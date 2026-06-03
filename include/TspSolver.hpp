#pragma once

#include <cstddef>
#include <iosfwd>
#include <string>
#include <vector>

namespace tsp {

// 搜索过程统计信息，便于观察分支定界剪枝效果。
struct SolveStats {
    std::size_t nodes_created = 0;
    std::size_t nodes_expanded = 0;
    std::size_t nodes_pruned_by_bound = 0;
    std::size_t nodes_pruned_infeasible = 0;
    double root_lower_bound = 0.0;
    double initial_upper_bound = 0.0;
};

// 求解结果。tour 中只保存一圈中的顶点序列，输出时再补回起点。
struct SolveResult {
    bool feasible = false;
    double cost = 0.0;
    std::vector<int> tour;
    SolveStats stats;
};

// 可选 debug 输出配置。output 为 nullptr 时不输出；interval 控制周期性进度行频率。
struct DebugOptions {
    std::ostream* output = nullptr;
    std::size_t interval = 1000;
};

struct Point {
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
};

// 支持本项目原始矩阵格式和 TSPLIB TSP 格式的统一问题表示。
struct TspProblem {
    std::string name;
    std::string type;
    std::string edge_weight_type;
    std::string edge_weight_format;
    std::vector<Point> coordinates;
    std::vector<std::vector<double>> matrix;

    // dimension() 返回问题中的顶点数。对于坐标型问题，返回 coordinates 的大小；对于矩阵型问题，返回 matrix 的行数。
    int dimension() const;
    bool hasCoordinates() const;
    bool hasDenseMatrix() const;
    double distance(int u, int v) const;
    // 如果问题已经包含距离矩阵则直接返回；否则根据坐标计算距离矩阵。max_dimension > 0 时限制问题规模，避免过大实例导致内存爆炸。
    std::vector<std::vector<double>> toDenseMatrix(std::size_t max_dimension = 0) const;
};

struct HeuristicOptions {
    std::size_t starts = 4;
    std::size_t nearest_scan_limit = 5000;
    std::size_t full_two_opt_limit = 2000;
    std::size_t two_opt_window = 64;
    std::size_t two_opt_passes = 2;
    unsigned seed = 1;
    DebugOptions debug;
};

// 基本分支定界 TSP 求解器。
// 当前实现面向对称 TSP：dist[i][j] 必须等于 dist[j][i]。
// 分支策略：
// Smart      — 优先对 1-tree 中度数最大的顶点选边分支（单边，二分叉）。
// Exhaustive — 对所有未决边同时展开 force/forbid 分支。
// Simple     — 任选一条未决边做 force/forbid 二分叉，无启发式选择。
enum class BranchStrategy {
    Smart,
    Exhaustive,
    Simple
};

class BranchBoundSolver {
public:
    explicit BranchBoundSolver(std::vector<std::vector<double>> distance);
    // 开启 / 关闭精确求解过程中的 stderr 等调试输出。
    void setDebugOutput(std::ostream& output, std::size_t progress_interval = 1000);
    void disableDebugOutput();
    // 求解 TSP，返回求解结果和搜索统计。
    SolveResult solve(BranchStrategy strategy = BranchStrategy::Smart);

private:
    struct Edge {
        int u = 0;
        int v = 0;
        double w = 0.0;
    };

    // 1-tree 下界结果：如果每个点度数都为 2，则该 1-tree 本身就是一条 Hamilton 回路。
    struct OneTree {
        bool feasible = false;
        double cost = 0.0;
        // 1-tree 的边集合和度数统计，避免在分支节点中重复计算。
        std::vector<Edge> edges;
        std::vector<int> degree;
    };

    // 分支定界节点：
    // forced 表示必须选择的边，forbidden 表示禁止选择的边。
    struct Node {
        int depth = 0;
        double bound = 0.0;
        // forced 和 forbidden 数组大小为 n*n，edgeId(u,v) 映射无向边 (u,v) 到其中一个位置。
        std::vector<unsigned char> forced;
        std::vector<unsigned char> forbidden;
    };
    // edgeId 把无向边 (u,v) 映射到 forced / forbidden 数组中的一个位置，方便 O(1) 标记边状态。
    std::size_t edgeId(int u, int v) const;
    // forced 数组中非 0 表示该边在当前分支节点中必须被选择。
    bool isForced(const std::vector<unsigned char>& forced, int u, int v) const;
    // forbidden 数组中非 0 表示该边在当前分支节点中禁止被选择。
    bool isForbidden(const std::vector<unsigned char>& forbidden, int u, int v) const;

    // 在 forced / forbidden 约束下构造最小 1-tree，作为该节点的下界。
    OneTree computeOneTree(const Node& node_,std::vector<Edge>& branch_candidates)const;
    // 判断一个 1-tree 是否已经是一条合法的 Hamilton 回路。
    bool isTour(const OneTree& one_tree) const;
    // 从 1-tree 的边集合构造访问顺序的顶点序列；如果无法构成合法回路则返回空。
    std::vector<int> buildTour(const std::vector<Edge>& edges) const;
    // 收集当前节点尚未决定且实际存在的边，作为本节点分支候选集。
    bool buildBranchCandidates(const Node& node,std::vector<Edge>& branch_candidates)const;
    // 从当前候选集中选择一条未决边做二分支：包含该边 / 禁止该边；选择时优先当前 1-tree。
    bool chooseBranchEdge(const Node& node,
                          const OneTree& one_tree,
                          const std::vector<Edge>& candidates,
                          Edge& edge) const;
    // 判断当前节点的下界是否已经不优于已知最优可行解，可以直接剪枝。
    bool shouldPrune(double bound, double best_cost) const;
    // 最近邻 + 2-opt，生成一个可行上界，帮助早剪枝。
    bool findInitialTour(std::vector<int>& tour, double& cost) const;
    // 计算一个 tour 的总成本。
    double tourCost(const std::vector<int>& tour) const;
    // 2-opt 局部优化：如果交换 tour 中的两条边能降低成本，就执行交换。
    void twoOpt(std::vector<int>& tour, double& cost) const;

    // 顶点数。
    int n_ = 0;
    // 距离矩阵，dist[i][j] 是顶点 i 和 j 之间的距离；dist[i][i] 必须为 0。
    std::vector<std::vector<double>> dist_;
    DebugOptions debug_;
};

// 自动识别本项目矩阵格式或 TSPLIB TSP 格式。
TspProblem readTspProblem(std::istream& input);
// 输入格式：第一项是 n，后面是 n*n 的距离矩阵。
std::vector<std::vector<double>> readDistanceMatrix(std::istream& input);
// 大图启发式求解：用于经典坐标型 benchmark 的快速可行 tour。
SolveResult solveHeuristic(const TspProblem& problem, const HeuristicOptions& options = {});
// 输出格式：0 -> 1 -> 2 -> 0，末尾补回起点表示闭合回路。
std::string formatTour(const std::vector<int>& tour);

} // namespace tsp
