# PHKMST 消融实验汇总

数据源：`/home/wj/code/TSP/outputs/phkmst-ablation-bp-20260917`。发现 11 个运行、9 组对比。逐实例完整数据见同目录 CSV 文件。

## 运行概览

| 配置 | run_id | 类型 | Solved count | PAR2 | 成功集几何平均 | 惩罚几何平均 | 超时 | 其他 |
| --- | --- | --- | ---: | ---: | ---: | ---: | ---: | ---: |
| CPHKMST-P32-LKH | `CPHKMST-P32-LKH-598122e510eb` | tsp_bb | 43/50 | 538.565 s | 564.583 ms | 1.925 s | 7 | 0 |
| Concorde | `Concorde-7c32cb4cd69e` | concorde | 50/50 | 340.497 ms | 200.208 ms | 200.208 ms | 0 | 0 |
| GAPMST-P32-LKH-BP-ascent-strong | `GAPMST-P32-LKH-BP-ascent-strong-9bbe061f25d1` | tsp_bb | 43/50 | 532.554 s | 532.806 ms | 1.831 s | 7 | 0 |
| GAPMST-P32-LKH-BP-ascent-strong-lift2 | `GAPMST-P32-LKH-BP-ascent-strong-lift2-b95e30138c43` | tsp_bb | 43/50 | 532.578 s | 554.127 ms | 1.894 s | 7 | 0 |
| GAPMST-P32-LKH-BP-ascent-strong-lift2-split2 | `GAPMST-P32-LKH-BP-ascent-strong-lift2-split2-677cd822c7b0` | tsp_bb | 43/50 | 532.714 s | 582.980 ms | 1.979 s | 7 | 0 |
| GAPMST-P32-LKH-BP-ascent-strong-split2 | `GAPMST-P32-LKH-BP-ascent-strong-split2-5a5c42ecf287` | tsp_bb | 43/50 | 532.776 s | 620.883 ms | 2.089 s | 7 | 0 |
| GAPMST-P32-LKH-BP-base | `GAPMST-P32-LKH-BP-base-0161298b4893` | tsp_bb | 43/50 | 534.650 s | 393.555 ms | 1.411 s | 7 | 0 |
| GAPMST-P32-LKH-BP-lift2 | `GAPMST-P32-LKH-BP-lift2-a8ef6cbbece9` | tsp_bb | 43/50 | 534.716 s | 527.665 ms | 1.816 s | 7 | 0 |
| GAPMST-P32-LKH-BP-lift2-split2 | `GAPMST-P32-LKH-BP-lift2-split2-8625e3037a75` | tsp_bb | 43/50 | 531.654 s | 561.604 ms | 1.916 s | 7 | 0 |
| GAPMST-P32-LKH-BP-split2 | `GAPMST-P32-LKH-BP-split2-fcf930fe1eca` | tsp_bb | 43/50 | 531.590 s | 569.208 ms | 1.938 s | 7 | 0 |
| GAPMST-P32-LKH-same | `GAPMST-P32-LKH-same-86f81b28e491` | tsp_bb | 43/50 | 535.482 s | 537.778 ms | 1.846 s | 7 | 0 |

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
| CPHKMST-P32-LKH | 4.0% | 32.0% | 60.0% | 80.0% |
| Concorde | 60.0% | 68.0% | 98.0% | 100.0% |
| GAPMST-P32-LKH-BP-ascent-strong | 6.0% | 34.0% | 64.0% | 80.0% |
| GAPMST-P32-LKH-BP-ascent-strong-lift2 | 0.0% | 30.0% | 64.0% | 80.0% |
| GAPMST-P32-LKH-BP-ascent-strong-lift2-split2 | 0.0% | 30.0% | 60.0% | 80.0% |
| GAPMST-P32-LKH-BP-ascent-strong-split2 | 0.0% | 26.0% | 60.0% | 80.0% |
| GAPMST-P32-LKH-BP-base | 28.0% | 54.0% | 62.0% | 80.0% |
| GAPMST-P32-LKH-BP-lift2 | 0.0% | 40.0% | 62.0% | 80.0% |
| GAPMST-P32-LKH-BP-lift2-split2 | 0.0% | 36.0% | 62.0% | 80.0% |
| GAPMST-P32-LKH-BP-split2 | 0.0% | 36.0% | 62.0% | 80.0% |
| GAPMST-P32-LKH-same | 2.0% | 32.0% | 62.0% | 80.0% |

## 节点势更新指标含义

- **节点势更新候选数**：通过前置下界剪枝后，真正到达势更新判定点的非根逻辑搜索节点数；每个节点只计一次，是触发率的分母。
- **节点势更新触发数**：候选节点通过策略、预算、数值安全、深度和 gap 等条件后，实际启动节点势优化的次数。
- **节点势迭代数**：所有已触发更新内部执行的次梯度迭代轮数之和；一次触发最多可执行 `--hk-update-iterations` 轮。
- **节点势更新跳过总数**：所有互斥跳过原因之和。求解器按第一个命中的原因计数，因此应满足 `候选数 = 触发数 + 跳过总数`。
- `--hk-update-budget` 限制的是**每轮根搜索的触发次数**，不是迭代总数；设为 `0` 表示不限。正数预算下，初始探测轮最多使用 `min(budget, 1000)` 次；若 incumbent 改善并重启根搜索，计数器会清零，下一轮可再使用完整预算。因此整个求解的触发数可能超过配置预算，迭代数还会再乘上每次更新的迭代轮数。预算耗尽后访问的候选节点仍会逐个计入“跳过：预算耗尽”，所以跳过数也可能远大于预算。
- 阶段耗时中 `replacement_seconds` 是根 fixing、BP 分支和候选删除过程中 fundamental-cut replacement 查询的子阶段，可能与其他阶段重叠，不能直接把所有耗时列相加当作总时间。

## 五、其他单因素与重复性对比

保留其余自动发现的单因素及相同配置重复运行对比，计算与展示口径不变。

### 对比 1：GAPMST-P32-LKH-BP-base → CPHKMST-P32-LKH

变化因素：**bp lift first depth、bp split zero gain depth、hk update gap change start depth、hk update max depth、hk update min gap change ratio、hk update skip last edges、root candidate compaction、求解器版本**（bp lift first depth=0；bp split zero gain depth=0；hk update gap change start depth=0；hk update max depth=0；hk update min gap change ratio=0；hk update skip last edges=0；root candidate compaction=off；求解器版本=f8e487148cc9 → bp lift first depth=<未设置>；bp split zero gain depth=<未设置>；hk update gap change start depth=<未设置>；hk update max depth=<未设置>；hk update min gap change ratio=<未设置>；hk update skip last edges=<未设置>；root candidate compaction=<未设置>；求解器版本=129d73c9e0df）。

成功实例：GAPMST-P32-LKH-BP-base=43，CPHKMST-P32-LKH=43，共同成功=43；右侧新增=0，右侧丢失=0。

| 指标 | 配对数 | 左侧总量 | 右侧总量 | 右侧相对变化 | 右侧下降/持平/上升 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 运行时间 | 43 | 1,532.479 s | 1,728.248 s | +12.77% | 4/0/39 |
| 根下界 | 43 | 898,754 | 898,754 | 0.00% | 0/43/0 |
| 分支创建数 | 43 | 2,612,950 | 2,612,950 | 0.00% | 0/43/0 |
| 初始/自适应 CLK 耗时 | 43 | 2.828 s | 2.873 s | +1.58% | 19/0/24 |
| 根势上升耗时 | 43 | 712.811 ms | 702.137 ms | -1.50% | 26/0/17 |
| LKH provider 调用数 | 43 | 43 | 43 | 0.00% | 0/43/0 |
| LKH provider 失败数 | 43 | 0 | 0 | 0.00% | 0/43/0 |
| LKH provider 耗时 | 43 | 3.224 s | 3.227 s | +0.10% | 17/0/26 |
| 根 fixing 耗时 | 43 | 20.218 ms | 20.059 ms | -0.79% | 14/10/19 |
| 节点势更新耗时 | 43 | 1,401.604 s | 1,589.989 s | +13.44% | 3/10/30 |
| 势 epoch 重建耗时 | 43 | 98.125 s | 102.424 s | +4.38% | 5/14/24 |
| replacement 查询耗时 | 43 | 12.999 s | 13.691 s | +5.33% | 9/10/24 |
| 节点势更新跳过总数 | 43 | 0 | 0 | 0.00% | 0/43/0 |

结果一致性：共同成功且有结果的实例均一致。
参考校验：左侧检查 43 个，右侧检查 43 个；均与参考结果一致。

### 对比 2：GAPMST-P32-LKH-BP-base → GAPMST-P32-LKH-BP-ascent-strong

变化因素：**branch edge order**（branch edge order=weight → branch edge order=ascent-strong-top2）。

成功实例：GAPMST-P32-LKH-BP-base=43，GAPMST-P32-LKH-BP-ascent-strong=43，共同成功=43；右侧新增=0，右侧丢失=0。

| 指标 | 配对数 | 左侧总量 | 右侧总量 | 右侧相对变化 | 右侧下降/持平/上升 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 运行时间 | 43 | 1,532.479 s | 1,427.702 s | -6.84% | 12/0/31 |
| 根下界 | 43 | 898,754 | 898,754 | 0.00% | 0/43/0 |
| 分支创建数 | 43 | 2,612,950 | 2,235,358 | -14.45% | 22/12/9 |
| 初始/自适应 CLK 耗时 | 43 | 2.828 s | 2.776 s | -1.82% | 18/0/25 |
| 根势上升耗时 | 43 | 712.811 ms | 717.930 ms | +0.72% | 19/0/24 |
| LKH provider 调用数 | 43 | 43 | 43 | 0.00% | 0/43/0 |
| LKH provider 失败数 | 43 | 0 | 0 | 0.00% | 0/43/0 |
| LKH provider 耗时 | 43 | 3.224 s | 3.181 s | -1.32% | 23/0/20 |
| 根 fixing 耗时 | 43 | 20.218 ms | 19.935 ms | -1.40% | 21/10/12 |
| 节点势更新耗时 | 43 | 1,401.604 s | 1,208.979 s | -13.74% | 25/10/8 |
| 势 epoch 重建耗时 | 43 | 98.125 s | 82.637 s | -15.78% | 23/13/7 |
| replacement 查询耗时 | 43 | 12.999 s | 20.883 s | +60.65% | 6/10/27 |
| 节点势更新跳过总数 | 43 | 0 | 0 | 0.00% | 0/43/0 |

结果一致性：共同成功且有结果的实例均一致。
参考校验：左侧检查 43 个，右侧检查 43 个；均与参考结果一致。

### 对比 3：GAPMST-P32-LKH-BP-base → GAPMST-P32-LKH-BP-ascent-strong-lift2

变化因素：**bp lift first depth、branch edge order**（bp lift first depth=0；branch edge order=weight → bp lift first depth=2；branch edge order=ascent-strong-top2）。

成功实例：GAPMST-P32-LKH-BP-base=43，GAPMST-P32-LKH-BP-ascent-strong-lift2=43，共同成功=43；右侧新增=0，右侧丢失=0。

| 指标 | 配对数 | 左侧总量 | 右侧总量 | 右侧相对变化 | 右侧下降/持平/上升 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 运行时间 | 43 | 1,532.479 s | 1,428.914 s | -6.76% | 11/0/32 |
| 根下界 | 43 | 898,754 | 898,754 | 0.00% | 0/43/0 |
| 分支创建数 | 43 | 2,612,950 | 2,235,358 | -14.45% | 22/12/9 |
| 初始/自适应 CLK 耗时 | 43 | 2.828 s | 2.793 s | -1.22% | 25/0/18 |
| 根势上升耗时 | 43 | 712.811 ms | 728.087 ms | +2.14% | 18/0/25 |
| LKH provider 调用数 | 43 | 43 | 43 | 0.00% | 0/43/0 |
| LKH provider 失败数 | 43 | 0 | 0 | 0.00% | 0/43/0 |
| LKH provider 耗时 | 43 | 3.224 s | 3.225 s | +0.03% | 19/0/24 |
| 根 fixing 耗时 | 43 | 20.218 ms | 19.874 ms | -1.70% | 18/10/15 |
| 节点势更新耗时 | 43 | 1,401.604 s | 1,209.545 s | -13.70% | 25/10/8 |
| 势 epoch 重建耗时 | 43 | 98.125 s | 82.903 s | -15.51% | 23/13/7 |
| replacement 查询耗时 | 43 | 12.999 s | 20.939 s | +61.09% | 9/10/24 |
| 节点势更新跳过总数 | 43 | 0 | 0 | 0.00% | 0/43/0 |

结果一致性：共同成功且有结果的实例均一致。
参考校验：左侧检查 43 个，右侧检查 43 个；均与参考结果一致。

### 对比 4：GAPMST-P32-LKH-BP-base → GAPMST-P32-LKH-BP-ascent-strong-lift2-split2

变化因素：**bp lift first depth、bp split zero gain depth、branch edge order**（bp lift first depth=0；bp split zero gain depth=0；branch edge order=weight → bp lift first depth=2；bp split zero gain depth=2；branch edge order=ascent-strong-top2）。

成功实例：GAPMST-P32-LKH-BP-base=43，GAPMST-P32-LKH-BP-ascent-strong-lift2-split2=43，共同成功=43；右侧新增=0，右侧丢失=0。

| 指标 | 配对数 | 左侧总量 | 右侧总量 | 右侧相对变化 | 右侧下降/持平/上升 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 运行时间 | 43 | 1,532.479 s | 1,435.715 s | -6.31% | 8/0/35 |
| 根下界 | 43 | 898,754 | 898,754 | 0.00% | 0/43/0 |
| 分支创建数 | 43 | 2,612,950 | 2,233,849 | -14.51% | 12/11/20 |
| 初始/自适应 CLK 耗时 | 43 | 2.828 s | 2.941 s | +4.02% | 18/0/25 |
| 根势上升耗时 | 43 | 712.811 ms | 715.766 ms | +0.41% | 18/0/25 |
| LKH provider 调用数 | 43 | 43 | 43 | 0.00% | 0/43/0 |
| LKH provider 失败数 | 43 | 0 | 0 | 0.00% | 0/43/0 |
| LKH provider 耗时 | 43 | 3.224 s | 3.241 s | +0.54% | 16/0/27 |
| 根 fixing 耗时 | 43 | 20.218 ms | 20.594 ms | +1.86% | 10/10/23 |
| 节点势更新耗时 | 43 | 1,401.604 s | 1,211.865 s | -13.54% | 16/10/17 |
| 势 epoch 重建耗时 | 43 | 98.125 s | 82.671 s | -15.75% | 16/13/14 |
| replacement 查询耗时 | 43 | 12.999 s | 21.098 s | +62.31% | 4/10/29 |
| 节点势更新跳过总数 | 43 | 0 | 0 | 0.00% | 0/43/0 |

结果一致性：共同成功且有结果的实例均一致。
参考校验：左侧检查 43 个，右侧检查 43 个；均与参考结果一致。

### 对比 5：GAPMST-P32-LKH-BP-base → GAPMST-P32-LKH-BP-ascent-strong-split2

变化因素：**bp split zero gain depth、branch edge order**（bp split zero gain depth=0；branch edge order=weight → bp split zero gain depth=2；branch edge order=ascent-strong-top2）。

成功实例：GAPMST-P32-LKH-BP-base=43，GAPMST-P32-LKH-BP-ascent-strong-split2=43，共同成功=43；右侧新增=0，右侧丢失=0。

| 指标 | 配对数 | 左侧总量 | 右侧总量 | 右侧相对变化 | 右侧下降/持平/上升 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 运行时间 | 43 | 1,532.479 s | 1,438.802 s | -6.11% | 8/0/35 |
| 根下界 | 43 | 898,754 | 898,754 | 0.00% | 0/43/0 |
| 分支创建数 | 43 | 2,612,950 | 2,233,849 | -14.51% | 12/11/20 |
| 初始/自适应 CLK 耗时 | 43 | 2.828 s | 2.841 s | +0.47% | 21/0/22 |
| 根势上升耗时 | 43 | 712.811 ms | 718.247 ms | +0.76% | 21/0/22 |
| LKH provider 调用数 | 43 | 43 | 43 | 0.00% | 0/43/0 |
| LKH provider 失败数 | 43 | 0 | 0 | 0.00% | 0/43/0 |
| LKH provider 耗时 | 43 | 3.224 s | 3.209 s | -0.45% | 20/0/23 |
| 根 fixing 耗时 | 43 | 20.218 ms | 20.203 ms | -0.07% | 18/10/15 |
| 节点势更新耗时 | 43 | 1,401.604 s | 1,213.709 s | -13.41% | 16/10/17 |
| 势 epoch 重建耗时 | 43 | 98.125 s | 83.250 s | -15.16% | 15/13/15 |
| replacement 查询耗时 | 43 | 12.999 s | 21.083 s | +62.19% | 5/10/28 |
| 节点势更新跳过总数 | 43 | 0 | 0 | 0.00% | 0/43/0 |

结果一致性：共同成功且有结果的实例均一致。
参考校验：左侧检查 43 个，右侧检查 43 个；均与参考结果一致。

### 对比 6：GAPMST-P32-LKH-BP-base → GAPMST-P32-LKH-BP-lift2

变化因素：**bp lift first depth**（bp lift first depth=0 → bp lift first depth=2）。

成功实例：GAPMST-P32-LKH-BP-base=43，GAPMST-P32-LKH-BP-lift2=43，共同成功=43；右侧新增=0，右侧丢失=0。

| 指标 | 配对数 | 左侧总量 | 右侧总量 | 右侧相对变化 | 右侧下降/持平/上升 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 运行时间 | 43 | 1,532.479 s | 1,535.821 s | +0.22% | 9/0/34 |
| 根下界 | 43 | 898,754 | 898,754 | 0.00% | 0/43/0 |
| 分支创建数 | 43 | 2,612,950 | 2,612,950 | 0.00% | 0/43/0 |
| 初始/自适应 CLK 耗时 | 43 | 2.828 s | 2.830 s | +0.09% | 22/0/21 |
| 根势上升耗时 | 43 | 712.811 ms | 741.838 ms | +4.07% | 17/0/26 |
| LKH provider 调用数 | 43 | 43 | 43 | 0.00% | 0/43/0 |
| LKH provider 失败数 | 43 | 0 | 0 | 0.00% | 0/43/0 |
| LKH provider 耗时 | 43 | 3.224 s | 3.208 s | -0.50% | 18/0/25 |
| 根 fixing 耗时 | 43 | 20.218 ms | 20.189 ms | -0.14% | 16/10/17 |
| 节点势更新耗时 | 43 | 1,401.604 s | 1,404.280 s | +0.19% | 17/10/16 |
| 势 epoch 重建耗时 | 43 | 98.125 s | 98.172 s | +0.05% | 8/14/21 |
| replacement 查询耗时 | 43 | 12.999 s | 12.969 s | -0.23% | 21/10/12 |
| 节点势更新跳过总数 | 43 | 0 | 0 | 0.00% | 0/43/0 |

结果一致性：共同成功且有结果的实例均一致。
参考校验：左侧检查 43 个，右侧检查 43 个；均与参考结果一致。

### 对比 7：GAPMST-P32-LKH-BP-base → GAPMST-P32-LKH-BP-lift2-split2

变化因素：**bp lift first depth、bp split zero gain depth**（bp lift first depth=0；bp split zero gain depth=0 → bp lift first depth=2；bp split zero gain depth=2）。

成功实例：GAPMST-P32-LKH-BP-base=43，GAPMST-P32-LKH-BP-lift2-split2=43，共同成功=43；右侧新增=0，右侧丢失=0。

| 指标 | 配对数 | 左侧总量 | 右侧总量 | 右侧相对变化 | 右侧下降/持平/上升 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 运行时间 | 43 | 1,532.479 s | 1,382.715 s | -9.77% | 11/0/32 |
| 根下界 | 43 | 898,754 | 898,754 | 0.00% | 0/43/0 |
| 分支创建数 | 43 | 2,612,950 | 2,385,395 | -8.71% | 11/11/21 |
| 初始/自适应 CLK 耗时 | 43 | 2.828 s | 2.776 s | -1.85% | 23/0/20 |
| 根势上升耗时 | 43 | 712.811 ms | 715.608 ms | +0.39% | 16/0/27 |
| LKH provider 调用数 | 43 | 43 | 43 | 0.00% | 0/43/0 |
| LKH provider 失败数 | 43 | 0 | 0 | 0.00% | 0/43/0 |
| LKH provider 耗时 | 43 | 3.224 s | 3.254 s | +0.95% | 17/0/26 |
| 根 fixing 耗时 | 43 | 20.218 ms | 20.249 ms | +0.15% | 18/10/15 |
| 节点势更新耗时 | 43 | 1,401.604 s | 1,262.251 s | -9.94% | 17/10/16 |
| 势 epoch 重建耗时 | 43 | 98.125 s | 89.191 s | -9.10% | 13/13/17 |
| replacement 查询耗时 | 43 | 12.999 s | 11.773 s | -9.43% | 12/10/21 |
| 节点势更新跳过总数 | 43 | 0 | 0 | 0.00% | 0/43/0 |

结果一致性：共同成功且有结果的实例均一致。
参考校验：左侧检查 43 个，右侧检查 43 个；均与参考结果一致。

### 对比 8：GAPMST-P32-LKH-BP-base → GAPMST-P32-LKH-BP-split2

变化因素：**bp split zero gain depth**（bp split zero gain depth=0 → bp split zero gain depth=2）。

成功实例：GAPMST-P32-LKH-BP-base=43，GAPMST-P32-LKH-BP-split2=43，共同成功=43；右侧新增=0，右侧丢失=0。

| 指标 | 配对数 | 左侧总量 | 右侧总量 | 右侧相对变化 | 右侧下降/持平/上升 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 运行时间 | 43 | 1,532.479 s | 1,379.481 s | -9.98% | 10/0/33 |
| 根下界 | 43 | 898,754 | 898,754 | 0.00% | 0/43/0 |
| 分支创建数 | 43 | 2,612,950 | 2,385,395 | -8.71% | 11/11/21 |
| 初始/自适应 CLK 耗时 | 43 | 2.828 s | 2.807 s | -0.75% | 22/0/21 |
| 根势上升耗时 | 43 | 712.811 ms | 718.303 ms | +0.77% | 20/0/23 |
| LKH provider 调用数 | 43 | 43 | 43 | 0.00% | 0/43/0 |
| LKH provider 失败数 | 43 | 0 | 0 | 0.00% | 0/43/0 |
| LKH provider 耗时 | 43 | 3.224 s | 3.245 s | +0.67% | 20/0/23 |
| 根 fixing 耗时 | 43 | 20.218 ms | 20.077 ms | -0.70% | 18/10/15 |
| 节点势更新耗时 | 43 | 1,401.604 s | 1,259.787 s | -10.12% | 17/10/16 |
| 势 epoch 重建耗时 | 43 | 98.125 s | 88.525 s | -9.78% | 13/13/17 |
| replacement 查询耗时 | 43 | 12.999 s | 11.780 s | -9.38% | 12/10/21 |
| 节点势更新跳过总数 | 43 | 0 | 0 | 0.00% | 0/43/0 |

结果一致性：共同成功且有结果的实例均一致。
参考校验：左侧检查 43 个，右侧检查 43 个；均与参考结果一致。

### 对比 9：GAPMST-P32-LKH-BP-base → GAPMST-P32-LKH-same

变化因素：**bp lift first depth、bp split zero gain depth、求解器版本**（bp lift first depth=0；bp split zero gain depth=0；求解器版本=f8e487148cc9 → bp lift first depth=<未设置>；bp split zero gain depth=<未设置>；求解器版本=89fc4d89dd02）。

成功实例：GAPMST-P32-LKH-BP-base=43，GAPMST-P32-LKH-same=43，共同成功=43；右侧新增=0，右侧丢失=0。

| 指标 | 配对数 | 左侧总量 | 右侧总量 | 右侧相对变化 | 右侧下降/持平/上升 |
| --- | ---: | ---: | ---: | ---: | ---: |
| 运行时间 | 43 | 1,532.479 s | 1,574.102 s | +2.72% | 2/0/41 |
| 根下界 | 43 | 898,754 | 898,754 | 0.00% | 0/43/0 |
| 分支创建数 | 43 | 2,612,950 | 2,612,950 | 0.00% | 0/43/0 |
| 初始/自适应 CLK 耗时 | 43 | 2.828 s | 2.875 s | +1.67% | 14/0/29 |
| 根势上升耗时 | 43 | 712.811 ms | 722.833 ms | +1.41% | 32/0/11 |
| LKH provider 调用数 | 43 | 43 | 43 | 0.00% | 0/43/0 |
| LKH provider 失败数 | 43 | 0 | 0 | 0.00% | 0/43/0 |
| LKH provider 耗时 | 43 | 3.224 s | 3.197 s | -0.83% | 19/0/24 |
| 根 fixing 耗时 | 43 | 20.218 ms | 20.415 ms | +0.97% | 18/10/15 |
| 节点势更新耗时 | 43 | 1,401.604 s | 1,440.174 s | +2.75% | 8/10/25 |
| 势 epoch 重建耗时 | 43 | 98.125 s | 99.235 s | +1.13% | 6/14/23 |
| replacement 查询耗时 | 43 | 12.999 s | 13.690 s | +5.32% | 5/10/28 |
| 节点势更新跳过总数 | 43 | 0 | 0 | 0.00% | 0/43/0 |

结果一致性：共同成功且有结果的实例均一致。
参考校验：左侧检查 43 个，右侧检查 43 个；均与参考结果一致。
