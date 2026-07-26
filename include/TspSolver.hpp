#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
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

// 分支定界 TSP 求解器，使用 BP (Branch Partitioning) 策略。
// 当前实现面向对称 TSP：dist[i][j] 必须等于 dist[j][i]。
// BP 策略：在 1-tree 上执行候选边划分（A 集/B 集），直接枚举 B 集的 force 子节点。

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
        // 顶点 1..n-1 上当前 MST 的动态邻接位图（按行存储）。
        // 根边不进入该状态；子节点复制 OneTree 时一并继承。
        std::vector<std::uint64_t> mst_adjacency_bits;
        // 边在 edges 中的位置，-1 表示不在此 1-tree 中。
        // 大小为 n_×n_，下标为 edgeId(u,v)。computeOneTree / replaceOneTreeEdge 维护。
        // 增量更新路径用此做 O(1) 存在性查询，替代 O(n) 的 find_if 扫描。
        std::vector<int> edge_index_in_tree;
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

    // BP 集绝大多数只有 1~4 项。内联常见情况，只有更长的 BP 链才回退
    // heap vector，避免每个扩展节点都为短 B 集分配内存。
    struct BranchSet {
        static constexpr std::size_t kInlineCapacity = 4;

        void push_back(const BranchChoice& choice)
        {
            if (overflow.empty() && inline_size < kInlineCapacity) {
                inline_choices[inline_size++] = choice;
                return;
            }
            if (overflow.empty()) {
                overflow.reserve(kInlineCapacity * 2);
                overflow.insert(
                    overflow.end(), inline_choices.begin(),
                    inline_choices.begin()
                        + static_cast<std::ptrdiff_t>(inline_size));
            }
            overflow.push_back(choice);
        }

        std::size_t size() const
        {
            return overflow.empty() ? inline_size : overflow.size();
        }

        bool empty() const { return size() == 0; }

        BranchChoice& back()
        {
            return overflow.empty()
                ? inline_choices[inline_size - 1] : overflow.back();
        }

        const BranchChoice& front() const
        {
            return overflow.empty() ? inline_choices[0] : overflow.front();
        }

        const BranchChoice& operator[](std::size_t index) const
        {
            return overflow.empty() ? inline_choices[index] : overflow[index];
        }

        std::array<BranchChoice, kInlineCapacity> inline_choices{};
        std::vector<BranchChoice> overflow;
        std::size_t inline_size = 0;
    };

    // DFS 热路径只维护一棵可变 1-tree。每次交换记录足够的信息，子节点
    // 返回时按 checkpoint 逆序恢复，避免复制 OneTree 中的 O(n^2) 索引表。
    struct TreeUndo {
        std::size_t edge_index = 0;
        Edge old_edge;
        Edge new_edge;
        double old_cost = 0.0;
        bool old_feasible = false;
        bool state_only = false;
        bool full_snapshot = false;
    };

    struct CandidateUndo {
        std::size_t index = 0;
    };

    struct TourCandidate {
        double cost = std::numeric_limits<double>::infinity();
        std::vector<int> tour;
    };

    struct RootReducedCostStats {
        std::size_t tested = 0;
        std::size_t fixed_zero = 0;
        std::size_t tree_tested = 0;
        std::size_t fixed_one = 0;
        std::size_t active_after = 0;
        bool proves_no_improvement = false;
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
        // n <= 64 的生产热路径用一个 word 保存每个 DSU 根的成员集合，
        // union/rollback 均为 O(1)，并可直接枚举较小分量。
        std::vector<std::uint64_t> forced_member_bits;
        // 每个 forced DSU 根的成员列表；union 时只向胜方追加，rollback 时
        // resize；n > 64 及局部测试兼容路径使用。
        std::vector<std::vector<int>> forced_members;
        // 各顶点在 forced 边子图中的度数。
        std::vector<int> forced_degree;
        // forced 边列表与 MST 部分（不含顶点 0 的边）的权值和及计数。
        std::vector<Edge> forced_edges;
        double forced_mst_cost = 0.0;
        int forced_mst_count = 0;
        // 仅供局部候选向量/单元测试的兼容掩码。生产 DFS 留空并以
        // candidate_bits 为唯一 active 状态，避免每次删除的 n*n 随机写。
        std::vector<unsigned char> candidate_mask;
        // 按全局 immutable candidates_sorted_ 的稳定下标保存 active 位。
        // 顺序枚举、incident 过滤和 MST cut 查询均直接使用该 bitset。
        std::vector<std::uint64_t> candidate_bits;
        std::size_t candidate_bit_count = 0;
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
    // 仅在根节点用次梯度法优化一次 Held-Karp 顶点势；搜索期间固定不变，
    // 因而候选排序和动态 MST replacement 仍可复用。
    void optimizeRootPotentials(double upper_bound);
    double adjustedEdgeWeight(int u, int v) const;
    // 禁用当前 1-tree 中的一条未强制边后，使用 MST replacement edge 增量更新。
    // 根边可用性由生产 active bitset（局部兼容路径为 candidate_mask）判定。
    bool updateOneTreeAfterForbid(const PartialSol& node,
                                  const std::vector<Edge>& branch_candidates,
                                  OneTree& tree,
                                  const Edge& forbidden_edge,
                                  bool record_undo = false) const;
    // force 分支后，buildBranchCandidates 移除的候选边中若包含当前 1-tree 的边，
    // 则为每条被移除的树边寻找替代边，增量更新 1-tree，避免从零重算 MST。
    // 返回 false 表示存在无法替代的树边，调用者应回退到完整 computeOneTree。
    bool updateOneTreeAfterCandidateRemoval(
        const PartialSol& node,
        const std::vector<Edge>& branch_candidates,
        OneTree& tree,
        const std::vector<std::size_t>& removed_edge_ids,
        bool record_undo = false) const;
    bool updateOneTreeAfterActiveRemoval(
        const PartialSol& node, OneTree& tree,
        std::size_t candidate_checkpoint, bool record_undo) const;
    bool updateOneTreeAfterRemovedCandidate(
        const PartialSol& node, const std::vector<Edge>& branch_candidates,
        OneTree& tree, std::size_t removed_id, bool record_undo) const;
    const Edge* findMstReplacement(const PartialSol& node,
                                   const std::vector<Edge>& branch_candidates,
                                   const OneTree& tree,
                                   const Edge& removed_edge) const;
    // 构建并维护 OneTree 携带的动态 MST 拓扑；根边不属于该状态。
    void initializeDynamicMst(OneTree& tree) const;
    void setDynamicMstEdge(OneTree& tree, int u, int v, bool present) const;
    void replaceOneTreeEdge(OneTree& tree, std::size_t edge_index,
                            const Edge& replacement,
                            bool record_undo = false) const;
    void markOneTreeInfeasible(OneTree& tree, bool record_undo) const;
    void replaceOneTreeWithRebuild(OneTree& tree, OneTree rebuilt) const;
    void rollbackOneTree(OneTree& tree, std::size_t checkpoint) const;
    // 从被删树边两端交替遍历，标记先完成（即较小）的分量。生产搜索随后
    // 将该分量的 incident candidate 位图异或成 fundamental cut 位图。
    bool markMstComponentWithoutEdge(const OneTree& tree,
                                     const Edge& removed_edge) const;
    bool dynamicMstMatchesEdges(const OneTree& tree) const;
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
    // 生产搜索路径使用稳定的全局候选表：过滤原地清除 active bit，
    // 并将实际发生的 active->inactive 变化写入 candidate_undo_。
    bool filterActiveCandidates(
        PartialSol& node, int merged_scan_root = -1,
        std::size_t merged_scan_count = 0,
        std::uint64_t merged_scan_bits = 0,
        std::vector<std::size_t>* removed_edge_ids = nullptr) const;
    void deactivateCandidate(PartialSol& node, std::size_t edge_id) const;
    void deactivateCandidateByIndex(
        PartialSol& node, std::size_t candidate_index) const;
    void deactivateCandidateBits(
        PartialSol& node, std::size_t word_index,
        std::uint64_t candidate_bits) const;
    void deactivateIncidentCandidates(PartialSol& node, int vertex) const;
    void rollbackCandidates(PartialSol& node, std::size_t checkpoint) const;
    void adjustAvailableDegree(int vertex, int delta) const;
    bool isCandidateActive(const PartialSol& node, std::size_t edge_id) const;
    // 在当前候选 bitset 中查找下一个 active 候选；未初始化 bitset 时回退逐项扫描。
    void resetCandidateBits(PartialSol& node, std::size_t candidate_count) const;
    std::size_t nextActiveCandidate(const PartialSol& node, std::size_t begin,
                                    std::size_t candidate_count) const;
    // 精确恢复 DFS force 前的 MST 缓存；不能用 +w/-w 逆运算，因为不同
    // 动态范围的浮点数相加会丢失旧值。
    void restoreForcedMstCache(PartialSol& node, double old_cost,
                               int old_count) const;
    // 判断当前节点的下界是否已经不优于已知最优可行解，可以直接剪枝。
    bool shouldPrune(double bound, double best_cost) const;
    // 使用根 1-tree 的 reduced cost 证明非树边不可能属于任何改进 tour，
    // 或证明树边必须属于每个改进 tour；结果成为本轮根搜索的永久状态。
    RootReducedCostStats applyRootReducedCostFixing(
        PartialSol& root, OneTree& root_tree) const;
    // 最近邻 + 2-opt + LK，生成一个可行上界；同时保留若干不同的局部
    // 最优 tour，根下界尚不能证明当前上界时再自适应执行额外 LK。
    bool findInitialTour(std::vector<int>& tour, double& cost,
                         std::vector<TourCandidate>& alternatives);
    bool improveInitialTourDiversified(
        std::vector<TourCandidate>& alternatives,
        double root_lower_bound,
        std::vector<int>& tour, double& cost);
    void maybeImproveIncumbentDiversified();
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
    BranchSet bpPartition(PartialSol& node, OneTree& current_tree);
    // BP 递归搜索：在每个节点执行 BP 划分后枚举“前缀 forbid + 当前 force”子节点。
    void search(PartialSol& node, OneTree& current_tree, int depth,
                bool count_node = true);

    // 顶点数。
    int n_ = 0;
    // 距离矩阵，dist[i][j] 是顶点 i 和 j 之间的距离；dist[i][i] 必须为 0。
    std::vector<std::vector<double>> dist_;
    // 固定 Held-Karp 顶点势。搜索边权为 dist(u,v)+pi[u]+pi[v]，所有
    // 1-tree 成本统一减去 2*sum(pi) 后才作为原问题下界。
    std::vector<double> vertex_potential_;
    double potential_correction_ = 0.0;
    double potential_roundoff_guard_ = 0.0;
    // 原问题所有有限边均为精确整数且任意 n 边和不超过 2^53 时，tour
    // 成本为精确整数；Held-Karp 浮点下界可向上取整后参与安全剪枝。
    bool exact_integer_costs_ = false;
    // 与顶点 0 相连的所有有限边，按权重升序排列。
    // 在 computeOneTree 中取前 2 条未被禁止的边作为 1-tree 的 root edges。
    std::vector<Edge> root_candidates_sorted_;
    // solve() 期间唯一一份按权重排序的候选边表。edge_rank_by_id_ 将无向
    // edgeId 映射到稳定下标，使 active bitset 可原地修改并精确回滚。
    std::vector<Edge> candidates_sorted_;
    std::vector<int> edge_rank_by_id_;
    mutable std::vector<CandidateUndo> candidate_undo_;
    // 当前 1-tree 中失效且非 forced 的边；最多 n 条，仅对这些边按全局
    // candidate rank 排序，避免排序本轮过滤删除的全部候选边。
    mutable std::vector<std::size_t> removed_candidate_scratch_;
    // 每个顶点的不可变 incident candidate 位图。与节点 active 位图相交后
    // 只枚举仍然可用的关联边，避免反复扫描已经失效的静态候选下标。
    std::vector<std::uint64_t> candidate_incident_bits_;
    // 仅包含非根候选边，replacement cut 位图与其相交后可直接跳过根边。
    std::vector<std::uint64_t> internal_candidate_bits_;
    std::size_t candidate_word_count_ = 0;
    mutable std::vector<int> available_degree_;
    mutable int insufficient_degree_count_ = 0;
    mutable std::vector<TreeUndo> tree_undo_;
    // 完整重建仅是增量更新失败时的冷路径；只有该路径保存完整快照。
    mutable std::vector<OneTree> tree_snapshot_undo_;
    // 动态 MST cut 查询复用的双向遍历、分量标记与候选 cut 位图。
    // epoch 避免每次清零 O(n)；从被删边两端交替扩展，先完成的一侧
    // 必然是不大于另一侧的分量。
    mutable std::vector<std::uint32_t> mst_component_mark_;
    mutable std::uint32_t mst_component_epoch_ = 0;
    mutable std::uint32_t mst_selected_component_epoch_ = 0;
    mutable std::uint64_t mst_selected_component_bits_ = 0;
    // 65..128 顶点的动态 MST 分量使用两个 machine word，避免回退到
    // epoch 标记和动态 vector。word_count 为 0 时表示未启用该快路径。
    mutable std::array<std::uint64_t, 2> mst_selected_component_words_{};
    mutable std::size_t mst_selected_component_word_count_ = 0;
    mutable std::vector<int> mst_component_left_;
    mutable std::vector<int> mst_component_right_;
    mutable bool mst_selected_component_is_left_ = true;
    mutable std::vector<std::uint64_t> mst_cut_candidate_bits_;
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
    std::vector<TourCandidate> initial_tour_alternatives_;
    bool diversified_tour_attempted_ = false;
    bool restart_search_requested_ = false;
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
