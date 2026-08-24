# NEWHKMST 节点势更新 Prim 实验

## 结论

`updateNodePotentialBound` 的内部受约束 MST 已从“每轮重新收集、排序并执行
Kruskal”改为“每次势更新预构建一次 active 邻接，随后每轮执行 Prim”。
九实例推荐配置下，展开节点保持 201,129；本机 Release 交替 A/B 五次的整批
wall time 中位数由 1.40 s 降至 1.13 s（-19.29%）。五次势更新时间合计的
中位数由 0.562324 s 降至 0.295674 s（-47.42%）。

## 实现边界

该改动只作用于节点势试探，不改变正式 epoch 的 `computeOneTree`、动态 MST
replacement、BP 分支或剪枝判定：

1. 在一次有限轮势上升开始时，用 DSU 收缩所有非根 forced 边；
2. active、forced 和 forbidden 状态只过滤一次，构建双向紧凑邻接；
3. 每组新势在 forced 分量图上执行 Prim；forced 边成本和度数单独计入；
4. 根顶点仍按原规则选择两条边；
5. 下界继续扣除 `2*sum(pi)` 和同口径 roundoff guard。

预处理仍需一次 O(n^2) 状态扫描；每轮内部 MST 从候选边排序变为
O(m_active + component_count^2) 的 Prim。较深节点的 active candidate 已明显
收缩，因此不能直接扫描完整距离矩阵：第一版矩阵 Prim 在四个主要实例上
反而变慢，改成预过滤邻接后才取得稳定收益。

## 九实例 A/B

配置为 `subtree-adaptive`、gap 2%、层距 2、16 轮、预算 5000。基线为修改前
同一 NEWHKMST 源码的 Release 构建；两组都在同一台 arm64 macOS 机器运行。

| 指标 | Kruskal 基线 | 邻接 Prim | 变化 |
|---|---:|---:|---:|
| wall time 五次中位数 | 1.40 s | 1.13 s | -19.29% |
| 势更新时间合计五次中位数 | 0.562324 s | 0.295674 s | -47.42% |
| created | 292,937 | 292,940 | +3 |
| expanded | 201,129 | 201,129 | 0 |
| attempts | 3,441 | 3,441 | 0 |
| direct-pruned | 2,802 | 2,802 | 0 |
| rebuilt | 533 | 533 | 0 |

Prim 与 Kruskal 在等权边上的确定性选树顺序可以不同，st70 因此多创建 3 个
尚未展开的节点，并多执行 1 次势迭代；所有实例的最优值和展开节点数不变。

四个主要耗时实例的一次代表性 `potential_update_seconds`：

| 实例 | Kruskal | 邻接 Prim | 变化 |
|---|---:|---:|---:|
| st70 | 0.185911 s | 0.093831 s | -49.53% |
| eil76 | 0.053542 s | 0.034943 s | -34.74% |
| rat99 | 0.068977 s | 0.038700 s | -43.89% |
| eil101 | 0.247194 s | 0.127781 s | -48.31% |

## 正确性验证

- CTest 4/4 通过；
- 测试构建启用 `TSP_VERIFY_INCREMENTAL_STATE`；
- 随机完整图和稀疏图的节点势更新继续与穷举最优值一致；
- 持久 epoch 的 Prim 上升证书继续与正式 Kruskal 重建下界一致；
- 新增 forced 内部连通分量和 forced 根边下，Prim 与受约束 Kruskal 下界的
  直接对照测试；
- 九个经典实例均返回原已知最优值。

当前结果支持在 NEWHKMST 中保留邻接 Prim，但九实例仍不足以证明所有稀疏度、
规模和势更新轮数下都稳定占优。后续可根据 `m_active`、分量数和预计迭代数，
再评估是否需要 Prim/Kruskal 双路径选择。
