# 1-tree forbid 增量更新重构汇总

## 目标

原 `bpPartition()` 在构造 B 集时，每禁用当前 1-tree 中的一条边都会：

1. 复制当前候选边集合；
2. 线性查找并删除被禁边；
3. 重新执行一次完整 `computeOneTree()`，包括受约束 Kruskal、根边选择、边集和度数重建。

本次重构把这条 forbid 热路径改为 replacement edge 增量更新。`force` 子节点仍完整计算
1-tree，因为新增强制边会改变 forced 分量、度数过滤和多条候选边的可用性，不满足单边删除
更新的前提。

## 具体修改

### 1. 新增 `updateOneTreeAfterForbid()`

文件：`include/TspSolver.hpp`、`src/TspSolver.cpp`

输入是当前约束下的精确最小 1-tree、已排序候选边，以及刚被禁用的当前树边。函数原地更新
`edges`、`degree` 和 `cost`。

非根边 `u != 0 && v != 0` 的处理：

1. 只取 `V - {0}` 上的 MST 边，排除被删除边后重建两个连通分量；
2. 顺序扫描已按权重升序排列的候选边；
3. 跳过根边、累计 forbidden 边和被 `candidate_mask` 排除的边；
4. 第一条跨越两个分量的边就是最轻 replacement edge；
5. 没有跨割边时，将受约束 1-tree 标记为不可行。

根边 `u == 0 || v == 0` 的处理：

1. `V - {0}` 上的 MST 保持不变；
2. 扫描只包含根关联边的预排序 `root_candidates_sorted_`；
3. 选择最轻的、仍允许且尚未被 1-tree 采用的根关联边；
4. 可用根边不足两条时，将受约束 1-tree 标记为不可行。

成功更新时统一执行：

```text
cost = cost - removed.weight + replacement.weight
degree[removed.u]--
degree[removed.v]--
degree[replacement.u]++
degree[replacement.v]++
```

禁用非树边时，当前 1-tree 不变。BP 当前只会把未强制的当前树边传入该函数。

### 2. `bpPartition()` 改为连续原地更新

`bpPartition()` 现在只在入口复制一次 `current_tree`。每轮 prefix forbid 都在上一轮得到的最小
1-tree 上执行一次增量更新，不再：

- 复制 `branch_candidates`；
- 用 `find_if + erase` 删除候选边；
- 临时修改再恢复 `candidate_mask`；
- 调用完整 `computeOneTree()`。

累计禁边由 `node.forbidden` 统一判定，因此已经禁用的 B 集前缀不会重新成为替代边。

### 3. 根节点复用已计算的 1-tree

`solve()` 为记录 `root_lower_bound` 已经计算过根 1-tree。现在通过 `precomputed_tree` 将它直接
传给 `search()`，避免根搜索入口再次完整计算同一棵 1-tree。其他搜索节点仍按原逻辑完整计算。

### 4. 修复 forced DSU 的既有问题

旧 `computeOneTree()` 复制包含顶点 0 的 forced DSU，再拆开 0 所在分量。这会丢失该分量中
非根 forced 边的连通关系。例如同时强制 `(0,1)` 和 `(1,2)` 时，`(1,2)` 已计入 MST 成本和
边数，但 1、2 可能在 Kruskal DSU 中被错误拆开。

现在 MST DSU 从独立集合开始，并显式合并全部非根 forced 边。初始化仍为
`O(n + |forced|)`，同时恢复了受约束 MST 和 replacement edge 所依赖的树结构不变量。

### 5. 清理与接口调整

- `computeOneTree()` 的候选参数改为 `const std::vector<Edge>&`；
- 删除从未写入或读取的 `OneTree::unfixed_edges`；
- CMake 接入 CTest，并增加无第三方测试可执行文件 `tsp_solver_tests`。

## 正确性边界

增量更新成立需要同时满足：

- 当前 `OneTree` 是当前约束下的精确最小 1-tree；
- 状态变化只有“新增禁止一条当前未强制树边”；
- 候选集合、其他 forced/forbidden 状态和边权没有同时改变；
- `candidate_mask` 与传入候选集合保持同步；
- 连续 forbid 必须基于上一轮更新后的 1-tree 串行执行。

以下情况继续完整调用 `computeOneTree()`：

- 新增 forced 边；
- force 导致度数、子回路或候选掩码批量变化；
- 将来若引入 Held-Karp 顶点势且势值发生变化，所有修正边权及根边顺序需要重新计算。

需要区分两种“无替代边”：

- 非根树边无替代边：`V - {0}` 上不再存在满足约束的生成树，因而 1-tree 不可行；原 TSP
  图仍可能通过顶点 0 保持整体连通。
- 根边无替代边：可用根关联边不足两条，因而 1-tree 不可行。

这两种情况都足以剪掉当前受约束搜索状态，但不能一概表述为原图删除该边后整体不连通。

## 复杂度变化

候选边已预排序，因此旧实现每轮 forbid 的主要成本是一次完整 Kruskal 扫描和完整结果重建。

- 非根边增量更新：重建当前 MST 的两个分量为 `O(n alpha(n))`，再扫描到第一条跨割候选边；
  最坏仍为 `O(m alpha(n))`，但不再重建整棵 MST、根边、度数和候选副本。
- 根边增量更新：最坏扫描全部根候选，为 `O(n)`。
- 一次 `bpPartition()` 只复制一次 `OneTree`，后续 prefix forbid 全部原地更新。

在本机 Release 构建、`fri26.tsp`、单进程两次运行中：

| 版本 | wall time |
|---|---:|
| 重构前 | 5.33 s、5.23 s，平均 5.28 s |
| 重构后 | 5.19 s、4.76 s，平均 4.98 s |

该样本平均 wall time 下降约 5.8%。运行时间会受机器状态和同权 MST 导致的搜索顺序影响，
因此这里作为本地效果记录，不作为稳定性能承诺。

## 测试与验证

新增 `tests/TspSolverTests.cpp`，直接将每一步增量结果与移除全部累计 forbidden 边后的完整
`computeOneTree()` 结果对拍。覆盖：

- 非根 MST 边的唯一替代边；
- 非根 MST 边无替代边；
- 根边改取第三轻合法边；
- 根边无替代边；
- 禁用非树边；
- 连续 prefix forbid；
- 相同权重的替代边；
- 同时存在 forced 根边和 forced 非根边；
- 已有一条 forced 根边时禁用另一条可选根边；
- 多条 forced 非根边及 `forced_degree == 2` 的候选过滤；
- 固定随机种子的 100 组完整图连续 forbid；
- 固定随机种子的 50 组稀疏、重复权重图连续 forbid。

已执行：

```text
cmake -S . -B build -DBUILD_TESTING=ON
cmake --build build --parallel 2
ctest --test-dir build --output-on-failure
```

结果：`1/1` CTest 通过。

其他验证：

- `examples/batch.txt` 的 19 个完整/稀疏小实例全部求解成功；
- 使用独立 Held-Karp 脚本校验上述 19 个实例：`ok=19, failed=0`；
- AddressSanitizer + UndefinedBehaviorSanitizer 构建下，CTest 和 19 实例批次均通过；
- `fri26.tsp` 重构前后都得到最优值 `937`。
