# TSP Algorithm Comparison

## Algorithms Compared

| Algorithm | Description |
|---|---|
| **Concorde** | State-of-the-art Concorde TSP solver (exact, with QSopt LP) — **reference** |
| **tsp_bb_26_07_02** | Branch & Bound solver — smart branching (legacy) |
| **tsp_bb_26_07_08_02** | Branch & Bound solver — BP (Branch Partitioning) |
| **tsp_bb_26_07_10** | Branch & Bound solver — BP (Branch Partitioning) |
| **tsp_bb_26_07_13_01** | Branch & Bound solver — BP (Branch Partitioning) |
| **tsp_bb_26_07_13_02** | Branch & Bound solver — BP (Branch Partitioning) |

**Instances:** 38 from `examples` (n <= 49), `data/classic/tsplib` (n <= 59), `data/classic/national` (n <= 59)  
**Timeout:** 1800s (0h) per method per instance  
**Reference:** Concorde exact solver  
**Solver versions:** `tsp_bb_26_07_02`, `tsp_bb_26_07_08_02`, `tsp_bb_26_07_10`, `tsp_bb_26_07_13_01`, `tsp_bb_26_07_13_02`  

---

## 1. `examples/random/complete/rnd-01-complete-n4.txt` (n=4)

**Concorde optimal cost:** 45  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 3 -> 2 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 45 | 3ms | 5 | 2 | 3 | 0 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 45 | 3ms | 3 | 2 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_10 | 45 | 3ms | 3 | 2 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 45 | 3ms | 3 | 2 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 45 | 3ms | 3 | 2 | 1 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 3 -> 2 -> 1 -> 0` cost=45
- **tsp_bb_26_07_02:** `0 -> 3 -> 2 -> 1 -> 0` cost=45 (=ref, same tour)
- **tsp_bb_26_07_08_02:** `0 -> 3 -> 2 -> 1 -> 0` cost=45 (=ref, same tour)
- **tsp_bb_26_07_10:** `0 -> 3 -> 2 -> 1 -> 0` cost=45 (=ref, same tour)
- **tsp_bb_26_07_13_01:** `0 -> 3 -> 2 -> 1 -> 0` cost=45 (=ref, same tour)
- **tsp_bb_26_07_13_02:** `0 -> 3 -> 2 -> 1 -> 0` cost=45 (=ref, same tour)

---

## 2. `examples/random/complete/rnd-03-complete-n4.txt` (n=4)

**Concorde optimal cost:** 102  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 1 -> 3 -> 2 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 102 | 3ms | 1 | 0 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 102 | 3ms | 1 | 0 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_10 | 102 | 3ms | 1 | 0 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 102 | 3ms | 1 | 0 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 102 | 3ms | 1 | 0 | 1 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 1 -> 3 -> 2 -> 0` cost=102
- **tsp_bb_26_07_02:** `0 -> 1 -> 3 -> 2 -> 0` cost=102 (=ref, same tour)
- **tsp_bb_26_07_08_02:** `0 -> 1 -> 3 -> 2 -> 0` cost=102 (=ref, same tour)
- **tsp_bb_26_07_10:** `0 -> 1 -> 3 -> 2 -> 0` cost=102 (=ref, same tour)
- **tsp_bb_26_07_13_01:** `0 -> 1 -> 3 -> 2 -> 0` cost=102 (=ref, same tour)
- **tsp_bb_26_07_13_02:** `0 -> 1 -> 3 -> 2 -> 0` cost=102 (=ref, same tour)

---

## 3. `examples/five-city.tsp` (n=5)

**Concorde optimal cost:** 26  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 2 -> 3 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 26 | 3ms | 13 | 6 | 7 | 0 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 26 | 4ms | 7 | 5 | 2 | 0 | :white_check_mark: |
| tsp_bb_26_07_10 | 26 | 4ms | 7 | 5 | 2 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 26 | 3ms | 7 | 5 | 2 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 26 | 3ms | 7 | 5 | 2 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 4 -> 2 -> 3 -> 1 -> 0` cost=26
- **tsp_bb_26_07_02:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_08_02:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_10:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_13_01:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_13_02:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)

---

## 4. `examples/five-city.txt` (n=5)

**Concorde optimal cost:** 26  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 2 -> 3 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 26 | 2ms | 13 | 6 | 7 | 0 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 26 | 4ms | 7 | 5 | 2 | 0 | :white_check_mark: |
| tsp_bb_26_07_10 | 26 | 4ms | 7 | 5 | 2 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 26 | 4ms | 7 | 5 | 2 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 26 | 3ms | 7 | 5 | 2 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 4 -> 2 -> 3 -> 1 -> 0` cost=26
- **tsp_bb_26_07_02:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_08_02:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_10:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_13_01:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_13_02:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)

---

## 5. `examples/converted/five-node-euc.txt` (n=5)

**Concorde optimal cost:** 8  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 1 -> 2 -> 3 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 8 | 3ms | 15 | 7 | 8 | 0 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 8 | 2ms | 8 | 4 | 4 | 0 | :white_check_mark: |
| tsp_bb_26_07_10 | 8 | 4ms | 6 | 3 | 3 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 8 | 3ms | 6 | 3 | 3 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 8 | 3ms | 8 | 4 | 4 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=8
- **tsp_bb_26_07_02:** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=8 (=ref, same tour)
- **tsp_bb_26_07_08_02:** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=8 (=ref, same tour)
- **tsp_bb_26_07_10:** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=8 (=ref, same tour)
- **tsp_bb_26_07_13_01:** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=8 (=ref, same tour)
- **tsp_bb_26_07_13_02:** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=8 (=ref, same tour)

---

## 6. `examples/converted/five-node-explicit.txt` (n=5)

**Concorde optimal cost:** 26  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 2 -> 3 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 26 | 3ms | 13 | 6 | 7 | 0 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 26 | 4ms | 7 | 5 | 2 | 0 | :white_check_mark: |
| tsp_bb_26_07_10 | 26 | 4ms | 7 | 5 | 2 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 26 | 3ms | 7 | 5 | 2 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 26 | 3ms | 7 | 5 | 2 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 4 -> 2 -> 3 -> 1 -> 0` cost=26
- **tsp_bb_26_07_02:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_08_02:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_10:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_13_01:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_13_02:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)

---

## 7. `examples/random/sparse/rnd-01-sparse-n5.txt` (n=5)

**Concorde:** failed to find optimal solution  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 110 | 3ms | 5 | 2 | 3 | 0 | - |
| tsp_bb_26_07_08_02 | 110 | 4ms | 3 | 2 | 1 | 0 | - |
| tsp_bb_26_07_10 | 110 | 4ms | 3 | 2 | 1 | 0 | - |
| tsp_bb_26_07_13_01 | 110 | 3ms | 3 | 2 | 1 | 0 | - |
| tsp_bb_26_07_13_02 | 110 | 3ms | 3 | 2 | 1 | 0 | - |

**Tours found:**
- **tsp_bb_26_07_02:** `3 -> 4 -> 2 -> 1 -> 0 -> 3` cost=110
- **tsp_bb_26_07_08_02:** `3 -> 4 -> 2 -> 1 -> 0 -> 3` cost=110
- **tsp_bb_26_07_10:** `3 -> 4 -> 2 -> 1 -> 0 -> 3` cost=110
- **tsp_bb_26_07_13_01:** `3 -> 4 -> 2 -> 1 -> 0 -> 3` cost=110
- **tsp_bb_26_07_13_02:** `3 -> 4 -> 2 -> 1 -> 0 -> 3` cost=110

---

## 8. `examples/random/complete/rnd-02-complete-n5.txt` (n=5)

**Concorde optimal cost:** 136  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 2 -> 3 -> 4 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 136 | 3ms | 7 | 3 | 4 | 0 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 136 | 4ms | 4 | 3 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_10 | 136 | 4ms | 5 | 4 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 136 | 3ms | 5 | 4 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 136 | 3ms | 4 | 3 | 1 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 2 -> 3 -> 4 -> 1 -> 0` cost=136
- **tsp_bb_26_07_02:** `0 -> 2 -> 3 -> 4 -> 1 -> 0` cost=136 (=ref, same tour)
- **tsp_bb_26_07_08_02:** `0 -> 2 -> 3 -> 4 -> 1 -> 0` cost=136 (=ref, same tour)
- **tsp_bb_26_07_10:** `0 -> 2 -> 3 -> 4 -> 1 -> 0` cost=136 (=ref, same tour)
- **tsp_bb_26_07_13_01:** `0 -> 2 -> 3 -> 4 -> 1 -> 0` cost=136 (=ref, same tour)
- **tsp_bb_26_07_13_02:** `0 -> 2 -> 3 -> 4 -> 1 -> 0` cost=136 (=ref, same tour)

---

## 9. `examples/random/complete/rnd-04-complete-n5.txt` (n=5)

**Concorde optimal cost:** 88  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 3 -> 2 -> 4 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 88 | 3ms | 7 | 3 | 4 | 0 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 88 | 3ms | 4 | 3 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_10 | 88 | 4ms | 4 | 3 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 88 | 3ms | 4 | 3 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 88 | 3ms | 4 | 3 | 1 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 3 -> 2 -> 4 -> 1 -> 0` cost=88
- **tsp_bb_26_07_02:** `0 -> 3 -> 2 -> 4 -> 1 -> 0` cost=88 (=ref, same tour)
- **tsp_bb_26_07_08_02:** `0 -> 3 -> 2 -> 4 -> 1 -> 0` cost=88 (=ref, same tour)
- **tsp_bb_26_07_10:** `0 -> 3 -> 2 -> 4 -> 1 -> 0` cost=88 (=ref, same tour)
- **tsp_bb_26_07_13_01:** `0 -> 3 -> 2 -> 4 -> 1 -> 0` cost=88 (=ref, same tour)
- **tsp_bb_26_07_13_02:** `0 -> 3 -> 2 -> 4 -> 1 -> 0` cost=88 (=ref, same tour)

---

## 10. `examples/random/complete/rnd-06-complete-n5.txt` (n=5)

**Concorde optimal cost:** 51  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 1 -> 2 -> 3 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 51 | 3ms | 7 | 3 | 4 | 0 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 51 | 4ms | 4 | 2 | 2 | 0 | :white_check_mark: |
| tsp_bb_26_07_10 | 51 | 4ms | 5 | 4 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 51 | 3ms | 5 | 4 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 51 | 3ms | 4 | 2 | 2 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=51
- **tsp_bb_26_07_02:** `0 -> 3 -> 2 -> 1 -> 4 -> 0` cost=51 (=ref, different tour)
- **tsp_bb_26_07_08_02:** `0 -> 3 -> 2 -> 1 -> 4 -> 0` cost=51 (=ref, different tour)
- **tsp_bb_26_07_10:** `0 -> 3 -> 2 -> 1 -> 4 -> 0` cost=51 (=ref, different tour)
- **tsp_bb_26_07_13_01:** `0 -> 3 -> 2 -> 1 -> 4 -> 0` cost=51 (=ref, different tour)
- **tsp_bb_26_07_13_02:** `0 -> 3 -> 2 -> 1 -> 4 -> 0` cost=51 (=ref, different tour)

---

## 11. `examples/random/sparse/rnd-04-sparse-n6.txt` (n=6)

**Concorde:** failed to find optimal solution  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 115 | 3ms | 1 | 0 | 1 | 0 | - |
| tsp_bb_26_07_08_02 | 115 | 4ms | 1 | 0 | 1 | 0 | - |
| tsp_bb_26_07_10 | 115 | 4ms | 1 | 0 | 1 | 0 | - |
| tsp_bb_26_07_13_01 | 115 | 3ms | 1 | 0 | 1 | 0 | - |
| tsp_bb_26_07_13_02 | 115 | 3ms | 1 | 0 | 1 | 0 | - |

**Tours found:**
- **tsp_bb_26_07_02:** `0 -> 4 -> 3 -> 5 -> 1 -> 2 -> 0` cost=115
- **tsp_bb_26_07_08_02:** `0 -> 4 -> 3 -> 5 -> 1 -> 2 -> 0` cost=115
- **tsp_bb_26_07_10:** `0 -> 4 -> 3 -> 5 -> 1 -> 2 -> 0` cost=115
- **tsp_bb_26_07_13_01:** `0 -> 4 -> 3 -> 5 -> 1 -> 2 -> 0` cost=115
- **tsp_bb_26_07_13_02:** `0 -> 4 -> 3 -> 5 -> 1 -> 2 -> 0` cost=115

---

## 12. `examples/random/sparse/rnd-05-sparse-n6.txt` (n=6)

**Concorde:** failed to find optimal solution  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 100 | 3ms | 30 | 22 | 8 | 15 | - |
| tsp_bb_26_07_08_02 | 100 | 4ms | 19 | 14 | 6 | 3 | - |
| tsp_bb_26_07_10 | 100 | 4ms | 20 | 15 | 5 | 5 | - |
| tsp_bb_26_07_13_01 | 100 | 3ms | 20 | 15 | 5 | 5 | - |
| tsp_bb_26_07_13_02 | 100 | 4ms | 18 | 13 | 6 | 3 | - |

**Tours found:**
- **tsp_bb_26_07_02:** `0 -> 5 -> 2 -> 1 -> 4 -> 3 -> 0` cost=100
- **tsp_bb_26_07_08_02:** `0 -> 5 -> 2 -> 1 -> 4 -> 3 -> 0` cost=100
- **tsp_bb_26_07_10:** `0 -> 5 -> 2 -> 1 -> 4 -> 3 -> 0` cost=100
- **tsp_bb_26_07_13_01:** `0 -> 5 -> 2 -> 1 -> 4 -> 3 -> 0` cost=100
- **tsp_bb_26_07_13_02:** `0 -> 5 -> 2 -> 1 -> 4 -> 3 -> 0` cost=100

---

## 13. `examples/random/sparse/rnd-02-sparse-n7.txt` (n=7)

**Concorde:** failed to find optimal solution  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 196 | 4ms | 19 | 13 | 6 | 8 | - |
| tsp_bb_26_07_08_02 | 196 | 4ms | 7 | 6 | 1 | 7 | - |
| tsp_bb_26_07_10 | 196 | 4ms | 15 | 14 | 1 | 8 | - |
| tsp_bb_26_07_13_01 | 196 | 3ms | 15 | 13 | 2 | 6 | - |
| tsp_bb_26_07_13_02 | 196 | 3ms | 7 | 6 | 1 | 7 | - |

**Tours found:**
- **tsp_bb_26_07_02:** `1 -> 5 -> 3 -> 0 -> 4 -> 2 -> 6 -> 1` cost=196
- **tsp_bb_26_07_08_02:** `1 -> 5 -> 3 -> 0 -> 4 -> 2 -> 6 -> 1` cost=196
- **tsp_bb_26_07_10:** `1 -> 5 -> 3 -> 0 -> 4 -> 2 -> 6 -> 1` cost=196
- **tsp_bb_26_07_13_01:** `1 -> 5 -> 3 -> 0 -> 4 -> 2 -> 6 -> 1` cost=196
- **tsp_bb_26_07_13_02:** `1 -> 5 -> 3 -> 0 -> 4 -> 2 -> 6 -> 1` cost=196

---

## 14. `examples/random/sparse/rnd-03-sparse-n7.txt` (n=7)

**Concorde:** failed to find optimal solution  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 86 | 4ms | 7 | 4 | 3 | 2 | - |
| tsp_bb_26_07_08_02 | 86 | 4ms | 5 | 3 | 2 | 1 | - |
| tsp_bb_26_07_10 | 86 | 4ms | 9 | 8 | 1 | 2 | - |
| tsp_bb_26_07_13_01 | 86 | 3ms | 9 | 8 | 1 | 2 | - |
| tsp_bb_26_07_13_02 | 86 | 3ms | 5 | 3 | 2 | 1 | - |

**Tours found:**
- **tsp_bb_26_07_02:** `1 -> 6 -> 3 -> 4 -> 5 -> 2 -> 0 -> 1` cost=86
- **tsp_bb_26_07_08_02:** `1 -> 6 -> 3 -> 4 -> 5 -> 2 -> 0 -> 1` cost=86
- **tsp_bb_26_07_10:** `1 -> 6 -> 3 -> 4 -> 5 -> 2 -> 0 -> 1` cost=86
- **tsp_bb_26_07_13_01:** `1 -> 6 -> 3 -> 4 -> 5 -> 2 -> 0 -> 1` cost=86
- **tsp_bb_26_07_13_02:** `1 -> 6 -> 3 -> 4 -> 5 -> 2 -> 0 -> 1` cost=86

---

## 15. `examples/random/complete/rnd-05-complete-n7.txt` (n=7)

**Concorde optimal cost:** 74  
**Concorde time:** 3ms  

**Concorde (reference) tour:** `0 -> 3 -> 6 -> 5 -> 1 -> 4 -> 2 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 74 | 3ms | 23 | 11 | 12 | 0 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 74 | 4ms | 12 | 8 | 4 | 0 | :white_check_mark: |
| tsp_bb_26_07_10 | 74 | 4ms | 16 | 12 | 4 | 1 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 74 | 2ms | 16 | 12 | 4 | 1 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 74 | 3ms | 12 | 8 | 4 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 3 -> 6 -> 5 -> 1 -> 4 -> 2 -> 0` cost=74
- **tsp_bb_26_07_02:** `0 -> 2 -> 4 -> 1 -> 5 -> 6 -> 3 -> 0` cost=74 (=ref, different tour)
- **tsp_bb_26_07_08_02:** `0 -> 2 -> 4 -> 1 -> 5 -> 6 -> 3 -> 0` cost=74 (=ref, different tour)
- **tsp_bb_26_07_10:** `0 -> 2 -> 4 -> 1 -> 5 -> 6 -> 3 -> 0` cost=74 (=ref, different tour)
- **tsp_bb_26_07_13_01:** `0 -> 2 -> 4 -> 1 -> 5 -> 6 -> 3 -> 0` cost=74 (=ref, different tour)
- **tsp_bb_26_07_13_02:** `0 -> 2 -> 4 -> 1 -> 5 -> 6 -> 3 -> 0` cost=74 (=ref, different tour)

---

## 16. `examples/random/sparse/rnd-06-sparse-n7.txt` (n=7)

**Concorde:** failed to find optimal solution  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 199 | 4ms | 28 | 21 | 7 | 15 | - |
| tsp_bb_26_07_08_02 | 199 | 4ms | 15 | 14 | 1 | 8 | - |
| tsp_bb_26_07_10 | 199 | 4ms | 15 | 14 | 1 | 10 | - |
| tsp_bb_26_07_13_01 | 199 | 3ms | 15 | 14 | 1 | 10 | - |
| tsp_bb_26_07_13_02 | 199 | 3ms | 15 | 14 | 1 | 8 | - |

**Tours found:**
- **tsp_bb_26_07_02:** `4 -> 1 -> 5 -> 3 -> 2 -> 6 -> 0 -> 4` cost=199
- **tsp_bb_26_07_08_02:** `4 -> 1 -> 5 -> 3 -> 2 -> 6 -> 0 -> 4` cost=199
- **tsp_bb_26_07_10:** `4 -> 1 -> 5 -> 3 -> 2 -> 6 -> 0 -> 4` cost=199
- **tsp_bb_26_07_13_01:** `4 -> 1 -> 5 -> 3 -> 2 -> 6 -> 0 -> 4` cost=199
- **tsp_bb_26_07_13_02:** `4 -> 1 -> 5 -> 3 -> 2 -> 6 -> 0 -> 4` cost=199

---

## 17. `examples/random/complete/rnd-08-complete-n7.txt` (n=7)

**Concorde optimal cost:** 73  
**Concorde time:** 3ms  

**Concorde (reference) tour:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 73 | 4ms | 21 | 10 | 11 | 0 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 73 | 4ms | 11 | 6 | 5 | 0 | :white_check_mark: |
| tsp_bb_26_07_10 | 73 | 2ms | 16 | 11 | 5 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 73 | 2ms | 16 | 11 | 5 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 73 | 3ms | 11 | 6 | 5 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0` cost=73
- **tsp_bb_26_07_02:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0` cost=73 (=ref, same tour)
- **tsp_bb_26_07_08_02:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0` cost=73 (=ref, same tour)
- **tsp_bb_26_07_10:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0` cost=73 (=ref, same tour)
- **tsp_bb_26_07_13_01:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0` cost=73 (=ref, same tour)
- **tsp_bb_26_07_13_02:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0` cost=73 (=ref, same tour)

---

## 18. `examples/random/complete/rnd-10-complete-n7.txt` (n=7)

**Concorde optimal cost:** 49  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 6 -> 4 -> 5 -> 3 -> 2 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 49 | 3ms | 13 | 6 | 7 | 0 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 49 | 2ms | 7 | 4 | 3 | 0 | :white_check_mark: |
| tsp_bb_26_07_10 | 49 | 3ms | 12 | 8 | 4 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 49 | 3ms | 12 | 8 | 4 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 49 | 3ms | 7 | 4 | 3 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 6 -> 4 -> 5 -> 3 -> 2 -> 1 -> 0` cost=49
- **tsp_bb_26_07_02:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 0` cost=49 (=ref, different tour)
- **tsp_bb_26_07_08_02:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 0` cost=49 (=ref, different tour)
- **tsp_bb_26_07_10:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 0` cost=49 (=ref, different tour)
- **tsp_bb_26_07_13_01:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 0` cost=49 (=ref, different tour)
- **tsp_bb_26_07_13_02:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 0` cost=49 (=ref, different tour)

---

## 19. `examples/random/complete/rnd-07-complete-n8.txt` (n=8)

**Concorde optimal cost:** 64  
**Concorde time:** 3ms  

**Concorde (reference) tour:** `0 -> 7 -> 4 -> 3 -> 2 -> 1 -> 6 -> 5 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 64 | 4ms | 23 | 11 | 12 | 0 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 64 | 4ms | 12 | 8 | 4 | 0 | :white_check_mark: |
| tsp_bb_26_07_10 | 64 | 5ms | 14 | 10 | 4 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 64 | 4ms | 14 | 10 | 4 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 64 | 3ms | 12 | 8 | 4 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 7 -> 4 -> 3 -> 2 -> 1 -> 6 -> 5 -> 0` cost=64
- **tsp_bb_26_07_02:** `2 -> 3 -> 4 -> 7 -> 0 -> 5 -> 6 -> 1 -> 2` cost=64 (=ref, different tour)
- **tsp_bb_26_07_08_02:** `2 -> 3 -> 4 -> 7 -> 0 -> 5 -> 6 -> 1 -> 2` cost=64 (=ref, different tour)
- **tsp_bb_26_07_10:** `2 -> 3 -> 4 -> 7 -> 0 -> 5 -> 6 -> 1 -> 2` cost=64 (=ref, different tour)
- **tsp_bb_26_07_13_01:** `2 -> 3 -> 4 -> 7 -> 0 -> 5 -> 6 -> 1 -> 2` cost=64 (=ref, different tour)
- **tsp_bb_26_07_13_02:** `2 -> 3 -> 4 -> 7 -> 0 -> 5 -> 6 -> 1 -> 2` cost=64 (=ref, different tour)

---

## 20. `examples/random/complete/rnd-09-complete-n8.txt` (n=8)

**Concorde optimal cost:** 93  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 5 -> 4 -> 6 -> 1 -> 3 -> 2 -> 7 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 93 | 4ms | 77 | 38 | 39 | 0 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 93 | 4ms | 39 | 25 | 14 | 0 | :white_check_mark: |
| tsp_bb_26_07_10 | 93 | 4ms | 41 | 24 | 17 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 93 | 4ms | 39 | 22 | 17 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 93 | 3ms | 39 | 25 | 14 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 5 -> 4 -> 6 -> 1 -> 3 -> 2 -> 7 -> 0` cost=93
- **tsp_bb_26_07_02:** `1 -> 3 -> 2 -> 7 -> 0 -> 5 -> 4 -> 6 -> 1` cost=93 (=ref, different tour)
- **tsp_bb_26_07_08_02:** `1 -> 3 -> 2 -> 7 -> 0 -> 5 -> 4 -> 6 -> 1` cost=93 (=ref, different tour)
- **tsp_bb_26_07_10:** `1 -> 3 -> 2 -> 7 -> 0 -> 5 -> 4 -> 6 -> 1` cost=93 (=ref, different tour)
- **tsp_bb_26_07_13_01:** `1 -> 3 -> 2 -> 7 -> 0 -> 5 -> 4 -> 6 -> 1` cost=93 (=ref, different tour)
- **tsp_bb_26_07_13_02:** `1 -> 3 -> 2 -> 7 -> 0 -> 5 -> 4 -> 6 -> 1` cost=93 (=ref, different tour)

---

## 21. `data/classic/tsplib/burma14.tsp` (n=14)

**Concorde optimal cost:** 3323  
**Concorde time:** 20ms  

**Concorde (reference) tour:** `0 -> 1 -> 13 -> 2 -> 3 -> 4 -> 5 -> 11 -> 6 -> 12 -> 7 -> 10 -> 8 -> 9 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 3323 | 38ms | 18111 | 9162 | 8949 | 214 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 3323 | 34ms | 8949 | 4582 | 4367 | 214 | :white_check_mark: |
| tsp_bb_26_07_10 | 3323 | 49ms | 18410 | 9217 | 9193 | 28 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 3323 | 45ms | 12438 | 5767 | 6671 | 24 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 3323 | 31ms | 7092 | 3578 | 3514 | 195 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 1 -> 13 -> 2 -> 3 -> 4 -> 5 -> 11 -> 6 -> 12 -> 7 -> 10 -> 8 -> 9 -> 0` cost=3323
- **tsp_bb_26_07_02:** `1 -> 0 -> 9 -> 8 -> 10 -> 7 -> 12 -> 6 -> 11 -> 5 -> 4 -> 3 -> 2 -> 13 -> 1` cost=3323 (=ref, different tour)
- **tsp_bb_26_07_08_02:** `1 -> 0 -> 9 -> 8 -> 10 -> 7 -> 12 -> 6 -> 11 -> 5 -> 4 -> 3 -> 2 -> 13 -> 1` cost=3323 (=ref, different tour)
- **tsp_bb_26_07_10:** `1 -> 0 -> 9 -> 8 -> 10 -> 7 -> 12 -> 6 -> 11 -> 5 -> 4 -> 3 -> 2 -> 13 -> 1` cost=3323 (=ref, different tour)
- **tsp_bb_26_07_13_01:** `1 -> 0 -> 9 -> 8 -> 10 -> 7 -> 12 -> 6 -> 11 -> 5 -> 4 -> 3 -> 2 -> 13 -> 1` cost=3323 (=ref, different tour)
- **tsp_bb_26_07_13_02:** `1 -> 0 -> 9 -> 8 -> 10 -> 7 -> 12 -> 6 -> 11 -> 5 -> 4 -> 3 -> 2 -> 13 -> 1` cost=3323 (=ref, different tour)

---

## 22. `data/classic/tsplib/ulysses16.tsp` (n=16)

**Concorde optimal cost:** 6859  
**Concorde time:** 26ms  

**Concorde (reference) tour:** `0 -> 13 -> 12 -> 11 -> 6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 6859 | 16.1s | 15447419 | 7797474 | 7649945 | 147530 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 6859 | 14.2s | 7650761 | 3883908 | 3766853 | 146714 | :white_check_mark: |
| tsp_bb_26_07_10 | 6859 | 16.4s | 9325503 | 4292704 | 5032799 | 138044 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 6859 | 11.8s | 4998386 | 2303570 | 2694816 | 67887 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 6859 | 8.6s | 4027046 | 2011182 | 2015864 | 64101 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 13 -> 12 -> 11 -> 6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0` cost=6859
- **tsp_bb_26_07_02:** `6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0 -> 13 -> 12 -> 11 -> 6` cost=6859 (=ref, different tour)
- **tsp_bb_26_07_08_02:** `6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0 -> 13 -> 12 -> 11 -> 6` cost=6859 (=ref, different tour)
- **tsp_bb_26_07_10:** `6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0 -> 13 -> 12 -> 11 -> 6` cost=6859 (=ref, different tour)
- **tsp_bb_26_07_13_01:** `6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0 -> 13 -> 12 -> 11 -> 6` cost=6859 (=ref, different tour)
- **tsp_bb_26_07_13_02:** `6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0 -> 13 -> 12 -> 11 -> 6` cost=6859 (=ref, different tour)

---

## 23. `data/classic/tsplib/gr17.tsp` (n=17)

**Concorde optimal cost:** 2085  
**Concorde time:** 26ms  

**Concorde (reference) tour:** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 2085 | 2.2s | 1817688 | 920131 | 897557 | 22575 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 2085 | 1.9s | 897557 | 463820 | 433737 | 22575 | :white_check_mark: |
| tsp_bb_26_07_10 | 2085 | 2.6s | 1305642 | 676329 | 629313 | 41747 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 2085 | 2.8s | 964168 | 486803 | 477365 | 33709 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 2085 | 1.2s | 452869 | 227558 | 225311 | 12105 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0` cost=2085
- **tsp_bb_26_07_02:** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0` cost=2085 (=ref, same tour)
- **tsp_bb_26_07_08_02:** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0` cost=2085 (=ref, same tour)
- **tsp_bb_26_07_10:** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0` cost=2085 (=ref, same tour)
- **tsp_bb_26_07_13_01:** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0` cost=2085 (=ref, same tour)
- **tsp_bb_26_07_13_02:** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0` cost=2085 (=ref, same tour)

---

## 24. `data/classic/tsplib/gr21.tsp` (n=21)

**Concorde optimal cost:** 2707  
**Concorde time:** 13ms  

**Concorde (reference) tour:** `0 -> 6 -> 7 -> 5 -> 15 -> 4 -> 8 -> 2 -> 1 -> 20 -> 14 -> 13 -> 12 -> 17 -> 9 -> 16 -> 18 -> 19 -> 10 -> 3 -> 11 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 2707 | 78ms | 34213 | 17314 | 16899 | 416 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 2707 | 73ms | 16899 | 9911 | 6988 | 416 | :white_check_mark: |
| tsp_bb_26_07_10 | 2707 | 135ms | 38777 | 24876 | 13901 | 926 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 2707 | 139ms | 36089 | 23161 | 12928 | 913 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 2707 | 61ms | 13625 | 7906 | 5719 | 342 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 6 -> 7 -> 5 -> 15 -> 4 -> 8 -> 2 -> 1 -> 20 -> 14 -> 13 -> 12 -> 17 -> 9 -> 16 -> 18 -> 19 -> 10 -> 3 -> 11 -> 0` cost=2707
- **tsp_bb_26_07_02:** `2 -> 8 -> 4 -> 15 -> 5 -> 7 -> 6 -> 0 -> 11 -> 3 -> 10 -> 19 -> 18 -> 16 -> 9 -> 17 -> 12 -> 13 -> 14 -> 20 -> 1 -> 2` cost=2707 (=ref, different tour)
- **tsp_bb_26_07_08_02:** `2 -> 8 -> 4 -> 15 -> 5 -> 7 -> 6 -> 0 -> 11 -> 3 -> 10 -> 19 -> 18 -> 16 -> 9 -> 17 -> 12 -> 13 -> 14 -> 20 -> 1 -> 2` cost=2707 (=ref, different tour)
- **tsp_bb_26_07_10:** `2 -> 8 -> 4 -> 15 -> 5 -> 7 -> 6 -> 0 -> 11 -> 3 -> 10 -> 19 -> 18 -> 16 -> 9 -> 17 -> 12 -> 13 -> 14 -> 20 -> 1 -> 2` cost=2707 (=ref, different tour)
- **tsp_bb_26_07_13_01:** `2 -> 8 -> 4 -> 15 -> 5 -> 7 -> 6 -> 0 -> 11 -> 3 -> 10 -> 19 -> 18 -> 16 -> 9 -> 17 -> 12 -> 13 -> 14 -> 20 -> 1 -> 2` cost=2707 (=ref, different tour)
- **tsp_bb_26_07_13_02:** `2 -> 8 -> 4 -> 15 -> 5 -> 7 -> 6 -> 0 -> 11 -> 3 -> 10 -> 19 -> 18 -> 16 -> 9 -> 17 -> 12 -> 13 -> 14 -> 20 -> 1 -> 2` cost=2707 (=ref, different tour)

---

## 25. `data/classic/tsplib/gr24.tsp` (n=24)

**Concorde optimal cost:** 1272  
**Concorde time:** 22ms  

**Concorde (reference) tour:** `0 -> 11 -> 3 -> 22 -> 8 -> 12 -> 13 -> 19 -> 1 -> 14 -> 18 -> 21 -> 17 -> 16 -> 9 -> 4 -> 20 -> 7 -> 23 -> 5 -> 6 -> 2 -> 10 -> 15 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 1272 | 1.2s | 571607 | 287023 | 284584 | 2440 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 1272 | 992ms | 284584 | 158220 | 126364 | 2440 | :white_check_mark: |
| tsp_bb_26_07_10 | 1272 | 1.7s | 534971 | 297291 | 237680 | 29 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 1272 | 1.6s | 391094 | 216073 | 175021 | 9 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 1272 | 617ms | 151939 | 82669 | 69270 | 1025 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 11 -> 3 -> 22 -> 8 -> 12 -> 13 -> 19 -> 1 -> 14 -> 18 -> 21 -> 17 -> 16 -> 9 -> 4 -> 20 -> 7 -> 23 -> 5 -> 6 -> 2 -> 10 -> 15 -> 0` cost=1272
- **tsp_bb_26_07_02:** `3 -> 11 -> 0 -> 15 -> 10 -> 2 -> 6 -> 5 -> 23 -> 7 -> 20 -> 4 -> 9 -> 16 -> 17 -> 21 -> 18 -> 14 -> 1 -> 19 -> 13 -> 12 -> 8 -> 22 -> 3` cost=1272 (=ref, different tour)
- **tsp_bb_26_07_08_02:** `3 -> 11 -> 0 -> 15 -> 10 -> 2 -> 6 -> 5 -> 23 -> 7 -> 20 -> 4 -> 9 -> 16 -> 17 -> 21 -> 18 -> 14 -> 1 -> 19 -> 13 -> 12 -> 8 -> 22 -> 3` cost=1272 (=ref, different tour)
- **tsp_bb_26_07_10:** `3 -> 11 -> 0 -> 15 -> 10 -> 2 -> 6 -> 5 -> 23 -> 7 -> 20 -> 4 -> 9 -> 16 -> 17 -> 21 -> 18 -> 14 -> 1 -> 19 -> 13 -> 12 -> 8 -> 22 -> 3` cost=1272 (=ref, different tour)
- **tsp_bb_26_07_13_01:** `3 -> 11 -> 0 -> 15 -> 10 -> 2 -> 6 -> 5 -> 23 -> 7 -> 20 -> 4 -> 9 -> 16 -> 17 -> 21 -> 18 -> 14 -> 1 -> 19 -> 13 -> 12 -> 8 -> 22 -> 3` cost=1272 (=ref, different tour)
- **tsp_bb_26_07_13_02:** `3 -> 11 -> 0 -> 15 -> 10 -> 2 -> 6 -> 5 -> 23 -> 7 -> 20 -> 4 -> 9 -> 16 -> 17 -> 21 -> 18 -> 14 -> 1 -> 19 -> 13 -> 12 -> 8 -> 22 -> 3` cost=1272 (=ref, different tour)

---

## 26. `data/classic/tsplib/fri26.tsp` (n=26)

**Concorde optimal cost:** 937  
**Concorde time:** 28ms  

**Concorde (reference) tour:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 7 -> 8 -> 9 -> 13 -> 14 -> 12 -> 11 -> 10 -> 15 -> 18 -> 19 -> 17 -> 16 -> 20 -> 21 -> 25 -> 22 -> 23 -> 24 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 937 | 1.9s | 984246 | 492159 | 492087 | 73 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 937 | 1.7s | 492087 | 264044 | 228043 | 73 | :white_check_mark: |
| tsp_bb_26_07_10 | 937 | 7.9s | 2647362 | 1470662 | 1176700 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 937 | 3.9s | 995871 | 545614 | 450257 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 937 | 805ms | 195433 | 95295 | 100138 | 33 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 7 -> 8 -> 9 -> 13 -> 14 -> 12 -> 11 -> 10 -> 15 -> 18 -> 19 -> 17 -> 16 -> 20 -> 21 -> 25 -> 22 -> 23 -> 24 -> 0` cost=937
- **tsp_bb_26_07_02:** `16 -> 20 -> 21 -> 25 -> 22 -> 23 -> 24 -> 0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 7 -> 8 -> 9 -> 13 -> 14 -> 11 -> 12 -> 10 -> 15 -> 18 -> 19 -> 17 -> 16` cost=937 (=ref, different tour)
- **tsp_bb_26_07_08_02:** `16 -> 20 -> 21 -> 25 -> 22 -> 23 -> 24 -> 0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 7 -> 8 -> 9 -> 13 -> 14 -> 11 -> 12 -> 10 -> 15 -> 18 -> 19 -> 17 -> 16` cost=937 (=ref, different tour)
- **tsp_bb_26_07_10:** `16 -> 20 -> 21 -> 25 -> 22 -> 23 -> 24 -> 0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 7 -> 8 -> 9 -> 13 -> 14 -> 11 -> 12 -> 10 -> 15 -> 18 -> 19 -> 17 -> 16` cost=937 (=ref, different tour)
- **tsp_bb_26_07_13_01:** `16 -> 20 -> 21 -> 25 -> 22 -> 23 -> 24 -> 0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 7 -> 8 -> 9 -> 13 -> 14 -> 11 -> 12 -> 10 -> 15 -> 18 -> 19 -> 17 -> 16` cost=937 (=ref, different tour)
- **tsp_bb_26_07_13_02:** `16 -> 20 -> 21 -> 25 -> 22 -> 23 -> 24 -> 0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 7 -> 8 -> 9 -> 13 -> 14 -> 11 -> 12 -> 10 -> 15 -> 18 -> 19 -> 17 -> 16` cost=937 (=ref, different tour)

---

## 27. `data/classic/tsplib/bayg29.tsp` (n=29)

**Concorde optimal cost:** 1610  
**Concorde time:** 24ms  

**Concorde (reference) tour:** `0 -> 27 -> 5 -> 11 -> 8 -> 25 -> 2 -> 28 -> 4 -> 20 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 13 -> 16 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 7 -> 26 -> 15 -> 12 -> 23 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 1610 | 27.8s | 12732970 | 6376947 | 6356023 | 20925 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 1610 | 24.8s | 6356023 | 3605784 | 2750239 | 20925 | :white_check_mark: |
| tsp_bb_26_07_10 | 1610 | 1.3m | 22711921 | 12520142 | 10191779 | 15396 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 1610 | 1.6m | 21749867 | 11918901 | 9830966 | 14692 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 1610 | 20.0s | 3812923 | 2071016 | 1741907 | 11673 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 27 -> 5 -> 11 -> 8 -> 25 -> 2 -> 28 -> 4 -> 20 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 13 -> 16 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 7 -> 26 -> 15 -> 12 -> 23 -> 0` cost=1610
- **tsp_bb_26_07_02:** `0 -> 27 -> 5 -> 11 -> 8 -> 25 -> 2 -> 28 -> 4 -> 20 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 13 -> 16 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 7 -> 26 -> 15 -> 12 -> 23 -> 0` cost=1610 (=ref, same tour)
- **tsp_bb_26_07_08_02:** `0 -> 27 -> 5 -> 11 -> 8 -> 25 -> 2 -> 28 -> 4 -> 20 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 13 -> 16 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 7 -> 26 -> 15 -> 12 -> 23 -> 0` cost=1610 (=ref, same tour)
- **tsp_bb_26_07_10:** `0 -> 27 -> 5 -> 11 -> 8 -> 25 -> 2 -> 28 -> 4 -> 20 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 13 -> 16 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 7 -> 26 -> 15 -> 12 -> 23 -> 0` cost=1610 (=ref, same tour)
- **tsp_bb_26_07_13_01:** `0 -> 27 -> 5 -> 11 -> 8 -> 25 -> 2 -> 28 -> 4 -> 20 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 13 -> 16 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 7 -> 26 -> 15 -> 12 -> 23 -> 0` cost=1610 (=ref, same tour)
- **tsp_bb_26_07_13_02:** `0 -> 27 -> 5 -> 11 -> 8 -> 25 -> 2 -> 28 -> 4 -> 20 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 13 -> 16 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 7 -> 26 -> 15 -> 12 -> 23 -> 0` cost=1610 (=ref, same tour)

---

## 28. `data/classic/tsplib/bays29.tsp` (n=29)

**Concorde optimal cost:** 2020  
**Concorde time:** 33ms  

**Concorde (reference) tour:** `0 -> 27 -> 5 -> 11 -> 8 -> 4 -> 25 -> 28 -> 2 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 16 -> 13 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 26 -> 7 -> 23 -> 15 -> 12 -> 20 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 2020 | 5.7m | 162609801 | 81440628 | 81169173 | 271456 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 2020 | 5.1m | 81169173 | 44122386 | 37046787 | 271456 | :white_check_mark: |
| tsp_bb_26_07_10 | 2020 | 8.9m | 158351409 | 88680956 | 69670453 | 683590 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 2020 | 12.1m | 160886767 | 90836187 | 70050580 | 676182 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 2020 | 4.5m | 58994338 | 30891270 | 28103068 | 178461 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 27 -> 5 -> 11 -> 8 -> 4 -> 25 -> 28 -> 2 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 16 -> 13 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 26 -> 7 -> 23 -> 15 -> 12 -> 20 -> 0` cost=2020
- **tsp_bb_26_07_02:** `7 -> 26 -> 22 -> 6 -> 24 -> 18 -> 10 -> 21 -> 13 -> 16 -> 17 -> 14 -> 3 -> 9 -> 19 -> 1 -> 2 -> 28 -> 25 -> 4 -> 8 -> 11 -> 5 -> 27 -> 0 -> 20 -> 12 -> 15 -> 23 -> 7` cost=2020 (=ref, different tour)
- **tsp_bb_26_07_08_02:** `7 -> 26 -> 22 -> 6 -> 24 -> 18 -> 10 -> 21 -> 13 -> 16 -> 17 -> 14 -> 3 -> 9 -> 19 -> 1 -> 2 -> 28 -> 25 -> 4 -> 8 -> 11 -> 5 -> 27 -> 0 -> 20 -> 12 -> 15 -> 23 -> 7` cost=2020 (=ref, different tour)
- **tsp_bb_26_07_10:** `7 -> 26 -> 22 -> 6 -> 24 -> 18 -> 10 -> 21 -> 13 -> 16 -> 17 -> 14 -> 3 -> 9 -> 19 -> 1 -> 2 -> 28 -> 25 -> 4 -> 8 -> 11 -> 5 -> 27 -> 0 -> 20 -> 12 -> 15 -> 23 -> 7` cost=2020 (=ref, different tour)
- **tsp_bb_26_07_13_01:** `7 -> 26 -> 22 -> 6 -> 24 -> 18 -> 10 -> 21 -> 13 -> 16 -> 17 -> 14 -> 3 -> 9 -> 19 -> 1 -> 2 -> 28 -> 25 -> 4 -> 8 -> 11 -> 5 -> 27 -> 0 -> 20 -> 12 -> 15 -> 23 -> 7` cost=2020 (=ref, different tour)
- **tsp_bb_26_07_13_02:** `7 -> 26 -> 22 -> 6 -> 24 -> 18 -> 10 -> 21 -> 13 -> 16 -> 17 -> 14 -> 3 -> 9 -> 19 -> 1 -> 2 -> 28 -> 25 -> 4 -> 8 -> 11 -> 5 -> 27 -> 0 -> 20 -> 12 -> 15 -> 23 -> 7` cost=2020 (=ref, different tour)

---

## 29. `data/classic/national/wi29.tsp` (n=29)

**Concorde optimal cost:** 27603  
**Concorde time:** 38ms  

**Concorde (reference) tour:** `0 -> 4 -> 7 -> 3 -> 2 -> 6 -> 8 -> 12 -> 13 -> 15 -> 23 -> 26 -> 24 -> 19 -> 25 -> 27 -> 28 -> 22 -> 21 -> 20 -> 16 -> 17 -> 18 -> 14 -> 11 -> 10 -> 9 -> 5 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 27603 | 201ms | 77709 | 38866 | 38843 | 24 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 27603 | 180ms | 38843 | 22423 | 16420 | 24 | :white_check_mark: |
| tsp_bb_26_07_10 | 27603 | 1.3m | 24043308 | 14867815 | 9175493 | 1877501 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 27603 | 51.8s | 12598510 | 7888715 | 4709795 | 1263652 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 27603 | 119ms | 20640 | 11230 | 9410 | 20 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 4 -> 7 -> 3 -> 2 -> 6 -> 8 -> 12 -> 13 -> 15 -> 23 -> 26 -> 24 -> 19 -> 25 -> 27 -> 28 -> 22 -> 21 -> 20 -> 16 -> 17 -> 18 -> 14 -> 11 -> 10 -> 9 -> 5 -> 1 -> 0` cost=27603
- **tsp_bb_26_07_02:** `21 -> 22 -> 28 -> 27 -> 25 -> 19 -> 24 -> 26 -> 23 -> 15 -> 13 -> 12 -> 8 -> 6 -> 2 -> 3 -> 7 -> 4 -> 0 -> 1 -> 5 -> 9 -> 10 -> 11 -> 14 -> 18 -> 17 -> 16 -> 20 -> 21` cost=27603 (=ref, different tour)
- **tsp_bb_26_07_08_02:** `21 -> 22 -> 28 -> 27 -> 25 -> 19 -> 24 -> 26 -> 23 -> 15 -> 13 -> 12 -> 8 -> 6 -> 2 -> 3 -> 7 -> 4 -> 0 -> 1 -> 5 -> 9 -> 10 -> 11 -> 14 -> 18 -> 17 -> 16 -> 20 -> 21` cost=27603 (=ref, different tour)
- **tsp_bb_26_07_10:** `21 -> 22 -> 28 -> 27 -> 25 -> 19 -> 24 -> 26 -> 23 -> 15 -> 13 -> 12 -> 8 -> 6 -> 2 -> 3 -> 7 -> 4 -> 0 -> 1 -> 5 -> 9 -> 10 -> 11 -> 14 -> 18 -> 17 -> 16 -> 20 -> 21` cost=27603 (=ref, different tour)
- **tsp_bb_26_07_13_01:** `21 -> 22 -> 28 -> 27 -> 25 -> 19 -> 24 -> 26 -> 23 -> 15 -> 13 -> 12 -> 8 -> 6 -> 2 -> 3 -> 7 -> 4 -> 0 -> 1 -> 5 -> 9 -> 10 -> 11 -> 14 -> 18 -> 17 -> 16 -> 20 -> 21` cost=27603 (=ref, different tour)
- **tsp_bb_26_07_13_02:** `21 -> 22 -> 28 -> 27 -> 25 -> 19 -> 24 -> 26 -> 23 -> 15 -> 13 -> 12 -> 8 -> 6 -> 2 -> 3 -> 7 -> 4 -> 0 -> 1 -> 5 -> 9 -> 10 -> 11 -> 14 -> 18 -> 17 -> 16 -> 20 -> 21` cost=27603 (=ref, different tour)

---

## 30. `data/classic/national/dj38.tsp` (n=38)

**Concorde optimal cost:** 6656  
**Concorde time:** 37ms  

**Concorde (reference) tour:** `0 -> 9 -> 13 -> 20 -> 28 -> 29 -> 31 -> 34 -> 36 -> 37 -> 32 -> 33 -> 35 -> 30 -> 26 -> 27 -> 23 -> 21 -> 24 -> 25 -> 22 -> 19 -> 14 -> 12 -> 15 -> 16 -> 17 -> 18 -> 10 -> 11 -> 8 -> 7 -> 6 -> 5 -> 4 -> 2 -> 3 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 6656 | 115ms | 25733 | 12866 | 12867 | 0 | :white_check_mark: |
| tsp_bb_26_07_08_02 | 6656 | 102ms | 12867 | 7990 | 4877 | 0 | :white_check_mark: |
| tsp_bb_26_07_10 | 6656 | 6.7s | 1305978 | 790892 | 515086 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 6656 | 3.1s | 485818 | 284445 | 201373 | 0 | :white_check_mark: |
| tsp_bb_26_07_13_02 | 6656 | 53ms | 4292 | 2290 | 2002 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 9 -> 13 -> 20 -> 28 -> 29 -> 31 -> 34 -> 36 -> 37 -> 32 -> 33 -> 35 -> 30 -> 26 -> 27 -> 23 -> 21 -> 24 -> 25 -> 22 -> 19 -> 14 -> 12 -> 15 -> 16 -> 17 -> 18 -> 10 -> 11 -> 8 -> 7 -> 6 -> 5 -> 4 -> 2 -> 3 -> 1 -> 0` cost=6656
- **tsp_bb_26_07_02:** `1 -> 0 -> 9 -> 13 -> 20 -> 28 -> 29 -> 31 -> 34 -> 36 -> 37 -> 32 -> 33 -> 35 -> 30 -> 26 -> 27 -> 23 -> 21 -> 24 -> 25 -> 22 -> 19 -> 14 -> 12 -> 15 -> 16 -> 17 -> 18 -> 10 -> 11 -> 8 -> 7 -> 6 -> 5 -> 4 -> 2 -> 3 -> 1` cost=6656 (=ref, different tour)
- **tsp_bb_26_07_08_02:** `1 -> 0 -> 9 -> 13 -> 20 -> 28 -> 29 -> 31 -> 34 -> 36 -> 37 -> 32 -> 33 -> 35 -> 30 -> 26 -> 27 -> 23 -> 21 -> 24 -> 25 -> 22 -> 19 -> 14 -> 12 -> 15 -> 16 -> 17 -> 18 -> 10 -> 11 -> 8 -> 7 -> 6 -> 5 -> 4 -> 2 -> 3 -> 1` cost=6656 (=ref, different tour)
- **tsp_bb_26_07_10:** `1 -> 0 -> 9 -> 13 -> 20 -> 28 -> 29 -> 31 -> 34 -> 36 -> 37 -> 32 -> 33 -> 35 -> 30 -> 26 -> 27 -> 23 -> 21 -> 24 -> 25 -> 22 -> 19 -> 14 -> 12 -> 15 -> 16 -> 17 -> 18 -> 10 -> 11 -> 8 -> 7 -> 6 -> 5 -> 4 -> 2 -> 3 -> 1` cost=6656 (=ref, different tour)
- **tsp_bb_26_07_13_01:** `1 -> 0 -> 9 -> 13 -> 20 -> 28 -> 29 -> 31 -> 34 -> 36 -> 37 -> 32 -> 33 -> 35 -> 30 -> 26 -> 27 -> 23 -> 21 -> 24 -> 25 -> 22 -> 19 -> 14 -> 12 -> 15 -> 16 -> 17 -> 18 -> 10 -> 11 -> 8 -> 7 -> 6 -> 5 -> 4 -> 2 -> 3 -> 1` cost=6656 (=ref, different tour)
- **tsp_bb_26_07_13_02:** `1 -> 0 -> 9 -> 13 -> 20 -> 28 -> 29 -> 31 -> 34 -> 36 -> 37 -> 32 -> 33 -> 35 -> 30 -> 26 -> 27 -> 23 -> 21 -> 24 -> 25 -> 22 -> 19 -> 14 -> 12 -> 15 -> 16 -> 17 -> 18 -> 10 -> 11 -> 8 -> 7 -> 6 -> 5 -> 4 -> 2 -> 3 -> 1` cost=6656 (=ref, different tour)

---

## 31. `data/classic/tsplib/dantzig42.tsp` (n=42)

**Concorde optimal cost:** 699  
**Concorde time:** 40ms  

**Concorde (reference) tour:** `0 -> 41 -> 40 -> 39 -> 38 -> 37 -> 36 -> 35 -> 34 -> 33 -> 32 -> 31 -> 30 -> 29 -> 28 -> 27 -> 26 -> 25 -> 24 -> 23 -> 22 -> 21 -> 20 -> 19 -> 18 -> 17 -> 16 -> 15 -> 14 -> 13 -> 12 -> 11 -> 10 -> 9 -> 8 -> 7 -> 6 -> 5 -> 4 -> 3 -> 2 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_08_02 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_10 | 699 | 23.4m | 289823275 | 168662422 | 121160853 | 46 | :white_check_mark: |
| tsp_bb_26_07_13_01 | 699 | 21.3m | 158011479 | 84420324 | 73591155 | 2 | :white_check_mark: |
| tsp_bb_26_07_13_02 | TIMEOUT | 30.0m | - | - | - | - | - |

**Tours found:**
- **Concorde (reference):** `0 -> 41 -> 40 -> 39 -> 38 -> 37 -> 36 -> 35 -> 34 -> 33 -> 32 -> 31 -> 30 -> 29 -> 28 -> 27 -> 26 -> 25 -> 24 -> 23 -> 22 -> 21 -> 20 -> 19 -> 18 -> 17 -> 16 -> 15 -> 14 -> 13 -> 12 -> 11 -> 10 -> 9 -> 8 -> 7 -> 6 -> 5 -> 4 -> 3 -> 2 -> 1 -> 0` cost=699
- **tsp_bb_26_07_10:** `23 -> 22 -> 21 -> 20 -> 19 -> 18 -> 17 -> 16 -> 15 -> 14 -> 13 -> 12 -> 11 -> 10 -> 9 -> 8 -> 7 -> 6 -> 5 -> 4 -> 3 -> 2 -> 1 -> 0 -> 41 -> 40 -> 39 -> 38 -> 37 -> 36 -> 35 -> 34 -> 33 -> 32 -> 31 -> 30 -> 29 -> 28 -> 27 -> 26 -> 25 -> 24 -> 23` cost=699 (=ref, different tour)
- **tsp_bb_26_07_13_01:** `23 -> 22 -> 21 -> 20 -> 19 -> 18 -> 17 -> 16 -> 15 -> 14 -> 13 -> 12 -> 11 -> 10 -> 9 -> 8 -> 7 -> 6 -> 5 -> 4 -> 3 -> 2 -> 1 -> 0 -> 41 -> 40 -> 39 -> 38 -> 37 -> 36 -> 35 -> 34 -> 33 -> 32 -> 31 -> 30 -> 29 -> 28 -> 27 -> 26 -> 25 -> 24 -> 23` cost=699 (=ref, different tour)

---

## 32. `data/classic/tsplib/swiss42.tsp` (n=42)

**Concorde optimal cost:** 1273  
**Concorde time:** 29ms  

**Concorde (reference) tour:** `0 -> 1 -> 6 -> 4 -> 3 -> 2 -> 27 -> 28 -> 29 -> 30 -> 38 -> 22 -> 39 -> 21 -> 24 -> 40 -> 23 -> 41 -> 9 -> 8 -> 10 -> 25 -> 11 -> 12 -> 18 -> 26 -> 5 -> 13 -> 19 -> 14 -> 16 -> 15 -> 37 -> 7 -> 17 -> 31 -> 36 -> 35 -> 20 -> 33 -> 34 -> 32 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_08_02 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_10 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_13_01 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_13_02 | TIMEOUT | 30.0m | - | - | - | - | - |

**Tours found:**
- **Concorde (reference):** `0 -> 1 -> 6 -> 4 -> 3 -> 2 -> 27 -> 28 -> 29 -> 30 -> 38 -> 22 -> 39 -> 21 -> 24 -> 40 -> 23 -> 41 -> 9 -> 8 -> 10 -> 25 -> 11 -> 12 -> 18 -> 26 -> 5 -> 13 -> 19 -> 14 -> 16 -> 15 -> 37 -> 7 -> 17 -> 31 -> 36 -> 35 -> 20 -> 33 -> 34 -> 32 -> 0` cost=1273

---

## 33. `data/classic/tsplib/att48.tsp` (n=48)

**Concorde optimal cost:** 10628  
**Concorde time:** 83ms  

**Concorde (reference) tour:** `0 -> 8 -> 39 -> 14 -> 11 -> 10 -> 12 -> 24 -> 13 -> 22 -> 2 -> 21 -> 15 -> 40 -> 33 -> 28 -> 1 -> 25 -> 3 -> 34 -> 44 -> 9 -> 23 -> 41 -> 4 -> 47 -> 38 -> 31 -> 20 -> 46 -> 19 -> 32 -> 45 -> 35 -> 29 -> 42 -> 16 -> 26 -> 18 -> 36 -> 5 -> 27 -> 6 -> 17 -> 43 -> 30 -> 37 -> 7 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_08_02 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_10 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_13_01 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_13_02 | TIMEOUT | 30.0m | - | - | - | - | - |

**Tours found:**
- **Concorde (reference):** `0 -> 8 -> 39 -> 14 -> 11 -> 10 -> 12 -> 24 -> 13 -> 22 -> 2 -> 21 -> 15 -> 40 -> 33 -> 28 -> 1 -> 25 -> 3 -> 34 -> 44 -> 9 -> 23 -> 41 -> 4 -> 47 -> 38 -> 31 -> 20 -> 46 -> 19 -> 32 -> 45 -> 35 -> 29 -> 42 -> 16 -> 26 -> 18 -> 36 -> 5 -> 27 -> 6 -> 17 -> 43 -> 30 -> 37 -> 7 -> 0` cost=10628

---

## 34. `data/classic/tsplib/gr48.tsp` (n=48)

**Concorde optimal cost:** 5046  
**Concorde time:** 90ms  

**Concorde (reference) tour:** `0 -> 12 -> 47 -> 15 -> 10 -> 35 -> 25 -> 5 -> 13 -> 8 -> 31 -> 26 -> 16 -> 20 -> 21 -> 7 -> 32 -> 4 -> 30 -> 11 -> 9 -> 14 -> 23 -> 36 -> 46 -> 42 -> 44 -> 1 -> 39 -> 38 -> 41 -> 34 -> 19 -> 37 -> 29 -> 3 -> 18 -> 2 -> 24 -> 22 -> 33 -> 17 -> 45 -> 40 -> 43 -> 27 -> 6 -> 28 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_08_02 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_10 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_13_01 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_13_02 | TIMEOUT | 30.0m | - | - | - | - | - |

**Tours found:**
- **Concorde (reference):** `0 -> 12 -> 47 -> 15 -> 10 -> 35 -> 25 -> 5 -> 13 -> 8 -> 31 -> 26 -> 16 -> 20 -> 21 -> 7 -> 32 -> 4 -> 30 -> 11 -> 9 -> 14 -> 23 -> 36 -> 46 -> 42 -> 44 -> 1 -> 39 -> 38 -> 41 -> 34 -> 19 -> 37 -> 29 -> 3 -> 18 -> 2 -> 24 -> 22 -> 33 -> 17 -> 45 -> 40 -> 43 -> 27 -> 6 -> 28 -> 0` cost=5046

---

## 35. `data/classic/tsplib/hk48.tsp` (n=48)

**Concorde optimal cost:** 11461  
**Concorde time:** 37ms  

**Concorde (reference) tour:** `0 -> 1 -> 47 -> 14 -> 42 -> 20 -> 32 -> 29 -> 22 -> 8 -> 9 -> 39 -> 35 -> 33 -> 5 -> 7 -> 46 -> 6 -> 37 -> 13 -> 17 -> 11 -> 21 -> 12 -> 27 -> 31 -> 24 -> 2 -> 4 -> 28 -> 25 -> 40 -> 23 -> 34 -> 16 -> 30 -> 19 -> 10 -> 15 -> 41 -> 3 -> 45 -> 44 -> 38 -> 43 -> 26 -> 36 -> 18 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_08_02 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_10 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_13_01 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_13_02 | TIMEOUT | 30.0m | - | - | - | - | - |

**Tours found:**
- **Concorde (reference):** `0 -> 1 -> 47 -> 14 -> 42 -> 20 -> 32 -> 29 -> 22 -> 8 -> 9 -> 39 -> 35 -> 33 -> 5 -> 7 -> 46 -> 6 -> 37 -> 13 -> 17 -> 11 -> 21 -> 12 -> 27 -> 31 -> 24 -> 2 -> 4 -> 28 -> 25 -> 40 -> 23 -> 34 -> 16 -> 30 -> 19 -> 10 -> 15 -> 41 -> 3 -> 45 -> 44 -> 38 -> 43 -> 26 -> 36 -> 18 -> 0` cost=11461

---

## 36. `data/classic/tsplib/eil51.tsp` (n=51)

**Concorde optimal cost:** 426  
**Concorde time:** 54ms  

**Concorde (reference) tour:** `0 -> 21 -> 7 -> 25 -> 30 -> 27 -> 2 -> 35 -> 34 -> 19 -> 1 -> 28 -> 20 -> 15 -> 49 -> 33 -> 29 -> 8 -> 48 -> 9 -> 38 -> 32 -> 44 -> 14 -> 43 -> 41 -> 39 -> 18 -> 40 -> 12 -> 24 -> 13 -> 23 -> 42 -> 6 -> 22 -> 47 -> 5 -> 26 -> 50 -> 45 -> 11 -> 46 -> 17 -> 3 -> 16 -> 36 -> 4 -> 37 -> 10 -> 31 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_08_02 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_10 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_13_01 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_13_02 | TIMEOUT | 30.0m | - | - | - | - | - |

**Tours found:**
- **Concorde (reference):** `0 -> 21 -> 7 -> 25 -> 30 -> 27 -> 2 -> 35 -> 34 -> 19 -> 1 -> 28 -> 20 -> 15 -> 49 -> 33 -> 29 -> 8 -> 48 -> 9 -> 38 -> 32 -> 44 -> 14 -> 43 -> 41 -> 39 -> 18 -> 40 -> 12 -> 24 -> 13 -> 23 -> 42 -> 6 -> 22 -> 47 -> 5 -> 26 -> 50 -> 45 -> 11 -> 46 -> 17 -> 3 -> 16 -> 36 -> 4 -> 37 -> 10 -> 31 -> 0` cost=426

---

## 37. `data/classic/tsplib/berlin52.tsp` (n=52)

**Concorde optimal cost:** 7542  
**Concorde time:** 52ms  

**Concorde (reference) tour:** `0 -> 48 -> 31 -> 44 -> 18 -> 40 -> 7 -> 8 -> 9 -> 42 -> 32 -> 50 -> 10 -> 51 -> 13 -> 12 -> 46 -> 25 -> 26 -> 27 -> 11 -> 24 -> 3 -> 5 -> 14 -> 4 -> 23 -> 47 -> 37 -> 36 -> 39 -> 38 -> 35 -> 34 -> 33 -> 43 -> 45 -> 15 -> 28 -> 49 -> 19 -> 22 -> 29 -> 1 -> 6 -> 41 -> 20 -> 16 -> 2 -> 17 -> 30 -> 21 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_08_02 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_10 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_13_01 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_13_02 | TIMEOUT | 30.0m | - | - | - | - | - |

**Tours found:**
- **Concorde (reference):** `0 -> 48 -> 31 -> 44 -> 18 -> 40 -> 7 -> 8 -> 9 -> 42 -> 32 -> 50 -> 10 -> 51 -> 13 -> 12 -> 46 -> 25 -> 26 -> 27 -> 11 -> 24 -> 3 -> 5 -> 14 -> 4 -> 23 -> 47 -> 37 -> 36 -> 39 -> 38 -> 35 -> 34 -> 33 -> 43 -> 45 -> 15 -> 28 -> 49 -> 19 -> 22 -> 29 -> 1 -> 6 -> 41 -> 20 -> 16 -> 2 -> 17 -> 30 -> 21 -> 0` cost=7542

---
