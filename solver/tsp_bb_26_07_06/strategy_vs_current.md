# 07-06 策略与当前策略对比

对比对象：

- 07-06 版本：`solver/tsp_bb_26_07_06/tsp_bb`，对应提交 `0b2fae2` 的求解器代码。
- 当前策略：当前 `HEAD` 的 `src/TspSolver.cpp` / `include/TspSolver.hpp`，并已打包为
  `solver/tsp_bb_26_07_08/tsp_bb`。

结论：两者都不是标准二分分支；当前策略仍然是 BP-chain。当前策略主要修正了
BP 划分的 B 集构造语义、forbid 链终止处理和复用开销，而不是换成另一套搜索框架。

## 总览

| 维度 | 07-06 策略 | 当前策略 |
|---|---|---|
| 搜索框架 | BP-chain 分支定界 | BP-chain 分支定界 |
| 下界 | 受约束 1-tree | 受约束 1-tree |
| 初始上界 | 最近邻 + 2-opt + LK | 最近邻 + 2-opt + LK |
| B 集选边来源 | degree 违规边优先，失败后全局最小未决边 fallback | 只从 degree > 2 的违规顶点选最小未决边 |
| B 集加入条件 | forbid 后 1-tree 可行且未被 bound 剪枝才加入 B | 选中的边先加入 B，再根据 forbid 后结果决定是否继续划分 |
| forbid 后出现 tour | 07-06 的 forbid 链不直接更新 incumbent | 当前在 bpPartition 和 forbid 链中都会识别 tour 并更新 incumbent |
| forbid 后不可行 | 07-06 通常不把该边放入 B | 当前仍把该边放入 B，forbid 分支随后记为 infeasible，force 分支仍会被覆盖 |
| forbid 后被 bound 剪枝 | 07-06 通常不把该边放入 B | 当前仍把该边放入 B，forbid 分支随后按 bound 剪枝，force 分支仍会被覆盖 |
| `forbid_trees` | 存储被认定为关键边后的 forbid 1-tree | 存储每个进入 B 的边 forbid 后的 1-tree，包括终止态 |
| 候选集过滤接口 | `buildBranchCandidates(..., removed)`，`removed` 实际不参与后续策略 | `buildBranchCandidates(...)`，移除无用 `removed` 参数 |
| 实现开销 | `bpPartition` 中每次测试复制候选集和 1-tree | 原地更新工作候选集，`work_tree` 用指针复用，B/forbid tree 预留容量 |

## BP 划分差异

### 07-06

`bpPartition()` 每轮最多测试两类边：

1. 当前 1-tree 中 degree 最大且 degree > 2 的顶点，取其关联的最小权重未决边。
2. 如果第一类测试失败，再测试当前 1-tree 的全局最小权重未决边。

测试边 `e` 时，07-06 会临时 forbid `e` 并重算 1-tree：

- 新 1-tree 不可行：撤销 forbid，`e` 不加入 B。
- 新 1-tree cost 超过当前 best：撤销 forbid，`e` 不加入 B，视为安全或可剪枝。
- 新 1-tree 可行且 cost 不超过当前 best：`e` 加入 B，保留工作副本中的 forbid 状态，继续下一轮 BP 划分。

这意味着 07-06 的 B 集更像“forbid 后仍可能保留改进解”的关键边集合。

### 当前策略

当前 `bpPartition()` 只从 degree > 2 的违规顶点选边，不再使用全局最小未决边 fallback。

测试边 `e` 时，当前策略先把 `e` 加入 B，并保存 forbid `e` 后的 1-tree：

- forbid 后不可行：停止继续划分，但 `e` 已在 B 中。
- forbid 后得到 tour：更新 incumbent，停止继续划分，但 `e` 已在 B 中。
- forbid 后被 bound 剪枝：停止继续划分，但 `e` 已在 B 中。
- forbid 后仍可行且未剪枝：保留 forbid 状态，切换到该 1-tree 继续划分。

这个变化的核心是：当前策略不会因为 forbid 分支终止就丢掉这个分支边。它仍保留
Force `e` 的搜索入口，同时让 Forbid `e` 在链上由预计算的 `forbid_tree` 立即终止。

## 链式分支差异

两版都使用同一类链式分支结构：

- Force `B[i]`：强制选择该边，过滤候选集，递归进入完整 `search()`。
- Forbid `B[i]`：禁止该边，复用 `forbid_trees[i]`，不重新做 BP 划分，继续链到 `B[i+1]`。

差异在 forbid 分支的终止处理。

### 07-06

07-06 的 forbid 链只做：

- 轻量可行性检查。
- 用 `forbid_trees[i].cost` 做 bound 剪枝。
- 如果没有剪枝，则继续 `chain_branch(i + 1)`。

它没有在 forbid 链上检查 `forbid_trees[i]` 是否：

- 不可行。
- 已经是 Hamilton tour。

因为 07-06 的 B 集构造通常只保存可行且未剪枝的 forbid tree，所以不可行情况较少进入链；
但 tour 终止态没有在 forbid 链上直接更新 incumbent，这是后续修正点。

### 当前策略

当前 forbid 链增加了完整终止处理：

- `!forbid_tree.feasible`：计入 infeasible 剪枝并返回。
- `isTour(forbid_tree)`：构造 tour，更新 incumbent，然后返回。
- `shouldPrune(forbid_tree.cost, best_cost_)`：计入 bound 剪枝并返回。
- 只有以上都不触发，才继续 `chain_branch(i + 1)`。

这与当前 B 集“先加入 B，再保存 forbid tree”的语义匹配。

## 对完备性和搜索形态的影响

07-06 的风险在于：如果某条 degree 违规边 forbid 后已经不可行或可剪枝，它不会进入 B，
然后尝试 fallback 或停止划分。这样会让 B 集只包含“forbid 后仍活着”的边。

当前策略更直接地把选中的 degree 违规边作为分支边：

- Force 分支总会被覆盖。
- Forbid 分支如果不可行、成 tour 或被 bound 剪枝，会在链上立刻终止。
- B 集不再依赖“forbid 后仍可行且未剪枝”这个条件。

因此当前策略的分支语义更清楚：选到的违规边就是本轮分支对象；forbid 结果只是决定
该链是否继续，而不是决定这条边是否有资格进入 B。

## 性能实现差异

当前策略做了几处实现层面的优化：

- `B_set.reserve(current_tree.edges.size())`。
- `forbid_trees.reserve(current_tree.edges.size())`。
- `work_tree` 从完整对象拷贝改为 `const OneTree*` 指针引用。
- `work_candidates` 在 BP 工作副本内原地 erase，不再每次测试都复制一份 `wc`。
- `buildBranchCandidates()` 删除 `removed` 输出参数，减少无用数据搬运。

这些变化主要减少 `bpPartition()` 中的复制和分配。

## 未变化部分

以下策略骨架没有变：

- 输入仍会转成稠密对称距离矩阵。
- 求解器仍要求对称 TSP 和非负边权。
- 初始上界仍是最近邻 + 2-opt + Lin-Kernighan。
- 下界仍是以顶点 0 为根的受约束 1-tree。
- `search()` 仍采用 DFS 递归。
- Force 子节点仍重新进入完整 BP 搜索。
- Forbid 子节点仍沿链走，不重新调用 `search()`。
- CLI 仍没有 `--method` / `--branch-strategy`。

## 非策略差异

- 07-06 二进制是 Linux x86-64 ELF。
- 当前打包的 07-08 二进制是 macOS arm64 Mach-O。
- 当前策略对应的源代码还包含 heatmap 分析工具更新，但这不改变核心 TSP 搜索策略。

