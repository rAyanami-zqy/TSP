# MST BP Singleton 传播实验记录

日期：2026-07-23
分支：`MST`
基准提交：`fe2b166`（`Optimize MST replacement and candidate tracking`）

## 1. 实验目的与范围

本轮目标是在 MST 版本上尝试基于树边 best replacement 的 BP 划分与
`x_e=1` 强制传播，并确认能否获得实际运行时间提升。

实验使用 `docs/strategy-comparison-results.md` 中可在较短时间内精确求解的
实例，不使用不适合反复测试的大实例。所有正式结果均基于不含 Held-Karp
节点势的 MST 实现。

MST 工作树在本轮开始前已经包含“根节点树边 reduced-cost fixing”的未提交
修改。本轮最终补丁叠加在该最新工作树上。HKMST 工作树及其现有过滤实现
没有被修改。

## 2. 最终保留方案

最终没有保留额外的 replacement-cover 预扫描，而是直接利用原 BP 已经生成
的严格证书：

```text
如果 BP 返回 B 且 |B| = 1：
    所有严格改善 incumbent 的 tour 都必须使用唯一边 e
    因此直接传播 x_e = 1
    在当前逻辑节点更新候选集和 1-tree
    继续搜索，不创建只有一个孩子的 BP 节点
```

实现要点：

1. `search` 增加 `count_node` 参数。
2. singleton 传播后的递归仍处于同一逻辑节点和深度，使用
   `search(..., false)`，不重复统计 expanded 节点。
3. 强制边后执行既有的度数、子回路和候选过滤。
4. 优先增量修复当前 1-tree；增量修复失败时回退完整重建。
5. 返回父节点前完整回滚 1-tree、候选 bitset、forced DSU、度数和缓存成本。
6. 验证构建继续用完整重建结果检查增量状态。

安全性来自原 BP 终止条件。`B={e}` 已经证明：

```text
所有严格改善 incumbent 的 tour 必须满足 x_e >= 1
```

由于 `x_e` 为 0/1 变量，因此可以直接推出 `x_e=1`。该优化不改变 BP
分支完备性，只消除一个必然只有单个孩子的搜索节点。

最终新增代码主要位于：

- `include/TspSolver.hpp`：`search` 增加 `count_node` 参数。
- `src/TspSolver.cpp`：在多路分支前处理 `B_set.size() == 1`。

## 3. 正式测试实例

```text
data/classic/tsplib/burma14.tsp
data/classic/tsplib/gr21.tsp
data/classic/national/wi29.tsp
data/classic/national/dj38.tsp
data/classic/tsplib/gr24.tsp
data/classic/tsplib/fri26.tsp
data/classic/tsplib/gr17.tsp
```

测试参数：

```text
Release 构建
--exact-max-n 100
每个实例、每个版本运行 15 次
基线和修改版交错运行，降低温度与运行顺序偏差
```

MST 最新工作树修改前的冻结基线二进制：

```text
/private/tmp/tsp-mst-tree-fixing-before-bp-singleton
```

修改后测试二进制：

```text
/private/tmp/tsp-mst-tree-fixing-build/tsp_bb
```

上述 `/private/tmp` 路径只用于本次实验，可能不会跨环境保留。可按后文命令
重新构建。

## 4. 正式性能结果

下表为每个实例 15 次运行的平均 wall time。

| 实例 | MST 最新基线 | Singleton 传播 | 变化 |
|---|---:|---:|---:|
| burma14 | 6.7 ms | 6.0 ms | -10.0%（短任务噪声较大） |
| gr21 | 12.7 ms | 12.7 ms | 0.0% |
| wi29 | 27.3 ms | 26.7 ms | -2.4% |
| dj38 | 12.0 ms | 12.0 ms | 0.0% |
| gr24 | 174.0 ms | 171.3 ms | -1.5% |
| fri26 | 349.3 ms | 344.7 ms | -1.3% |
| gr17 | 482.0 ms | 472.0 ms | -2.1% |
| **合计** | **1064.0 ms** | **1045.3 ms** | **-1.8%** |

长实例 `gr24`、`fri26`、`gr17` 均得到正收益，因此合计提升不是只由短任务
启动噪声造成。

## 5. 搜索规模变化

根节点树边 fixing 在这些实例上均得到 `fixed_one=0`，所以修改前后的搜索
规模变化来自 BP singleton 传播。

| 实例 | 基线 expanded | 修改后 expanded | 基线 created | 修改后 created |
|---|---:|---:|---:|---:|
| burma14 | 4,582 | 3,889 | 8,949 | 6,756 |
| gr21 | 11,402 | 9,015 | 19,252 | 13,071 |
| wi29 | 22,496 | 16,586 | 39,030 | 25,973 |
| dj38 | 7,977 | 5,577 | 12,893 | 7,917 |
| gr24 | 156,436 | 126,504 | 282,062 | 197,236 |
| fri26 | 295,025 | 236,141 | 564,736 | 406,049 |
| gr17 | 463,622 | 395,955 | 897,158 | 661,069 |
| **合计** | **961,540** | **793,667** | **1,824,080** | **1,318,071** |

汇总变化：

- expanded：减少 167,873，约 **17.5%**；
- created：减少 506,009，约 **27.7%**；
- wall time：提升约 **1.8%**。

节点下降明显大于时间下降，是因为 singleton 子节点虽然不再计为独立逻辑
节点，但强制、候选传播和下一轮 BP 计算仍然需要执行。实际节省主要来自
省去子节点创建、父子状态切换和部分重复检查。

## 6. 已尝试但未保留的方案

下表中的时间变化为相对各自对应基线的方向性结果；只有最终 singleton
传播使用了上面的正式 15 次逐实例测试。

| 方案 | 结果 | 结论 |
|---|---:|---|
| 全候选 best-replacement cover | 约慢 7.1% | 节点大幅下降，但每节点批量割成本过高 |
| 只保留 replacement singleton fixing | 仍慢约 1.9% | 预扫描成本仍未收回 |
| 仅处理候选数不超过 2 | 约慢 6.1% | cover 成本与传播收益不平衡 |
| 仅处理唯一候选，不复用 fallback 首边 | 约慢 4.1% | 失败后原 BP 重复扫描 |
| fallback 首边与第一 replacement 复用 | 比对应版本快约 3% | 有效，但整体仍不如取消预扫描 |
| 最大收益形式反向加边精简 B | 约慢 1.8% | 精简命中少，额外 1-tree/插边成本不值 |
| 自适应完整重建，阈值 2 | 约慢 23.4% | 重建过于频繁 |
| 自适应完整重建，阈值 3 | 约慢 2.1% | 仍为负收益 |
| 自适应完整重建，阈值 4 | 基本持平 | 无收益且增加复杂度 |
| 度违规同权 tie-break | 约慢 1.3% | 个别实例节点下降，但总体退化 |
| 度违规作为主排序 | 约慢 6.3% | created 节点明显增加 |
| singleton 候选线性取最大值替代排序 | 约慢 0.5% | 候选通常只有 2～4 条，小排序并非瓶颈 |
| **原 BP singleton 直接传播** | **快约 1.8%** | **最终保留** |

核心结论：

```text
不要为发现 singleton 证书再做一次 replacement 预扫描。
直接消费原 BP 已经计算出的 |B|=1 证书更轻、更稳定。
```

## 7. 构建与复现命令

Release 构建：

```bash
cmake -S /private/tmp/tsp-mst-tree-fixing \
      -B /private/tmp/tsp-mst-tree-fixing-build \
      -DCMAKE_BUILD_TYPE=Release
cmake --build /private/tmp/tsp-mst-tree-fixing-build -j 8 \
      --target tsp_bb tsp_solver_tests tsp_bb_variants
ctest --test-dir /private/tmp/tsp-mst-tree-fixing-build \
      --output-on-failure
```

批量求解：

```bash
/private/tmp/tsp-mst-tree-fixing-build/tsp_bb \
    --exact-max-n 100 \
    --batch /private/tmp/tsp-mst-seven-small.txt
```

Sanitizer 构建：

```bash
cmake -S /private/tmp/tsp-mst-tree-fixing \
      -B /private/tmp/tsp-mst-tree-fixing-sanitize \
      -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_CXX_FLAGS='-fsanitize=address,undefined -fno-omit-frame-pointer'
cmake --build /private/tmp/tsp-mst-tree-fixing-sanitize -j 8 \
      --target tsp_bb tsp_solver_tests
ctest --test-dir /private/tmp/tsp-mst-tree-fixing-sanitize \
      --output-on-failure
ASAN_OPTIONS=halt_on_error=1 \
UBSAN_OPTIONS=halt_on_error=1 \
/private/tmp/tsp-mst-tree-fixing-sanitize/tsp_bb \
    --exact-max-n 100 \
    --batch /private/tmp/tsp-mst-seven-small.txt
```

macOS 当前 ASan 不支持 `detect_leaks=1`，因此完整实例验证使用
`halt_on_error=1`，未启用 leak sanitizer。

## 8. 验证结果

最终叠加到 MST 最新工作树的版本已经通过：

- 默认 Release 求解器构建；
- `tsp_bb_variants` 全部四个变体构建；
- `tsp_solver_tests`；
- 三个 CLI CTest；
- 7 个小实例的最优成本和 tour 验证；
- `TSP_VERIFY_INCREMENTAL_STATE` 下的完整重建一致性检查；
- 7 个实例的 ASan/UBSan 完整运行。

所有实例的最优成本与修改前一致：

```text
burma14 = 3323
gr21    = 2707
wi29    = 27603
dj38    = 6656
gr24    = 1272
fri26   = 937
gr17    = 2085
```

## 9. 后续建议

1. 下一轮性能实验应以当前 singleton 传播版本为新基线。
2. 不建议恢复全节点 replacement-cover 预扫描。
3. 若继续加强，可只研究不增加额外 1-tree/割扫描的传播，例如复用 BP
   已生成的多边前缀信息。
4. 新策略必须同时报告 wall time 和搜索节点；仅减少节点不足以证明优化。
5. 正式提交前应确认 MST 工作树中根节点树边 fixing 与本补丁一起提交，
   或将两项拆成独立提交便于回退。

## 10. 2026-07-24 热路径优化

### 10.1 对比基线与方法

本轮以干净的 MST 提交 `fe2b166` 为未修改基线，与包含根 tree-edge
fixing、BP singleton 传播及本轮热路径优化的工作树比较。两边均使用
AppleClang Release 构建。

每个实例预热 3 次，随后运行 30 轮；基线和候选版本逐轮交换先后顺序，
降低温度和运行顺序偏差。下表为 wall time 平均值：

| 实例 | 未修改 MST | 最终工作树 | 变化 |
|---|---:|---:|---:|
| burma14 | 6.532 ms | 5.100 ms | -21.9% |
| gr21 | 13.306 ms | 9.250 ms | -30.5% |
| wi29 | 27.975 ms | 18.885 ms | -32.5% |
| dj38 | 12.827 ms | 9.936 ms | -22.5% |
| gr24 | 176.452 ms | 109.472 ms | -38.0% |
| fri26 | 353.692 ms | 226.209 ms | -36.0% |
| gr17 | 480.357 ms | 299.555 ms | -37.6% |
| **合计** | **1071.140 ms** | **678.406 ms** | **-36.7%** |

最终版本在 7 个实例上均有提升。三个长实例提升约 36%～38%，总收益不是
短任务启动噪声造成。

### 10.2 最终保留的实现

1. `n <= 64` 的动态 MST component 查询改用两个 `seen/frontier`
   64-bit bitset，从删除边两端交替扩展。
2. 选中的较小 MST component 直接保留为 bit mask；fundamental cut
   查询直接消费该 mask，不再复制 component vector 或逐顶点写 epoch。
3. `n <= 64` 的 forced DSU 成员集合使用 64-bit bitset，union/rollback
   为 O(1)，singleton 过滤直接枚举较小分量。
4. BP 集使用 4 项 inline `BranchSet`；只有更长 BP 链才分配 heap vector。
   同时直接从 BP 集恢复 forbid 状态，删除两份重复日志 vector。
5. `updateOneTreeAfterActiveRemoval` 在本轮删除候选少于树边数时只检查
   candidate undo delta；删除较多时才扫描完整 1-tree。
6. 删除一个顶点的全部 incident candidates 时，该顶点的可用度数一次
   批量更新，另一端和 undo 顺序仍逐边维护。
7. `n > 64` 保留原通用 component epoch/vector 与 forced-members 路径。

这些修改没有改变 7 个正式实例的最优成本、tour 或搜索节点统计；收益来自
减少同一搜索树上的 component、状态维护和分配成本。

### 10.3 尝试后回退的方案

| 方案 | 相对当时基线 | 结论 |
|---|---:|---|
| candidate undo 按 64-bit word 合并 | 慢约 0.8% | 恢复仍需逐 bit 更新，较大的 undo 记录得不偿失 |
| 为所有 forced-members vector 预留 n 项 | 30 轮快约 0.1% | 无稳定收益且增加 O(n²) 预分配，回退 |

### 10.4 最终验证

- 默认 Release 与全部 4 个 solver variants 构建；
- 4 个 CTest；
- 7 个正式实例，成本、tour、搜索节点均与优化前工作树一致；
- 仓库 16 个和额外生成 200 个小实例通过独立 Held-Karp 对拍；
- ASan/UBSan 下的 4 个 CTest 和 7 个正式实例；
- `n=65` 稀疏回归，覆盖通用 MST component/forced-members 回退路径；
- `git diff --check`。

## 11. 2026-07-24 best-replacement cover 复测

### 11.1 实现

在已保存的有效基线 `f36d206` 上重新适配 2026-07-23 的完整方案：

1. 找到当前最高度违规顶点。
2. 一次遍历 MST，为该顶点关联的全部未决内部树边标记互不相交的基本割。
3. 利用 active candidate 位图，为每条树边预计算 best replacement 和安全
   replacement delta。
4. 按 delta 降序选择能够闭合 incumbent gap 的最短 cover。
5. 用实际受约束的 prefix 1-tree 验证 cover，并生成搜索阶段可直接重放的
   replacement delta；验证不闭合则回退原顺序 BP。
6. 第一个 prefix 直接复用预计算的 replacement，避免重复割查询。

验证构建仍以逐边 `findMstReplacement` 和完整 `computeOneTree` 作为预言机。
4 个 CTest 全部通过。

### 11.2 正式性能结果

以 `f36d206` 为基线，每个实例预热 2 次、交错运行 15 次：

| 实例 | `f36d206` | 完整 replacement cover | 变化 |
|---|---:|---:|---:|
| burma14 | 5.539 ms | 6.218 ms | +12.3% |
| gr21 | 8.596 ms | 10.310 ms | +19.9% |
| wi29 | 18.405 ms | 22.708 ms | +23.4% |
| dj38 | 9.301 ms | 11.421 ms | +22.8% |
| gr24 | 109.291 ms | 137.186 ms | +25.5% |
| fri26 | 225.037 ms | 277.942 ms | +23.5% |
| gr17 | 299.773 ms | 358.991 ms | +19.8% |
| **合计** | **675.942 ms** | **824.777 ms** | **+22.0%** |

搜索规模没有同比例下降：

| 实例 | 基线 created / expanded | cover created / expanded |
|---|---:|---:|
| fri26 | 406,049 / 236,141 | 383,490 / 236,159 |
| gr17 | 661,069 / 395,955 | 618,575 / 397,966 |

cover 减少了约 6% 的 created 节点，但 expanded 基本不变或略增。3 秒采样中
`tryBestReplacementCover` 自身占约 22.7% CPU，预计算成本没有被节点收益
抵消。

### 11.3 自适应门槛

还测试了只在下界接近 incumbent 时启用 cover：

| 启用窗口 | 7 实例合计变化 |
|---|---:|
| gap 不超过 5% | +20.4% |
| gap 不超过 1% | +9.5% |
| gap 不超过 0.2% | +4.1% |

收窄窗口只能减少退化，没有出现净收益。因此有效版本仍保持
`f36d206`；完整 cover 实现保存在 `TMST` 实验分支，不替代 MST 基线。

### 11.4 预计算路径优化

完整 cover 的选择本身已经是最大收益优先：所有 `safe_delta` 按降序排列，
再取累计 delta 首次闭合 incumbent gap 的最短前缀。继续优化的重点因此
不是 B 集排序，而是减少没有回报的预计算。

本轮依次加入：

1. 先只查询原 degree-BP 第一条边；若单条 replacement delta 已闭合，直接
   返回 singleton B，不构造批量割。
2. 第一条边不能闭合时复用已取得的 replacement，避免 fallback 重复查询。
3. 原 BP 第一条为根边时跳过内部 MST cover。
4. 用第一条 delta 预测收益：只有剩余 gap 不超过第一条 delta 的 10% 时，
   才继续预计算其余树边。
5. 两候选场景逐边查询第二条 replacement；至少三条时才建立共享批量割。
6. 仍按 `safe_delta` 降序最大化收益，并以真实 prefix 1-tree 验证最终 B。

各阶段相对 `f36d206` 的 7 实例快速交错结果：

| 版本 | 合计变化 |
|---|---:|
| 完整全节点 cover | +22.0% |
| 第一条 singleton gate | +11.1% |
| 剩余 gap ≤ 2×第一 delta | +5.0% |
| 剩余 gap ≤ 1×第一 delta | +4.0% |
| 剩余 gap ≤ 0.5×第一 delta | +1.8% |
| 两候选改用逐边查询 | +1.2% |
| 剩余 gap ≤ 0.1×第一 delta | 快速测试 -0.3% |

最终 0.1 门槛版本预热 3 次、交错运行 30 轮：

| 实例 | `f36d206` | 自适应 cover | 变化 |
|---|---:|---:|---:|
| burma14 | 5.050 ms | 5.136 ms | +1.7% |
| gr21 | 9.543 ms | 8.859 ms | -7.2% |
| wi29 | 18.788 ms | 18.807 ms | +0.1% |
| dj38 | 9.512 ms | 9.451 ms | -0.6% |
| gr24 | 108.518 ms | 108.451 ms | -0.1% |
| fri26 | 225.830 ms | 225.816 ms | 0.0% |
| gr17 | 300.792 ms | 298.432 ms | -0.8% |
| **合计** | **678.034 ms** | **674.952 ms** | **-0.46%** |

最终版本 created 略降但 expanded 略升。例如 `gr17` 的 created 从
661,069 降至 657,749，expanded 从 395,955 升至 396,441。当前约 0.5%
收益较小，仍应视为 `TMST` 实验结果；正式替代 `f36d206` 前需要更多实例
验证。
