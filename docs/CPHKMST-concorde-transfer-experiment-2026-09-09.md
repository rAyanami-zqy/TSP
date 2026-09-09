# CPHKMST：BP 缩支、预处理与 Concorde 可迁移策略

## 约束与结论

本轮保持 branch-and-bound、禁用 cutting plane，并继续只用受约束 1-tree
作为下界。结论如下：

1. 当前 BP 的 `B` 集是被压缩的连续 forbid 右链，不是先完整预探测所有子树；
   直接改为二叉分支只会把同样的残余链移到下一层。
2. 单棵 1-tree 可以批量给出安全的边 fixing 证书，但不能仅凭“边出现在当前
   1-tree 中”构造更小且完备的 `B` 集。
3. Concorde LP 求解器的候选稀疏化、变量消元和 strong branching 依赖 LP
   对偶、精确 pricing 或 cuts，不能在本项目约束下直接移植。
4. Concorde 独立的 `HELDKARP` 求解器会把一个节点结束时的势传给后续兄弟
   节点。将这一 dual warm-start 思路阻尼移植后，两组实验均有明显正收益。

## 当前 BP 实际在做什么

`bpPartition` 依次选择当前 1-tree 的违规关联边 `e0,e1,...`，在同一棵工作树
上增量执行 forbid replacement，直到全部前缀禁止的残余问题不可行、达到
incumbent 下界或已经成为 tour。真正递归的互斥分支为：

```text
force e0
forbid e0, force e1
forbid e0, forbid e1, force e2
...
```

最后的 `forbid e0,...,forbid ek` 残余已经在构造 `B` 时被证明无需搜索。因此
`B` 集把普通二叉 B&B 的 forbid 右链压缩在一个逻辑节点中，并复用每次单边
replacement；它并没有为每个 force 子树预先跑完整下界。

若只计算当前一棵 1-tree，树边本身并不是所有 tour 的必选边，不能安全声称
“任一改进 tour 必须包含这几条树边之一”。能安全使用的是敏感度证书：

- 非树边强制加入时，以树路径可删除的最重边得到 `force_delta`；若
  `LB + force_delta >= UB`，可固定 `x_e=0`；
- 树边禁止时，以跨 cut 的最轻 replacement 得到 `forbid_delta`；若
  `LB + forbid_delta >= UB`，可固定 `x_e=1`。

根节点的 `applyRootReducedCostFixing` 已经实现这两类证书。它们适合进一步移到
少量 subtree epoch 锚点，而不适合在每个节点完整扫描全部边。

## 单树分支策略复筛

四个活跃实例 `st70/eil76/rat99/eil101` 上，直接按当前树的 forbid sensitivity
选择边没有得到更小的总搜索：

| 策略 | wall | created | expanded | 节点势迭代 |
|---|---:|---:|---:|---:|
| `weight` | 0.662 s | 1,808 | 534 | 23,556 |
| `forbid-delta-desc` | 3.246 s | 11,231 | 3,530 | 173,320 |
| `forbid-delta-asc` | 3.002 s | 28,187 | 2,497 | 200,155 |
| `forbid-degree-desc` | 6.696 s | 32,012 | 9,669 | 464,971 |
| `strong-top2` | 3.403 s | 12,435 | 3,648 | 182,926 |

`max-degree-max-undecided` 在原 9 例上较好：中位 wall `0.549s -> 0.419s`，
created `1854 -> 1203`。但训练外的 `gr96` 从 5,152 扩大到 21,765 created，
`rd100` 从 742 扩大到 10,749，因此仍保留 `weight` 默认值。

这说明“让 residual forbid 侧尽快抬高下界”不足以决定好分支；force 侧的形状
同样重要。后续应使用有可靠度门槛的双侧历史增益，而不是继续叠加静态分数。

## Concorde 代码对照

### 可迁移：HELDKARP 的跨兄弟势 warm start

`concorde/HELDKARP/heldkarp.c` 是独立的 1-tree branch-and-bound：

- 根节点最多 1000 轮、非根节点最多 10 轮 Held–Karp 上升；
- 选择当前 1-tree 中调整权重最小的未决定边；
- 对该边执行 forbid/force 二元分支；
- `y` 势数组在递归返回时不回滚，因此后访问的兄弟节点从最近节点的势开始。

最后一点不依赖 LP 或 cuts，且任意有限顶点势都必须重新构造 1-tree 后才会
成为有效证书，因而可以安全移植。

当前实现增加 `--hk-sibling-warm-weight`：

```text
initial_pi = (1-w) * parent_epoch_pi + w * last_node_pi
```

默认 `w=0.25`，`w=0` 可关闭。缓存每次根搜索/重启时清空；兄弟势只作为
`updateNodePotentialBound` 的初值，节点仍按自己的 forced/forbidden/active
约束重新计算 1-tree。直接照搬 Concorde 的 `w=1` 在 `rd100` 上退化，阻尼可
避免持久 epoch 与远处兄弟节点的势相互污染。

### 不可直接迁移：LP strong branching 与消元

`concorde/TSP/branch.c` 先从分数 LP 边中筛候选，再用有限 dual-simplex
strong-branch 迭代估计两侧，最终分数近似为 `100*min(down,up)+max(down,up)`。
这里的候选值、warm start 和两侧 penalty 都来自 LP，当前 1-tree 没有对应的
分数 `x_e`。

`concorde/TSP/tsp_lp.c` 的 `CCtsp_eliminate_variables` 以及 `ex_price.c` 的
exact pricing 使用节点、clique 和 cut 对偶量。直接把 Concorde 的稀疏候选图
当作精确边集会丢失最优 tour；除非像 Concorde 一样提供完整 pricing 证明，
否则只能用于 LK/replacement 的查询提示，不能作为硬删除。

### 可迁移但只改善上界：候选边生成

Concorde 的 nearest/quad-nearest/Delaunay/Linkern 候选生成可以用于更快找到
incumbent，但不能替代精确候选图。当前项目已有 k-nearest LK 候选及 tour 边
提示；下一步可把根 1-tree 的 alpha-nearness 边并入 LK 候选，再保留完整图作
精确 replacement fallback。

## 阻尼兄弟势实验

统一配置仍为 P32：节点 Polyak、`subtree-adaptive`、depth 1、gap 2%、32 轮、
无限预算、`branch-edge-order=weight`。每组做 5 轮 baseline/warm25 交错测试。

| 测试组 | 版本 | wall 中位数 | created | expanded | 节点势迭代 |
|---|---|---:|---:|---:|---:|
| 原 9 例 | baseline | 0.551095 s | 1,854 | 548 | 23,999 |
| 原 9 例 | warm25 | **0.426550 s** | **1,078** | **300** | **12,585** |
| 训练外 6 例 | baseline | 1.700147 s | 8,403 | 2,842 | 125,638 |
| 训练外 6 例 | warm25 | **1.118402 s** | **4,875** | **1,684** | **73,318** |

对应 wall 改善约为 22.6% 和 34.2%，所有实例最优值一致。训练外单轮拆分中：

| 实例 | baseline wall / created | warm25 wall / created |
|---|---:|---:|
| `brazil58` | 0.0359 s / 88 | 0.0275 s / 66 |
| `eil51` | 0.0583 s / 686 | 0.0538 s / 584 |
| `gr48` | 0.0911 s / 1,724 | 0.0560 s / 816 |
| `gr96` | 1.1351 s / 5,152 | 0.7395 s / 3,226 |
| `rd100` | 0.2357 s / 742 | 0.1245 s / 172 |
| `lin105` | 0.0871 s / 11 | 0.0774 s / 11 |

权重筛选中，`w=0.125/0.25/0.5` 的原 9 例 created 分别为
`1720/1078/1154`，训练外 6 例分别为 `7287/4875/5989`，因此本轮选择
`0.25`。`pr76` 的 baseline 和 warm25 均未在 30 秒内完成，本轮不对该实例
声称收益。

CTest 5/5 通过；其中 `tsp_solver_tests` 启用增量状态全量重建校验，并包含随机
完整/稀疏图与穷举最优值对照。另在训练外 6 例上将
`--hk-sibling-warm-weight` 设为 `0`，其 cost、created、expanded、各类剪枝数
和节点势迭代数均与改动前二进制逐项一致，仅计时噪声不同。

## 下一步优先级

1. **subtree epoch 局部 fixing**：只在成功安装新势的锚点批量计算一次
   force/forbid sensitivity，并在整个该子树复用。它最接近“单次下界计算后
   缩小 B 集”，也能摊薄全边扫描成本。
2. **节点 degree-2 可逆传播**：force/forbid 过滤后，若某顶点只剩两条可行
   关联边，立即强制并继续队列传播。根预处理实测几乎没有新增机会，但深层
   节点更可能触发；实现时需要完整 undo 日志。
3. **reliability pseudo-cost**：记录边在历史 force/forbid 子节点的实际 1-tree
   增益；样本不足时使用 `weight`，可靠后以偏重较弱一侧的分数排序。这是
   Concorde strong branching 思想在无 LP 条件下更合适的替代。
4. **alpha-nearness LK 候选**：仅改善 incumbent 和 replacement 提示，完整图
   fallback 保证精确性。

以上方案都不需要 cutting plane，也不改变当前 1-tree 下界体系。
