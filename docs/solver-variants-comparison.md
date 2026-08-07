# Solver 版本实现差异汇总

生成日期：2026-07-15，基于 `main` 分支 `solver/` 目录下的归档二进制。

## 总览

| 目录 | CMake 目标 | 搜索 | 选边策略 | 1-tree | 源码提交 |
|---|---|---|---|---|---|
| `tsp_bb_26_07_02` | `tsp_bb_26_07_02` | smart DFS | smart 三级回退 | 增量 | `997e472e` |
| `tsp_bb_26_07_02_full` | `tsp_bb_26_07_02_full` | smart DFS | smart 三级回退 | 完整重建 | `997e472e` |
| `tsp_bb_26_07_14_deg` | `tsp_bb_26_07_14_deg` | BP | degree | 增量 | `3131435` |
| `tsp_bb_26_07_14_full_deg` | `tsp_bb_26_07_14_full_deg` | BP | degree | 完整重建 | `3131435` |
| `tsp_bb_26_07_14_min_edge` | `tsp_bb_26_07_14_min_edge` | BP | min-edge | 增量 | `3131435` |
| `tsp_bb_26_07_14_full_min_edge` | `tsp_bb_26_07_14_full_min_edge` | BP | min-edge | 完整重建 | `3131435` |

---

## 三大维度差异

### 1. 搜索算法

| 算法 | 使用的版本 | 说明 |
|---|---|---|
| **BP（Best Projection）** | 全部 `07_14` 系列 | 执行 BP/A-B 划分，每个节点可同时 force/forbid 多条边，按投影代价排序生成子节点 |
| **smart DFS** | `07_02`, `07_02_full` | 不执行 BP 划分。每个节点只选一条边，按 `force → forbid` 顺序生成两个互斥子节点做深度优先搜索 |

编译宏：`TSP_DISABLE_BP=1` 禁用 BP，回退到 smart DFS。

### 2. 分支选边策略

| 策略 | 使用的版本 | 选边逻辑 |
|---|---|---|
| **degree** | `07_14_deg`, `07_14_full_deg` | 从 1-tree 中选择度数 > 2 的违规顶点，取该顶点关联的最轻未决树边 |
| **min-edge** | `07_14_min_edge`, `07_14_full_min_edge` | 不限定度数违规顶点，直接从 1-tree 全部未决边中选最轻边 |
| **smart 三级回退** | `07_02`, `07_02_full` | ① 选 1-tree 中度数最大且 degree > 2 的顶点，取其最轻未决树边；② 无满足条件的顶点时回退到全部 1-tree 未决边中最轻边；③ 仍无法选取时回退到全部候选边中最轻边 |

编译宏：`TSP_BRANCH_STRATEGY_MIN_EDGE=1` 启用 min-edge；均不定义时默认 degree。smart 由 `TSP_DISABLE_BP=1` 随搜索算法切换触发。

### 3. 1-tree 构造方式

| 方式 | 使用的版本 | 说明 |
|---|---|---|
| **增量（incremental）** | `07_02`, `07_14_deg`, `07_14_min_edge` | forbid 子节点用 replacement edge 更新被禁边；force 子节点对候选过滤移除的树边做 replacement 更新。含安全回退——校验失败时完整重建 |
| **完整重建（full）** | `07_02_full`, `07_14_full_deg`, `07_14_full_min_edge` | 每个子节点依据当前 forced/forbidden 约束完整执行受约束 Kruskal 重建，不复用父节点结构 |

编译宏：`TSP_DISABLE_INCREMENTAL_ONETREE=1` 禁用增量。

---

## 编译宏矩阵

```
                            TSP_DISABLE_BP  DISABLE_INCREMENTAL  BRANCH_MIN_EDGE
tsp_bb_26_07_02             ✓               —                    —
tsp_bb_26_07_02_full        ✓               ✓                    —
tsp_bb_26_07_14_deg         —               —                    —
tsp_bb_26_07_14_full_deg    —               ✓                    —
tsp_bb_26_07_14_min_edge    —               —                    ✓
tsp_bb_26_07_14_full_min_edge —             ✓                    ✓
```

---

## 源码世代

```
提交 3131435 (main, 07-14 BP correctness fix)
├── tsp_bb_26_07_14_deg           ← degree + incremental
├── tsp_bb_26_07_14_full_deg      ← degree + full
├── tsp_bb_26_07_14_min_edge      ← min-edge + incremental
└── tsp_bb_26_07_14_full_min_edge ← min-edge + full

提交 997e472e (refactor/07_02-smart-dfs)
├── tsp_bb_26_07_02               ← smart DFS + incremental
└── tsp_bb_26_07_02_full          ← smart DFS + full
```

07_02 rebuilt 版基于当前源码重构，保留了 07_14 的数值安全剪枝、DSU 回滚、LK 上界、约束校验和错误回退修复，仅恢复旧 smart 分支语义，不是历史归档二进制的逐字节复刻。

---

## 实验对照关系

### 搜索算法对比
- `07_02` vs `07_14_deg`：smart DFS vs BP（同 degree 系选边 + 增量）

### 1-tree 构造对比（增量 vs 完整重建）
- `07_02` vs `07_02_full`（smart DFS 下）
- `07_14_deg` vs `07_14_full_deg`（degree + BP 下）
- `07_14_min_edge` vs `07_14_full_min_edge`（min-edge + BP 下）

### 选边策略对比（degree vs min-edge）
- `07_14_deg` vs `07_14_min_edge`（增量下）
- `07_14_full_deg` vs `07_14_full_min_edge`（完整重建下）
