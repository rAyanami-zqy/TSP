# CPHKMST 后续优化与分支策略复筛

## 约束

本轮实验保持以下边界：

- 求解框架仍为 branch-and-bound；
- 不引入 cutting plane；
- 下界仍为当前 Held–Karp 势下的受约束 1-tree；
- 工程优化不得改变受约束 1-tree 的选择规则；浮点保护可以更保守，但不能
  抬高下界。分支策略实验只允许改变 `bpPartition` 选择分支边的顺序。

基线为 `CPHKMST` 的 `9d7a961`，参数为：

```text
--hk-ascent polyak
--hk-node-ascent polyak
--branch-edge-order weight
--hk-potential-update subtree-adaptive
--hk-update-depth 1
--hk-update-gap-ratio 0.02
--hk-update-min-gap-ratio 0.0
--hk-update-iterations 32
--hk-update-budget 0
--exact-max-n 130
```

## 1-tree 热路径优化

采样显示，`updateNodePotentialBound` 的多轮受约束 1-tree 评估是主要热点。
其中每轮为浮点舍入保护重新扫描完整距离矩阵，并不参与 MST 或根边选择。

本轮做了两项等价变换：

1. 构造调整权重绝对值上界时，用
   `max(dist) + 2 * max(abs(pi))` 的三角不等式保守上界代替每轮 `O(n^2)`
   全图扫描。保护量只会相同或更保守，不会抬高 1-tree 下界。
2. 节点势评估的根边由“收集后排序”改成线性维护两条最轻合法边；forced
   根边仍优先，权重和端点平局规则保持不变。

9 个 `batch-node-ascent.txt` 实例进行 5 轮 baseline/optimized 交错测试：

| 指标 | baseline 中位数 | optimized 中位数 | 变化 |
|---|---:|---:|---:|
| 总 wall | 0.590209 s | 0.542028 s | -8.2% |
| 节点势更新 | 0.240172 s | 0.189754 s | -21.0% |
| epoch 重建 | 0.079395 s | 0.077573 s | -2.3% |
| nodes created | 1,854 | 1,854 | 不变 |
| nodes expanded | 548 | 548 | 不变 |
| 节点势迭代 | 23,999 | 23,999 | 不变 |

所有实例的状态与最优值逐项一致。CTest 5/5 通过，其中
`tsp_solver_tests` 启用了 `TSP_VERIFY_INCREMENTAL_STATE`，会用完整重建核对增量
1-tree 和新 epoch 证书。

三个未包含在上述 9 实例组中的实例再做 3 轮交错复测，最优值、created 和
expanded 均逐项一致：

| 实例 | baseline 中位 wall | optimized 中位 wall | 变化 |
|---|---:|---:|---:|
| `gr48` | 0.106662 s | 0.092283 s | -13.5% |
| `gr96` | 1.326583 s | 1.137921 s | -14.2% |
| `rd100` | 0.259444 s | 0.236218 s | -9.0% |

`gr96` 的节点势迭代由 80,993 变为 80,992；更保守的浮点 guard 带来极小
证书差异，使内部收敛/停止路径相差一轮。搜索树和最优值不变，且保护量只会
降低数值证书，不会把下界抬高。

## 分支策略复筛

Concorde 风格预处理显著缩小搜索树后，重新测试了仓库已有的分支顺序。
四个较活跃实例（`st70/eil76/rat99/eil101`）的单轮筛选中：

| 分支顺序 | wall | created | expanded |
|---|---:|---:|---:|
| `weight` | 0.497 s | 1,808 | 534 |
| `root-alpha-asc` | **0.329 s** | **974** | 249 |
| `root-alpha-desc` | 0.352 s | 1,698 | **215** |
| `local-excess-cover-weight` | 0.524 s | 2,189 | 531 |
| `forced-degree-weight` | 0.699 s | 3,915 | 904 |
| `root-alpha-global-desc` | 0.640 s | 4,830 | 602 |
| `root-alpha-global-asc` | 0.921 s | 4,411 | 1,252 |
| `max-degree-all-weight` | 1.098 s | 5,724 | 1,688 |
| `weight-desc` | 3.535 s | 22,281 | 2,832 |

在完整 9 实例组进行 5 轮交错测试：

| 分支顺序 | wall 中位数 | created | expanded | 节点势迭代 |
|---|---:|---:|---:|---:|
| `weight` | 0.536899 s | 1,854 | 548 | 23,999 |
| `root-alpha-asc` | **0.385970 s** | **1,018** | 262 | **10,652** |
| `root-alpha-desc` | 0.403103 s | 1,781 | **233** | 12,068 |

但是独立实例复核表明 `root-alpha-asc` 尚不具备稳定泛化性：

| 实例 | `weight` wall / created | `root-alpha-asc` wall / created | 判断 |
|---|---:|---:|---|
| `brazil58` | 0.0293 s / 88 | 0.0260 s / 57 | 正向 |
| `eil51` | 0.0564 s / 686 | 0.0557 s / 680 | 接近 |
| `gr48` | 0.0937 s / 1,724 | 0.0829 s / 1,380 | 正向 |
| `gr96` | 1.128 s / 5,152 | 0.911 s / 4,319 | 正向 |
| `rd100` | **0.255 s / 742** | 1.065 s / 4,710 | 明显退化 |
| `pr76` | >10 s | >10 s | 本轮未完成 |

因此保留 `weight` 为默认值；`root-alpha-asc` 继续作为实验选项，不根据这组
小样本自动启用。其他静态局部分数叠加策略也未超过默认值。

## 后续方向

1. 优先探索 reliability pseudo-cost：记录 force/forbid 子节点实际带来的
   1-tree 下界增益，样本不足时回退 `weight`。它比静态 root alpha 更贴近
   当前节点，但应先只做统计，确认命中率和额外计算成本后再参与排序。
2. epoch 重建中的全边排序已成为相对更大的热点。若继续优化，需要独立
   scratch 缓冲区，不能原位改写已移入父 epoch snapshot 的候选数组。
3. 在更大的训练外实例组上验证 root alpha，重点解释 `rd100` 类反例；在有
   可复现判据前不增加按实例手工阈值。

以上方向均保持 branch-and-bound 与当前 1-tree 下界，不需要割平面。
