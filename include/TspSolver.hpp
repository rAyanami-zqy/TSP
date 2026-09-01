#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <limits>
#include <string>
#include <vector>

namespace tsp {

// 精确求解过程的累计统计。除时间和下界外，计数器均从一次 solve() 开始
// 计数，并跨 diversified-incumbent 触发的根搜索重启累计。
struct SolveStats {
    // 已创建的逻辑 BP 节点数，包含根节点和随后生成的 force 子节点。
    std::size_t nodes_created = 0;
    // 实际进入分支处理的节点数；直接传播的单子节点不重复计数。
    std::size_t nodes_expanded = 0;
    // 因下界不可能严格改善 incumbent 而被剪掉的节点数。
    std::size_t nodes_pruned_by_bound = 0;
    // 因 forced/forbidden 约束无法形成合法 1-tree 而被剪掉的节点数。
    std::size_t nodes_pruned_infeasible = 0;
    // 最后一次根搜索所使用的根 1-tree 下界，位于原问题成本坐标系。
    double root_lower_bound = 0.0;
    // 进入精确搜索前由 NN、2-opt 和 LK 得到的初始可行 tour 成本。
    double initial_upper_bound = 0.0;
    // 搜索节点上的势更新统计。证书模式只使用临时下界；subtree 模式
    // 还会重建候选排序和 HKMST 状态，并让新势在该锚点子树内持续生效。
    // 通过触发条件并实际开始执行的节点势更新次数。
    std::size_t potential_updates_attempted = 0;
    // 得到严格强于原节点下界的势更新次数。
    std::size_t potential_updates_improved = 0;
    // 新势下界直接达到剪枝条件的次数。
    std::size_t potential_updates_pruned = 0;
    // subtree 模式成功安装新势并重建候选/MST epoch 的次数。
    std::size_t potential_updates_rebuilt = 0;
    // 已取得足以安全剪枝的证书，因此不再执行剩余势迭代的尝试数。
    std::size_t potential_updates_stopped_prunable = 0;
    // 所有节点势更新实际执行的次梯度轮数之和。
    std::size_t potential_update_iterations = 0;
    // 节点势上升本身的累计墙钟秒数，不含 subtree 状态重建。
    double potential_update_seconds = 0.0;
    // subtree 模式重建候选排序、位图和 1-tree 的累计墙钟秒数。
    double potential_update_rebuild_seconds = 0.0;
    // 所有有效势更新带来的下界增量之和。
    double potential_update_total_gain = 0.0;
    // 单次节点势更新取得的最大下界增量。
    double potential_update_max_gain = 0.0;
};

// 求解结果。tour 中只保存一圈中的顶点序列，输出时再补回起点。
struct SolveResult {
    // 是否找到至少一条满足输入图约束的 Hamilton 回路。
    bool feasible = false;
    // feasible=true 时为返回 tour 的成本；否则通常为正无穷。
    double cost = 0.0;
    // 不重复末尾起点的顶点访问序列，长度应等于问题顶点数。
    std::vector<int> tour;
    // 本次求解的下界、节点数、剪枝和势更新统计。
    SolveStats stats;
};

// 可选调试输出配置。
struct DebugOptions {
    // 非拥有指针；nullptr 表示关闭调试输出，调用方须保证流在求解期间有效。
    std::ostream* output = nullptr;
    // 每扩展多少个节点输出一次周期性进度；配置为 0 时会归一化为 1。
    std::size_t interval = 1000;
};

// 根节点 Held-Karp 势的上升策略。Polyak 是当前 HKMST 基线；Helsgaun
// 复现论文中的 period/步长与平滑次梯度思想；Hybrid 先运行 Polyak，再
// 从其最佳势出发执行 Helsgaun 精修，并保留实际固定根下界更强的一组势。
enum class RootAscentStrategy {
    // 不改变顶点势，根下界使用原始边权上的最小 1-tree。
    None,
    // 使用上界 gap 与次梯度范数确定步长的 Polyak 上升。
    Polyak,
    // 使用 period、步长缩放和平滑次梯度的 Helsgaun 上升。
    Helsgaun,
    // 先执行 Polyak，再从其最优点做 Helsgaun 精修并保留更强结果。
    Hybrid,
};

// 搜索节点内部的有限轮势上升算法。触发位置和势的作用范围由
// PotentialUpdateStrategy 独立控制；这里仅决定一次更新尝试的步长调度。
enum class NodeAscentStrategy {
    // 使用 incumbent gap 归一化的 Polyak 步长；这是现有 PHKMST 默认值。
    Polyak,
    // 使用 LKH 报告中的 period、步长缩放和 0.7/0.3 平滑次梯度。
    Helsgaun,
};

// 搜索节点上的势更新触发策略。Depth/Adaptive 只生成临时证书；
// SubtreeDepth/SubtreeAdaptive 把更新势安装成可嵌套、回溯恢复的子树 epoch。
enum class PotentialUpdateStrategy {
    // 搜索节点不做额外势更新，只使用根势。
    None,
    // 每隔固定 DFS 深度做临时势更新证书。
    Depth,
    // 深度达到门槛且相对 UB-LB gap 足够小时做临时证书。
    Adaptive,
    // 每隔固定 epoch 深度安装一组在整棵子树内持续生效的新势。
    SubtreeDepth,
    // 同时满足 epoch 深度间隔和相对 gap 时安装子树势。
    SubtreeAdaptive,
};

// BP 在最高度违规顶点上选择下一条树边的顺序。AdjustedWeight 是现有
// PHKMST 基线；RootAlpha* 使用根 1-tree 的静态先验；
// CurrentForbidDeltaDescending 试探当前树边被禁止后的即时 replacement 增量。
enum class BranchEdgeOrder {
    // 按当前势下的调整权重升序选择分支边；这是默认基线。
    AdjustedWeight,
    // 仍只考察默认选中的最大度违规顶点，但优先调整权重较大的树边，
    // 用于验证默认边权方向是否正确。
    AdjustedWeightDescending,
    // 同时考察所有并列最大度违规顶点关联的树边，再按调整权重升序选择；
    // 消除顶点编号平局，不增加 replacement 或 1-tree 试算。
    MaximumDegreeAllAdjustedWeight,
    // 考察所有至少连接一个违规顶点的树边，先最大化两端超度覆盖量，
    // 再按调整权重升序选择。
    MaximumExcessCoverAdjustedWeight,
    // 保持默认最大度违规顶点不变，仅在它的关联树边之间优先选择另一端
    // 超度较大的边；用于隔离“覆盖两个违规点”而不改变分支顶点。
    LocalExcessCoverAdjustedWeight,
    // 只考察连接任一最大度违规顶点的树边，先最大化两端超度覆盖量，
    // 再按调整权重升序选择。
    MaximumDegreeExcessCoverAdjustedWeight,
    // 保持默认最大度违规顶点；在其关联树边中优先选择两端 forced 度更高、
    // active 候选余量更小的边，以提高 force 后触发度满传播的机会。
    PropagationPotentialAdjustedWeight,
    // PropagationPotential 的保守拆分：只优先两端已有 forced 度更高的边，
    // 不使用 active 候选余量；所有 forced 度相同时完全回退默认权重顺序。
    ForcedDegreeAdjustedWeight,
    // 最大度仍是首要顶点规则；并列时选择未决 1-tree 关联边最少的顶点，
    // 再沿用默认调整权重边顺序。
    MaximumDegreeMinimumUndecided,
    // 与上一实验相反，并列最大度时选择未决 1-tree 关联边最多的顶点。
    MaximumDegreeMaximumUndecided,
    // 优先选择根 1-tree alpha-nearness 较小的边。
    RootAlphaAscending,
    // 优先选择根 1-tree alpha-nearness 较大的边。
    RootAlphaDescending,
    // 优先禁止后使当前 1-tree 下界增量最大的树边；无 replacement 的边
    // 视为正无穷。评分只查询当前 fundamental cut，不修改工作 1-tree。
    CurrentForbidDeltaDescending,
    // 优先禁止后使当前 1-tree 下界增量最小的树边，用于隔离评分方向；
    // 同样只做只读 replacement 查询。
    CurrentForbidDeltaAscending,
    // 先保留连接任一当前最大度违规顶点的树边，再按 forbid 增量、两端
    // 超度覆盖量和 adjusted-weight 排序，避免同度热点按顶点编号偏置。
    CurrentForbidDeltaDegreeAware,
    // 统计根势上升各轮 1-tree 的选边频率；在当前最大度热点中优先选择
    // 出现频率最接近 1/2 的树边，模拟 LP middle branching 的不确定性。
    RootOneTreeFrequencyMiddle,
    // 仅对 adjusted-weight 最优的两条当前树边试探 force/forbid 两侧，
    // 按较弱一侧的下界增益降序选择，用于受限 strong-branching 实验。
    TwoSidedStrongBranchingTop2,
};

// TSPLIB 坐标点。二维格式只使用 x/y，三维格式同时使用 z。
struct Point {
    // 第一坐标分量。
    double x = 0.0;
    // 第二坐标分量。
    double y = 0.0;
    // 第三坐标分量；二维问题保持 0。
    double z = 0.0;
};

// 支持本项目原始矩阵格式和 TSPLIB TSP 格式的统一问题表示。
struct TspProblem {
    // TSPLIB NAME；原始矩阵格式使用实现生成的默认名称。
    std::string name;
    // TSPLIB TYPE，精确 TSP 路径通常要求为 TSP。
    std::string type;
    // 距离定义，如 EUC_2D、GEO、EXPLICIT。
    std::string edge_weight_type;
    // EXPLICIT 矩阵的展开格式，如 FULL_MATRIX、UPPER_ROW。
    std::string edge_weight_format;
    // 坐标型实例的顶点坐标；矩阵型实例可为空。
    std::vector<Point> coordinates;
    // 显式对称距离矩阵；坐标型实例可延迟到 toDenseMatrix() 再生成。
    std::vector<std::vector<double>> matrix;

    // 返回顶点数：优先取显式矩阵行数，否则取坐标数量。
    int dimension() const;
    // coordinates 非空时返回 true。
    bool hasCoordinates() const;
    // matrix 非空时返回 true。
    bool hasDenseMatrix() const;
    // 返回原始问题中边 (u,v) 的距离；坐标型问题按 edge_weight_type 计算。
    // u/v 必须是 [0, dimension()) 内的顶点下标。
    double distance(int u, int v) const;
    // 返回 n×n 稠密距离矩阵；已有 matrix 时复制返回，否则由坐标生成。
    // max_dimension>0 且 n 超限时抛出异常，防止意外分配过大矩阵。
    std::vector<std::vector<double>> toDenseMatrix(std::size_t max_dimension = 0) const;
};

// 大规模启发式求解器的性能/质量参数；不用于 BranchBoundSolver 精确路径。
struct HeuristicOptions {
    // 构造初始 tour 时尝试的起点数量。
    std::size_t starts = 4;
    // n 超过该阈值时跳过 O(n²) 最近邻，改用 Morton 空间顺序构造初始 tour。
    std::size_t nearest_scan_limit = 5000;
    // n 不超过该值时允许完整 O(n²) 2-opt 扫描。
    std::size_t full_two_opt_limit = 2000;
    // 大实例窗口化 2-opt 中每个位置向前检查的最大跨度。
    std::size_t two_opt_window = 64;
    // 2-opt 最多执行的完整改进轮数。
    std::size_t two_opt_passes = 2;
    // 随机起点/扰动使用的确定性种子。
    unsigned seed = 1;
    // 启发式进度输出配置。
    DebugOptions debug;
};

// 单元测试专用友元访问器的前置声明，不属于生产 API。
struct BranchBoundSolverTestAccess;

// 分支定界 TSP 求解器，使用 BP (Branch Partitioning) 策略。
// 当前实现面向对称 TSP：dist[i][j] 必须等于 dist[j][i]。
// BP 策略：在 1-tree 上执行候选边划分（A 集/B 集），直接枚举 B 集的 force 子节点。

class BranchBoundSolver {
public:
    // 用对称 n×n 距离矩阵构造求解器，并验证尺寸、对角线和对称性。
    // distance 的所有权移入求解器；正无穷可表示不存在的边。
    explicit BranchBoundSolver(std::vector<std::vector<double>> distance);
    // 开启调试输出。output 由调用方拥有，progress_interval 控制节点进度频率。
    void setDebugOutput(std::ostream& output, std::size_t progress_interval = 1000);
    // 关闭调试输出并清除现有输出流指针和频率设置。
    void disableDebugOutput();
    // 设置根节点 Held-Karp 势上升算法；应在 solve() 前调用。
    void setRootAscentStrategy(RootAscentStrategy strategy);
    // 设置搜索节点一次势更新尝试所用的上升算法；不改变触发机制或 epoch 语义。
    void setNodeAscentStrategy(NodeAscentStrategy strategy);
    // 设置 BP 在违规顶点内部选择分支边的比较顺序；不改变下界算法。
    void setBranchEdgeOrder(BranchEdgeOrder order);
    // 配置搜索节点势更新：depth 是深度/epoch 间隔，iterations 是单次
    // 最大轮数，gap_ratio 是 Adaptive 的相对 gap，budget 是每轮根搜索
    // 的最大尝试次数。启用策略时 depth/iterations/budget 必须为正。
    void setPotentialUpdateOptions(PotentialUpdateStrategy strategy,
                                   std::size_t depth,
                                   std::size_t iterations,
                                   double gap_ratio,
                                   std::size_t budget);
    // 只建立启发式上界、根势和根 1-tree，不进入精确 BP 搜索。该模式用于
    // 可复现地下界实验；返回的 cost/tour 只是可行上界，不能视作最优证明。
    void setRootBoundOnly(bool enabled);
    // 从头执行一次求解并返回可行 tour、成本和累计统计。默认模式会完成
    // 精确 BP 证明；root-bound-only 模式只返回启发式 tour 和根下界统计。
    // 同一对象可重复调用，solve() 会重置上一次搜索的可变状态。
    SolveResult solve();

private:
    // 允许测试代码验证内部 1-tree、势更新和回滚不变量。
    friend struct BranchBoundSolverTestAccess;

    // 无向边的紧凑值对象。通常保证 u<v；w 的坐标系取决于所在容器：
    // 精确搜索中的树边通常保存当前 Held-Karp 势下的调整权重。
    struct Edge {
        // 第一个端点下标。
        int u = 0;
        // 第二个端点下标。
        int v = 0;
        // 边权；可能是原始距离，也可能是 dist[u][v]+pi[u]+pi[v]。
        double w = 0.0;
    };

    // 1-tree 下界结果：如果每个点度数都为 2，则该 1-tree 本身就是一条 Hamilton 回路。
    struct OneTree {
        // 是否成功构造出满足当前 forced/forbidden 约束的完整 1-tree。
        bool feasible = false;
        // 已减去势修正和舍入保护后的原问题下界；不可行时为正无穷。
        double cost = 0.0;
        // 恰好 n 条边：顶点 1..n-1 上的 n-2 条 MST 边和两条根边。
        std::vector<Edge> edges;
        // degree[v] 是 v 在 edges 中的度数；全为 2 时可能构成 Hamilton 回路。
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
        // 本项 BP 分支要 force 的边，也是后续项前缀中要 forbid 的边。
        Edge edge;
        // forbid edge 后是否记录了可直接重放的单边 replacement。
        bool has_forbid_replacement = false;
        // true 表示当时通过完整重建或不可重放路径得到前缀树，search 必须重建。
        bool replay_requires_rebuild = false;
        // edge 在当时 prefix OneTree::edges 中的位置。
        std::size_t tree_edge_index = 0;
        // forbid edge 后写入 tree_edge_index 的替代边；仅在 has_* 为 true 时有效。
        Edge forbid_replacement;
    };

    // BP 集绝大多数只有 1~4 项。内联常见情况，只有更长的 BP 链才回退
    // heap vector，避免每个扩展节点都为短 B 集分配内存。
    struct BranchSet {
        // 常见 B 集直接存储在对象内部的最大元素数。
        static constexpr std::size_t kInlineCapacity = 4;

        // 在尾部加入一项；超过内联容量时一次性迁移到 overflow。
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

        // 返回当前 B 集元素数，与实际使用内联还是 heap 存储无关。
        std::size_t size() const
        {
            return overflow.empty() ? inline_size : overflow.size();
        }

        // 集合无元素时返回 true。
        bool empty() const { return size() == 0; }

        // 返回最后一项的可写引用；调用前集合必须非空。
        BranchChoice& back()
        {
            return overflow.empty()
                ? inline_choices[inline_size - 1] : overflow.back();
        }

        // 返回第一项的只读引用；调用前集合必须非空。
        const BranchChoice& front() const
        {
            return overflow.empty() ? inline_choices[0] : overflow.front();
        }

        // 按 BP 顺序返回第 index 项；index 必须小于 size()。
        const BranchChoice& operator[](std::size_t index) const
        {
            return overflow.empty() ? inline_choices[index] : overflow[index];
        }

        // 未溢出时保存前 inline_size 项。
        std::array<BranchChoice, kInlineCapacity> inline_choices{};
        // 一旦启用就保存全部元素，而不仅是超出内联容量的尾部。
        std::vector<BranchChoice> overflow;
        // overflow 为空时 inline_choices 中有效元素的数量。
        std::size_t inline_size = 0;
    };

    // DFS 热路径只维护一棵可变 1-tree。每次交换记录足够的信息，子节点
    // 返回时按 checkpoint 逆序恢复，避免复制 OneTree 中的 O(n^2) 索引表。
    struct TreeUndo {
        // 普通单边替换时被修改的 OneTree::edges 下标。
        std::size_t edge_index = 0;
        // 修改前的树边。
        Edge old_edge;
        // 修改后用于替代 old_edge 的边。
        Edge new_edge;
        // 修改前的原问题下界，回滚时直接恢复以避免浮点加减漂移。
        double old_cost = 0.0;
        // 修改前的可行标记。
        bool old_feasible = false;
        // true 表示日志只记录 feasible/cost 变化，没有交换边。
        bool state_only = false;
        // true 表示完整旧 OneTree 保存在 tree_snapshot_undo_，其余字段无效。
        bool full_snapshot = false;
    };

    // 一次 active->inactive 候选变化的回滚日志。
    struct CandidateUndo {
        // candidates_sorted_ 中的稳定下标，而不是 edgeId。
        std::size_t index = 0;
    };

    // 启发式候选回路及其缓存成本。
    struct TourCandidate {
        // tour 在原始距离矩阵上的闭环成本。
        double cost = std::numeric_limits<double>::infinity();
        // 长度为 n、不重复末尾起点的访问序列。
        std::vector<int> tour;
    };

    // 根 reduced-cost fixing 的局部统计和整体证明结果。
    struct RootReducedCostStats {
        // 已测试能否固定为 x_e=0 的 active 非树边数。
        std::size_t tested = 0;
        // 已证明不可能属于任何改进 tour、因而固定为 0 的边数。
        std::size_t fixed_zero = 0;
        // 已做单边 forbid sensitivity 的根 1-tree 边数。
        std::size_t tree_tested = 0;
        // 已证明每个改进 tour 都必须包含、因而固定为 1 的树边数。
        std::size_t fixed_one = 0;
        // fixing 完成后仍为 active 的可选边数。
        std::size_t active_after = 0;
        // true 表示 fixing 已经证明不存在严格改善 incumbent 的 tour。
        bool proves_no_improvement = false;
    };

    // 分支定界节点（部分解 P）：
    // forced 表示必须选择的边，forbidden 表示禁止选择的边。
    // 同时维护 forced 边的并查集和度数统计，加速 1-tree 计算与候选集筛选。
    struct PartialSol {
        // 从根逻辑节点开始的 BP force 分支深度。
        int depth = 0;
        // 当前 1-tree 或临时势证书给出的原问题下界。
        double bound = 0.0;
        // 大小为 n*n；forced[edgeId(u,v)]!=0 表示边必须进入 tour。
        std::vector<unsigned char> forced;
        // 大小为 n*n；forbidden[edgeId(u,v)]!=0 表示边不得进入 tour。
        std::vector<unsigned char> forbidden;
        // forced 边构成的连通分量并查集（无路径压缩，保证回溯可逆）。
        // forced_parent[v] 是 v 的 DSU 父节点；根满足 parent[root]==root。
        std::vector<int> forced_parent;
        // 仅 DSU 根位置有效的按秩合并高度估计；它不是搜索深度。
        std::vector<int> forced_rank;
        // 仅 DSU 根位置有效的 forced 连通分量顶点数，用于检测提前子回路。
        std::vector<int> forced_comp_size;
        // n <= 64 的生产热路径用一个 word 保存每个 DSU 根的成员集合，
        // union/rollback 均为 O(1)，并可直接枚举较小分量。
        std::vector<std::uint64_t> forced_member_bits;
        // 每个 forced DSU 根的成员列表；union 时只向胜方追加，rollback 时
        // resize；n > 64 及局部测试兼容路径使用。
        std::vector<std::vector<int>> forced_members;
        // forced_degree[v] 是 v 当前已经强制选择的关联边数，合法值不超过 2。
        std::vector<int> forced_degree;
        // 所有 forced 边的顺序列表；Edge::w 使用当前势 epoch 的调整权重。
        std::vector<Edge> forced_edges;
        // forced_edges 中非根边的调整权重和，供 computeOneTree 快速初始化。
        double forced_mst_cost = 0.0;
        // forced_edges 中非根边数量；内部 MST 最终需要 n-2 条边。
        int forced_mst_count = 0;
        // 仅供局部候选向量/单元测试的兼容掩码。生产 DFS 留空并以
        // candidate_bits 为唯一 active 状态，避免每次删除的 n*n 随机写。
        std::vector<unsigned char> candidate_mask;
        // 按全局 immutable candidates_sorted_ 的稳定下标保存 active 位。
        // 顺序枚举、incident 过滤和 MST cut 查询均直接使用该 bitset。
        std::vector<std::uint64_t> candidate_bits;
        // candidate_bits 所对应的候选总数；用于验证位图属于当前 epoch。
        std::size_t candidate_bit_count = 0;
    };

    // 兼容旧式子节点构造路径的暂存结果；生产 bitset 路径通常不使用。
    struct PendingChild {
        // filtered_candidates 是否已成功构造且可供子节点使用。
        bool available = false;
        // 过滤 forced/forbidden/度数/子回路约束后的候选边。
        std::vector<Edge> filtered_candidates;
    };
    // 将无向边 (u,v) 规范化为 u<=v 后映射到 [0,n*n) 的稳定槽位；
    // 返回值与候选权重排序无关，可用于 forced/forbidden 等稠密状态表。
    std::size_t edgeId(int u, int v) const;
    // 查询外部 forced 状态表中边 (u,v) 是否被强制；不修改任何状态。
    bool isForced(const std::vector<unsigned char>& forced, int u, int v) const;
    // 查询外部 forbidden 状态表中边 (u,v) 是否被禁止；不修改任何状态。
    bool isForbidden(const std::vector<unsigned char>& forbidden, int u, int v) const;

    // 在 node 的 forced/forbidden/active 约束下，用已按调整权重排序的
    // branch_candidates 完整构造最小 1-tree。成功时返回合法下界及动态
    // MST 索引；失败时 feasible=false。这是所有增量更新的正确性后备路径。
    OneTree computeOneTree(const PartialSol& node,
                           const std::vector<Edge>& branch_candidates) const;
    // 以 upper_bound 为 Polyak gap 目标，在无分支约束的根节点优化 Held-Karp
    // 顶点势，并安装最佳 vertex_potential_、correction 和 roundoff guard。
    // upper_bound 非有限或数值动态范围不安全时保留全零势。
    void optimizeRootPotentials(double upper_bound);
    // 基于已安装根势和 root_tree 计算所有非树边的强制加入代价 alpha，
    // 写入 root_alpha_by_edge_id_；只用于分支排序，不参与下界计算。
    void buildRootAlphaNearness(const OneTree& root_tree);
    // 估计在当前 node/tree 中禁止 tree_edge 后的精确单边 replacement
    // 增量。仅做只读 cut 查询，不安装 replacement；无替代边时返回 infinity。
    double currentForbidReplacementDelta(
        PartialSol& node, const OneTree& tree,
        const Edge& tree_edge) const;
    // 在 node 副本上强制 tree_edge、执行隐含候选过滤并构造受约束 1-tree，
    // 返回 force 侧试探下界；可证明增益为零时直接复用 current_tree.cost。
    // 函数不修改调用方节点和当前工作树。
    double currentForceBranchBound(
        const PartialSol& node, const OneTree& current_tree,
        const Edge& tree_edge) const;

    // 一次搜索节点势更新的最佳证书。
    struct NodePotentialUpdateResult {
        // 当前约束图是否至少能形成一棵完整 1-tree。
        bool feasible = false;
        // 是否因正式剪枝谓词成立而省掉至少一轮剩余迭代。
        bool stopped_prunable = false;
        // 所有已评估势中最强的原问题下界。
        double bound = -std::numeric_limits<double>::infinity();
        // 实际完成的 1-tree/次梯度评估次数。
        std::size_t iterations = 0;
        // 取得 bound 时对应的顶点势，而不是最后一次工作势。
        std::vector<double> potentials;
    };
    // 从当前 epoch 势 warm start，对 node 临时做至多 max_iterations 轮已配置
    // 的节点上升。current_bound 是已有证书，upper_bound 是 incumbent；返回
    // 历史最强下界和对应势。函数不安装势，也不修改 node 或全局候选状态。
    NodePotentialUpdateResult updateNodePotentialBound(
        const PartialSol& node, double current_bound,
        double upper_bound, std::size_t max_iterations) const;
    // 根据策略、深度、相对 gap、次梯度是否非零和本轮预算判断当前节点
    // 是否值得尝试势更新；仅作判断，不消耗预算也不修改搜索状态。
    bool shouldUpdatePotentials(const OneTree& tree, int depth,
                                double bound, double upper_bound) const;
    // 当前策略是否会安装并持续使用子树势 epoch。
    bool usesPersistentPotentialUpdates() const;
    // 在选中的锚点尝试安装新势和新候选排序，并递归搜索整棵子树。返回
    // true 表示该子树已被新证书剪枝或已由新 epoch 完整处理，调用者不应
    // 再走父势路径；false 表示提升不足或重建失败，应继续原路径。
    bool searchSubtreeWithUpdatedPotentials(
        PartialSol& node, const OneTree& current_tree,
        int depth, bool count_node);
    // 将 potentials 安装为当前势，并依据 active_by_edge_id 重建候选排序、
    // active 位图、根边表和受约束 1-tree。node 会被改写为新 epoch 坐标系。
    OneTree rebuildPotentialEpoch(
        PartialSol& node, const std::vector<double>& potentials,
        const std::vector<unsigned char>& active_by_edge_id);
    // 返回当前势下的修改边权 dist[u][v]+pi[u]+pi[v]；势未初始化时返回原权。
    double adjustedEdgeWeight(int u, int v) const;
    // forbidden_edge 已在 node 中标为禁止后，增量修复 tree：非树边无需
    // 修改，根树边补下一条根边，内部树边寻找 MST replacement。返回 true
    // 表示更新后仍有 1-tree；false 表示不可行。record_undo 控制是否可回滚。
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
    // 从 candidate_checkpoint 之后的候选 undo 日志中找出刚失活且仍在 tree
    // 中的边并逐项替换。返回 false 表示至少一条树边无合法 replacement；
    // record_undo=true 时所有成功交换可由 rollbackOneTree 恢复。
    bool updateOneTreeAfterActiveRemoval(
        const PartialSol& node, OneTree& tree,
        std::size_t candidate_checkpoint, bool record_undo) const;
    // 若 removed_id 对应 active 集合中刚删除的非 forced 树边，为它寻找并
    // 安装替代边；删除的是非树边时直接成功。false 表示树已无法修复。
    bool updateOneTreeAfterRemovedCandidate(
        const PartialSol& node, const std::vector<Edge>& branch_candidates,
        OneTree& tree, std::size_t removed_id, bool record_undo) const;
    // 删除内部 MST 边 removed_edge 后，在 branch_candidates 中查找跨越其
    // fundamental cut 的最轻 active 合法边。返回的指针借用候选容器；
    // nullptr 表示不存在 replacement。根边不应传入本函数。
    const Edge* findMstReplacement(const PartialSol& node,
                                   const std::vector<Edge>& branch_candidates,
                                   const OneTree& tree,
                                   const Edge& removed_edge) const;
    // 根据 tree.edges 重建按行存储的内部 MST 邻接位图；忽略两条根边。
    void initializeDynamicMst(OneTree& tree) const;
    // 在动态 MST 位图中设置或清除无向内部边 (u,v)；根边调用会被忽略。
    void setDynamicMstEdge(OneTree& tree, int u, int v, bool present) const;
    // 用 replacement 替换 tree.edges[edge_index]，同步成本、度数、动态 MST
    // 和边索引；record_undo=true 时先向 tree_undo_ 写入可逆日志。
    void replaceOneTreeEdge(OneTree& tree, std::size_t edge_index,
                            const Edge& replacement,
                            bool record_undo = false) const;
    // 将 tree 标为不可行并把成本置为正无穷；可选记录仅状态回滚日志。
    void markOneTreeInfeasible(OneTree& tree, bool record_undo) const;
    // 用完整重建结果替换 tree，并把旧树移入 snapshot undo 栈供 DFS 恢复。
    void replaceOneTreeWithRebuild(OneTree& tree, OneTree rebuilt) const;
    // 逆序撤销 tree_undo_ 中 checkpoint 之后的所有树变化；checkpoint 必须
    // 来自同一势 epoch 和同一 tree 对象的进入时栈长度。
    void rollbackOneTree(OneTree& tree, std::size_t checkpoint) const;
    // 在逻辑上删除 removed_edge 后，从两端交替遍历并标记先完成（即较小）
    // 的内部 MST 分量，供 replacement cut 查询复用。动态拓扑不一致时返回 false。
    bool markMstComponentWithoutEdge(const OneTree& tree,
                                     const Edge& removed_edge) const;
    // 调试验证：比较 tree 的动态 MST 邻接位图与 edges 中的内部边是否一致。
    bool dynamicMstMatchesEdges(const OneTree& tree) const;
    // 验证增量 1-tree 的生产不变量：完整结构、缓存度数/成本，以及所有
    // forced/forbidden/candidate 约束。true 表示可安全作为下界；false 时
    // 调用者必须完整重建。
    bool oneTreeSatisfiesConstraints(const PartialSol& node,
                                     const OneTree& tree) const;
    // 检查 one_tree 是否可行且所有顶点度数均为 2；1-tree 连通性保证此时
    // 它是一条 Hamilton 回路。
    bool isTour(const OneTree& one_tree) const;
    // 从无序边集合构造从顶点 0 开始的访问序列；结构不构成单一 n 点回路
    // 时返回空 vector。
    std::vector<int> buildTour(const std::vector<Edge>& edges) const;
    // 兼容局部候选路径：从输入候选中过滤 forced/forbidden、度满和提前
    // 子回路边。成功时覆盖 branch_candidates；可选返回被移除的 edgeId。
    bool buildBranchCandidates(const PartialSol& node,
                               std::vector<Edge>& branch_candidates,
                               std::vector<std::size_t>* removed_edge_ids = nullptr) const;
    // 生产搜索路径使用稳定的全局候选表：过滤原地清除 active bit，
    // 并将实际发生的 active->inactive 变化写入 candidate_undo_。merged_scan_*
    // 可指定刚被 forced DSU 合并的较小分量，只检查可能新形成子回路的边；
    // removed_edge_ids 非空时同时收集删除边。返回 false 表示某顶点可用度<2。
    bool filterActiveCandidates(
        PartialSol& node, int merged_scan_root = -1,
        std::size_t merged_scan_count = 0,
        std::uint64_t merged_scan_bits = 0,
        std::vector<std::size_t>* removed_edge_ids = nullptr) const;
    // 按稳定 edgeId 将单条候选设为 inactive；已失活或不存在时不操作。
    void deactivateCandidate(PartialSol& node, std::size_t edge_id) const;
    // 按 candidates_sorted_ 下标将单条候选设为 inactive，并写 undo/更新度数。
    void deactivateCandidateByIndex(
        PartialSol& node, std::size_t candidate_index) const;
    // 在第 word_index 个候选位图 word 中批量清除 candidate_bits 指定的位，
    // 并为每条实际变化记录 undo 和可用度数变化。
    void deactivateCandidateBits(
        PartialSol& node, std::size_t word_index,
        std::uint64_t candidate_bits) const;
    // 将与 vertex 关联的所有 active 可选边批量失活。
    void deactivateIncidentCandidates(PartialSol& node, int vertex) const;
    // 恢复 candidate_undo_ 中 checkpoint 之后的候选 active 位和可用度数。
    void rollbackCandidates(PartialSol& node, std::size_t checkpoint) const;
    // 给 available_degree_[vertex] 加 delta，并同步维护度数不足 2 的顶点计数。
    void adjustAvailableDegree(int vertex, int delta) const;
    // 查询 edge_id 对应候选在 node 当前 epoch 中是否 active；必要时兼容
    // candidate_mask，既无位图也无掩码时视作 active。
    bool isCandidateActive(const PartialSol& node, std::size_t edge_id) const;
    // 将 node 的候选位图初始化为 candidate_count 个全 active 位，并清除
    // 最后一个 machine word 中超出范围的填充位。
    void resetCandidateBits(PartialSol& node, std::size_t candidate_count) const;
    // 返回 [begin,candidate_count) 中首个 active 候选下标；找不到时返回
    // candidate_count。位图与给定数量不匹配时兼容性地返回 begin。
    std::size_t nextActiveCandidate(const PartialSol& node, std::size_t begin,
                                    std::size_t candidate_count) const;
    // 精确恢复 DFS force 前的 MST 缓存；不能用 +w/-w 逆运算，因为不同
    // 动态范围的浮点数相加会丢失旧值。old_cost/old_count 必须来自同一次
    // apply_force 之前的快照。
    void restoreForcedMstCache(PartialSol& node, double old_cost,
                               int old_count) const;
    // 判断 bound 是否已证明当前节点不可能严格改善 best_cost。整数安全域
    // 使用保守 ceil(bound-tolerance)，普通 double 域要求差值超过舍入保护。
    bool shouldPrune(double bound, double best_cost) const;
    // 使用根 1-tree 的 reduced cost 证明非树边 x_e=0，或用单边 forbid
    // sensitivity 证明树边 x_e=1；原地更新 root、root_tree 和候选 active
    // 状态，结果成为本轮根搜索不参与 DFS rollback 的永久基线。
    RootReducedCostStats applyRootReducedCostFixing(
        PartialSol& root, OneTree& root_tree) const;
    // 最近邻 + 2-opt + LK 生成可行上界，并通过输出参数写入最佳 tour/cost；
    // alternatives 保留若干不同局部最优供延迟 LK。找到回路返回 true，
    // 稀疏图上所有构造均失败时返回 false。
    bool findInitialTour(std::vector<int>& tour, double& cost,
                         std::vector<TourCandidate>& alternatives);
    // 对 alternatives 中至多固定数量的不同局部最优 tour 继续执行 LK；
    // 若找到更优结果则原地更新 tour/cost 并返回 true。根下界已证明当前
    // cost 最优时可提前停止。
    bool improveInitialTourDiversified(
        std::vector<TourCandidate>& alternatives,
        double root_lower_bound,
        std::vector<int>& tour, double& cost);
    // 当节点扩展数达到困难度阈值时至多执行一次 diversified LK；若改善
    // incumbent，则设置 restart_search_requested_，要求完整回退并重启根搜索。
    void maybeImproveIncumbentDiversified();
    // 用原始 dist_ 逐边重算闭合 tour 成本；长度错误或含缺边时返回正无穷。
    double tourCost(const std::vector<int>& tour) const;
    // 对 tour 原地执行 2-opt 直到局部无改善，并同步更新缓存 cost。
    void twoOpt(std::vector<int>& tour, double& cost) const;
    // 构建位置映射 pos[v]=v 在 tour 中的下标；tour 必须包含每个顶点一次。
    std::vector<int> buildPositionMap(const std::vector<int>& tour) const;
    // 惰性预计算每个顶点至多 kLkCandidateSize 个最近邻，写入 candidate_set_。
    void buildCandidateSets() const;
    // LK 核心：从当前 tour 边 (t1,t2) 出发搜索顺序 k-opt。next/prev 是
    // 可原地重连的双向链表，active 是 don't-look 标记；成功应用一次严格
    // 改善并更新链表时返回 true。
    bool lkSearch(int t1, int t2, std::vector<int>& next, std::vector<int>& prev,
                  std::vector<bool>& active) const;
    // 对双向链表表示的 tour 遍历起始边并重复执行 lkSearch；若找到任何
    // 严格改善则重建 tour、更新 cost 并返回 true。
    bool linKernighanImprove(std::vector<int>& tour, double& cost) const;
    // 对 tour 原地执行一次确定性随机 double-bridge 非顺序 4-opt 扰动；
    // 小于所需规模时不修改。
    void doubleBridgeKick(std::vector<int>& tour) const;
    // Chained LK 总入口：交替执行局部 LK 和 double-bridge kick，最终将
    // tour/cost 更新为所有尝试中最优的一组。
    void linKernighan(std::vector<int>& tour, double& cost) const;

    // ── BP (Branch Partitioning) 搜索 ──
    // 在 current_tree 上执行 BP 划分：依次选取违规顶点的未决树边并测试
    // forbid 前缀，返回覆盖所有潜在改进 tour 的关键边集合 B。函数内部
    // 临时修改 node/tree，返回前恢复到调用时状态。
    BranchSet bpPartition(PartialSol& node, OneTree& current_tree);
    // BP 深度优先搜索：先剪枝/更新势/识别 tour，再枚举“前缀 forbid +
    // 当前 force”子节点。count_node 控制统计，allow_potential_anchor 防止
    // 刚进入新 epoch 时在同一锚点重复更新势。
    void search(PartialSol& node, OneTree& current_tree, int depth,
                bool count_node = true,
                bool allow_potential_anchor = true);

    // 对称 TSP 顶点数，也是 dist_ 的行列数；顶点下标范围为 [0,n_)。
    int n_ = 0;
    // 原始对称距离矩阵；dist_[i][i]==0，正无穷表示边不存在。求解期间只读。
    std::vector<std::vector<double>> dist_;
    // 当前 Held-Karp 势。通常是根势；subtree 模式进入锚点时替换为更新势，
    // 离开该子树时连同所有依赖势的 HKMST 状态一起恢复。
    std::vector<double> vertex_potential_;
    // 2*sum(vertex_potential_)；从修改权重 1-tree 总和中减去它可回到原问题下界。
    double potential_correction_ = 0.0;
    // 为调整权重求和和势修正相消预留的保守误差；从下界中额外减去。
    double potential_roundoff_guard_ = 0.0;
    // 下一次根搜索使用的根势上升算法配置。
    RootAscentStrategy root_ascent_strategy_ = RootAscentStrategy::Polyak;
    // 搜索节点一次有限轮势更新内部使用的步长调度；默认保持原 Polyak 行为。
    NodeAscentStrategy node_ascent_strategy_ = NodeAscentStrategy::Polyak;
    // BP 在最高度违规顶点上比较未决树边的策略。
    BranchEdgeOrder branch_edge_order_ = BranchEdgeOrder::AdjustedWeight;
    // 大小为 n*n、按 edgeId 索引的根静态 alpha；树边为 0，缺失/未知为
    // infinity。默认 AdjustedWeight 策略下清空以避免预处理和内存开销。
    std::vector<double> root_alpha_by_edge_id_;
    // 根势上升所选策略的所有可行 1-tree 中，每条边按 edgeId 统计的出现
    // 次数；仅 RootOneTreeFrequencyMiddle 分配。与下面样本数共同表示频率，
    // 使用整数比较 |2*count-samples| 可避免浮点排序噪声。
    std::vector<std::uint32_t> root_one_tree_edge_counts_;
    // root_one_tree_edge_counts_ 的 1-tree 样本总数；根重启时重新统计。
    std::uint32_t root_one_tree_sample_count_ = 0;
    // 搜索节点势更新的触发和生命周期策略。
    PotentialUpdateStrategy potential_update_strategy_
        = PotentialUpdateStrategy::None;
    // Depth 策略的层间隔、Adaptive 的最小深度，也是 subtree epoch 的最小间隔。
    std::size_t potential_update_depth_ = 4;
    // 每次 updateNodePotentialBound 最多执行的次梯度轮数。
    std::size_t potential_update_iterations_ = 8;
    // Adaptive/SubtreeAdaptive 允许更新的最大相对 gap。
    double potential_update_gap_ratio_ = 0.05;
    // 每轮根搜索最多尝试多少次节点势更新。
    std::size_t potential_update_budget_ = 1000;
    // 预算按一次精确 DFS 轮次计算；incumbent 改善并重启时清零。
    std::size_t potential_updates_in_round_ = 0;
    // 当前已安装势 epoch 的锚点 DFS 深度；根势 epoch 为 0。
    int current_potential_epoch_depth_ = 0;
    // true 时 solve() 在构造根 1-tree 后返回，不执行 reduced-cost fixing/BP。
    bool root_bound_only_ = false;
    // 原问题所有有限边均为精确整数且任意 n 边和不超过 2^53 时，tour
    // 成本为精确整数；Held-Karp 浮点下界可向上取整后参与安全剪枝。
    bool exact_integer_costs_ = false;
    // 极端动态范围下禁用根上升和节点势更新，避免修改权重与势修正发生
    // 灾难性消减。
    bool potential_ascent_numerically_safe_ = true;
    // 与顶点 0 相连的所有有限边，按权重升序排列。
    // Edge::w 使用当前势；computeOneTree 从中取前两条合法边作为 root edges。
    std::vector<Edge> root_candidates_sorted_;
    // 当前势 epoch 唯一一份按调整权重升序排列的全部有限无向边。
    // 在 epoch 内不可重排，使 node.candidate_bits 的下标稳定。
    std::vector<Edge> candidates_sorted_;
    // 大小 n*n；edge_rank_by_id_[edgeId] 给出该边在 candidates_sorted_ 中
    // 的下标，不存在的边为 -1。
    std::vector<int> edge_rank_by_id_;
    // 当前 DFS/epoch 的候选失活日志；mutable 允许 const 辅助函数维护缓存。
    mutable std::vector<CandidateUndo> candidate_undo_;
    // 当前 1-tree 中失效且非 forced 的边；最多 n 条，仅对这些边按全局
    // candidate rank 排序，避免排序本轮过滤删除的全部候选边。
    mutable std::vector<std::size_t> removed_candidate_scratch_;
    // 每个顶点的不可变 incident candidate 位图。与节点 active 位图相交后
    // 只枚举仍然可用的关联边，避免反复扫描已经失效的静态候选下标。
    std::vector<std::uint64_t> candidate_incident_bits_;
    // 仅包含非根候选边，replacement cut 位图与其相交后可直接跳过根边。
    std::vector<std::uint64_t> internal_candidate_bits_;
    // 一个候选 active/incident 位图所需的 uint64_t word 数，即 ceil(m/64)。
    std::size_t candidate_word_count_ = 0;
    // available_degree_[v] 是 forced 边数加仍 active 的可选关联边数，用于
    // O(1) 发现不可能达到 tour 度数 2 的顶点。
    mutable std::vector<int> available_degree_;
    // available_degree_<2 的顶点数；非零即可判定当前候选图结构不可行。
    mutable int insufficient_degree_count_ = 0;
    // 当前 DFS/epoch 的增量 1-tree 变化日志；checkpoint 是进入作用域前的 size。
    mutable std::vector<TreeUndo> tree_undo_;
    // 完整重建仅是增量更新失败时的冷路径；只有该路径保存完整快照。
    mutable std::vector<OneTree> tree_snapshot_undo_;
    // 动态 MST cut 查询复用的双向遍历、分量标记与候选 cut 位图。
    // epoch 避免每次清零 O(n)；从被删边两端交替扩展，先完成的一侧
    // 必然是不大于另一侧的分量。
    // mark_[v] 保存最近一次访问 v 的 epoch 标号，避免每次 cut 查询清零 O(n)。
    mutable std::vector<std::uint32_t> mst_component_mark_;
    // 下一次通用双向分量遍历使用的访问 epoch；溢出时实现会重置标记数组。
    mutable std::uint32_t mst_component_epoch_ = 0;
    // 通用路径最终选中较小分量所对应的 epoch 标号。
    mutable std::uint32_t mst_selected_component_epoch_ = 0;
    // n<=64 快路径中较小分量的顶点位图；0 表示该表示法当前未使用。
    mutable std::uint64_t mst_selected_component_bits_ = 0;
    // 65..128 顶点的动态 MST 分量使用两个 machine word，避免回退到
    // epoch 标记和动态 vector。word_count 为 0 时表示未启用该快路径。
    mutable std::array<std::uint64_t, 2> mst_selected_component_words_{};
    // 上述数组中有效 word 数，当前实现为 0 或 2。
    mutable std::size_t mst_selected_component_word_count_ = 0;
    // n>128 通用路径从被删边左端开始扩展的 BFS/DFS 队列及已发现顶点。
    mutable std::vector<int> mst_component_left_;
    // n>128 通用路径从被删边右端开始扩展的队列及已发现顶点。
    mutable std::vector<int> mst_component_right_;
    // 通用路径完成后，true 表示 left_ 是选中的较小分量，否则选择 right_。
    mutable bool mst_selected_component_is_left_ = true;
    // fundamental cut 的候选位图临时缓冲；由所选分量的 incident 行异或得到。
    mutable std::vector<std::uint64_t> mst_cut_candidate_bits_;
    // 当前调试输出流和节点进度间隔配置。
    DebugOptions debug_;

    // 每个顶点预留的 LK 最近邻候选数上限。
    static constexpr int kLkCandidateSize = 8;
    // LK 顺序 k-opt 递归允许的最大交换深度。
    static constexpr int kLkMaxDepth = 5;
    // Chained LK 尝试的 double-bridge 扰动次数上限。
    static constexpr int kLkMaxKicks = 10;
    // candidate_set_[v] 保存 v 的近邻顶点下标，按原始距离排序。
    mutable std::vector<std::vector<int>> candidate_set_;
    // candidate_set_ 是否已经针对当前 dist_ 完成惰性构建。
    mutable bool candidate_set_built_ = false;

    // 当前已知最优可行 tour 的原始成本，即分支定界上界 UB。
    double best_cost_ = std::numeric_limits<double>::infinity();
    // 与 best_cost_ 对应、不重复闭环起点的顶点序列。
    std::vector<int> best_tour_;
    // 初始 NN+2-opt 得到的不同次优局部最优，困难时供 diversified LK 使用。
    std::vector<TourCandidate> initial_tour_alternatives_;
    // 当前 solve() 是否已经执行过一次按节点阈值触发的 diversified LK。
    bool diversified_tour_attempted_ = false;
    // diversified LK 改善 UB 后置 true，通知递归搜索立即回退到根并重启。
    bool restart_search_requested_ = false;
    // solve() 正在累计的返回对象；每次 solve 开头清零，结束时写入最优 tour。
    SolveResult result_;
};

// 从输入流读取全部内容，自动识别本项目“n 后接 n*n 权重”矩阵格式或
// TSPLIB TSP 格式；解析失败、格式不支持或数据不一致时抛出异常。
TspProblem readTspProblem(std::istream& input);
// 只读取原始矩阵格式：第一项是正整数 n，随后必须恰有 n*n 个距离值；
// 返回经过尺寸、有限性/缺边表示和对称性验证的矩阵。
std::vector<std::vector<double>> readDistanceMatrix(std::istream& input);
// 大图启发式求解入口：按 options 构造并局部改进可行 tour，不提供最优性
// 证明；返回 stats 主要用于统一接口，不等同于精确搜索节点统计。
SolveResult solveHeuristic(const TspProblem& problem, const HeuristicOptions& options = {});
// 将不重复起点的 tour 格式化为“0 -> 1 -> 2 -> 0”；空 tour 返回空字符串。
std::string formatTour(const std::vector<int>& tour);

} // namespace tsp
