# 根 candidate epoch 压缩 + LKH 对比

配置：hybrid-reverse 根策略、subtree-adaptive/32 节点势更新、LKH tour + PI warm start、16 轮本地精修。
同一 Release 二进制通过 `--root-candidate-compaction off|on` A/B；每实例 10 次，交替运行模式和实例顺序。
LKH provider 为本地隔离式 provider，所有记录的 `lkh_provider_seconds` 均大于 0，确认没有回退到内部 CLK。

关键参数：

```text
--lkh-provider auto
--lkh-pi-mode warm-start
--root-pi-refine-iterations 16
--hk-ascent hybrid-reverse
--hk-node-ascent polyak
--branch-edge-order weight
--hk-potential-update subtree-adaptive
--hk-update-depth 1
--hk-update-gap-ratio 0.02
--hk-update-min-gap-ratio 0
--hk-update-iterations 32
--hk-update-budget 0
--exact-max-n 199
```

| 实例 | expanded | 边压缩 | off 中位数(s) | on 中位数(s) | 时间变化 | 加速比 |
|---|---:|---:|---:|---:|---:|---:|
| eil101 | 17 | 96.5% | 0.034565 | 0.034404 | -0.47% | 1.005x |
| gr96 | 453 | 88.4% | 0.196182 | 0.195893 | -0.15% | 1.001x |
| pr124 | 750 | 84.3% | 0.554876 | 0.556290 | +0.25% | 0.997x |
| kroC100 | 3082 | 87.4% | 1.383216 | 1.380331 | -0.21% | 1.002x |
| u159 | 479 | 96.1% | 0.451831 | 0.451419 | -0.09% | 1.001x |

- 五实例逐实例中位数加速比的几何平均：1.0013x。
- 每轮五实例总时间中位数：off=2.627584s，on=2.625081s，变化=-0.10%。
- 所有 A/B 的最优值、根下界、创建节点数和扩展节点数完全一致。

## 结论

根压缩正确且开销很小：单次约 13--30 微秒，物理候选边减少 84.3%--96.5%。
但在这组配置上端到端收益只有约 0.1%，目前不能声称有稳定速度提升；`pr124`
的 0.25% 回退也属于同量级计时波动。主要原因是 `subtree-adaptive` 在较浅节点
频繁进入新的 potential epoch，而既有 `rebuildPotentialEpoch` 本来就只物化当前
active/forced 边；同时总耗时主要落在节点势上升，而不是候选位图扫描。因此本次
改动的直接收益集中在根到首次成功 epoch 重建之前，以及未触发/未安装新 epoch 的
子树。

原始重复数据见 `raw_results.csv`，逐实例中位数见 `summary.csv`。
