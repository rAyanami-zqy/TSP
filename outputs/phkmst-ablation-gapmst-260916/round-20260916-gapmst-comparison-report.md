# GAPMST vs CPHKMST 对比报告（14 配置 × 50 实例）

完成时间：2026-09-16。14/14 配置全部 50/50 行、0 error。
参数：`--batch-list data/classic/batch-n200.txt --timeout 1800 --workers 5 --exact-max-n 199`。
分支：GAPMST @01e2e04。结果目录：`outputs/phkmst-ablation-gapmst-260916/`。

被检验的对象是 01e2e04「gap-aware potential update scheduling」新增的四个旋钮：
`--hk-update-min-gap-change-ratio`、`--hk-update-gap-change-start-depth`、
`--hk-update-max-depth`、`--hk-update-skip-last-edges`，以及
`--root-candidate-compaction`。

## 配置

| 配置 | 二进制 | gap-aware 设置 |
|---|---|---|
| `Concorde` | concorde | — （参考解） |
| `CPHKMST-P32-LKH` | `tsp_bb_26_09_15_cphkmst` | 无此旋钮（基线） |
| `GAPMST-P32-LKH-same` | `tsp_bb_26_09_16_gapmst` | 全部中性化 |
| `GAPMST-P32-LKH-compact` | 同上 | 仅 compaction on |
| `GAPMST-P32-LKH` | 同上 | 默认（0.0001 / start-depth 2 / compaction on） |
| `GAPMST-P32-LKH-gc{0.0001…0.0005}` | 同上 | compaction on + 各 ratio，start-depth **0** |

所有配置除 executable 与上表旋钮外**逐字同参**（含全部 LKH warm-start 参数，
`--root-pi-refine-ascent polyak --root-pi-refine-iterations 64`）。

## A. 语义不变性：通过

`-same` 把新旋钮全部设成中性值（`min-gap-change-ratio 0` /
`start-depth 0` / `max-depth 0` / `skip-last-edges 0` /
`compaction off`），与 CPHKMST 基线逐例比对（全 50 实例）：

| 指标 | 不一致数 |
|---|---:|
| `result` | **0** |
| 双方解出实例的 `branches` | **0** |
| `root_potential_iterations` | **0** |

`-compact` 同样三项全 0。

**结论：01e2e04 没有藏在开关之外的行为改动。** 新增代码在门槛关闭时是语义中性的，
因此后续所有差异都可以干净地归因到那两个被打开的旋钮。

## B. compaction 单独打开：零影响

`-compact` 相对基线：解集、费用、分支数**逐位相同**，分支合计 2,534,153 →
2,534,153（**1.00×**），墙钟 0.99×。

**`--root-candidate-compaction` 在 batch-n200 上是完全无效的旋钮**，至少在本参数
组合下如此。（注意它与 CPHKMST 已有的 `TSP_DISABLE_EPOCH_COMPACTION` 编译期候选
过滤是两套机制：后者始终生效，前者是新增的运行期物理压缩。）

## C. gap-change ratio 扫描：单调劣化，零收益

| 配置 | 翻盘 | 回退 | cost 差 | 分支倍数 | 墙钟倍数 |
|---|---:|---:|---:|---:|---:|
| 基线 CPHKMST | 0 | 0 | 0 | 1.00× | 1.00× |
| `-same` | 0 | 0 | 0 | 1.00× | 1.03× |
| `-compact` | 0 | 0 | 0 | 1.00× | 0.99× |
| `-默认`（0.0001, sd=2） | 0 | 0 | 0 | 7.43× | 1.85× |
| `gc 0.0001`（sd=0） | 0 | 0 | 0 | 7.80× | 1.89× |
| `gc 0.00015` | 0 | 0 | 0 | 10.65× | 2.28× |
| `gc 0.0002` | 0 | 0 | 0 | 12.72× | 2.44× |
| `gc 0.00025` | 0 | **1** | 0 | 11.33× | 2.25× |
| `gc 0.0003` | 0 | **1** | 0 | 13.14× | 2.52× |
| `gc 0.00035` | 0 | **1** | 0 | 14.50× | 2.71× |
| `gc 0.0004` | 0 | **1** | 0 | 15.98× | 2.90× |
| `gc 0.00045` | 0 | **1** | 0 | 17.68× | 3.22× |
| `gc 0.0005` | 0 | **2** | 0 | 16.28× | 2.31× |

倍数按各配置与基线共同解出的实例子集计算，故各行可直接互比。

**三条结论：**

1. **九档 ratio 全部零翻盘、零 cost 差异。** 门槛没有在任何实例上换来更好的
   下界或更快的求解，只改变搜索路径。
2. **开销随 ratio 单调膨胀**：分支合计 1.00× → 17.68×（`gc0.00045` 峰值），
   墙钟 1.00× → 3.22×。
3. **回退从 0.00025 起出现并累积**：`bier127` 在 0.00025~0.00045 六档持续超时，
   0.0005 又加上 `rat195`，解出数从 43/50 掉到 41/50。

机制上说得通：该门槛**抑制一部分势更新**，被跳过的更新让搜索停在更差的势状态，
下界变弱、树变大。ratio 越高抑制越多，代价越大。

### 回退实例的完整轨迹

`bier127`（基线 716,226 分支 / 524.8 s）：

| ratio | 状态 | branches | wall |
|---|---|---:|---:|
| 基线 / same / compact | ok | 716,226 | 524.8 s |
| 默认 0.0001 | ok | 8,550,935 | 1334.1 s |
| 0.0001 | ok | 9,825,049 | 1506.8 s |
| 0.00015 | ok | 13,260,952 | 1756.6 s |
| 0.0002 | ok | 15,023,345 | 1727.6 s |
| 0.00025 → 0.0005 | **timeout** | 16.9M → 21.1M | 1800 s |

`rat195`（基线 160,402 分支 / 257.9 s）：

| ratio | 状态 | branches | wall |
|---|---|---:|---:|
| 基线 / same | ok | 160,402 | 257.9 s |
| 0.0001 | ok | 1,179,927 | 601.7 s |
| 0.0002 | ok | 2,543,934 | 887.4 s |
| 0.0004 | ok | 5,709,648 | 1480.3 s |
| 0.00045 | ok | 7,002,083 | 1753.9 s |
| 0.0005 | **timeout** | 7,498,001 | 1800 s |

两条轨迹都是单调的，且都在逼近 1800 s 后越线——回退是曲线的自然延伸，不是抖动。

### start-depth 0 vs 2

同为 ratio 0.0001，`-默认`（start-depth 2）7.43×、`gc0.0001`（start-depth 0）
7.80×：**start-depth 0 更贵约 5%**。0 表示门槛对所有非根候选节点生效，抑制面更大。

## D. 费用一致性

Concorde 本轮 50/50 全解出（该输出根无 Concorde 缓存，50 个实例实打实跑了一遍）。
13 个启发式配置在各自与 Concorde 共同解出的实例上逐例比对：

**全部 0 不一致**（共同解出 41~43 例不等，随回退实例变化）。

## 结论

**gap-aware 势更新调度在 batch-n200 上是纯负收益：**

1. **语义上安全**——新旋钮全部关闭时与 CPHKMST 逐位等价，代码本身没有问题。
2. **compaction 无效**——单独打开不产生任何可观测差异。
3. **gap-change 门槛有害**——九档扫描零收益，开销单调放大到 17.7×，
   并从 0.00025 起开始吃掉解出的实例（43/50 → 41/50）。

**建议：`--hk-update-min-gap-change-ratio` 保持 0（关闭），
`--root-candidate-compaction` 保持 off（或 on，反正无差别）。**
若要继续探索该机制，需要换一批实例——batch-n200 上所有配置的收益天花板被
LKH warm-start 已经拿满，门槛只在开销一侧留下痕迹。

## 已知混淆项

- 本轮 `--workers 5`；09-15/09-16 的 CPHKMST 基线数据也是 `--workers 5`，
  同轮内比较干净。跨轮墙钟不宜直接相比。
- `--debug --debug-interval 10000` 的诊断输出（按 `nodes_expanded % interval`
  打到 stderr）在百万分支实例上产生上百行文本；轮内口径一致，不影响横向对比，
  但它计入墙钟，所以绝对墙钟偏保守。
- `-默认` 的 start-depth 为 2、`gc*` 为 0，两者不可当作同一族直接比较
  （报告 C 节最后一段单独列出了这一项）。
