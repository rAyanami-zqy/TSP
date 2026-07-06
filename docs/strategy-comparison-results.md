# TSP Algorithm Comparison

## Algorithms Compared

| Algorithm | Description |
|---|---|
| **Concorde** | State-of-the-art Concorde TSP solver (exact, with QSopt LP) — **reference** |
| **tsp_bb_26_07_02** | Branch & Bound solver — smart branching (legacy) |
| **tsp_bb_26_07_06** | Branch & Bound solver — BP (Branch Partitioning) |

**Instances:** 39 from `examples` (n <= 49), `data/classic/tsplib` (n <= 59), `data/classic/national` (n <= 59)  
**Timeout:** 1800s (0h) per method per instance  
**Reference:** Concorde exact solver  
**Solver versions:** `tsp_bb_26_07_02`, `tsp_bb_26_07_06`  

---

## 1. `examples/random/complete/rnd-01-complete-n4.txt` (n=4)

**Concorde optimal cost:** 45  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 3 -> 2 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 45 | 3ms | 5 | 2 | 3 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 45 | 3ms | 3 | 2 | 2 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 3 -> 2 -> 1 -> 0` cost=45
- **tsp_bb_26_07_02:** `0 -> 3 -> 2 -> 1 -> 0` cost=45 (=ref, same tour)
- **tsp_bb_26_07_06:** `0 -> 3 -> 2 -> 1 -> 0` cost=45 (=ref, same tour)

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

**Tours found:**
- **Concorde (reference):** `0 -> 1 -> 3 -> 2 -> 0` cost=102
- **tsp_bb_26_07_02:** `0 -> 1 -> 3 -> 2 -> 0` cost=102 (=ref, same tour)
- **tsp_bb_26_07_06:** `0 -> 1 -> 3 -> 2 -> 0` cost=102 (=ref, same tour)

---

## 3. `examples/five-city.tsp` (n=5)

**Concorde optimal cost:** 26  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 2 -> 3 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 26 | 3ms | 13 | 6 | 7 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 26 | 4ms | 1 | 1 | 1 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 4 -> 2 -> 3 -> 1 -> 0` cost=26
- **tsp_bb_26_07_02:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_06:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)

---

## 4. `examples/five-city.txt` (n=5)

**Concorde optimal cost:** 26  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 2 -> 3 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 26 | 2ms | 13 | 6 | 7 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 26 | 3ms | 1 | 1 | 1 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 4 -> 2 -> 3 -> 1 -> 0` cost=26
- **tsp_bb_26_07_02:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_06:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)

---

## 5. `examples/converted/five-node-euc.txt` (n=5)

**Concorde optimal cost:** 8  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 1 -> 2 -> 3 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 8 | 3ms | 15 | 7 | 8 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 8 | 3ms | 11 | 3 | 6 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=8
- **tsp_bb_26_07_02:** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=8 (=ref, same tour)
- **tsp_bb_26_07_06:** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=8 (=ref, same tour)

---

## 6. `examples/converted/five-node-explicit.txt` (n=5)

**Concorde optimal cost:** 26  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 2 -> 3 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 26 | 3ms | 13 | 6 | 7 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 26 | 3ms | 1 | 1 | 1 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 4 -> 2 -> 3 -> 1 -> 0` cost=26
- **tsp_bb_26_07_02:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **tsp_bb_26_07_06:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)

---

## 7. `examples/random/sparse/rnd-01-sparse-n5.txt` (n=5)

**Concorde:** failed to find optimal solution  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 110 | 3ms | 5 | 2 | 3 | 0 | - |
| tsp_bb_26_07_06 | 110 | 3ms | 5 | 2 | 3 | 0 | - |

**Tours found:**
- **tsp_bb_26_07_02:** `3 -> 4 -> 2 -> 1 -> 0 -> 3` cost=110
- **tsp_bb_26_07_06:** `3 -> 4 -> 2 -> 1 -> 0 -> 3` cost=110

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

**Tours found:**
- **Concorde (reference):** `0 -> 2 -> 3 -> 4 -> 1 -> 0` cost=136
- **tsp_bb_26_07_02:** `0 -> 2 -> 3 -> 4 -> 1 -> 0` cost=136 (=ref, same tour)
- **tsp_bb_26_07_06:** `0 -> 2 -> 3 -> 4 -> 1 -> 0` cost=136 (=ref, same tour)

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

**Tours found:**
- **Concorde (reference):** `0 -> 3 -> 2 -> 4 -> 1 -> 0` cost=88
- **tsp_bb_26_07_02:** `0 -> 3 -> 2 -> 4 -> 1 -> 0` cost=88 (=ref, same tour)
- **tsp_bb_26_07_06:** `0 -> 3 -> 2 -> 4 -> 1 -> 0` cost=88 (=ref, same tour)

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

**Tours found:**
- **Concorde (reference):** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=51
- **tsp_bb_26_07_02:** `0 -> 3 -> 2 -> 1 -> 4 -> 0` cost=51 (=ref, different tour)
- **tsp_bb_26_07_06:** `0 -> 3 -> 2 -> 1 -> 4 -> 0` cost=51 (=ref, different tour)

---

## 11. `examples/random/sparse/rnd-04-sparse-n6.txt` (n=6)

**Concorde:** failed to find optimal solution  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 115 | 3ms | 1 | 0 | 1 | 0 | - |
| tsp_bb_26_07_06 | 115 | 3ms | 1 | 0 | 1 | 0 | - |

**Tours found:**
- **tsp_bb_26_07_02:** `0 -> 4 -> 3 -> 5 -> 1 -> 2 -> 0` cost=115
- **tsp_bb_26_07_06:** `0 -> 4 -> 3 -> 5 -> 1 -> 2 -> 0` cost=115

---

## 12. `examples/random/sparse/rnd-05-sparse-n6.txt` (n=6)

**Concorde:** failed to find optimal solution  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 100 | 3ms | 30 | 22 | 8 | 15 | - |
| tsp_bb_26_07_06 | 100 | 3ms | 15 | 8 | 3 | 4 | - |

**Tours found:**
- **tsp_bb_26_07_02:** `0 -> 5 -> 2 -> 1 -> 4 -> 3 -> 0` cost=100
- **tsp_bb_26_07_06:** `0 -> 5 -> 2 -> 1 -> 4 -> 3 -> 0` cost=100

---

## 13. `examples/random/sparse/rnd-02-sparse-n7.txt` (n=7)

**Concorde:** failed to find optimal solution  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 196 | 4ms | 19 | 13 | 6 | 8 | - |
| tsp_bb_26_07_06 | 196 | 3ms | 13 | 6 | 3 | 2 | - |

**Tours found:**
- **tsp_bb_26_07_02:** `1 -> 5 -> 3 -> 0 -> 4 -> 2 -> 6 -> 1` cost=196
- **tsp_bb_26_07_06:** `1 -> 5 -> 3 -> 0 -> 4 -> 2 -> 6 -> 1` cost=196

---

## 14. `examples/random/sparse/rnd-03-sparse-n7.txt` (n=7)

**Concorde:** failed to find optimal solution  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 86 | 4ms | 7 | 4 | 3 | 2 | - |
| tsp_bb_26_07_06 | 86 | 3ms | 1 | 1 | 1 | 0 | - |

**Tours found:**
- **tsp_bb_26_07_02:** `1 -> 6 -> 3 -> 4 -> 5 -> 2 -> 0 -> 1` cost=86
- **tsp_bb_26_07_06:** `1 -> 6 -> 3 -> 4 -> 5 -> 2 -> 0 -> 1` cost=86

---

## 15. `examples/random/complete/rnd-05-complete-n7.txt` (n=7)

**Concorde optimal cost:** 74  
**Concorde time:** 3ms  

**Concorde (reference) tour:** `0 -> 3 -> 6 -> 5 -> 1 -> 4 -> 2 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 74 | 3ms | 23 | 11 | 12 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 74 | 4ms | 8 | 4 | 1 | 1 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 3 -> 6 -> 5 -> 1 -> 4 -> 2 -> 0` cost=74
- **tsp_bb_26_07_02:** `0 -> 2 -> 4 -> 1 -> 5 -> 6 -> 3 -> 0` cost=74 (=ref, different tour)
- **tsp_bb_26_07_06:** `0 -> 2 -> 4 -> 1 -> 5 -> 6 -> 3 -> 0` cost=74 (=ref, different tour)

---

## 16. `examples/random/sparse/rnd-06-sparse-n7.txt` (n=7)

**Concorde:** failed to find optimal solution  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 199 | 4ms | 28 | 21 | 7 | 15 | - |
| tsp_bb_26_07_06 | 199 | 4ms | 7 | 4 | 1 | 0 | - |

**Tours found:**
- **tsp_bb_26_07_02:** `4 -> 1 -> 5 -> 3 -> 2 -> 6 -> 0 -> 4` cost=199
- **tsp_bb_26_07_06:** `4 -> 1 -> 5 -> 3 -> 2 -> 6 -> 0 -> 4` cost=199

---

## 17. `examples/random/complete/rnd-08-complete-n7.txt` (n=7)

**Concorde optimal cost:** 73  
**Concorde time:** 3ms  

**Concorde (reference) tour:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 73 | 4ms | 21 | 10 | 11 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 73 | 3ms | 3 | 2 | 1 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0` cost=73
- **tsp_bb_26_07_02:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0` cost=73 (=ref, same tour)
- **tsp_bb_26_07_06:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0` cost=73 (=ref, same tour)

---

## 18. `examples/random/complete/rnd-10-complete-n7.txt` (n=7)

**Concorde optimal cost:** 49  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 6 -> 4 -> 5 -> 3 -> 2 -> 1 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 49 | 3ms | 13 | 6 | 7 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 49 | 3ms | 5 | 3 | 1 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 6 -> 4 -> 5 -> 3 -> 2 -> 1 -> 0` cost=49
- **tsp_bb_26_07_02:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 0` cost=49 (=ref, different tour)
- **tsp_bb_26_07_06:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 0` cost=49 (=ref, different tour)

---

## 19. `examples/random/complete/rnd-07-complete-n8.txt` (n=8)

**Concorde optimal cost:** 64  
**Concorde time:** 3ms  

**Concorde (reference) tour:** `0 -> 7 -> 4 -> 3 -> 2 -> 1 -> 6 -> 5 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 64 | 4ms | 23 | 11 | 12 | 0 | :white_check_mark: |
| tsp_bb_26_07_06 | 64 | 4ms | 7 | 4 | 2 | 0 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 7 -> 4 -> 3 -> 2 -> 1 -> 6 -> 5 -> 0` cost=64
- **tsp_bb_26_07_02:** `2 -> 3 -> 4 -> 7 -> 0 -> 5 -> 6 -> 1 -> 2` cost=64 (=ref, different tour)
- **tsp_bb_26_07_06:** `2 -> 3 -> 4 -> 7 -> 0 -> 5 -> 6 -> 1 -> 2` cost=64 (=ref, different tour)

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

**Tours found:**
- **Concorde (reference):** `0 -> 5 -> 4 -> 6 -> 1 -> 3 -> 2 -> 7 -> 0` cost=93
- **tsp_bb_26_07_02:** `1 -> 3 -> 2 -> 7 -> 0 -> 5 -> 4 -> 6 -> 1` cost=93 (=ref, different tour)
- **tsp_bb_26_07_06:** `1 -> 3 -> 2 -> 7 -> 0 -> 5 -> 4 -> 6 -> 1` cost=93 (=ref, different tour)

---

## 21. `data/classic/tsplib/burma14.tsp` (n=14)

**Concorde optimal cost:** 3323  
**Concorde time:** 20ms  

**Concorde (reference) tour:** `0 -> 1 -> 13 -> 2 -> 3 -> 4 -> 5 -> 11 -> 6 -> 12 -> 7 -> 10 -> 8 -> 9 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 3323 | 38ms | 18111 | 9162 | 8949 | 214 | :white_check_mark: |
| tsp_bb_26_07_06 | 3323 | 43ms | 16816 | 6016 | 4640 | 21 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 1 -> 13 -> 2 -> 3 -> 4 -> 5 -> 11 -> 6 -> 12 -> 7 -> 10 -> 8 -> 9 -> 0` cost=3323
- **tsp_bb_26_07_02:** `1 -> 0 -> 9 -> 8 -> 10 -> 7 -> 12 -> 6 -> 11 -> 5 -> 4 -> 3 -> 2 -> 13 -> 1` cost=3323 (=ref, different tour)
- **tsp_bb_26_07_06:** `1 -> 0 -> 9 -> 8 -> 10 -> 7 -> 12 -> 6 -> 11 -> 5 -> 4 -> 3 -> 2 -> 13 -> 1` cost=3323 (=ref, different tour)

---

## 22. `data/classic/tsplib/ulysses16.tsp` (n=16)

**Concorde optimal cost:** 6859  
**Concorde time:** 26ms  

**Concorde (reference) tour:** `0 -> 13 -> 12 -> 11 -> 6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 6859 | 16.1s | 15447419 | 7797474 | 7649945 | 147530 | :white_check_mark: |
| tsp_bb_26_07_06 | 6859 | 13.8s | 9328699 | 2855266 | 2796120 | 45196 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 13 -> 12 -> 11 -> 6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0` cost=6859
- **tsp_bb_26_07_02:** `6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0 -> 13 -> 12 -> 11 -> 6` cost=6859 (=ref, different tour)
- **tsp_bb_26_07_06:** `6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0 -> 13 -> 12 -> 11 -> 6` cost=6859 (=ref, different tour)

---

## 23. `data/classic/tsplib/gr17.tsp` (n=17)

**Concorde optimal cost:** 2085  
**Concorde time:** 26ms  

**Concorde (reference) tour:** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 2085 | 2.2s | 1817688 | 920131 | 897557 | 22575 | :white_check_mark: |
| tsp_bb_26_07_06 | 2085 | 2.1s | 1237494 | 443475 | 343503 | 19611 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0` cost=2085
- **tsp_bb_26_07_02:** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0` cost=2085 (=ref, same tour)
- **tsp_bb_26_07_06:** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0` cost=2085 (=ref, same tour)

---

## 24. `data/classic/tsplib/gr21.tsp` (n=21)

**Concorde optimal cost:** 2707  
**Concorde time:** 13ms  

**Concorde (reference) tour:** `0 -> 6 -> 7 -> 5 -> 15 -> 4 -> 8 -> 2 -> 1 -> 20 -> 14 -> 13 -> 12 -> 17 -> 9 -> 16 -> 18 -> 19 -> 10 -> 3 -> 11 -> 0`  

### Results

| Solver | Cost | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | Match Ref |
|---|---|---|---|---|---|---|---|
| tsp_bb_26_07_02 | 2707 | 78ms | 34213 | 17314 | 16899 | 416 | :white_check_mark: |
| tsp_bb_26_07_06 | 2707 | 75ms | 21386 | 8915 | 4966 | 301 | :white_check_mark: |

**Tours found:**
- **Concorde (reference):** `0 -> 6 -> 7 -> 5 -> 15 -> 4 -> 8 -> 2 -> 1 -> 20 -> 14 -> 13 -> 12 -> 17 -> 9 -> 16 -> 18 -> 19 -> 10 -> 3 -> 11 -> 0` cost=2707
- **tsp_bb_26_07_02:** `2 -> 8 -> 4 -> 15 -> 5 -> 7 -> 6 -> 0 -> 11 -> 3 -> 10 -> 19 -> 18 -> 16 -> 9 -> 17 -> 12 -> 13 -> 14 -> 20 -> 1 -> 2` cost=2707 (=ref, different tour)
- **tsp_bb_26_07_06:** `2 -> 8 -> 4 -> 15 -> 5 -> 7 -> 6 -> 0 -> 11 -> 3 -> 10 -> 19 -> 18 -> 16 -> 9 -> 17 -> 12 -> 13 -> 14 -> 20 -> 1 -> 2` cost=2707 (=ref, different tour)

---
