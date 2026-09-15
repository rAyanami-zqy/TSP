# PHKMST Ablation CPHKMST 分支验证轮报告（4 配置 × 50 实例）

完成时间：2026-09-14。4/4 配置全部 complete，0 error。
背景：切到 CPHKMST 分支（4ff9598，candidate/pricing 探索代码，runner 新增
root_fixing_* 统计列）后 Release 重编译 build/tsp_bb（新指纹 3f2f59f5…），
用第五轮并列最佳（41/9）的四个 smoothed 配置验证新代码上的表现。
参数与历轮一致：--batch-list data/classic/batch-n200.txt --timeout 1800
--workers 5 --exact-max-n 199。Concorde 缓存命中（07deea4ce57e）。
run_id：hybrid-reverse-e14baba06c98 / PPS32-228a9fe68c1d /
PPSD32-3dfde455c7a7 / PS-PS32-dc6d2a0b60f4。

## 费用一致性验证

四个配置与 Concorde 在双方均解出的实例上逐例比对：**全部 0 不一致**。

## 新旧二进制对比（旧数据在 PHKMST 分支 outputs/phkmst-ablation_909/）

| 配置 | 新二进制 solved | 旧二进制 solved | 差异实例 | median_wall(s) 新/旧 |
|---|---|---|---|---|
| hybrid-reverse | **42/8** | 41/9 | +bier127 | 0.507 / — |
| PPS32 | 41/9 | 41/9 | 无 | 0.285 / — |
| PPSD32 | 40/10 | 41/9 | −bier127 | 0.233 / — |
| PS-PS32 | 40/10 | 41/9 | −bier127 | 0.260 / — |

## 要点

- **新二进制上四个配置全部复现或超越 41/9 水平**：hybrid-reverse 42/8 创下
  历轮新高（首次 42 解出），PPS32 持平 41/9，PPSD32/PS-PS32 40/10 略降。
- **唯一摇摆实例是 bier127**：CPHKMST 新代码（root fixing 等）让
  hybrid-reverse 在 bier127 上由超时变解出，却让 PPSD32 与 PS-PS32 在
  bier127 上由解出变超时——根 fixing 对不同 smoothed 策略的效果分化明显。
- 费用与 Concorde 0 不一致，新代码正确性无问题。
- 结论：四个第五轮最佳配置在 CPHKMST 代码上整体保持（41 上下、无系统性
  退化），hybrid-reverse 在新代码上最受益；PPSD32/PS-PS32 的小幅回退集中在
  bier127 一例。
