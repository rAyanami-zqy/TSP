# PHKMST 代码阅读顺序建议

> 更新时间：2026-08-27
> 适用范围：`include/TspSolver.hpp`、`src/main.cpp`、`src/TspSolver.cpp`、`src/*.ipp` 与 `tests/TspSolverTests.cpp`

## 阅读原则

不建议从 `TspSolver.cpp` 第一行顺序读到最后。精确搜索主文件仍包含 Held-Karp 势、1-tree、BP 搜索、增量 MST 和实验策略；输入解析与启发式实现已经按职责拆到 `.ipp` 实现片段，线性阅读仍容易过早陷入细节。

建议沿下面的主线逐层深入：

```text
数据结构
  ↓
程序入口
  ↓
solve 总体流程
  ↓
1-tree 下界与剪枝
  ↓
search + bpPartition
  ↓
候选边、增量 MST 与回溯
  ↓
势更新
  ↓
root alpha 分支实验
  ↓
启发式与输入解析
  ↓
测试验证
```

其中，`solve → computeOneTree → search → bpPartition → search` 是必须最先理解的核心调用链。其他部分主要用于增强下界、改善上界或加速这条主链。

## 重构后的文件边界

`.ipp` 文件不是独立翻译单元，而是由 `TspSolver.cpp` 在原位置包含。这样可以按职责阅读源码，同时保持原来的编译顺序、内联机会与热路径代码布局。

| 文件 | 职责 |
|---|---|
| `TspSolver.cpp` | 精确 BP/HKMST 主流程、势更新、1-tree、增量 MST、分支与回溯 |
| `TspInitialTour.ipp` | 精确求解器的初始上界和困难实例多起点入口 |
| `TspLinKernighan.ipp` | 精确求解器内部的 2-opt/Lin-Kernighan 改善 |
| `TspProblemText.ipp` | 输入文本规范化与基础数值转换 |
| `TspProblemWeight.ipp` | 显式距离矩阵权值解析 |
| `TspProblemCoordinate.ipp` | TSPLIB 坐标距离模型 |
| `TspProblemModel.ipp` | `TspProblem` 数据访问与稠密矩阵物化 |
| `TspProblemIO.ipp` | 普通矩阵与 TSPLIB 输入识别、解析 |
| `TspHeuristicSolver.ipp` | 不进入精确 BP 搜索的通用 tour/2-opt 启发式 |

这些拆分是源码组织边界，不引入虚函数、函数包装或运行时模块层，因此主调用链没有增加额外开销。

## 第一阶段：建立数据模型

首先阅读 [`TspSolver.hpp`](../include/TspSolver.hpp)，重点关注：

1. `SolveStats` 与 `SolveResult`；
2. `RootAscentStrategy`、`PotentialUpdateStrategy` 和 `BranchEdgeOrder`；
3. `Edge`；
4. `OneTree`；
5. `PartialSol`；
6. `BranchChoice` 与 `BranchSet`；
7. `TreeUndo` 与 `CandidateUndo`。

第一遍只需要建立以下概念：

- `best_cost_`：当前已知可行 tour 的上界 UB；
- `OneTree::cost`：当前节点的 Held-Karp 下界 LB；
- `PartialSol::forced/forbidden`：分支节点施加的边约束；
- `candidate_bits`：当前仍可参与搜索的候选边；
- `TreeUndo`、`CandidateUndo`：DFS 返回父节点时用于恢复状态的日志。

动态 MST 位图和替换边缓存第一遍可以跳过。

## 第二阶段：沿入口进入精确搜索

按下面的调用顺序阅读 [`main.cpp`](../src/main.cpp)：

```text
main
  └─ parseArgs
      └─ solveInput
          ├─ readTspProblem
          ├─ TspProblem::toDenseMatrix
          ├─ BranchBoundSolver(...)
          └─ BranchBoundSolver::solve
```

这一阶段不必逐行研究 CSV 输出和格式化代码，只需确认命令行参数如何传入求解器。重点关注：

- `--hk-ascent`；
- `--hk-potential-update`；
- `--branch-edge-order`。

## 第三阶段：精读 `solve()` 主骨架

完整阅读 [`BranchBoundSolver::solve()`](../src/TspSolver.cpp)，先将其中的大型子函数视为黑盒。

主流程为：

```text
solve
  ├─ findInitialTour                 建立初始上界
  ├─ optimizeRootPotentials          优化根节点 Held-Karp 势
  ├─ 初始化 forced/forbidden/候选位图
  ├─ computeOneTree                  构造根 1-tree
  ├─ buildRootAlphaNearness          可选的分支顺序实验
  ├─ applyRootReducedCostFixing      根节点边固定
  └─ search                          进入 BP 深度优先搜索
```

第一次阅读 `solve()` 时，先不要进入以下函数的实现：

- `findInitialTour`；
- `optimizeRootPotentials`；
- `applyRootReducedCostFixing`。

先理解它们分别怎样影响上界、下界和候选边。

还要注意 `solve()` 中的外层 `while (true)`：若搜索期间的 diversified heuristic 改善 incumbent，当前 DFS 会完整回退，求解器重新优化根势并开始新一轮精确搜索。

## 第四阶段：理解 1-tree 下界

接下来精读 `computeOneTree()`。它构造的下界可以概括为：

```text
顶点 1..n-1 上的受约束 MST
                  +
顶点 0 关联的两条最小合法边
                  =
最小 1-tree
```

阅读时逐项确认：

- forced 内部边怎样预先加入 MST；
- forbidden 边怎样被跳过；
- 怎样避免 forced 边形成非法环；
- 为什么顶点 0 单独处理；
- `potential_correction_` 怎样将调整权重还原为原问题的合法下界；
- `OneTree::degree` 为什么能够判断 1-tree 是否已经是 Hamilton 回路。

随后阅读：

- `adjustedEdgeWeight()`；
- `isTour()`；
- `buildTour()`；
- `shouldPrune()`。

完成本阶段后，应当能够解释：

> 为什么 1-tree 是 TSP 的合法下界，以及为什么 `shouldPrune(LB, UB)` 不会错误删除最优解？

## 第五阶段：精读 BP 分支循环

将 `search()` 和 `bpPartition()` 对照阅读。这两个函数共同构成精确搜索核心：

```text
search(node, tree)
  ├─ 下界剪枝
  ├─ 可选势更新
  ├─ 检查 tree 是否为 tour
  ├─ bpPartition(node, tree)
  │   ├─ 选择最高度违规顶点
  │   ├─ 从该顶点选择分支边 e
  │   ├─ 临时 forbid(e)
  │   ├─ 增量更新 1-tree
  │   └─ 重复形成 B 集
  │
  └─ 枚举 B 集
      ├─ 保留前缀 forbid
      ├─ force 当前边
      ├─ 过滤不再合法的候选边
      ├─ 修复或重建 1-tree
      ├─ search(child)
      └─ rollback
```

如果：

```text
B = [e1, e2, e3]
```

则产生的子节点近似为：

```text
force(e1)

forbid(e1), force(e2)

forbid(e1), forbid(e2), force(e3)
```

因此，`bpPartition()` 逐步累积 forbid 前缀，`search()` 再沿此前缀枚举 force 分支。应先理解这个划分逻辑，再研究 undo、bitset 和 replacement edge。

## 第六阶段：候选边与回溯

建议按顺序阅读：

1. `filterActiveCandidates()`；
2. `deactivateCandidate()`；
3. `deactivateCandidateBits()`；
4. `rollbackCandidates()`；
5. `restoreForcedMstCache()`。

重点掌握三个不变量：

1. `candidates_sorted_` 的顺序在一个势 epoch 内保持不变；
2. DFS 主要通过 `candidate_bits` 修改候选边 active 状态；
3. 返回父节点时，候选边、forced 状态和 1-tree 必须精确恢复。

checkpoint 可以理解为轻量事务：

```text
记录 checkpoint
    ↓
修改候选位图/1-tree
    ↓
递归搜索
    ↓
rollback(checkpoint)
```

## 第七阶段：增量 HKMST 热点

这部分集中了原 HKMST 围绕 Kruskal 完整重建所做的大部分优化。建议按顺序阅读：

1. `updateOneTreeAfterForbid()`；
2. `updateOneTreeAfterCandidateRemoval()`；
3. `updateOneTreeAfterRemovedCandidate()`；
4. `findMstReplacement()`；
5. `markMstComponentWithoutEdge()`；
6. `replaceOneTreeEdge()`；
7. `rollbackOneTree()`。

核心过程为：

```text
删掉一条当前树边
      ↓
树被分成两个连通分量
      ↓
优先标记较小分量
      ↓
搜索跨越 fundamental cut 的最小合法边
      ↓
找到：替换树边
找不到：1-tree 不可行
状态可疑：完整 computeOneTree 重建
```

阅读时要区分：

- 根边替换与内部 MST 边替换；
- 增量修复成功与增量修复失败；
- 完整重建冷路径；
- DFS 返回时的 undo 恢复。

## 第八阶段：势更新机制

### 根势

先阅读 `optimizeRootPotentials()`：

```text
计算 1-tree
  ↓
观察每个顶点 degree - 2
  ↓
按次梯度更新顶点势
  ↓
重新计算 1-tree 下界
  ↓
保留下界最强的一组势
```

### 搜索节点势更新

随后按顺序阅读：

1. `shouldUpdatePotentials()`；
2. `updateNodePotentialBound()`；
3. `searchSubtreeWithUpdatedPotentials()`；
4. `rebuildPotentialEpoch()`。

需要明确两类模式的区别：

- `Depth/Adaptive`：临时计算更强的下界证书，不改变后续候选边顺序；
- `SubtreeDepth/SubtreeAdaptive`：安装新势、重新建立候选排序，并让新势在整个锚点子树内持续生效。

当前“达到正式可剪枝条件后停止剩余势迭代”的逻辑位于 `updateNodePotentialBound()`。其依据是：继续更新只会加固同一个剪枝证书，不会改变该节点不能改善 incumbent 的结论。

## 第九阶段：根 α-nearness 分支实验

最后阅读：

- `buildRootAlphaNearness()`；
- `bpPartition()` 中的 `branch_edge_better` 比较器。

内部非树边的 alpha 为：

```text
alpha(u,v) = wπ(u,v) - 根 MST 路径上的最大边权
```

非树根边的 alpha 为：

```text
alpha(0,v) = wπ(0,v) - 已选两条根边中的较大权重
```

该策略只改变 BP 在违规顶点上优先选择哪条分支边。它不会：

- 重排 `candidates_sorted_`；
- 改变 Kruskal 结果；
- 改变 1-tree 下界；
- 改变可行性判断。

因此应当在默认 `AdjustedWeight` 分支流程完全读懂后，再研究 root alpha 实验。

### 当前树 forbid-delta 实验

`CurrentForbidDeltaAscending/Descending` 不再使用根静态先验，而是调用
`currentForbidReplacementDelta()`，复用当前动态 MST 的 fundamental-cut
replacement 查询，计算禁止候选树边后的即时成本增量。评分只临时修改并
回滚候选 active 位，不交换工作 1-tree；真正选中边后仍走原增量更新。

该指标仍只观察 forbid 一侧，而 BP 首先递归的是对应 force 子树，因此实验中
虽然降序减少了部分 created 节点，却没有减少 expanded，并因额外 cut 查询而
回退。阅读实现时应重点区分“只读评分查询”和“正式树边替换”。实验数据见
`docs/PHKMST-current-forbid-delta-branch-experiment-2026-08-27.md`。

`CurrentForbidDeltaDegreeAware` 则先收集所有连接当前最大度热点的树边，再按
forbid delta、两端超度覆盖和默认权重顺序选择。它消除了原路径只取一个
最大度顶点造成的编号偏置。关闭节点势更新时 expanded 减少约 69.7%，但逐边
replacement 查询仍使 wall 增加约 28.6%；与 subtree-adaptive 组合时搜索树
反而显著扩大。因此阅读该策略时还应关注“分支顺序—势 epoch—incumbent
重启”之间的耦合。

### 双侧 strong-top2 实验

`TwoSidedStrongBranchingTop2` 先按默认 adjusted-weight 比较器保存两条候选，
然后调用 `currentForbidReplacementDelta()` 和 `currentForceBranchBound()` 分别
估计两侧增益，按“较弱侧增益、两侧乘积、默认顺序”选择。force 试算复制
`PartialSol` 并完整重建局部 1-tree；若候选两端尚无 forced 边，则可证明
当前树仍然可行，直接走零增益快路。

阅读时要特别观察三层成本差异：forbid 侧只是 fundamental-cut 查询，force
侧可能是完整 Kruskal，而最终正式分支仍复用增量树。该实验在关闭势更新时
减少了约 40% expanded，却因试算开销慢约 13.9 倍；开启 subtree-adaptive
后还会改变势 epoch 和 incumbent 重启时机，因此当前只保留为显式实验模式。

### 根多 1-tree 频率中间分支实验

`RootOneTreeFrequencyMiddle` 横跨两个阅读位置：先看
`optimizeRootPotentials()` 如何在原有 Polyak/Helsgaun 评估中累计每条边的
出现次数，再看 `bpPartition()` 如何在所有当前最大度热点树边中最小化
`|2*count-samples|`。该整数式等价于选择出现频率最接近 1/2 的边。

这份统计属于根搜索 epoch：根重启时重算，但 subtree 势更新和普通 DFS 节点
不会刷新。它不增加 1-tree 构造，只改变 BP 顺序；没有根上升样本时回退默认
比较器。九实例中开启节点势后总 expanded 几乎不变但 wall 慢约 11.8%，关闭
节点势后 expanded 放大约 15.9 倍，因此只保留为实验模式。完整数据见
[`PHKMST-root-frequency-middle-branch-experiment-2026-08-27.md`](PHKMST-root-frequency-middle-branch-experiment-2026-08-27.md)。

### 低成本度结构与传播实验

`MaximumExcessCoverAdjustedWeight` 直接从当前 1-tree 度数组合
`max(0, degree[u]-2)+max(0, degree[v]-2)`；`PropagationPotentialAdjustedWeight`
则读取 `PartialSol::forced_degree` 和求解器的 `available_degree_`，优先预计
force 后更容易触发度满过滤的边。其余 `MaximumDegree*`、`LocalExcess*` 和
`ForcedDegree*` 变体用于隔离顶点平局、超度覆盖范围和传播分数来源。

这些策略不执行 replacement 查询或额外 1-tree，评分成本很低；但搜索树仍有
明显实例依赖。九实例无节点势配置中超度覆盖快约 44.4%，在 `gr48` 上却使
expanded 增加约 6 倍；传播策略在 `gr48` 的 subtree-adaptive 配置中更快，
在九实例汇总中却回退。阅读时应把“局部传播潜力”和“整棵 force 子树大小”
严格区分。完整筛选见
[`PHKMST-cheap-branch-strategy-screen-2026-08-28.md`](PHKMST-cheap-branch-strategy-screen-2026-08-28.md)。

## 第十阶段：上界启发式与输入解析

### 上界启发式

先阅读 [`TspInitialTour.ipp`](../src/TspInitialTour.ipp) 和 [`TspLinKernighan.ipp`](../src/TspLinKernighan.ipp)，建议按顺序理解：

1. `findInitialTour()`；
2. `twoOpt()`；
3. `linKernighan()`；
4. `maybeImproveIncumbentDiversified()`。

这些函数不定义分支定界的正确性，但更好的初始上界会直接增强剪枝效果。

### 输入解析

最后阅读 [`TspProblemIO.ipp`](../src/TspProblemIO.ipp) 和 [`TspProblemModel.ipp`](../src/TspProblemModel.ipp)：

1. `readTspProblem()`；
2. `buildExplicitMatrix()`；
3. `TspProblem::toDenseMatrix()`。

若还要理解大规模实例不进入精确 BP 时的独立启发式路径，再阅读 [`TspHeuristicSolver.ipp`](../src/TspHeuristicSolver.ipp)。

## 用测试反向验证理解

推荐按以下顺序阅读 [`TspSolverTests.cpp`](../tests/TspSolverTests.cpp)：

1. `testRandomCompleteSolveAgainstBruteForce()`：验证整体精确搜索；
2. `testRootAscentStrategies()`：验证根势上升；
3. `testSearchNodePotentialUpdates()`：验证节点势更新；
4. `testRootAlphaNearness()`：验证 alpha 计算；
5. `testRootOneTreeFrequencyCollection()`：验证每个根样本累计恰好 `n` 条边；
6. 从 `testInternalReplacement()` 到 `testRandomSparseTiedForbids()`：理解增量 MST；
7. `testRootReducedCostFixing()`：理解根节点 reduced-cost fixing。

## 推荐调试断点

如果结合调试器阅读，建议先在小规模实例上依次设置断点：

1. `BranchBoundSolver::solve()`；
2. `computeOneTree()`；
3. `search()`；
4. `bpPartition()`；
5. `updateOneTreeAfterForbid()`；
6. `findMstReplacement()`；
7. `rollbackOneTree()`。

每次暂停时重点观察：

- `best_cost_`；
- `node.bound` 与 `current_tree.cost`；
- `node.forced_edges`；
- `node.forbidden`；
- `current_tree.edges` 与 `current_tree.degree`；
- `candidate_undo_` 和 `tree_undo_` 的 checkpoint。

建议先使用 5～8 个顶点的实例单步执行。规模太大时，BP 链、候选位图和 replacement edge 的状态变化很难人工追踪。

## 阅读完成标准

完成详细阅读后，应当能够独立回答以下问题：

1. 当前实现为什么使用 1-tree 作为 TSP 下界？
2. BP 的 B 集如何覆盖当前节点的全部可行改进 tour？
3. force 和 forbid 分支分别修改哪些状态？
4. 为什么候选位图和动态 1-tree 可以安全回滚？
5. 增量 MST 失败时为何必须回退到完整 `computeOneTree()`？
6. 根势、临时节点势和 subtree 势 epoch 的生命周期有何区别？
7. root alpha、root frequency 和 strong-top2 为什么只影响搜索顺序而不影响下界正确性？
8. 哪些代码属于算法正确性主线，哪些代码只是性能优化？
