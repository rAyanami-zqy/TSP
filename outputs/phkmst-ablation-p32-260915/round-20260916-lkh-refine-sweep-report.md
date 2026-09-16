# LKH warm-start 精修预算/策略扫描报告（4 配置 × 50 实例）

完成时间：2026-09-16。4/4 配置全部 50/50 行、0 error。
参数：`--batch-list data/classic/batch-n200.txt --timeout 1800 --workers 5 --exact-max-n 199`
（起跑用 `--workers 3`，配置 1 跑到 26/50 时按要求重启为 5；因 runner 的
`strategy_identity` 把 `workers` 计入指纹，重启后四个配置都换了 run_id，
26 行的旧目录 `CPHKMST-P32-LKH128-9076aa647de2` 作废、不计入本报告。
被丢弃的工作量很小——那 26 行合计仅 74 秒求解时间）。

二进制与 LKH provider 与 09-15 那轮完全一致（`tsp_bb_26_09_15_cphkmst`，
摘要 `c968ee87…`；`build-lkh/tsp_lkh_provider`），本轮唯一变量是
`--root-pi-refine-*`。

## 四个配置

| # | 配置 | 精修策略 | 轮数 | run_id |
|---|---|---|---:|---|
| 1 | `CPHKMST-P32-LKH128` | polyak | 128 | `…-7b019689c923` |
| 2 | `CPHKMST-P32-LKH64-PS` | polyak-smoothed | 64 | `…-c76a39803435` |
| 3 | `CPHKMST-P32-LKH128-PS` | polyak-smoothed | 128 | `…-879f942a69f7` |
| 4 | `Concorde` | — | — | `Concorde-07deea4ce57e` |

除 `--root-pi-refine-*` 外，三者的参数与 09-15 轮的 `CPHKMST-P32-LKH`
（polyak/64，`…-4ad05ce5fb20`）逐字相同。平滑权重沿用默认 0.7。

## 总览（50 实例）

中位 wall 为 runner summary.csv 口径（仅统计成功调用，不含超时的 1800 s）。

| 配置 | solved | 超时 | 中位 wall | 中位分支 |
|---|---:|---:|---:|---:|
| Concorde | **50/50** | 0 | 0.156 s | 1 |
| classic-P32（09-15） | 40/50 | 10 | 0.322 s | 480.5 |
| LKH polyak/64（09-15） | 43/50 | 7 | 0.147 s | 39 |
| LKH polyak/128 | 43/50 | 7 | 0.169 s | 25 |
| LKH PS/64 | 43/50 | 7 | 0.180 s | 39 |
| LKH PS/128 | 43/50 | 7 | 0.169 s | 93 |

## A. 费用一致性 vs Concorde

Concorde 本轮 50/50 全解出（该输出根此前无 Concorde 缓存，50 个实例实打实
跑了一遍）。各配置在双方均解出的实例上逐例比对：

| 配置 | 共同解出 | 费用不一致 |
|---|---:|---:|
| classic-P32 | 40 | **0** |
| LKH polyak/64 | 43 | **0** |
| LKH polyak/128 | 43 | **0** |
| LKH PS/64 | 43 | **0** |
| LKH PS/128 | 43 | **0** |

## B. 三个 LKH 变体之间：完全没有区别

任意两两对比（全 50 实例）都是 **0 超时翻盘、0 回退、0 cost 差异**：

| 对比 | 超时→解出 | 解出→超时 | cost 差异 |
|---|---:|---:|---:|
| polyak/64 → polyak/128 | 0 | 0 | 0 |
| polyak/64 → PS/64 | 0 | 0 | 0 |
| PS/64 → PS/128 | 0 | 0 | 0 |
| polyak/128 → PS/128 | 0 | 0 | 0 |

四个 LKH 变体在 43 个共同解出实例上 **result 逐位全等**。分支数只有 22/43
全等（说明搜索路径确实不同，但都收敛到同一结论），墙钟则无一全等（噪声）。

**结论：在 LKH warm-start 这条线上，把精修轮数从 64 提到 128、或把精修策略
从 polyak 换成 polyak-smoothed，对求解结果没有任何影响。** 精修预算在 64 就
已饱和，平滑策略亦无增益。这是干净的负结果。

## C. 唯一有效的变量：用不用 LKH warm-start

classic-P32 → LKH（任一档）：**+3 解出、0 回退、0 cost 变差**。

| 实例 | classic-P32 | LKH（四档一致） |
|---|---|---|
| bier127 | 超时 1800 s | 534.4 s |
| brg180 | 超时 1800 s | 0.32 s |
| pr136 | 超时 1800 s | 680.2 s |

两侧共同解出的 40 个实例上，LKH 的中位 wall 为 0.133–0.153 s（classic-P32
0.322 s），中位分支 23–66（classic-P32 480.5）——**又更快、又解得更多**。

## 结论

1. **LKH PI warm-start 的收益已经到顶**：三档精修变体（polyak/128、
   PS/64、PS/128）与已有的 polyak/64 逐位等价，没有进一步调参空间。
   要再提升求解率，得从 LKH 本身的预算（`--lkh-runs`/`--lkh-max-trials`）
   或 warm-start 之外的机制入手。
2. **polyak/64 就是该线上的最优配置**（结果与其他三档相同，且是参数最简单的
   一档）。09-15 轮选它作默认是对的。
3. 尚未被 LKH 攻克的 7 个实例：`d198`、`kroB150`、`pr107`、`pr152`、
   `qa194`、`si175`、`xqf131`——与 classic-P32 的 10 个超时相比只少了 3 个，
   这 7 个是下一轮真正要啃的目标。

## 已知混淆项

- 本轮的墙钟数据在 `--workers 5` 下测得，09-15 那三轮是 `--workers 3`。
  结论 B/C 都建立在 result 逐位比对之上，不受并行度影响；但**跨轮的墙钟数值
  不宜直接相比**（同轮内比较是干净的）。
- 与 09-15 轮相同的 `--debug --debug-interval 10000` 混淆项依然存在：
  诊断输出按 `nodes_expanded % interval` 打到 stderr，百万分支实例会产生上百行。
  轮内三者口径一致，横向对比不受影响。
