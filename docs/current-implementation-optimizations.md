# TSP 求解器 — 当前已有实现优化

基于对 `include/TspSolver.hpp` 和 `src/TspSolver.cpp` 的代码审查，按类别列出所有已落地的**实现层面优化**（非策略/算法选型，而是数据结构、内存布局、缓存、位运算等工程优化）。

---

## 一、数据结构与内存布局

### 1. `vector<unsigned char>` 存边状态

`TspSolver.hpp:113-128`

强制/禁止/候选掩码三个数组均用 `vector<unsigned char>`（1 字节/条目），而非 `bool` 或 `int`。O(1) 随机访问，无位运算开销，n=42 时每个数组 ~1.7KB。

### 2. `edgeId()` 线性化映射 → O(1) 查找

`TspSolver.cpp:756-763`

```cpp
std::size_t edgeId(int u, int v) const {
    if (u > v) std::swap(u, v);
    return static_cast<std::size_t>(u) * n + v;
}
```

无向边规范化为 u<v，映射到 `u*n+v` 的线性索引。三个边状态数组平铺为 1D 数组，比 2D 结构更 cache-friendly。

### 3. POD `struct Edge`（24 字节）

`TspSolver.hpp:91-95`

```cpp
struct Edge { int u, v; double w; };
```

平凡可复制类型，存于连续 `vector<Edge>` 中，无虚函数、无指针间接。

### 4. 可逆 DSU（无路径压缩）

`TspSolver.hpp:116-118`, `TspSolver.cpp:554-560`

专为分支回溯设计的并查集，只做按秩合并，**禁用路径压缩**以保证状态可逆：

```cpp
auto forced_find = [&](int x) -> int {
    while (node.forced_parent[x] != x) x = node.forced_parent[x];
    return x;
};
```

附带 `forced_comp_size` 追踪分量大小用于子回路检测。

---

## 二、预计算与缓存

### 5. 全局边预排序

`TspSolver.cpp:386-398, 406-413`

- `branch_candidates`：所有有限权边按权重升序排序，一次性完成
- `root_candidates_sorted_`：顶点 0 的关联边按权重排序

每次 `computeOneTree()` 只需线性扫描已排序列表，避免每节点 O(m log m) 排序。

### 6. 候选集原地 Compaction（读写指针）

`TspSolver.cpp:929-982`

```
write=0; for read in 0..size: if keep: candidates[write++]=candidates[read]; resize(write)
```

读写指针原地过滤，只在有空洞时才移动元素。保持排序序，后代节点直接复用。

### 7. DSU 根缓存 → O(1) 边可行性检测

`TspSolver.cpp:937-945`

compaction 前将所有顶点的 DSU 根预计算到局部数组 `dsu_root[n]`，循环内 O(m) 边检测从 O(tree_height) 降为 O(1)。

### 8. `candidate_mask` O(1) 边存在性查询

`TspSolver.cpp:400-404, 653-662`

`vector<unsigned char>` 大小 n² 的 bloom-filter 式掩码。`chooseBranchEdge()` 可 O(1) 检查某边是否还在候选集中，无需扫描全表。force 分支后从 compacted 结果重建，forbid 分支直接清除对应位。

### 9. 强制边 MST 成本增量维护

`TspSolver.cpp:610-615, 814-815`

添加 force 边时同步累加 `forced_mst_cost` 和 `forced_mst_count`，回溯时等量扣减。`computeOneTree()` 直接用缓存值而无需重扫强制边。

### 10. Kruskal 中直接复用强制边

`TspSolver.cpp:812-819`

强制边不经过 DSU 合并逻辑，直接遍历 `node.forced_edges`（O(#forced) ≤ O(n)）加入输出。

### 11. DSU 复制 + 顶点 0 分离

`TspSolver.cpp:788-805`

从节点的强制边 DSU 复制父子/秩数组初始化 Kruskal DSU，然后分离含顶点 0 的分量——释放该分量中非 0 顶点，避免 1-tree 构造时的连通性错误。

### 12. 根边选择：强制优先 → 预排扫描 → 掩码过滤

`TspSolver.cpp:844-865`

1-tree 的两条根边选择流程：
1. 先扫强制边列表（最多 2 条）
2. 再扫预排 `root_candidates_sorted_`，用 forced/forbidden/mask 三数组 O(1) 过滤

### 13. 惰性 LK 候选集 + 部分排序

`TspSolver.cpp:1202-1229`

每个顶点的 K=8 近邻用 `std::nth_element`（O(n) 部分排序）选 K 小，然后排序 K 个结果（O(K log K)）。只需构建一次，跨多次 LK kick 复用。

### 14. Forbid 分支的 1-tree 复用

`TspSolver.cpp:727-740, 466-468`

如果 forbid 边不在父节点 1-tree 中，父节点的 1-tree 仍是子节点的合法下界（虽然非最优），直接传递为 `pre_tree` 跳过 `computeOneTree()`。P0 级优化。

---

## 三、增量/可逆状态（避免拷贝）

### 15. `LevelChanges` 结构体回滚（仅 13 个 int）

`TspSolver.cpp:544-552`

分支时只保存 delta（被修改的边 ID、两个 DSU 根、旧 rank/comp_size、顶点度数），13 个整数。回滚时按字段逐一恢复：

```cpp
struct LevelChanges {
    size_t forced_id, forbidden_id;
    int fu, fv, root_u, root_v;
    int old_rank_u, old_rank_v, old_size_u, old_size_v;
};
```

避免整个 `PartialSol`（含 ~n² 大小的 vector）拷贝。**整个递归搜索树不分配堆内存。**

### 16. 候选集 Copy + Move-Restore

`TspSolver.cpp:671-672, 745-746`

兄弟分支间通过 `std::move` 保存/恢复 `branch_candidates` 和 `candidate_mask`：

```cpp
branch_candidates = std::move(original_candidates);
node.candidate_mask = std::move(original_mask);
```

### 17. Forbid Fast Path

`TspSolver.cpp:675-707`

禁止边分支只做单条边 `std::find_if` + `erase` + mask 清零，不做完整的 `buildBranchCandidates`（跳过 DSU 根缓存和子回路检测）。

---

## 四、循环与提前退出

### 18. Kruskal 提前退出

`TspSolver.cpp:825-826`

```cpp
for (const Edge& edge : branch_candidates) {
    if (mst_edges == n_ - 2) break;
```

MST 边数达到 n-2 立即终止扫描，利用候选集已排序保证最优性。

### 19. 度数可行性预检

`TspSolver.cpp:985-994`

compaction 后 O(m) 扫描：若任一顶点剩余可用度数不足 2，立即返回不可行，避免后续 1-tree 计算。

### 20. 2-opt First-Improvement 提前退出

`TspSolver.cpp:1165-1166`

双层循环发现首个改进即 `break`，trade per-pass quality for faster convergence：

```cpp
for (int i = 1; i < n_ - 1 && !improved; ++i) {
    for (int k = i + 1; k < n_ && !improved; ++k) {
```

### 21. LK Don't-Look Bits

`TspSolver.cpp:1412-1456`

`vector<bool> active` 数组：尝试过且未找到改进的顶点标记为 inactive，后续轮次跳过。有改进时仅激活受影响顶点。

---

## 五、位运算与数值技巧

### 22. Morton Z-order 曲线（位交错）

`TspSolver.cpp:1890-1945`

对于坐标实例的启发式构造，通过位扩展（bit spreading）实现 Morton 编码：

```cpp
uint64_t partBy1(uint32_t value) {
    uint64_t result = value;
    result = (result | (result << 16U)) & 0x0000FFFF0000FFFFULL;
    result = (result | (result << 8U))  & 0x00FF00FF00FF00FFULL;
    result = (result | (result << 4U))  & 0x0F0F0F0F0F0F0F0FULL;
    result = (result | (result << 2U))  & 0x3333333333333333ULL;
    result = (result | (result << 1U))  & 0x5555555555555555ULL;
    return result;
}
```

key = `partBy1(x) | (partBy1(y) << 1)`，按 Z 序排列得到空间局部保持的启发式回路。

### 23. 浮点 epsilon 统一比较

`TspSolver.cpp:25`

```cpp
constexpr double kEps = 1e-9;
```

所有比较（剪枝、2-opt delta、LK gain 检查）统一使用，防止浮点波动导致死循环。

### 24. 确定性双桥 Kick

`TspSolver.cpp:1461-1494`

扰动偏移量基于实例规模确定计算（无随机数），保证跨运行可复现：

```cpp
const int a = 1 + (n_ / 7) % max(1, n_ / 4 - 1);
```

---

## 六、编译器/语言级

### 25. C++17，无虚函数，无 RTTI

`CMakeLists.txt:18`, `TspSolver.hpp:82-205`

`BranchBoundSolver` 类无虚方法，用 `if/else` 分派 `BranchStrategy` 枚举值。无 dynamic_cast、无异常。

### 26. 搜索递归全程零堆分配

`TspSolver.cpp:543-753`

`search()` 栈上用 `LevelChanges`、`Edge`，向量原地修改不分配。仅初始 `solve()` 和兄弟分支 `std::move` 还原时有分配。

---

## 汇总表

| # | 优化 | 类别 | 源码位置 |
|---|------|------|---------|
| 1 | `unsigned char` 边状态数组 | 数据结构 | `H:113-128` |
| 2 | `edgeId()` 线性映射 | 数据结构 | `C:756-763` |
| 3 | POD `Edge` struct | 数据结构 | `H:91-95` |
| 4 | 可逆 DSU（无路径压缩） | 数据结构 | `C:554-560` |
| 5 | 全局边预排序 | 预计算 | `C:386-398` |
| 6 | 读写指针原地 compaction | 内存 | `C:929-982` |
| 7 | DSU 根缓存 | 缓存 | `C:937-945` |
| 8 | `candidate_mask` O(1) 查询 | 缓存 | `C:400-404` |
| 9 | 强制边 MST 成本增量 | 增量更新 | `C:610-615` |
| 10 | Kruskal 复用强制边 | 预计算 | `C:812-819` |
| 11 | DSU 复制+顶点 0 分离 | 内存 | `C:788-805` |
| 12 | 根边选择优先级扫描 | 预计算 | `C:844-865` |
| 13 | 惰性 LK 候选集+nth_element | 预计算 | `C:1202-1229` |
| 14 | Forbid 分支 1-tree 复用 | 缓存 | `C:727-740` |
| 15 | `LevelChanges` 13-int 回滚 | 增量更新 | `C:544-552` |
| 16 | 候选集 copy+move-restore | 内存 | `C:671-672` |
| 17 | Forbid fast path | 提前退出 | `C:675-707` |
| 18 | Kruskal 提前退出 | 循环优化 | `C:825-826` |
| 19 | 度数可行性预检 | 提前退出 | `C:985-994` |
| 20 | 2-opt first-improvement | 循环优化 | `C:1165-1166` |
| 21 | LK don't-look bits | 循环优化 | `C:1412-1456` |
| 22 | Morton Z-order 位交错 | 位运算 | `C:1890-1945` |
| 23 | `kEps = 1e-9` 统一容差 | 数值技巧 | `C:25` |
| 24 | 确定性双桥 kick | 数值技巧 | `C:1461-1494` |
| 25 | C++17，无虚函数/RTTI | 语言级 | `H:82-205` |
| 26 | 搜索零堆分配 | 内存 | `C:543-753` |

> H = `include/TspSolver.hpp`, C = `src/TspSolver.cpp`

---

## 已知可优化但尚未实现的点

| # | 待优化 | 预期收益 |
|---|--------|---------|
| 1 | bitset 替代 `vector<unsigned char>` 存边状态 | 内存 1/8，位运算批量操作 |
| 2 | 距离矩阵平铺为 1D `vector<double>(n²)` | 减少指针间接，改善 cache |
| 3 | 2-opt/3-opt 移动评估增量计算 | 避免每次 O(n) 重算 |
| 4 | `__builtin_prefetch` 预取候选边 | 改善 Kruskal 扫描 cache miss |
