# CPHKMST 顺序策略探索与回退报告

## 范围与保留标准

本轮按预定顺序探索五项策略，始终保持：

- 精确框架是 branch-and-bound；
- 不使用 cutting plane；
- 下界仍是当前 Held–Karp 势下的 constrained 1-tree；
- 原 9 例和训练外 6 例的精确最优值必须逐项一致；
- 只有两组总体 wall/搜索规模均无实质回退，且训练外难例不爆树，
  才保留策略。

统一参数为 P32：根/节点 Polyak，`branch-edge-order=weight`，
`subtree-adaptive`，depth 1，gap 2%，32 轮，无预算上限，保留当前
`sibling warm=0.25`。基线为 `2346a61`。wall 是同一轮批处理中各实例
`instance_wall_seconds` 之和；搜索计数是确定值。

## 结果总览

| 顺序 | 策略 | 原 9 例 | 训练外 6 例 | 决策 |
|---:|---|---|---|---|
| 1 | degree-clause branching | created `1078→7312` | created `4875→7426` | 回退 |
| 2 | 父节点局部 warm memory | created `1078→1828` | created `4875→6620` | 回退 |
| 3 | reliability pseudo-cost | created `1078→1350` | created `4875→5919` | 回退 |
| 4 | exact component-pair pricing | 结构不变，wall `+12.5%` | 结构不变，wall `+26.8%` | 回退 |
| 5 | 深层 degree-2 传播 | created `1078→991` | created `4875→5358` | 回退 |

所有原型都通过 CTest 5/5，两组 cost sum 始终为 `18947` 和 `108365`。
最终求解器源码已回到 `2346a61` 行为，没有保留任何无效策略。

## 1. degree-clause branching

对当前 1-tree 的超度顶点，设剩余度数槽位为 `r=2-forced_degree[v]`，
取 `r+1` 条未决树边，枚举“前缀 force + 首条 forbid”的最多三个子分支。
全 force 余项由 degree-2 约束排除，因而析取完备且不需要 probe。

| 测试组 | 版本 | wall | created | expanded | 节点势迭代 |
|---|---|---:|---:|---:|---:|
| 原 9 例 | 基线 | 0.449903 s | 1,078 | 300 | 12,585 |
| 原 9 例 | degree clause | 1.915151 s | 7,312 | 2,611 | 111,318 |
| 训练外 6 例 | 基线 | 1.067257 s | 4,875 | 1,684 | 73,318 |
| 训练外 6 例 | degree clause | 1.669720 s | 7,426 | 2,790 | 117,597 |

结论：当前 BP 的 forbid-right-chain 已在构造分支时用 replacement 下界证明
最终 residual 无需搜索。固定 2–3 路 degree 析取虽然“分支数小”，却放弃了
原 BP 的提前剪枝，实际搜索树大幅扩张。

## 2. 父节点局部 warm memory

原型按逻辑深度分离势缓存，并在每个父节点的子分支枚举中建立作用域：
只有真正同父兄弟能相互 warm start，后代和堂兄弟不能污染。

| 测试组 | 基线 wall / created | 局部 warm wall / created | expanded | 势迭代 |
|---|---:|---:|---:|---:|
| 原 9 例 | 0.428162 s / 1,078 | 0.538000 s / 1,828 | 300→533 | 12,585→23,323 |
| 训练外 6 例 | 1.066225 s / 4,875 | 1.297190 s / 6,620 | 1,684→2,205 | 73,318→97,200 |

结论：跨层势信息并非只有污染，当前全局传递加 `0.25` 阻尼实际上保留了
大量有用信息。完全局部化的损失大于它消除的负交互。

## 3. reliability pseudo-cost

按边累计搜索已经实际支付的 force/forbid 1-tree 下界增益，两侧样本
均达门槛后，优先最大化较弱一侧的历史平均增益；不执行额外 strong probe。

| 门槛 | 测试组 | 基线 wall / created | pseudo-cost wall / created | expanded | 势迭代 |
|---:|---|---:|---:|---:|---:|
| 2 | 原 9 例 | 0.425148 s / 1,078 | 0.456507 s / 1,350 | 300→331 | 12,585→14,872 |
| 2 | 训练外 6 例 | 1.067355 s / 4,875 | 1.145421 s / 5,919 | 1,684→1,620 | 73,318→77,752 |
| 4 | 原 9 例 | 0.423732 s / 1,078 | 0.474595 s / 1,160 | 300→302 | 12,585→12,927 |
| 4 | 训练外 6 例 | 1.064299 s / 4,875 | 1.464762 s / 7,820 | 1,684→2,101 | 73,318→102,514 |

结论：同一物理边在不同 forced/forbidden 上下文和不同势 epoch 中的两侧增益
不稳定。增加样本门槛只是推迟使用，不能修复这个上下文错配。

## 4. exact component-pair pricing

每轮节点势评估对完整 active 无向边各 pricing 一次，对每对 forced component
仅保留最轻并行边，再在分量矩阵上执行 Prim。这是完整图的精确 MST 压缩，
不会把 LK/近邻候选当作硬边集。搜索结构、最优值和势迭代均与基线完全一致。

| 测试组 | 基线 wall 中位数 | pricing wall 中位数 | 变化 |
|---|---:|---:|---:|
| 原 9 例 | 0.424431 s | 0.477342 s | +12.5% |
| 训练外 6 例 | 1.081445 s | 1.371696 s | +26.8% |

该表是 5 轮 baseline/pricing 顺序交错结果。结论：当前 `n<=130` 规模下，
分量对矩阵的清零和随机写入成本高于省掉的反向弧扫描。真正的稀疏候选若仍需
每轮全边 pricing 才能证明完整性，也不会比当前邻接 Prim 更快。

## 5. 深层 degree-2 可逆传播

完整版在子节点过滤后反复扫描 `available_degree[v]==2`的顶点，将其全部
剩余 active 边强制，并用 force/DSU/candidate 日志逆序回滚。由于传播边未必在
当前 1-tree 中，命中后完整重建 constrained 1-tree。

| 测试组 | 基线 wall / created | 完整传播 wall / created | expanded | 势迭代 |
|---|---:|---:|---:|---:|
| 原 9 例 | 0.426309 s / 1,078 | **0.408339 s / 991** | 300→278 | 12,585→11,584 |
| 训练外 6 例 | 1.059024 s / 4,875 | 1.076698 s / 5,358 | 1,684→1,814 | 73,318→78,837 |

另复筛“已有一条 forced、只剩一条 active”的 unit-degree 版本：原 9 例 created
1,142，训练外 5,320，两组 wall 分别比同轮基线高 9.5% 和 1.8%。

结论：完整传播在训练组有局部价值，但固定会改变后续 1-tree/势 warm 轨迹，
训练外搜索树增大。按预先约定的泛化标准不保留。

## 最终结论与下一批值得做的方向

本轮没有策略同时通过两组。因此保留原 CPHKMST：当前 BP forbid-right-chain、
`branch-edge-order=weight`、全局阻尼 `sibling warm=0.25` 和现有 active-adjacency Prim。

新证据表明，下一步应优先选择“不改搜索轨迹”或“只改善 UB”的方向：

1. **alpha-nearness 增强 LK 候选**：只把根 1-tree alpha 较小边并入 LK 候选，
   完整边图仍由精确搜索使用。CPHKMST 已证明更紧初始 UB 是最大收益来源。
2. **BP replacement 证书缓存**：对当前树边缓存 fundamental-cut 候选前沿，
   只在树边替换或 active 删除影响该 cut 时局部失效。不改分支集和下界。
3. **Prim 热路 SoA/连续内存化**：保留现有选边顺序，只把 `PrimArc`
   的 `to/next/original_weight` 分离成连续数组，以及减少 `relax_component`
   内的间接访问和重复分量查表；要求 created/expanded/迭代逐项不变。
4. **warm cache 接纳门控**：不局部化缓存，而是在兄弟势与父 epoch 势差过大、
   或首次 1-tree 下界比纯父势差时拒绝 warm；拒绝即精确回到现有父势路径。

其中建议下一步先做 **alpha-nearness 增强 LK 候选**，因为它不改精确分支树的
完备性，也最直接攻击本分支已被实验确认的主要收益源——incumbent UB。
