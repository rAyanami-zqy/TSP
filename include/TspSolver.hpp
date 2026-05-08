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

// 基本分支定界 TSP 求解器。
// 当前实现面向对称 TSP：dist[i][j] 必须等于 dist[j][i]。
class BranchBoundSolver {
public:
    explicit BranchBoundSolver(std::vector<std::vector<double>> distance);

    SolveResult solve();

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
        std::vector<Edge> edges;
        std::vector<int> degree;
    };

    // 分支定界节点：
    // forced 表示必须选择的边，forbidden 表示禁止选择的边。
    // one_tree_edges 和 degree 缓存当前节点下界对应的 1-tree。
    struct Node {
        int depth = 0;
        double bound = 0.0;
        std::vector<unsigned char> forced;
        std::vector<unsigned char> forbidden;
        std::vector<Edge> one_tree_edges;
        std::vector<int> degree;
    };

    std::size_t edgeId(int u, int v) const;
    bool isForced(const std::vector<unsigned char>& forced, int u, int v) const;
    bool isForbidden(const std::vector<unsigned char>& forbidden, int u, int v) const;

    // 在 forced / forbidden 约束下构造最小 1-tree，作为该节点的下界。
    OneTree computeOneTree(const std::vector<unsigned char>& forced,
                           const std::vector<unsigned char>& forbidden) const;
    bool isTour(const OneTree& one_tree) const;
    std::vector<int> buildTour(const std::vector<Edge>& edges) const;
    // 从当前 1-tree 中选择一条边做二分支：包含该边 / 禁止该边。
    bool chooseBranchEdge(const Node& node, Edge& edge) const;
    bool shouldPrune(double bound, double best_cost) const;
    // 最近邻 + 2-opt，生成一个可行上界，帮助早剪枝。
    bool findInitialTour(std::vector<int>& tour, double& cost) const;
    double tourCost(const std::vector<int>& tour) const;
    void twoOpt(std::vector<int>& tour, double& cost) const;

    int n_ = 0;
    std::vector<std::vector<double>> dist_;
};

// 输入格式：第一项是 n，后面是 n*n 的距离矩阵。
std::vector<std::vector<double>> readDistanceMatrix(std::istream& input);
std::string formatTour(const std::vector<int>& tour);

} // namespace tsp
