# HKMST replacement-cover 实验与回退记录

## 2026-07-26 当前分支状态

`HKMST` 当前恢复为原顺序 degree-BP，不再调用最大收益
replacement-cover。回退删除了 cover 候选、批量基本割、独立 delta
排序、精确 prefix 验证及其 fallback replacement 缓存；以下优化继续保留：

- 固定 Held-Karp 顶点势及相应安全数值下界；
- 原 BP 的 singleton 强制传播；
- 动态 1-tree replacement 与可逆 prefix 重放；
- 65–128 点 MST 分量的双 64-bit word 快路径；
- 有界直接 replacement 扫描及完整位图回退；
- 64、65、128、129 点边界回归。

回退前后均基于提交 `2d44b5a` 的其余代码。Release 构建使用
AppleClang 17 和 `--exact-max-n 130`；短实例交错运行 25 轮，
`eil101` 运行 7 轮，另对四实例整套交错运行 5 轮：

| 实例 | cover 开启 | cover 移除 | 移除后变化 |
|---|---:|---:|---:|
| st70 | 0.2713 s | 0.2733 s | +0.76% |
| eil76 | 0.2716 s | 0.2741 s | +0.89% |
| rat99 | 0.1326 s | 0.1327 s | +0.11% |
| eil101 | 3.0275 s | 3.0831 s | +1.84% |
| 四实例整套 | 3.6582 s | 3.7239 s | +1.80% |

两版在四个实例上的最优值、created、expanded、bound prune 和
infeasible prune 逐项一致。回退约损失 1.8% wall time，但不改变这些实例的
搜索树规模。当前分支以回退版本为准；下文保留 2026-07-24 cover 实验，
仅作为历史实现与性能记录。

## 2026-07-24 18:23 CST 实验快照

## 快照信息

- 分支：`HKMST`
- 记录时间：2026-07-24 18:23 CST
- 基线提交：`2825520`（`HKMST 2026-07-24 14:31 CST: preserve reduced-cost and BP hot-path gains`）
- 内容：最大收益 replacement-cover B 集合、动态 MST replacement 热路径优化及边界回归
- 历史状态：本文曾与对应源码、测试一起提交；cover 已于 2026-07-26
  从当前 `HKMST` 源码移除

## 结果摘要

计时使用 Release 构建，按三种实现交错执行。st70、eil76、rat99 各运行 15 次，
eil101 运行 3 次，表中为中位数。`cover` 表示只加入最大收益 B 集合方案，
`final` 表示再加入本快照中的 MST replacement 热路径优化。

| 实例 | 已推送 HKMST | cover | final | final 相对基线 |
|---|---:|---:|---:|---:|
| st70 | 0.317173 s | 0.314072 s | 0.262142 s | -17.35% |
| eil76 | 0.330145 s | 0.328111 s | 0.263757 s | -20.11% |
| rat99 | 0.149119 s | 0.144441 s | 0.131007 s | -12.15% |
| eil101 | 4.136522 s | 4.125222 s | 3.037010 s | -26.58% |

三种实现得到完全相同的最优值、创建节点数和展开节点数：

| 实例 | 最优值 | 创建节点 | 展开节点 |
|---|---:|---:|---:|
| st70 | 675 | 176651 | 121714 |
| eil76 | 538 | 162489 | 121189 |
| rat99 | 1211 | 49944 | 37385 |
| eil101 | 629 | 2210554 | 1630895 |

这说明当前主要收益来自每个搜索节点的 replacement 查询加速，而不是改变搜索树。
最大收益 B 集合方案单独可带来约 0.27%–3.14% 的收益。

## 当前 BP 与 B 集合计算流程

### 1. 进入 replacement-cover 快路径

`bpPartition()` 先调用 `tryBestReplacementCover()`；快路径不能形成经过精确验证的 B 集合时，
完整回退到原有顺序 BP，不改变原算法的完备性。

快路径保留 degree-BP 的分支顶点语义：

1. 在当前 1-tree 中选择度数最大的违规顶点，要求度数大于 2。
2. 根顶点 0 的边不属于内部 MST，遇到根顶点或首条回退边为根边时直接走原 BP。
3. 收集该顶点关联、尚未 forced/forbidden 的内部树边作为 cover 候选。

### 2. 预计算每条树边的最佳 replacement

先对原 degree-BP 会选择的第一条边做单边 replacement 查询：

1. 临时禁止该树边并从 active candidate bitset 中移除。
2. 在 `T-e` 的 fundamental cut 上取得当前允许图中最轻 replacement。
3. 计算保守收益
   `safe_delta = max(0, replacement_weight - tree_edge_weight - roundoff_guard)`。
4. 回滚候选状态，并缓存 replacement rank。

若该边单独已经使下界达到 incumbent，直接返回单元素 B 集合。若首条边收益为 0，或离闭合
incumbent gap 仍较远，则停止 cover 尝试，避免批量预计算成本超过收益。

候选多于两条时，以违规顶点为根对 MST 做一次遍历，同时编号各相邻子树。每个子树中所有顶点
的 incident candidate 位图进行异或后，内部边出现两次并抵消，结果就是对应树边的
fundamental cut。随后在全局权重有序的 active candidate 位图中，为每个 cut 取得第一条合法
跨割边。验证构建会用原单边查询逐条核对批量结果。

### 3. 最大化收益并生成 B 集合

1. 丢弃 `safe_delta == 0` 的候选。
2. 按 `safe_delta` 降序排列；等收益时按树边权重和端点稳定排序。
3. 从最大收益开始累加，选择能使
   `current_tree_cost + sum(safe_delta)` 达到 incumbent 的最短前缀。
4. 单元素前缀转成 replacement-cost fixing，由现有 `|B|=1` 传播处理。

因此当前 B 集合确实是“以预计算 replacement delta 为收益，优先选择最大收益边”的方案。

### 4. 精确 prefix 验证

预计算 delta 只用于挑选候选集合，不直接作为最终剪枝证明。选出的边仍按顺序真实执行 forbid：

1. 第一条边复用预计算 replacement。
2. 后续边基于已经变化的 prefix tree 重新执行动态 replacement 查询。
3. 每步保存可重放的 replacement；增量更新失败时允许完整重建。
4. 只有 prefix 最终不可行、形成 tour 或下界达到 incumbent 时才接受该 B 集合。
5. 否则完整回滚，并回退原顺序 BP。

搜索阶段继续枚举互斥子节点：

```text
force B[0]
forbid B[0] + force B[1]
...
forbid B[0..k-2] + force B[k-1]
```

全部 B 边均 forbidden 的剩余分支已经由上述精确 prefix 验证证明不能改善 incumbent。

## 本快照保留的效率优化

### 65–128 点双 word MST 分量

动态 MST 邻接矩阵每行恰好两个 64-bit word 时，使用固定大小的
`seen/frontier` 双向扩展：

- 避免通用 epoch 标记数组写入；
- 避免动态 vector push；
- 仍然从被删边两端交替扩展，选择先完成的小分量；
- n 小于等于 64 的单 word 路径和 n 大于 128 的通用路径保持独立。

### 有界直接 replacement 扫描

对 65–128 点的全局候选查询，先从被删树边权重的 `lower_bound` 开始扫描 active candidate：

- 使用双 word component mask 做 O(1) 跨割判断；
- 扫描预算为 `clamp(component_size * candidate_word_count / 8, 32, 256)`；
- 命中时直接返回权重序中的第一条跨割边；
- 预算耗尽后回退到完整 incident 位图 XOR，不损失正确性。

active bitset 是候选合法性的权威状态，forced、forbidden 和局部过滤都会同步清除 active bit。
因此直接扫描不再重复访问三份 edge-id 状态数组；验证构建会检查该不变量。

## 正确性验证

- Release CTest：4/4 通过。
- ASan + UBSan CTest：4/4 通过。
- 四个策略变体 `tsp_bb_variants` 全部编译通过。
- `TSP_VERIFY_INCREMENTAL_STATE=1` 下完成 rat99 全搜索：
  - 最优值 1211；
  - 创建节点 49944；
  - 展开节点 37385；
  - 每次增量 1-tree、replacement-cover prefix 和完整重建一致。
- 新增 n=64、65、128、129 边界回归：
  - 构造确定性的路径 MST；
  - 删除跨 64-bit word 边界的树边；
  - 比较增量 replacement 与完整 constrained MST 重建。
- `git diff --check` 通过。

常用复现命令：

```sh
cmake -S . -B /private/tmp/tsp-hkmst-cover-final-build \
  -DCMAKE_BUILD_TYPE=Release
cmake --build /private/tmp/tsp-hkmst-cover-final-build -j8
ctest --test-dir /private/tmp/tsp-hkmst-cover-final-build --output-on-failure
cmake --build /private/tmp/tsp-hkmst-cover-final-build \
  --target tsp_bb_variants -j8
```

## 已尝试但回退的方案

### 稀疏 incident word 索引

为每个顶点预存非零 incident word 下标，但全局候选按权重排序后，关联边通常分散在大量 word
中。额外的间接寻址破坏顺序内存访问，实测退化约 1%–20%，已完整回退。

### 扩大直接扫描预算

- 上限从 256 增至 512：仅约 0.5% 波动，没有稳定证据。
- 预算翻倍且上限增至 1024：四个实例退化约 0.1%–0.8%。

最终恢复为上限 256，避免为计时噪声保留参数变化。

## 后续热点

优化后 eil101 的 2 秒采样中：

- `findMstReplacement()`：446 个栈顶样本；
- `markMstComponentWithoutEdge()`：432 个栈顶样本；
- 两者合计约占 2000 个样本的 44%。

下一阶段若继续优化，应优先研究动态 MST 的低度邻接遍历或可回滚的 cut 数据，而不是继续扩大
候选扫描预算。此类修改比本快照更侵入，需继续保留完整重建预言机和边界回归。
