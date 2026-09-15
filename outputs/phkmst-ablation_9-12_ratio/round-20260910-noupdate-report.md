# PHKMST Ablation 小轮次报告：关闭节点势更新（P32-no-node-update）

完成时间：2026-09-10 约 06:00。配置 = P32 仅把 `--hk-potential-update subtree-adaptive` 改为 `none`，其余参数一致。参数：--timeout 1800 --workers 5 --exact-max-n 199，50 实例。

## 与 P32 对比

| 配置 | solved | timeout | error | mismatch(vs Concorde) | median_wall | median_branches |
|---|---|---|---|---|---|---|
| P32 | **40** | 10 | 0 | 0 | 0.322s | 480.5 |
| P32-no-node-update | **30** | 20 | 0 | 0 | 0.080s | 360.5 |

- 费用与 Concorde **0 不一致**
- 关闭节点势更新后 **10 个实例由解出变超时**，0 个反超：ch130、ch150、gr120、kroA150、kroB100、kroE100、pr144、pr76、rat195、u159
- 解出的实例上更快、分支更少（median 0.08s/360 vs 0.32s/480），说明节点势更新在难实例上是关键加速，简单实例上省掉反而更轻

## 结论

搜索节点势更新（subtree-adaptive）对 P32 是正收益：关掉后求解率 40/50 → 30/50，难实例（pr 系列、kroB/kroE 100、ch130 等）全部失守。
