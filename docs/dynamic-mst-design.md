# BP 搜索中的动态 MST 设计

## 结论

这里采用的是面向 BP 深度优先搜索的 **decremental dynamic MST**，不是通用的 fully dynamic MST：

- 单条搜索路径上，候选边只会被删除；
- force 的边来自当前 BP prefix tree，已经在当前 1-tree 中；
- 兄弟节点通过 `OneTree` 值复制隔离，回溯时丢弃子副本；
- 顶点 0 的两条根边独立维护，不属于内部 MST。

因此不传递一个无法处理 cut 的最终 DSU，也不复用单向 Kruskal cursor。`OneTree` 直接携带当前
内部 MST 的拓扑；删除树边时在 `T-e` 上求 fundamental cut，再选择当前 active 图中最轻的
跨割边完成 cut/link。

## 为什么不是“DSU 快照 + Kruskal 游标”

最终 MST 的 DSU 只有一个连通分量。删除树边后需要把它拆成两个分量，而普通或持久化 DSU
只支持合并/回滚既有 union，不能从最终状态直接执行任意 cut。

Kruskal 游标也不能只向后移动。某条边可能在第一次 Kruskal 时因为成环而被跳过，但在删除
树边后成为最轻 replacement：

```text
12=1, 23=2, 13=3, 34=4, 14=10, 24=11
```

初始内部 MST 是 `{12,23,34}`。`13` 在原 Kruskal 中被跳过；删除 `23` 后，正确 replacement
恰好是游标之前的 `13`。因此扫描起点只能是全局 0，或在“边权不变且状态只收紧”的前提下用
`lower_bound(weight(e))`，并且必须保留整个等权组。

## 状态布局

`OneTree` 随搜索节点传递以下状态：

```cpp
struct OneTree {
    bool feasible;
    double cost;
    std::vector<Edge> edges;
    std::vector<int> degree;
    std::vector<std::uint64_t> mst_adjacency_bits;
    std::size_t root_candidate_cursor;
};
```

`mst_adjacency_bits` 是按行展开的邻接位图，只表示顶点 `1..n-1` 上的内部 MST。行宽为
`ceil(n / 64)` 个 word；第 0 行和第 0 列始终为空。求 cut 时复用 solver 级 scratch：

- `mst_component_mark_`：epoch 标记一个分量；
- `mst_component_stack_`：无分配 DFS 栈；
- `mst_component_epoch_`：避免每次清空标记数组。

候选边 vector 始终保持全局权重顺序，`candidate_mask` 与 `candidate_bits` 表示当前 active 集。
这让 replacement 查询既能保持 Kruskal 顺序，又能按 64 条边一组跳过已删除候选。

## 更新算法

### 删除非树边

当前 MST 不变，不做任何拓扑更新。

### 删除内部树边 `e=(u,v)`

1. 若一次 force 导致多条候选失效，先把整批边全部写入 inactive mask；刚 force 的边标记为
   mandatory，不能从树中删除。
2. 在当前 MST 邻接位图上从 `u` 做 DFS，遍历时临时忽略 `e`，标记 `T-e` 的一侧。
3. 从全局候选序列的 `lower_bound(weight(e))` 开始扫描，跳过根边、forced、forbidden 和
   inactive 边。
4. 第一条两个端点分属 cut 两侧的边 `f`，就是最轻 replacement。
5. 找到 `f` 后原子执行 `cut(e)`、`link(f)`，并同步更新 `edges`、`degree` 和 `cost`。
6. 若不存在 `f`，内部图无法形成满足当前约束的生成树，该分支的 1-tree 不可行。

查询阶段不会先修改树。这样失败时不会留下“边表还是旧树、邻接已经 cut”的半更新状态。

### 删除根边

顶点 0 的两条边不进入内部 MST 位图。删除根边时只从 `root_candidates_sorted_` 的游标开始找
下一条合法边，失败再做前缀安全回扫；成功后只更新边表、度数、成本和根游标。

### BP prefix replay

`bpPartition()` 在一个 `work_tree` 上连续执行 forbid，并把每一步 replacement delta 存入
`BranchChoice`。枚举子节点时，`search()` 从父 `current_tree` 复制出 `prefix_tree`，按顺序用同一个
cut/link helper 重放这些 delta。因此后续 `B[i]` 的 force 子节点继承的是正确 prefix MST，且
邻接位图不会落后于边表。

## 正确性不变量

内部 replacement 使用 MST cut property：从 MST `T` 删除树边 `e` 后，取当前允许图中跨越
`T-e` 两个分量的最轻边 `f`，`T-e+f` 仍是当前约束下的最小生成树。

连续删除按上述交换逐步归纳。批量删除时先统一更新 mask 仍然安全：未轮到处理的待删树边仍在
当前树中，不可能跨越本轮 `T-e` 的 fundamental cut；待删非树边提前排除正是最终约束要求。

`lower_bound(weight(e))` 的前提是边权不变且 active 集只收紧。当前树在每一步都是收紧前图的
MST，所以不存在仍 active、跨本轮 cut 且严格轻于 `e` 的边。等权 replacement 仍必须从该权重组
的第一条开始扫描。

测试构建还会逐节点验证：

- 邻接位图与内部边一一对应、对称、无自环；
- 内部边恰有 `n-2` 条并连通、无环；
- 根边恰有两条且不污染内部邻接；
- 增量成本与完整 constrained Kruskal 一致；
- 父子 `OneTree` 的动态状态互不影响。

## 复制、回溯与复杂度

`OneTree` 的 vector 是值语义。进入子节点时复制父树，子节点原地 cut/link；返回时丢弃子副本，
无需共享可变状态或额外 undo log。

设内部顶点数为 `N=n-1`，候选边数为 `m`，一次 replacement 实际探测 `s` 条 active candidate：

- cut 分量标记：`O(N²/64)`（`N<=64` 时近似线性）；
- replacement scan：`O(s)`，最坏 `O(m)`；
- cut/link：`O(1)`；
- 每棵 `OneTree` 的附加空间：`O(N²/64)` 个 machine word。

它消除了每次 replacement 重建 DSU 的工作，但没有消除候选扫描。当前 `n=16..17` 的基准中
新增位图复制与省掉的 DSU 工作大致抵消，实测性能基本持平；价值首先是把可继续演进的动态拓扑
状态真正传给了子节点。

## 适用边界与后续演进

以下变化不能直接使用本方案：

- force 一条不在当前 prefix tree 中的边：需要在形成的 cycle 上删除最重可选边；
- 插入候选边；
- Held-Karp 顶点势或其他机制改变边权；
- 并行共享同一个 solver scratch。

这些情况应完整重算，或升级为支持 cycle exchange/rollback 的通用动态树。Link-cut tree 能提供
动态森林的 link/cut 与路径聚合，但不会单独解决“最轻跨割非树边”的维护；通用 fully dynamic
MST（例如 Holm–de Lichtenberg–Thorup 的分层结构）实现和常数都明显更重，适合在 profiling
证明候选扫描成为大规模实例主瓶颈后再评估：

- [Sleator–Tarjan, A Data Structure for Dynamic Trees](https://www.cs.cmu.edu/~sleator/papers/dynamic-trees.pdf)
- [Holm–de Lichtenberg–Thorup, Poly-Logarithmic Deterministic Fully-Dynamic Algorithms](https://www.cs.princeton.edu/courses/archive/fall07/cos521/handouts/poly.pdf)
