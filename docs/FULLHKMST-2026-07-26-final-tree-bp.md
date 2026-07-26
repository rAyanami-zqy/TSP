# FULLHKMST：复用节点势最后一轮 1-tree 的 BP 实验

## 实验目标

`FULLHKMST` 从当前 `HKMST` 工作树创建。与 HKMST “节点势只用于强化下界、
BP 继续使用固定根势树”不同，FULLHKMST 在每个可扩展搜索节点执行：

1. 从根 Held–Karp 势开始，在当前 forced / forbidden / active 约束下更新势；
2. 势迭代阶段继续用收缩 forced forest 的稠密 Prim 计算安全对偶下界；
3. 迭代结束后只构造一次最后一轮势对应的局部候选排序和受约束 1-tree；
4. 直接复用该 1-tree 查询每条候选树边的 best replacement；
5. 按 replacement delta 从大到小取得最短 cover，并用真实 forbid prefix
   重放验证后生成 B 集；
6. 子节点不跨势复用父节点动态 MST，而是完整构造自己的局部势树。

这样 BP 的违规顶点、选边权重、最佳替换边和 B 集全部来自节点的新势，而不是
固定根势。

## 正确性边界

- 全局 `candidates_sorted_`、active bitset 与撤销栈仍保持固定根势排序；
- 节点新势使用独立局部排序，边是否 active 仍通过稳定的 edge id 查询；
- 不同节点的势不同，因此动态 MST 只在同一节点的 BP forbid prefix 内复用；
- 节点剪枝可使用势迭代过程中遇到的最佳安全下界；
- BP 划分使用最后一轮树自身的安全下界与 replacement delta；
- 若尚无有限 incumbent，无法定义次梯度步长，节点先使用已构造的根势局部树；
- 数值失败回退根势局部树，不能被解释成节点不可行。

新增统计 `branch_potential_tree_reused` 记录最终 1-tree 实际交给 BP 的次数。

## 停止条件扫描

`gr48` 单次运行：

| 最大更新轮数 | 收缩阈值 | 时间 | created | expanded |
|---:|---:|---:|---:|---:|
| 1 | 0 | 21.49 s | 313,181 | 235,364 |
| 4 | 0 | 9.66 s | 136,727 | 51,916 |
| 8 | 0 | 4.32 s | 58,165 | 16,071 |
| 16 | 0 | 2.55 s | 32,297 | 7,356 |
| 32 | 0 | **2.32 s** | **23,394** | **4,891** |
| 64 | 0 | 3.23 s | 24,752 | 5,077 |

`64` 轮、窗口 `8`、最小收缩 `0.1%` 的自适应配置运行 156.61 秒仍未完成
首个 `gr48`，已中止。阈值停止不只改变计算量，也改变最后一轮树和后续 BP
顺序；较早停止可能形成明显更差的搜索树。因此 FULLHKMST 默认采用固定
`32` 轮、阈值 `0`，同时保留命令行阈值供后续实验。

## 五实例实际运行时间

两组都使用同一 Release 可执行文件：

- HKMST 对照：`--fixed-root-bp`
- FULLHKMST：`--branch-potential-updates 32 --branch-potential-min-shrink-percent 0`

| 实例 | HKMST 时间 | FULLHKMST 时间 | 时间倍数 | HKMST expanded | FULLHKMST expanded | expanded 变化 |
|---|---:|---:|---:|---:|---:|---:|
| gr48 | 0.09 s | 2.21 s | 24.56× | 90,045 | 4,891 | -94.57% |
| st70 | 0.25 s | 36.20 s | 144.80× | 121,714 | 35,253 | -71.04% |
| eil76 | 0.25 s | 1.52 s | 6.08× | 121,189 | 1,266 | -98.96% |
| rat99 | 0.12 s | 1.56 s | 13.00× | 37,385 | 868 | -97.68% |
| eil101 | 2.97 s | 96.95 s | 32.64× | 1,630,895 | 41,719 | -97.44% |

所有实例均返回已知最优成本：`5046 / 675 / 538 / 1211 / 629`。

## 结论

- 使用新势的最终 1-tree 进行 BP 能把 expanded 缩小 71%–99%；
- 最终树、最佳替换边和最大收益 B 集已经真正复用，不再回到固定势树；
- 当前实现每个节点需要势迭代、局部边排序和完整子节点重建，wall time 仍比
  HKMST 慢 6.7×–163×；
- FULLHKMST 适合作为后续“选择性触发、继承父势、减少局部排序成本”的实验
  分支，当前结果不应替代 HKMST 的默认性能版本。

## 验证

```text
cmake --build build-hkmst-branch-pot -j 8
ctest --test-dir build-hkmst-branch-pot --output-on-failure
```

普通 Release、启用 `TSP_VERIFY_INCREMENTAL_STATE` 的测试以及 AddressSanitizer
测试均通过。
