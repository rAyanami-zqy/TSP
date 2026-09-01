# PHKMST 搜索节点 Polyak/Helsgaun 势更新对照

## 结论

已为搜索节点势更新增加独立的运行时策略：

```text
--hk-node-ascent polyak|helsgaun
```

该选项只改变一次 `updateNodePotentialBound` 内部的步长调度，不改变节点更新的
depth/gap/budget 触发条件，也不改变临时证书与 persistent subtree epoch 的
作用范围。默认值继续保持 `polyak`。

在九实例、相同触发配置和相同 16 轮上限下，节点 Helsgaun 将 created 从
292,940 增加到 2,249,346（+667.85%），expanded 从 201,129 增加到
1,649,370（+720.06%）；五轮交错运行的整批 wall time 中位数由 1.08 s
增加到 5.16 s（+377.78%）。因此当前适配不应替换节点 Polyak 默认策略。

## 实现边界

两种节点策略共享以下部分：

- 从当前根势或父 epoch 势 warm start；
- 当前 forced/forbidden/active-candidate 约束；
- forced 分量收缩与预过滤邻接；
- 固定特殊节点 0 的 Prim 1-tree 评估；
- 历史最强安全下界和对应势；
- `shouldPrune` 可剪枝证书早停；
- subtree epoch 的重建、嵌套和回溯恢复。

唯一变量是势更新调度：

| 节点策略 | 步长和方向 |
|---|---|
| `polyak` | `t = alpha*(UB-w)/||g||^2`，直接使用 `g`，连续 4 轮无改善后 `alpha` 减半 |
| `helsgaun` | 初始 `t=1`、初始 period=`n/2`，首 period 改善时倍增步长，随后 period/步长减半，方向为 `0.7g(k)+0.3g(k-1)` |

这里的 Helsgaun 与根 `--hk-ascent helsgaun` 使用同一适配调度，但节点版本从
当前势 warm start，并受单次最大迭代数限制；它不是包含动态特殊节点和
alpha-nearest 上升候选集的完整 LKH 实现。

## 实验配置

Release 构建，九实例清单为 `data/classic/batch-node-ascent.txt`。两组统一使用：

```text
根势：polyak
分支顺序：weight
节点触发：subtree-adaptive
epoch 最小层距：2
相对 gap：2%
单次迭代上限：16
每轮预算：5000
```

命令模板：

```sh
./build/tsp_bb --hk-ascent polyak \
  --hk-node-ascent polyak \
  --branch-edge-order weight \
  --hk-potential-update subtree-adaptive \
  --hk-update-depth 2 --hk-update-gap-ratio 0.02 \
  --hk-update-iterations 16 --hk-update-budget 5000 \
  --batch data/classic/batch-node-ascent.txt
```

Helsgaun 组只把 `--hk-node-ascent polyak` 改为 `helsgaun`。计时采用
Polyak/Helsgaun 交换先后顺序的五轮运行。

## 汇总结果

| 指标 | 节点 Polyak | 节点 Helsgaun | Helsgaun 相对变化 |
|---|---:|---:|---:|
| wall time 中位数 | **1.08 s** | 5.16 s | +377.78% |
| created | **292,940** | 2,249,346 | +667.85% |
| expanded | **201,129** | 1,649,370 | +720.06% |
| 更新尝试 | **3,441** | 11,568 | +236.18% |
| 有效改善 | 3,335 | 8,365 | +150.82% |
| 直接剪枝 | 2,802 | 6,132 | +118.84% |
| epoch 重建 | **533** | 2,233 | +318.95% |
| 势迭代 | **23,619** | 125,718 | +432.27% |
| 代表性势更新时间 | **0.238619 s** | 1.401915 s | +487.51% |

所有实例都返回相同已知最优值。两组的触发谓词相同，但安装的新势会改变
候选排序、1-tree、后续分支路径和再次触发的位置，因此整轮更新尝试次数不必
相同。Helsgaun 的直接剪枝绝对数更大，是因为它访问并尝试了更多节点；按尝试
计，直接剪枝率由 Polyak 的 81.43% 降至 Helsgaun 的 53.01%。

## 逐实例搜索规模

| 实例 | Polyak created/expanded | Helsgaun created/expanded |
|---|---:|---:|
| dantzig42 | 7 / 3 | 7 / 3 |
| att48 | **48 / 24** | 101 / 42 |
| bayg29 | 3 / 3 | 3 / 3 |
| bays29 | **12 / 7** | 16 / 10 |
| fri26 | 1 / 0 | 1 / 0 |
| st70 | 135,216 / **100,254** | **129,551** / 101,474 |
| eil76 | **617 / 224** | 135,910 / 100,653 |
| rat99 | **624 / 248** | 1,365 / 610 |
| eil101 | **156,412 / 100,366** | 1,982,392 / 1,446,575 |

退化主要来自 eil76 和 eil101，说明该调度对不同节点局部约束和 warm-start 势
不够稳健。st70 的 created 略少，但 expanded 仍增加，无法抵消其他实例退化。

## 分析

1. LKH 的 period 调度主要用于从零势做较长的根 Ascent，并改善 alpha-nearness
   候选集；当前节点更新的目标是在 16 轮内跨过精确剪枝阈值。
2. 固定初始步长 1 没有利用节点局部 `UB-w` 和次梯度范数进行尺度归一化；
   Polyak 步长会随每个节点的剩余 gap 自动调整。
3. 0.7/0.3 平滑方向降低振荡，但在短轮次、受约束且从父势 warm start 的环境
   中也会降低对当前 1-tree 度数违规的响应速度。
4. 较弱或不同的 epoch 势会改变候选排序与 BP 搜索树，随后放大为更多节点、
   更新尝试和重建，而不仅是单次势计算变慢。

若继续研究，更合理的候选是保留平滑方向和 period 衰减，但用 Polyak gap
初始化或归一化步长；这将是新的 hybrid 节点策略，而不是论文调度的直接对照。

## 验证

- Release CTest 4/4 通过；
- 完整图和稀疏图的既有穷举对拍继续通过；
- 新增节点 Helsgaun 在临时证书和 subtree epoch 下的小实例穷举验证；
- dantzig42 明确覆盖节点 Helsgaun 更新循环；
- 九个经典实例的两种节点策略均返回相同已知最优值。
