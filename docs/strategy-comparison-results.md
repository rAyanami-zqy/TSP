# TSP Algorithm Comparison

## Algorithms Compared

| Algorithm | Description |
|---|---|
| **Concorde** | State-of-the-art Concorde TSP solver (exact, with QSopt LP) — **reference** |
| **tsp_bb_26_07_02** | Branch & Bound solver — smart branching (legacy) |
| **tsp_bb_26_07_02_full** | Branch & Bound solver — smart branching (legacy) |
| **tsp_bb_26_07_14_deg** | Branch & Bound solver — BP (Branch Partitioning) |
| **tsp_bb_26_07_14_full_deg** | Branch & Bound solver — BP (Branch Partitioning) |
| **tsp_bb_26_07_14_full_min_edge** | Branch & Bound solver — BP (Branch Partitioning) |
| **tsp_bb_26_07_14_min_edge** | Branch & Bound solver — BP (Branch Partitioning) |

**Instances:** 38 from `examples` (n <= 49), `data/classic/tsplib` (n <= 59), `data/classic/national` (n <= 59)  
**Timeout:** 1800s (0h) per method per instance  
**Reference:** Concorde exact solver  
**Solver versions:** `tsp_bb_26_07_02`, `tsp_bb_26_07_02_full`, `tsp_bb_26_07_14_deg`, `tsp_bb_26_07_14_full_deg`, `tsp_bb_26_07_14_full_min_edge`, `tsp_bb_26_07_14_min_edge`  

---
