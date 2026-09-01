# PHKMST 当前树 forbid-delta 分支实验

## 目标

验证 BP 是否可以不使用静态 root α，而按“禁止当前 1-tree 树边后的即时下界
增量”选择分支边。实验选项为：

```text
--branch-edge-order forbid-delta-desc
--branch-edge-order forbid-delta-asc
```

对当前树边 `e` 定义：

```text
delta_forbid(e) = replacement_weight(e) - tree_edge_weight(e)
```

无合法 replacement 时记为正无穷。降序优先测试 forbid 后下界增量最大的
边；升序仅用于隔离方向。

## 与增量 1-tree 的关系

该实验不会为每个候选边完整重建 1-tree，也不会在评分时交换工作树边：

1. 仅考察当前最高度违规顶点关联的未决 1-tree 树边；
2. 对内部 MST 边临时将候选 active 位清零；
3. 复用 `findMstReplacement` 查询当前 fundamental cut 的最轻跨割边；
4. 对根边复用 `root_candidates_sorted_` 查找下一条合法根边；
5. 立即回滚 candidate checkpoint；
6. 只有最终选中的边才进入原有 `updateOneTreeAfterForbid` 增量交换。

因此评分查询和正式增量更新使用同一候选状态与 replacement 语义，但评分会
为同一分支点的多个候选树边重复执行 cut 查询。这是额外成本来源。

## 九实例结果

实例为 dantzig42、att48、bayg29、bays29、fri26、st70、eil76、rat99 和
eil101。Release 构建在同一二进制内交错运行。

节点势配置：`subtree-adaptive`、gap 2.05%、epoch 层距 1、最多 16 轮、
预算 5000。wall 为七次中位数。

| 分支顺序 | wall | created | expanded | 势尝试 | 势迭代 |
|---|---:|---:|---:|---:|---:|
| weight | **0.869000 s** | 161,702 | **100,660** | 2,550 | 19,984 |
| forbid-delta-desc | 1.033065 s | **147,780** | 100,708 | 2,631 | 21,008 |

降序使 created 减少 13,922（-8.61%），但 expanded 几乎不变（+0.05%），
wall 反而增加 18.88%。减少的主要是未展开节点，无法抵消每次 BP 选择前的
多次 replacement 查询。

升序的一次代表运行已经扩大到 created 245,691、expanded 121,354，明显不如
降序，因此不再做重复计时。

## 关闭节点势更新的隔离实验

关闭搜索节点势更新后交错运行三次：

| 分支顺序 | wall 中位数 | created | expanded |
|---|---:|---:|---:|
| weight | **3.706776 s** | **2,600,206** | **1,911,618** |
| forbid-delta-desc | 8.225706 s | 3,607,525 | 2,878,054 |

wall 增加 121.91%，created 增加 38.74%，expanded 增加 50.55%。因此回退不只是
与自适应势 epoch 的交互；把单边 forbid delta 作为主分支分数本身会恶化 BP
搜索树。

升序在该隔离配置下更差：created 20,490,212、expanded 10,829,384，说明较小
forbid delta 不具备可用的主排序信号。

## 原因分析

`delta_forbid` 比 root α 更贴近当前节点，但仍然只评估分支的一侧。当前 BP
对子边的实际枚举顺序是：

```text
force(e1)
forbid(e1), force(e2)
forbid(e1), forbid(e2), force(e3)
...
```

较大的 `delta_forbid(e)` 说明 forbid 一侧可能很强，却没有说明首先进入的
`force(e)` 子树是否小、是否提高下界、是否能传播更多候选删除。实验中经常
出现“forbid 侧很快终止，但 force 侧非常大”的不平衡划分。

此外，一次 BP 选择若有 `d` 条候选树边，基线只对最终选择的边执行一次
replacement；该实验需要先执行约 `d` 次只读 replacement 查询，再对选中边
执行正式增量更新。即使搜索树略微缩小，查询成本仍可能主导 wall time。

## 结论

1. 当前树 forbid delta 可以和增量 1-tree 安全共存，不要求逐候选完整重建；
2. 它比静态 root α 更新鲜，但单独作为主排序仍不够；
3. 降序在开启势更新时减少 created，却没有减少 expanded，最终仍回退；
4. 关闭势更新后搜索树显著扩大，否定了直接替换默认 weight 排序；
5. 默认策略继续使用 `--branch-edge-order weight`。

## 最大度热点 + forbid 增长实验

为了消除原策略在多个同度热点之间按顶点编号选择的偏置，增加：

```text
--branch-edge-order forbid-degree-desc
```

每轮 BP 的选择顺序是：

1. 求当前 1-tree 中大于 2 的最大度数；
2. 收集所有至少连接一个该最大度顶点的未决树边；
3. 对候选计算当前 forbid replacement delta；
4. 按 `(forbid delta, 两端超度覆盖, -adjusted weight, 端点)` 选择。

其中两端超度覆盖为：

```text
max(0, degree[u] - 2) + max(0, degree[v] - 2)
```

度数是候选集合的硬门槛，不与有权值单位的 forbid delta 直接相加；覆盖量
只在 forbid delta 相同时打破平局。

### 开启 subtree-adaptive 势更新

同一九实例交错运行七次：

| 分支顺序 | wall 中位数 | created | expanded | 势尝试 | 势迭代 |
|---|---:|---:|---:|---:|---:|
| weight | **0.87 s** | **161,702** | **100,660** | **2,550** | **19,984** |
| forbid-delta-desc | 1.03 s | 147,780 | 100,708 | 2,631 | 21,008 |
| forbid-degree-desc | 5.37 s | 673,197 | 522,151 | 9,836 | 97,376 |

新策略并非只在单个异常实例上回退：`st70`、`eil76`、`rat99` 和 `eil101`
的搜索树均明显扩大。其中 `st70` 从 weight 的 expanded 240 增至 261,214。
同度热点中最大的 forbid 增长仍然只描述 forbid 一侧；它会把首先访问的
force 子树和后续 subtree 势 epoch 引向更差的搜索顺序。

### 关闭节点势更新

三次交错运行：

| 分支顺序 | wall 中位数 | created | expanded |
|---|---:|---:|---:|
| weight | **3.67 s** | 2,600,206 | 1,911,618 |
| forbid-delta-desc | 8.23 s | 3,607,525 | 2,878,054 |
| forbid-degree-desc | 4.72 s | **724,445** | **579,752** |

与 weight 相比，新策略使 created 减少 72.14%、expanded 减少 69.67%，说明
同时考虑全部最大度热点确实修复了旧单顶点策略的搜索树问题；但每轮需要为
更多候选重复扫描 replacement cut，wall 仍增加约 28.6%。相较旧
forbid-delta-desc，它已经同时显著减少节点和耗时。

该结果表明后续最有价值的方向不是继续修改度权重，而是批量计算同一当前树
中多条树边的 replacement delta，或只对少量热点边查询。如果能降低单节点
评分成本，关闭势更新配置已有足够大的节点收益转化为实际加速。

## 受限双侧 strong branching 后续实验

进一步实现了显式实验选项：

```text
--branch-edge-order strong-top2
```

每次 BP 选边只保留 adjusted-weight 顺序最优的两条边。对每条边分别计算：

```text
gain_forbid = 禁止该边后的当前 replacement delta
gain_force  = 强制该边后的试探 1-tree 下界 - 当前 1-tree 下界
```

评分先最大化 `min(gain_force, gain_forbid)`，再最大化两侧增益的乘积，完全
相同时回退 adjusted-weight/端点顺序。乘积在任一侧为零时显式取零，避免
`0 * infinity` 产生 NaN。

force 试探在 `PartialSol` 副本和局部候选集合中运行，不修改 DFS 的 candidate
bitset、动态 MST、available-degree 缓存或 undo 栈。由于候选边本来就在当前
1-tree 中，如果两个端点的既有 forced degree 都为零，force 后当前树仍是
可行证书，增益严格为零；实现直接返回当前下界，省去完整重建。其他情况则
执行度满/forced 子回路过滤并用 Kruskal 完整构造受约束 1-tree。

### 开启 subtree-adaptive 势更新

同一九实例集合交错运行三次：

| 分支顺序 | wall 中位数 | created | expanded | 势尝试 | 势迭代 |
|---|---:|---:|---:|---:|---:|
| weight | **0.88 s** | **161,702** | **100,660** | **2,550** | **19,984** |
| strong-top2 | 12.01 s | 334,531 | 200,751 | 3,998 | 39,550 |

回退集中在 `st70`：weight 仅 created 690、expanded 240；strong-top2 扩大到
created 156,949、expanded 100,086，跨过 100,000 节点阈值并触发 diversified
incumbent 尝试/根重启。说明少量局部双侧增益较高的选边会改变后续势 epoch
与 incumbent 时机，局部分数并不能预测整棵 BP 子树。

### 关闭节点势更新

| 分支顺序 | wall | created | expanded |
|---|---:|---:|---:|
| weight | **3.64 s** | 2,600,206 | 1,911,618 |
| strong-top2 | 50.71 s | **1,599,592** | **1,143,740** |

strong-top2 在隔离配置下使 created 减少 38.48%、expanded 减少 40.17%，证明
双侧信号可以改善搜索树；但 wall 约为基线的 13.9 倍，完整 force 试算的成本
远大于节点缩减收益。

### 双侧实验结论

1. 双侧试算与当前增量 1-tree 可以安全共存，因为评分状态完全隔离；
2. 精确 force 重建不适合在每次 BP 循环中调用，即使只评估 top-2；
3. 节点减少不必然转化为加速，尤其会与 subtree 势 epoch、incumbent 更新和
   100,000 节点后的 diversified restart 发生强耦合；
4. 若继续优化，应改为浅层采样后累计 pseudo-cost、仅在两候选接近时触发，
   或复用增量删除/替换估计 force 侧，而不是每次完整 Kruskal；
5. 默认策略继续使用 `--branch-edge-order weight`，`strong-top2` 仅供实验。

## 验证

- 新增内部 MST 边和根边 forbid-delta 数值测试；
- 升序、降序、forbid-degree-desc 和 strong-top2 均在小实例上与穷举最优值一致；
- `TSP_VERIFY_INCREMENTAL_STATE` 测试构建通过；
- CTest 4/4 通过；
- 四个历史实验目标均可编译；
- 九个经典实例在所有实验策略下均返回相同已知最优值；
- 当前默认 weight 与改动前冻结二进制的 created/expanded 完全相同；七次
  交错 wall 中位数约为 0.87 s 对 0.86 s，约 1% 波动，未发现可区分的默认
  路径性能变化。
