# 经典 P32 三配置分支对比报告（3 配置 × 50 实例）

完成时间：2026-09-15。3/3 配置全部 50/50 行，0 error。
参数：`--batch-list data/classic/batch-n200.txt --timeout 1800 --workers 3 --exact-max-n 199`。
日志：`outputs/phkmst-ablation-p32-260915/`（本次输出根）。

## 二进制与配置

本轮的三个配置各用**不同分支编译出的独立二进制**，二进制摘要参与缓存指纹，
因此不会互相误用缓存记录。

| 配置 | 二进制 | 分支 | 分支摘要 | 参数 |
|---|---|---|---|---|
| PHKMST-P32 | `build/tsp_bb_26_09_15_phkmst` | PHKMST @97197e7 | `28b3835c…` | 经典 P32 |
| CPHKMST-P32 | `build/tsp_bb_26_09_15_cphkmst` | CPHKMST @e1cbf95 | `c968ee87…` | 与上行逐字同参 |
| CPHKMST-P32-LKH | 同上 | CPHKMST @e1cbf95 | `c968ee87…` | P32 + LKH warm-start |

LKH provider 由 LKH 2.0.11 源码（`/home/wj/code/LKH-2.0.11`，仓库外，依其
research-use 许可不转载）经 `-DTSP_LKH_SOURCE_DIR` 编译为
`build-lkh/tsp_lkh_provider`。三个配置的 run_id：
`PHKMST-P32-12e74ded71f6` / `CPHKMST-P32-ca5488b3b33a` /
`CPHKMST-P32-LKH-4ad05ce5fb20`。

`CPHKMST-P32-LKH` 的 LKH 参数取 `lkh-pi-smooth-sweep-20260915` 结论中"默认采用
warm-start + polyak64"那一档：`--lkh-pi-mode warm-start`、
`--root-pi-refine-ascent polyak`、`--root-pi-refine-iterations 64`、
`--lkh-runs 1 --lkh-max-trials 0`（0 = 实例维数）、`--lkh-seed 123`、
`--lkh-provider-failure error`。因 `src/main.cpp` 的 `refine_external_potentials`
分支会用 `--root-pi-refine-*` 整体覆盖 `--hk-ascent`，该配置特意不写
`--hk-ascent`，以免留下不生效的参数；节点上升与子树势更新仍沿用 P32 参数。

## 总览

| 配置 | solved | 超时 | 中位 wall | 中位分支 | 成功调用合计 wall |
|---|---:|---:|---:|---:|---:|
| PHKMST-P32 | 40/50 | 10 | 0.340 s | 480.5 | 3850.4 s |
| CPHKMST-P32 | 40/50 | 10 | 0.322 s | 480.5 | 2678.5 s |
| CPHKMST-P32-LKH | **43/50** | **7** | **0.147 s** | **39** | **1716.2 s** |

## A. 回归检查：PHKMST-P32 vs CPHKMST-P32

**result 逐例一致 50/50**，超时集合完全相同（bier127/brg180/d198/kroB150/
pr107/pr136/pr152/qa194/si175/xqf131）。

CPHKMST 的新代码（LKH 隔离、root fixing、candidate/pricing 等）**没有污染非
LKH 的经典路径**：同参数下两个二进制的费用逐位一致，中位 wall 的差异
（0.340 → 0.322 s）在噪声量级。这为 LKH 那条线提供了干净基线。

## B. LKH 收益：CPHKMST-P32 vs CPHKMST-P32-LKH

| 重叠 50 实例 | 数量 | 实例 |
|---|---:|---|
| **超时 → 解出** | **3** | bier127、brg180、pr136 |
| 解出 → 超时 | **0** | — |
| 双方都解出但 cost 变差 | **0** | — |

三例翻盘明细：

| 实例 | P32 | LKH |
|---|---|---|
| bier127 | 超时 1800 s | 534.4 s，cost 118282 |
| brg180 | 超时 1800 s | **0.32 s**，cost 1950 |
| pr136 | 超时 1800 s | 680.2 s，cost 96772 |

LKH 剩余 7 个超时：d198、kroB150、pr107、pr152、qa194、si175、xqf131。

## C. 根下界机制（不是全面碾压）

在 40 个双方都解出的实例上，根下界并非单调变好：

| 根下界 | 数量 | 合计变化 |
|---|---:|---:|
| 更高 | 14 | +809.03 |
| 持平 | 6 | 0 |
| 更低 | 20 | −85.47 |
| **总计** | | **+723.56** |

提升集中在少数难例（pr144 +548.8、brazil58 +193.0、kroA150 +27.5），损失则
幅度很小（pr76 −69.4 为最大，其余多在 −2 量级）。

原因在预算口径不同：LKH 那条用外部 PI 作 warm start 后**只做 64 轮**精修
（中位根势迭代 64），而纯 P32 的本地根上升用满预算（中位 333.5 轮）。因此
**中位实例上 P32 的根下界反而略高**（6885.12 vs 6884.48），LKH 的收益集中在
难例尾部——正是那三例翻盘的来源。把 LKH 的收益描述成"根下界普遍更高"是错的。

## D. 开销

LKH provider 的调用时间未进 CSV（runner 的 `OUTPUT_STATISTICS` 无
`lkh_provider_*` 列），但总 wall 是外部计时的，已包含该开销。成功调用合计
wall 从 2678.5 s 降到 1716.2 s（−36%），因为少撞 3 次 1800 s 超时；中位 wall
降 54%、中位分支数从 480.5 降到 39（−92%），符合"根下界更好 → 剪枝更早"的预期。

## 结论

1. CPHKMST 的新代码在经典 P32 路径上与 PHKMST **完全等价**（50/50 逐例一致），
   可以放心把 CPHKMST 作为后续 LKH 实验的基线。
2. LKH PI warm-start + 64 轮精修在 batch-n200 上**净 +3 解出、零回退**，且总
   墙钟反而下降，是目前唯一同时改善求解率与耗时的改动。
3. 收益机制是补强难例尾部的根势，而非普遍抬升根下界；若要让中位实例也受益，
   需要把 `--root-pi-refine-iterations` 从 64 提高（扫描中 polyak128 的累计增益
   1484 高于 64 的 601，但那是纯根下界口径，全量求解下的最优预算未验证）。

## 已知混淆项

本轮三个配置都跑 `--debug --debug-interval 10000`（CPHKMST 分支 runner 的
`DEFAULT_DEBUG_INTERVAL` 自 commit 2de1669 起为 10_000），诊断输出按
`nodes_expanded % interval` 打到 stderr。**轮内三者口径一致，横向对比干净**；
但若与 `--debug-interval 5000000` 的历轮结果比较墙钟，需要先消除这一差异。
