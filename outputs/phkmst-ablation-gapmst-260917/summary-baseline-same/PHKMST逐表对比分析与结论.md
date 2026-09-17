# PHKMST 消融实验汇总

数据源：`/home/wj/code/TSP/outputs/phkmst-ablation-gapmst-260917`。发现 5 个运行、3 组对比。逐实例完整数据见同目录 CSV 文件。

## 运行概览

| 配置 | run_id | 类型 | Solved count | PAR2 | 成功集几何平均 | 惩罚几何平均 | 超时 | 其他 |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| CPHKMST-P32-LKH | `CPHKMST-P32-LKH-db6a5c729bce` | tsp_bb | 43/50 | 538.713 s | 579.197 ms | 1.967 s | 7 | 0 |
| Concorde | `Concorde-7c32cb4cd69e` | concorde | 50/50 | 340.497 ms | 200.208 ms | 200.208 ms | 0 | 0 |
| GAPMST-P32-LKH | `GAPMST-P32-LKH-8971cab9e6d2` | tsp_bb | 43/50 | 556.928 s | 549.094 ms | 1.879 s | 7 | 0 |
| GAPMST-P32-LKH-gc0.0001 | `GAPMST-P32-LKH-gc0.0001-c86b2cb541dc` | tsp_bb | 43/50 | 559.052 s | 575.489 ms | 1.957 s | 7 | 0 |
| GAPMST-P32-LKH-same | `GAPMST-P32-LKH-same-20de80c52702` | tsp_bb | 43/50 | 535.521 s | 511.711 ms | 1.769 s | 7 | 0 |

## 自动化口径

- 当前每个配置、每个实例只有一次观测；不计算跨实例中位数，也不把单次差异解释为统计优势。
- 默认生成单因素对比；参数完全相同的运行保留为重复性对比。同一因素有多个水平时，以连接其他配置最多的运行作为基线。
- 性能总量只统计双方共同成功且该指标都有记录的实例；先比较成功数，再解释共同成功集合的总量。
- 右侧相对变化采用 `右侧总量/左侧总量-1`；负数表示右侧减少，正数表示右侧增加。
- 下降/持平/上升按每个共同成功实例的原始单次观测计数。
- 新增的数值型 results.csv 列会自动进入明细和成对汇总 CSV，无需修改本脚本。
- PAR2 在统一实例全集上计算：成功使用实际墙钟时间，超时、错误、部分完成或缺失使用 `2 × 该配置 timeout`；惩罚几何平均使用同一组 PAR2 样本。成功集几何平均只使用成功实例。
- 性能剖面使用 Dolan–Moré 时间比：每个实例以所有成功配置中的最快时间为 1；未成功配置视为无穷，在任何有限 τ 下都不计入。
- 正确性参考：Concorde（`Concorde-7c32cb4cd69e`）。

## 墙钟时间性能剖面

下表给出性能剖面的几个代表性 τ；完整阶梯点见 `performance_profile.csv`。

| 配置 | 1× | 2× | 10× | 100× |
| --- | ---: | ---: | ---: | ---: |
| CPHKMST-P32-LKH | 8.0% | 42.0% | 60.0% | 80.0% |
| Concorde | 62.0% | 74.0% | 100.0% | 100.0% |
| GAPMST-P32-LKH | 6.0% | 50.0% | 62.0% | 78.0% |
| GAPMST-P32-LKH-gc0.0001 | 6.0% | 50.0% | 62.0% | 78.0% |
| GAPMST-P32-LKH-same | 18.0% | 48.0% | 62.0% | 80.0% |

## 节点势更新指标含义

- **节点势更新候选数**：通过前置下界剪枝后，真正到达势更新判定点的非根逻辑搜索节点数；每个节点只计一次，是触发率的分母。
- **节点势更新触发数**：候选节点通过策略、预算、数值安全、深度和 gap 等条件后，实际启动节点势优化的次数。
- **节点势迭代数**：所有已触发更新内部执行的次梯度迭代轮数之和；一次触发最多可执行 `--hk-update-iterations` 轮。
- **节点势更新跳过总数**：所有互斥跳过原因之和。求解器按第一个命中的原因计数，因此应满足 `候选数 = 触发数 + 跳过总数`。
- `--hk-update-budget` 限制的是**每轮根搜索的触发次数**，不是迭代总数；设为 `0` 表示不限。正数预算下，初始探测轮最多使用 `min(budget, 1000)` 次；若 incumbent 改善并重启根搜索，计数器会清零，下一轮可再使用完整预算。因此整个求解的触发数可能超过配置预算，迭代数还会再乘上每次更新的迭代轮数。预算耗尽后访问的候选节点仍会逐个计入“跳过：预算耗尽”，所以跳过数也可能远大于预算。
- 阶段耗时中 `replacement_seconds` 是根 fixing、BP 分支和候选删除过程中 fundamental-cut replacement 查询的子阶段，可能与其他阶段重叠，不能直接把所有耗时列相加当作总时间。

## 五、其他单因素与重复性对比

保留其余自动发现的单因素及相同配置重复运行对比，计算与展示口径不变。

### 对比 1：GAPMST-P32-LKH-same → CPHKMST-P32-LKH

变化因素：**hk update gap change start depth、hk update max depth、hk update min gap change ratio、hk update skip last edges、root candidate compaction、求解器版本**（hk update gap change start depth=0；hk update max depth=0；hk update min gap change ratio=0；hk update skip last edges=0；root candidate compaction=off；求解器版本=89fc4d89dd02 → hk update gap change start depth=<未设置>；hk update max depth=<未设置>；hk update min gap change ratio=<未设置>；hk update skip last edges=<未设置>；root candidate compaction=<未设置>；求解器版本=129d73c9e0df）。

成功实例：GAPMST-P32-LKH-same=43，CPHKMST-P32-LKH=43，共同成功=43；右侧新增=0，右侧丢失=0。

| 指标 | 配对数 | 左侧总量 | 右侧总量 | 右侧相对变化 | 右侧下降/持平/上升 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 运行时间 | 43 | 1,576.058 s | 1,735.627 s | +10.12% | 10/0/33 |
| 根下界 | 43 | 898,754 | 898,754 | 0.00% | 0/43/0 |
| 分支创建数 | 43 | 2,612,950 | 2,612,950 | 0.00% | 0/43/0 |
| 初始/自适应 CLK 耗时 | 43 | 2.900 s | 2.842 s | -2.01% | 24/0/19 |
| 根势上升耗时 | 43 | 737.267 ms | 707.693 ms | -4.01% | 14/0/29 |
| LKH provider 调用数 | 43 | 43 | 43 | 0.00% | 0/43/0 |
| LKH provider 失败数 | 43 | 0 | 0 | 0.00% | 0/43/0 |
| LKH provider 耗时 | 43 | 3.198 s | 3.235 s | +1.15% | 19/0/24 |
| 根 fixing 耗时 | 43 | 20.865 ms | 20.177 ms | -3.30% | 19/10/14 |
| 节点势更新耗时 | 43 | 1,442.294 s | 1,596.741 s | +10.71% | 2/10/31 |
| 势 epoch 重建耗时 | 43 | 99.363 s | 102.991 s | +3.65% | 7/14/22 |
| replacement 查询耗时 | 43 | 13.699 s | 13.764 s | +0.47% | 13/10/20 |
| 节点势更新跳过总数 | 43 | 0 | 0 | 0.00% | 0/43/0 |

结果一致性：共同成功且有结果的实例均一致。
参考校验：左侧检查 43 个，右侧检查 43 个；均与参考结果一致。

### 对比 2：GAPMST-P32-LKH-same → GAPMST-P32-LKH

变化因素：**hk update gap change start depth、hk update max depth、hk update min gap change ratio、hk update skip last edges、root candidate compaction**（hk update gap change start depth=0；hk update max depth=0；hk update min gap change ratio=0；hk update skip last edges=0；root candidate compaction=off → hk update gap change start depth=<未设置>；hk update max depth=<未设置>；hk update min gap change ratio=<未设置>；hk update skip last edges=<未设置>；root candidate compaction=<未设置>）。

成功实例：GAPMST-P32-LKH-same=43，GAPMST-P32-LKH=43，共同成功=43；右侧新增=0，右侧丢失=0。

| 指标 | 配对数 | 左侧总量 | 右侧总量 | 右侧相对变化 | 右侧下降/持平/上升 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 运行时间 | 43 | 1,576.058 s | 2,646.393 s | +67.91% | 18/0/25 |
| 根下界 | 43 | 898,754 | 898,754 | 0.00% | 0/43/0 |
| 分支创建数 | 43 | 2,612,950 | 19,150,110 | +632.89% | 0/19/24 |
| 初始/自适应 CLK 耗时 | 43 | 2.900 s | 3.140 s | +8.28% | 17/0/26 |
| 根势上升耗时 | 43 | 737.267 ms | 722.392 ms | -2.02% | 20/0/23 |
| LKH provider 调用数 | 43 | 43 | 43 | 0.00% | 0/43/0 |
| LKH provider 失败数 | 43 | 0 | 0 | 0.00% | 0/43/0 |
| LKH provider 耗时 | 43 | 3.198 s | 3.221 s | +0.71% | 22/0/21 |
| 根 fixing 耗时 | 43 | 20.865 ms | 19.978 ms | -4.25% | 18/10/15 |
| 节点势更新耗时 | 43 | 1,442.294 s | 2,397.334 s | +66.22% | 17/10/16 |
| 势 epoch 重建耗时 | 43 | 99.363 s | 74.853 s | -24.67% | 26/14/3 |
| replacement 查询耗时 | 43 | 13.699 s | 121.764 s | +788.83% | 7/10/26 |
| 节点势更新跳过总数 | 43 | 0 | 8,265,001 | — | 0/18/25 |

结果一致性：共同成功且有结果的实例均一致。
参考校验：左侧检查 43 个，右侧检查 43 个；均与参考结果一致。

### 对比 3：GAPMST-P32-LKH-same → GAPMST-P32-LKH-gc0.0001

变化因素：**hk update min gap change ratio、root candidate compaction**（hk update min gap change ratio=0；root candidate compaction=off → hk update min gap change ratio=0.0001；root candidate compaction=on）。

成功实例：GAPMST-P32-LKH-same=43，GAPMST-P32-LKH-gc0.0001=43，共同成功=43；右侧新增=0，右侧丢失=0。

| 指标 | 配对数 | 左侧总量 | 右侧总量 | 右侧相对变化 | 右侧下降/持平/上升 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 运行时间 | 43 | 1,576.058 s | 2,752.606 s | +74.65% | 16/0/27 |
| 根下界 | 43 | 898,754 | 898,754 | 0.00% | 0/43/0 |
| 分支创建数 | 43 | 2,612,950 | 20,127,463 | +670.30% | 0/12/31 |
| 初始/自适应 CLK 耗时 | 43 | 2.900 s | 3.134 s | +8.07% | 18/0/25 |
| 根势上升耗时 | 43 | 737.267 ms | 704.191 ms | -4.49% | 24/0/19 |
| LKH provider 调用数 | 43 | 43 | 43 | 0.00% | 0/43/0 |
| LKH provider 失败数 | 43 | 0 | 0 | 0.00% | 0/43/0 |
| LKH provider 耗时 | 43 | 3.198 s | 3.229 s | +0.97% | 21/0/22 |
| 根 fixing 耗时 | 43 | 20.865 ms | 20.506 ms | -1.72% | 19/10/14 |
| 节点势更新耗时 | 43 | 1,442.294 s | 2,491.434 s | +72.74% | 22/10/11 |
| 势 epoch 重建耗时 | 43 | 99.363 s | 77.753 s | -21.75% | 26/14/3 |
| replacement 查询耗时 | 43 | 13.699 s | 128.446 s | +837.60% | 6/10/27 |
| 节点势更新跳过总数 | 43 | 0 | 8,660,191 | — | 0/10/33 |

结果一致性：共同成功且有结果的实例均一致。
参考校验：左侧检查 43 个，右侧检查 43 个；均与参考结果一致。
