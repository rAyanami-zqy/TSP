# TSP Branch-and-Bound 1-Tree 优化方向

## 1. `buildBranchCandidates` 增量化（收益最大，难度中等）

**现状**: 每个展开节点都做一次 `O(|candidates|)` 全量扫描 + 压实。初始 `|candidates| ≈ n²`，虽然后续缩小，但在搜索树上层（节点数多）时反复扫描，是最大单点瓶颈。

**优化方向**: force 边后知道哪些边会失效，不用全扫一遍才知道：
- degree-2 顶点 → 关联的 `O(n)` 条边直接移除
- 两个 forced 分量合并 → 跨分量的边变为子回路边，直接从候选集中删除

可以维护候选边的双向索引（顶点 → 候选边列表），删除时用懒标记 + 惰性清理，避免每次都做全量压实。但这增加了实现复杂度，且可能损失缓存友好性。

---

## 2. MST 替换边查询：避免每次重建 DSU

**现状**: `updateOneTreeAfterForbid` 和 `updateOneTreeAfterCandidateRemoval` 里，每次替换 MST 边都要：

```
重建 DSU: O(n)                // 遍历 n-3 条树边
扫描候选集: O(|candidates|)    // 找第一条跨割边
```

这两个操作在每次 forbid/force-child 时都会触发。

**优化方向 A — MST 敏感性预处理**: 对 MST 中每条边，预计算"如果这条边被删除，最优替代边是什么"。有经典算法可以在 `O(m α(n))` 时间内为所有 `n-3` 条 MST 边找到替代边（`m = n²` 对于完全图）。预处理完之后，所有替换查询都是 `O(1)`。

**优化方向 B — 持久化 DSU**: 把 `computeOneTree` 中 Kruskal 每一步的 DSU 快照存下来。替换某条 MST 边时，回退到该边加入前的 DSU 状态，直接定位割的两端。增量 DSU rollback 可以在 `O(α(n))` 内完成（类似 persistent array 或记录 change log）。

---

## 3. MST DSU 向子节点传递

**现状**: 我们现在传了 1-tree，但 `computeOneTree` 里的 `tree_components`（Kruskal 用的 DSU）每次都是从零建的。如果子节点需要完整重算（增量更新失败），Kruskal 会把所有 forced 边重新 unite 一遍，再扫描候选边。

**优化方向**: 把建好的 MST DSU 也随 1-tree 一起传给子节点。子节点做 Kruskal 时跳过已经在 DSU 中的 forced 边，直接从上次中断的位置继续扫描候选边——"warm start Kruskal"。

```cpp
struct OneTree {
    // ... existing fields ...
    DisjointSet mst_dsu;          // MST 的连通分量状态
    std::size_t kruskal_cursor;   // 上次 Kruskal 扫描到的候选边位置
};
```

---

## 4. 根边索引指针（简单，即刻可做）

**现状**: 每次替换根边，都从 `root_candidates_sorted_` 的第一条开始扫描，跳过 forbidden/forced/已在树中的边。

**优化方向**: 维护一个 `next_root_candidate_` 索引，指向下一次应该考虑的根边。当前根边被禁/被移出候选集时，指针后移即可。失败时 fallback 到完整扫描。

---

## 5. Prim 替代 Kruskal 做根节点 1-tree

**现状**: 根节点用 Kruskal 构造 MST。完全图上 `m = n(n-1)/2`，对预排序的候选边做 Kruskal 是 `O(n² α(n))`。排序在根节点前已做好，所以这一步不算差。

**优化方向**: 对稠密图，Prim 的 `O(n²)` 可以比 Kruskal 常数更小（无需并查集操作，只需维护 `nearest` 数组）。Concorde 用的就是 Prim。对于 `n ≤ 100` 左右的精确求解，差异不大；`n > 500` 时 Prim 可能明显更快。

---

## 6. `prefix_candidates.erase` 优化

**现状**（`search()` 中 B_set 前缀 forbid 循环）:

```cpp
auto it = std::find_if(prefix_candidates.begin(), prefix_candidates.end(), ...);
if (it != prefix_candidates.end()) {
    prefix_candidates.erase(it);  // O(|candidates|) 移动
}
```

每次 forbid 循环迭代都要从 vector 中部 erase 一个元素（B_set 的每个边），导致 `O(|B| × |candidates|)` 移动开销。

**优化**: 不实际删除，改用懒标记（`std::vector<bool> removed`），或把 B 集的边 swap 到末尾再 `pop_back`（已有 `find_if` 定位，swap + pop 是 O(1)）。

---

## 优先级排序

| 优化 | 收益 | 难度 | 说明 |
|---|---|---|---|
| 根边索引 | 低 | 低 | 几行代码，根边很少被替换 |
| `prefix_candidates` swap-pop | 中 | 低 | 简单，减少线性移动 |
| MST DSU 传递 + warm start | 中高 | 中 | 减少 fallback 重算开销 |
| `buildBranchCandidates` 增量化 | **高** | 高 | 最大瓶颈，但复杂 |
| MST 替换边预处理 | 中高 | 中 | 让 forbid 替换变 O(1) |
| Prim 替代 Kruskal（根节点） | 低中 | 低 | 常数级优化，n 大时有效 |
