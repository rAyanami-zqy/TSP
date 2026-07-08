# TSP Algorithm Comparison

## Algorithms Compared

| Algorithm | Description |
|---|---|
| **Concorde** | State-of-the-art Concorde TSP solver (exact, with QSopt LP) — **reference** |
| **tsp_bb_26_07_02** | Branch & Bound solver — smart branching (legacy) |
| **tsp_bb_26_07_06** | Branch & Bound solver — BP (Branch Partitioning) |
| **tsp_bb_26_07_07** | Branch & Bound solver — BP (Branch Partitioning) |
| **tsp_bb_26_07_08** | Branch & Bound solver — BP (Branch Partitioning) |

**Instances:** 39 from `examples` (n <= 49), `data/classic/tsplib` (n <= 59), `data/classic/national` (n <= 59)  
**Timeout:** 1800s (0h) per method per instance  
**Reference:** Concorde exact solver  
**Solver versions:** `tsp_bb_26_07_02`, `tsp_bb_26_07_06`, `tsp_bb_26_07_07`, `tsp_bb_26_07_08`  

---

## 1. `examples/random/complete/rnd-01-complete-n4.txt` (n=4)

**Concorde optimal cost:** 45  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 3 -> 2 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 45 | 3ms | 5 | 2 | 3 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 45 | 3ms | 5 | 2 | 2 | 0 | :white_check_mark: |
| tsp_bb_26_07_07 | 45 | 3ms | 5 | 2 | 2 | 0 | :white_check_mark: |
| tsp_bb_26_07_08 | 45 | 3ms | 5 | 2 | 2 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 3 -> 2 -> 1 -> 0` cost=45
- **tsp_bb_26_07_02:** `0 -> 3 -> 2 -> 1 -> 0` cost=45 (=ref, same tour)
- **tsp_bb_26_07_06:** `0 -> 3 -> 2 -> 1 -> 0` cost=45 (=ref, same tour)
- **tsp_bb_26_07_07:** `0 -> 3 -> 2 -> 1 -> 0` cost=45 (=ref, same tour)
- **tsp_bb_26_07_08:** `0 -> 3 -> 2 -> 1 -> 0` cost=45 (=ref, same tour)

---

## 2. `examples/random/complete/rnd-03-complete-n4.txt` (n=4)

**Concorde optimal cost:** 102  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 1 -> 3 -> 2 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 102 | 3ms | 1 | 0 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 102 | 3ms | 1 | 0 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_07 | 102 | 3ms | 1 | 0 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_08 | 102 | 3ms | 1 | 0 | 1 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 1 -> 3 -> 2 -> 0` cost=102
- **tsp_bb_26_07_02:** `0 -> 1 -> 3 -> 2 -> 0` cost=102 (=ref, same tour)
- **tsp_bb_26_07_06:** `0 -> 1 -> 3 -> 2 -> 0` cost=102 (=ref, same tour)
- **tsp_bb_26_07_07:** `0 -> 1 -> 3 -> 2 -> 0` cost=102 (=ref, same tour)
- **tsp_bb_26_07_08:** `0 -> 1 -> 3 -> 2 -> 0` cost=102 (=ref, same tour)

---

## 3. `examples/five-city.tsp` (n=5)

**Concorde optimal cost:** 26  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 2 -> 3 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 26 | 3ms | 13 | 6 | 7 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 26 | 3ms | 17 | 4 | 8 | 0 | :white_check_mark: |
| tsp_bb_26_07_07 | 26 | 5ms | 13 | 5 | 6 | 0 | :white_check_mark: |
| tsp_bb_26_07_08 | 26 | 3ms | 13 | 5 | 6 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 4 -> 2 -> 3 -> 1 -> 0` cost=26
- **tsp_bb_26_07_02:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_06:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_07:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_08:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)

---

## 4. `examples/five-city.txt` (n=5)

**Concorde optimal cost:** 26  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 2 -> 3 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 26 | 2ms | 13 | 6 | 7 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 26 | 3ms | 17 | 4 | 8 | 0 | :white_check_mark: |
| tsp_bb_26_07_07 | 26 | 4ms | 13 | 5 | 6 | 0 | :white_check_mark: |
| tsp_bb_26_07_08 | 26 | 3ms | 13 | 5 | 6 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 4 -> 2 -> 3 -> 1 -> 0` cost=26
- **tsp_bb_26_07_02:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_06:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_07:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_08:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)

---

## 5. `examples/converted/five-node-euc.txt` (n=5)

**Concorde optimal cost:** 8  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 1 -> 2 -> 3 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 8 | 3ms | 15 | 7 | 8 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 8 | 3ms | 15 | 4 | 8 | 0 | :white_check_mark: |
| tsp_bb_26_07_07 | 8 | 5ms | 15 | 4 | 6 | 0 | :white_check_mark: |
| tsp_bb_26_07_08 | 8 | 4ms | 15 | 4 | 6 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=8
- **tsp_bb_26_07_02:** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=8 (=ref, same tour)
- **tsp_bb_26_07_06:** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=8 (=ref, same tour)
- **tsp_bb_26_07_07:** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=8 (=ref, same tour)
- **tsp_bb_26_07_08:** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=8 (=ref, same tour)

---

## 6. `examples/converted/five-node-explicit.txt` (n=5)

**Concorde optimal cost:** 26  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 2 -> 3 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 26 | 3ms | 13 | 6 | 7 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 26 | 3ms | 17 | 4 | 8 | 0 | :white_check_mark: |
| tsp_bb_26_07_07 | 26 | 4ms | 13 | 5 | 6 | 0 | :white_check_mark: |
| tsp_bb_26_07_08 | 26 | 3ms | 13 | 5 | 6 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 4 -> 2 -> 3 -> 1 -> 0` cost=26
- **tsp_bb_26_07_02:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_06:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_07:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_08:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)

---

## 7. `examples/random/sparse/rnd-01-sparse-n5.txt` (n=5)

**Concorde:** failed to find optimal solution  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 110 | 3ms | 5 | 2 | 3 | 0 | - |
| tsp_bb_26_07_06 | 110 | 3ms | 5 | 2 | 3 | 0 | - |
| tsp_bb_26_07_07 | 110 | 4ms | 5 | 2 | 2 | 0 | - |
| tsp_bb_26_07_08 | 110 | 3ms | 5 | 2 | 2 | 0 | - |

**Tours found:**
- **tsp_bb_26_07_02:** `3 -> 4 -> 2 -> 1 -> 0 -> 3` cost=110
- **tsp_bb_26_07_06:** `3 -> 4 -> 2 -> 1 -> 0 -> 3` cost=110
- **tsp_bb_26_07_07:** `3 -> 4 -> 2 -> 1 -> 0 -> 3` cost=110
- **tsp_bb_26_07_08:** `3 -> 4 -> 2 -> 1 -> 0 -> 3` cost=110

---

## 8. `examples/random/complete/rnd-02-complete-n5.txt` (n=5)

**Concorde optimal cost:** 136  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 2 -> 3 -> 4 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 136 | 3ms | 7 | 3 | 4 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 136 | 3ms | 1 | 1 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_07 | 136 | 3ms | 7 | 3 | 3 | 0 | :white_check_mark: |
| tsp_bb_26_07_08 | 136 | 3ms | 7 | 3 | 3 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 2 -> 3 -> 4 -> 1 -> 0` cost=136
- **tsp_bb_26_07_02:** `0 -> 2 -> 3 -> 4 -> 1 -> 0` cost=136 (=ref, same tour)
- **tsp_bb_26_07_06:** `0 -> 2 -> 3 -> 4 -> 1 -> 0` cost=136 (=ref, same tour)
- **tsp_bb_26_07_07:** `0 -> 2 -> 3 -> 4 -> 1 -> 0` cost=136 (=ref, same tour)
- **tsp_bb_26_07_08:** `0 -> 2 -> 3 -> 4 -> 1 -> 0` cost=136 (=ref, same tour)

---

## 9. `examples/random/complete/rnd-04-complete-n5.txt` (n=5)

**Concorde optimal cost:** 88  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 3 -> 2 -> 4 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 88 | 3ms | 7 | 3 | 4 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 88 | 3ms | 1 | 1 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_07 | 88 | 4ms | 7 | 3 | 3 | 0 | :white_check_mark: |
| tsp_bb_26_07_08 | 88 | 3ms | 7 | 3 | 3 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 3 -> 2 -> 4 -> 1 -> 0` cost=88
- **tsp_bb_26_07_02:** `0 -> 3 -> 2 -> 4 -> 1 -> 0` cost=88 (=ref, same tour)
- **tsp_bb_26_07_06:** `0 -> 3 -> 2 -> 4 -> 1 -> 0` cost=88 (=ref, same tour)
- **tsp_bb_26_07_07:** `0 -> 3 -> 2 -> 4 -> 1 -> 0` cost=88 (=ref, same tour)
- **tsp_bb_26_07_08:** `0 -> 3 -> 2 -> 4 -> 1 -> 0` cost=88 (=ref, same tour)

---

## 10. `examples/random/complete/rnd-06-complete-n5.txt` (n=5)

**Concorde optimal cost:** 51  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 1 -> 2 -> 3 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 51 | 3ms | 7 | 3 | 4 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 51 | 3ms | 1 | 1 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_07 | 51 | 4ms | 7 | 2 | 4 | 0 | :white_check_mark: |
| tsp_bb_26_07_08 | 51 | 3ms | 7 | 2 | 4 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=51
- **tsp_bb_26_07_02:** `0 -> 3 -> 2 -> 1 -> 4 -> 0` cost=51 (=ref, different tour)
- **tsp_bb_26_07_06:** `0 -> 3 -> 2 -> 1 -> 4 -> 0` cost=51 (=ref, different tour)
- **tsp_bb_26_07_07:** `0 -> 3 -> 2 -> 1 -> 4 -> 0` cost=51 (=ref, different tour)
- **tsp_bb_26_07_08:** `0 -> 3 -> 2 -> 1 -> 4 -> 0` cost=51 (=ref, different tour)

---

## 11. `examples/random/sparse/rnd-04-sparse-n6.txt` (n=6)

**Concorde:** failed to find optimal solution  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 115 | 3ms | 1 | 0 | 1 | 0 | - |
| tsp_bb_26_07_06 | 115 | 3ms | 1 | 0 | 1 | 0 | - |
| tsp_bb_26_07_07 | 115 | 4ms | 1 | 0 | 1 | 0 | - |
| tsp_bb_26_07_08 | 115 | 3ms | 1 | 0 | 1 | 0 | - |

**Tours found:**
- **tsp_bb_26_07_02:** `0 -> 4 -> 3 -> 5 -> 1 -> 2 -> 0` cost=115
- **tsp_bb_26_07_06:** `0 -> 4 -> 3 -> 5 -> 1 -> 2 -> 0` cost=115
- **tsp_bb_26_07_07:** `0 -> 4 -> 3 -> 5 -> 1 -> 2 -> 0` cost=115
- **tsp_bb_26_07_08:** `0 -> 4 -> 3 -> 5 -> 1 -> 2 -> 0` cost=115

---

## 12. `examples/random/sparse/rnd-05-sparse-n6.txt` (n=6)

**Concorde:** failed to find optimal solution  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 100 | 3ms | 30 | 22 | 8 | 15 | - |
| tsp_bb_26_07_06 | 100 | 3ms | 19 | 9 | 5 | 4 | - |
| tsp_bb_26_07_07 | 100 | 3ms | 30 | 14 | 8 | 13 | - |
| tsp_bb_26_07_08 | 100 | 3ms | 30 | 14 | 8 | 13 | - |

**Tours found:**
- **tsp_bb_26_07_02:** `0 -> 5 -> 2 -> 1 -> 4 -> 3 -> 0` cost=100
- **tsp_bb_26_07_06:** `0 -> 5 -> 2 -> 1 -> 4 -> 3 -> 0` cost=100
- **tsp_bb_26_07_07:** `0 -> 5 -> 2 -> 1 -> 4 -> 3 -> 0` cost=100
- **tsp_bb_26_07_08:** `0 -> 5 -> 2 -> 1 -> 4 -> 3 -> 0` cost=100

---

## 13. `examples/random/sparse/rnd-02-sparse-n7.txt` (n=7)

**Concorde:** failed to find optimal solution  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 196 | 4ms | 19 | 13 | 6 | 8 | - |
| tsp_bb_26_07_06 | 196 | 3ms | 10 | 3 | 1 | 3 | - |
| tsp_bb_26_07_07 | 196 | 4ms | 19 | 6 | 5 | 8 | - |
| tsp_bb_26_07_08 | 196 | 3ms | 19 | 6 | 5 | 8 | - |

**Tours found:**
- **tsp_bb_26_07_02:** `1 -> 5 -> 3 -> 0 -> 4 -> 2 -> 6 -> 1` cost=196
- **tsp_bb_26_07_06:** `1 -> 5 -> 3 -> 0 -> 4 -> 2 -> 6 -> 1` cost=196
- **tsp_bb_26_07_07:** `1 -> 5 -> 3 -> 0 -> 4 -> 2 -> 6 -> 1` cost=196
- **tsp_bb_26_07_08:** `1 -> 5 -> 3 -> 0 -> 4 -> 2 -> 6 -> 1` cost=196

---

## 14. `examples/random/sparse/rnd-03-sparse-n7.txt` (n=7)

**Concorde:** failed to find optimal solution  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 86 | 4ms | 7 | 4 | 3 | 2 | - |
| tsp_bb_26_07_06 | 86 | 4ms | 4 | 2 | 1 | 1 | - |
| tsp_bb_26_07_07 | 86 | 3ms | 7 | 3 | 2 | 2 | - |
| tsp_bb_26_07_08 | 86 | 3ms | 7 | 3 | 2 | 2 | - |

**Tours found:**
- **tsp_bb_26_07_02:** `1 -> 6 -> 3 -> 4 -> 5 -> 2 -> 0 -> 1` cost=86
- **tsp_bb_26_07_06:** `1 -> 6 -> 3 -> 4 -> 5 -> 2 -> 0 -> 1` cost=86
- **tsp_bb_26_07_07:** `1 -> 6 -> 3 -> 4 -> 5 -> 2 -> 0 -> 1` cost=86
- **tsp_bb_26_07_08:** `1 -> 6 -> 3 -> 4 -> 5 -> 2 -> 0 -> 1` cost=86

---

## 15. `examples/random/complete/rnd-05-complete-n7.txt` (n=7)

**Concorde optimal cost:** 74  
**Concorde time:** 3ms  

**Concorde (reference) tour:** `0 -> 3 -> 6 -> 5 -> 1 -> 4 -> 2 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 74 | 3ms | 23 | 11 | 12 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 74 | 3ms | 7 | 4 | 2 | 0 | :white_check_mark: |
| tsp_bb_26_07_07 | 74 | 3ms | 23 | 8 | 9 | 0 | :white_check_mark: |
| tsp_bb_26_07_08 | 74 | 3ms | 23 | 8 | 9 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 3 -> 6 -> 5 -> 1 -> 4 -> 2 -> 0` cost=74
- **tsp_bb_26_07_02:** `0 -> 2 -> 4 -> 1 -> 5 -> 6 -> 3 -> 0` cost=74 (=ref, different tour)
- **tsp_bb_26_07_06:** `0 -> 2 -> 4 -> 1 -> 5 -> 6 -> 3 -> 0` cost=74 (=ref, different tour)
- **tsp_bb_26_07_07:** `0 -> 2 -> 4 -> 1 -> 5 -> 6 -> 3 -> 0` cost=74 (=ref, different tour)
- **tsp_bb_26_07_08:** `0 -> 2 -> 4 -> 1 -> 5 -> 6 -> 3 -> 0` cost=74 (=ref, different tour)

---

## 16. `examples/random/sparse/rnd-06-sparse-n7.txt` (n=7)

**Concorde:** failed to find optimal solution  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 199 | 4ms | 28 | 21 | 7 | 15 | - |
| tsp_bb_26_07_06 | 199 | 3ms | 16 | 7 | 2 | 5 | - |
| tsp_bb_26_07_07 | 199 | 2ms | 28 | 14 | 6 | 15 | - |
| tsp_bb_26_07_08 | 199 | 3ms | 28 | 14 | 6 | 15 | - |

**Tours found:**
- **tsp_bb_26_07_02:** `4 -> 1 -> 5 -> 3 -> 2 -> 6 -> 0 -> 4` cost=199
- **tsp_bb_26_07_06:** `4 -> 1 -> 5 -> 3 -> 2 -> 6 -> 0 -> 4` cost=199
- **tsp_bb_26_07_07:** `4 -> 1 -> 5 -> 3 -> 2 -> 6 -> 0 -> 4` cost=199
- **tsp_bb_26_07_08:** `4 -> 1 -> 5 -> 3 -> 2 -> 6 -> 0 -> 4` cost=199

---

## 17. `examples/random/complete/rnd-08-complete-n7.txt` (n=7)

**Concorde optimal cost:** 73  
**Concorde time:** 3ms  

**Concorde (reference) tour:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 73 | 4ms | 21 | 10 | 11 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 73 | 3ms | 7 | 3 | 2 | 0 | :white_check_mark: |
| tsp_bb_26_07_07 | 73 | 3ms | 21 | 6 | 11 | 0 | :white_check_mark: |
| tsp_bb_26_07_08 | 73 | 3ms | 21 | 6 | 11 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0` cost=73
- **tsp_bb_26_07_02:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0` cost=73 (=ref, same tour)
- **tsp_bb_26_07_06:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0` cost=73 (=ref, same tour)
- **tsp_bb_26_07_07:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0` cost=73 (=ref, same tour)
- **tsp_bb_26_07_08:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0` cost=73 (=ref, same tour)

---

## 18. `examples/random/complete/rnd-10-complete-n7.txt` (n=7)

**Concorde optimal cost:** 49  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 6 -> 4 -> 5 -> 3 -> 2 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 49 | 3ms | 13 | 6 | 7 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 49 | 4ms | 5 | 3 | 1 | 0 | :white_check_mark: |
| tsp_bb_26_07_07 | 49 | 3ms | 13 | 4 | 7 | 0 | :white_check_mark: |
| tsp_bb_26_07_08 | 49 | 4ms | 13 | 4 | 7 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 6 -> 4 -> 5 -> 3 -> 2 -> 1 -> 0` cost=49
- **tsp_bb_26_07_02:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 0` cost=49 (=ref, different tour)
- **tsp_bb_26_07_06:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 0` cost=49 (=ref, different tour)
- **tsp_bb_26_07_07:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 0` cost=49 (=ref, different tour)
- **tsp_bb_26_07_08:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 0` cost=49 (=ref, different tour)

---

## 19. `examples/random/complete/rnd-07-complete-n8.txt` (n=8)

**Concorde optimal cost:** 64  
**Concorde time:** 3ms  

**Concorde (reference) tour:** `0 -> 7 -> 4 -> 3 -> 2 -> 1 -> 6 -> 5 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 64 | 4ms | 23 | 11 | 12 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 64 | 4ms | 7 | 3 | 2 | 0 | :white_check_mark: |
| tsp_bb_26_07_07 | 64 | 3ms | 23 | 8 | 11 | 0 | :white_check_mark: |
| tsp_bb_26_07_08 | 64 | 4ms | 23 | 8 | 11 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 7 -> 4 -> 3 -> 2 -> 1 -> 6 -> 5 -> 0` cost=64
- **tsp_bb_26_07_02:** `2 -> 3 -> 4 -> 7 -> 0 -> 5 -> 6 -> 1 -> 2` cost=64 (=ref, different tour)
- **tsp_bb_26_07_06:** `2 -> 3 -> 4 -> 7 -> 0 -> 5 -> 6 -> 1 -> 2` cost=64 (=ref, different tour)
- **tsp_bb_26_07_07:** `2 -> 3 -> 4 -> 7 -> 0 -> 5 -> 6 -> 1 -> 2` cost=64 (=ref, different tour)
- **tsp_bb_26_07_08:** `2 -> 3 -> 4 -> 7 -> 0 -> 5 -> 6 -> 1 -> 2` cost=64 (=ref, different tour)

---

## 20. `examples/random/complete/rnd-09-complete-n8.txt` (n=8)

**Concorde optimal cost:** 93  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 5 -> 4 -> 6 -> 1 -> 3 -> 2 -> 7 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 93 | 4ms | 77 | 38 | 39 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 93 | 4ms | 33 | 14 | 7 | 0 | :white_check_mark: |
| tsp_bb_26_07_07 | 93 | 4ms | 77 | 25 | 37 | 0 | :white_check_mark: |
| tsp_bb_26_07_08 | 93 | 4ms | 77 | 25 | 37 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 5 -> 4 -> 6 -> 1 -> 3 -> 2 -> 7 -> 0` cost=93
- **tsp_bb_26_07_02:** `1 -> 3 -> 2 -> 7 -> 0 -> 5 -> 4 -> 6 -> 1` cost=93 (=ref, different tour)
- **tsp_bb_26_07_06:** `1 -> 3 -> 2 -> 7 -> 0 -> 5 -> 4 -> 6 -> 1` cost=93 (=ref, different tour)
- **tsp_bb_26_07_07:** `1 -> 3 -> 2 -> 7 -> 0 -> 5 -> 4 -> 6 -> 1` cost=93 (=ref, different tour)
- **tsp_bb_26_07_08:** `1 -> 3 -> 2 -> 7 -> 0 -> 5 -> 4 -> 6 -> 1` cost=93 (=ref, different tour)

---

## 21. `data/classic/tsplib/burma14.tsp` (n=14)

**Concorde optimal cost:** 3323  
**Concorde time:** 20ms  

**Concorde (reference) tour:** `0 -> 1 -> 13 -> 2 -> 3 -> 4 -> 5 -> 11 -> 6 -> 12 -> 7 -> 10 -> 8 -> 9 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 3323 | 38ms | 18111 | 9162 | 8949 | 214 | :white_check_mark: |
| tsp_bb_26_07_06 | 3323 | 44ms | 14252 | 5043 | 3706 | 113 | :white_check_mark: |
| tsp_bb_26_07_07 | 3323 | 38ms | 18111 | 4582 | 8945 | 214 | :white_check_mark: |
| tsp_bb_26_07_08 | 3323 | 39ms | 18111 | 4582 | 8945 | 214 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 1 -> 13 -> 2 -> 3 -> 4 -> 5 -> 11 -> 6 -> 12 -> 7 -> 10 -> 8 -> 9 -> 0` cost=3323
- **tsp_bb_26_07_02:** `1 -> 0 -> 9 -> 8 -> 10 -> 7 -> 12 -> 6 -> 11 -> 5 -> 4 -> 3 -> 2 -> 13 -> 1` cost=3323 (=ref, different tour)
- **tsp_bb_26_07_06:** `1 -> 0 -> 9 -> 8 -> 10 -> 7 -> 12 -> 6 -> 11 -> 5 -> 4 -> 3 -> 2 -> 13 -> 1` cost=3323 (=ref, different tour)
- **tsp_bb_26_07_07:** `1 -> 0 -> 9 -> 8 -> 10 -> 7 -> 12 -> 6 -> 11 -> 5 -> 4 -> 3 -> 2 -> 13 -> 1` cost=3323 (=ref, different tour)
- **tsp_bb_26_07_08:** `1 -> 0 -> 9 -> 8 -> 10 -> 7 -> 12 -> 6 -> 11 -> 5 -> 4 -> 3 -> 2 -> 13 -> 1` cost=3323 (=ref, different tour)

---

## 22. `data/classic/tsplib/ulysses16.tsp` (n=16)

**Concorde optimal cost:** 6859  
**Concorde time:** 26ms  

**Concorde (reference) tour:** `0 -> 13 -> 12 -> 11 -> 6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 6859 | 16.1s | 15447419 | 7797474 | 7649945 | 147530 | :white_check_mark: |
| tsp_bb_26_07_06 | 6859 | 14.7s | 7661012 | 2637820 | 2090400 | 89559 | :white_check_mark: |
| tsp_bb_26_07_07 | 6859 | 19.7s | 15447419 | 3883908 | 7609491 | 147530 | :white_check_mark: |
| tsp_bb_26_07_08 | 6859 | 18.9s | 15447419 | 3883908 | 7609491 | 147530 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 13 -> 12 -> 11 -> 6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0` cost=6859
- **tsp_bb_26_07_02:** `6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0 -> 13 -> 12 -> 11 -> 6` cost=6859 (=ref, different tour)
- **tsp_bb_26_07_06:** `6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0 -> 13 -> 12 -> 11 -> 6` cost=6859 (=ref, different tour)
- **tsp_bb_26_07_07:** `6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0 -> 13 -> 12 -> 11 -> 6` cost=6859 (=ref, different tour)
- **tsp_bb_26_07_08:** `6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0 -> 13 -> 12 -> 11 -> 6` cost=6859 (=ref, different tour)

---

## 23. `data/classic/tsplib/gr17.tsp` (n=17)

**Concorde optimal cost:** 2085  
**Concorde time:** 26ms  

**Concorde (reference) tour:** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 2085 | 2.2s | 1817688 | 920131 | 897557 | 22575 | :white_check_mark: |
| tsp_bb_26_07_06 | 2085 | 2.1s | 959753 | 354212 | 257007 | 10826 | :white_check_mark: |
| tsp_bb_26_07_07 | 2085 | 2.7s | 1817688 | 463820 | 897458 | 22575 | :white_check_mark: |
| tsp_bb_26_07_08 | 2085 | 2.5s | 1817688 | 463820 | 897458 | 22575 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0` cost=2085
- **tsp_bb_26_07_02:** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0` cost=2085 (=ref, same tour)
- **tsp_bb_26_07_06:** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0` cost=2085 (=ref, same tour)
- **tsp_bb_26_07_07:** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0` cost=2085 (=ref, same tour)
- **tsp_bb_26_07_08:** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0` cost=2085 (=ref, same tour)

---

## 24. `data/classic/tsplib/gr21.tsp` (n=21)

**Concorde optimal cost:** 2707  
**Concorde time:** 13ms  

**Concorde (reference) tour:** `0 -> 6 -> 7 -> 5 -> 15 -> 4 -> 8 -> 2 -> 1 -> 20 -> 14 -> 13 -> 12 -> 17 -> 9 -> 16 -> 18 -> 19 -> 10 -> 3 -> 11 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 2707 | 78ms | 34213 | 17314 | 16899 | 416 | :white_check_mark: |
| tsp_bb_26_07_06 | 2707 | 63ms | 13034 | 5506 | 2841 | 187 | :white_check_mark: |
| tsp_bb_26_07_07 | 2707 | 95ms | 34213 | 9911 | 16899 | 416 | :white_check_mark: |
| tsp_bb_26_07_08 | 2707 | 89ms | 34213 | 9911 | 16899 | 416 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 6 -> 7 -> 5 -> 15 -> 4 -> 8 -> 2 -> 1 -> 20 -> 14 -> 13 -> 12 -> 17 -> 9 -> 16 -> 18 -> 19 -> 10 -> 3 -> 11 -> 0` cost=2707
- **tsp_bb_26_07_02:** `2 -> 8 -> 4 -> 15 -> 5 -> 7 -> 6 -> 0 -> 11 -> 3 -> 10 -> 19 -> 18 -> 16 -> 9 -> 17 -> 12 -> 13 -> 14 -> 20 -> 1 -> 2` cost=2707 (=ref, different tour)
- **tsp_bb_26_07_06:** `2 -> 8 -> 4 -> 15 -> 5 -> 7 -> 6 -> 0 -> 11 -> 3 -> 10 -> 19 -> 18 -> 16 -> 9 -> 17 -> 12 -> 13 -> 14 -> 20 -> 1 -> 2` cost=2707 (=ref, different tour)
- **tsp_bb_26_07_07:** `2 -> 8 -> 4 -> 15 -> 5 -> 7 -> 6 -> 0 -> 11 -> 3 -> 10 -> 19 -> 18 -> 16 -> 9 -> 17 -> 12 -> 13 -> 14 -> 20 -> 1 -> 2` cost=2707 (=ref, different tour)
- **tsp_bb_26_07_08:** `2 -> 8 -> 4 -> 15 -> 5 -> 7 -> 6 -> 0 -> 11 -> 3 -> 10 -> 19 -> 18 -> 16 -> 9 -> 17 -> 12 -> 13 -> 14 -> 20 -> 1 -> 2` cost=2707 (=ref, different tour)

---

## 25. `data/classic/tsplib/ulysses22.tsp` (n=22)

**Concorde optimal cost:** 7013  
**Concorde time:** 67ms  

**Concorde (reference) tour:** `0 -> 13 -> 12 -> 11 -> 6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 18 -> 19 -> 20 -> 15 -> 2 -> 1 -> 16 -> 21 -> 3 -> 17 -> 7 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_06 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_07 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_08 | TIMEOUT | 30.0m | - | - | - | - | - |

**Tours found:**
- **Concorde (reference):** `0 -> 13 -> 12 -> 11 -> 6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 18 -> 19 -> 20 -> 15 -> 2 -> 1 -> 16 -> 21 -> 3 -> 17 -> 7 -> 0` cost=7013

---

## 26. `data/classic/tsplib/gr24.tsp` (n=24)

**Concorde optimal cost:** 1272  
**Concorde time:** 22ms  

**Concorde (reference) tour:** `0 -> 11 -> 3 -> 22 -> 8 -> 12 -> 13 -> 19 -> 1 -> 14 -> 18 -> 21 -> 17 -> 16 -> 9 -> 4 -> 20 -> 7 -> 23 -> 5 -> 6 -> 2 -> 10 -> 15 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 1272 | 1.2s | 571607 | 287023 | 284584 | 2440 | :white_check_mark: |
| tsp_bb_26_07_06 | 1272 | 810ms | 231105 | 90932 | 61408 | 90 | :white_check_mark: |
| tsp_bb_26_07_07 | 1272 | 1.4s | 571607 | 158220 | 284581 | 2440 | :white_check_mark: |
| tsp_bb_26_07_08 | 1272 | 1.3s | 571607 | 158220 | 284581 | 2440 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 11 -> 3 -> 22 -> 8 -> 12 -> 13 -> 19 -> 1 -> 14 -> 18 -> 21 -> 17 -> 16 -> 9 -> 4 -> 20 -> 7 -> 23 -> 5 -> 6 -> 2 -> 10 -> 15 -> 0` cost=1272
- **tsp_bb_26_07_02:** `3 -> 11 -> 0 -> 15 -> 10 -> 2 -> 6 -> 5 -> 23 -> 7 -> 20 -> 4 -> 9 -> 16 -> 17 -> 21 -> 18 -> 14 -> 1 -> 19 -> 13 -> 12 -> 8 -> 22 -> 3` cost=1272 (=ref, different tour)
- **tsp_bb_26_07_06:** `3 -> 11 -> 0 -> 15 -> 10 -> 2 -> 6 -> 5 -> 23 -> 7 -> 20 -> 4 -> 9 -> 16 -> 17 -> 21 -> 18 -> 14 -> 1 -> 19 -> 13 -> 12 -> 8 -> 22 -> 3` cost=1272 (=ref, different tour)
- **tsp_bb_26_07_07:** `3 -> 11 -> 0 -> 15 -> 10 -> 2 -> 6 -> 5 -> 23 -> 7 -> 20 -> 4 -> 9 -> 16 -> 17 -> 21 -> 18 -> 14 -> 1 -> 19 -> 13 -> 12 -> 8 -> 22 -> 3` cost=1272 (=ref, different tour)
- **tsp_bb_26_07_08:** `3 -> 11 -> 0 -> 15 -> 10 -> 2 -> 6 -> 5 -> 23 -> 7 -> 20 -> 4 -> 9 -> 16 -> 17 -> 21 -> 18 -> 14 -> 1 -> 19 -> 13 -> 12 -> 8 -> 22 -> 3` cost=1272 (=ref, different tour)

---

## 27. `data/classic/tsplib/fri26.tsp` (n=26)

**Concorde optimal cost:** 937  
**Concorde time:** 28ms  

**Concorde (reference) tour:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 7 -> 8 -> 9 -> 13 -> 14 -> 12 -> 11 -> 10 -> 15 -> 18 -> 19 -> 17 -> 16 -> 20 -> 21 -> 25 -> 22 -> 23 -> 24 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 937 | 1.9s | 984246 | 492159 | 492087 | 73 | :white_check_mark: |
| tsp_bb_26_07_06 | 937 | 2.1s | 575467 | 228573 | 148803 | 0 | :white_check_mark: |
| tsp_bb_26_07_07 | 937 | 2.4s | 984246 | 264044 | 492087 | 73 | :white_check_mark: |
| tsp_bb_26_07_08 | 937 | 2.2s | 984246 | 264044 | 492087 | 73 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 7 -> 8 -> 9 -> 13 -> 14 -> 12 -> 11 -> 10 -> 15 -> 18 -> 19 -> 17 -> 16 -> 20 -> 21 -> 25 -> 22 -> 23 -> 24 -> 0` cost=937
- **tsp_bb_26_07_02:** `16 -> 20 -> 21 -> 25 -> 22 -> 23 -> 24 -> 0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 7 -> 8 -> 9 -> 13 -> 14 -> 11 -> 12 -> 10 -> 15 -> 18 -> 19 -> 17 -> 16` cost=937 (=ref, different tour)
- **tsp_bb_26_07_06:** `16 -> 20 -> 21 -> 25 -> 22 -> 23 -> 24 -> 0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 7 -> 8 -> 9 -> 13 -> 14 -> 11 -> 12 -> 10 -> 15 -> 18 -> 19 -> 17 -> 16` cost=937 (=ref, different tour)
- **tsp_bb_26_07_07:** `16 -> 20 -> 21 -> 25 -> 22 -> 23 -> 24 -> 0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 7 -> 8 -> 9 -> 13 -> 14 -> 11 -> 12 -> 10 -> 15 -> 18 -> 19 -> 17 -> 16` cost=937 (=ref, different tour)
- **tsp_bb_26_07_08:** `16 -> 20 -> 21 -> 25 -> 22 -> 23 -> 24 -> 0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 7 -> 8 -> 9 -> 13 -> 14 -> 11 -> 12 -> 10 -> 15 -> 18 -> 19 -> 17 -> 16` cost=937 (=ref, different tour)

---

## 28. `data/classic/tsplib/bayg29.tsp` (n=29)

**Concorde optimal cost:** 1610  
**Concorde time:** 24ms  

**Concorde (reference) tour:** `0 -> 27 -> 5 -> 11 -> 8 -> 25 -> 2 -> 28 -> 4 -> 20 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 13 -> 16 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 7 -> 26 -> 15 -> 12 -> 23 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 1610 | 27.8s | 12732970 | 6376947 | 6356023 | 20925 | :white_check_mark: |
| tsp_bb_26_07_06 | 1610 | 20.6s | 5254425 | 2147600 | 1369125 | 6894 | :white_check_mark: |
| tsp_bb_26_07_07 | 1610 | 33.5s | 12732970 | 3605784 | 6355868 | 20925 | :white_check_mark: |
| tsp_bb_26_07_08 | 1610 | 30.9s | 12732970 | 3605784 | 6355868 | 20925 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 27 -> 5 -> 11 -> 8 -> 25 -> 2 -> 28 -> 4 -> 20 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 13 -> 16 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 7 -> 26 -> 15 -> 12 -> 23 -> 0` cost=1610
- **tsp_bb_26_07_02:** `0 -> 27 -> 5 -> 11 -> 8 -> 25 -> 2 -> 28 -> 4 -> 20 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 13 -> 16 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 7 -> 26 -> 15 -> 12 -> 23 -> 0` cost=1610 (=ref, same tour)
- **tsp_bb_26_07_06:** `0 -> 27 -> 5 -> 11 -> 8 -> 25 -> 2 -> 28 -> 4 -> 20 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 13 -> 16 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 7 -> 26 -> 15 -> 12 -> 23 -> 0` cost=1610 (=ref, same tour)
- **tsp_bb_26_07_07:** `0 -> 27 -> 5 -> 11 -> 8 -> 25 -> 2 -> 28 -> 4 -> 20 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 13 -> 16 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 7 -> 26 -> 15 -> 12 -> 23 -> 0` cost=1610 (=ref, same tour)
- **tsp_bb_26_07_08:** `0 -> 27 -> 5 -> 11 -> 8 -> 25 -> 2 -> 28 -> 4 -> 20 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 13 -> 16 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 7 -> 26 -> 15 -> 12 -> 23 -> 0` cost=1610 (=ref, same tour)

---

## 29. `data/classic/tsplib/bays29.tsp` (n=29)

**Concorde optimal cost:** 2020  
**Concorde time:** 33ms  

**Concorde (reference) tour:** `0 -> 27 -> 5 -> 11 -> 8 -> 4 -> 25 -> 28 -> 2 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 16 -> 13 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 26 -> 7 -> 23 -> 15 -> 12 -> 20 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 2020 | 5.7m | 162609801 | 81440628 | 81169173 | 271456 | :white_check_mark: |
| tsp_bb_26_07_06 | 2020 | 4.3m | 70575486 | 26997572 | 18956858 | 64265 | :white_check_mark: |
| tsp_bb_26_07_07 | 2020 | 6.9m | 162609801 | 44122386 | 81168931 | 271456 | :white_check_mark: |
| tsp_bb_26_07_08 | 2020 | 6.3m | 162609801 | 44122386 | 81168931 | 271456 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 27 -> 5 -> 11 -> 8 -> 4 -> 25 -> 28 -> 2 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 16 -> 13 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 26 -> 7 -> 23 -> 15 -> 12 -> 20 -> 0` cost=2020
- **tsp_bb_26_07_02:** `7 -> 26 -> 22 -> 6 -> 24 -> 18 -> 10 -> 21 -> 13 -> 16 -> 17 -> 14 -> 3 -> 9 -> 19 -> 1 -> 2 -> 28 -> 25 -> 4 -> 8 -> 11 -> 5 -> 27 -> 0 -> 20 -> 12 -> 15 -> 23 -> 7` cost=2020 (=ref, different tour)
- **tsp_bb_26_07_06:** `7 -> 26 -> 22 -> 6 -> 24 -> 18 -> 10 -> 21 -> 13 -> 16 -> 17 -> 14 -> 3 -> 9 -> 19 -> 1 -> 2 -> 28 -> 25 -> 4 -> 8 -> 11 -> 5 -> 27 -> 0 -> 20 -> 12 -> 15 -> 23 -> 7` cost=2020 (=ref, different tour)
- **tsp_bb_26_07_07:** `7 -> 26 -> 22 -> 6 -> 24 -> 18 -> 10 -> 21 -> 13 -> 16 -> 17 -> 14 -> 3 -> 9 -> 19 -> 1 -> 2 -> 28 -> 25 -> 4 -> 8 -> 11 -> 5 -> 27 -> 0 -> 20 -> 12 -> 15 -> 23 -> 7` cost=2020 (=ref, different tour)
- **tsp_bb_26_07_08:** `7 -> 26 -> 22 -> 6 -> 24 -> 18 -> 10 -> 21 -> 13 -> 16 -> 17 -> 14 -> 3 -> 9 -> 19 -> 1 -> 2 -> 28 -> 25 -> 4 -> 8 -> 11 -> 5 -> 27 -> 0 -> 20 -> 12 -> 15 -> 23 -> 7` cost=2020 (=ref, different tour)

---

## 30. `data/classic/national/wi29.tsp` (n=29)

**Concorde optimal cost:** 27603  
**Concorde time:** 38ms  

**Concorde (reference) tour:** `0 -> 4 -> 7 -> 3 -> 2 -> 6 -> 8 -> 12 -> 13 -> 15 -> 23 -> 26 -> 24 -> 19 -> 25 -> 27 -> 28 -> 22 -> 21 -> 20 -> 16 -> 17 -> 18 -> 14 -> 11 -> 10 -> 9 -> 5 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 27603 | 201ms | 77709 | 38866 | 38843 | 24 | :white_check_mark: |
| tsp_bb_26_07_06 | 27603 | 666ms | 161579 | 67141 | 40781 | 4 | :white_check_mark: |
| tsp_bb_26_07_07 | 27603 | 242ms | 77709 | 22423 | 38843 | 24 | :white_check_mark: |
| tsp_bb_26_07_08 | 27603 | 227ms | 77709 | 22423 | 38843 | 24 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 4 -> 7 -> 3 -> 2 -> 6 -> 8 -> 12 -> 13 -> 15 -> 23 -> 26 -> 24 -> 19 -> 25 -> 27 -> 28 -> 22 -> 21 -> 20 -> 16 -> 17 -> 18 -> 14 -> 11 -> 10 -> 9 -> 5 -> 1 -> 0` cost=27603
- **tsp_bb_26_07_02:** `21 -> 22 -> 28 -> 27 -> 25 -> 19 -> 24 -> 26 -> 23 -> 15 -> 13 -> 12 -> 8 -> 6 -> 2 -> 3 -> 7 -> 4 -> 0 -> 1 -> 5 -> 9 -> 10 -> 11 -> 14 -> 18 -> 17 -> 16 -> 20 -> 21` cost=27603 (=ref, different tour)
- **tsp_bb_26_07_06:** `21 -> 22 -> 28 -> 27 -> 25 -> 19 -> 24 -> 26 -> 23 -> 15 -> 13 -> 12 -> 8 -> 6 -> 2 -> 3 -> 7 -> 4 -> 0 -> 1 -> 5 -> 9 -> 10 -> 11 -> 14 -> 18 -> 17 -> 16 -> 20 -> 21` cost=27603 (=ref, different tour)
- **tsp_bb_26_07_07:** `21 -> 22 -> 28 -> 27 -> 25 -> 19 -> 24 -> 26 -> 23 -> 15 -> 13 -> 12 -> 8 -> 6 -> 2 -> 3 -> 7 -> 4 -> 0 -> 1 -> 5 -> 9 -> 10 -> 11 -> 14 -> 18 -> 17 -> 16 -> 20 -> 21` cost=27603 (=ref, different tour)
- **tsp_bb_26_07_08:** `21 -> 22 -> 28 -> 27 -> 25 -> 19 -> 24 -> 26 -> 23 -> 15 -> 13 -> 12 -> 8 -> 6 -> 2 -> 3 -> 7 -> 4 -> 0 -> 1 -> 5 -> 9 -> 10 -> 11 -> 14 -> 18 -> 17 -> 16 -> 20 -> 21` cost=27603 (=ref, different tour)

---

## 31. `data/classic/national/dj38.tsp` (n=38)

**Concorde optimal cost:** 6656  
**Concorde time:** 37ms  

**Concorde (reference) tour:** `0 -> 9 -> 13 -> 20 -> 28 -> 29 -> 31 -> 34 -> 36 -> 37 -> 32 -> 33 -> 35 -> 30 -> 26 -> 27 -> 23 -> 21 -> 24 -> 25 -> 22 -> 19 -> 14 -> 12 -> 15 -> 16 -> 17 -> 18 -> 10 -> 11 -> 8 -> 7 -> 6 -> 5 -> 4 -> 2 -> 3 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 6656 | 115ms | 25733 | 12866 | 12867 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 6656 | 119ms | 14545 | 6751 | 3358 | 0 | :white_check_mark: |
| tsp_bb_26_07_07 | 6656 | 128ms | 25733 | 7990 | 12867 | 0 | :white_check_mark: |
| tsp_bb_26_07_08 | 6656 | 112ms | 25733 | 7990 | 12867 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 9 -> 13 -> 20 -> 28 -> 29 -> 31 -> 34 -> 36 -> 37 -> 32 -> 33 -> 35 -> 30 -> 26 -> 27 -> 23 -> 21 -> 24 -> 25 -> 22 -> 19 -> 14 -> 12 -> 15 -> 16 -> 17 -> 18 -> 10 -> 11 -> 8 -> 7 -> 6 -> 5 -> 4 -> 2 -> 3 -> 1 -> 0` cost=6656
- **tsp_bb_26_07_02:** `1 -> 0 -> 9 -> 13 -> 20 -> 28 -> 29 -> 31 -> 34 -> 36 -> 37 -> 32 -> 33 -> 35 -> 30 -> 26 -> 27 -> 23 -> 21 -> 24 -> 25 -> 22 -> 19 -> 14 -> 12 -> 15 -> 16 -> 17 -> 18 -> 10 -> 11 -> 8 -> 7 -> 6 -> 5 -> 4 -> 2 -> 3 -> 1` cost=6656 (=ref, different tour)
- **tsp_bb_26_07_06:** `1 -> 0 -> 9 -> 13 -> 20 -> 28 -> 29 -> 31 -> 34 -> 36 -> 37 -> 32 -> 33 -> 35 -> 30 -> 26 -> 27 -> 23 -> 21 -> 24 -> 25 -> 22 -> 19 -> 14 -> 12 -> 15 -> 16 -> 17 -> 18 -> 10 -> 11 -> 8 -> 7 -> 6 -> 5 -> 4 -> 2 -> 3 -> 1` cost=6656 (=ref, different tour)
- **tsp_bb_26_07_07:** `1 -> 0 -> 9 -> 13 -> 20 -> 28 -> 29 -> 31 -> 34 -> 36 -> 37 -> 32 -> 33 -> 35 -> 30 -> 26 -> 27 -> 23 -> 21 -> 24 -> 25 -> 22 -> 19 -> 14 -> 12 -> 15 -> 16 -> 17 -> 18 -> 10 -> 11 -> 8 -> 7 -> 6 -> 5 -> 4 -> 2 -> 3 -> 1` cost=6656 (=ref, different tour)
- **tsp_bb_26_07_08:** `1 -> 0 -> 9 -> 13 -> 20 -> 28 -> 29 -> 31 -> 34 -> 36 -> 37 -> 32 -> 33 -> 35 -> 30 -> 26 -> 27 -> 23 -> 21 -> 24 -> 25 -> 22 -> 19 -> 14 -> 12 -> 15 -> 16 -> 17 -> 18 -> 10 -> 11 -> 8 -> 7 -> 6 -> 5 -> 4 -> 2 -> 3 -> 1` cost=6656 (=ref, different tour)

---

## 32. `data/classic/tsplib/dantzig42.tsp` (n=42)

**Concorde optimal cost:** 699  
**Concorde time:** 40ms  

**Concorde (reference) tour:** `0 -> 41 -> 40 -> 39 -> 38 -> 37 -> 36 -> 35 -> 34 -> 33 -> 32 -> 31 -> 30 -> 29 -> 28 -> 27 -> 26 -> 25 -> 24 -> 23 -> 22 -> 21 -> 20 -> 19 -> 18 -> 17 -> 16 -> 15 -> 14 -> 13 -> 12 -> 11 -> 10 -> 9 -> 8 -> 7 -> 6 -> 5 -> 4 -> 3 -> 2 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_06 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_07 | TIMEOUT | 30.0m | - | - | - | - | - |
| tsp_bb_26_07_08 | TIMEOUT | 30.0m | - | - | - | - | - |

**Tours found:**
- **Concorde (reference):** `0 -> 41 -> 40 -> 39 -> 38 -> 37 -> 36 -> 35 -> 34 -> 33 -> 32 -> 31 -> 30 -> 29 -> 28 -> 27 -> 26 -> 25 -> 24 -> 23 -> 22 -> 21 -> 20 -> 19 -> 18 -> 17 -> 16 -> 15 -> 14 -> 13 -> 12 -> 11 -> 10 -> 9 -> 8 -> 7 -> 6 -> 5 -> 4 -> 3 -> 2 -> 1 -> 0` cost=699

---
