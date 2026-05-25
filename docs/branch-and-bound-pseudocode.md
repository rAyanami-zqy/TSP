# TSP 分支定界求解器 —— 主求解过程伪代码

## 1. 顶层求解入口 `solve(strategy)`

```
function solve(strategy):
    best_tour ← null
    best_cost ← +∞

    // 阶段一：启发式求初始上界
    findInitialTour(best_tour, best_cost)

    // 阶段二：构造根节点（无边约束）
    root.forced  ← [0, 0, ..., 0]    // n×n 全零
    root.forbidden ← [0, 0, ..., 0]
    root.depth ← 0

    root_tree ← computeOneTree(root.forced, root.forbidden)
    if not root_tree.feasible:
        return INFEASIBLE

    root.bound ← root_tree.cost          // 全局下界

    // 阶段三：递归深度优先搜索
    search(root, root_tree)

    return (best_tour, best_cost)
```

---

## 2. 递归搜索框架 `search(node, current_tree)`

```
function search(node, current_tree):
    node.bound ← current_tree.cost

    // 剪枝规则 1：下界不优于已知最优
    if shouldPrune(node.bound, best_cost):
        nodes_pruned_by_bound++
        return

    nodes_expanded++

    // 终止条件：当前 1-tree 已经是合法 Hamilton 回路
    if isTour(current_tree):
        tour ← buildTour(current_tree.edges)
        if tour is valid and current_tree.cost < best_cost:
            best_cost ← current_tree.cost
            best_tour ← tour
        return

    // 收集当前节点所有未决边作为分支候选
    candidates ← buildBranchCandidates(node)

    if strategy == Smart:
        searchSmart(node, current_tree, candidates)
    else:
        searchExhaustive(node, current_tree, candidates)
```

---

## 3. 分支策略一：Smart（单边二分叉）

**核心思想**：在当前 1-tree 中选一条边做 force/forbid 二分支。优先选度数 >2 的顶点（说明 1-tree 在该点违反了回路约束）的 incident edge，再选权重最大的候选边。

```
function searchSmart(node, current_tree, candidates):
    // 步骤 1：选择一条分支边
    edge ← chooseBranchEdge(node, current_tree, candidates)
    if edge not found:
        return   // 死胡同：无边可分

    // 步骤 2：构造 force 子节点（该边必选）
    forced_child ← makeChild(node, edge, force=true)
    // 步骤 3：构造 forbid 子节点（该边禁止）
    forbidden_child ← makeChild(node, edge, force=false)

    // 步骤 4：按下界升序递归，尽早改进上界
    if both children available
       and forbidden_child.bound < forced_child.bound:
        search(forbidden_child.node, forbidden_child.tree)
        search(forced_child.node, forced_child.tree)
    else:
        search(forced_child.node, forced_child.tree)
        search(forbidden_child.node, forbidden_child.tree)


function chooseBranchEdge(node, current_tree, candidates):
    // 优先级 1：找 1-tree 中度数 > 2 的顶点，从其 incident edges 中选权值最大的候选
    v ← 度数最大的顶点（度数 > 2 时优先）
    if v exists:
        for each edge in current_tree.edges incident to v:
            if edge is eligible (未决、非禁、边权有限):
                选权值最大的

    // 优先级 2：退化为从 1-tree 中任选一条 eligible 边
    for each edge in current_tree.edges:
        if edge is eligible:
            选权值最大的

    // 优先级 3：退化为从所有候选中任选
    for each edge in candidates:
        if edge is eligible:
            选权值最大的

    return selected_edge


function makeChild(node, branch_edge, force):
    child.depth ← node.depth + 1
    child.forced  ← copy(node.forced)
    child.forbidden ← copy(node.forbidden)

    if force:
        child.forced[edgeId(branch_edge)] ← 1
    else:
        child.forbidden[edgeId(branch_edge)] ← 1

    // 在约束下重新计算 1-tree 下界
    child_tree ← computeOneTree(child.forced, child.forbidden)
    if not child_tree.feasible:
        nodes_pruned_infeasible++
        return INVALID_CHILD

    child.bound ← child_tree.cost
    nodes_created++
    return (child, child_tree)
```

**分支树示意（Smart）**：

```
                    root
                     │
              force(e₁) / \ forbid(e₁)
                      /   \
                    N₁    N₂      ← 每条分支边恰好两个子节点
                   / \
            force(e₂) \ forbid(e₂)
             /         \
           ...         ...
```

---

## 4. 分支策略二：Exhaustive（全候选展开）

**核心思想**：对当前节点**所有未决边**同时展开。每条边生成 force 和 forbid 两个子节点，按子节点下界升序排列后依次递归搜索。

```
function searchExhaustive(node, current_tree, candidates):
    if candidates is empty:
        return   // 死胡同：无边可分

    children ← []

    // 对每条候选边，同时生成 force 和 forbid 两个子节点
    for each edge in candidates:
        forced_child ← makeChild(node, edge, force=true)
        if forced_child is valid:
            children.append(forced_child)

        forbidden_child ← makeChild(node, edge, force=false)
        if forbidden_child is valid:
            children.append(forbidden_child)

    // 按下界升序排序：优先探索最有希望的分支
    sort children by child.bound ascending

    // 依次递归搜索所有子节点
    for each child in children:
        search(child.node, child.tree)
```

**分支树示意（Exhaustive）**：

```
                         root
                          │
          ┌───────┬───────┼───────┬───────┐
        F(e₁)  F(e₁)  F(e₂)  F(e₂)  ...    ← 每条候选边生成 force + forbid
          │      │       │      │
         展开   展开    展开   展开             ← 每个子节点再次全展开
```

---

## 5. 1-tree 下界计算 `computeOneTree(forced, forbidden)`

```
function computeOneTree(forced, forbidden):
    // 步骤 0：可行性预检查
    for each edge (u,v):
        if forced[u,v] and forbidden[u,v]:
            return INFEASIBLE     // 冲突
        if forced[u,v] and not isFinite(dist[u,v]):
            return INFEASIBLE     // 强制缺边

    for each vertex v:
        forced_degree[v] ← 强制边中 v 的度数
        if forced_degree[v] > 2:
            return INFEASIBLE     // 度数超限

    // 检查强制边是否已形成子回路（真子集上 |E| = |V|）
    build forced components via DisjointSet
    for each component c:
        if |Vc| > 0 and |Ec| = |Vc| and |Vc| < n:
            return INFEASIBLE

    // 步骤 1：在顶点 {1, 2, ..., n-1} 上构造受约束 MST
    //        （即去掉特殊顶点 0 后其余的生成树）
    tree_components ← DisjointSet(n)
    MST_edges ← [强制边中不含 0 的边，逐条 unite]
    mst_edges_count ← |MST_edges|

    // 补齐候选边：未决、非禁、边权有限、两端都不为 0
    mst_candidates ← sort(filtered_edges, by weight ascending)
    for each edge in mst_candidates (Kruskal):
        if mst_edges_count == n - 2: break
        if tree_components.unite(u, v):
            MST_edges.append(edge)
            cost += edge.w
            mst_edges_count++

    if mst_edges_count != n - 2:
        return INFEASIBLE

    // 步骤 2：给顶点 0 选两条最小 incident edges
    root_edges ← [与 0 相邻的强制边]
    if |root_edges| > 2:
        return INFEASIBLE

    root_candidates ← sort(与 0 相邻的未决边, by weight)
    for each edge in root_candidates:
        if |root_edges| == 2: break
        root_edges.append(edge)

    if |root_edges| != 2:
        return INFEASIBLE

    // 步骤 3：合并并统计度数
    all_edges ← MST_edges ∪ root_edges
    degree[v] ← 每个顶点在 all_edges 中的度数

    return OneTree(feasible=true, cost, edges=all_edges, degree)
```

**1-tree 示意图**：

```
        0          ← 特殊顶点，固定度数为 2
       / \
      /   \
     1 --- 2     ← 顶点 {1..n-1} 构成 MST (n-2 条边)
     |     |
     3 --- 4

    总边数 = (n-2) + 2 = n
```

---

## 6. 辅助函数

```
function edgeId(u, v):
    if u > v: swap(u, v)
    return u × n + v

function isForced(forced, u, v):
    return forced[edgeId(u, v)] != 0

function isForbidden(forbidden, u, v):
    return forbidden[edgeId(u, v)] != 0

function shouldPrune(bound, best_cost):
    return isFinite(best_cost) and bound >= best_cost - ε

function isTour(one_tree):
    return |edges| = n and all_degrees = 2

function buildBranchCandidates(node):
    candidates ← []
    for u ← 0 to n-1:
        for v ← u+1 to n-1:
            if not forced and not forbidden and isFinite(dist[u][v]):
                candidates.append(Edge(u, v, dist[u][v]))
    return candidates
```

---

## 7. 初始上界启发式 `findInitialTour`

```
function findInitialTour(tour, cost):
    best_cost ← +∞
    best_tour ← null

    // 从每个顶点出发跑最近邻
    for start ← 0 to n-1:
        current ← start
        used ← [false, ..., false]
        used[start] ← true
        candidate ← [start]
        ok ← true

        // 最近邻构造
        for step ← 1 to n-1:
            next ← 未访问顶点中距离 current 最近的
            if no reachable next:
                ok ← false; break
            candidate_cost += dist[current][next]
            current ← next
            candidate.append(current)
            used[current] ← true

        if not ok or dist[current][start] is inf:
            continue
        candidate_cost += dist[current][start]

        // 2-opt 局部改进
        twoOpt(candidate, candidate_cost)

        if candidate_cost < best_cost:
            best_cost ← candidate_cost
            best_tour ← candidate

    if found:
        tour ← best_tour
        cost ← best_cost
        return true
    return false
```

---

## 8. 两种策略对比总结

| 维度 | Smart | Exhaustive |
|------|-------|------------|
| 分支因子 | 2（单边二分叉） | 2 × \|candidates\|（全部展开） |
| 搜索树宽度 | 窄 | 宽 |
| 选择策略 | 启发式（度数最大顶点） | 无选择（穷举所有边） |
| 排序 | 按下界大小决定搜索顺序 | 所有子节点按下界升序排列 |
| 适用场景 | 剪枝高效时（大实例） | 搜索完整时（小规模验证） |
| 完备性 | 完备（每条边最终会被决定） | 完备（显式枚举所有可能） |

---

## 9. Smart 策略完备性证明：每条边最终会被决定

### 9.1 要证明什么

**命题**：对于任意 TSP 实例，Smart 分支策略在有限步内终止，且搜索过程蕴含了对所有边决策的隐式穷举——即任意一条边 $(u,v)$ 在搜索树的每一条根到叶的路径上，最终都会被确定为 forced 或 forbidden（或该路径因剪枝/search 回溯而不再展开，此时未决边不影响已找到的最优解）。

更精确的等价表述：

> Smart 策略是 **完备的**（complete）：若最优解存在，算法必能找到。

### 9.2 搜索树结构

将搜索过程视为一棵二叉树 $T$：

- 每个内部节点对应一个**未决边集合** $U \neq \emptyset$ 和一个被选出的分支边 $e \in U$。
- 左子节点：$U' = U \setminus \{e\}$，且 $e$ 被标记为 **forced**。
- 右子节点：$U' = U \setminus \{e\}$，且 $e$ 被标记为 **forbidden**。

记根节点 $U_0 = \{\text{所有有限边}\}$，共 $M \leq \binom{n}{2}$ 条。

### 9.3 引理 1：每条根到叶的路径上 undecided 边数严格递减

在 Smart 策略中，`chooseBranchEdge` 每次从 `candidates`（即 $U$）中选出一条边 $e$，并在两个子节点中分别将其标记为 forced 或 forbidden。因此：

$$|U_{\text{child}}| = |U_{\text{parent}}| - 1$$

证毕。该边在子节点中不再属于 undecided 集合。

### 9.4 引理 2：搜索树深度有界

由引理 1，沿任意一条根到叶的路径，每步严格减少一条未决边。根节点 $|U_0| = M \leq \binom{n}{2}$。因此：

$$\text{max\_depth} \leq M \leq \frac{n(n-1)}{2}$$

不存在无限深的路径。

### 9.5 引理 3：叶节点的三种终止情况

搜索在以下三种情况下不再继续展开子节点：

| 终止类型 | 条件 | 未决边状态 |
|---------|------|-----------|
| **找到回路** | `isTour(one_tree)` → true | 一条合法 Hamilton 回路仅含 $n$ 条 forced 边；其余 $M - n$ 条边隐式为 forbidden。此节点上所有边事实已被决定。 |
| **Bound 剪枝** | `bound >= best_cost - ε` | 该子树不可能改进已知最优解，搜索回溯。未决边无需在此分支继续决定。 |
| **不可行剪枝** | `computeOneTree` 返回不可行 | 当前约束下无合法 1-tree，搜索回溯。 |

关键洞察：**剪枝不会导致"漏解"**。被剪掉的分支中即使存在合法回路，其 cost 也 $\geq$ `best_cost`（bound 剪枝）或根本不存在合法回路（infeasible 剪枝）。而已知 `best_cost` 已被某条完整路径（所有边均已决定）上的合法回路所实现。

### 9.6 引理 4：解空间的完全划分

对于任意 Hamilton 回路 $H$（$n$ 条边组成的环），考虑搜索树的根节点：

- $H$ 中每条边 $e \in H$，在左分支（force $e$）中 $H$ 仍然存在；在右分支（forbid $e$）中 $H$ 被排除。
- $H$ 中不包含的边 $e \notin H$，在右分支（forbid $e$）中 $H$ 仍然存在；在左分支（force $e$）中 $H$ 可能仍存在（除非 force 导致度数超限）。

因此，**每条分支边 $e$ 将剩余解空间划分为两个不交子集**：包含 $e$ 的解和不含 $e$ 的解。任意回路 $H$ 对每条分支边的决策有且仅有一种归属。搜索树的每条根到叶路径对应一个 **部分指派**（对已决定边的 force/forbid 赋值），且所有路径的叶节点覆盖了全部可能指派。

### 9.7 主定理

**定理**：Smart 策略必然能找到最优 TSP 回路（若存在），且在有限步内终止。

**证明**：

1. **有限性**：由引理 2，搜索树深度 $\leq M$，每层最多 2 个子节点（二叉），总节点数 $\leq 2^{M+1}$，有限。
2. **完备划分**：由引理 4，每条分支边将当前解空间做二分划，最优解 $H^*$ 落在搜索树的唯一一条从根到叶的路径上（在 force/forbid 的每个决策中恰好选一侧）。
3. **不遗漏**：
   - 若 $H^*$ 所在路径未被剪枝（bound < best_cost 始终成立，且沿路 1-tree 始终可行），则算法会沿该路径走到 `isTour` = true 的叶节点，从而记录 $H^*$ 为 `best_tour`。
   - 若 $H^*$ 所在路径在某个中间节点被 bound 剪枝，说明此时已有另一个可行解 $H'$ 满足 $\text{cost}(H') \leq \text{bound}(\text{node}) \leq \text{cost}(H^*)$，因此 $H'$ 同样是最优的。
4. **回溯不丢解**：由引理 3，剪枝只丢弃不可能含更优解或根本不可行的子树。

综上，算法终止时 `best_tour` 为最优解。$\square$

### 9.8 直观理解

将 Smart 策略想象为在 $M$ 维超立方体的顶点（每个维度对应一条边的 force/forbid）上做深度优先搜索，但用 1-tree 下界提前跳过不包含更优解的区域。每条边是一个"维度"，每步分支固定一个维度的取值——从根到最优解的路径恰好在每个维度上做了一次选择，遍历了全部 $M$ 个维度。
