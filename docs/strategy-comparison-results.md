# TSP Algorithm Comparison

## Algorithms Compared

| Algorithm | Description |
|---|---|
| **Concorde** | State-of-the-art Concorde TSP solver (exact, with QSopt LP) — **reference** |
| **Smart** | Branch & Bound with smart branching (1-tree degree + edge candidate) |
| **Simple** | Branch & Bound with simple branching (max-degree vertex) |

**Instances:** 39 from `examples` (n <= 49), `data/classic/tsplib` (n <= 59), `data/classic/national` (n <= 59)  
**Timeout:** 3600s (1h) per method per instance  
**Reference:** Concorde exact solver  

---

## 1. `examples/random/complete/rnd-01-complete-n4.txt` (n=4)

**Concorde optimal cost:** 45  
**Concorde time:** 3ms  

**Concorde (reference) tour:** `0 -> 3 -> 2 -> 1 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 45 | ref | 3ms | - | - | - | - | - |
| Smart | 45 | :white_check_mark: | 4ms | 5 | 2 | 3 | 0 | - |
| Simple | 45 | :white_check_mark: | 4ms | 5 | 2 | 3 | 0 | - |

**Tours found:**
- **Concorde:** `0 -> 3 -> 2 -> 1 -> 0` cost=45 (reference)
- **Smart:** `0 -> 3 -> 2 -> 1 -> 0` cost=45 (=ref, same tour)
- **Simple:** `0 -> 3 -> 2 -> 1 -> 0` cost=45 (=ref, same tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=4 method=exact
[tsp-debug] exact solve started: vertices=4
[tsp-debug] initial incumbent: cost=45
[tsp-debug] root: lower_bound=36 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=45 expanded=2 created=5 pruned_bound=3 pruned_infeasible=0
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=4 method=exact
[tsp-debug] exact solve started: vertices=4
[tsp-debug] initial incumbent: cost=45
[tsp-debug] root: lower_bound=36 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=45 expanded=2 created=5 pruned_bound=3 pruned_infeasible=0
```
</details>

---

## 2. `examples/random/complete/rnd-03-complete-n4.txt` (n=4)

**Concorde optimal cost:** 102  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 1 -> 3 -> 2 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 102 | ref | 2ms | - | - | - | - | - |
| Smart | 102 | :white_check_mark: | 4ms | 1 | 0 | 1 | 0 | - |
| Simple | 102 | :white_check_mark: | 4ms | 1 | 0 | 1 | 0 | - |

**Tours found:**
- **Concorde:** `0 -> 1 -> 3 -> 2 -> 0` cost=102 (reference)
- **Smart:** `0 -> 1 -> 3 -> 2 -> 0` cost=102 (=ref, same tour)
- **Simple:** `0 -> 1 -> 3 -> 2 -> 0` cost=102 (=ref, same tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=4 method=exact
[tsp-debug] exact solve started: vertices=4
[tsp-debug] initial incumbent: cost=102
[tsp-debug] root: lower_bound=102 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=102 expanded=0 created=1 pruned_bound=1 pruned_infeasible=0
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=4 method=exact
[tsp-debug] exact solve started: vertices=4
[tsp-debug] initial incumbent: cost=102
[tsp-debug] root: lower_bound=102 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=102 expanded=0 created=1 pruned_bound=1 pruned_infeasible=0
```
</details>

---

## 3. `examples/five-city.tsp` (n=5)

**Concorde optimal cost:** 26  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 2 -> 3 -> 1 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 26 | ref | 2ms | - | - | - | - | - |
| Smart | 26 | :white_check_mark: | 4ms | 20 | 10 | 10 | 1 | - |
| Simple | 26 | :white_check_mark: | 2ms | 13 | 6 | 7 | 0 | - |

**Tours found:**
- **Concorde:** `0 -> 4 -> 2 -> 3 -> 1 -> 0` cost=26 (reference)
- **Smart:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **Simple:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=five_city dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=26
[tsp-debug] root: lower_bound=21 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=26 expanded=10 created=20 pruned_bound=10 pruned_infeasible=1
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=five_city dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=26
[tsp-debug] root: lower_bound=21 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=26 expanded=6 created=13 pruned_bound=7 pruned_infeasible=0
```
</details>

---

## 4. `examples/five-city.txt` (n=5)

**Concorde optimal cost:** 26  
**Concorde time:** 3ms  

**Concorde (reference) tour:** `0 -> 4 -> 2 -> 3 -> 1 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 26 | ref | 3ms | - | - | - | - | - |
| Smart | 26 | :white_check_mark: | 4ms | 20 | 10 | 10 | 1 | - |
| Simple | 26 | :white_check_mark: | 3ms | 13 | 6 | 7 | 0 | - |

**Tours found:**
- **Concorde:** `0 -> 4 -> 2 -> 3 -> 1 -> 0` cost=26 (reference)
- **Smart:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **Simple:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=26
[tsp-debug] root: lower_bound=21 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=26 expanded=10 created=20 pruned_bound=10 pruned_infeasible=1
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=26
[tsp-debug] root: lower_bound=21 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=26 expanded=6 created=13 pruned_bound=7 pruned_infeasible=0
```
</details>

---

## 5. `examples/converted/five-node-euc.txt` (n=5)

**Concorde optimal cost:** 8  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 1 -> 2 -> 3 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 8 | ref | 2ms | - | - | - | - | - |
| Smart | 8 | :white_check_mark: | 3ms | 11 | 5 | 6 | 0 | - |
| Simple | 8 | :white_check_mark: | 3ms | 11 | 5 | 6 | 0 | - |

**Tours found:**
- **Concorde:** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=8 (reference)
- **Smart:** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=8 (=ref, same tour)
- **Simple:** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=8 (=ref, same tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=8
[tsp-debug] root: lower_bound=6 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=8 expanded=5 created=11 pruned_bound=6 pruned_infeasible=0
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=8
[tsp-debug] root: lower_bound=6 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=8 expanded=5 created=11 pruned_bound=6 pruned_infeasible=0
```
</details>

---

## 6. `examples/converted/five-node-explicit.txt` (n=5)

**Concorde optimal cost:** 26  
**Concorde time:** 3ms  

**Concorde (reference) tour:** `0 -> 4 -> 2 -> 3 -> 1 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 26 | ref | 3ms | - | - | - | - | - |
| Smart | 26 | :white_check_mark: | 4ms | 20 | 10 | 10 | 1 | - |
| Simple | 26 | :white_check_mark: | 2ms | 13 | 6 | 7 | 0 | - |

**Tours found:**
- **Concorde:** `0 -> 4 -> 2 -> 3 -> 1 -> 0` cost=26 (reference)
- **Smart:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)
- **Simple:** `0 -> 1 -> 3 -> 2 -> 4 -> 0` cost=26 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=26
[tsp-debug] root: lower_bound=21 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=26 expanded=10 created=20 pruned_bound=10 pruned_infeasible=1
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=26
[tsp-debug] root: lower_bound=21 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=26 expanded=6 created=13 pruned_bound=7 pruned_infeasible=0
```
</details>

---

## 7. `examples/random/sparse/rnd-01-sparse-n5.txt` (n=5)

**Concorde optimal cost:** 110  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 3 -> 4 -> 2 -> 1 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 110 | ref | 2ms | - | - | - | - | - |
| Smart | 110 | :white_check_mark: | 3ms | 5 | 2 | 3 | 0 | - |
| Simple | 110 | :white_check_mark: | 3ms | 5 | 2 | 3 | 0 | - |

**Tours found:**
- **Concorde:** `0 -> 3 -> 4 -> 2 -> 1 -> 0` cost=110 (reference)
- **Smart:** `3 -> 4 -> 2 -> 1 -> 0 -> 3` cost=110 (=ref, different tour)
- **Simple:** `3 -> 4 -> 2 -> 1 -> 0 -> 3` cost=110 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=110
[tsp-debug] root: lower_bound=102 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=110 expanded=2 created=5 pruned_bound=3 pruned_infeasible=0
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=110
[tsp-debug] root: lower_bound=102 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=110 expanded=2 created=5 pruned_bound=3 pruned_infeasible=0
```
</details>

---

## 8. `examples/random/complete/rnd-02-complete-n5.txt` (n=5)

**Concorde optimal cost:** 136  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 2 -> 3 -> 4 -> 1 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 136 | ref | 2ms | - | - | - | - | - |
| Smart | 136 | :white_check_mark: | 2ms | 9 | 4 | 5 | 0 | - |
| Simple | 136 | :white_check_mark: | 3ms | 9 | 4 | 5 | 0 | - |

**Tours found:**
- **Concorde:** `0 -> 2 -> 3 -> 4 -> 1 -> 0` cost=136 (reference)
- **Smart:** `0 -> 2 -> 3 -> 4 -> 1 -> 0` cost=136 (=ref, same tour)
- **Simple:** `0 -> 2 -> 3 -> 4 -> 1 -> 0` cost=136 (=ref, same tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=136
[tsp-debug] root: lower_bound=127 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=136 expanded=4 created=9 pruned_bound=5 pruned_infeasible=0
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=136
[tsp-debug] root: lower_bound=127 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=136 expanded=4 created=9 pruned_bound=5 pruned_infeasible=0
```
</details>

---

## 9. `examples/random/complete/rnd-04-complete-n5.txt` (n=5)

**Concorde optimal cost:** 88  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 3 -> 2 -> 4 -> 1 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 88 | ref | 2ms | - | - | - | - | - |
| Smart | 88 | :white_check_mark: | 2ms | 9 | 4 | 5 | 0 | - |
| Simple | 88 | :white_check_mark: | 3ms | 7 | 3 | 4 | 0 | - |

**Tours found:**
- **Concorde:** `0 -> 3 -> 2 -> 4 -> 1 -> 0` cost=88 (reference)
- **Smart:** `0 -> 3 -> 2 -> 4 -> 1 -> 0` cost=88 (=ref, same tour)
- **Simple:** `0 -> 3 -> 2 -> 4 -> 1 -> 0` cost=88 (=ref, same tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=88
[tsp-debug] root: lower_bound=79 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=88 expanded=4 created=9 pruned_bound=5 pruned_infeasible=0
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=88
[tsp-debug] root: lower_bound=79 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=88 expanded=3 created=7 pruned_bound=4 pruned_infeasible=0
```
</details>

---

## 10. `examples/random/complete/rnd-06-complete-n5.txt` (n=5)

**Concorde optimal cost:** 51  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 1 -> 2 -> 3 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 51 | ref | 2ms | - | - | - | - | - |
| Smart | 51 | :white_check_mark: | 2ms | 9 | 4 | 5 | 0 | - |
| Simple | 51 | :white_check_mark: | 3ms | 9 | 4 | 5 | 0 | - |

**Tours found:**
- **Concorde:** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=51 (reference)
- **Smart:** `0 -> 3 -> 2 -> 1 -> 4 -> 0` cost=51 (=ref, different tour)
- **Simple:** `0 -> 3 -> 2 -> 1 -> 4 -> 0` cost=51 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=51
[tsp-debug] root: lower_bound=43 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=51 expanded=4 created=9 pruned_bound=5 pruned_infeasible=0
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=51
[tsp-debug] root: lower_bound=43 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=51 expanded=4 created=9 pruned_bound=5 pruned_infeasible=0
```
</details>

---

## 11. `examples/random/sparse/rnd-04-sparse-n6.txt` (n=6)

**Concorde optimal cost:** 115  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 3 -> 5 -> 1 -> 2 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 115 | ref | 2ms | - | - | - | - | - |
| Smart | 115 | :white_check_mark: | 2ms | 1 | 0 | 1 | 0 | - |
| Simple | 115 | :white_check_mark: | 3ms | 1 | 0 | 1 | 0 | - |

**Tours found:**
- **Concorde:** `0 -> 4 -> 3 -> 5 -> 1 -> 2 -> 0` cost=115 (reference)
- **Smart:** `0 -> 4 -> 3 -> 5 -> 1 -> 2 -> 0` cost=115 (=ref, same tour)
- **Simple:** `0 -> 4 -> 3 -> 5 -> 1 -> 2 -> 0` cost=115 (=ref, same tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=6 method=exact
[tsp-debug] exact solve started: vertices=6
[tsp-debug] initial incumbent: cost=115
[tsp-debug] root: lower_bound=115 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=115 expanded=0 created=1 pruned_bound=1 pruned_infeasible=0
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=6 method=exact
[tsp-debug] exact solve started: vertices=6
[tsp-debug] initial incumbent: cost=115
[tsp-debug] root: lower_bound=115 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=115 expanded=0 created=1 pruned_bound=1 pruned_infeasible=0
```
</details>

---

## 12. `examples/random/sparse/rnd-05-sparse-n6.txt` (n=6)

**Concorde optimal cost:** 100  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 5 -> 2 -> 1 -> 4 -> 3 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 100 | ref | 2ms | - | - | - | - | - |
| Smart | 100 | :white_check_mark: | 4ms | 26 | 18 | 8 | 11 | - |
| Simple | 100 | :white_check_mark: | 4ms | 29 | 22 | 7 | 16 | - |

**Tours found:**
- **Concorde:** `0 -> 5 -> 2 -> 1 -> 4 -> 3 -> 0` cost=100 (reference)
- **Smart:** `0 -> 5 -> 2 -> 1 -> 4 -> 3 -> 0` cost=100 (=ref, same tour)
- **Simple:** `0 -> 5 -> 2 -> 1 -> 4 -> 3 -> 0` cost=100 (=ref, same tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=6 method=exact
[tsp-debug] exact solve started: vertices=6
[tsp-debug] initial incumbent: cost=100
[tsp-debug] root: lower_bound=53 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=100 expanded=18 created=26 pruned_bound=8 pruned_infeasible=11
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=6 method=exact
[tsp-debug] exact solve started: vertices=6
[tsp-debug] initial incumbent: cost=100
[tsp-debug] root: lower_bound=53 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=100 expanded=22 created=29 pruned_bound=7 pruned_infeasible=16
```
</details>

---

## 13. `examples/random/sparse/rnd-02-sparse-n7.txt` (n=7)

**Concorde optimal cost:** 196  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 3 -> 5 -> 1 -> 6 -> 2 -> 4 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 196 | ref | 2ms | - | - | - | - | - |
| Smart | 196 | :white_check_mark: | 2ms | 16 | 10 | 6 | 5 | - |
| Simple | 196 | :white_check_mark: | 3ms | 31 | 21 | 10 | 12 | - |

**Tours found:**
- **Concorde:** `0 -> 3 -> 5 -> 1 -> 6 -> 2 -> 4 -> 0` cost=196 (reference)
- **Smart:** `1 -> 5 -> 3 -> 0 -> 4 -> 2 -> 6 -> 1` cost=196 (=ref, different tour)
- **Simple:** `1 -> 5 -> 3 -> 0 -> 4 -> 2 -> 6 -> 1` cost=196 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=196
[tsp-debug] root: lower_bound=161 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=196 expanded=10 created=16 pruned_bound=6 pruned_infeasible=5
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=196
[tsp-debug] root: lower_bound=161 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=196 expanded=21 created=31 pruned_bound=10 pruned_infeasible=12
```
</details>

---

## 14. `examples/random/sparse/rnd-03-sparse-n7.txt` (n=7)

**Concorde optimal cost:** 86  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 2 -> 5 -> 4 -> 3 -> 6 -> 1 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 86 | ref | 2ms | - | - | - | - | - |
| Smart | 86 | :white_check_mark: | 4ms | 7 | 4 | 3 | 2 | - |
| Simple | 86 | :white_check_mark: | 3ms | 12 | 9 | 3 | 7 | - |

**Tours found:**
- **Concorde:** `0 -> 2 -> 5 -> 4 -> 3 -> 6 -> 1 -> 0` cost=86 (reference)
- **Smart:** `1 -> 6 -> 3 -> 4 -> 5 -> 2 -> 0 -> 1` cost=86 (=ref, different tour)
- **Simple:** `1 -> 6 -> 3 -> 4 -> 5 -> 2 -> 0 -> 1` cost=86 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=86
[tsp-debug] root: lower_bound=78 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=86 expanded=4 created=7 pruned_bound=3 pruned_infeasible=2
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=86
[tsp-debug] root: lower_bound=78 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=86 expanded=9 created=12 pruned_bound=3 pruned_infeasible=7
```
</details>

---

## 15. `examples/random/complete/rnd-05-complete-n7.txt` (n=7)

**Concorde optimal cost:** 74  
**Concorde time:** 3ms  

**Concorde (reference) tour:** `0 -> 3 -> 6 -> 5 -> 1 -> 4 -> 2 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 74 | ref | 3ms | - | - | - | - | - |
| Smart | 74 | :white_check_mark: | 3ms | 33 | 16 | 17 | 0 | - |
| Simple | 74 | :white_check_mark: | 3ms | 32 | 16 | 16 | 1 | - |

**Tours found:**
- **Concorde:** `0 -> 3 -> 6 -> 5 -> 1 -> 4 -> 2 -> 0` cost=74 (reference)
- **Smart:** `0 -> 2 -> 4 -> 1 -> 5 -> 6 -> 3 -> 0` cost=74 (=ref, different tour)
- **Simple:** `0 -> 2 -> 4 -> 1 -> 5 -> 6 -> 3 -> 0` cost=74 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=74
[tsp-debug] root: lower_bound=49 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=74 expanded=16 created=33 pruned_bound=17 pruned_infeasible=0
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=74
[tsp-debug] root: lower_bound=49 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=74 expanded=16 created=32 pruned_bound=16 pruned_infeasible=1
```
</details>

---

## 16. `examples/random/sparse/rnd-06-sparse-n7.txt` (n=7)

**Concorde optimal cost:** 199  
**Concorde time:** 3ms  

**Concorde (reference) tour:** `0 -> 4 -> 1 -> 5 -> 3 -> 2 -> 6 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 199 | ref | 3ms | - | - | - | - | - |
| Smart | 199 | :white_check_mark: | 4ms | 32 | 22 | 10 | 13 | - |
| Simple | 199 | :white_check_mark: | 2ms | 28 | 22 | 6 | 17 | - |

**Tours found:**
- **Concorde:** `0 -> 4 -> 1 -> 5 -> 3 -> 2 -> 6 -> 0` cost=199 (reference)
- **Smart:** `4 -> 1 -> 5 -> 3 -> 2 -> 6 -> 0 -> 4` cost=199 (=ref, different tour)
- **Simple:** `4 -> 1 -> 5 -> 3 -> 2 -> 6 -> 0 -> 4` cost=199 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=199
[tsp-debug] root: lower_bound=169 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=199 expanded=22 created=32 pruned_bound=10 pruned_infeasible=13
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=199
[tsp-debug] root: lower_bound=169 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=199 expanded=22 created=28 pruned_bound=6 pruned_infeasible=17
```
</details>

---

## 17. `examples/random/complete/rnd-08-complete-n7.txt` (n=7)

**Concorde optimal cost:** 73  
**Concorde time:** 3ms  

**Concorde (reference) tour:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 73 | ref | 3ms | - | - | - | - | - |
| Smart | 73 | :white_check_mark: | 2ms | 27 | 13 | 14 | 0 | - |
| Simple | 73 | :white_check_mark: | 3ms | 31 | 15 | 16 | 0 | - |

**Tours found:**
- **Concorde:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0` cost=73 (reference)
- **Smart:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0` cost=73 (=ref, same tour)
- **Simple:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0` cost=73 (=ref, same tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=73
[tsp-debug] root: lower_bound=50 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=73 expanded=13 created=27 pruned_bound=14 pruned_infeasible=0
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=73
[tsp-debug] root: lower_bound=50 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=73 expanded=15 created=31 pruned_bound=16 pruned_infeasible=0
```
</details>

---

## 18. `examples/random/complete/rnd-10-complete-n7.txt` (n=7)

**Concorde optimal cost:** 49  
**Concorde time:** 1ms  

**Concorde (reference) tour:** `0 -> 6 -> 4 -> 5 -> 3 -> 2 -> 1 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 49 | ref | 1ms | - | - | - | - | - |
| Smart | 49 | :white_check_mark: | 3ms | 15 | 7 | 8 | 0 | - |
| Simple | 49 | :white_check_mark: | 2ms | 23 | 11 | 12 | 0 | - |

**Tours found:**
- **Concorde:** `0 -> 6 -> 4 -> 5 -> 3 -> 2 -> 1 -> 0` cost=49 (reference)
- **Smart:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 0` cost=49 (=ref, different tour)
- **Simple:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 0` cost=49 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=49
[tsp-debug] root: lower_bound=45 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=49 expanded=7 created=15 pruned_bound=8 pruned_infeasible=0
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=49
[tsp-debug] root: lower_bound=45 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=49 expanded=11 created=23 pruned_bound=12 pruned_infeasible=0
```
</details>

---

## 19. `examples/random/complete/rnd-07-complete-n8.txt` (n=8)

**Concorde optimal cost:** 64  
**Concorde time:** 3ms  

**Concorde (reference) tour:** `0 -> 7 -> 4 -> 3 -> 2 -> 1 -> 6 -> 5 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 64 | ref | 3ms | - | - | - | - | - |
| Smart | 64 | :white_check_mark: | 4ms | 31 | 15 | 16 | 0 | - |
| Simple | 64 | :white_check_mark: | 4ms | 27 | 13 | 14 | 0 | - |

**Tours found:**
- **Concorde:** `0 -> 7 -> 4 -> 3 -> 2 -> 1 -> 6 -> 5 -> 0` cost=64 (reference)
- **Smart:** `2 -> 3 -> 4 -> 7 -> 0 -> 5 -> 6 -> 1 -> 2` cost=64 (=ref, different tour)
- **Simple:** `2 -> 3 -> 4 -> 7 -> 0 -> 5 -> 6 -> 1 -> 2` cost=64 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=8 method=exact
[tsp-debug] exact solve started: vertices=8
[tsp-debug] initial incumbent: cost=64
[tsp-debug] root: lower_bound=46 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=64 expanded=15 created=31 pruned_bound=16 pruned_infeasible=0
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=8 method=exact
[tsp-debug] exact solve started: vertices=8
[tsp-debug] initial incumbent: cost=64
[tsp-debug] root: lower_bound=46 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=64 expanded=13 created=27 pruned_bound=14 pruned_infeasible=0
```
</details>

---

## 20. `examples/random/complete/rnd-09-complete-n8.txt` (n=8)

**Concorde optimal cost:** 93  
**Concorde time:** 1ms  

**Concorde (reference) tour:** `0 -> 5 -> 4 -> 6 -> 1 -> 3 -> 2 -> 7 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 93 | ref | 1ms | - | - | - | - | - |
| Smart | 93 | :white_check_mark: | 4ms | 116 | 58 | 58 | 1 | - |
| Simple | 93 | :white_check_mark: | 2ms | 81 | 40 | 41 | 0 | - |

**Tours found:**
- **Concorde:** `0 -> 5 -> 4 -> 6 -> 1 -> 3 -> 2 -> 7 -> 0` cost=93 (reference)
- **Smart:** `1 -> 3 -> 2 -> 7 -> 0 -> 5 -> 4 -> 6 -> 1` cost=93 (=ref, different tour)
- **Simple:** `1 -> 3 -> 2 -> 7 -> 0 -> 5 -> 4 -> 6 -> 1` cost=93 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=8 method=exact
[tsp-debug] exact solve started: vertices=8
[tsp-debug] initial incumbent: cost=93
[tsp-debug] root: lower_bound=40 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=93 expanded=58 created=116 pruned_bound=58 pruned_infeasible=1
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=8 method=exact
[tsp-debug] exact solve started: vertices=8
[tsp-debug] initial incumbent: cost=93
[tsp-debug] root: lower_bound=40 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=93 expanded=40 created=81 pruned_bound=41 pruned_infeasible=0
```
</details>

---

## 21. `data/classic/tsplib/burma14.tsp` (n=14)

**Concorde optimal cost:** 3323  
**Concorde time:** 19ms  

**Concorde (reference) tour:** `0 -> 1 -> 13 -> 2 -> 3 -> 4 -> 5 -> 11 -> 6 -> 12 -> 7 -> 10 -> 8 -> 9 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 3323 | ref | 19ms | - | - | - | - | 1 |
| Smart | 3323 | :white_check_mark: | 317ms | 92069 | 46037 | 46032 | 6 | - |
| Simple | 3323 | :white_check_mark: | 112ms | 36847 | 18437 | 18410 | 28 | - |

**Tours found:**
- **Concorde:** `0 -> 1 -> 13 -> 2 -> 3 -> 4 -> 5 -> 11 -> 6 -> 12 -> 7 -> 10 -> 8 -> 9 -> 0` cost=3323 (reference)
- **Smart:** `1 -> 0 -> 9 -> 8 -> 10 -> 7 -> 12 -> 6 -> 11 -> 5 -> 4 -> 3 -> 2 -> 13 -> 1` cost=3323 (=ref, different tour)
- **Simple:** `1 -> 0 -> 9 -> 8 -> 10 -> 7 -> 12 -> 6 -> 11 -> 5 -> 4 -> 3 -> 2 -> 13 -> 1` cost=3323 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=burma14 dimension=14 method=exact
[tsp-debug] exact solve started: vertices=14
[tsp-debug] initial incumbent: cost=3323
[tsp-debug] root: lower_bound=2542 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=3323 expanded=46037 created=92069 pruned_bound=46032 pruned_infeasible=6
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=burma14 dimension=14 method=exact
[tsp-debug] exact solve started: vertices=14
[tsp-debug] initial incumbent: cost=3323
[tsp-debug] root: lower_bound=2542 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=3323 expanded=18437 created=36847 pruned_bound=18410 pruned_infeasible=28
```
</details>

<details>
<summary>Concorde debug output</summary>

```
ERROR: No dual change in basis finding code
Did not find a basic optimal solution
Fractional matching routine failed
Warning: restarting running timer Miscellaneous
No warmstart, stumbling on anyway
```
</details>

---

## 22. `data/classic/tsplib/ulysses16.tsp` (n=16)

**Concorde optimal cost:** 6859  
**Concorde time:** 41ms  

**Concorde (reference) tour:** `0 -> 13 -> 12 -> 11 -> 6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 6859 | ref | 41ms | - | - | - | - | 1 |
| Smart | 6859 | :white_check_mark: | 2.8m | 43906384 | 21954796 | 21951588 | 3209 | - |
| Simple | 6859 | :white_check_mark: | 54.7s | 18881497 | 9511724 | 9369773 | 141952 | - |

**Tours found:**
- **Concorde:** `0 -> 13 -> 12 -> 11 -> 6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0` cost=6859 (reference)
- **Smart:** `6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0 -> 13 -> 12 -> 11 -> 6` cost=6859 (=ref, different tour)
- **Simple:** `6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0 -> 13 -> 12 -> 11 -> 6` cost=6859 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=ulysses16.tsp dimension=16 method=exact
[tsp-debug] exact solve started: vertices=16
[tsp-debug] initial incumbent: cost=6859
[tsp-debug] root: lower_bound=4746 search=recursive-dfs strategy=smart
[tsp-debug] progress: expanded=5000000 created=9998687 depth=37 bound=6843 best=6859 pruned_bound=4998687 pruned_infeasible=1304
[tsp-debug] progress: expanded=10000000 created=19997452 depth=31 bound=6607 best=6859 pruned_bound=9997452 pruned_infeasible=2541
[tsp-debug] progress: expanded=15000000 created=29997145 depth=44 bound=6855 best=6859 pruned_bound=14997145 pruned_infeasible=2849
[tsp-debug] progress: expanded=20000000 created=39996811 depth=44 bound=6446 best=6859 pruned_bound=19996811 pruned_infeasible=3183
[tsp-debug] exact solve finished: feasible=yes cost=6859 expanded=21954796 created=43906384 pruned_bound=21951588 pruned_infeasible=3209
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=ulysses16.tsp dimension=16 method=exact
[tsp-debug] exact solve started: vertices=16
[tsp-debug] initial incumbent: cost=6859
[tsp-debug] root: lower_bound=4746 search=recursive-dfs strategy=simple
[tsp-debug] progress: expanded=5000000 created=9924156 depth=17 bound=6169 best=6859 pruned_bound=4924156 pruned_infeasible=75835
[tsp-debug] exact solve finished: feasible=yes cost=6859 expanded=9511724 created=18881497 pruned_bound=9369773 pruned_infeasible=141952
```
</details>

---

## 23. `data/classic/tsplib/gr17.tsp` (n=17)

**Concorde optimal cost:** 2085  
**Concorde time:** 25ms  

**Concorde (reference) tour:** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 2085 | ref | 25ms | - | - | - | - | 1 |
| Smart | 2085 | :white_check_mark: | 44.9s | 10034093 | 5026364 | 5007729 | 18636 | - |
| Simple | 2085 | :white_check_mark: | 10.3s | 2664277 | 1352962 | 1311315 | 41648 | - |

**Tours found:**
- **Concorde:** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0` cost=2085 (reference)
- **Smart:** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0` cost=2085 (=ref, same tour)
- **Simple:** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0` cost=2085 (=ref, same tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=gr17 dimension=17 method=exact
[tsp-debug] exact solve started: vertices=17
[tsp-debug] initial incumbent: cost=2085
[tsp-debug] root: lower_bound=1501 search=recursive-dfs strategy=smart
[tsp-debug] progress: expanded=5000000 created=9981442 depth=27 bound=2022 best=2085 pruned_bound=4981442 pruned_infeasible=18551
[tsp-debug] exact solve finished: feasible=yes cost=2085 expanded=5026364 created=10034093 pruned_bound=5007729 pruned_infeasible=18636
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=gr17 dimension=17 method=exact
[tsp-debug] exact solve started: vertices=17
[tsp-debug] initial incumbent: cost=2085
[tsp-debug] root: lower_bound=1501 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=2085 expanded=1352962 created=2664277 pruned_bound=1311315 pruned_infeasible=41648
```
</details>

---

## 24. `data/classic/tsplib/gr21.tsp` (n=21)

**Concorde optimal cost:** 2707  
**Concorde time:** 13ms  

**Concorde (reference) tour:** `0 -> 6 -> 7 -> 5 -> 15 -> 4 -> 8 -> 2 -> 1 -> 20 -> 14 -> 13 -> 12 -> 17 -> 9 -> 16 -> 18 -> 19 -> 10 -> 3 -> 11 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 2707 | ref | 13ms | - | - | - | - | 1 |
| Smart | 2707 | :white_check_mark: | 883ms | 125049 | 62716 | 62333 | 384 | - |
| Simple | 2707 | :white_check_mark: | 539ms | 78937 | 39933 | 39004 | 930 | - |

**Tours found:**
- **Concorde:** `0 -> 6 -> 7 -> 5 -> 15 -> 4 -> 8 -> 2 -> 1 -> 20 -> 14 -> 13 -> 12 -> 17 -> 9 -> 16 -> 18 -> 19 -> 10 -> 3 -> 11 -> 0` cost=2707 (reference)
- **Smart:** `2 -> 8 -> 4 -> 15 -> 5 -> 7 -> 6 -> 0 -> 11 -> 3 -> 10 -> 19 -> 18 -> 16 -> 9 -> 17 -> 12 -> 13 -> 14 -> 20 -> 1 -> 2` cost=2707 (=ref, different tour)
- **Simple:** `2 -> 8 -> 4 -> 15 -> 5 -> 7 -> 6 -> 0 -> 11 -> 3 -> 10 -> 19 -> 18 -> 16 -> 9 -> 17 -> 12 -> 13 -> 14 -> 20 -> 1 -> 2` cost=2707 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=gr21 dimension=21 method=exact
[tsp-debug] exact solve started: vertices=21
[tsp-debug] initial incumbent: cost=2707
[tsp-debug] root: lower_bound=2252 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=2707 expanded=62716 created=125049 pruned_bound=62333 pruned_infeasible=384
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=gr21 dimension=21 method=exact
[tsp-debug] exact solve started: vertices=21
[tsp-debug] initial incumbent: cost=2707
[tsp-debug] root: lower_bound=2252 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=2707 expanded=39933 created=78937 pruned_bound=39004 pruned_infeasible=930
```
</details>

---

## 25. `data/classic/tsplib/ulysses22.tsp` (n=22)

**Concorde optimal cost:** 7013  
**Concorde time:** 79ms  

**Concorde (reference) tour:** `0 -> 13 -> 12 -> 11 -> 6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 18 -> 19 -> 20 -> 15 -> 2 -> 1 -> 16 -> 21 -> 3 -> 17 -> 7 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 7013 | ref | 79ms | - | - | - | - | 1 |
| Smart | TIMEOUT | - | 1.0h | - | - | - | - | - |
| Simple | TIMEOUT | - | 1.0h | - | - | - | - | - |

**Tours found:**
- **Concorde:** `0 -> 13 -> 12 -> 11 -> 6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 18 -> 19 -> 20 -> 15 -> 2 -> 1 -> 16 -> 21 -> 3 -> 17 -> 7 -> 0` cost=7013 (reference)

<details>
<summary>Concorde debug output</summary>

```
ERROR: No dual change in basis finding code
Did not find a basic optimal solution
Fractional matching routine failed
Warning: restarting running timer Miscellaneous
No warmstart, stumbling on anyway
```
</details>

---

## 26. `data/classic/tsplib/gr24.tsp` (n=24)

**Concorde optimal cost:** 1272  
**Concorde time:** 21ms  

**Concorde (reference) tour:** `0 -> 11 -> 3 -> 22 -> 8 -> 12 -> 13 -> 19 -> 1 -> 14 -> 18 -> 21 -> 17 -> 16 -> 9 -> 4 -> 20 -> 7 -> 23 -> 5 -> 6 -> 2 -> 10 -> 15 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 1272 | ref | 21ms | - | - | - | - | 1 |
| Smart | 1272 | :white_check_mark: | 15.0s | 1743379 | 872844 | 870535 | 2310 | - |
| Simple | 1272 | :white_check_mark: | 8.6s | 1093814 | 546921 | 546893 | 29 | - |

**Tours found:**
- **Concorde:** `0 -> 11 -> 3 -> 22 -> 8 -> 12 -> 13 -> 19 -> 1 -> 14 -> 18 -> 21 -> 17 -> 16 -> 9 -> 4 -> 20 -> 7 -> 23 -> 5 -> 6 -> 2 -> 10 -> 15 -> 0` cost=1272 (reference)
- **Smart:** `3 -> 11 -> 0 -> 15 -> 10 -> 2 -> 6 -> 5 -> 23 -> 7 -> 20 -> 4 -> 9 -> 16 -> 17 -> 21 -> 18 -> 14 -> 1 -> 19 -> 13 -> 12 -> 8 -> 22 -> 3` cost=1272 (=ref, different tour)
- **Simple:** `3 -> 11 -> 0 -> 15 -> 10 -> 2 -> 6 -> 5 -> 23 -> 7 -> 20 -> 4 -> 9 -> 16 -> 17 -> 21 -> 18 -> 14 -> 1 -> 19 -> 13 -> 12 -> 8 -> 22 -> 3` cost=1272 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=gr24 dimension=24 method=exact
[tsp-debug] exact solve started: vertices=24
[tsp-debug] initial incumbent: cost=1272
[tsp-debug] root: lower_bound=1081 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=1272 expanded=872844 created=1743379 pruned_bound=870535 pruned_infeasible=2310
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=gr24 dimension=24 method=exact
[tsp-debug] exact solve started: vertices=24
[tsp-debug] initial incumbent: cost=1272
[tsp-debug] root: lower_bound=1081 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=1272 expanded=546921 created=1093814 pruned_bound=546893 pruned_infeasible=29
```
</details>

---

## 27. `data/classic/tsplib/fri26.tsp` (n=26)

**Concorde optimal cost:** 937  
**Concorde time:** 25ms  

**Concorde (reference) tour:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 7 -> 8 -> 9 -> 13 -> 14 -> 12 -> 11 -> 10 -> 15 -> 18 -> 19 -> 17 -> 16 -> 20 -> 21 -> 25 -> 22 -> 23 -> 24 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 937 | ref | 25ms | - | - | - | - | 1 |
| Smart | 937 | :white_check_mark: | 27.9s | 2951175 | 1476757 | 1474418 | 2340 | - |
| Simple | 937 | :white_check_mark: | 37.9s | 5230237 | 2615118 | 2615119 | 0 | - |

**Tours found:**
- **Concorde:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 7 -> 8 -> 9 -> 13 -> 14 -> 12 -> 11 -> 10 -> 15 -> 18 -> 19 -> 17 -> 16 -> 20 -> 21 -> 25 -> 22 -> 23 -> 24 -> 0` cost=937 (reference)
- **Smart:** `16 -> 20 -> 21 -> 25 -> 22 -> 23 -> 24 -> 0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 7 -> 8 -> 9 -> 13 -> 14 -> 11 -> 12 -> 10 -> 15 -> 18 -> 19 -> 17 -> 16` cost=937 (=ref, different tour)
- **Simple:** `16 -> 20 -> 21 -> 25 -> 22 -> 23 -> 24 -> 0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 7 -> 8 -> 9 -> 13 -> 14 -> 11 -> 12 -> 10 -> 15 -> 18 -> 19 -> 17 -> 16` cost=937 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=fri26 dimension=26 method=exact
[tsp-debug] exact solve started: vertices=26
[tsp-debug] initial incumbent: cost=937
[tsp-debug] root: lower_bound=824 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=937 expanded=1476757 created=2951175 pruned_bound=1474418 pruned_infeasible=2340
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=fri26 dimension=26 method=exact
[tsp-debug] exact solve started: vertices=26
[tsp-debug] initial incumbent: cost=937
[tsp-debug] root: lower_bound=824 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=937 expanded=2615118 created=5230237 pruned_bound=2615119 pruned_infeasible=0
```
</details>

---

## 28. `data/classic/tsplib/bayg29.tsp` (n=29)

**Concorde optimal cost:** 1610  
**Concorde time:** 26ms  

**Concorde (reference) tour:** `0 -> 27 -> 5 -> 11 -> 8 -> 25 -> 2 -> 28 -> 4 -> 20 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 13 -> 16 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 7 -> 26 -> 15 -> 12 -> 23 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 1610 | ref | 26ms | - | - | - | - | 1 |
| Smart | 1610 | :white_check_mark: | 20.9m | 103461942 | 51731048 | 51730894 | 151 | - |
| Simple | 1610 | :white_check_mark: | 7.8m | 51171959 | 25602100 | 25569859 | 32240 | - |

**Tours found:**
- **Concorde:** `0 -> 27 -> 5 -> 11 -> 8 -> 25 -> 2 -> 28 -> 4 -> 20 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 13 -> 16 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 7 -> 26 -> 15 -> 12 -> 23 -> 0` cost=1610 (reference)
- **Smart:** `0 -> 23 -> 12 -> 15 -> 26 -> 7 -> 22 -> 6 -> 24 -> 18 -> 10 -> 21 -> 16 -> 13 -> 17 -> 14 -> 3 -> 9 -> 19 -> 1 -> 20 -> 4 -> 28 -> 2 -> 25 -> 8 -> 11 -> 5 -> 27 -> 0` cost=1610 (=ref, different tour)
- **Simple:** `0 -> 23 -> 12 -> 15 -> 26 -> 7 -> 22 -> 6 -> 24 -> 18 -> 10 -> 21 -> 16 -> 13 -> 17 -> 14 -> 3 -> 9 -> 19 -> 1 -> 20 -> 4 -> 28 -> 2 -> 25 -> 8 -> 11 -> 5 -> 27 -> 0` cost=1610 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=bayg29 dimension=29 method=exact
[tsp-debug] exact solve started: vertices=29
[tsp-debug] initial incumbent: cost=1618
[tsp-debug] root: lower_bound=1375 search=recursive-dfs strategy=smart
[tsp-debug] progress: expanded=5000000 created=9999985 depth=30 bound=1579 best=1618 pruned_bound=4999985 pruned_infeasible=6
[tsp-debug] progress: expanded=10000000 created=19999965 depth=36 bound=1615 best=1618 pruned_bound=9999965 pruned_infeasible=24
[tsp-debug] new incumbent: cost=1615 source=recursive-node depth=39
[tsp-debug] progress: expanded=15000000 created=29999930 depth=26 bound=1606 best=1615 pruned_bound=14999930 pruned_infeasible=54
[tsp-debug] progress: expanded=20000000 created=39999907 depth=29 bound=1609 best=1615 pruned_bound=19999907 pruned_infeasible=80
[tsp-debug] progress: expanded=25000000 created=49999901 depth=31 bound=1608 best=1615 pruned_bound=24999901 pruned_infeasible=87
[tsp-debug] progress: expanded=30000000 created=59999902 depth=23 bound=1569 best=1615 pruned_bound=29999902 pruned_infeasible=87
[tsp-debug] progress: expanded=35000000 created=69999903 depth=31 bound=1590 best=1615 pruned_bound=34999903 pruned_infeasible=87
[tsp-debug] progress: expanded=40000000 created=79999876 depth=31 bound=1606 best=1615 pruned_bound=39999876 pruned_infeasible=114
[tsp-debug] progress: expanded=45000000 created=89999851 depth=31 bound=1611 best=1615 pruned_bound=44999851 pruned_infeasible=136
[tsp-debug] new incumbent: cost=1610 source=recursive-node depth=33
[tsp-debug] progress: expanded=50000000 created=99999844 depth=32 bound=1585 best=1610 pruned_bound=49999844 pruned_infeasible=146
[tsp-debug] exact solve finished: feasible=yes cost=1610 expanded=51731048 created=103461942 pruned_bound=51730894 pruned_infeasible=151
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=bayg29 dimension=29 method=exact
[tsp-debug] exact solve started: vertices=29
[tsp-debug] initial incumbent: cost=1618
[tsp-debug] root: lower_bound=1375 search=recursive-dfs strategy=simple
[tsp-debug] progress: expanded=5000000 created=9985202 depth=31 bound=1606 best=1618 pruned_bound=4985202 pruned_infeasible=14779
[tsp-debug] new incumbent: cost=1610 source=recursive-node depth=28
[tsp-debug] progress: expanded=10000000 created=19976954 depth=31 bound=1605 best=1610 pruned_bound=9976954 pruned_infeasible=23028
[tsp-debug] progress: expanded=15000000 created=29973338 depth=27 bound=1534 best=1610 pruned_bound=14973338 pruned_infeasible=26641
[tsp-debug] progress: expanded=20000000 created=39968963 depth=28 bound=1605 best=1610 pruned_bound=19968963 pruned_infeasible=31021
[tsp-debug] progress: expanded=25000000 created=49967751 depth=29 bound=1599 best=1610 pruned_bound=24967751 pruned_infeasible=32227
[tsp-debug] exact solve finished: feasible=yes cost=1610 expanded=25602100 created=51171959 pruned_bound=25569859 pruned_infeasible=32240
```
</details>

---

## 29. `data/classic/tsplib/bays29.tsp` (n=29)

**Concorde optimal cost:** 2020  
**Concorde time:** 33ms  

**Concorde (reference) tour:** `0 -> 27 -> 5 -> 11 -> 8 -> 4 -> 25 -> 28 -> 2 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 16 -> 13 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 26 -> 7 -> 23 -> 15 -> 12 -> 20 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 2020 | ref | 33ms | - | - | - | - | 1 |
| Smart | TIMEOUT | - | 1.0h | - | - | - | - | - |
| Simple | 2020 | :white_check_mark: | 50.4m | 323233887 | 161957590 | 161276297 | 681294 | - |

**Tours found:**
- **Concorde:** `0 -> 27 -> 5 -> 11 -> 8 -> 4 -> 25 -> 28 -> 2 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 16 -> 13 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 26 -> 7 -> 23 -> 15 -> 12 -> 20 -> 0` cost=2020 (reference)
- **Simple:** `7 -> 26 -> 22 -> 6 -> 24 -> 18 -> 10 -> 21 -> 13 -> 16 -> 17 -> 14 -> 3 -> 9 -> 19 -> 1 -> 2 -> 28 -> 25 -> 4 -> 8 -> 11 -> 5 -> 27 -> 0 -> 20 -> 12 -> 15 -> 23 -> 7` cost=2020 (=ref, different tour)

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=bays29 dimension=29 method=exact
[tsp-debug] exact solve started: vertices=29
[tsp-debug] initial incumbent: cost=2020
[tsp-debug] root: lower_bound=1622 search=recursive-dfs strategy=simple
[tsp-debug] progress: expanded=5000000 created=9954250 depth=31 bound=2015 best=2020 pruned_bound=4954250 pruned_infeasible=45731
[tsp-debug] progress: expanded=10000000 created=19920178 depth=28 bound=2011 best=2020 pruned_bound=9920178 pruned_infeasible=79807
[tsp-debug] progress: expanded=15000000 created=29893909 depth=29 bound=2017 best=2020 pruned_bound=14893909 pruned_infeasible=106070
[tsp-debug] progress: expanded=20000000 created=39870923 depth=34 bound=1996 best=2020 pruned_bound=19870923 pruned_infeasible=129057
[tsp-debug] progress: expanded=25000000 created=49835588 depth=27 bound=1972 best=2020 pruned_bound=24835588 pruned_infeasible=164396
[tsp-debug] progress: expanded=30000000 created=59813460 depth=30 bound=2007 best=2020 pruned_bound=29813460 pruned_infeasible=186523
[tsp-debug] progress: expanded=35000000 created=69796878 depth=26 bound=1963 best=2020 pruned_bound=34796878 pruned_infeasible=203105
[tsp-debug] progress: expanded=40000000 created=79757725 depth=27 bound=1986 best=2020 pruned_bound=39757725 pruned_infeasible=242261
[tsp-debug] progress: expanded=45000000 created=89742955 depth=30 bound=1988 best=2020 pruned_bound=44742955 pruned_infeasible=257030
[tsp-debug] progress: expanded=50000000 created=99720839 depth=32 bound=1980 best=2020 pruned_bound=49720839 pruned_infeasible=279145
[tsp-debug] progress: expanded=55000000 created=109701350 depth=25 bound=2003 best=2020 pruned_bound=54701350 pruned_infeasible=298640
[tsp-debug] progress: expanded=60000000 created=119679550 depth=33 bound=2015 best=2020 pruned_bound=59679550 pruned_infeasible=320432
[tsp-debug] progress: expanded=65000000 created=129635842 depth=32 bound=1990 best=2020 pruned_bound=64635842 pruned_infeasible=364142
[tsp-debug] progress: expanded=70000000 created=139621887 depth=28 bound=1978 best=2020 pruned_bound=69621887 pruned_infeasible=378099
[tsp-debug] progress: expanded=75000000 created=149594512 depth=34 bound=2018 best=2020 pruned_bound=74594512 pruned_infeasible=405473
[tsp-debug] progress: expanded=80000000 created=159566782 depth=27 bound=1992 best=2020 pruned_bound=79566782 pruned_infeasible=433207
[tsp-debug] progress: expanded=85000000 created=169549526 depth=30 bound=2017 best=2020 pruned_bound=84549526 pruned_infeasible=450459
[tsp-debug] progress: expanded=90000000 created=179538305 depth=27 bound=1970 best=2020 pruned_bound=89538305 pruned_infeasible=461679
[tsp-debug] progress: expanded=95000000 created=189498450 depth=27 bound=2019 best=2020 pruned_bound=94498450 pruned_infeasible=501533
[tsp-debug] progress: expanded=100000000 created=199490241 depth=32 bound=2009 best=2020 pruned_bound=99490241 pruned_infeasible=509743
[tsp-debug] progress: expanded=105000000 created=209468575 depth=27 bound=1950 best=2020 pruned_bound=104468575 pruned_infeasible=531408
[tsp-debug] progress: expanded=110000000 created=219449735 depth=30 bound=2004 best=2020 pruned_bound=109449735 pruned_infeasible=550251
[tsp-debug] progress: expanded=115000000 created=229442871 depth=26 bound=2007 best=2020 pruned_bound=114442871 pruned_infeasible=557115
[tsp-debug] progress: expanded=120000000 created=239423684 depth=28 bound=1985 best=2020 pruned_bound=119423684 pruned_infeasible=576302
[tsp-debug] progress: expanded=125000000 created=249405890 depth=31 bound=2017 best=2020 pruned_bound=124405890 pruned_infeasible=594095
[tsp-debug] progress: expanded=130000000 created=259388219 depth=24 bound=1998 best=2020 pruned_bound=129388219 pruned_infeasible=611769
[tsp-debug] progress: expanded=135000000 created=269372718 depth=31 bound=2001 best=2020 pruned_bound=134372718 pruned_infeasible=627264
[tsp-debug] progress: expanded=140000000 created=279366014 depth=25 bound=2013 best=2020 pruned_bound=139366014 pruned_infeasible=633974
[tsp-debug] progress: expanded=145000000 created=289348601 depth=26 bound=1959 best=2020 pruned_bound=144348601 pruned_infeasible=651382
[tsp-debug] progress: expanded=150000000 created=299334208 depth=22 bound=1958 best=2020 pruned_bound=149334208 pruned_infeasible=665780
[tsp-debug] progress: expanded=155000000 created=309327555 depth=28 bound=2013 best=2020 pruned_bound=154327555 pruned_infeasible=672430
[tsp-debug] progress: expanded=160000000 created=319319275 depth=28 bound=2018 best=2020 pruned_bound=159319275 pruned_infeasible=680712
[tsp-debug] exact solve finished: feasible=yes cost=2020 expanded=161957590 created=323233887 pruned_bound=161276297 pruned_infeasible=681294
```
</details>

---

## 30. `data/classic/national/wi29.tsp` (n=29)

**Concorde optimal cost:** 27603  
**Concorde time:** 39ms  

**Concorde (reference) tour:** `0 -> 4 -> 7 -> 3 -> 2 -> 6 -> 8 -> 12 -> 13 -> 15 -> 23 -> 26 -> 24 -> 19 -> 25 -> 27 -> 28 -> 22 -> 21 -> 20 -> 16 -> 17 -> 18 -> 14 -> 11 -> 10 -> 9 -> 5 -> 1 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 27603 | ref | 39ms | - | - | - | - | 1 |
| Smart | 27603 | :white_check_mark: | 1.8s | 133749 | 66874 | 66875 | 0 | - |
| Simple | 27603 | :white_check_mark: | 7.3m | 49241904 | 25462772 | 23779132 | 1683641 | - |

**Tours found:**
- **Concorde:** `0 -> 4 -> 7 -> 3 -> 2 -> 6 -> 8 -> 12 -> 13 -> 15 -> 23 -> 26 -> 24 -> 19 -> 25 -> 27 -> 28 -> 22 -> 21 -> 20 -> 16 -> 17 -> 18 -> 14 -> 11 -> 10 -> 9 -> 5 -> 1 -> 0` cost=27603 (reference)
- **Smart:** `21 -> 22 -> 28 -> 27 -> 25 -> 19 -> 24 -> 26 -> 23 -> 15 -> 13 -> 12 -> 8 -> 6 -> 2 -> 3 -> 7 -> 4 -> 0 -> 1 -> 5 -> 9 -> 10 -> 11 -> 14 -> 18 -> 17 -> 16 -> 20 -> 21` cost=27603 (=ref, different tour)
- **Simple:** `21 -> 22 -> 28 -> 27 -> 25 -> 19 -> 24 -> 26 -> 23 -> 15 -> 13 -> 12 -> 8 -> 6 -> 2 -> 3 -> 7 -> 4 -> 0 -> 1 -> 5 -> 9 -> 10 -> 11 -> 14 -> 18 -> 17 -> 16 -> 20 -> 21` cost=27603 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=wi29 dimension=29 method=exact
[tsp-debug] exact solve started: vertices=29
[tsp-debug] initial incumbent: cost=27603
[tsp-debug] root: lower_bound=22727 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=27603 expanded=66874 created=133749 pruned_bound=66875 pruned_infeasible=0
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=wi29 dimension=29 method=exact
[tsp-debug] exact solve started: vertices=29
[tsp-debug] initial incumbent: cost=27603
[tsp-debug] root: lower_bound=22727 search=recursive-dfs strategy=simple
[tsp-debug] progress: expanded=5000000 created=9947936 depth=31 bound=27548 best=27603 pruned_bound=4947936 pruned_infeasible=52044
[tsp-debug] progress: expanded=10000000 created=19809917 depth=27 bound=27590 best=27603 pruned_bound=9809917 pruned_infeasible=190069
[tsp-debug] progress: expanded=15000000 created=29622420 depth=35 bound=27418 best=27603 pruned_bound=14622420 pruned_infeasible=377556
[tsp-debug] progress: expanded=20000000 created=38959687 depth=28 bound=27586 best=27603 pruned_bound=18959687 pruned_infeasible=1040292
[tsp-debug] progress: expanded=25000000 created=48410481 depth=28 bound=27352 best=27603 pruned_bound=23410481 pruned_infeasible=1589506
[tsp-debug] exact solve finished: feasible=yes cost=27603 expanded=25462772 created=49241904 pruned_bound=23779132 pruned_infeasible=1683641
```
</details>

---

## 31. `data/classic/national/dj38.tsp` (n=38)

**Concorde optimal cost:** 6656  
**Concorde time:** 35ms  

**Concorde (reference) tour:** `0 -> 9 -> 13 -> 20 -> 28 -> 29 -> 31 -> 34 -> 36 -> 37 -> 32 -> 33 -> 35 -> 30 -> 26 -> 27 -> 23 -> 21 -> 24 -> 25 -> 22 -> 19 -> 14 -> 12 -> 15 -> 16 -> 17 -> 18 -> 10 -> 11 -> 8 -> 7 -> 6 -> 5 -> 4 -> 2 -> 3 -> 1 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 6656 | ref | 35ms | - | - | - | - | 1 |
| Smart | 6656 | :white_check_mark: | 2.5s | 104213 | 52106 | 52107 | 0 | - |
| Simple | 6656 | :white_check_mark: | 44.4s | 2611687 | 1305843 | 1305844 | 0 | - |

**Tours found:**
- **Concorde:** `0 -> 9 -> 13 -> 20 -> 28 -> 29 -> 31 -> 34 -> 36 -> 37 -> 32 -> 33 -> 35 -> 30 -> 26 -> 27 -> 23 -> 21 -> 24 -> 25 -> 22 -> 19 -> 14 -> 12 -> 15 -> 16 -> 17 -> 18 -> 10 -> 11 -> 8 -> 7 -> 6 -> 5 -> 4 -> 2 -> 3 -> 1 -> 0` cost=6656 (reference)
- **Smart:** `1 -> 0 -> 9 -> 13 -> 20 -> 28 -> 29 -> 31 -> 34 -> 36 -> 37 -> 32 -> 33 -> 35 -> 30 -> 26 -> 27 -> 23 -> 21 -> 24 -> 25 -> 22 -> 19 -> 14 -> 12 -> 15 -> 16 -> 17 -> 18 -> 10 -> 11 -> 8 -> 7 -> 6 -> 5 -> 4 -> 2 -> 3 -> 1` cost=6656 (=ref, different tour)
- **Simple:** `1 -> 0 -> 9 -> 13 -> 20 -> 28 -> 29 -> 31 -> 34 -> 36 -> 37 -> 32 -> 33 -> 35 -> 30 -> 26 -> 27 -> 23 -> 21 -> 24 -> 25 -> 22 -> 19 -> 14 -> 12 -> 15 -> 16 -> 17 -> 18 -> 10 -> 11 -> 8 -> 7 -> 6 -> 5 -> 4 -> 2 -> 3 -> 1` cost=6656 (=ref, different tour)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=dj38 dimension=38 method=exact
[tsp-debug] exact solve started: vertices=38
[tsp-debug] initial incumbent: cost=6656
[tsp-debug] root: lower_bound=6285 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=6656 expanded=52106 created=104213 pruned_bound=52107 pruned_infeasible=0
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=dj38 dimension=38 method=exact
[tsp-debug] exact solve started: vertices=38
[tsp-debug] initial incumbent: cost=6656
[tsp-debug] root: lower_bound=6285 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=6656 expanded=1305843 created=2611687 pruned_bound=1305844 pruned_infeasible=0
```
</details>

---

## 32. `data/classic/tsplib/dantzig42.tsp` (n=42)

**Concorde optimal cost:** 699  
**Concorde time:** 40ms  

**Concorde (reference) tour:** `0 -> 41 -> 40 -> 39 -> 38 -> 37 -> 36 -> 35 -> 34 -> 33 -> 32 -> 31 -> 30 -> 29 -> 28 -> 27 -> 26 -> 25 -> 24 -> 23 -> 22 -> 21 -> 20 -> 19 -> 18 -> 17 -> 16 -> 15 -> 14 -> 13 -> 12 -> 11 -> 10 -> 9 -> 8 -> 7 -> 6 -> 5 -> 4 -> 3 -> 2 -> 1 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 699 | ref | 40ms | - | - | - | - | 1 |
| Smart | TIMEOUT | - | 1.0h | - | - | - | - | - |
| Simple | TIMEOUT | - | 1.0h | - | - | - | - | - |

**Tours found:**
- **Concorde:** `0 -> 41 -> 40 -> 39 -> 38 -> 37 -> 36 -> 35 -> 34 -> 33 -> 32 -> 31 -> 30 -> 29 -> 28 -> 27 -> 26 -> 25 -> 24 -> 23 -> 22 -> 21 -> 20 -> 19 -> 18 -> 17 -> 16 -> 15 -> 14 -> 13 -> 12 -> 11 -> 10 -> 9 -> 8 -> 7 -> 6 -> 5 -> 4 -> 3 -> 2 -> 1 -> 0` cost=699 (reference)

---

## 33. `data/classic/tsplib/swiss42.tsp` (n=42)

**Concorde optimal cost:** 1273  
**Concorde time:** 30ms  

**Concorde (reference) tour:** `0 -> 1 -> 6 -> 4 -> 3 -> 2 -> 27 -> 28 -> 29 -> 30 -> 38 -> 22 -> 39 -> 21 -> 24 -> 40 -> 23 -> 41 -> 9 -> 8 -> 10 -> 25 -> 11 -> 12 -> 18 -> 26 -> 5 -> 13 -> 19 -> 14 -> 16 -> 15 -> 37 -> 7 -> 17 -> 31 -> 36 -> 35 -> 20 -> 33 -> 34 -> 32 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 1273 | ref | 30ms | - | - | - | - | 1 |
| Smart | TIMEOUT | - | 1.0h | - | - | - | - | - |
| Simple | TIMEOUT | - | 1.0h | - | - | - | - | - |

**Tours found:**
- **Concorde:** `0 -> 1 -> 6 -> 4 -> 3 -> 2 -> 27 -> 28 -> 29 -> 30 -> 38 -> 22 -> 39 -> 21 -> 24 -> 40 -> 23 -> 41 -> 9 -> 8 -> 10 -> 25 -> 11 -> 12 -> 18 -> 26 -> 5 -> 13 -> 19 -> 14 -> 16 -> 15 -> 37 -> 7 -> 17 -> 31 -> 36 -> 35 -> 20 -> 33 -> 34 -> 32 -> 0` cost=1273 (reference)

---

## 34. `data/classic/tsplib/att48.tsp` (n=48)

**Concorde optimal cost:** 10628  
**Concorde time:** 80ms  

**Concorde (reference) tour:** `0 -> 8 -> 39 -> 14 -> 11 -> 10 -> 12 -> 24 -> 13 -> 22 -> 2 -> 21 -> 15 -> 40 -> 33 -> 28 -> 1 -> 25 -> 3 -> 34 -> 44 -> 9 -> 23 -> 41 -> 4 -> 47 -> 38 -> 31 -> 20 -> 46 -> 19 -> 32 -> 45 -> 35 -> 29 -> 42 -> 16 -> 26 -> 18 -> 36 -> 5 -> 27 -> 6 -> 17 -> 43 -> 30 -> 37 -> 7 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 10628 | ref | 80ms | - | - | - | - | 1 |
| Smart | TIMEOUT | - | 1.0h | - | - | - | - | - |
| Simple | TIMEOUT | - | 1.0h | - | - | - | - | - |

**Tours found:**
- **Concorde:** `0 -> 8 -> 39 -> 14 -> 11 -> 10 -> 12 -> 24 -> 13 -> 22 -> 2 -> 21 -> 15 -> 40 -> 33 -> 28 -> 1 -> 25 -> 3 -> 34 -> 44 -> 9 -> 23 -> 41 -> 4 -> 47 -> 38 -> 31 -> 20 -> 46 -> 19 -> 32 -> 45 -> 35 -> 29 -> 42 -> 16 -> 26 -> 18 -> 36 -> 5 -> 27 -> 6 -> 17 -> 43 -> 30 -> 37 -> 7 -> 0` cost=10628 (reference)

---

## 35. `data/classic/tsplib/gr48.tsp` (n=48)

**Concorde optimal cost:** 5046  
**Concorde time:** 71ms  

**Concorde (reference) tour:** `0 -> 12 -> 47 -> 15 -> 10 -> 35 -> 25 -> 5 -> 13 -> 8 -> 31 -> 26 -> 16 -> 20 -> 21 -> 7 -> 32 -> 4 -> 30 -> 11 -> 9 -> 14 -> 23 -> 36 -> 46 -> 42 -> 44 -> 1 -> 39 -> 38 -> 41 -> 34 -> 19 -> 37 -> 29 -> 3 -> 18 -> 2 -> 24 -> 22 -> 33 -> 17 -> 45 -> 40 -> 43 -> 27 -> 6 -> 28 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 5046 | ref | 71ms | - | - | - | - | 1 |
| Smart | TIMEOUT | - | 1.0h | - | - | - | - | - |
| Simple | TIMEOUT | - | 1.0h | - | - | - | - | - |

**Tours found:**
- **Concorde:** `0 -> 12 -> 47 -> 15 -> 10 -> 35 -> 25 -> 5 -> 13 -> 8 -> 31 -> 26 -> 16 -> 20 -> 21 -> 7 -> 32 -> 4 -> 30 -> 11 -> 9 -> 14 -> 23 -> 36 -> 46 -> 42 -> 44 -> 1 -> 39 -> 38 -> 41 -> 34 -> 19 -> 37 -> 29 -> 3 -> 18 -> 2 -> 24 -> 22 -> 33 -> 17 -> 45 -> 40 -> 43 -> 27 -> 6 -> 28 -> 0` cost=5046 (reference)

---

## 36. `data/classic/tsplib/hk48.tsp` (n=48)

**Concorde optimal cost:** 11461  
**Concorde time:** 39ms  

**Concorde (reference) tour:** `0 -> 1 -> 47 -> 14 -> 42 -> 20 -> 32 -> 29 -> 22 -> 8 -> 9 -> 39 -> 35 -> 33 -> 5 -> 7 -> 46 -> 6 -> 37 -> 13 -> 17 -> 11 -> 21 -> 12 -> 27 -> 31 -> 24 -> 2 -> 4 -> 28 -> 25 -> 40 -> 23 -> 34 -> 16 -> 30 -> 19 -> 10 -> 15 -> 41 -> 3 -> 45 -> 44 -> 38 -> 43 -> 26 -> 36 -> 18 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 11461 | ref | 39ms | - | - | - | - | 1 |
| Smart | TIMEOUT | - | 1.0h | - | - | - | - | - |
| Simple | TIMEOUT | - | 1.0h | - | - | - | - | - |

**Tours found:**
- **Concorde:** `0 -> 1 -> 47 -> 14 -> 42 -> 20 -> 32 -> 29 -> 22 -> 8 -> 9 -> 39 -> 35 -> 33 -> 5 -> 7 -> 46 -> 6 -> 37 -> 13 -> 17 -> 11 -> 21 -> 12 -> 27 -> 31 -> 24 -> 2 -> 4 -> 28 -> 25 -> 40 -> 23 -> 34 -> 16 -> 30 -> 19 -> 10 -> 15 -> 41 -> 3 -> 45 -> 44 -> 38 -> 43 -> 26 -> 36 -> 18 -> 0` cost=11461 (reference)

---

## 37. `data/classic/tsplib/eil51.tsp` (n=51)

**Concorde optimal cost:** 426  
**Concorde time:** 56ms  

**Concorde (reference) tour:** `0 -> 21 -> 7 -> 25 -> 30 -> 27 -> 2 -> 35 -> 34 -> 19 -> 1 -> 28 -> 20 -> 15 -> 49 -> 33 -> 29 -> 8 -> 48 -> 9 -> 38 -> 32 -> 44 -> 14 -> 43 -> 41 -> 39 -> 18 -> 40 -> 12 -> 24 -> 13 -> 23 -> 42 -> 6 -> 22 -> 47 -> 5 -> 26 -> 50 -> 45 -> 11 -> 46 -> 17 -> 3 -> 16 -> 36 -> 4 -> 37 -> 10 -> 31 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 426 | ref | 56ms | - | - | - | - | 1 |
| Smart | TIMEOUT | - | 1.0h | - | - | - | - | - |
| Simple | TIMEOUT | - | 1.0h | - | - | - | - | - |

**Tours found:**
- **Concorde:** `0 -> 21 -> 7 -> 25 -> 30 -> 27 -> 2 -> 35 -> 34 -> 19 -> 1 -> 28 -> 20 -> 15 -> 49 -> 33 -> 29 -> 8 -> 48 -> 9 -> 38 -> 32 -> 44 -> 14 -> 43 -> 41 -> 39 -> 18 -> 40 -> 12 -> 24 -> 13 -> 23 -> 42 -> 6 -> 22 -> 47 -> 5 -> 26 -> 50 -> 45 -> 11 -> 46 -> 17 -> 3 -> 16 -> 36 -> 4 -> 37 -> 10 -> 31 -> 0` cost=426 (reference)

---

## 38. `data/classic/tsplib/berlin52.tsp` (n=52)

**Concorde optimal cost:** 7542  
**Concorde time:** 55ms  

**Concorde (reference) tour:** `0 -> 48 -> 31 -> 44 -> 18 -> 40 -> 7 -> 8 -> 9 -> 42 -> 32 -> 50 -> 10 -> 51 -> 13 -> 12 -> 46 -> 25 -> 26 -> 27 -> 11 -> 24 -> 3 -> 5 -> 14 -> 4 -> 23 -> 47 -> 37 -> 36 -> 39 -> 38 -> 35 -> 34 -> 33 -> 43 -> 45 -> 15 -> 28 -> 49 -> 19 -> 22 -> 29 -> 1 -> 6 -> 41 -> 20 -> 16 -> 2 -> 17 -> 30 -> 21 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 7542 | ref | 55ms | - | - | - | - | 1 |
| Smart | TIMEOUT | - | 1.0h | - | - | - | - | - |
| Simple | TIMEOUT | - | 1.0h | - | - | - | - | - |

**Tours found:**
- **Concorde:** `0 -> 48 -> 31 -> 44 -> 18 -> 40 -> 7 -> 8 -> 9 -> 42 -> 32 -> 50 -> 10 -> 51 -> 13 -> 12 -> 46 -> 25 -> 26 -> 27 -> 11 -> 24 -> 3 -> 5 -> 14 -> 4 -> 23 -> 47 -> 37 -> 36 -> 39 -> 38 -> 35 -> 34 -> 33 -> 43 -> 45 -> 15 -> 28 -> 49 -> 19 -> 22 -> 29 -> 1 -> 6 -> 41 -> 20 -> 16 -> 2 -> 17 -> 30 -> 21 -> 0` cost=7542 (reference)

---
