# TSP 下界计算策略对比：1-tree · Held-Karp · Concorde LP

> 三者的下界质量递进：basic 1-tree (gap 15-30%) → Held-Karp (gap <1%) → Concorde LP+Cutting Planes (gap <0.01%)

---

## 零、三种策略总览

| | basic 1-tree（当前） | Held-Karp | Concorde LP+Cutting Planes |
|---|---|---|---|
| **数学本质** | 最小 1-tree 的代价 | Lagrangian relaxation 的最优值 | LP relaxation + 割平面的最优值 |
| **约束集合** | 顶点 0 度数=2 | 顶点 0 度数=2 + 惩罚其他顶点度数偏差 | 所有顶点度数=2 + 子回路消除 + comb/blossom/clique tree/... |
| **求解方法** | 一次 Kruskal | 50-200 次 Kruskal（次梯度迭代） | 对偶单纯形法 + 割平面循环 |
| **gap 典型值** | 15-30% | <1% | <0.01%（通常 0%） |
| **计算量** | O(m·α(n)) | O(I·m·log m) | O(LP 求解 + 割生成) |
| **实现难度** | 低 | 中 | 极高（需 LP 求解器 + 多种割生成器） |

### 1.1 计算流程（`computeOneTree`, TspSolver.cpp:779-882）

```
输入：部分解 node（含 forced/forbidden 边约束）、候选边列表 branch_candidates
输出：OneTree（feasible, cost, edges, degree）

步骤：
  1. 拷贝 forced 并查集，将顶点 0 所在分量中的非 0 顶点分离为独立根
  2. 直接取用 forced 边中的非 0 边（O(#forced)），累加 forced_mst_cost
  3. Kruskal 扫描候选边（已预排序）：跳过含顶点 0 的边，补齐至 n-2 条 MST 边
  4. 给顶点 0 选 2 条最小权 legal 边（优先 forced，再从 root_candidates_sorted_ 补充）
  5. 统计各顶点度数，返回 total_cost = MST_cost + 2_root_edges_cost
```

### 1.2 下界的数学形式

记原图边权为 `w(u,v)`，顶点 0 为特殊顶点。

```
1-tree_cost = min { MST_cost(V \ {0}) + w(0,a) + w(0,b) }
               其中 a,b ∈ V\{0}, a≠b, 且 (0,a),(0,b) 为顶点 0 的两条最小关联边
```

**性质**：任何 TSP 回路去掉与顶点 0 关联的两条边后，剩余部分是一棵覆盖 V\{0} 的生成树。因此 `1-tree_cost ≤ TSP_optimal_cost`，构成有效下界。

### 1.3 为什么 1-tree 下界很弱

| 实例 | n | root_lower_bound | optimal | gap | gap% |
|------|---|-----------------|---------|-----|------|
| burma14 | 14 | 2542 | 3323 | 781 | 23.5% |
| ulysses16 | 16 | 4746 | 6859 | 2113 | 30.8% |
| gr17 | 17 | 1501 | 2085 | 584 | 28.0% |
| gr21 | 21 | 2252 | 2707 | 455 | 16.8% |
| bays29 | 29 | 1622 | 2020 | 398 | 19.7% |
| bayg29 | 29 | 1375 | 1610 | 235 | 14.6% |
| dantzig42 | 42 | 未知 | 699 | — | — |

**根本原因**：1-tree 只保证顶点 0 的度数为 2，其他顶点的度数完全不受约束。MST 中某些顶点可能度数为 1（叶子）或 ≥3（分支点）。TSP 回路要求**所有**顶点度数恰好为 2。

例如，MST 可以有一个顶点的度数为 5（连接 5 个邻居），这在 1-tree 中合法，但在 TSP 回路中完全不合法。这些"过度连接"的顶点使得 1-tree 使用了太多短边，从而低估了真实成本。

---

## 二、Held-Karp 下界（Lagrangian Relaxation）

### 2.1 核心思想

给每个顶点 v 分配一个**惩罚值** π[v]（实数，可正可负），修改边权：

```
w'(u,v) = w(u,v) + π[u] + π[v]
```

在修改后的图上计算最小 1-tree，然后：

```
L(π) = min_1tree_cost(π) - 2 · Σ π[v]
                v∈V
```

**关键定理**（Held & Karp, 1970）：对于任意 π，L(π) 都是 TSP 最优值的有效下界。

**为什么有效**：任何 TSP 回路中每个顶点度数恰好为 2，因此：
- 回路在修改边权下的总成本 = `Σ_{边} w(u,v) + Σ_{边} (π[u] + π[v])`
- 每个顶点贡献 2 次 π：`Σ_{边} (π[u] + π[v]) = 2 · Σ π[v]`
- 所以：`TSP_cost(w') = TSP_cost(w) + 2 · Σ π[v]`
- 而 `min_1tree_cost(w') ≤ TSP_cost(w')`
- 因此：`min_1tree_cost(w') - 2 · Σ π[v] ≤ TSP_cost(w)` ∎

### 2.2 直观理解：π 如何收紧下界

当前 basic 1-tree 的问题：

```
顶点 v 在 1-tree 中 degree = d(v)
TSP 要求 degree = 2
偏差 = d(v) - 2
```

Held-Karp 利用这个偏差来调整 π：

- 若 `d(v) > 2`（1-tree 中过度连接）：增大 π[v]，使该顶点的关联边变贵，下一次 1-tree 会减少使用该顶点的边
- 若 `d(v) = 1`（1-tree 中连接不足）：减小 π[v]，使该顶点的关联边变便宜，下一次 1-tree 会更多使用该顶点的边
- 若 `d(v) = 2`（恰好）：π[v] 保持不变

通过反复迭代，π 逐步将 1-tree "推"向所有顶点度数为 2 的状态——即 TSP 回路。

### 2.3 次梯度优化算法

```
输入：距离矩阵 w, 初始上界 UB, 最大迭代次数 max_iter
输出：最大下界 L*

初始化：π[v] = 0  for all v
        step_size = 2.0（初始步长参数）
        L* = -∞
        best_1tree = null

for iter = 1 to max_iter:
    // Step 1: 在修改边权下计算最小 1-tree
    w'(u,v) = w(u,v) + π[u] + π[v]  for all u,v
    T = minimum_1tree(w')
    
    // Step 2: 计算当前下界
    L(π) = cost(T) - 2 * Σ π[v]
    L* = max(L*, L(π))
    
    // Step 3: 计算次梯度
    for each v:
        g[v] = degree_in_1tree(v, T) - 2    // 与目标度数的偏差
    
    // Step 4: 检查终止条件
    if ||g|| == 0: break          // 找到了 TSP 回路（所有度数=2）
    if L(π) >= UB - ε: break      // 已达到上界，无需继续
    
    // Step 5: 更新 π（次梯度上升）
    norm_sq = Σ g[v]²
    t = step_size * (UB - L(π)) / norm_sq  // Polyak 步长规则
    
    for each v:
        π[v] = π[v] + t * g[v]
    
    // Step 6: 调整步长参数
    if L(π) 在最近 K 次迭代中没有改善:
        step_size = step_size / 2
```

### 2.4 一个具体例子

考虑 n=5 的简单实例，边权如下（对称）：

```
    0   1   2   3   4
0   -  10   8  15  12
1  10   -   7   9  11
2   8   7   -   6  14
3  15   9   6   -   5
4  12  11  14   5   -
```

**Basic 1-tree（π 全为 0）**：

```
Step 1: MST on {1,2,3,4}
  最短边: (3,4)=5, (2,3)=6, (1,2)=7
  MST_cost = 5+6+7 = 18

Step 2: 顶点 0 的两条最小关联边
  (0,2)=8, (0,1)=10
  root_cost = 8+10 = 18

1-tree_cost = 18+18 = 36
度数分布: d(0)=2, d(1)=2, d(2)=3, d(3)=2, d(4)=1
                                                    ↑d(2)>2  ↑d(4)<2
```

**Held-Karp 迭代一次后**：

次梯度 `g = d - 2 = [0, 0, +1, 0, -1]`

- 顶点 2 度数过高（3→目标2），增大 π[2] → 含顶点 2 的边变贵
- 顶点 4 度数过低（1→目标2），减小 π[4] → 含顶点 4 的边变便宜

更新 π 后重新计算 1-tree，新的 MST 可能选择不同边，使得度数分布更接近 [2,2,2,2,2]。

收敛后下界从 36 提升到更接近真实 TSP 最优值。

### 2.5 Held-Karp 的理论极限

Held-Karp 下界的最大值等于 TSP 的**线性规划松弛（LP relaxation）**的最优值，即 Subtour Elimination Polytope 的 LP 最优值。

**已知事实**：
- Held-Karp bound ≥ basic 1-tree bound（严格占优）
- Held-Karp bound ≤ TSP_optimal（有效下界）
- 对几乎所有 TSPLIB 实例，Held-Karp gap < 1%（远优于 basic 1-tree 的 15-30% gap）
- Held-Karp bound = LP optimal value（Wolsey, 1980 证明等价性）

---

## 三、Concorde 的 LP + 割平面（Cutting Planes）策略

Concorde 是目前世界上最快的 TSP 精确求解器，其下界计算策略与 1-tree 和 Held-Karp 有本质区别。

### 3.1 核心架构：Branch-and-Cut（分支切割）

```
┌─────────────────────────────────────────────────────────────┐
│                    Concorde 下界计算流程                       │
├─────────────────────────────────────────────────────────────┤
│  1. 初始 LP：所有顶点度数=2（等式约束），0 ≤ x_e ≤ 1          │
│                         ↓                                   │
│  2. 割平面循环（Cutting Plane Loop）：                       │
│     对偶单纯形求解 LP → 提取分数解 →                         │
│     ┌─ 连通分量割 (Subtour/Segment/Exact min-cut)            │
│     ├─ Blossom 不等式 (Fast/Exact Padberg-Rao)               │
│     ├─ Comb 不等式 (Block/Grow)                              │
│     ├─ Clique Tree 不等式 (含 Double Decker/Star/Handling)    │
│     ├─ PQ-Tree 割 (Consecutive Ones/Necklace)                │
│     └─ Local Cuts（解小 TSP 子问题生成）                      │
│     → 有 violated cut 就加入 LP，重新求解 → 循环              │
│                         ↓                                   │
│  3. 若 LP 解为整数且构成回路 → 可行解（更新上界）              │
│  4. 若 LP 下界 ≥ 上界 → 剪枝                                 │
│  5. 否则 → 分支（选分数边，强分支评估）→ 回到步骤 1           │
└─────────────────────────────────────────────────────────────┘
```

### 3.2 初始 LP：比 Held-Karp 更弱，但这无妨

Concorde 的初始 LP 只包含：
- **度约束**：每个顶点 `Σ x_e = 2`（等式约束）
- **边界约束**：`0 ≤ x_e ≤ 1`

**这个初始 LP 比 basic 1-tree 还要弱！** 因为它完全没有禁止子回路——最优 LP 解可能是一个由多个小子回路组成的"分数 2-匹配"。然而 Concorde 的策略不是依赖初始 LP 的紧度，而是通过后续的割平面循环逐步加强 LP。

### 3.3 割平面层次：从弱到强的梯次

Concorde 按复杂度和开销递增的顺序调用割生成器：

| 割类型 | 数学形式 | 作用 | 计算开销 |
|--------|---------|------|---------|
| **Subtour (连通分量)** | `x(δ(S)) ≥ 2` | 消除独立连通分量 | 低（BFS + min-cut） |
| **Segment** | `x(δ(S)) ≥ 2`（线性时间） | 快速检测特殊子回路 | 极低（O(n) 扫描） |
| **Exact Subtour** | `x(δ(S)) ≥ 2`（精确最小割） | 找到最违反的子回路割 | 中（Gomory-Hu / Stoer-Wagner） |
| **Blossom (奇集)** | `x(δ(S)) + Σ x(δ(T_i)) ≥ 3k+1` | 消除奇数结构的分数解 | 中-高（Padberg-Rao） |
| **Comb (梳子)** | `x(δ(H)) + Σ x(δ(T_i)) ≥ 3k+1` | 最强大的割族之一 | 高（块分解+贪心生长） |
| **Clique Tree** | 推广的梳子，多手柄 | 进一步约束分数解 | 中（从已有 comb 构造） |
| **Double Decker** | 双层梳子 | 特殊结构的紧割 | 中（从已有 comb 构造） |
| **Star/Handling** | 星形/柄形不等式 | 补充 comb 无法捕获的结构 | 中 |
| **Consecutive Ones (PQ-tree)** | PQ-树割 | 特定排列结构的割 | 高（PQ-树操作） |
| **Necklace (PQ-tree)** | 项链不等式 | 更复杂的 PQ-树结构 | 高 |
| **Local Cuts** | 解小 TSP 子问题 | 理论上可生成任何 violated 割 | 极高（枚举+求解子 TSP） |

**关键设计原则**：
1. 每次迭代先尝试便宜而有效的割（subtour），再逐步升级到昂贵的割
2. 只有当 gap 小于特定阈值时才启用昂贵割（`cutselect` 中的 tolerance 系统）
3. 割生成器的 `tolerance` 随 `min(LB, UB-LB)` 动态调整——gap 越小，容忍越小的 violation
4. 所有生成的割存入全局 **Cut Pool**，在 B&B 树的不同节点间共享和复用

### 3.4 割的容忍度系统

```
beta = min(current_lower_bound, upper_bound - current_lower_bound)
tolerance = beta * tolerance_factor

例如：上界=2020，下界=1622
  beta = min(1622, 398) = 398
  subtour 容忍 = 398 * 0.01 = 3.98（粗略，只加 violation ≥ 3.98 的割）
  local cut 容忍 = 398 * 0.001 = 0.398（精细，violation ≥ 0.398 即可）

随着 gap 缩小，容忍度同步缩小 → 越接近最优解，割的生成越精细
```

### 3.5 LP 求解：对偶单纯形法 + 热启动

与 Held-Karp 的次梯度迭代不同，Concorde 使用精确的线性规划求解器：

- **求解器**：QSopt（Concorde 内置），也可替换为 CPLEX
- **算法**：对偶单纯形法（Dual Simplex），利用 LP 的稀疏结构
- **热启动**：每次加割后从上次的基重新开始（`CClp_load_warmstart`），只需少数几次 pivots
- **列生成**：通过 pricing 动态添加边变量（从稀疏边集开始，按需扩展）

**为什么对偶单纯形更适合**：加割 = 加行（增加新约束），对偶单纯形加行后 primal 不可行但 dual 可行，从 dual feasible basis 出发只需几次迭代即可恢复最优。

### 3.6 Branch-and-Cut 中的下界继承

```
根节点：完整割平面循环（可能 10-50 轮）→ 得到紧下界 L0
子节点：从父节点 LP 基热启动
  ├── 加分支约束（x_e=0 或 x_e=1）
  ├── 对偶单纯形快速恢复最优
  ├── 重新进入割平面循环（通常 1-5 轮即可，因为父节点已加过大量割）
  └── 割池复用：从全局 cut pool 搜索 violated cuts（O(pool_size) 扫描）
```

**关键优势**：割在整棵 B&B 树中累积——父节点发现的 comb 割对子节点同样有效（violation 可能在子节点更大）。

### 3.7 精确有理数验证

Concorde 在剪枝前进行精确有理数验证：

```
输入：LP 对偶变量（浮点数）
1. 将对偶变量转换为 CCbigguy（任意精度有理数）
2. 精确计算 Σ (dual_degree[v] * 2) + Σ (dual_cut[c] * rhs[c])
3. 这就是 TSP 最优值的严格下界
4. 若精确下界 ≥ 上界 → 安全剪枝
5. 若浮点数下界已 ≥ 上界但精确下界不满足 → 不剪枝（安全侧）
```

这消除了浮点 LP 求解的数值误差导致错误剪枝的风险。

### 3.8 实际表现：为什么 Concorde 能在 39ms 内求解 dantzig42

以 dantzig42 (n=42, optimal=699) 为例：

| 阶段 | Concorde | 我们的 B&B | 差异原因 |
|------|---------|-----------|---------|
| 初始下界 | ≈ 699（割平面循环后） | 未知（1-tree，可能 500-600） | Concorde 加 subtour+comb+blossom 割后 LP 下界直接触及 optimum |
| 割平面轮数 | 约 10-20 轮 | — | Concorde 每次加割后 LP 下界跃升 |
| 需要 B&B? | 否（LP 已整数） | 是（gap 巨大） | Concorde 的 LP 解已是整数回路 |
| 总时间 | 39ms | 超时（>30min） | 我们需探索大量 B&B 节点才能证明最优 |

**核心原因**：dantzig42 的 LP relaxation + cutting planes 在根节点就已经产生整数最优解。Concorde 不需要任何分支！

---

## 四、三维详细对比表

| 维度 | basic 1-tree（当前） | Held-Karp | Concorde LP+Cutting Planes |
|------|---------------------|-----------|---------------------------|
| **下界质量** | gap 15-30% | gap < 1% | gap < 0.01%（通常 0%） |
| **计算原理** | 单次最小生成树 | Lagrangian relaxation + 次梯度 | LP 松弛 + 多族割平面 |
| **求解器** | Kruskal 算法 | 迭代 Kruskal（50-200 次） | 对偶单纯形法（QSopt） |
| **约束类型** | 顶点 0 度数=2 | 顶点 0 度数=2 + 惩罚项 | 全度数=2 + subtour + comb + blossom + clique tree + ... |
| **根节点开销** | 数微秒 | 数毫秒 | 数十毫秒（含多轮割生成） |
| **B&B 节点开销** | 数微秒（快速剪枝） | 数毫秒（热启动） | 数毫秒-数秒（LP 重优化+割生成） |
| **割/约束生成** | 无 | 无（通过 π 间接约束） | 10+ 种割生成器，按开销梯次调用 |
| **列生成** | 不需要 | 不需要 | 支持（稀疏边集动态扩展） |
| **数值安全性** | 精确（只有 + 运算） | 浮点（次梯度迭代） | 浮点 LP + 精确有理数验证 |
| **热启动** | 不支持 | π 热启动 | LP 基 + 割池共享 |
| **实现难度** | 低（~200 行） | 中（~500 行） | 极高（~50000 行 + LP 求解器） |
| **依赖** | 无 | 无 | QSopt / CPLEX LP 求解器 |

---

## 五、对搜索树规模的影响估算

以 bays29 (n=29, optimal=2020) 为例：

| 指标 | basic 1-tree（当前） | Held-Karp（估算） | Concorde（实际） |
|------|---------------------|-------------------|-------------------|
| root_lower_bound | 1622 | ≈ 2000-2010 | 2020（触及 optimum） |
| optimal | 2020 | 2020 | 2020 |
| root gap | 398 (19.7%) | ≈ 10-20 (<1%) | 0 (0%) |
| 根节点是否已整数 | 否 | 通常否 | 是（LP 解已是回路） |
| 是否需要 B&B | 是 | 是 | 否（直接证明最优） |
| nodes_expanded | 81,440,628 | 估算 < 1,000,000 | 0 |
| 总时间 | 7 分钟 | 估算 < 10 秒 | < 50ms |

**为什么 gap 缩小效果如此显著**：

分支定界的搜索树大小与 gap 呈指数关系。设根节点 bound = LB，最优值 = OPT：

- 每个 B&B 节点中，bound 在 [LB, OPT] 之间
- 若 LB 提升到 LB'（LB' > LB），所有 bound < LB' 的节点在根部就被剪枝
- 随着 bound 趋近 OPT，剪枝效率指数提升

```
gap = (OPT - LB) / OPT
basic 1-tree:  gap = 19.7% → 需探索大量子树来"证明"OPT≥1622
Held-Karp:     gap = 0.8%  → 只需证明 OPT≥2005，树规模缩小 100-1000x
Concorde LP:   gap = 0%    → LP 解直接是整数回路，无需 B&B
```

**以 dantzig42 (n=42, optimal=699) 为例**：

| 指标 | basic 1-tree | Held-Karp | Concorde |
|------|-------------|-----------|----------|
| root_lower_bound | ≈ 500-550（估算） | ≈ 690-698（估算） | 699 |
| gap | ≈ 20-25% | < 1% | 0% |
| 时间 | >30min（超时） | 估算 < 1min | 39ms |
| 节点数 | 数十亿 | 估算 10K-100K | 0（根节点即证最优） |

---

## 六、在 B&B 节点中应用各策略的方式

### 6.1 Basic 1-tree（当前）

每个 B&B 节点都计算一次 1-tree（约数微秒），用于 bound 剪枝。

### 6.2 Held-Karp

| 策略 | 描述 | 根节点开销 | 子节点开销 | 适用场景 |
|------|------|-----------|-----------|---------|
| 仅根节点 | 根节点 200 次迭代，子节点退化为 basic 1-tree | 数毫秒 | 数微秒 | 根 bound 已足够紧 |
| 热启动 | 根节点 200 次，浅层 10-20 次（从父 π 热启动） | 数毫秒 | 0.5-1ms | **推荐** |
| 全节点 | 每个节点 5-10 次迭代 | 数毫秒 | 0.3-0.5ms | 极难实例 |

### 6.3 Concorde (Branch-and-Cut)

每个 B&B 节点重新进入割平面循环（从父节点 LP 基热启动），通常 1-5 轮即可收敛。割池跨节点共享。

**三种策略的本质差异**：

```
Basic 1-tree:  "算一次 MST，得到什么就是什么"
Held-Karp:     "迭代调整 π，把 MST 推向所有顶点度=2"
Concorde:      "从弱 LP 开始，迭代加割约束，直到 LP 解变成整数回路"
```

---

## 七、伪代码：Held-Karp 根节点实现

```cpp
double computeHeldKarpBound(
    const PartialSol& node,
    const std::vector<Edge>& candidates,
    double upper_bound,    // 当前最优上界
    int max_iter = 200,
    std::vector<double>& out_pi  // 输出最优 π，供子节点热启动
) {
    int n = dist_.size();
    std::vector<double> pi(n, 0.0);
    std::vector<double> best_pi(n, 0.0);
    double best_L = -inf;
    double step_param = 2.0;
    int no_improve = 0;

    for (int iter = 0; iter < max_iter; ++iter) {
        // Step 1: 构建修改边权的候选集
        std::vector<Edge> mod_candidates;
        for (const Edge& e : candidates) {
            mod_candidates.push_back({
                e.u, e.v,
                e.w + pi[e.u] + pi[e.v]   // w'(u,v)
            });
        }
        // 注意：mod_candidates 的排序可能改变，需重新排序
        std::sort(mod_candidates.begin(), mod_candidates.end(),
                  [](const Edge& a, const Edge& b) { return a.w < b.w; });

        // Step 2: 在修改边权下计算 1-tree
        OneTree tree = computeOneTreeWithModifiedWeights(node, mod_candidates);

        // Step 3: 还原真实成本并计算 L(π)
        double real_cost = 0;
        std::vector<int> degree(n, 0);
        for (const Edge& e : tree.edges) {
            real_cost += dist_[e.u][e.v];  // 使用原始边权
            ++degree[e.u];
            ++degree[e.v];
        }
        double L = real_cost;  // L(π) = 1-tree_real_cost（因为 -2Σπ 被抵消了）

        // 实际上 L(π) = modified_cost - 2*Σπ[v]
        // 其中 modified_cost = Σ w'(u,v) = Σ (w(u,v) + π[u] + π[v])
        //                  = real_cost + Σ degree[v]*π[v]
        // 所以 L(π) = real_cost + Σ degree[v]*π[v] - 2*Σπ[v]
        //          = real_cost + Σ (degree[v] - 2)*π[v]

        for (int v = 0; v < n; ++v) {
            L += (degree[v] - 2) * pi[v];
        }

        // Step 4: 更新最优
        if (L > best_L + kEps) {
            best_L = L;
            best_pi = pi;
            no_improve = 0;
        } else {
            ++no_improve;
        }

        // Step 5: 检查终止
        if (best_L >= upper_bound - kEps) break;

        bool all_degree_two = true;
        for (int v = 0; v < n; ++v) {
            if (degree[v] != 2) { all_degree_two = false; break; }
        }
        if (all_degree_two) break;  // 找到了 TSP 回路!

        // Step 6: 计算次梯度
        double norm_sq = 0;
        for (int v = 0; v < n; ++v) {
            double g = degree[v] - 2;
            norm_sq += g * g;
        }

        // Step 7: 更新 π
        double t = step_param * (upper_bound - L) / norm_sq;
        for (int v = 0; v < n; ++v) {
            pi[v] += t * (degree[v] - 2);
        }

        // Step 8: 步长衰减
        if (no_improve >= 20) {
            step_param *= 0.5;
            no_improve = 0;
        }
    }

    out_pi = best_pi;
    return best_L;
}
```

### 关键实现细节

**1. 边权修改后的排序**：加上 π 后边的相对顺序可能改变，每次迭代需重新排序候选边（O(m log m)）。可优化为：只在根节点全排序，迭代中利用 `std::nth_element` 部分排序。

**2. forced/forbidden 约束**：修改边权只影响候选边的选取顺序，不影响 forced/forbidden 的合法性判断。`computeOneTree` 中 forced 边始终优先加入，不受 π 影响。

**3. 数值稳定性**：π 值不应过大（否则边权溢出或精度丢失）。可在每次更新 π 后做中心化：`π[v] -= mean(π)`，这不会改变 1-tree 的结构（因为所有边增加相同常数不影响 Kruskal 的选择）。

**4. 上界来源**：`upper_bound` 来自 LK 启发式（`best_cost_`）。上界越紧，步长计算越准确，收敛越快。

---

## 八、复杂度分析

| 操作 | basic 1-tree | Held-Karp（根节点） | Held-Karp（每个 B&B 节点） |
|------|-------------|---------------------|--------------------------|
| 边排序 | O(1)（已预排序） | O(I · m log m) | O(I · m log m) |
| Kruskal | O(m · α(n)) | O(I · m · α(n)) | O(I · m · α(n)) |
| π 更新 | — | O(I · n) | O(I · n) |
| 总复杂度 | O(m · α(n)) | O(I · m · (log m + α(n))) | O(I · m · (log m + α(n))) |

其中 m = 候选边数（≤ n²/2），I = 迭代次数（50-200）。

对于 n=42，m=861：Held-Karp 根节点约 200 × 861 × log(861) ≈ 200 × 861 × 10 ≈ 1.7M 次操作，实际约 5-20ms。

---

## 九、总结

| | basic 1-tree | Held-Karp | Concorde LP+Cuts |
|---|---|---|---|
| **理论** | 1-tree 松弛（只约束顶点 0） | Lagrangian relaxation 惩罚度数偏差 | LP 松弛 + 不等式族逼近凸包 |
| **gap** | 15-30% | <1% | <0.01%（常为 0%） |
| **实现** | 一次 Kruskal | 50-200 次 Kruskal + 次梯度迭代 | LP 求解器 + 10+ 种割生成器 |
| **收益** | —（基准） | 搜索树缩小 10-1000 倍 | 常无需 B&B（根节点即证最优） |
| **推荐** | 子节点快速 bound | 根节点（+浅层）计算 | 需要工业级 LP 求解器，不适合轻量集成 |

**三者的本质递进关系**：

```
1-tree:        MST(V\{0}) + 2 edges to 0
               ↓ Lagrangian relaxation (π惩罚度数偏差)
Held-Karp:     max_π min_1tree(w + π_penalty)
               ↓ 等价于 LP: degree=2 + subtour constraints
               ↓ 增加 comb, blossom, clique tree 等不等式族
Concorde LP:   min c^T x  s.t.  degree=2, subtour, comb, blossom, ...
               ↓ 若 LP 解非整数 → Branch-and-Cut
               ↓ 若 LP 解整数 → 证明最优
```

**对当前项目的建议路线**：

1. **先实现 Held-Karp**（中等难度，~500 行代码）：将 root gap 从 15-30% 降到 <1%，预计可使 n=40-50 的实例在秒级可解
2. **再考虑轻量割平面**（较高难度）：在 Held-Karp 基础上加入子回路割（min-cut），比完整 Concorde 简单得多但比纯 Held-Karp 更紧
3. **完整 Concorde 路线**（极高难度）：需集成 LP 求解器 + 多族割生成器，不适合作为学习项目

---

## 参考

- Held, M., & Karp, R. M. (1970). The traveling-salesman problem and minimum spanning trees. *Operations Research*, 18(6), 1138-1162.
- Held, M., & Karp, R. M. (1971). The traveling-salesman problem and minimum spanning trees: Part II. *Mathematical Programming*, 1(1), 6-25.
- Wolsey, L. A. (1980). Heuristic analysis, linear programming and branch and bound. *Mathematical Programming Study*, 13, 121-134.
- Valenzuela, C. L., & Jones, A. J. (1997). Estimating the Held-Karp lower bound for the geometric TSP. *European Journal of Operational Research*, 102(1), 157-175.
