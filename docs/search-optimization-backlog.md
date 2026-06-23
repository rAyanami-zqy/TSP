# TSP Search Optimization Backlog

所有已识别的优化项，按优先级排列。✅ = 已实现。

## ✅ P0-1: Forbid 分支复用父 1-tree

当 forbid 边不在父 1-tree 中时，父 1-tree 对子节点仍然合法，直接复用，省去完整 Kruskal。

实现：`search()` 新增 `pre_tree` 参数，`explore_child` 中 forbid 分支检查并传入。

## ✅ P0-2: 根节点预排序候选集

`branch_candidates` 在根节点排序一次，`buildBranchCandidates` 的 compaction 过滤保持有序，
`computeOneTree` 不再排序。用 `original_candidates` 完整保存/恢复取代之前的复杂 swap 模式。

## ✅ P1: `candidate_mask` 缓存在 `PartialSol` 中

`chooseBranchEdge()` 不再每节点分配 n² 的掩码。掩码在 `solve()` 初始化，`explore_child` 中
通过 `original_mask` 保存/恢复，子节点通过 `rebuild_mask()` 从过滤后候选集重建。

**关键细节**: `computeOneTree` 的 root edges 选择必须检查 `candidate_mask`，
因为预排序的 `root_candidates_sorted_` 包含所有根边，但某些可能已被候选集过滤。

## ✅ P1: 顶点 0 incident edges 预排序

`solve()` 中预计算 `root_candidates_sorted_`（所有与顶点 0 相连的有限边，按权重排序）。
`computeOneTree` 中直接扫描此列表取前 2 条合法边，配合 `candidate_mask` 过滤。

## ✅ P2: Forbid 分支 fast-path

forbid 分支只移除一条 forbidden 边，不改变 forced 度数或 DSU。在 `explore_child` 中直接
`std::find_if` + `erase` 移除目标边，然后做轻量 degree 可行性检查，跳过完整的 `buildBranchCandidates` 扫描。

## ✅ P2: DSU find 缓存

`buildBranchCandidates` 开始时 O(n) 预计算所有顶点的 DSU 根到 `dsu_root` 数组，
边遍历中 O(1) 查根，省去每条边 O(tree_height) 的 while 循环。

---

## Benchmark 结果 (新 vs 旧, HEAD~1)

| 实例 | 旧时间 | 新时间 | 加速比 | 旧节点数 | 新节点数 |
|------|--------|--------|--------|----------|----------|
| burma14 (n=14) | 0.29s | 0.04s | **7.68x** | 79,563 | 18,111 |
| ulysses16 (n=16) | 104.45s | 16.05s | **6.51x** | 27,456,187 | 15,447,419 |
| gr17 (n=17) | 9.54s | 2.21s | **4.32x** | 1,785,976 | 1,817,688 |
| gr21 (n=21) | 0.41s | 0.08s | **5.19x** | 44,660 | 34,213 |
