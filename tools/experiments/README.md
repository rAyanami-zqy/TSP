# CPHKMST 候选集与定价研究工具

这是可选研究工具，不是默认 CMake 构建依赖。精确求解仍是本仓库 constrained
Held–Karp 1-tree B&B；不调用 Concorde 的 LP、cut 或精确求解器。
Linkern/KD/EDGEGEN 直接链接仓库 `concorde/` 的 C 实现。
该代码限学术研究使用，其他用途须另行取得授权，见 `concorde/README`。

## 构建和运行

需要 C++17 编译器、C 编译器、make、Python 3。以下命令从仓库根目录运行；
使用新的构建目录，不复用仓库中的历史 Linux 二进制或 CMake 缓存。

```bash
python3 tools/experiments/build_cphkmst_explore.py --build /private/tmp/cphkmst-research
python3 tools/experiments/build_cphkmst_explore.py --build /private/tmp/cphkmst-research --baseline
python3 tools/experiments/build_cphkmst_explore.py --build /private/tmp/cphkmst-research --verify
python3 tools/experiments/build_cphkmst_explore.py --build /private/tmp/cphkmst-research --spatial

/private/tmp/cphkmst-research/explore-compact clk-multi data/classic/tsplib/eil101.tsp root 1
/private/tmp/cphkmst-research/explore-compact pricing data/classic/tsplib/eil101.tsp
/private/tmp/cphkmst-research/spatial
```

驱动参数为 `MODE INSTANCE [root|exact] [seed]`。`root` 仅计算初始可行解与根下界，
**不是最优证明**，也在根 fixing 之前返回。结果在唯一的 `RESULT {JSON}` 行，
其余标准输出来自 Concorde。`--verify` 构建额外校验增量树与完整重建，不能用于性能比较。

本次可复现实验命令（`--output` 可自行修改）：

```bash
python3 tools/experiments/run_cphkmst_explore.py \
  --executable /private/tmp/cphkmst-research/explore-compact \
  --suite root50 --output /private/tmp/cphkmst-root50.jsonl
python3 tools/experiments/run_cphkmst_explore.py \
  --executable /private/tmp/cphkmst-research/explore-compact \
  --suite confirm --modes baseline clk-knn clk-walk clk-multi --repeats 3 \
  --output /private/tmp/cphkmst-confirm.jsonl
python3 tools/experiments/run_cphkmst_explore.py \
  --executable /private/tmp/cphkmst-research/explore-compact \
  --suite root50 --modes pricing --output /private/tmp/cphkmst-pricing50.jsonl
python3 tools/experiments/compare_cphkmst_epochs.py \
  --baseline /private/tmp/cphkmst-research/explore-baseline \
  --compact /private/tmp/cphkmst-research/explore-compact \
  --output /private/tmp/cphkmst-epochs.jsonl
python3 tools/experiments/run_cphkmst_explore.py \
  --executable /private/tmp/cphkmst-research/explore-compact \
  --suite hard --modes baseline clk-multi --timeout 3 \
  --output /private/tmp/cphkmst-hard35.jsonl
```

批处理串行运行、逐行落盘、按二进制 SHA256 检查恢复边界。改变二进制或实验参数时
应换输出文件；已有条目不自动重测。`confirm` 的 repeat 1/2/3 也对应 seed 1/2/3；
`compare_cphkmst_epochs.py` 则固定 seed=1，交替两版本先后顺序，逐项断言最优值、
UB/LB、节点数、势迭代、fixing 和返回回路不变。

`seconds` 从额外候选生成之前计时，包含外部 LK、内置启发式、根势及精确搜索；
不含文件解析、稠密矩阵构造、跨库距离验证。`process_seconds` 包含这些费用及进程启动。
包括 baseline 在内的模式还统一构造 kNN8 图用于统计；研究驱动计时不等于主程序裸跑。
超时不是不可行，也不是求得最优值；不能把仅完成子集的耗时总和当作全组加速比。

## 模式的准确含义

| 模式 | 操作 |
|---|---|
| baseline | 原有有向 kNN8 + 内置多启动 LK |
| knn12 / knn20 | 只替换内置启发式候选和 replacement 提示，不裁剪精确图 |
| kd8 | Concorde nearest8 的无向并集；方向与等距破同分也会变化，不是纯 KD 数据结构对照 |
| quad2 | kNN8 并入各象限近邻 2 条；非支持范数由原库走适用的近邻回退 |
| delaunay | 仅 EUC_2D：kNN8 并入 Delaunay，不适用实例明确跳过 |
| union | kNN8 并入原生 EDGEGEN 的 10 次 Linkern 回路并集；每次 n/100+1 kicks |
| clk-zero | 原生 Linkern，quad2 候选，1 起点、0 kicks |
| clk-knn | 原生 Linkern，kNN8 无向并集，1 起点、10 geometric kicks |
| clk-geo | 原生 Linkern，quad2，1 起点、10 geometric kicks |
| clk-walk | 原生 Linkern，quad2，1 起点、10 random-walk kicks |
| clk-multi | 原生 Linkern，quad2，3 起点、各 10 geometric kicks，择优 |
| pricing | 24 个固定势/约束状态，用全图 constrained 1-tree 作独立重建对照 |

所有 CLK 起点由固定 seed 的原库随机生成；非 KD 范数的 geometric kick 会由原库
回退。CLK 模式保留内置启发式并与外部结果择优，外部库成本必须与本仓库逐边重算一致。
`graph_edges` 和 `returned_tour_edges_outside_graph` 指驱动中的启发式图；CLK 模式中
它们仍指原 kNN8，**不是外部 Linkern 实际用的 quad2 图**。

GEO 的圆周率约定在两库间有差异。驱动首先逐边比较，如果不一致，将本仓库的原始
整数距离矩阵传给 Concorde MATRIX 范数，不能让外部库悄悄改写待求问题。
此时 quad/KD 名称不再表示几何加速；`native_distance_mismatches` 记录差异数。
研究驱动仅支持 n≤2000；跨库矩阵回退限非负、有限、≤10⁷ 的整数距离。
这些限制不影响主求解器的输入能力。

`pricing` 从 kNN8 加可行回路边开始，扫描遗漏边的**带符号** MST 环交换/根边替换
增量，补回可能更优或同分的边，直至证书完整；不可行稀疏树回退全图。
每例 24 状态含优化根势、扰动势、随机强制非根回路森林边及禁止非回路边。
比较成本和 canonical degree，而非仅比较“下界看起来接近”。全图与稀疏计时共用已排序
边输入；稀疏版含建稀疏列表、路径支持值、全遗漏边扫描和重建。不是生产 LP pricing。

空间树工具独立测试 100/1000/10000 个随机二维点、k=8/20；包括 KD 建树，扫描使用
`nth_element`，对每个查询比较排序后的近邻整数距离，允许等距时端点不同。
未测试 Quadtree 或高维几何，不能从二维结果外推高维性能。

## 将 Linkern 上界交给主程序

```bash
python3 tools/experiments/export_cphkmst_tour.py /private/tmp/cphkmst-root50.jsonl \
  --instance eil101 --mode clk-multi --seed 1 --output /private/tmp/eil101-seed.tour
./build/tsp_bb --initial-tour /private/tmp/eil101-seed.tour \
  --hk-potential-update subtree-adaptive --hk-update-depth 1 \
  --hk-update-iterations 32 --hk-update-budget 0 \
  data/classic/tsplib/eil101.tsp
```

导出拒绝覆盖已有文件。主程序重新核验回路；即使误选了不适配实例的文件，也不会
信任 JSON 中的成本。主程序只需要文本回路，不需要链接 Concorde。

结论和原始证据索引见
[2026-09-11 探索报告](../../docs/CPHKMST-candidate-pricing-exploration-2026-09-11.md)。

重新核对已保存结果并生成汇总：

```bash
python3 tools/experiments/summarize_cphkmst_explore.py \
  outputs/cphkmst-exploration-20260911 \
  --output outputs/cphkmst-exploration-20260911/summary.json
```
