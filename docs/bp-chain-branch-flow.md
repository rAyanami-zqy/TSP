# TSP Branch & Bound — BP 链式分支 完整流程

## 1. 顶层入口 `search()`

```
search(node, branch_candidates, depth):
  │
  ├─ Step 1: computeOneTree(node, branch_candidates) → current_tree
  │          Kruskal MST(non-root) + 2 min root-edges → 1-tree
  │
  ├─ Step 2: if current_tree.cost > best_cost_ → 剪枝 return
  │
  ├─ Step 3: if isTour(current_tree) → 更新 best_cost_, best_tour_ → return
  │
  ├─ Step 4: bpPartition(node, branch_candidates, current_tree)
  │          → B_set = {e₁, e₂, ..., eₖ},  forbid_trees = {T₁, T₂, ..., Tₖ}
  │          若 B_set 为空 → 剪枝 return
  │
  └─ Step 5: chain_branch(0)
             ├─ Force  e₁ → search() ─── 递归 Step 1~5（新 bpPartition）
             └─ Forbid e₁ → chain_branch(1)
                            ├─ Force  e₂ → search() ─── 递归
                            └─ Forbid e₂ → chain_branch(2)
                                           ├─ Force  e₃ → search()
                                           └─ Forbid e₃ → ... → chain_branch(k) → 死胡同
```

**关键性质：**

- **Force 节点** = 调用 `search()` → 完整递归（Step 1~5），开销大
- **Forbid 节点** = 沿链走 `chain_branch(idx+1)`，**不调 search()，不调 bpPartition，不调 computeOneTree**，开销≈0
- **完备性**：BP 保证至少一条 B 集边 ∈ 最优解，forbid 链到底与保证矛盾 → 死胡同
- **expanded nodes** = search() 调用次数 = Force 节点数

---

## 2. BP 划分 `bpPartition()`

```
bpPartition(node, branch_candidates, current_tree):
  │
  │  work_tree = current_tree (每轮迭代更新)
  │  work_node = copy of node
  │  work_candidates = branch_candidates
  │  B_set = [],  forbid_trees = []
  │
  └─ while true:
       │
       ├─ 1. 选边（二级优先，每轮最多测 2 条）：
       │      deg_best    = max-degree 顶点关联的 min-weight 未决边
       │      global_best = 全局 min-weight 未决边（排除 deg_best）
       │      if 二者皆空 → break
       │
       ├─ 2. 测试 deg_best（若存在）：
       │      forbid → computeOneTree → 检查下界
       │      ├─ 关键 (cost ≤ best_cost) → 加入 B_set, forbid_trees
       │      │   更新 work_tree = 新树, work_candidates = 移除该边
       │      │   → continue（下一轮，新树）
       │      └─ 安全/不可行 → restore forbidden
       │
       ├─ 3. 测试 global_best（若存在）：
       │      forbid → computeOneTree → 检查下界
       │      ├─ 关键 → 加入 B_set, forbid_trees, 更新 work_tree
       │      │   → continue
       │      └─ 安全 → break（MST 单调性：全局最小安全 ⇒ 全部安全）
       │
       └─ 4. break（无边可加入 B）

     返回 B_set, forbid_trees
```

**选边策略图解：**

```
work_tree 的 degree 分布:

    deg(v₀)=4  ★ max-degree vertex
    deg(v₁)=2      v₀
    deg(v₂)=2     /|\ \
    deg(v₃)=2    / | \ \
    ...         v₁ v₂ v₃ v₄

    1) deg_best: 扫描 v₀ 关联边 {e(v₀,v₁), e(v₀,v₂), e(v₀,v₃), e(v₀,v₄)}
       取最小权重未决边（如 e(v₀,v₁), w=5）

    2) global_best: 扫描所有未决边，排除 deg_best
       取最小权重（如 e(v₂,v₃), w=3）

    测试顺序: deg_best → (若安全) → global_best → (若安全) → 停止
```

---

## 3. 链式分支 `chain_branch(idx)`

```
chain_branch(idx):
  │
  │  if idx >= B_set.size() → return  // 死胡同
  │
  │  branch_edge = B_set[idx]
  │
  ├─ Force 分支:
  │    apply_flag(branch_edge, force=true)
  │    buildBranchCandidates(node, branch_candidates)
  │    search(node, branch_candidates, depth+1)  ← 完整递归
  │    revert_flag()
  │
  └─ Forbid 分支:
       apply_flag(branch_edge, force=false)
       从 branch_candidates 中移除 branch_edge
       轻量可行性检查 (每顶点 degree ≥ 2)
       │
       ├─ 不可行 → revert, 统计 pruned_infeasible
       │
       └─ 可行:
            forbid_tree = forbid_trees[idx]  ← bpPartition 预计算
            if forbid_tree.cost > best_cost_ → prune (bound)
            chain_branch(idx + 1)  ← 沿链继续，不调 search()
            revert_flag()
```

**chain_branch 执行示意图（|B| = 3）：**

```
                    search() 展开节点
                    B = {e₁, e₂, e₃}
                    forbid_trees = {T₁, T₂, T₃}
                         │
              ┌──────────┴──────────┐
           Force e₁              Forbid e₁
           search() ─┐           chain_branch(1)
           (新 BP)   │           (T₁ 已预计算)
                     │                │
              ┌──────┴──────┐   ┌────┴────┐
           Force e₁'  Forbid e₁'  Force e₂  Forbid e₂
           search()   chain(1)   search()  chain(2)
                                 (新 BP)   (T₂ 已预计算)
                                                │
                                          ┌─────┴─────┐
                                       Force e₃   Forbid e₃
                                       search()   chain(3)
                                       (新 BP)    → idx≥3
                                                   dead end
```

---

## 4. computeOneTree()

```
computeOneTree(node, branch_candidates):
  │
  ├─ 1. 初始化 DSU，复用 node.forced_parent
  │     将顶点 0 的分量中的其他顶点脱离为独立分量
  │
  ├─ 2. 收集 forced 边 (u,v ≠ 0)，加入 edges，计入 cost
  │
  ├─ 3. Kruskal MST on vertices {1..n-1}:
  │     for edge in branch_candidates (已预排序):
  │       if edge.u==0 or edge.v==0 → skip（留给第二步）
  │       if unite(edge.u, edge.v) → 加入 edges, 累计 cost
  │     if MST 边数 ≠ n-2 → 不可行，返回
  │
  ├─ 4. 选择 2 条与顶点 0 相连的最小边:
  │     先从 forced 中收集（≤2 条）
  │     再从 root_candidates_sorted_ 补充（跳过 forbidden/不在候选集）
  │     if 不足 2 条 → 不可行，返回
  │     加入 edges, 累计 cost
  │
  └─ 5. 统计 degree, 返回 OneTree{feasible, cost, edges, degree}
```

---

## 5. 搜索树结构对比

| | 旧版 Smart 二分 | BP 链式分支 |
|---|---|---|
| 每展开节点 child 数 | 2 (force+forbid) | 2 (force + forbid→chain) |
| Force child | search() 递归 | search() 递归 (同) |
| Forbid child | search() 递归 | **chain_branch(idx+1)** — 免重算 |
| forbid 节点是否 bpPartition | 是 (search 内调用) | **否** — 用预计算 forbid_trees |
| 完备性 | 二叉树穷举 | BP 保证 + 链穷举 |
| 分支数 (expanded) | 全部 search() 调用 | **仅 Force 节点** |
| Per-expanded-node 开销 | 1 computeOneTree | 1 + |B|×computeOneTree (bpPartition) |

---

## 6. 状态统计

| 统计量 | 含义 |
|---|---|
| `nodes_expanded` | `search()` 被调用的次数 = Force 节点数 |
| `nodes_created` | expanded + forbid 链节点数 |
| `nodes_pruned_by_bound` | 下界剪枝次数 |
| `nodes_pruned_infeasible` | 不可行剪枝次数 |

**说明：** `nodes_created` > `nodes_expanded` 的差额来自 forbid 链节点。forbid 链节点仅标记禁止边 + 检查 forbid_trees 下界，开销远低于 search()。
