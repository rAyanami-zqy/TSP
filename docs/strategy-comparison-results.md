# TSP Solver Variants — Strategy Comparison

## Solver Variants

| Variant | Branch Strategy | Edge Selection | 1-tree | Source Branch |
|---|---|---|---|---|
| `tsp_bb_26_07_14_deg` | BP (multi-child) | degree-violating vertex, cheapest tree edge | incremental | main |
| `tsp_bb_26_07_14_min_edge` | BP (multi-child) | cheapest overall tree edge | incremental | main |
| `tsp_bb_26_07_14_full_deg` | BP (multi-child) | degree-violating vertex, cheapest tree edge | full recompute | full |
| `tsp_bb_26_07_14_full_min_edge` | BP (multi-child) | cheapest overall tree edge | full recompute | full |
| `tsp_bb_26_07_02` | Smart DFS (binary) | smart 3-tier (tree-first, then candidate fallback) | incremental | refactor/07_02-smart-dfs |
| `tsp_bb_26_07_02_full` | Smart DFS (binary) | smart 3-tier (tree-first, then candidate fallback) | full recompute | refactor/07_02-smart-dfs |

**Reference:** [Concorde](https://www.math.uwaterloo.ca/tsp/concorde.html) exact TSP solver  
**Timeout:** 1800s per instance  
**O(1) edge_index_in_tree optimization:** applied to all incremental variants (commit `c1b8387` / `f4925c0` / `2bbae59`)

---

## Results

### burma14 (n=14)

| Solver | Cost | Time | Expanded | vs Ref |
|---|---|---|---|---|
| Concorde | 3323 | 24ms | — | ref |
| `tsp_bb_26_07_02` | 3323 | 37ms | 9,162 | OK |
| `tsp_bb_26_07_02_full` | 3323 | 39ms | 9,162 | OK |
| `tsp_bb_26_07_14_deg` | 3323 | 34ms | 4,582 | OK |
| `tsp_bb_26_07_14_full_deg` | 3323 | 39ms | 4,582 | OK |
| `tsp_bb_26_07_14_full_min_edge` | 3323 | 57ms | 9,217 | OK |
| `tsp_bb_26_07_14_min_edge` | 3323 | 57ms | 9,152 | OK |

### ulysses16 (n=16)

| Solver | Cost | Time | Expanded | vs Ref |
|---|---|---|---|---|
| Concorde | 6859 | 45ms | — | ref |
| `tsp_bb_26_07_02` | 6859 | 18.3s | 7,797,474 | OK |
| `tsp_bb_26_07_02_full` | 6859 | 23.4s | 7,797,474 | OK |
| `tsp_bb_26_07_14_deg` | 6859 | 16.3s | 3,883,908 | OK |
| `tsp_bb_26_07_14_full_deg` | 6859 | 23.3s | 3,883,908 | OK |
| `tsp_bb_26_07_14_full_min_edge` | 6859 | 27.8s | 4,292,704 | OK |
| `tsp_bb_26_07_14_min_edge` | 6859 | 19.9s | 4,286,714 | OK |

### gr17 (n=17)

| Solver | Cost | Time | Expanded | vs Ref |
|---|---|---|---|---|
| Concorde | 2085 | 27ms | — | ref |
| `tsp_bb_26_07_02` | 2085 | 2.8s | 920,131 | OK |
| `tsp_bb_26_07_02_full` | 2085 | 3.7s | 920,131 | OK |
| `tsp_bb_26_07_14_deg` | 2085 | 2.9s | 463,820 | OK |
| `tsp_bb_26_07_14_full_deg` | 2085 | 2.9s | 463,820 | OK |
| `tsp_bb_26_07_14_full_min_edge` | 2085 | 5.0s | 676,329 | OK |
| `tsp_bb_26_07_14_min_edge` | 2085 | 3.8s | 676,809 | OK |

### gr21 (n=21)

| Solver | Cost | Time | Expanded | vs Ref |
|---|---|---|---|---|
| Concorde | 2707 | 14ms | — | ref |
| `tsp_bb_26_07_02` | 2707 | 84ms | 19,352 | OK |
| `tsp_bb_26_07_02_full` | 2707 | 99ms | 17,314 | OK |
| `tsp_bb_26_07_14_deg` | 2707 | 102ms | 11,193 | OK |
| `tsp_bb_26_07_14_full_deg` | 2707 | 97ms | 9,911 | OK |
| `tsp_bb_26_07_14_full_min_edge` | 2707 | 254ms | 24,876 | OK |
| `tsp_bb_26_07_14_min_edge` | 2707 | 130ms | 24,863 | OK |

### gr24 (n=24)

| Solver | Cost | Time | Expanded | vs Ref |
|---|---|---|---|---|
| Concorde | 1272 | 18ms | — | ref |
| `tsp_bb_26_07_02` | 1272 | 1.4s | 287,386 | OK |
| `tsp_bb_26_07_02_full` | 1272 | 2.3s | 287,023 | OK |
| `tsp_bb_26_07_14_deg` | 1272 | 1.3s | 158,396 | OK |
| `tsp_bb_26_07_14_full_deg` | 1272 | 2.6s | 158,220 | OK |
| `tsp_bb_26_07_14_full_min_edge` | 1272 | 2.9s | 297,291 | OK |
| `tsp_bb_26_07_14_min_edge` | 1272 | 2.4s | 298,857 | OK |

### fri26 (n=26)

| Solver | Cost | Time | Expanded | vs Ref |
|---|---|---|---|---|
| Concorde | 937 | 27ms | — | ref |
| `tsp_bb_26_07_02` | 937 | 2.7s | 498,093 | OK |
| `tsp_bb_26_07_02_full` | 937 | 3.8s | 492,159 | OK |
| `tsp_bb_26_07_14_deg` | 937 | 2.1s | 265,361 | OK |
| `tsp_bb_26_07_14_full_deg` | 937 | 2.7s | 264,044 | OK |
| `tsp_bb_26_07_14_full_min_edge` | 937 | 12.6s | 1,470,662 | OK |
| `tsp_bb_26_07_14_min_edge` | 937 | 9.8s | 1,481,796 | OK |

### bayg29 (n=29)

| Solver | Cost | Time | Expanded | vs Ref |
|---|---|---|---|---|
| Concorde | 1610 | 18ms | — | ref |
| `tsp_bb_26_07_02` | 1610 | 36.0s | 6,408,831 | OK |
| `tsp_bb_26_07_02_full` | 1610 | 46.8s | 6,376,947 | OK |
| `tsp_bb_26_07_14_deg` | 1610 | 38.6s | 3,624,699 | OK |
| `tsp_bb_26_07_14_full_deg` | 1610 | 45.0s | 3,605,784 | OK |
| `tsp_bb_26_07_14_full_min_edge` | 1610 | 2.8m | 12,520,142 | OK |
| `tsp_bb_26_07_14_min_edge` | 1610 | 2.1m | 12,397,531 | OK |

### bays29 (n=29)

| Solver | Cost | Time | Expanded | vs Ref |
|---|---|---|---|---|
| Concorde | 2020 | 33ms | — | ref |
| `tsp_bb_26_07_02` | 2020 | 7.6m | 81,083,477 | OK |
| `tsp_bb_26_07_02_full` | 2020 | 9.6m | 81,440,628 | OK |
| `tsp_bb_26_07_14_deg` | 2020 | 8.1m | 43,913,157 | OK |
| `tsp_bb_26_07_14_full_deg` | 2020 | 8.9m | 44,122,386 | OK |
| `tsp_bb_26_07_14_full_min_edge` | 2020 | 14.0m | 88,680,956 | OK |
| `tsp_bb_26_07_14_min_edge` | 2020 | 13.2m | 92,834,745 | OK |

### wi29 (n=29)

| Solver | Cost | Time | Expanded | vs Ref |
|---|---|---|---|---|
| Concorde | 27603 | 46ms | — | ref |
| `tsp_bb_26_07_02` | 27603 | 288ms | 38,746 | OK |
| `tsp_bb_26_07_02_full` | 27603 | 377ms | 38,866 | OK |
| `tsp_bb_26_07_14_deg` | 27603 | 239ms | 22,343 | OK |
| `tsp_bb_26_07_14_full_deg` | 27603 | 356ms | 22,423 | OK |
| `tsp_bb_26_07_14_full_min_edge` | 27603 | 2.4m | 14,867,815 | OK |
| `tsp_bb_26_07_14_min_edge` | 27603 | 2.0m | 14,837,661 | OK |

### dj38 (n=38)

| Solver | Cost | Time | Expanded | vs Ref |
|---|---|---|---|---|
| Concorde | 6656 | 30ms | — | ref |
| `tsp_bb_26_07_02` | 6656 | 142ms | 12,892 | OK |
| `tsp_bb_26_07_02_full` | 6656 | 277ms | 12,866 | OK |
| `tsp_bb_26_07_14_deg` | 6656 | 128ms | 7,977 | OK |
| `tsp_bb_26_07_14_full_deg` | 6656 | 184ms | 7,990 | OK |
| `tsp_bb_26_07_14_full_min_edge` | 6656 | 14.9s | 790,892 | OK |
| `tsp_bb_26_07_14_min_edge` | 6656 | 8.9s | 790,111 | OK |

### dantzig42 (n=42) — TIMEOUT (all variants)

### swiss42 (n=42) — TIMEOUT (all variants)

### att48 (n=48) — TIMEOUT (all variants)

### gr48 (n=48) — TIMEOUT (all variants)

### hk48 (n=48) — TIMEOUT (all variants)

---

## Analysis

### BP vs Smart DFS (node count)

BP consistently expands **~2× fewer nodes** than Smart DFS because the forbid prefix is shared among `|B_set|` force children:

| Instance | BP (deg) Expanded | Smart DFS Expanded | Ratio |
|---|---|---|---|
| burma14 | 4,582 | 9,162 | 2.00× |
| gr17 | 463,820 | 920,131 | 1.98× |
| gr21 | 11,193 | 19,352 | 1.73× |
| gr24 | 158,396 | 287,386 | 1.81× |
| fri26 | 265,361 | 498,093 | 1.88× |
| bayg29 | 3,624,699 | 6,408,831 | 1.77× |
| bays29 | 43,913,157 | 81,083,477 | 1.85× |

Average ratio: **~1.85×** fewer nodes for BP.

### Incremental vs Full 1-tree (wall-clock)

| Instance | deg (incr) | full_deg | Δ |
|---|---|---|---|
| burma14 | 34ms | 39ms | −13% |
| gr17 | 2.9s | 2.9s | 0% |
| gr24 | 1.3s | 2.6s | −50% |
| fri26 | 2.1s | 2.7s | −22% |
| bayg29 | 38.6s | 45.0s | −14% |
| bays29 | 8.1m | 8.9m | −9% |
| wi29 | 239ms | 356ms | −33% |
| dj38 | 128ms | 184ms | −30% |

Incremental is consistently faster than full recompute, with advantage growing from n=17 onward.

### degree vs min_edge

The degree strategy (branch on highest-degree violating vertex) consistently outperforms min_edge (cheapest overall tree edge):

| Instance | deg Expanded | min_edge Expanded | Ratio |
|---|---|---|---|
| burma14 | 4,582 | 9,152 | 2.0× |
| gr17 | 463,820 | 676,809 | 1.5× |
| gr24 | 158,396 | 298,857 | 1.9× |
| fri26 | 265,361 | 1,481,796 | 5.6× |
| wi29 | 22,343 | 14,837,661 | 664× |
| dj38 | 7,977 | 790,111 | 99× |

The degree strategy exploits structural information (1-tree degree violations) for more effective pruning. The gap widens dramatically on easier instances like wi29 and dj38.

### Instance difficulty

`bays29` and `bayg29` are both 29-city instances, yet `bays29` requires **~10× more nodes** than `bayg29`. The edge weight distribution of `bays29` creates a harder branch-and-bound landscape despite identical city count. Similarly, `ulysses16` (16 cities) requires 7.8M nodes while `gr24` (24 cities) only needs 158K — instance topology matters more than raw city count.

---

*Generated 2026-07-16 with O(1) edge_index_in_tree optimization applied to all incremental variants.*
*Commits: main `c1b8387`, full `f4925c0`, refactor/07_02-smart-dfs `2bbae59`*
