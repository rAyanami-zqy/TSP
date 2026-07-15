#pragma once

#include <cstddef>
#include <iosfwd>
#include <limits>
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

struct BranchBoundSolverTestAccess;

// 分支定界 TSP 求解器。当前默认使用 BP (Branch Partitioning)；
// TSP_DISABLE_BP 变体使用历史 smart 选边的 force/forbid 二分搜索。
// 实现面向对称 TSP：dist[i][j] 必须等于 dist[j][i]。

class BranchBoundSolver {
public:
    explicit BranchBoundSolver(std::vector<std::vector<double>> distance);
    // 开启 / 关闭精确求解过程中的 stderr 等调试输出。
    void setDebugOutput(std::ostream& output, std::size_t progress_interval = 1000);
    void disableDebugOutput();
    // 求解 TSP，返回求解结果和搜索统计。
    SolveResult solve();

private:
    friend struct BranchBoundSolverTestAccess;

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

    // BP 链中的一项。记录 forbid edge 对前缀树的一步变换；search 只维护
    // 一棵 prefix_tree 并顺序重放，避免为 B 中每项复制整棵树。
    struct BranchChoice {
        Edge edge;
        bool has_forbid_replacement = false;
        bool replay_requires_rebuild = false;
        std::size_t tree_edge_index = 0;
        Edge forbid_replacement;
    };

    // 分支定界节点（部分解 P）：
    // forced 表示必须选择的边，forbidden 表示禁止选择的边。
    // 同时维护 forced 边的并查集和度数统计，加速 1-tree 计算与候选集筛选。
    struct PartialSol {
        int depth = 0;
        double bound = 0.0;
        // forced 和 forbidden 数组大小为 n*n，edgeId(u,v) 映射无向边 (u,v) 到其中一个位置。
        std::vector<unsigned char> forced;
        std::vector<unsigned char> forbidden;
        // forced 边构成的连通分量并查集（无路径压缩，保证回溯可逆）。
        std::vector<int> forced_parent;   // parent[i] = parent of vertex i
        std::vector<int> forced_rank;     // rank[i] = rank of component rooted at i
        std::vector<int> forced_comp_size; // comp_size[i] = size of component rooted at i
        // 各顶点在 forced 边子图中的度数。
        std::vector<int> forced_degree;
        // forced 边列表与 MST 部分（不含顶点 0 的边）的权值和及计数。
        std::vector<Edge> forced_edges;
        double forced_mst_cost = 0.0;
        int forced_mst_count = 0;
        // 候选集快速查找表（大小 n*n），由 buildBranchCandidates 维护，
        // chooseBranchEdge 直接读取，避免重复分配 O(n²) 的掩码数组。
        std::vector<unsigned char> candidate_mask;
    };

    // tryChild 过滤后的候选集保存在这里，visitChild 直接取用，避免重复过滤。
    struct PendingChild {
        bool available = false;
        std::vector<Edge> filtered_candidates;
    };
    // edgeId 把无向边 (u,v) 映射到 forced / forbidden 数组中的一个位置，方便 O(1) 标记边状态。
    std::size_t edgeId(int u, int v) const;
    // forced 数组中非 0 表示该边在当前分支节点中必须被选择。
    bool isForced(const std::vector<unsigned char>& forced, int u, int v) const;
    // forbidden 数组中非 0 表示该边在当前分支节点中禁止被选择。
    bool isForbidden(const std::vector<unsigned char>& forbidden, int u, int v) const;

    // 在 forced / forbidden 约束下构造最小 1-tree，作为该节点的下界。
    OneTree computeOneTree(const PartialSol& node,
                           const std::vector<Edge>& branch_candidates) const;
    // 禁用当前 1-tree 中的一条未强制边后，使用 MST replacement edge 增量更新。
    // node.candidate_mask 必须与 branch_candidates 保持同步，根边可用性由该 mask 判定。
    bool updateOneTreeAfterForbid(const PartialSol& node,
                                  const std::vector<Edge>& branch_candidates,
                                  OneTree& tree,
                                  const Edge& forbidden_edge) const;
    // force 分支后，buildBranchCandidates 移除的候选边中若包含当前 1-tree 的边，
    // 则为每条被移除的树边寻找替代边，增量更新 1-tree，避免从零重算 MST。
    // 返回 false 表示存在无法替代的树边，调用者应回退到完整 computeOneTree。
    bool updateOneTreeAfterCandidateRemoval(
        const PartialSol& node,
        const std::vector<Edge>& branch_candidates,
        OneTree& tree,
        const std::vector<std::size_t>& removed_edge_ids) const;
    // 验证增量 1-tree 的生产不变量：完整结构、缓存度数/成本，以及所有
    // forced / forbidden / candidate 约束。失败时调用者必须完整重建。
    bool oneTreeSatisfiesConstraints(const PartialSol& node,
                                     const OneTree& tree) const;
    // 判断一个 1-tree 是否已经是一条合法的 Hamilton 回路。
    bool isTour(const OneTree& one_tree) const;
    // 从 1-tree 的边集合构造访问顺序的顶点序列；如果无法构成合法回路则返回空。
    std::vector<int> buildTour(const std::vector<Edge>& edges) const;
    // 收集当前节点尚未决定且实际存在的边，作为本节点分支候选集。
    bool buildBranchCandidates(const PartialSol& node,
                               std::vector<Edge>& branch_candidates,
                               std::vector<std::size_t>* removed_edge_ids = nullptr) const;
    // 历史 smart 选边：先在 1-tree 中选最高违规度顶点的最轻未决边，
    // 再回退到全部 1-tree 未决边和全候选集的最轻边。
    bool chooseSmartBranchEdge(const PartialSol& node,
                               const OneTree& one_tree,
                               const std::vector<Edge>& candidates,
                               Edge& edge) const;
    // 精确恢复 DFS force 前的 MST 缓存；不能用 +w/-w 逆运算，因为不同
    // 动态范围的浮点数相加会丢失旧值。
    void restoreForcedMstCache(PartialSol& node, double old_cost,
                               int old_count) const;
    // 判断当前节点的下界是否已经不优于已知最优可行解，可以直接剪枝。
    bool shouldPrune(double bound, double best_cost) const;
    // 最近邻 + 2-opt + LK，生成一个可行上界，帮助早剪枝。
    bool findInitialTour(std::vector<int>& tour, double& cost);
    // 计算一个 tour 的总成本。
    double tourCost(const std::vector<int>& tour) const;
    // 2-opt 局部优化：如果交换 tour 中的两条边能降低成本，就执行交换。
    void twoOpt(std::vector<int>& tour, double& cost) const;
    // 构建位置映射：pos[v] = v 在 tour 中的下标。
    std::vector<int> buildPositionMap(const std::vector<int>& tour) const;
    // 预计算每个顶点的 K 近邻候选集，供 LK 搜索使用。
    void buildCandidateSets() const;
    // LK 核心：从边 (t1,t2) 出发进行顺序 k-opt 搜索，返回是否找到改进。
    // next/prev 是 tour 的双向链表表示，active 是 don't-look 标记。
    bool lkSearch(int t1, int t2, std::vector<int>& next, std::vector<int>& prev,
                  std::vector<bool>& active) const;
    // LK 改进主循环：遍历所有 tour 边作为起点，连续应用 k-opt 直到无改进。
    bool linKernighanImprove(std::vector<int>& tour, double& cost) const;
    // Double-bridge kick：非顺序 4-opt 扰动，帮助 LK 跳出局部最优。
    void doubleBridgeKick(std::vector<int>& tour) const;
    // Chained LK 总入口：多次 LK + double-bridge kick 迭代，取最优结果。
    void linKernighan(std::vector<int>& tour, double& cost) const;

    // ── BP (Branch Partitioning) 搜索 ──
    // 在当前 1-tree 上执行 BP 划分：依次测试前缀禁止约束并返回关键边集合 B。
    std::vector<BranchChoice> bpPartition(PartialSol& node,
                                          const std::vector<Edge>& branch_candidates,
                                          const OneTree& current_tree);
    // BP 递归搜索：在每个节点执行 BP 划分后枚举“前缀 forbid + 当前 force”子节点。
    void search(PartialSol& node,
                std::vector<Edge>& branch_candidates,
                int depth,
                const OneTree* precomputed_tree = nullptr);
    // 不使用 BP 划分的 smart 二分搜索：每层只生成 force / forbid
    // 两个互斥子节点，其他下界、回滚和 1-tree 安全校验与当前实现共享。
    void searchSmart(PartialSol& node,
                     std::vector<Edge>& branch_candidates,
                     int depth,
                     const OneTree* precomputed_tree = nullptr);

    // 顶点数。
    int n_ = 0;
    // 距离矩阵，dist[i][j] 是顶点 i 和 j 之间的距离；dist[i][i] 必须为 0。
    std::vector<std::vector<double>> dist_;
    // 所有有限边均为精确整数且任意 n 边和不超过 2^53 时，成本求和本身
    // 精确，可安全执行 bound >= incumbent 的等值剪枝。
    bool exact_integer_costs_ = false;
    // 与顶点 0 相连的所有有限边，按权重升序排列。
    // 在 computeOneTree 中取前 2 条未被禁止的边作为 1-tree 的 root edges。
    std::vector<Edge> root_candidates_sorted_;
    DebugOptions debug_;

    // LK 候选集：每个顶点的 K 近邻，惰性初始化。
    static constexpr int kLkCandidateSize = 8;
    static constexpr int kLkMaxDepth = 5;
    static constexpr int kLkMaxKicks = 10;
    mutable std::vector<std::vector<int>> candidate_set_;
    mutable bool candidate_set_built_ = false;

    // 搜索过程中的状态。
    double best_cost_ = std::numeric_limits<double>::infinity();
    std::vector<int> best_tour_;
    SolveResult result_;
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
