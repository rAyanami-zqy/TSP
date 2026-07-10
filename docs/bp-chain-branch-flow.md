# TSP Branch & Bound - BP B 集直接分支流程

## 1. 顶层入口 `search()`

```text
search(node, branch_candidates, depth):
  |
  +-- Step 1: computeOneTree(node, branch_candidates) -> current_tree
  |
  +-- Step 2: current_tree.cost >= best_cost_ -> bound 剪枝
  |
  +-- Step 3: current_tree 是 tour -> 更新 incumbent 后返回
  |
  +-- Step 4: bpPartition(node, branch_candidates, current_tree)
  |            -> B = {e1, e2, ..., ek}
  |
  `-- Step 5: 直接创建 k 个互斥的 force 子节点
               +-- force e1                              -> search()
               +-- forbid e1 + force e2                 -> search()
               +-- forbid e1,e2 + force e3              -> search()
               `-- forbid e1,...,e(k-1) + force ek      -> search()
```

分支阶段不再创建单独的 forbid 节点。完成 `force ei` 子节点后，代码回滚
force 状态，再把 `ei` 直接加入后续子节点的前缀禁止约束，然后处理 `e(i+1)`。

这些子节点两两互斥。`bpPartition()` 的终止测试已经证明“全部 B 边均禁止”的
剩余区域不可改进，因此不需要为该区域再创建节点。

## 2. BP 划分 `bpPartition()`

```text
bpPartition(node, branch_candidates, current_tree):
  work_tree       = current_tree
  work_candidates = branch_candidates
  B               = []

  while work_tree 中仍有未决边 e:
    1. 把 e 加入 B
    2. 在工作状态中累计 forbid e，并从 work_candidates 删除 e
    3. computeOneTree(node, work_candidates) -> next_tree
    4. 根据 next_tree 处理：
       - 不可行：终止
       - 是 tour：更新 incumbent，终止
       - bound 不优于 incumbent：终止
       - 否则：work_tree = next_tree，继续选择下一条边

  恢复临时 forbidden 状态
  return B
```

当前选边策略从滚动 1-tree 的所有未决边中选择权值最小的边。BP 内部只保存最新的
滚动 1-tree，不再向分支阶段返回 `forbid_trees`。

终止时，最后一个 `next_tree` 对应约束 `forbid B[0..k-1]`：

- 不可行时，该剩余区域为空。
- 为 tour 时，它同时给出该区域的最小 1-tree 下界和一个同成本可行解。
- bound 可剪时，该区域不可能改进当前 incumbent。

因此搜索只需覆盖 B 中第一条被选择的边：

```text
C0 = force B[0]
C1 = forbid B[0] + force B[1]
...
C(k-1) = forbid B[0..k-2] + force B[k-1]
```

## 3. 分支状态维护

每个 `Ci` 使用同一份 `PartialSol` 做可逆更新：

1. 前缀 `forbid B[0..i-1]` 在循环迭代之间持续生效。
2. `force B[i]` 更新 forced 标记、度数、并查集、forced 边列表和 MST 缓存。
3. `buildBranchCandidates()` 基于当前前缀约束构造该子节点候选集。
4. `search()` 返回后立即回滚 `force B[i]`。
5. 若还有下一条 B 边，只写入 `forbid B[i]` 并从前缀候选集和掩码中删除它。
6. 所有 force 子节点完成后，逆序恢复前缀 forbidden 标记和候选掩码。

force 并查集不做路径压缩，保证每次合并可按栈顺序恢复。前缀 forbid 不修改
并查集。

## 4. `computeOneTree()`

```text
computeOneTree(node, branch_candidates):
  1. 从 node.forced_parent 初始化 MST 并查集
  2. 直接加入顶点 1..n-1 之间的 forced 边
  3. 对其余候选边执行 Kruskal，构造顶点 1..n-1 的 MST
  4. 加入与顶点 0 相连的两条可用最小边（forced 边优先）
  5. 计算成本和各顶点度数，返回受约束 1-tree
```

## 5. 搜索统计

| 字段 | 含义 |
|---|---|
| `nodes_created` | 根节点加实际进入 `search()` 的 force 子节点数 |
| `nodes_expanded` | 通过当前节点 bound 检查并继续展开的 `search()` 调用数 |
| `nodes_pruned_by_bound` | `search()` 中的下界剪枝，以及 B 为空的终止计数 |
| `nodes_pruned_infeasible` | force 约束或子节点候选集不可行的次数 |

前缀 forbid 只是构造下一个 force 子节点的约束，不计为节点，也不单独增加剪枝统计。
