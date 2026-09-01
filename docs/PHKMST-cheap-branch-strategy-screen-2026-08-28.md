# PHKMST 低成本分支策略筛选实验

## 目标与口径

在 root-frequency、current-forbid-delta 和 strong-top2 都未稳定超过默认策略后，
继续筛选只读取当前 1-tree、forced 度和 active 候选缓存的低成本规则。所有
策略只改变 `bpPartition()` 的选边顺序，不修改下界、可行域、BP 覆盖关系或
剪枝条件。

主要对照为：

```text
--branch-edge-order weight
```

九实例集合包含 dantzig42、att48、bayg29、bays29、fri26、st70、eil76、
rat99 和 eil101。扩展验证加入 `gr48`，因为它能暴露九实例汇总没有显示的
反向热点。

## 初筛策略

| CLI | 策略 |
|---|---|
| `weight-desc` | 默认最大度顶点内优先最重树边 |
| `max-degree-all-weight` | 合并所有并列最大度顶点，再选最轻边 |
| `excess-cover-weight` | 全部违规点关联边中，最大化两端超度覆盖，再选最轻边 |
| `local-excess-cover-weight` | 保持默认分支顶点，只优先另一端超度较大的边 |
| `max-degree-excess-weight` | 最大度热点集合内最大化两端超度覆盖 |
| `propagation-weight` | 默认顶点内优先 forced 度高、active 候选余量小的边 |
| `forced-degree-weight` | 上一策略的保守版，只使用 forced 度 |
| `max-degree-min-undecided` | 并列最大度时优先未决树边最少的顶点 |
| `max-degree-max-undecided` | 并列最大度时优先未决树边最多的顶点 |

历史编译策略“从整棵 1-tree 选择最轻未决边”也纳入初筛。

## subtree-adaptive 九实例初筛

节点势参数为 gap 2.05%、epoch 层距 1、单次最多 16 轮、预算 5000。

| 分支策略 | created | expanded | 相对默认 expanded |
|---|---:|---:|---:|
| weight | 161,702 | 100,660 | — |
| 全 1-tree 最轻边 | 521,095 | 351,780 | +249.5% |
| weight-desc | 404,299 | 200,531 | +99.2% |
| max-degree-all-weight | 293,008 | 201,384 | +100.1% |
| excess-cover-weight | 26,092 | 15,544 | **-84.6%** |
| local-excess-cover-weight | 446,232 | 300,596 | +198.6% |
| max-degree-excess-weight | 25,127 | 14,888 | **-85.2%** |
| propagation-weight | 319,327 | 200,613 | +99.3% |
| forced-degree-weight | 299,884 | 202,326 | +101.0% |
| max-degree-min-undecided | 227,546 | 156,055 | +55.0% |
| max-degree-max-undecided | 436,481 | 300,641 | +198.7% |

`excess-cover-weight` 的七次交错 wall 中位数与默认均为约 `0.84 s`。虽然它
大幅减少节点，但势尝试从 2,550 增至 3,239、势迭代从 19,984 增至 29,543，
当前按 weight 调出的势更新参数抵消了节点收益。

`propagation-weight` 与默认另一次七轮交错的中位数为 `1.32 s` 对 `0.82 s`，
约慢 61%；其节点和势更新次数同时扩大，因此九实例上没有隐藏的 wall 收益。

## 关闭搜索节点势更新

根 Polyak 势保持不变，仅把搜索节点势策略设为 `none`。

| 分支策略 | wall 中位数 | created | expanded |
|---|---:|---:|---:|
| weight（与 excess 交错） | 3.69 s | 2,600,206 | 1,911,618 |
| excess-cover-weight | **2.05 s** | **1,172,888** | **708,671** |
| weight（与 propagation 交错） | 3.72 s | 2,600,206 | 1,911,618 |
| propagation-weight | 2.84 s | 2,255,945 | 1,466,803 |

在这九个实例上，超度覆盖使 created 减少 54.9%、expanded 减少 62.9%、
wall 减少 44.4%；传播策略使 expanded 减少 23.3%、wall 减少约 23.7%。
因此二者都比默认无节点势配置更好，其中超度覆盖收益最大。

## `gr48` 反例

`gr48` 表明九实例结果不能直接升级为默认策略。开启相同 subtree-adaptive
参数时：

| 分支策略 | wall 中位数 | created | expanded |
|---|---:|---:|---:|
| weight | 0.11 s | 85,324 | 60,977 |
| excess-cover-weight | 0.46 s | 716,545 | 430,588 |
| propagation-weight | **0.07 s** | **44,332** | **28,448** |

超度覆盖在 `gr48` 上让 expanded 增加 606.2%；传播策略反而减少 53.4%。但
传播策略在九实例 subtree-adaptive 汇总中又使 expanded 增加 99.3%。关闭
节点势后，`gr48` 的 propagation 也从 90,001 增至 143,601 expanded。

这组互补反例说明：

1. 超度覆盖擅长消除某些实例的度热点，但可能选择 force 子树极大的边；
2. forced/候选余量反映当前传播机会，却会和 subtree 势 epoch 的触发位置
   强耦合；
3. 单一静态优先级无法同时判断两种实例；
4. 只看一个基准集合的总节点数会掩盖热点迁移。

## 结论

- 若运行默认的“根势 + 无搜索节点势”配置，在当前九实例上
  `excess-cover-weight` 明显好于 `weight`，但存在 `gr48` 单实例回退；
- 在当前推荐的 subtree-adaptive 配置中，没有一个新规则稳定好于默认
  `weight`；
- 因此默认策略暂不切换，所有新规则保留为显式实验选项；
- 下一步更合理的方向是可靠性 pseudo-cost：先用默认规则，累计真实 force
  子树和 forbid replacement 的历史收益，仅在样本可信时在
  `weight`、超度覆盖和传播候选之间选择，而不是固定使用某一个分数。

## 验证

- 所有新策略均加入小实例穷举最优值回归；
- CTest 4/4 通过；
- 默认 `weight` 的九实例 created/expanded 保持 161,702/100,660；
- 默认热路径与冻结二进制七次交错 wall 中位数约为 0.82 s 对 0.84 s，
  未发现性能回退。
