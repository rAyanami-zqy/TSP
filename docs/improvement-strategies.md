# TSP 求解器改进策略

基于对当前 branch-and-bound 求解器的全面分析，按影响从大到小排列。

---

## 当前状态总结

- **算法**：基于最小 1-tree 下界的精确分支定界，DFS 递归搜索
- **2026-08-06 更新**：固定根 Held-Karp 势已经实现，默认使用 Polyak
  上升；论文式 Helsgaun 调度和组合精修可通过 `--hk-ascent` 对照。实验见
  `docs/HKMST-LKH-1tree-experiment-2026-08-06.md`。
- **2026-08-06 后续**：已实现选择性持久子树势 epoch；更新点会重建
  HKMST 势相关状态，新势由整棵子树继承，回溯时恢复。当前九实例均衡参数
  为层距 2、gap 2%、16 轮，详见
  `docs/HKMST-persistent-potential-epoch-experiment-2026-08-06.md`。
- **核心瓶颈**：1-tree 下界太弱。如 bays29(n=29) 的 root_lower_bound=1622 vs optimal=2020，gap 约 20%；ulysses22(n=22) 到 dantzig42(n=42) 直接超时

---

## 一、下界增强

### 1. Held-Karp 下界（Lagrangian Relaxation）【P0】

**最重要的单项改进。**

基本思想：
- 给每个顶点 i 分配惩罚值 π[i]
- 修改边权：`w'(i,j) = w(i,j) + π[i] + π[j]`
- 在修改后的图上计算最小 1-tree
- 对于任意 π，`L(π) = 1-tree_cost(π) - 2 * Σπ[i]` 都是有效下界
- 目标：通过次梯度优化找到 max L(π)

次梯度优化流程：
```
初始化 π[i] = 0, step_size = 某个初始值
迭代直到收敛或达到最大迭代次数:
    计算 w'(i,j) = w(i,j) + π[i] + π[j]
    构建最小 1-tree
    对每个顶点 i: subgradient[i] = degree_in_1tree[i] - 2
    L = 1-tree_cost - 2 * Σπ[i]
    更新最佳下界
    更新 π[i] = π[i] + step_size * subgradient[i]
    调整 step_size（如每 K 步减半）
```

理论极限：Held-Karp bound 等价于 TSP 的 LP 松弛，与整数最优值的 gap 通常在 1% 以内。

实现要点：
- 在根节点进行一次 Held-Karp 迭代，得到强化后的下界
- 可选：在每个节点或每隔几层重新运行少量迭代
- 步长策略：`step = α * (upper_bound - L) / ||subgradient||²`，α 从 2 开始逐步减半

参考：这是 Concorde 求解器的核心技术之一（Held & Karp, 1970/1971）。

### 2. 顶点最小边 Bound【P2】

快速补充下界，在 1-tree 计算前做预剪枝：

```
for each vertex v:
    bound = max(bound, (cheapest[v][0] + cheapest[v][1]) / 2)
```

- 实现简单，几乎零开销
- 对某些实例能提供额外几个百分点的剪枝率

---

## 二、搜索策略改进

### 3. Best-First Search（最佳优先搜索）【P1】

当前纯 DFS。改为优先队列（min-heap，按 lower_bound 排序）：

- 优点：更快找到最优解（更早上界 → 更强剪枝）；避免 DFS 陷入大子树（bays29 搜索了 1.6 亿节点才证明最优）
- 缺点：内存增加（需保留活跃节点）
- 折中方案：Hybrid 策略——前 K 层用 best-first，之后切回 DFS（保留当前的可逆状态设计）

实现建议：
```
前 5-10 层用 priority_queue<PartialSol*> 按 bound 排序
深层切回 DFS，复用现有的 reversible state 机制
```

### 4. Strong Branching【P2】

当前 Smart 用启发式选分支边。Strong branching 评估 K 条候选边：

```
对每条候选边 e:
    试探 force e → 快速估算下界 Δ_force
    试探 forbid e → 快速估算下界 Δ_forbid
    评分 = max(min(Δ_force, Δ_forbid))  // max-min criterion
选评分最高的边
```

- 单步开销大（K 次下界估算），但能显著减少搜索树规模
- 可以只评估 5-10 条最有希望的边（如 1-tree 中度数最高的顶点关联边）

---

## 三、上界改进

### 5. Lin-Kernighan 启发式【P2】

当前初始上界：多次最近邻 + 2-opt。替换为更强的启发式：

- **3-opt**：比 2-opt 多一种交换模式（3 条边换 3 条），实现复杂度适中
- **2.5-opt**：2-opt + 节点重插入，代码量小但效果接近 3-opt
- **完整 LK**：动态决定交换深度，是 TSP 启发式的黄金标准

对 bayg29 的影响：初始上界 1618 → 1610，gap 从 8 降到 0，根节点即可证明最优。

### 6. 搜索过程中的 2-opt 改进【P3】

每当找到一个更优的可行解时，运行一次 2-opt 尝试进一步改进，可能额外降低上界。

---

## 四、逻辑推导（Constraint Propagation）

### 7. 度数约束传播【P0】

在每个节点展开前，迭代进行约束推导（在 `buildBranchCandidates` 中或之前）：

```
重复直到没有新推导:
    for each vertex v:
        if forced_degree[v] == 2:
            forbid 所有其他 incident edges  // 度数已满
        if forced_degree[v] == 1 && 只剩 1 条候选边:
            force 该边                       // 必须连这条
        if forced_degree[v] == 0 && 只剩 2 条候选边:
            force 这两条边                   // 必须连这两条
        if available_degree[v] < 2:
            return INFEASIBLE               // 无法满足度数要求
```

- 推导出的 force/forbid 可能触发新一轮推导（迭代至不动点）
- 实现成本很低，但可能指数级减少分支因子
- 预计在 bays29 等实例上减少 10-30% 的节点数

### 8. 子回路消除分支【P2】

当 1-tree 包含子回路时（可从边集合检测），优先对子回路中的边分支：

```
检测 1-tree 中的所有连通分量（子回路）
若有长度 < n 的子回路:
    选择子回路中的一条边作为分支边
    优先 forbid（强制打破该子回路）
```

比 Smart 策略的"高度数顶点"分支更精准，能更快消除不可行结构。

---

## 五、搜索顺序微优化

### 9. Force/Forbid 分支顺序动态调整【P3】

当前始终先 force 后 forbid。可根据 bound 变化决定顺序：

```
估算 Δ_force 和 Δ_forbid:
先探索 bound 增加更少的分支（更可能包含最优解）
```

从 benchmark 看，先 force 后 forbid 大多数情况表现良好，但在某些实例上可能相反。

### 10. 残差 Bound 预剪枝【P3】

维护每个顶点的最小剩余成本：
```
remaining[v] = 两条最便宜未决边的成本和
bound + max_v(remaining[v]) >= best_cost → 剪枝
```

可以在 1-tree 计算前过滤明显无望的节点。

---

## 六、欧几里得实例专项优化

### 11. K-近邻候选集限制【P1】

对于坐标型 TSP 实例，只保留每个顶点的 K 个最近邻作为候选边：

- 最优解中的边几乎从不连接"非近邻"顶点
- 候选集从 O(n²) 降到 O(nK)（如 K=20）
- 所有操作常数级加速：排序、过滤、1-tree 构造

注意：需要验证 K 是否足够大以保证最优性（K=20 对于大多数实例是安全的，K=30 几乎肯定安全）。

### 12. 基于几何的初始候选集剪枝【P3】

对欧几里得实例，利用三角不等式排除不可能出现在最优解中的边：
- α-nearness：只保留最小生成树中"附近"的边
- 基于象限/角度的启发式排除

---

## 七、架构改进

### 13. 并行搜索【P3】

多线程并行搜索不同的子树：

- 简单方案：根节点的不同分支分配给不同线程（共享 `best_cost_` 和 `best_tour_`）
- 更复杂方案：work-stealing 任务队列
- 需要原子操作保护全局最优解

### 14. Bitset 替代 vector<unsigned char>【P3】

将 `forced`、`forbidden`、`candidate_mask` 从 `vector<unsigned char>` 改为 `vector<uint64_t>`（bitset），每个 64-bit word 存 64 条边的状态：

- 内存减为 1/8
- 位运算批量操作更快
- n=42 时 n²=1764 条边，只需 28 个 uint64_t

---

## 优先级汇总

| 优先级 | 策略 | 预期收益 | 实现难度 | 解决的核心问题 |
|--------|------|----------|----------|---------------|
| **P0** | Held-Karp 下界强化 | 根本性突破，可能解决 n=40+ | 中 | 1-tree bound 太弱 |
| **P0** | 度数约束传播 | 改动小收益大 | 低 | 分支因子过大 |
| **P1** | Best-First 搜索 | 更早找到最优解 | 中 | DFS 陷入大子树 |
| **P1** | K-近邻候选集 | 大幅减少候选边 | 低 | 候选集 O(n²) 开销 |
| **P2** | Lin-Kernighan / 3-opt 上界 | 更紧初始上界 | 中-高 | 初始上界不够好 |
| **P2** | Strong Branching | 更少分支节点 | 中 | 分支选择不够精准 |
| **P2** | 子回路消除分支 | 更精准的分支目标 | 低-中 | 分支策略盲目 |
| **P2** | 顶点最小边 Bound | 快速预剪枝 | 极低 | 补充下界 |
| **P3** | Force/Forbid 顺序调整 | 微优化 | 极低 | 搜索顺序 |
| **P3** | 残差 Bound 预剪枝 | 微优化 | 极低 | 提前过滤 |
| **P3** | Bitset 存储压缩 | 内存/缓存优化 | 低 | 常数因子 |
| **P3** | 几何候选集剪枝 | 欧几里得加速 | 低 | 候选集冗余 |
| **P3** | 并行搜索 | 多核利用 | 中-高 | 单线程瓶颈 |
| **P3** | 搜索中 2-opt 改进 | 微优化 | 低 | 上界改进 |

---

## 建议实施路线

**第一阶段（快速见效）**：
1. 度数约束传播 — 1-2 天
2. K-近邻候选集限制 — 1-2 天
3. 顶点最小边 Bound — 半天

**第二阶段（核心改进）**：
4. Held-Karp 下界强化 — 1-2 周
5. Best-First 搜索 — 3-5 天

**第三阶段（精益求精）**：
6. Lin-Kernighan 或 3-opt 上界 — 1 周
7. Strong Branching — 3-5 天
8. 子回路消除分支 — 2-3 天

**第四阶段（锦上添花）**：
9. 并行搜索、Bitset 压缩等微优化

---

## 参考

- Held, M., & Karp, R. M. (1970). The traveling-salesman problem and minimum spanning trees. *Operations Research*, 18(6), 1138-1162.
- Held, M., & Karp, R. M. (1971). The traveling-salesman problem and minimum spanning trees: Part II. *Mathematical Programming*, 1(1), 6-25.
- Lin, S., & Kernighan, B. W. (1973). An effective heuristic algorithm for the traveling-salesman problem. *Operations Research*, 21(2), 498-516.
- Applegate, D. L., Bixby, R. E., Chvátal, V., & Cook, W. J. (2006). *The Traveling Salesman Problem: A Computational Study*. Princeton University Press.
- Helsgaun, K. (2000). An effective implementation of the Lin-Kernighan traveling salesman heuristic. *European Journal of Operational Research*, 126(1), 106-130.
