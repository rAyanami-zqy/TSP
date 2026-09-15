# Adaptive CLK gap ratio：50 实例 root-only 扫描

所有配置只执行初始 tour、根势上升和根 1-tree；不执行 root fixing、节点势更新或 BP。
每格时间为各实例多次重复的中位数，再跨 50 个实例求和。

| 配置 | 触发实例 | 改善实例 | CLK starts | 漏掉 triple 有益实例 | 避免 triple 增时实例 | 总 wall(s) | 相对 single(s) | 初始 tour(s) |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| single | 0 | 0 | 50 | 21 | 43/43 | 5.908438 | +0.000000 | 1.293357 |
| triple | 0 | 0 | 150 | 21 | 43/43 | 6.790444 | +0.882006 | 2.074448 |
| adaptive-0.014 | 29 | 20 | 108 | 1 | 20/43 | 9.181097 | +3.272659 | 1.900085 |

- triple 相对 single 能改善 UB 的实例：21。
- triple 具有实质 wall 增时（>10% 且 >1ms）的实例：43。
- triple 具有实质初始 tour 增时（>10% 且 >1ms）的实例：50。
- `漏掉 triple 有益实例` 只说明该阈值没有触发这些实例；root-only 实验不测它们对完整 BP 节点数的后续影响。

完整逐实例数据见 `instance_comparison.csv`，原始重复数据见 `raw_results.csv`。
