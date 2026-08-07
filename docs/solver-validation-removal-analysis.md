# oneTreeSatisfiesConstraints 移除分析与热力图验证

日期：2026-07-15

## 改动

三个分支的 `src/TspSolver.cpp` 中注释掉所有 `oneTreeSatisfiesConstraints` 调用：

| 分支 | 注释数 | 负责编译的版本 |
|---|---|---|
| `main` | 10 | `tsp_bb_26_07_14_deg`, `tsp_bb_26_07_14_min_edge` |
| `full` | 10 | `tsp_bb_26_07_14_full_deg`, `tsp_bb_26_07_14_full_min_edge` |
| `refactor/07_02-smart-dfs` | 18 | `tsp_bb_26_07_02`, `tsp_bb_26_07_02_full` |

恢复方式：搜索 `已关闭`，取消对应行注释。

## 热力图验证（burma14, n=14）

### 性能对比

| 对比 | 增量版 | full版 | 差距 |
|---|---|---|---|
| deg (07_14) | 36.1ms | 36.2ms | +0.3% |
| min-edge (07_14) | 55.7ms | 45.8ms | **full 快 17.8%** |
| smart DFS (07_02) | 35.2ms | 34.3ms | **full 快 2.5%** |

### 函数占比

| 函数 | deg增量 | deg_full | min增量 | min_full | 07_02增量 | 07_02_full |
|---|---|---|---|---|---|---|
| `computeOneTree` | — | 27.6% | — | 27.4% | — | 27.0% |
| `updateOneTreeAfterCandidateRemoval` | 23.7% | — | 24.6% | — | 22.6% | — |
| `updateOneTreeAfterForbid` | 6.9% | — | 6.7% | — | 6.4% | — |
| Solver self% | 68.3% | 63.1% | 68.2% | 61.9% | 69.3% | 66.0% |
| `oneTreeSatisfiesConstraints` | 0% | 0% | 0% | 0% | 0% | 0% |

## 结论

增量 1-tree 替换不比 Kruskal 快：

1. **入场费等同**：`updateOneTreeAfterForbid` / `updateOneTreeAfterCandidateRemoval` 每次调用都重建 DSU（O(n·α(n))）+ 扫描候选边（O(K)），开销 ≈ 一次完整 Kruskal

2. **Kruskal 有增量没有的优化**：
   - `forced_mst_cost` / `forced_mst_count` 缓存（O(1) 取强制边信息）
   - `mst_edges == n - 2` 提前终止（通常前几条边就凑满 MST）
   - `branch_candidates` 由调用者保证已按权重排序

3. **min-edge 输最多**：min-edge 选边策略从 1-tree 全部未决边中选最轻边，force 分支的 `buildBranchCandidates` 过滤更激进，被删树边更多（k 更大），`updateOneTreeAfterCandidateRemoval` 的 DSU 重建累积开销更大

4. **之前增量版看起来有竞争力**是因为 `oneTreeSatisfiesConstraints` 占 27%+ 的时间——增量版在校验上花的钱比替换逻辑本身还多。去掉校验后，纯算法成本的优劣一目了然

## 演进记录

| 阶段 | 增量版 | full版 | 说明 |
|---|---|---|---|
| 校验全开 | 58.7ms | 61.2ms | 增量稍快（校验占比掩盖了替换成本） |
| 增量路径关 | 50.5ms | 59.8ms | 增量省了部分校验 |
| 全部关 | 34.2ms | 34.8ms | 几乎持平 |
| **当前（DSU，全部关）** | **35-56ms** | **34-46ms** | full 全面反超 |
