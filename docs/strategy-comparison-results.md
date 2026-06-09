# TSP Algorithm Comparison

## Algorithms Compared

| Algorithm | Description |
|---|---|
| **Concorde** | State-of-the-art Concorde TSP solver (exact, with QSopt LP) — **reference** |
| **Smart** | Branch & Bound with smart branching (1-tree degree + edge candidate) |
| **Simple** | Branch & Bound with simple branching (max-degree vertex) |

**Instances:** 36 from `examples` (n <= 30), `data/classic/tsplib` (n <= 49), `data/classic/national` (n <= 49)  
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
| Simple | 45 | :white_check_mark: | 4ms | 5 | 3 | 2 | 2 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=45 expanded=3 created=5 pruned_bound=2 pruned_infeasible=2
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
| Simple | 102 | :white_check_mark: | 3ms | 1 | 0 | 1 | 0 | - |

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
| Smart | 26 | :white_check_mark: | 3ms | 20 | 10 | 10 | 1 | - |
| Simple | 26 | :white_check_mark: | 2ms | 15 | 10 | 5 | 6 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=26 expanded=10 created=15 pruned_bound=5 pruned_infeasible=6
```
</details>

---

## 4. `examples/five-city.txt` (n=5)

**Concorde optimal cost:** 26  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 2 -> 3 -> 1 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 26 | ref | 2ms | - | - | - | - | - |
| Smart | 26 | :white_check_mark: | 3ms | 20 | 10 | 10 | 1 | - |
| Simple | 26 | :white_check_mark: | 1ms | 15 | 10 | 5 | 6 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=26 expanded=10 created=15 pruned_bound=5 pruned_infeasible=6
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
| Smart | 8 | :white_check_mark: | 4ms | 11 | 5 | 6 | 0 | - |
| Simple | 8 | :white_check_mark: | 3ms | 20 | 13 | 7 | 7 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=8 expanded=13 created=20 pruned_bound=7 pruned_infeasible=7
```
</details>

---

## 6. `examples/converted/five-node-explicit.txt` (n=5)

**Concorde optimal cost:** 26  
**Concorde time:** 1ms  

**Concorde (reference) tour:** `0 -> 4 -> 2 -> 3 -> 1 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 26 | ref | 1ms | - | - | - | - | - |
| Smart | 26 | :white_check_mark: | 3ms | 20 | 10 | 10 | 1 | - |
| Simple | 26 | :white_check_mark: | 3ms | 15 | 10 | 5 | 6 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=26 expanded=10 created=15 pruned_bound=5 pruned_infeasible=6
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
| Smart | 110 | :white_check_mark: | 1ms | 5 | 2 | 3 | 0 | - |
| Simple | 110 | :white_check_mark: | 3ms | 5 | 3 | 2 | 2 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=110 expanded=3 created=5 pruned_bound=2 pruned_infeasible=2
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
| Smart | 136 | :white_check_mark: | 3ms | 9 | 4 | 5 | 0 | - |
| Simple | 136 | :white_check_mark: | 4ms | 14 | 8 | 6 | 3 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=136 expanded=8 created=14 pruned_bound=6 pruned_infeasible=3
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
| Smart | 88 | :white_check_mark: | 3ms | 9 | 4 | 5 | 0 | - |
| Simple | 88 | :white_check_mark: | 4ms | 12 | 7 | 5 | 3 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=88 expanded=7 created=12 pruned_bound=5 pruned_infeasible=3
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
| Smart | 51 | :white_check_mark: | 4ms | 9 | 4 | 5 | 0 | - |
| Simple | 51 | :white_check_mark: | 3ms | 10 | 5 | 5 | 1 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=51 expanded=5 created=10 pruned_bound=5 pruned_infeasible=1
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
| Smart | 115 | :white_check_mark: | 3ms | 1 | 0 | 1 | 0 | - |
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
| Smart | 100 | :white_check_mark: | 4ms | 25 | 17 | 8 | 10 | - |
| Simple | 100 | :white_check_mark: | 3ms | 29 | 20 | 9 | 12 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=100 expanded=17 created=25 pruned_bound=8 pruned_infeasible=10
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=6 method=exact
[tsp-debug] exact solve started: vertices=6
[tsp-debug] initial incumbent: cost=100
[tsp-debug] root: lower_bound=53 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=100 expanded=20 created=29 pruned_bound=9 pruned_infeasible=12
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
| Smart | 196 | :white_check_mark: | 3ms | 16 | 10 | 6 | 5 | - |
| Simple | 196 | :white_check_mark: | 2ms | 20 | 15 | 5 | 11 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=196 expanded=15 created=20 pruned_bound=5 pruned_infeasible=11
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
| Smart | 86 | :white_check_mark: | 1ms | 7 | 4 | 3 | 2 | - |
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
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 3 -> 6 -> 5 -> 1 -> 4 -> 2 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 74 | ref | 2ms | - | - | - | - | - |
| Smart | 74 | :white_check_mark: | 1ms | 33 | 16 | 17 | 0 | - |
| Simple | 74 | :white_check_mark: | 3ms | 82 | 47 | 35 | 13 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=74 expanded=47 created=82 pruned_bound=35 pruned_infeasible=13
```
</details>

---

## 16. `examples/random/sparse/rnd-06-sparse-n7.txt` (n=7)

**Concorde optimal cost:** 199  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 4 -> 1 -> 5 -> 3 -> 2 -> 6 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 199 | ref | 2ms | - | - | - | - | - |
| Smart | 199 | :white_check_mark: | 3ms | 30 | 21 | 9 | 13 | - |
| Simple | 199 | :white_check_mark: | 3ms | 21 | 17 | 4 | 14 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=199 expanded=21 created=30 pruned_bound=9 pruned_infeasible=13
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=199
[tsp-debug] root: lower_bound=169 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=199 expanded=17 created=21 pruned_bound=4 pruned_infeasible=14
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
| Smart | 73 | :white_check_mark: | 4ms | 27 | 13 | 14 | 0 | - |
| Simple | 73 | :white_check_mark: | 4ms | 54 | 31 | 23 | 9 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=73 expanded=31 created=54 pruned_bound=23 pruned_infeasible=9
```
</details>

---

## 18. `examples/random/complete/rnd-10-complete-n7.txt` (n=7)

**Concorde optimal cost:** 49  
**Concorde time:** 2ms  

**Concorde (reference) tour:** `0 -> 6 -> 4 -> 5 -> 3 -> 2 -> 1 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 49 | ref | 2ms | - | - | - | - | - |
| Smart | 49 | :white_check_mark: | 4ms | 15 | 7 | 8 | 0 | - |
| Simple | 49 | :white_check_mark: | 4ms | 58 | 33 | 25 | 9 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=49 expanded=33 created=58 pruned_bound=25 pruned_infeasible=9
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
| Simple | 64 | :white_check_mark: | 4ms | 97 | 57 | 40 | 18 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=64 expanded=57 created=97 pruned_bound=40 pruned_infeasible=18
```
</details>

---

## 20. `examples/random/complete/rnd-09-complete-n8.txt` (n=8)

**Concorde optimal cost:** 93  
**Concorde time:** 3ms  

**Concorde (reference) tour:** `0 -> 5 -> 4 -> 6 -> 1 -> 3 -> 2 -> 7 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 93 | ref | 3ms | - | - | - | - | - |
| Smart | 93 | :white_check_mark: | 4ms | 116 | 58 | 58 | 1 | - |
| Simple | 93 | :white_check_mark: | 7ms | 634 | 371 | 263 | 109 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=93 expanded=371 created=634 pruned_bound=263 pruned_infeasible=109
```
</details>

---

## 21. `data/classic/tsplib/burma14.tsp` (n=14)

**Concorde optimal cost:** 3323  
**Concorde time:** 20ms  

**Concorde (reference) tour:** `0 -> 1 -> 13 -> 2 -> 3 -> 4 -> 5 -> 11 -> 6 -> 12 -> 7 -> 10 -> 8 -> 9 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 3323 | ref | 20ms | - | - | - | - | 1 |
| Smart | 3323 | :white_check_mark: | 444ms | 92069 | 46037 | 46032 | 6 | - |
| Simple | 3323 | :white_check_mark: | 380ms | 128002 | 79039 | 48963 | 30077 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=3323 expanded=79039 created=128002 pruned_bound=48963 pruned_infeasible=30077
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
**Concorde time:** 42ms  

**Concorde (reference) tour:** `0 -> 13 -> 12 -> 11 -> 6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 15 -> 2 -> 1 -> 3 -> 7 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 6859 | ref | 42ms | - | - | - | - | 1 |
| Smart | 6859 | :white_check_mark: | 3.5m | 43911332 | 21957243 | 21954089 | 3155 | - |
| Simple | 6859 | :white_check_mark: | 19.4s | 6105092 | 3798019 | 2307073 | 1490947 | - |

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
[tsp-debug] progress: expanded=100000 created=199972 depth=30 bound=6409 best=6859 pruned_bound=99949 pruned_infeasible=27
[tsp-debug] progress: expanded=200000 created=399957 depth=35 bound=6730 best=6859 pruned_bound=199929 pruned_infeasible=42
[tsp-debug] progress: expanded=300000 created=599909 depth=46 bound=6674 best=6859 pruned_bound=299875 pruned_infeasible=90
[tsp-debug] progress: expanded=400000 created=799892 depth=37 bound=6707 best=6859 pruned_bound=399869 pruned_infeasible=107
[tsp-debug] progress: expanded=500000 created=999873 depth=34 bound=6317 best=6859 pruned_bound=499845 pruned_infeasible=126
[tsp-debug] progress: expanded=600000 created=1199864 depth=31 bound=6510 best=6859 pruned_bound=599840 pruned_infeasible=135
[tsp-debug] progress: expanded=700000 created=1399848 depth=29 bound=6823 best=6859 pruned_bound=699827 pruned_infeasible=151
[tsp-debug] progress: expanded=800000 created=1599829 depth=35 bound=6789 best=6859 pruned_bound=799800 pruned_infeasible=170
[tsp-debug] progress: expanded=900000 created=1799813 depth=35 bound=6415 best=6859 pruned_bound=899788 pruned_infeasible=186
[tsp-debug] progress: expanded=1000000 created=1999760 depth=35 bound=6185 best=6859 pruned_bound=999735 pruned_infeasible=239
[tsp-debug] progress: expanded=1100000 created=2199741 depth=28 bound=6850 best=6859 pruned_bound=1099722 pruned_infeasible=258
[tsp-debug] progress: expanded=1200000 created=2399740 depth=44 bound=6713 best=6859 pruned_bound=1199713 pruned_infeasible=259
[tsp-debug] progress: expanded=1300000 created=2599733 depth=41 bound=6716 best=6859 pruned_bound=1299706 pruned_infeasible=266
[tsp-debug] progress: expanded=1400000 created=2799717 depth=31 bound=6596 best=6859 pruned_bound=1399694 pruned_infeasible=282
[tsp-debug] progress: expanded=1500000 created=2999689 depth=36 bound=6702 best=6859 pruned_bound=1499661 pruned_infeasible=310
[tsp-debug] progress: expanded=1600000 created=3199645 depth=20 bound=6478 best=6859 pruned_bound=1599633 pruned_infeasible=354
[tsp-debug] progress: expanded=1700000 created=3399610 depth=31 bound=6835 best=6859 pruned_bound=1699590 pruned_infeasible=389
[tsp-debug] progress: expanded=1800000 created=3599453 depth=36 bound=6667 best=6859 pruned_bound=1799430 pruned_infeasible=546
[tsp-debug] progress: expanded=1900000 created=3799353 depth=27 bound=6393 best=6859 pruned_bound=1899335 pruned_infeasible=646
[tsp-debug] progress: expanded=2000000 created=3999337 depth=35 bound=6624 best=6859 pruned_bound=1999313 pruned_infeasible=662
[tsp-debug] progress: expanded=2100000 created=4199305 depth=47 bound=6477 best=6859 pruned_bound=2099272 pruned_infeasible=694
[tsp-debug] progress: expanded=2200000 created=4399285 depth=43 bound=6631 best=6859 pruned_bound=2199259 pruned_infeasible=714
[tsp-debug] progress: expanded=2300000 created=4599253 depth=35 bound=6725 best=6859 pruned_bound=2299229 pruned_infeasible=746
[tsp-debug] progress: expanded=2400000 created=4799226 depth=28 bound=6663 best=6859 pruned_bound=2399208 pruned_infeasible=773
[tsp-debug] progress: expanded=2500000 created=4999186 depth=33 bound=6364 best=6859 pruned_bound=2499168 pruned_infeasible=813
[tsp-debug] progress: expanded=2600000 created=5199181 depth=27 bound=6631 best=6859 pruned_bound=2599167 pruned_infeasible=818
[tsp-debug] progress: expanded=2700000 created=5399181 depth=28 bound=6561 best=6859 pruned_bound=2699163 pruned_infeasible=818
[tsp-debug] progress: expanded=2800000 created=5599149 depth=30 bound=6596 best=6859 pruned_bound=2799128 pruned_infeasible=850
[tsp-debug] progress: expanded=2900000 created=5799149 depth=20 bound=6480 best=6859 pruned_bound=2899138 pruned_infeasible=850
[tsp-debug] progress: expanded=3000000 created=5999098 depth=46 bound=6784 best=6859 pruned_bound=2999062 pruned_infeasible=901
[tsp-debug] progress: expanded=3100000 created=6199086 depth=35 bound=6411 best=6859 pruned_bound=3099064 pruned_infeasible=913
[tsp-debug] progress: expanded=3200000 created=6399086 depth=38 bound=6679 best=6859 pruned_bound=3199059 pruned_infeasible=913
[tsp-debug] progress: expanded=3300000 created=6599086 depth=38 bound=6745 best=6859 pruned_bound=3299065 pruned_infeasible=913
[tsp-debug] progress: expanded=3400000 created=6799086 depth=38 bound=6829 best=6859 pruned_bound=3399071 pruned_infeasible=913
[tsp-debug] progress: expanded=3500000 created=6999084 depth=41 bound=6825 best=6859 pruned_bound=3499061 pruned_infeasible=915
[tsp-debug] progress: expanded=3600000 created=7199083 depth=26 bound=6629 best=6859 pruned_bound=3599064 pruned_infeasible=916
[tsp-debug] progress: expanded=3700000 created=7399073 depth=39 bound=6694 best=6859 pruned_bound=3699047 pruned_infeasible=926
[tsp-debug] progress: expanded=3800000 created=7599062 depth=33 bound=6728 best=6859 pruned_bound=3799038 pruned_infeasible=937
[tsp-debug] progress: expanded=3900000 created=7799016 depth=30 bound=6634 best=6859 pruned_bound=3898996 pruned_infeasible=983
[tsp-debug] progress: expanded=4000000 created=7998997 depth=32 bound=6841 best=6859 pruned_bound=3998978 pruned_infeasible=1002
[tsp-debug] progress: expanded=4100000 created=8198991 depth=35 bound=6624 best=6859 pruned_bound=4098972 pruned_infeasible=1008
[tsp-debug] progress: expanded=4200000 created=8398986 depth=25 bound=6693 best=6859 pruned_bound=4198968 pruned_infeasible=1013
[tsp-debug] progress: expanded=4300000 created=8598977 depth=36 bound=6670 best=6859 pruned_bound=4298951 pruned_infeasible=1022
[tsp-debug] progress: expanded=4400000 created=8798920 depth=50 bound=6852 best=6859 pruned_bound=4398891 pruned_infeasible=1079
[tsp-debug] progress: expanded=4500000 created=8998893 depth=29 bound=6639 best=6859 pruned_bound=4498871 pruned_infeasible=1106
[tsp-debug] progress: expanded=4600000 created=9198874 depth=32 bound=6780 best=6859 pruned_bound=4598856 pruned_infeasible=1125
[tsp-debug] progress: expanded=4700000 created=9398868 depth=26 bound=6745 best=6859 pruned_bound=4698850 pruned_infeasible=1131
[tsp-debug] progress: expanded=4800000 created=9598852 depth=24 bound=6576 best=6859 pruned_bound=4798835 pruned_infeasible=1147
[tsp-debug] progress: expanded=4900000 created=9798845 depth=35 bound=6715 best=6859 pruned_bound=4898817 pruned_infeasible=1154
[tsp-debug] progress: expanded=5000000 created=9998835 depth=21 bound=6713 best=6859 pruned_bound=4998822 pruned_infeasible=1164
[tsp-debug] progress: expanded=5100000 created=10198787 depth=29 bound=6852 best=6859 pruned_bound=5098766 pruned_infeasible=1212
[tsp-debug] progress: expanded=5200000 created=10398763 depth=42 bound=6710 best=6859 pruned_bound=5198736 pruned_infeasible=1236
[tsp-debug] progress: expanded=5300000 created=10598756 depth=20 bound=6026 best=6859 pruned_bound=5298744 pruned_infeasible=1243
[tsp-debug] progress: expanded=5400000 created=10798743 depth=32 bound=6790 best=6859 pruned_bound=5398724 pruned_infeasible=1256
[tsp-debug] progress: expanded=5500000 created=10998743 depth=26 bound=6052 best=6859 pruned_bound=5498720 pruned_infeasible=1256
[tsp-debug] progress: expanded=5600000 created=11198740 depth=27 bound=6713 best=6859 pruned_bound=5598721 pruned_infeasible=1259
[tsp-debug] progress: expanded=5700000 created=11398736 depth=39 bound=6457 best=6859 pruned_bound=5698710 pruned_infeasible=1263
[tsp-debug] progress: expanded=5800000 created=11598728 depth=41 bound=6817 best=6859 pruned_bound=5798704 pruned_infeasible=1271
[tsp-debug] progress: expanded=5900000 created=11798721 depth=37 bound=6775 best=6859 pruned_bound=5898695 pruned_infeasible=1278
[tsp-debug] progress: expanded=6000000 created=11998715 depth=38 bound=6767 best=6859 pruned_bound=5998686 pruned_infeasible=1284
[tsp-debug] progress: expanded=6100000 created=12198711 depth=37 bound=6795 best=6859 pruned_bound=6098688 pruned_infeasible=1288
[tsp-debug] progress: expanded=6200000 created=12398706 depth=33 bound=6548 best=6859 pruned_bound=6198685 pruned_infeasible=1293
[tsp-debug] progress: expanded=6300000 created=12598701 depth=27 bound=6282 best=6859 pruned_bound=6298683 pruned_infeasible=1298
[tsp-debug] progress: expanded=6400000 created=12798694 depth=28 bound=6564 best=6859 pruned_bound=6398675 pruned_infeasible=1305
[tsp-debug] progress: expanded=6500000 created=12998690 depth=26 bound=6558 best=6859 pruned_bound=6498676 pruned_infeasible=1309
[tsp-debug] progress: expanded=6600000 created=13198672 depth=41 bound=6842 best=6859 pruned_bound=6598646 pruned_infeasible=1327
[tsp-debug] progress: expanded=6700000 created=13398672 depth=32 bound=6546 best=6859 pruned_bound=6698655 pruned_infeasible=1327
[tsp-debug] progress: expanded=6800000 created=13598656 depth=45 bound=6705 best=6859 pruned_bound=6798632 pruned_infeasible=1343
[tsp-debug] progress: expanded=6900000 created=13798652 depth=39 bound=6702 best=6859 pruned_bound=6898624 pruned_infeasible=1347
[tsp-debug] progress: expanded=7000000 created=13998641 depth=32 bound=6723 best=6859 pruned_bound=6998620 pruned_infeasible=1358
[tsp-debug] progress: expanded=7100000 created=14198615 depth=21 bound=6699 best=6859 pruned_bound=7098602 pruned_infeasible=1384
[tsp-debug] progress: expanded=7200000 created=14398595 depth=34 bound=6468 best=6859 pruned_bound=7198575 pruned_infeasible=1404
[tsp-debug] progress: expanded=7300000 created=14598576 depth=41 bound=6425 best=6859 pruned_bound=7298543 pruned_infeasible=1423
[tsp-debug] progress: expanded=7400000 created=14798576 depth=49 bound=6827 best=6859 pruned_bound=7398537 pruned_infeasible=1423
[tsp-debug] progress: expanded=7500000 created=14998576 depth=33 bound=6745 best=6859 pruned_bound=7498552 pruned_infeasible=1423
[tsp-debug] progress: expanded=7600000 created=15198575 depth=27 bound=6837 best=6859 pruned_bound=7598556 pruned_infeasible=1424
[tsp-debug] progress: expanded=7700000 created=15398566 depth=29 bound=6685 best=6859 pruned_bound=7698547 pruned_infeasible=1433
[tsp-debug] progress: expanded=7800000 created=15598545 depth=31 bound=6655 best=6859 pruned_bound=7798524 pruned_infeasible=1454
[tsp-debug] progress: expanded=7900000 created=15798519 depth=28 bound=6604 best=6859 pruned_bound=7898497 pruned_infeasible=1480
[tsp-debug] progress: expanded=8000000 created=15998500 depth=26 bound=6797 best=6859 pruned_bound=7998483 pruned_infeasible=1499
[tsp-debug] progress: expanded=8100000 created=16198498 depth=40 bound=6841 best=6859 pruned_bound=8098471 pruned_infeasible=1501
[tsp-debug] progress: expanded=8200000 created=16398498 depth=44 bound=6846 best=6859 pruned_bound=8198469 pruned_infeasible=1501
[tsp-debug] progress: expanded=8300000 created=16598498 depth=31 bound=6853 best=6859 pruned_bound=8298477 pruned_infeasible=1501
[tsp-debug] progress: expanded=8400000 created=16798497 depth=30 bound=6492 best=6859 pruned_bound=8398478 pruned_infeasible=1502
[tsp-debug] progress: expanded=8500000 created=16998481 depth=26 bound=6706 best=6859 pruned_bound=8498464 pruned_infeasible=1518
[tsp-debug] progress: expanded=8600000 created=17198480 depth=35 bound=6704 best=6859 pruned_bound=8598458 pruned_infeasible=1519
[tsp-debug] progress: expanded=8700000 created=17398480 depth=29 bound=6597 best=6859 pruned_bound=8698461 pruned_infeasible=1519
[tsp-debug] progress: expanded=8800000 created=17598480 depth=28 bound=6814 best=6859 pruned_bound=8798464 pruned_infeasible=1519
[tsp-debug] progress: expanded=8900000 created=17798480 depth=35 bound=6644 best=6859 pruned_bound=8898458 pruned_infeasible=1519
[tsp-debug] progress: expanded=9000000 created=17998480 depth=31 bound=6402 best=6859 pruned_bound=8998462 pruned_infeasible=1519
[tsp-debug] progress: expanded=9100000 created=18198480 depth=34 bound=6745 best=6859 pruned_bound=9098463 pruned_infeasible=1519
[tsp-debug] progress: expanded=9200000 created=18398480 depth=38 bound=6567 best=6859 pruned_bound=9198462 pruned_infeasible=1519
[tsp-debug] progress: expanded=9300000 created=18598480 depth=40 bound=6842 best=6859 pruned_bound=9298462 pruned_infeasible=1519
[tsp-debug] progress: expanded=9400000 created=18798479 depth=35 bound=6150 best=6859 pruned_bound=9398451 pruned_infeasible=1520
[tsp-debug] progress: expanded=9500000 created=18998479 depth=33 bound=6675 best=6859 pruned_bound=9498463 pruned_infeasible=1520
[tsp-debug] progress: expanded=9600000 created=19198477 depth=31 bound=6771 best=6859 pruned_bound=9598458 pruned_infeasible=1522
[tsp-debug] progress: expanded=9700000 created=19398473 depth=37 bound=6854 best=6859 pruned_bound=9698448 pruned_infeasible=1526
[tsp-debug] progress: expanded=9800000 created=19598471 depth=40 bound=6765 best=6859 pruned_bound=9798451 pruned_infeasible=1528
[tsp-debug] progress: expanded=9900000 created=19798470 depth=43 bound=6790 best=6859 pruned_bound=9898443 pruned_infeasible=1529
[tsp-debug] progress: expanded=10000000 created=19998465 depth=39 bound=6610 best=6859 pruned_bound=9998442 pruned_infeasible=1534
[tsp-debug] progress: expanded=10100000 created=20198459 depth=42 bound=6848 best=6859 pruned_bound=10098437 pruned_infeasible=1540
[tsp-debug] progress: expanded=10200000 created=20398450 depth=34 bound=6489 best=6859 pruned_bound=10198434 pruned_infeasible=1549
[tsp-debug] progress: expanded=10300000 created=20598444 depth=25 bound=6520 best=6859 pruned_bound=10298433 pruned_infeasible=1555
[tsp-debug] progress: expanded=10400000 created=20798437 depth=26 bound=6755 best=6859 pruned_bound=10398425 pruned_infeasible=1562
[tsp-debug] progress: expanded=10500000 created=20998437 depth=36 bound=6756 best=6859 pruned_bound=10498412 pruned_infeasible=1562
[tsp-debug] progress: expanded=10600000 created=21198436 depth=26 bound=6607 best=6859 pruned_bound=10598422 pruned_infeasible=1563
[tsp-debug] progress: expanded=10700000 created=21398428 depth=32 bound=6567 best=6859 pruned_bound=10698411 pruned_infeasible=1571
[tsp-debug] progress: expanded=10800000 created=21598339 depth=35 bound=6823 best=6859 pruned_bound=10798317 pruned_infeasible=1660
[tsp-debug] progress: expanded=10900000 created=21798337 depth=30 bound=6694 best=6859 pruned_bound=10898319 pruned_infeasible=1662
[tsp-debug] progress: expanded=11000000 created=21998324 depth=30 bound=6576 best=6859 pruned_bound=10998307 pruned_infeasible=1675
[tsp-debug] progress: expanded=11100000 created=22198222 depth=29 bound=6803 best=6859 pruned_bound=11098205 pruned_infeasible=1777
[tsp-debug] progress: expanded=11200000 created=22398194 depth=28 bound=6667 best=6859 pruned_bound=11198174 pruned_infeasible=1805
[tsp-debug] progress: expanded=11300000 created=22598190 depth=36 bound=6831 best=6859 pruned_bound=11298166 pruned_infeasible=1809
[tsp-debug] progress: expanded=11400000 created=22798188 depth=26 bound=6609 best=6859 pruned_bound=11398175 pruned_infeasible=1811
[tsp-debug] progress: expanded=11500000 created=22998170 depth=22 bound=6689 best=6859 pruned_bound=11498158 pruned_infeasible=1829
[tsp-debug] progress: expanded=11600000 created=23198159 depth=29 bound=6637 best=6859 pruned_bound=11598135 pruned_infeasible=1840
[tsp-debug] progress: expanded=11700000 created=23398156 depth=38 bound=6633 best=6859 pruned_bound=11698131 pruned_infeasible=1843
[tsp-debug] progress: expanded=11800000 created=23598155 depth=39 bound=6716 best=6859 pruned_bound=11798130 pruned_infeasible=1844
[tsp-debug] progress: expanded=11900000 created=23798151 depth=25 bound=6575 best=6859 pruned_bound=11898131 pruned_infeasible=1848
[tsp-debug] progress: expanded=12000000 created=23998090 depth=33 bound=6575 best=6859 pruned_bound=11998067 pruned_infeasible=1909
[tsp-debug] progress: expanded=12100000 created=24197922 depth=28 bound=6489 best=6859 pruned_bound=12097905 pruned_infeasible=2077
[tsp-debug] progress: expanded=12200000 created=24397891 depth=29 bound=6674 best=6859 pruned_bound=12197877 pruned_infeasible=2108
[tsp-debug] progress: expanded=12300000 created=24597833 depth=22 bound=6556 best=6859 pruned_bound=12297820 pruned_infeasible=2166
[tsp-debug] progress: expanded=12400000 created=24797646 depth=34 bound=6767 best=6859 pruned_bound=12397623 pruned_infeasible=2353
[tsp-debug] progress: expanded=12500000 created=24997523 depth=31 bound=6770 best=6859 pruned_bound=12497506 pruned_infeasible=2476
[tsp-debug] progress: expanded=12600000 created=25197424 depth=38 bound=6641 best=6859 pruned_bound=12597401 pruned_infeasible=2575
[tsp-debug] progress: expanded=12700000 created=25397416 depth=32 bound=6854 best=6859 pruned_bound=12697388 pruned_infeasible=2583
[tsp-debug] progress: expanded=12800000 created=25597416 depth=34 bound=6611 best=6859 pruned_bound=12797389 pruned_infeasible=2583
[tsp-debug] progress: expanded=12900000 created=25797395 depth=22 bound=6763 best=6859 pruned_bound=12897381 pruned_infeasible=2604
[tsp-debug] progress: expanded=13000000 created=25997389 depth=48 bound=6742 best=6859 pruned_bound=12997352 pruned_infeasible=2610
[tsp-debug] progress: expanded=13100000 created=26197388 depth=32 bound=6689 best=6859 pruned_bound=13097369 pruned_infeasible=2611
[tsp-debug] progress: expanded=13200000 created=26397376 depth=37 bound=6473 best=6859 pruned_bound=13197347 pruned_infeasible=2623
[tsp-debug] progress: expanded=13300000 created=26597376 depth=35 bound=6796 best=6859 pruned_bound=13297350 pruned_infeasible=2623
[tsp-debug] progress: expanded=13400000 created=26797366 depth=18 bound=6478 best=6859 pruned_bound=13397358 pruned_infeasible=2633
[tsp-debug] progress: expanded=13500000 created=26997366 depth=34 bound=6382 best=6859 pruned_bound=13497341 pruned_infeasible=2633
[tsp-debug] progress: expanded=13600000 created=27197343 depth=41 bound=6662 best=6859 pruned_bound=13597318 pruned_infeasible=2656
[tsp-debug] progress: expanded=13700000 created=27397334 depth=26 bound=6416 best=6859 pruned_bound=13697320 pruned_infeasible=2665
[tsp-debug] progress: expanded=13800000 created=27597312 depth=41 bound=6576 best=6859 pruned_bound=13797285 pruned_infeasible=2687
[tsp-debug] progress: expanded=13900000 created=27797302 depth=61 bound=6778 best=6859 pruned_bound=13897263 pruned_infeasible=2697
[tsp-debug] progress: expanded=14000000 created=27997296 depth=38 bound=6621 best=6859 pruned_bound=13997275 pruned_infeasible=2703
[tsp-debug] progress: expanded=14100000 created=28197296 depth=39 bound=6717 best=6859 pruned_bound=14097277 pruned_infeasible=2703
[tsp-debug] progress: expanded=14200000 created=28397278 depth=57 bound=6479 best=6859 pruned_bound=14197253 pruned_infeasible=2721
[tsp-debug] progress: expanded=14300000 created=28597270 depth=41 bound=6686 best=6859 pruned_bound=14297243 pruned_infeasible=2729
[tsp-debug] progress: expanded=14400000 created=28797268 depth=39 bound=6541 best=6859 pruned_bound=14397245 pruned_infeasible=2731
[tsp-debug] progress: expanded=14500000 created=28997268 depth=36 bound=6578 best=6859 pruned_bound=14497248 pruned_infeasible=2731
[tsp-debug] progress: expanded=14600000 created=29197261 depth=32 bound=6746 best=6859 pruned_bound=14597246 pruned_infeasible=2738
[tsp-debug] progress: expanded=14700000 created=29397254 depth=34 bound=6843 best=6859 pruned_bound=14697230 pruned_infeasible=2745
[tsp-debug] progress: expanded=14800000 created=29597246 depth=24 bound=6430 best=6859 pruned_bound=14797230 pruned_infeasible=2753
[tsp-debug] progress: expanded=14900000 created=29797228 depth=28 bound=6770 best=6859 pruned_bound=14897213 pruned_infeasible=2771
[tsp-debug] progress: expanded=15000000 created=29997217 depth=35 bound=6638 best=6859 pruned_bound=14997195 pruned_infeasible=2782
[tsp-debug] progress: expanded=15100000 created=30197203 depth=25 bound=6695 best=6859 pruned_bound=15097188 pruned_infeasible=2796
[tsp-debug] progress: expanded=15200000 created=30397187 depth=32 bound=6598 best=6859 pruned_bound=15197172 pruned_infeasible=2812
[tsp-debug] progress: expanded=15300000 created=30597172 depth=44 bound=6849 best=6859 pruned_bound=15297141 pruned_infeasible=2827
[tsp-debug] progress: expanded=15400000 created=30797172 depth=31 bound=6402 best=6859 pruned_bound=15397149 pruned_infeasible=2827
[tsp-debug] progress: expanded=15500000 created=30997145 depth=44 bound=6812 best=6859 pruned_bound=15497119 pruned_infeasible=2854
[tsp-debug] progress: expanded=15600000 created=31197127 depth=36 bound=6790 best=6859 pruned_bound=15597098 pruned_infeasible=2872
[tsp-debug] progress: expanded=15700000 created=31397113 depth=26 bound=6854 best=6859 pruned_bound=15697096 pruned_infeasible=2886
[tsp-debug] progress: expanded=15800000 created=31597099 depth=34 bound=6750 best=6859 pruned_bound=15797085 pruned_infeasible=2900
[tsp-debug] progress: expanded=15900000 created=31797093 depth=54 bound=6795 best=6859 pruned_bound=15897061 pruned_infeasible=2906
[tsp-debug] progress: expanded=16000000 created=31997090 depth=44 bound=6791 best=6859 pruned_bound=15997063 pruned_infeasible=2909
[tsp-debug] progress: expanded=16100000 created=32197087 depth=33 bound=6829 best=6859 pruned_bound=16097069 pruned_infeasible=2912
[tsp-debug] progress: expanded=16200000 created=32397083 depth=38 bound=6780 best=6859 pruned_bound=16197067 pruned_infeasible=2916
[tsp-debug] progress: expanded=16300000 created=32597081 depth=54 bound=6590 best=6859 pruned_bound=16297058 pruned_infeasible=2918
[tsp-debug] progress: expanded=16400000 created=32797074 depth=29 bound=6805 best=6859 pruned_bound=16397060 pruned_infeasible=2925
[tsp-debug] progress: expanded=16500000 created=32997074 depth=26 bound=6691 best=6859 pruned_bound=16497056 pruned_infeasible=2925
[tsp-debug] progress: expanded=16600000 created=33197074 depth=21 bound=6759 best=6859 pruned_bound=16597062 pruned_infeasible=2925
[tsp-debug] progress: expanded=16700000 created=33397074 depth=24 bound=6617 best=6859 pruned_bound=16697066 pruned_infeasible=2925
[tsp-debug] progress: expanded=16800000 created=33597074 depth=48 bound=6803 best=6859 pruned_bound=16797036 pruned_infeasible=2925
[tsp-debug] progress: expanded=16900000 created=33797074 depth=32 bound=6781 best=6859 pruned_bound=16897056 pruned_infeasible=2925
[tsp-debug] progress: expanded=17000000 created=33997074 depth=28 bound=6779 best=6859 pruned_bound=16997058 pruned_infeasible=2925
[tsp-debug] progress: expanded=17100000 created=34197074 depth=25 bound=6701 best=6859 pruned_bound=17097060 pruned_infeasible=2925
[tsp-debug] progress: expanded=17200000 created=34397067 depth=32 bound=6574 best=6859 pruned_bound=17197048 pruned_infeasible=2932
[tsp-debug] progress: expanded=17300000 created=34597053 depth=28 bound=6797 best=6859 pruned_bound=17297037 pruned_infeasible=2946
[tsp-debug] progress: expanded=17400000 created=34797034 depth=35 bound=6844 best=6859 pruned_bound=17397012 pruned_infeasible=2965
[tsp-debug] progress: expanded=17500000 created=34997006 depth=28 bound=6279 best=6859 pruned_bound=17496990 pruned_infeasible=2993
[tsp-debug] progress: expanded=17600000 created=35197006 depth=25 bound=6150 best=6859 pruned_bound=17596991 pruned_infeasible=2993
[tsp-debug] progress: expanded=17700000 created=35397006 depth=35 bound=6463 best=6859 pruned_bound=17696986 pruned_infeasible=2993
[tsp-debug] progress: expanded=17800000 created=35597006 depth=53 bound=6849 best=6859 pruned_bound=17796967 pruned_infeasible=2993
[tsp-debug] progress: expanded=17900000 created=35797006 depth=26 bound=6580 best=6859 pruned_bound=17896992 pruned_infeasible=2993
[tsp-debug] progress: expanded=18000000 created=35997006 depth=28 bound=6577 best=6859 pruned_bound=17996994 pruned_infeasible=2993
[tsp-debug] progress: expanded=18100000 created=36197006 depth=33 bound=6776 best=6859 pruned_bound=18096984 pruned_infeasible=2993
[tsp-debug] progress: expanded=18200000 created=36396984 depth=29 bound=6738 best=6859 pruned_bound=18196967 pruned_infeasible=3015
[tsp-debug] progress: expanded=18300000 created=36596980 depth=28 bound=6527 best=6859 pruned_bound=18296965 pruned_infeasible=3019
[tsp-debug] progress: expanded=18400000 created=36796922 depth=33 bound=6731 best=6859 pruned_bound=18396906 pruned_infeasible=3077
[tsp-debug] progress: expanded=18500000 created=36996898 depth=28 bound=6822 best=6859 pruned_bound=18496883 pruned_infeasible=3101
[tsp-debug] progress: expanded=18600000 created=37196887 depth=48 bound=6767 best=6859 pruned_bound=18596854 pruned_infeasible=3112
[tsp-debug] progress: expanded=18700000 created=37396879 depth=29 bound=6654 best=6859 pruned_bound=18696864 pruned_infeasible=3120
[tsp-debug] progress: expanded=18800000 created=37596877 depth=39 bound=6664 best=6859 pruned_bound=18796854 pruned_infeasible=3122
[tsp-debug] progress: expanded=18900000 created=37796877 depth=43 bound=6728 best=6859 pruned_bound=18896857 pruned_infeasible=3122
[tsp-debug] progress: expanded=19000000 created=37996876 depth=30 bound=6720 best=6859 pruned_bound=18996863 pruned_infeasible=3123
[tsp-debug] progress: expanded=19100000 created=38196872 depth=24 bound=6495 best=6859 pruned_bound=19096862 pruned_infeasible=3127
[tsp-debug] progress: expanded=19200000 created=38396872 depth=32 bound=6775 best=6859 pruned_bound=19196857 pruned_infeasible=3127
[tsp-debug] progress: expanded=19300000 created=38596872 depth=24 bound=6830 best=6859 pruned_bound=19296861 pruned_infeasible=3127
[tsp-debug] progress: expanded=19400000 created=38796871 depth=26 bound=6853 best=6859 pruned_bound=19396860 pruned_infeasible=3128
[tsp-debug] progress: expanded=19500000 created=38996871 depth=29 bound=6526 best=6859 pruned_bound=19496856 pruned_infeasible=3128
[tsp-debug] progress: expanded=19600000 created=39196870 depth=21 bound=6546 best=6859 pruned_bound=19596860 pruned_infeasible=3129
[tsp-debug] progress: expanded=19700000 created=39396870 depth=39 bound=6826 best=6859 pruned_bound=19696850 pruned_infeasible=3129
[tsp-debug] progress: expanded=19800000 created=39596868 depth=26 bound=6552 best=6859 pruned_bound=19796859 pruned_infeasible=3131
[tsp-debug] progress: expanded=19900000 created=39796867 depth=32 bound=6856 best=6859 pruned_bound=19896855 pruned_infeasible=3132
[tsp-debug] progress: expanded=20000000 created=39996867 depth=26 bound=6729 best=6859 pruned_bound=19996858 pruned_infeasible=3132
[tsp-debug] progress: expanded=20100000 created=40196865 depth=41 bound=6790 best=6859 pruned_bound=20096837 pruned_infeasible=3134
[tsp-debug] progress: expanded=20200000 created=40396865 depth=43 bound=6755 best=6859 pruned_bound=20196842 pruned_infeasible=3134
[tsp-debug] progress: expanded=20300000 created=40596862 depth=29 bound=6846 best=6859 pruned_bound=20296849 pruned_infeasible=3137
[tsp-debug] progress: expanded=20400000 created=40796862 depth=41 bound=6670 best=6859 pruned_bound=20396841 pruned_infeasible=3137
[tsp-debug] progress: expanded=20500000 created=40996860 depth=31 bound=6842 best=6859 pruned_bound=20496847 pruned_infeasible=3139
[tsp-debug] progress: expanded=20600000 created=41196860 depth=46 bound=6676 best=6859 pruned_bound=20596838 pruned_infeasible=3139
[tsp-debug] progress: expanded=20700000 created=41396859 depth=35 bound=6833 best=6859 pruned_bound=20696843 pruned_infeasible=3140
[tsp-debug] progress: expanded=20800000 created=41596856 depth=42 bound=6695 best=6859 pruned_bound=20796840 pruned_infeasible=3143
[tsp-debug] progress: expanded=20900000 created=41796855 depth=33 bound=6787 best=6859 pruned_bound=20896839 pruned_infeasible=3144
[tsp-debug] progress: expanded=21000000 created=41996855 depth=31 bound=6594 best=6859 pruned_bound=20996841 pruned_infeasible=3144
[tsp-debug] progress: expanded=21100000 created=42196855 depth=47 bound=6840 best=6859 pruned_bound=21096833 pruned_infeasible=3144
[tsp-debug] progress: expanded=21200000 created=42396853 depth=35 bound=6614 best=6859 pruned_bound=21196842 pruned_infeasible=3146
[tsp-debug] progress: expanded=21300000 created=42596849 depth=42 bound=6796 best=6859 pruned_bound=21296830 pruned_infeasible=3150
[tsp-debug] progress: expanded=21400000 created=42796849 depth=32 bound=6174 best=6859 pruned_bound=21396837 pruned_infeasible=3150
[tsp-debug] progress: expanded=21500000 created=42996846 depth=47 bound=6458 best=6859 pruned_bound=21496825 pruned_infeasible=3153
[tsp-debug] progress: expanded=21600000 created=43196844 depth=52 bound=6849 best=6859 pruned_bound=21596825 pruned_infeasible=3155
[tsp-debug] progress: expanded=21700000 created=43396844 depth=41 bound=6785 best=6859 pruned_bound=21696830 pruned_infeasible=3155
[tsp-debug] progress: expanded=21800000 created=43596844 depth=33 bound=6498 best=6859 pruned_bound=21796839 pruned_infeasible=3155
[tsp-debug] progress: expanded=21900000 created=43796844 depth=47 bound=6733 best=6859 pruned_bound=21896832 pruned_infeasible=3155
[tsp-debug] exact solve finished: feasible=yes cost=6859 expanded=21957243 created=43911332 pruned_bound=21954089 pruned_infeasible=3155
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=ulysses16.tsp dimension=16 method=exact
[tsp-debug] exact solve started: vertices=16
[tsp-debug] initial incumbent: cost=6859
[tsp-debug] root: lower_bound=4746 search=recursive-dfs strategy=simple
[tsp-debug] progress: expanded=100000 created=148015 depth=84 bound=6858 best=6859 pruned_bound=47939 pruned_infeasible=51984
[tsp-debug] progress: expanded=200000 created=313024 depth=81 bound=6802 best=6859 pruned_bound=112948 pruned_infeasible=86975
[tsp-debug] progress: expanded=300000 created=481370 depth=80 bound=6722 best=6859 pruned_bound=181296 pruned_infeasible=118629
[tsp-debug] progress: expanded=400000 created=630362 depth=84 bound=6102 best=6859 pruned_bound=230286 pruned_infeasible=169637
[tsp-debug] progress: expanded=500000 created=781738 depth=73 bound=6857 best=6859 pruned_bound=281672 pruned_infeasible=218261
[tsp-debug] progress: expanded=600000 created=936422 depth=72 bound=6002 best=6859 pruned_bound=336355 pruned_infeasible=263577
[tsp-debug] progress: expanded=700000 created=1091652 depth=70 bound=6786 best=6859 pruned_bound=391588 pruned_infeasible=308347
[tsp-debug] progress: expanded=800000 created=1254121 depth=78 bound=6657 best=6859 pruned_bound=454049 pruned_infeasible=345878
[tsp-debug] progress: expanded=900000 created=1417865 depth=46 bound=6806 best=6859 pruned_bound=517823 pruned_infeasible=382134
[tsp-debug] progress: expanded=1000000 created=1576261 depth=68 bound=6785 best=6859 pruned_bound=576202 pruned_infeasible=423738
[tsp-debug] progress: expanded=1100000 created=1741901 depth=79 bound=6653 best=6859 pruned_bound=641830 pruned_infeasible=458098
[tsp-debug] progress: expanded=1200000 created=1896606 depth=90 bound=6448 best=6859 pruned_bound=696527 pruned_infeasible=503393
[tsp-debug] progress: expanded=1300000 created=2055686 depth=80 bound=6681 best=6859 pruned_bound=755612 pruned_infeasible=544313
[tsp-debug] progress: expanded=1400000 created=2220480 depth=72 bound=6598 best=6859 pruned_bound=820415 pruned_infeasible=579519
[tsp-debug] progress: expanded=1500000 created=2386506 depth=79 bound=6843 best=6859 pruned_bound=886437 pruned_infeasible=613493
[tsp-debug] progress: expanded=1600000 created=2553382 depth=68 bound=6820 best=6859 pruned_bound=953321 pruned_infeasible=646617
[tsp-debug] progress: expanded=1700000 created=2719878 depth=78 bound=6613 best=6859 pruned_bound=1019806 pruned_infeasible=680121
[tsp-debug] progress: expanded=1800000 created=2885906 depth=78 bound=6852 best=6859 pruned_bound=1085835 pruned_infeasible=714093
[tsp-debug] progress: expanded=1900000 created=3052756 depth=63 bound=6779 best=6859 pruned_bound=1152697 pruned_infeasible=747243
[tsp-debug] progress: expanded=2000000 created=3197651 depth=85 bound=6858 best=6859 pruned_bound=1197582 pruned_infeasible=802348
[tsp-debug] progress: expanded=2100000 created=3344468 depth=68 bound=6492 best=6859 pruned_bound=1244406 pruned_infeasible=855531
[tsp-debug] progress: expanded=2200000 created=3500762 depth=77 bound=6780 best=6859 pruned_bound=1300692 pruned_infeasible=899237
[tsp-debug] progress: expanded=2300000 created=3652198 depth=83 bound=6695 best=6859 pruned_bound=1352125 pruned_infeasible=947801
[tsp-debug] progress: expanded=2400000 created=3816110 depth=67 bound=6377 best=6859 pruned_bound=1416047 pruned_infeasible=983889
[tsp-debug] progress: expanded=2500000 created=3977178 depth=61 bound=6269 best=6859 pruned_bound=1477122 pruned_infeasible=1022821
[tsp-debug] progress: expanded=2600000 created=4137205 depth=69 bound=6767 best=6859 pruned_bound=1537141 pruned_infeasible=1062794
[tsp-debug] progress: expanded=2700000 created=4299002 depth=70 bound=6818 best=6859 pruned_bound=1598938 pruned_infeasible=1100997
[tsp-debug] progress: expanded=2800000 created=4459354 depth=71 bound=6244 best=6859 pruned_bound=1659287 pruned_infeasible=1140645
[tsp-debug] progress: expanded=2900000 created=4613563 depth=62 bound=6784 best=6859 pruned_bound=1713506 pruned_infeasible=1186436
[tsp-debug] progress: expanded=3000000 created=4777374 depth=83 bound=6724 best=6859 pruned_bound=1777299 pruned_infeasible=1222625
[tsp-debug] progress: expanded=3100000 created=4943524 depth=57 bound=6742 best=6859 pruned_bound=1843472 pruned_infeasible=1256475
[tsp-debug] progress: expanded=3200000 created=5111747 depth=82 bound=6656 best=6859 pruned_bound=1911674 pruned_infeasible=1288252
[tsp-debug] progress: expanded=3300000 created=5276280 depth=63 bound=6675 best=6859 pruned_bound=1976224 pruned_infeasible=1323719
[tsp-debug] progress: expanded=3400000 created=5441719 depth=72 bound=6692 best=6859 pruned_bound=2041654 pruned_infeasible=1358280
[tsp-debug] progress: expanded=3500000 created=5609471 depth=78 bound=6827 best=6859 pruned_bound=2109400 pruned_infeasible=1390528
[tsp-debug] progress: expanded=3600000 created=5765086 depth=75 bound=6378 best=6859 pruned_bound=2165019 pruned_infeasible=1434913
[tsp-debug] progress: expanded=3700000 created=5933938 depth=56 bound=6714 best=6859 pruned_bound=2233890 pruned_infeasible=1466061
[tsp-debug] exact solve finished: feasible=yes cost=6859 expanded=3798019 created=6105092 pruned_bound=2307073 pruned_infeasible=1490947
```
</details>

---

## 23. `data/classic/tsplib/gr17.tsp` (n=17)

**Concorde optimal cost:** 2085  
**Concorde time:** 26ms  

**Concorde (reference) tour:** `0 -> 3 -> 12 -> 6 -> 7 -> 5 -> 16 -> 13 -> 14 -> 2 -> 10 -> 9 -> 1 -> 4 -> 8 -> 11 -> 15 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 2085 | ref | 26ms | - | - | - | - | 1 |
| Smart | 2085 | :white_check_mark: | 1.0m | 10035553 | 5027174 | 5008379 | 18796 | - |
| Simple | 2085 | :white_check_mark: | 5.8s | 1614343 | 1008891 | 605452 | 403440 | - |

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
[tsp-debug] progress: expanded=100000 created=199159 depth=30 bound=1991 best=2085 pruned_bound=99134 pruned_infeasible=840
[tsp-debug] progress: expanded=200000 created=398068 depth=22 bound=2070 best=2085 pruned_bound=198052 pruned_infeasible=1931
[tsp-debug] progress: expanded=300000 created=597145 depth=24 bound=2075 best=2085 pruned_bound=297127 pruned_infeasible=2854
[tsp-debug] progress: expanded=400000 created=796364 depth=38 bound=2054 best=2085 pruned_bound=396348 pruned_infeasible=3635
[tsp-debug] progress: expanded=500000 created=996115 depth=27 bound=1990 best=2085 pruned_bound=496095 pruned_infeasible=3884
[tsp-debug] progress: expanded=600000 created=1196045 depth=33 bound=1996 best=2085 pruned_bound=596031 pruned_infeasible=3954
[tsp-debug] progress: expanded=700000 created=1395881 depth=22 bound=2057 best=2085 pruned_bound=695866 pruned_infeasible=4118
[tsp-debug] progress: expanded=800000 created=1595726 depth=26 bound=1951 best=2085 pruned_bound=795713 pruned_infeasible=4273
[tsp-debug] progress: expanded=900000 created=1795565 depth=40 bound=2077 best=2085 pruned_bound=895537 pruned_infeasible=4434
[tsp-debug] progress: expanded=1000000 created=1995300 depth=30 bound=2008 best=2085 pruned_bound=995285 pruned_infeasible=4699
[tsp-debug] progress: expanded=1100000 created=2195189 depth=34 bound=2053 best=2085 pruned_bound=1095167 pruned_infeasible=4810
[tsp-debug] progress: expanded=1200000 created=2394979 depth=26 bound=2062 best=2085 pruned_bound=1194964 pruned_infeasible=5020
[tsp-debug] progress: expanded=1300000 created=2594749 depth=24 bound=1910 best=2085 pruned_bound=1294734 pruned_infeasible=5250
[tsp-debug] progress: expanded=1400000 created=2794623 depth=29 bound=2050 best=2085 pruned_bound=1394606 pruned_infeasible=5376
[tsp-debug] progress: expanded=1500000 created=2994350 depth=44 bound=2062 best=2085 pruned_bound=1494334 pruned_infeasible=5649
[tsp-debug] progress: expanded=1600000 created=3194187 depth=28 bound=2024 best=2085 pruned_bound=1594170 pruned_infeasible=5812
[tsp-debug] progress: expanded=1700000 created=3394097 depth=25 bound=1971 best=2085 pruned_bound=1694083 pruned_infeasible=5902
[tsp-debug] progress: expanded=1800000 created=3593885 depth=30 bound=1973 best=2085 pruned_bound=1793859 pruned_infeasible=6114
[tsp-debug] progress: expanded=1900000 created=3793660 depth=20 bound=1980 best=2085 pruned_bound=1893650 pruned_infeasible=6339
[tsp-debug] progress: expanded=2000000 created=3993004 depth=24 bound=1999 best=2085 pruned_bound=1992990 pruned_infeasible=6995
[tsp-debug] progress: expanded=2100000 created=4192791 depth=25 bound=1933 best=2085 pruned_bound=2092774 pruned_infeasible=7208
[tsp-debug] progress: expanded=2200000 created=4392761 depth=29 bound=2047 best=2085 pruned_bound=2192741 pruned_infeasible=7238
[tsp-debug] progress: expanded=2300000 created=4592710 depth=29 bound=2062 best=2085 pruned_bound=2292695 pruned_infeasible=7289
[tsp-debug] progress: expanded=2400000 created=4792495 depth=30 bound=1954 best=2085 pruned_bound=2392480 pruned_infeasible=7504
[tsp-debug] progress: expanded=2500000 created=4992343 depth=28 bound=1954 best=2085 pruned_bound=2492327 pruned_infeasible=7656
[tsp-debug] progress: expanded=2600000 created=5192173 depth=33 bound=1992 best=2085 pruned_bound=2592149 pruned_infeasible=7826
[tsp-debug] progress: expanded=2700000 created=5392018 depth=12 bound=1852 best=2085 pruned_bound=2692012 pruned_infeasible=7981
[tsp-debug] progress: expanded=2800000 created=5591655 depth=30 bound=2076 best=2085 pruned_bound=2791638 pruned_infeasible=8344
[tsp-debug] progress: expanded=2900000 created=5791112 depth=30 bound=2053 best=2085 pruned_bound=2891091 pruned_infeasible=8887
[tsp-debug] progress: expanded=3000000 created=5990367 depth=28 bound=2074 best=2085 pruned_bound=2990349 pruned_infeasible=9632
[tsp-debug] progress: expanded=3100000 created=6189588 depth=31 bound=2043 best=2085 pruned_bound=3089568 pruned_infeasible=10411
[tsp-debug] progress: expanded=3200000 created=6388875 depth=21 bound=2005 best=2085 pruned_bound=3188864 pruned_infeasible=11124
[tsp-debug] progress: expanded=3300000 created=6588268 depth=26 bound=2035 best=2085 pruned_bound=3288256 pruned_infeasible=11731
[tsp-debug] progress: expanded=3400000 created=6787482 depth=28 bound=2042 best=2085 pruned_bound=3387467 pruned_infeasible=12517
[tsp-debug] progress: expanded=3500000 created=6987330 depth=28 bound=2082 best=2085 pruned_bound=3487316 pruned_infeasible=12669
[tsp-debug] progress: expanded=3600000 created=7186958 depth=31 bound=2080 best=2085 pruned_bound=3586944 pruned_infeasible=13041
[tsp-debug] progress: expanded=3700000 created=7386632 depth=41 bound=2084 best=2085 pruned_bound=3686612 pruned_infeasible=13367
[tsp-debug] progress: expanded=3800000 created=7586234 depth=28 bound=2028 best=2085 pruned_bound=3786217 pruned_infeasible=13765
[tsp-debug] progress: expanded=3900000 created=7785771 depth=33 bound=2074 best=2085 pruned_bound=3885758 pruned_infeasible=14228
[tsp-debug] progress: expanded=4000000 created=7985566 depth=27 bound=2001 best=2085 pruned_bound=3985555 pruned_infeasible=14433
[tsp-debug] progress: expanded=4100000 created=8185368 depth=33 bound=2078 best=2085 pruned_bound=4085354 pruned_infeasible=14631
[tsp-debug] progress: expanded=4200000 created=8385058 depth=39 bound=2068 best=2085 pruned_bound=4185034 pruned_infeasible=14941
[tsp-debug] progress: expanded=4300000 created=8584644 depth=24 bound=2061 best=2085 pruned_bound=4284635 pruned_infeasible=15355
[tsp-debug] progress: expanded=4400000 created=8784259 depth=44 bound=2042 best=2085 pruned_bound=4384232 pruned_infeasible=15740
[tsp-debug] progress: expanded=4500000 created=8983849 depth=41 bound=2070 best=2085 pruned_bound=4483827 pruned_infeasible=16150
[tsp-debug] progress: expanded=4600000 created=9183331 depth=39 bound=2052 best=2085 pruned_bound=4583314 pruned_infeasible=16668
[tsp-debug] progress: expanded=4700000 created=9382788 depth=32 bound=1966 best=2085 pruned_bound=4682775 pruned_infeasible=17211
[tsp-debug] progress: expanded=4800000 created=9582205 depth=35 bound=2082 best=2085 pruned_bound=4782187 pruned_infeasible=17794
[tsp-debug] progress: expanded=4900000 created=9781760 depth=34 bound=2034 best=2085 pruned_bound=4881746 pruned_infeasible=18239
[tsp-debug] progress: expanded=5000000 created=9981335 depth=27 bound=2037 best=2085 pruned_bound=4981326 pruned_infeasible=18664
[tsp-debug] exact solve finished: feasible=yes cost=2085 expanded=5027174 created=10035553 pruned_bound=5008379 pruned_infeasible=18796
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=gr17 dimension=17 method=exact
[tsp-debug] exact solve started: vertices=17
[tsp-debug] initial incumbent: cost=2085
[tsp-debug] root: lower_bound=1501 search=recursive-dfs strategy=simple
[tsp-debug] progress: expanded=100000 created=143939 depth=97 bound=1902 best=2085 pruned_bound=43849 pruned_infeasible=56060
[tsp-debug] progress: expanded=200000 created=293992 depth=99 bound=2055 best=2085 pruned_bound=93904 pruned_infeasible=106007
[tsp-debug] progress: expanded=300000 created=443828 depth=96 bound=1976 best=2085 pruned_bound=143741 pruned_infeasible=156171
[tsp-debug] progress: expanded=400000 created=601822 depth=78 bound=1894 best=2085 pruned_bound=201752 pruned_infeasible=198177
[tsp-debug] progress: expanded=500000 created=762888 depth=78 bound=2084 best=2085 pruned_bound=262819 pruned_infeasible=237111
[tsp-debug] progress: expanded=600000 created=923888 depth=101 bound=2021 best=2085 pruned_bound=323797 pruned_infeasible=276111
[tsp-debug] progress: expanded=700000 created=1087384 depth=54 bound=2082 best=2085 pruned_bound=387332 pruned_infeasible=312615
[tsp-debug] progress: expanded=800000 created=1258023 depth=80 bound=2027 best=2085 pruned_bound=457949 pruned_infeasible=341976
[tsp-debug] progress: expanded=900000 created=1423390 depth=80 bound=2066 best=2085 pruned_bound=523316 pruned_infeasible=376609
[tsp-debug] progress: expanded=1000000 created=1597917 depth=66 bound=2046 best=2085 pruned_bound=597856 pruned_infeasible=402082
[tsp-debug] exact solve finished: feasible=yes cost=2085 expanded=1008891 created=1614343 pruned_bound=605452 pruned_infeasible=403440
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
| Smart | 2707 | :white_check_mark: | 1.3s | 123912 | 62151 | 61761 | 391 | - |
| Simple | 2707 | :white_check_mark: | 508ms | 99317 | 56707 | 42610 | 14098 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=2707 expanded=62151 created=123912 pruned_bound=61761 pruned_infeasible=391
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=gr21 dimension=21 method=exact
[tsp-debug] exact solve started: vertices=21
[tsp-debug] initial incumbent: cost=2707
[tsp-debug] root: lower_bound=2252 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=2707 expanded=56707 created=99317 pruned_bound=42610 pruned_infeasible=14098
```
</details>

---

## 25. `data/classic/tsplib/ulysses22.tsp` (n=22)

**Concorde optimal cost:** 7013  
**Concorde time:** 63ms  

**Concorde (reference) tour:** `0 -> 13 -> 12 -> 11 -> 6 -> 5 -> 14 -> 4 -> 10 -> 8 -> 9 -> 18 -> 19 -> 20 -> 15 -> 2 -> 1 -> 16 -> 21 -> 3 -> 17 -> 7 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 7013 | ref | 63ms | - | - | - | - | 1 |
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
**Concorde time:** 22ms  

**Concorde (reference) tour:** `0 -> 11 -> 3 -> 22 -> 8 -> 12 -> 13 -> 19 -> 1 -> 14 -> 18 -> 21 -> 17 -> 16 -> 9 -> 4 -> 20 -> 7 -> 23 -> 5 -> 6 -> 2 -> 10 -> 15 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 1272 | ref | 22ms | - | - | - | - | 1 |
| Smart | 1272 | :white_check_mark: | 23.5s | 1735685 | 868999 | 866686 | 2314 | - |
| Simple | 1272 | :white_check_mark: | 1.2m | 14219957 | 8165748 | 6054209 | 2111540 | - |

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
[tsp-debug] progress: expanded=100000 created=199936 depth=25 bound=1263 best=1272 pruned_bound=99921 pruned_infeasible=63
[tsp-debug] progress: expanded=200000 created=399344 depth=27 bound=1270 best=1272 pruned_bound=199328 pruned_infeasible=655
[tsp-debug] progress: expanded=300000 created=599175 depth=23 bound=1244 best=1272 pruned_bound=299162 pruned_infeasible=824
[tsp-debug] progress: expanded=400000 created=798848 depth=25 bound=1256 best=1272 pruned_bound=398833 pruned_infeasible=1151
[tsp-debug] progress: expanded=500000 created=998521 depth=31 bound=1260 best=1272 pruned_bound=498502 pruned_infeasible=1478
[tsp-debug] progress: expanded=600000 created=1198154 depth=41 bound=1258 best=1272 pruned_bound=598133 pruned_infeasible=1845
[tsp-debug] progress: expanded=700000 created=1397845 depth=28 bound=1264 best=1272 pruned_bound=697831 pruned_infeasible=2154
[tsp-debug] progress: expanded=800000 created=1597748 depth=25 bound=1246 best=1272 pruned_bound=797735 pruned_infeasible=2251
[tsp-debug] exact solve finished: feasible=yes cost=1272 expanded=868999 created=1735685 pruned_bound=866686 pruned_infeasible=2314
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=gr24 dimension=24 method=exact
[tsp-debug] exact solve started: vertices=24
[tsp-debug] initial incumbent: cost=1272
[tsp-debug] root: lower_bound=1081 search=recursive-dfs strategy=simple
[tsp-debug] progress: expanded=100000 created=148505 depth=216 bound=1238 best=1272 pruned_bound=48296 pruned_infeasible=51494
[tsp-debug] progress: expanded=200000 created=305767 depth=220 bound=1233 best=1272 pruned_bound=105552 pruned_infeasible=94232
[tsp-debug] progress: expanded=300000 created=463552 depth=219 bound=1258 best=1272 pruned_bound=163339 pruned_infeasible=136447
[tsp-debug] progress: expanded=400000 created=626886 depth=204 bound=1251 best=1272 pruned_bound=226687 pruned_infeasible=173113
[tsp-debug] progress: expanded=500000 created=781666 depth=217 bound=1257 best=1272 pruned_bound=281456 pruned_infeasible=218333
[tsp-debug] progress: expanded=600000 created=945257 depth=214 bound=1246 best=1272 pruned_bound=345050 pruned_infeasible=254742
[tsp-debug] progress: expanded=700000 created=1106298 depth=221 bound=1225 best=1272 pruned_bound=406084 pruned_infeasible=293701
[tsp-debug] progress: expanded=800000 created=1267542 depth=227 bound=1270 best=1272 pruned_bound=467327 pruned_infeasible=332457
[tsp-debug] progress: expanded=900000 created=1435611 depth=226 bound=1258 best=1272 pruned_bound=535392 pruned_infeasible=364388
[tsp-debug] progress: expanded=1000000 created=1605914 depth=225 bound=1270 best=1272 pruned_bound=605702 pruned_infeasible=394085
[tsp-debug] progress: expanded=1100000 created=1776316 depth=218 bound=1232 best=1272 pruned_bound=676103 pruned_infeasible=423683
[tsp-debug] progress: expanded=1200000 created=1947735 depth=210 bound=1249 best=1272 pruned_bound=747531 pruned_infeasible=452264
[tsp-debug] progress: expanded=1300000 created=2112541 depth=223 bound=1251 best=1272 pruned_bound=812327 pruned_infeasible=487458
[tsp-debug] progress: expanded=1400000 created=2283410 depth=210 bound=1259 best=1272 pruned_bound=883207 pruned_infeasible=516589
[tsp-debug] progress: expanded=1500000 created=2458054 depth=223 bound=1241 best=1272 pruned_bound=957837 pruned_infeasible=541945
[tsp-debug] progress: expanded=1600000 created=2630144 depth=209 bound=1197 best=1272 pruned_bound=1029941 pruned_infeasible=569855
[tsp-debug] progress: expanded=1700000 created=2793419 depth=200 bound=1216 best=1272 pruned_bound=1093222 pruned_infeasible=606580
[tsp-debug] progress: expanded=1800000 created=2967935 depth=198 bound=1258 best=1272 pruned_bound=1167742 pruned_infeasible=632064
[tsp-debug] progress: expanded=1900000 created=3145427 depth=189 bound=1230 best=1272 pruned_bound=1245243 pruned_infeasible=654572
[tsp-debug] progress: expanded=2000000 created=3318684 depth=226 bound=1267 best=1272 pruned_bound=1318468 pruned_infeasible=681315
[tsp-debug] progress: expanded=2100000 created=3485661 depth=214 bound=1255 best=1272 pruned_bound=1385452 pruned_infeasible=714338
[tsp-debug] progress: expanded=2200000 created=3662975 depth=224 bound=1207 best=1272 pruned_bound=1462756 pruned_infeasible=737024
[tsp-debug] progress: expanded=2300000 created=3835995 depth=210 bound=1205 best=1272 pruned_bound=1535790 pruned_infeasible=764004
[tsp-debug] progress: expanded=2400000 created=4010388 depth=214 bound=1239 best=1272 pruned_bound=1610180 pruned_infeasible=789611
[tsp-debug] progress: expanded=2500000 created=4180912 depth=200 bound=1271 best=1272 pruned_bound=1680718 pruned_infeasible=819087
[tsp-debug] progress: expanded=2600000 created=4359401 depth=197 bound=1183 best=1272 pruned_bound=1759210 pruned_infeasible=840598
[tsp-debug] progress: expanded=2700000 created=4536295 depth=206 bound=1267 best=1272 pruned_bound=1836093 pruned_infeasible=863704
[tsp-debug] progress: expanded=2800000 created=4714183 depth=214 bound=1266 best=1272 pruned_bound=1913976 pruned_infeasible=885816
[tsp-debug] progress: expanded=2900000 created=4885288 depth=223 bound=1264 best=1272 pruned_bound=1985075 pruned_infeasible=914711
[tsp-debug] progress: expanded=3000000 created=5064888 depth=217 bound=1234 best=1272 pruned_bound=2064676 pruned_infeasible=935111
[tsp-debug] progress: expanded=3100000 created=5236574 depth=220 bound=1250 best=1272 pruned_bound=2136359 pruned_infeasible=963425
[tsp-debug] progress: expanded=3200000 created=5418274 depth=228 bound=1267 best=1272 pruned_bound=2218053 pruned_infeasible=981725
[tsp-debug] progress: expanded=3300000 created=5597232 depth=212 bound=1259 best=1272 pruned_bound=2297027 pruned_infeasible=1002767
[tsp-debug] progress: expanded=3400000 created=5777700 depth=203 bound=1261 best=1272 pruned_bound=2377502 pruned_infeasible=1022299
[tsp-debug] progress: expanded=3500000 created=5957022 depth=192 bound=1269 best=1272 pruned_bound=2456833 pruned_infeasible=1042977
[tsp-debug] progress: expanded=3600000 created=6139315 depth=205 bound=1262 best=1272 pruned_bound=2539116 pruned_infeasible=1060684
[tsp-debug] progress: expanded=3700000 created=6316899 depth=210 bound=1235 best=1272 pruned_bound=2616696 pruned_infeasible=1083100
[tsp-debug] progress: expanded=3800000 created=6495836 depth=215 bound=1271 best=1272 pruned_bound=2695631 pruned_infeasible=1104163
[tsp-debug] progress: expanded=3900000 created=6674070 depth=192 bound=1260 best=1272 pruned_bound=2773881 pruned_infeasible=1125929
[tsp-debug] progress: expanded=4000000 created=6853515 depth=214 bound=1259 best=1272 pruned_bound=2853307 pruned_infeasible=1146484
[tsp-debug] progress: expanded=4100000 created=7034672 depth=182 bound=1212 best=1272 pruned_bound=2934494 pruned_infeasible=1165327
[tsp-debug] progress: expanded=4200000 created=7213402 depth=222 bound=1271 best=1272 pruned_bound=3013184 pruned_infeasible=1186597
[tsp-debug] progress: expanded=4300000 created=7397599 depth=193 bound=1256 best=1272 pruned_bound=3097408 pruned_infeasible=1202400
[tsp-debug] progress: expanded=4400000 created=7582537 depth=217 bound=1203 best=1272 pruned_bound=3182326 pruned_infeasible=1217462
[tsp-debug] progress: expanded=4500000 created=7769894 depth=209 bound=1265 best=1272 pruned_bound=3269688 pruned_infeasible=1230105
[tsp-debug] progress: expanded=4600000 created=7925432 depth=226 bound=1263 best=1272 pruned_bound=3325215 pruned_infeasible=1274567
[tsp-debug] progress: expanded=4700000 created=8083982 depth=218 bound=1220 best=1272 pruned_bound=3383769 pruned_infeasible=1316017
[tsp-debug] progress: expanded=4800000 created=8247712 depth=225 bound=1241 best=1272 pruned_bound=3447496 pruned_infeasible=1352287
[tsp-debug] progress: expanded=4900000 created=8416137 depth=215 bound=1259 best=1272 pruned_bound=3515928 pruned_infeasible=1383862
[tsp-debug] progress: expanded=5000000 created=8578011 depth=227 bound=1209 best=1272 pruned_bound=3577793 pruned_infeasible=1421988
[tsp-debug] progress: expanded=5100000 created=8743733 depth=215 bound=1238 best=1272 pruned_bound=3643525 pruned_infeasible=1456266
[tsp-debug] progress: expanded=5200000 created=8910626 depth=220 bound=1262 best=1272 pruned_bound=3710414 pruned_infeasible=1489373
[tsp-debug] progress: expanded=5300000 created=9078375 depth=214 bound=1252 best=1272 pruned_bound=3778168 pruned_infeasible=1521624
[tsp-debug] progress: expanded=5400000 created=9252583 depth=223 bound=1270 best=1272 pruned_bound=3852371 pruned_infeasible=1547416
[tsp-debug] progress: expanded=5500000 created=9426236 depth=220 bound=1247 best=1272 pruned_bound=3926023 pruned_infeasible=1573763
[tsp-debug] progress: expanded=5600000 created=9600724 depth=222 bound=1262 best=1272 pruned_bound=4000510 pruned_infeasible=1599275
[tsp-debug] progress: expanded=5700000 created=9775419 depth=215 bound=1148 best=1272 pruned_bound=4075210 pruned_infeasible=1624580
[tsp-debug] progress: expanded=5800000 created=9944853 depth=216 bound=1271 best=1272 pruned_bound=4144646 pruned_infeasible=1655146
[tsp-debug] progress: expanded=5900000 created=10123902 depth=207 bound=1269 best=1272 pruned_bound=4223701 pruned_infeasible=1676097
[tsp-debug] progress: expanded=6000000 created=10298858 depth=206 bound=1238 best=1272 pruned_bound=4298658 pruned_infeasible=1701141
[tsp-debug] progress: expanded=6100000 created=10477262 depth=209 bound=1258 best=1272 pruned_bound=4377058 pruned_infeasible=1722737
[tsp-debug] progress: expanded=6200000 created=10655150 depth=199 bound=1271 best=1272 pruned_bound=4454956 pruned_infeasible=1744849
[tsp-debug] progress: expanded=6300000 created=10832977 depth=221 bound=1257 best=1272 pruned_bound=4532764 pruned_infeasible=1767022
[tsp-debug] progress: expanded=6400000 created=11003635 depth=204 bound=1245 best=1272 pruned_bound=4603437 pruned_infeasible=1796364
[tsp-debug] progress: expanded=6500000 created=11181767 depth=231 bound=1264 best=1272 pruned_bound=4681544 pruned_infeasible=1818232
[tsp-debug] progress: expanded=6600000 created=11361159 depth=196 bound=1263 best=1272 pruned_bound=4760970 pruned_infeasible=1838840
[tsp-debug] progress: expanded=6700000 created=11542754 depth=214 bound=1271 best=1272 pruned_bound=4842545 pruned_infeasible=1857245
[tsp-debug] progress: expanded=6800000 created=11725304 depth=208 bound=1265 best=1272 pruned_bound=4925103 pruned_infeasible=1874695
[tsp-debug] progress: expanded=6900000 created=11906806 depth=221 bound=1271 best=1272 pruned_bound=5006592 pruned_infeasible=1893193
[tsp-debug] progress: expanded=7000000 created=12083949 depth=184 bound=1262 best=1272 pruned_bound=5083772 pruned_infeasible=1916050
[tsp-debug] progress: expanded=7100000 created=12262080 depth=202 bound=1242 best=1272 pruned_bound=5161883 pruned_infeasible=1937919
[tsp-debug] progress: expanded=7200000 created=12445625 depth=224 bound=1257 best=1272 pruned_bound=5245408 pruned_infeasible=1954374
[tsp-debug] progress: expanded=7300000 created=12627010 depth=204 bound=1258 best=1272 pruned_bound=5326811 pruned_infeasible=1972989
[tsp-debug] progress: expanded=7400000 created=12811211 depth=204 bound=1265 best=1272 pruned_bound=5411012 pruned_infeasible=1988788
[tsp-debug] progress: expanded=7500000 created=12992517 depth=222 bound=1263 best=1272 pruned_bound=5492303 pruned_infeasible=2007482
[tsp-debug] progress: expanded=7600000 created=13173771 depth=209 bound=1267 best=1272 pruned_bound=5573571 pruned_infeasible=2026228
[tsp-debug] progress: expanded=7700000 created=13356325 depth=203 bound=1257 best=1272 pruned_bound=5656126 pruned_infeasible=2043674
[tsp-debug] progress: expanded=7800000 created=13538782 depth=195 bound=1249 best=1272 pruned_bound=5738591 pruned_infeasible=2061217
[tsp-debug] progress: expanded=7900000 created=13722787 depth=208 bound=1269 best=1272 pruned_bound=5822587 pruned_infeasible=2077212
[tsp-debug] progress: expanded=8000000 created=13908001 depth=215 bound=1239 best=1272 pruned_bound=5907791 pruned_infeasible=2091998
[tsp-debug] progress: expanded=8100000 created=14094988 depth=215 bound=1271 best=1272 pruned_bound=5994779 pruned_infeasible=2105011
[tsp-debug] exact solve finished: feasible=yes cost=1272 expanded=8165748 created=14219957 pruned_bound=6054209 pruned_infeasible=2111540
```
</details>

---

## 27. `data/classic/tsplib/fri26.tsp` (n=26)

**Concorde optimal cost:** 937  
**Concorde time:** 24ms  

**Concorde (reference) tour:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6 -> 7 -> 8 -> 9 -> 13 -> 14 -> 12 -> 11 -> 10 -> 15 -> 18 -> 19 -> 17 -> 16 -> 20 -> 21 -> 25 -> 22 -> 23 -> 24 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 937 | ref | 24ms | - | - | - | - | 1 |
| Smart | 937 | :white_check_mark: | 40.4s | 2639196 | 1320625 | 1318571 | 2055 | - |
| Simple | 937 | :white_check_mark: | 7.6s | 1137208 | 616922 | 520286 | 96637 | - |

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
[tsp-debug] progress: expanded=100000 created=199797 depth=26 bound=935 best=937 pruned_bound=99779 pruned_infeasible=202
[tsp-debug] progress: expanded=200000 created=399600 depth=22 bound=932 best=937 pruned_bound=199587 pruned_infeasible=399
[tsp-debug] progress: expanded=300000 created=599548 depth=36 bound=935 best=937 pruned_bound=299526 pruned_infeasible=451
[tsp-debug] progress: expanded=400000 created=799274 depth=31 bound=931 best=937 pruned_bound=399253 pruned_infeasible=725
[tsp-debug] progress: expanded=500000 created=999045 depth=46 bound=936 best=937 pruned_bound=499014 pruned_infeasible=954
[tsp-debug] progress: expanded=600000 created=1198942 depth=30 bound=915 best=937 pruned_bound=598926 pruned_infeasible=1057
[tsp-debug] progress: expanded=700000 created=1398629 depth=30 bound=935 best=937 pruned_bound=698610 pruned_infeasible=1370
[tsp-debug] progress: expanded=800000 created=1598523 depth=30 bound=917 best=937 pruned_bound=798511 pruned_infeasible=1476
[tsp-debug] progress: expanded=900000 created=1798391 depth=31 bound=936 best=937 pruned_bound=898368 pruned_infeasible=1608
[tsp-debug] progress: expanded=1000000 created=1998361 depth=29 bound=936 best=937 pruned_bound=998343 pruned_infeasible=1638
[tsp-debug] progress: expanded=1100000 created=2198201 depth=38 bound=926 best=937 pruned_bound=1098183 pruned_infeasible=1798
[tsp-debug] progress: expanded=1200000 created=2398117 depth=36 bound=923 best=937 pruned_bound=1198098 pruned_infeasible=1882
[tsp-debug] progress: expanded=1300000 created=2598087 depth=26 bound=928 best=937 pruned_bound=1298079 pruned_infeasible=1912
[tsp-debug] exact solve finished: feasible=yes cost=937 expanded=1320625 created=2639196 pruned_bound=1318571 pruned_infeasible=2055
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=fri26 dimension=26 method=exact
[tsp-debug] exact solve started: vertices=26
[tsp-debug] initial incumbent: cost=937
[tsp-debug] root: lower_bound=824 search=recursive-dfs strategy=simple
[tsp-debug] progress: expanded=100000 created=175751 depth=259 bound=929 best=937 pruned_bound=75499 pruned_infeasible=24248
[tsp-debug] progress: expanded=200000 created=359984 depth=236 bound=914 best=937 pruned_bound=159752 pruned_infeasible=40015
[tsp-debug] progress: expanded=300000 created=547983 depth=221 bound=933 best=937 pruned_bound=247766 pruned_infeasible=52016
[tsp-debug] progress: expanded=400000 created=732257 depth=252 bound=933 best=937 pruned_bound=332012 pruned_infeasible=67742
[tsp-debug] progress: expanded=500000 created=915854 depth=224 bound=931 best=937 pruned_bound=415635 pruned_infeasible=84145
[tsp-debug] progress: expanded=600000 created=1104204 depth=283 bound=929 best=937 pruned_bound=503929 pruned_infeasible=95795
[tsp-debug] exact solve finished: feasible=yes cost=937 expanded=616922 created=1137208 pruned_bound=520286 pruned_infeasible=96637
```
</details>

---

## 28. `data/classic/tsplib/bayg29.tsp` (n=29)

**Concorde optimal cost:** 1610  
**Concorde time:** 12ms  

**Concorde (reference) tour:** `0 -> 27 -> 5 -> 11 -> 8 -> 25 -> 2 -> 28 -> 4 -> 20 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 13 -> 16 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 7 -> 26 -> 15 -> 12 -> 23 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 1610 | ref | 12ms | - | - | - | - | 1 |
| Smart | 1610 | :white_check_mark: | 36.1m | 110735263 | 55367726 | 55367537 | 186 | - |
| Simple | 1610 | :white_check_mark: | 10.7m | 103920892 | 64228007 | 39692885 | 24535119 | - |

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
[tsp-debug] progress: expanded=100000 created=199999 depth=31 bound=1589 best=1618 pruned_bound=99974 pruned_infeasible=0
[tsp-debug] progress: expanded=200000 created=399999 depth=50 bound=1598 best=1618 pruned_bound=199963 pruned_infeasible=0
[tsp-debug] progress: expanded=300000 created=599999 depth=35 bound=1599 best=1618 pruned_bound=299975 pruned_infeasible=0
[tsp-debug] progress: expanded=400000 created=799996 depth=30 bound=1598 best=1618 pruned_bound=399973 pruned_infeasible=3
[tsp-debug] progress: expanded=500000 created=999996 depth=29 bound=1510 best=1618 pruned_bound=499972 pruned_infeasible=3
[tsp-debug] progress: expanded=600000 created=1199995 depth=30 bound=1614 best=1618 pruned_bound=599974 pruned_infeasible=4
[tsp-debug] progress: expanded=700000 created=1399995 depth=56 bound=1595 best=1618 pruned_bound=699958 pruned_infeasible=4
[tsp-debug] progress: expanded=800000 created=1599995 depth=26 bound=1555 best=1618 pruned_bound=799977 pruned_infeasible=4
[tsp-debug] progress: expanded=900000 created=1799995 depth=38 bound=1615 best=1618 pruned_bound=899971 pruned_infeasible=4
[tsp-debug] progress: expanded=1000000 created=1999995 depth=22 bound=1564 best=1618 pruned_bound=999977 pruned_infeasible=4
[tsp-debug] progress: expanded=1100000 created=2199995 depth=27 bound=1592 best=1618 pruned_bound=1099978 pruned_infeasible=4
[tsp-debug] progress: expanded=1200000 created=2399995 depth=27 bound=1584 best=1618 pruned_bound=1199974 pruned_infeasible=4
[tsp-debug] progress: expanded=1300000 created=2599993 depth=14 bound=1602 best=1618 pruned_bound=1299982 pruned_infeasible=6
[tsp-debug] progress: expanded=1400000 created=2799993 depth=21 bound=1611 best=1618 pruned_bound=1399980 pruned_infeasible=6
[tsp-debug] progress: expanded=1500000 created=2999993 depth=32 bound=1539 best=1618 pruned_bound=1499968 pruned_infeasible=6
[tsp-debug] progress: expanded=1600000 created=3199993 depth=26 bound=1614 best=1618 pruned_bound=1599975 pruned_infeasible=6
[tsp-debug] progress: expanded=1700000 created=3399993 depth=26 bound=1581 best=1618 pruned_bound=1699976 pruned_infeasible=6
[tsp-debug] progress: expanded=1800000 created=3599993 depth=36 bound=1596 best=1618 pruned_bound=1799963 pruned_infeasible=6
[tsp-debug] progress: expanded=1900000 created=3799993 depth=33 bound=1599 best=1618 pruned_bound=1899970 pruned_infeasible=6
[tsp-debug] progress: expanded=2000000 created=3999993 depth=31 bound=1611 best=1618 pruned_bound=1999971 pruned_infeasible=6
[tsp-debug] progress: expanded=2100000 created=4199993 depth=44 bound=1606 best=1618 pruned_bound=2099984 pruned_infeasible=6
[tsp-debug] progress: expanded=2200000 created=4399993 depth=34 bound=1592 best=1618 pruned_bound=2199969 pruned_infeasible=6
[tsp-debug] progress: expanded=2300000 created=4599993 depth=37 bound=1615 best=1618 pruned_bound=2299968 pruned_infeasible=6
[tsp-debug] progress: expanded=2400000 created=4799993 depth=24 bound=1617 best=1618 pruned_bound=2399977 pruned_infeasible=6
[tsp-debug] progress: expanded=2500000 created=4999993 depth=26 bound=1600 best=1618 pruned_bound=2499978 pruned_infeasible=6
[tsp-debug] progress: expanded=2600000 created=5199993 depth=33 bound=1607 best=1618 pruned_bound=2599968 pruned_infeasible=6
[tsp-debug] progress: expanded=2700000 created=5399993 depth=24 bound=1557 best=1618 pruned_bound=2699976 pruned_infeasible=6
[tsp-debug] progress: expanded=2800000 created=5599993 depth=52 bound=1616 best=1618 pruned_bound=2799962 pruned_infeasible=6
[tsp-debug] progress: expanded=2900000 created=5799993 depth=46 bound=1614 best=1618 pruned_bound=2899964 pruned_infeasible=6
[tsp-debug] progress: expanded=3000000 created=5999993 depth=36 bound=1604 best=1618 pruned_bound=2999970 pruned_infeasible=6
[tsp-debug] progress: expanded=3100000 created=6199993 depth=36 bound=1616 best=1618 pruned_bound=3099966 pruned_infeasible=6
[tsp-debug] progress: expanded=3200000 created=6399993 depth=41 bound=1616 best=1618 pruned_bound=3199967 pruned_infeasible=6
[tsp-debug] progress: expanded=3300000 created=6599993 depth=22 bound=1606 best=1618 pruned_bound=3299979 pruned_infeasible=6
[tsp-debug] progress: expanded=3400000 created=6799993 depth=40 bound=1597 best=1618 pruned_bound=3399965 pruned_infeasible=6
[tsp-debug] progress: expanded=3500000 created=6999993 depth=34 bound=1617 best=1618 pruned_bound=3499977 pruned_infeasible=6
[tsp-debug] progress: expanded=3600000 created=7199993 depth=35 bound=1615 best=1618 pruned_bound=3599973 pruned_infeasible=6
[tsp-debug] progress: expanded=3700000 created=7399993 depth=58 bound=1595 best=1618 pruned_bound=3699958 pruned_infeasible=6
[tsp-debug] progress: expanded=3800000 created=7599993 depth=41 bound=1597 best=1618 pruned_bound=3799965 pruned_infeasible=6
[tsp-debug] progress: expanded=3900000 created=7799993 depth=45 bound=1617 best=1618 pruned_bound=3899970 pruned_infeasible=6
[tsp-debug] progress: expanded=4000000 created=7999993 depth=40 bound=1616 best=1618 pruned_bound=3999965 pruned_infeasible=6
[tsp-debug] progress: expanded=4100000 created=8199993 depth=29 bound=1593 best=1618 pruned_bound=4099977 pruned_infeasible=6
[tsp-debug] progress: expanded=4200000 created=8399993 depth=24 bound=1608 best=1618 pruned_bound=4199979 pruned_infeasible=6
[tsp-debug] progress: expanded=4300000 created=8599993 depth=37 bound=1608 best=1618 pruned_bound=4299980 pruned_infeasible=6
[tsp-debug] progress: expanded=4400000 created=8799993 depth=31 bound=1581 best=1618 pruned_bound=4399969 pruned_infeasible=6
[tsp-debug] progress: expanded=4500000 created=8999993 depth=27 bound=1613 best=1618 pruned_bound=4499975 pruned_infeasible=6
[tsp-debug] progress: expanded=4600000 created=9199993 depth=35 bound=1615 best=1618 pruned_bound=4599968 pruned_infeasible=6
[tsp-debug] progress: expanded=4700000 created=9399993 depth=48 bound=1614 best=1618 pruned_bound=4699967 pruned_infeasible=6
[tsp-debug] progress: expanded=4800000 created=9599993 depth=33 bound=1612 best=1618 pruned_bound=4799976 pruned_infeasible=6
[tsp-debug] progress: expanded=4900000 created=9799993 depth=26 bound=1603 best=1618 pruned_bound=4899975 pruned_infeasible=6
[tsp-debug] progress: expanded=5000000 created=9999993 depth=32 bound=1616 best=1618 pruned_bound=4999970 pruned_infeasible=6
[tsp-debug] progress: expanded=5100000 created=10199993 depth=34 bound=1611 best=1618 pruned_bound=5099978 pruned_infeasible=6
[tsp-debug] progress: expanded=5200000 created=10399993 depth=34 bound=1607 best=1618 pruned_bound=5199964 pruned_infeasible=6
[tsp-debug] progress: expanded=5300000 created=10599993 depth=20 bound=1596 best=1618 pruned_bound=5299982 pruned_infeasible=6
[tsp-debug] progress: expanded=5400000 created=10799993 depth=31 bound=1599 best=1618 pruned_bound=5399976 pruned_infeasible=6
[tsp-debug] progress: expanded=5500000 created=10999993 depth=36 bound=1617 best=1618 pruned_bound=5499975 pruned_infeasible=6
[tsp-debug] progress: expanded=5600000 created=11199993 depth=15 bound=1527 best=1618 pruned_bound=5599986 pruned_infeasible=6
[tsp-debug] progress: expanded=5700000 created=11399993 depth=36 bound=1610 best=1618 pruned_bound=5699973 pruned_infeasible=6
[tsp-debug] progress: expanded=5800000 created=11599992 depth=37 bound=1604 best=1618 pruned_bound=5799970 pruned_infeasible=7
[tsp-debug] progress: expanded=5900000 created=11799991 depth=36 bound=1607 best=1618 pruned_bound=5899976 pruned_infeasible=8
[tsp-debug] progress: expanded=6000000 created=11999990 depth=28 bound=1611 best=1618 pruned_bound=5999972 pruned_infeasible=9
[tsp-debug] progress: expanded=6100000 created=12199990 depth=27 bound=1582 best=1618 pruned_bound=6099971 pruned_infeasible=9
[tsp-debug] progress: expanded=6200000 created=12399990 depth=29 bound=1604 best=1618 pruned_bound=6199971 pruned_infeasible=9
[tsp-debug] progress: expanded=6300000 created=12599990 depth=29 bound=1584 best=1618 pruned_bound=6299976 pruned_infeasible=9
[tsp-debug] progress: expanded=6400000 created=12799988 depth=28 bound=1603 best=1618 pruned_bound=6399969 pruned_infeasible=11
[tsp-debug] progress: expanded=6500000 created=12999987 depth=32 bound=1595 best=1618 pruned_bound=6499970 pruned_infeasible=12
[tsp-debug] progress: expanded=6600000 created=13199971 depth=28 bound=1615 best=1618 pruned_bound=6599951 pruned_infeasible=28
[tsp-debug] progress: expanded=6700000 created=13399971 depth=21 bound=1567 best=1618 pruned_bound=6699959 pruned_infeasible=28
[tsp-debug] progress: expanded=6800000 created=13599971 depth=33 bound=1611 best=1618 pruned_bound=6799950 pruned_infeasible=28
[tsp-debug] progress: expanded=6900000 created=13799971 depth=40 bound=1615 best=1618 pruned_bound=6899943 pruned_infeasible=28
[tsp-debug] progress: expanded=7000000 created=13999971 depth=35 bound=1559 best=1618 pruned_bound=6999945 pruned_infeasible=28
[tsp-debug] progress: expanded=7100000 created=14199971 depth=29 bound=1613 best=1618 pruned_bound=7099950 pruned_infeasible=28
[tsp-debug] progress: expanded=7200000 created=14399971 depth=41 bound=1601 best=1618 pruned_bound=7199951 pruned_infeasible=28
[tsp-debug] progress: expanded=7300000 created=14599971 depth=32 bound=1600 best=1618 pruned_bound=7299952 pruned_infeasible=28
[tsp-debug] progress: expanded=7400000 created=14799971 depth=36 bound=1609 best=1618 pruned_bound=7399948 pruned_infeasible=28
[tsp-debug] progress: expanded=7500000 created=14999971 depth=23 bound=1572 best=1618 pruned_bound=7499961 pruned_infeasible=28
[tsp-debug] progress: expanded=7600000 created=15199971 depth=33 bound=1610 best=1618 pruned_bound=7599947 pruned_infeasible=28
[tsp-debug] progress: expanded=7700000 created=15399971 depth=32 bound=1603 best=1618 pruned_bound=7699948 pruned_infeasible=28
[tsp-debug] progress: expanded=7800000 created=15599971 depth=28 bound=1602 best=1618 pruned_bound=7799953 pruned_infeasible=28
[tsp-debug] progress: expanded=7900000 created=15799971 depth=44 bound=1613 best=1618 pruned_bound=7899949 pruned_infeasible=28
[tsp-debug] progress: expanded=8000000 created=15999970 depth=32 bound=1600 best=1618 pruned_bound=7999953 pruned_infeasible=29
[tsp-debug] progress: expanded=8100000 created=16199969 depth=28 bound=1588 best=1618 pruned_bound=8099949 pruned_infeasible=30
[tsp-debug] progress: expanded=8200000 created=16399969 depth=37 bound=1571 best=1618 pruned_bound=8199952 pruned_infeasible=30
[tsp-debug] progress: expanded=8300000 created=16599969 depth=40 bound=1608 best=1618 pruned_bound=8299948 pruned_infeasible=30
[tsp-debug] progress: expanded=8400000 created=16799969 depth=30 bound=1606 best=1618 pruned_bound=8399951 pruned_infeasible=30
[tsp-debug] progress: expanded=8500000 created=16999969 depth=26 bound=1593 best=1618 pruned_bound=8499953 pruned_infeasible=30
[tsp-debug] progress: expanded=8600000 created=17199969 depth=31 bound=1590 best=1618 pruned_bound=8599953 pruned_infeasible=30
[tsp-debug] progress: expanded=8700000 created=17399969 depth=24 bound=1616 best=1618 pruned_bound=8699952 pruned_infeasible=30
[tsp-debug] progress: expanded=8800000 created=17599969 depth=24 bound=1593 best=1618 pruned_bound=8799958 pruned_infeasible=30
[tsp-debug] progress: expanded=8900000 created=17799969 depth=34 bound=1611 best=1618 pruned_bound=8899948 pruned_infeasible=30
[tsp-debug] progress: expanded=9000000 created=17999968 depth=31 bound=1604 best=1618 pruned_bound=8999951 pruned_infeasible=31
[tsp-debug] progress: expanded=9100000 created=18199968 depth=45 bound=1611 best=1618 pruned_bound=9099943 pruned_infeasible=31
[tsp-debug] progress: expanded=9200000 created=18399968 depth=26 bound=1613 best=1618 pruned_bound=9199952 pruned_infeasible=31
[tsp-debug] progress: expanded=9300000 created=18599968 depth=33 bound=1594 best=1618 pruned_bound=9299953 pruned_infeasible=31
[tsp-debug] progress: expanded=9400000 created=18799968 depth=30 bound=1609 best=1618 pruned_bound=9399949 pruned_infeasible=31
[tsp-debug] progress: expanded=9500000 created=18999967 depth=39 bound=1599 best=1618 pruned_bound=9499940 pruned_infeasible=32
[tsp-debug] progress: expanded=9600000 created=19199967 depth=26 bound=1553 best=1618 pruned_bound=9599953 pruned_infeasible=32
[tsp-debug] progress: expanded=9700000 created=19399967 depth=34 bound=1614 best=1618 pruned_bound=9699944 pruned_infeasible=32
[tsp-debug] progress: expanded=9800000 created=19599967 depth=33 bound=1577 best=1618 pruned_bound=9799945 pruned_infeasible=32
[tsp-debug] progress: expanded=9900000 created=19799967 depth=34 bound=1617 best=1618 pruned_bound=9899951 pruned_infeasible=32
[tsp-debug] progress: expanded=10000000 created=19999967 depth=32 bound=1573 best=1618 pruned_bound=9999945 pruned_infeasible=32
[tsp-debug] progress: expanded=10100000 created=20199967 depth=21 bound=1536 best=1618 pruned_bound=10099955 pruned_infeasible=32
[tsp-debug] progress: expanded=10200000 created=20399967 depth=43 bound=1604 best=1618 pruned_bound=10199938 pruned_infeasible=32
[tsp-debug] progress: expanded=10300000 created=20599967 depth=37 bound=1613 best=1618 pruned_bound=10299945 pruned_infeasible=32
[tsp-debug] progress: expanded=10400000 created=20799967 depth=32 bound=1610 best=1618 pruned_bound=10399946 pruned_infeasible=32
[tsp-debug] progress: expanded=10500000 created=20999967 depth=25 bound=1552 best=1618 pruned_bound=10499954 pruned_infeasible=32
[tsp-debug] progress: expanded=10600000 created=21199967 depth=31 bound=1616 best=1618 pruned_bound=10599948 pruned_infeasible=32
[tsp-debug] progress: expanded=10700000 created=21399967 depth=24 bound=1568 best=1618 pruned_bound=10699956 pruned_infeasible=32
[tsp-debug] progress: expanded=10800000 created=21599967 depth=33 bound=1597 best=1618 pruned_bound=10799944 pruned_infeasible=32
[tsp-debug] progress: expanded=10900000 created=21799967 depth=41 bound=1597 best=1618 pruned_bound=10899943 pruned_infeasible=32
[tsp-debug] progress: expanded=11000000 created=21999967 depth=41 bound=1597 best=1618 pruned_bound=10999945 pruned_infeasible=32
[tsp-debug] progress: expanded=11100000 created=22199967 depth=44 bound=1612 best=1618 pruned_bound=11099950 pruned_infeasible=32
[tsp-debug] progress: expanded=11200000 created=22399967 depth=33 bound=1614 best=1618 pruned_bound=11199947 pruned_infeasible=32
[tsp-debug] progress: expanded=11300000 created=22599967 depth=37 bound=1604 best=1618 pruned_bound=11299942 pruned_infeasible=32
[tsp-debug] progress: expanded=11400000 created=22799967 depth=19 bound=1565 best=1618 pruned_bound=11399958 pruned_infeasible=32
[tsp-debug] progress: expanded=11500000 created=22999967 depth=28 bound=1564 best=1618 pruned_bound=11499949 pruned_infeasible=32
[tsp-debug] progress: expanded=11600000 created=23199967 depth=38 bound=1614 best=1618 pruned_bound=11599938 pruned_infeasible=32
[tsp-debug] progress: expanded=11700000 created=23399967 depth=29 bound=1604 best=1618 pruned_bound=11699948 pruned_infeasible=32
[tsp-debug] progress: expanded=11800000 created=23599967 depth=26 bound=1583 best=1618 pruned_bound=11799951 pruned_infeasible=32
[tsp-debug] progress: expanded=11900000 created=23799967 depth=27 bound=1606 best=1618 pruned_bound=11899953 pruned_infeasible=32
[tsp-debug] progress: expanded=12000000 created=23999967 depth=23 bound=1600 best=1618 pruned_bound=11999955 pruned_infeasible=32
[tsp-debug] progress: expanded=12100000 created=24199967 depth=39 bound=1609 best=1618 pruned_bound=12099942 pruned_infeasible=32
[tsp-debug] progress: expanded=12200000 created=24399967 depth=26 bound=1597 best=1618 pruned_bound=12199947 pruned_infeasible=32
[tsp-debug] progress: expanded=12300000 created=24599967 depth=26 bound=1610 best=1618 pruned_bound=12299954 pruned_infeasible=32
[tsp-debug] progress: expanded=12400000 created=24799967 depth=24 bound=1573 best=1618 pruned_bound=12399955 pruned_infeasible=32
[tsp-debug] progress: expanded=12500000 created=24999963 depth=32 bound=1615 best=1618 pruned_bound=12499946 pruned_infeasible=36
[tsp-debug] progress: expanded=12600000 created=25199963 depth=26 bound=1605 best=1618 pruned_bound=12599948 pruned_infeasible=36
[tsp-debug] progress: expanded=12700000 created=25399963 depth=20 bound=1539 best=1618 pruned_bound=12699953 pruned_infeasible=36
[tsp-debug] progress: expanded=12800000 created=25599963 depth=25 bound=1601 best=1618 pruned_bound=12799943 pruned_infeasible=36
[tsp-debug] progress: expanded=12900000 created=25799963 depth=23 bound=1604 best=1618 pruned_bound=12899948 pruned_infeasible=36
[tsp-debug] progress: expanded=13000000 created=25999963 depth=49 bound=1610 best=1618 pruned_bound=12999931 pruned_infeasible=36
[tsp-debug] progress: expanded=13100000 created=26199963 depth=26 bound=1614 best=1618 pruned_bound=13099951 pruned_infeasible=36
[tsp-debug] progress: expanded=13200000 created=26399963 depth=17 bound=1591 best=1618 pruned_bound=13199952 pruned_infeasible=36
[tsp-debug] progress: expanded=13300000 created=26599963 depth=39 bound=1610 best=1618 pruned_bound=13299930 pruned_infeasible=36
[tsp-debug] progress: expanded=13400000 created=26799962 depth=51 bound=1610 best=1618 pruned_bound=13399922 pruned_infeasible=37
[tsp-debug] progress: expanded=13500000 created=26999961 depth=45 bound=1580 best=1618 pruned_bound=13499928 pruned_infeasible=38
[tsp-debug] progress: expanded=13600000 created=27199961 depth=44 bound=1589 best=1618 pruned_bound=13599935 pruned_infeasible=38
[tsp-debug] progress: expanded=13700000 created=27399961 depth=33 bound=1611 best=1618 pruned_bound=13699934 pruned_infeasible=38
[tsp-debug] progress: expanded=13800000 created=27599961 depth=27 bound=1607 best=1618 pruned_bound=13799942 pruned_infeasible=38
[tsp-debug] progress: expanded=13900000 created=27799960 depth=18 bound=1569 best=1618 pruned_bound=13899950 pruned_infeasible=39
[tsp-debug] progress: expanded=14000000 created=27999960 depth=27 bound=1610 best=1618 pruned_bound=13999944 pruned_infeasible=39
[tsp-debug] progress: expanded=14100000 created=28199960 depth=40 bound=1614 best=1618 pruned_bound=14099934 pruned_infeasible=39
[tsp-debug] progress: expanded=14200000 created=28399959 depth=39 bound=1570 best=1618 pruned_bound=14199933 pruned_infeasible=40
[tsp-debug] progress: expanded=14300000 created=28599957 depth=44 bound=1587 best=1618 pruned_bound=14299923 pruned_infeasible=42
[tsp-debug] progress: expanded=14400000 created=28799957 depth=17 bound=1484 best=1618 pruned_bound=14399947 pruned_infeasible=42
[tsp-debug] progress: expanded=14500000 created=28999957 depth=49 bound=1543 best=1618 pruned_bound=14499926 pruned_infeasible=42
[tsp-debug] progress: expanded=14600000 created=29199956 depth=36 bound=1572 best=1618 pruned_bound=14599933 pruned_infeasible=43
[tsp-debug] progress: expanded=14700000 created=29399956 depth=41 bound=1567 best=1618 pruned_bound=14699931 pruned_infeasible=43
[tsp-debug] progress: expanded=14800000 created=29599956 depth=43 bound=1600 best=1618 pruned_bound=14799930 pruned_infeasible=43
[tsp-debug] progress: expanded=14900000 created=29799956 depth=33 bound=1604 best=1618 pruned_bound=14899934 pruned_infeasible=43
[tsp-debug] progress: expanded=15000000 created=29999956 depth=25 bound=1604 best=1618 pruned_bound=14999938 pruned_infeasible=43
[tsp-debug] progress: expanded=15100000 created=30199956 depth=33 bound=1598 best=1618 pruned_bound=15099930 pruned_infeasible=43
[tsp-debug] progress: expanded=15200000 created=30399956 depth=25 bound=1604 best=1618 pruned_bound=15199943 pruned_infeasible=43
[tsp-debug] progress: expanded=15300000 created=30599956 depth=29 bound=1596 best=1618 pruned_bound=15299935 pruned_infeasible=43
[tsp-debug] progress: expanded=15400000 created=30799949 depth=23 bound=1554 best=1618 pruned_bound=15399933 pruned_infeasible=50
[tsp-debug] progress: expanded=15500000 created=30999949 depth=41 bound=1608 best=1618 pruned_bound=15499917 pruned_infeasible=50
[tsp-debug] progress: expanded=15600000 created=31199946 depth=22 bound=1591 best=1618 pruned_bound=15599932 pruned_infeasible=53
[tsp-debug] progress: expanded=15700000 created=31399943 depth=41 bound=1612 best=1618 pruned_bound=15699918 pruned_infeasible=56
[tsp-debug] progress: expanded=15800000 created=31599939 depth=29 bound=1559 best=1618 pruned_bound=15799918 pruned_infeasible=60
[tsp-debug] progress: expanded=15900000 created=31799939 depth=41 bound=1616 best=1618 pruned_bound=15899913 pruned_infeasible=60
[tsp-debug] progress: expanded=16000000 created=31999933 depth=18 bound=1564 best=1618 pruned_bound=15999923 pruned_infeasible=66
[tsp-debug] progress: expanded=16100000 created=32199933 depth=37 bound=1578 best=1618 pruned_bound=16099908 pruned_infeasible=66
[tsp-debug] progress: expanded=16200000 created=32399933 depth=31 bound=1616 best=1618 pruned_bound=16199913 pruned_infeasible=66
[tsp-debug] progress: expanded=16300000 created=32599933 depth=35 bound=1617 best=1618 pruned_bound=16299906 pruned_infeasible=66
[tsp-debug] progress: expanded=16400000 created=32799933 depth=27 bound=1611 best=1618 pruned_bound=16399917 pruned_infeasible=66
[tsp-debug] progress: expanded=16500000 created=32999933 depth=46 bound=1597 best=1618 pruned_bound=16499911 pruned_infeasible=66
[tsp-debug] progress: expanded=16600000 created=33199933 depth=32 bound=1611 best=1618 pruned_bound=16599914 pruned_infeasible=66
[tsp-debug] progress: expanded=16700000 created=33399929 depth=40 bound=1596 best=1618 pruned_bound=16699907 pruned_infeasible=70
[tsp-debug] progress: expanded=16800000 created=33599929 depth=27 bound=1615 best=1618 pruned_bound=16799908 pruned_infeasible=70
[tsp-debug] progress: expanded=16900000 created=33799929 depth=26 bound=1582 best=1618 pruned_bound=16899913 pruned_infeasible=70
[tsp-debug] progress: expanded=17000000 created=33999929 depth=19 bound=1529 best=1618 pruned_bound=16999915 pruned_infeasible=70
[tsp-debug] progress: expanded=17100000 created=34199929 depth=31 bound=1605 best=1618 pruned_bound=17099908 pruned_infeasible=70
[tsp-debug] progress: expanded=17200000 created=34399929 depth=35 bound=1617 best=1618 pruned_bound=17199910 pruned_infeasible=70
[tsp-debug] progress: expanded=17300000 created=34599929 depth=15 bound=1504 best=1618 pruned_bound=17299921 pruned_infeasible=70
[tsp-debug] progress: expanded=17400000 created=34799929 depth=37 bound=1616 best=1618 pruned_bound=17399905 pruned_infeasible=70
[tsp-debug] progress: expanded=17500000 created=34999929 depth=36 bound=1582 best=1618 pruned_bound=17499910 pruned_infeasible=70
[tsp-debug] progress: expanded=17600000 created=35199929 depth=48 bound=1595 best=1618 pruned_bound=17599902 pruned_infeasible=70
[tsp-debug] progress: expanded=17700000 created=35399923 depth=38 bound=1617 best=1618 pruned_bound=17699897 pruned_infeasible=76
[tsp-debug] progress: expanded=17800000 created=35599922 depth=34 bound=1550 best=1618 pruned_bound=17799905 pruned_infeasible=77
[tsp-debug] progress: expanded=17900000 created=35799922 depth=29 bound=1589 best=1618 pruned_bound=17899907 pruned_infeasible=77
[tsp-debug] progress: expanded=18000000 created=35999920 depth=26 bound=1615 best=1618 pruned_bound=17999904 pruned_infeasible=79
[tsp-debug] progress: expanded=18100000 created=36199920 depth=31 bound=1616 best=1618 pruned_bound=18099903 pruned_infeasible=79
[tsp-debug] progress: expanded=18200000 created=36399920 depth=42 bound=1602 best=1618 pruned_bound=18199895 pruned_infeasible=79
[tsp-debug] progress: expanded=18300000 created=36599920 depth=48 bound=1614 best=1618 pruned_bound=18299889 pruned_infeasible=79
[tsp-debug] progress: expanded=18400000 created=36799918 depth=39 bound=1589 best=1618 pruned_bound=18399902 pruned_infeasible=81
[tsp-debug] progress: expanded=18500000 created=36999917 depth=34 bound=1597 best=1618 pruned_bound=18499892 pruned_infeasible=82
[tsp-debug] progress: expanded=18600000 created=37199917 depth=31 bound=1602 best=1618 pruned_bound=18599899 pruned_infeasible=82
[tsp-debug] progress: expanded=18700000 created=37399917 depth=33 bound=1608 best=1618 pruned_bound=18699900 pruned_infeasible=82
[tsp-debug] progress: expanded=18800000 created=37599917 depth=28 bound=1617 best=1618 pruned_bound=18799903 pruned_infeasible=82
[tsp-debug] progress: expanded=18900000 created=37799917 depth=40 bound=1600 best=1618 pruned_bound=18899901 pruned_infeasible=82
[tsp-debug] progress: expanded=19000000 created=37999917 depth=41 bound=1600 best=1618 pruned_bound=18999897 pruned_infeasible=82
[tsp-debug] progress: expanded=19100000 created=38199917 depth=65 bound=1611 best=1618 pruned_bound=19099887 pruned_infeasible=82
[tsp-debug] progress: expanded=19200000 created=38399917 depth=37 bound=1609 best=1618 pruned_bound=19199897 pruned_infeasible=82
[tsp-debug] progress: expanded=19300000 created=38599917 depth=27 bound=1617 best=1618 pruned_bound=19299902 pruned_infeasible=82
[tsp-debug] progress: expanded=19400000 created=38799917 depth=28 bound=1596 best=1618 pruned_bound=19399904 pruned_infeasible=82
[tsp-debug] progress: expanded=19500000 created=38999917 depth=18 bound=1591 best=1618 pruned_bound=19499905 pruned_infeasible=82
[tsp-debug] progress: expanded=19600000 created=39199917 depth=36 bound=1588 best=1618 pruned_bound=19599896 pruned_infeasible=82
[tsp-debug] progress: expanded=19700000 created=39399917 depth=38 bound=1609 best=1618 pruned_bound=19699904 pruned_infeasible=82
[tsp-debug] progress: expanded=19800000 created=39599917 depth=25 bound=1594 best=1618 pruned_bound=19799902 pruned_infeasible=82
[tsp-debug] progress: expanded=19900000 created=39799916 depth=36 bound=1615 best=1618 pruned_bound=19899891 pruned_infeasible=83
[tsp-debug] progress: expanded=20000000 created=39999916 depth=23 bound=1611 best=1618 pruned_bound=19999904 pruned_infeasible=83
[tsp-debug] progress: expanded=20100000 created=40199915 depth=28 bound=1609 best=1618 pruned_bound=20099900 pruned_infeasible=84
[tsp-debug] progress: expanded=20200000 created=40399915 depth=32 bound=1595 best=1618 pruned_bound=20199889 pruned_infeasible=84
[tsp-debug] progress: expanded=20300000 created=40599915 depth=35 bound=1582 best=1618 pruned_bound=20299890 pruned_infeasible=84
[tsp-debug] progress: expanded=20400000 created=40799915 depth=29 bound=1578 best=1618 pruned_bound=20399896 pruned_infeasible=84
[tsp-debug] progress: expanded=20500000 created=40999915 depth=27 bound=1590 best=1618 pruned_bound=20499897 pruned_infeasible=84
[tsp-debug] progress: expanded=20600000 created=41199915 depth=35 bound=1582 best=1618 pruned_bound=20599890 pruned_infeasible=84
[tsp-debug] progress: expanded=20700000 created=41399915 depth=37 bound=1611 best=1618 pruned_bound=20699887 pruned_infeasible=84
[tsp-debug] progress: expanded=20800000 created=41599915 depth=46 bound=1614 best=1618 pruned_bound=20799884 pruned_infeasible=84
[tsp-debug] progress: expanded=20900000 created=41799915 depth=33 bound=1602 best=1618 pruned_bound=20899894 pruned_infeasible=84
[tsp-debug] progress: expanded=21000000 created=41999915 depth=46 bound=1606 best=1618 pruned_bound=20999881 pruned_infeasible=84
[tsp-debug] progress: expanded=21100000 created=42199915 depth=40 bound=1616 best=1618 pruned_bound=21099889 pruned_infeasible=84
[tsp-debug] progress: expanded=21200000 created=42399915 depth=43 bound=1611 best=1618 pruned_bound=21199887 pruned_infeasible=84
[tsp-debug] progress: expanded=21300000 created=42599915 depth=29 bound=1608 best=1618 pruned_bound=21299895 pruned_infeasible=84
[tsp-debug] progress: expanded=21400000 created=42799915 depth=30 bound=1598 best=1618 pruned_bound=21399891 pruned_infeasible=84
[tsp-debug] progress: expanded=21500000 created=42999915 depth=28 bound=1584 best=1618 pruned_bound=21499901 pruned_infeasible=84
[tsp-debug] progress: expanded=21600000 created=43199915 depth=29 bound=1608 best=1618 pruned_bound=21599898 pruned_infeasible=84
[tsp-debug] progress: expanded=21700000 created=43399915 depth=29 bound=1610 best=1618 pruned_bound=21699897 pruned_infeasible=84
[tsp-debug] progress: expanded=21800000 created=43599915 depth=24 bound=1616 best=1618 pruned_bound=21799901 pruned_infeasible=84
[tsp-debug] progress: expanded=21900000 created=43799915 depth=27 bound=1615 best=1618 pruned_bound=21899903 pruned_infeasible=84
[tsp-debug] progress: expanded=22000000 created=43999915 depth=35 bound=1611 best=1618 pruned_bound=21999891 pruned_infeasible=84
[tsp-debug] progress: expanded=22100000 created=44199915 depth=29 bound=1602 best=1618 pruned_bound=22099896 pruned_infeasible=84
[tsp-debug] progress: expanded=22200000 created=44399915 depth=26 bound=1598 best=1618 pruned_bound=22199899 pruned_infeasible=84
[tsp-debug] progress: expanded=22300000 created=44599915 depth=42 bound=1595 best=1618 pruned_bound=22299887 pruned_infeasible=84
[tsp-debug] progress: expanded=22400000 created=44799915 depth=35 bound=1617 best=1618 pruned_bound=22399894 pruned_infeasible=84
[tsp-debug] progress: expanded=22500000 created=44999915 depth=31 bound=1607 best=1618 pruned_bound=22499899 pruned_infeasible=84
[tsp-debug] progress: expanded=22600000 created=45199915 depth=37 bound=1606 best=1618 pruned_bound=22599889 pruned_infeasible=84
[tsp-debug] progress: expanded=22700000 created=45399915 depth=44 bound=1600 best=1618 pruned_bound=22699889 pruned_infeasible=84
[tsp-debug] progress: expanded=22800000 created=45599915 depth=38 bound=1591 best=1618 pruned_bound=22799898 pruned_infeasible=84
[tsp-debug] progress: expanded=22900000 created=45799915 depth=36 bound=1607 best=1618 pruned_bound=22899892 pruned_infeasible=84
[tsp-debug] progress: expanded=23000000 created=45999915 depth=42 bound=1616 best=1618 pruned_bound=22999893 pruned_infeasible=84
[tsp-debug] progress: expanded=23100000 created=46199915 depth=27 bound=1583 best=1618 pruned_bound=23099897 pruned_infeasible=84
[tsp-debug] progress: expanded=23200000 created=46399915 depth=44 bound=1599 best=1618 pruned_bound=23199890 pruned_infeasible=84
[tsp-debug] progress: expanded=23300000 created=46599915 depth=35 bound=1596 best=1618 pruned_bound=23299891 pruned_infeasible=84
[tsp-debug] progress: expanded=23400000 created=46799915 depth=25 bound=1609 best=1618 pruned_bound=23399900 pruned_infeasible=84
[tsp-debug] progress: expanded=23500000 created=46999915 depth=26 bound=1591 best=1618 pruned_bound=23499900 pruned_infeasible=84
[tsp-debug] progress: expanded=23600000 created=47199915 depth=42 bound=1588 best=1618 pruned_bound=23599890 pruned_infeasible=84
[tsp-debug] progress: expanded=23700000 created=47399915 depth=29 bound=1605 best=1618 pruned_bound=23699896 pruned_infeasible=84
[tsp-debug] progress: expanded=23800000 created=47599915 depth=42 bound=1600 best=1618 pruned_bound=23799887 pruned_infeasible=84
[tsp-debug] progress: expanded=23900000 created=47799915 depth=38 bound=1605 best=1618 pruned_bound=23899889 pruned_infeasible=84
[tsp-debug] progress: expanded=24000000 created=47999915 depth=37 bound=1606 best=1618 pruned_bound=23999888 pruned_infeasible=84
[tsp-debug] progress: expanded=24100000 created=48199915 depth=31 bound=1598 best=1618 pruned_bound=24099897 pruned_infeasible=84
[tsp-debug] progress: expanded=24200000 created=48399915 depth=40 bound=1607 best=1618 pruned_bound=24199893 pruned_infeasible=84
[tsp-debug] progress: expanded=24300000 created=48599915 depth=37 bound=1602 best=1618 pruned_bound=24299898 pruned_infeasible=84
[tsp-debug] progress: expanded=24400000 created=48799915 depth=44 bound=1579 best=1618 pruned_bound=24399899 pruned_infeasible=84
[tsp-debug] progress: expanded=24500000 created=48999915 depth=37 bound=1597 best=1618 pruned_bound=24499890 pruned_infeasible=84
[tsp-debug] progress: expanded=24600000 created=49199915 depth=33 bound=1608 best=1618 pruned_bound=24599899 pruned_infeasible=84
[tsp-debug] progress: expanded=24700000 created=49399915 depth=43 bound=1611 best=1618 pruned_bound=24699890 pruned_infeasible=84
[tsp-debug] progress: expanded=24800000 created=49599915 depth=45 bound=1609 best=1618 pruned_bound=24799887 pruned_infeasible=84
[tsp-debug] progress: expanded=24900000 created=49799915 depth=43 bound=1598 best=1618 pruned_bound=24899895 pruned_infeasible=84
[tsp-debug] progress: expanded=25000000 created=49999915 depth=21 bound=1555 best=1618 pruned_bound=24999902 pruned_infeasible=84
[tsp-debug] progress: expanded=25100000 created=50199915 depth=49 bound=1608 best=1618 pruned_bound=25099884 pruned_infeasible=84
[tsp-debug] progress: expanded=25200000 created=50399915 depth=37 bound=1614 best=1618 pruned_bound=25199886 pruned_infeasible=84
[tsp-debug] progress: expanded=25300000 created=50599915 depth=47 bound=1609 best=1618 pruned_bound=25299875 pruned_infeasible=84
[tsp-debug] progress: expanded=25400000 created=50799915 depth=22 bound=1602 best=1618 pruned_bound=25399905 pruned_infeasible=84
[tsp-debug] progress: expanded=25500000 created=50999915 depth=36 bound=1582 best=1618 pruned_bound=25499893 pruned_infeasible=84
[tsp-debug] progress: expanded=25600000 created=51199915 depth=34 bound=1610 best=1618 pruned_bound=25599895 pruned_infeasible=84
[tsp-debug] progress: expanded=25700000 created=51399915 depth=35 bound=1615 best=1618 pruned_bound=25699896 pruned_infeasible=84
[tsp-debug] progress: expanded=25800000 created=51599915 depth=36 bound=1586 best=1618 pruned_bound=25799906 pruned_infeasible=84
[tsp-debug] progress: expanded=25900000 created=51799915 depth=40 bound=1614 best=1618 pruned_bound=25899890 pruned_infeasible=84
[tsp-debug] progress: expanded=26000000 created=51999915 depth=34 bound=1617 best=1618 pruned_bound=25999896 pruned_infeasible=84
[tsp-debug] progress: expanded=26100000 created=52199914 depth=34 bound=1615 best=1618 pruned_bound=26099891 pruned_infeasible=85
[tsp-debug] progress: expanded=26200000 created=52399914 depth=26 bound=1603 best=1618 pruned_bound=26199900 pruned_infeasible=85
[tsp-debug] progress: expanded=26300000 created=52599914 depth=42 bound=1600 best=1618 pruned_bound=26299886 pruned_infeasible=85
[tsp-debug] progress: expanded=26400000 created=52799914 depth=37 bound=1603 best=1618 pruned_bound=26399896 pruned_infeasible=85
[tsp-debug] progress: expanded=26500000 created=52999914 depth=34 bound=1617 best=1618 pruned_bound=26499889 pruned_infeasible=85
[tsp-debug] progress: expanded=26600000 created=53199914 depth=49 bound=1605 best=1618 pruned_bound=26599872 pruned_infeasible=85
[tsp-debug] progress: expanded=26700000 created=53399914 depth=32 bound=1609 best=1618 pruned_bound=26699895 pruned_infeasible=85
[tsp-debug] progress: expanded=26800000 created=53599914 depth=22 bound=1605 best=1618 pruned_bound=26799903 pruned_infeasible=85
[tsp-debug] progress: expanded=26900000 created=53799914 depth=26 bound=1612 best=1618 pruned_bound=26899900 pruned_infeasible=85
[tsp-debug] progress: expanded=27000000 created=53999914 depth=32 bound=1607 best=1618 pruned_bound=26999895 pruned_infeasible=85
[tsp-debug] progress: expanded=27100000 created=54199914 depth=41 bound=1612 best=1618 pruned_bound=27099882 pruned_infeasible=85
[tsp-debug] progress: expanded=27200000 created=54399914 depth=42 bound=1608 best=1618 pruned_bound=27199886 pruned_infeasible=85
[tsp-debug] progress: expanded=27300000 created=54599914 depth=40 bound=1613 best=1618 pruned_bound=27299898 pruned_infeasible=85
[tsp-debug] progress: expanded=27400000 created=54799914 depth=35 bound=1615 best=1618 pruned_bound=27399893 pruned_infeasible=85
[tsp-debug] progress: expanded=27500000 created=54999914 depth=54 bound=1593 best=1618 pruned_bound=27499885 pruned_infeasible=85
[tsp-debug] progress: expanded=27600000 created=55199914 depth=36 bound=1587 best=1618 pruned_bound=27599898 pruned_infeasible=85
[tsp-debug] progress: expanded=27700000 created=55399914 depth=40 bound=1595 best=1618 pruned_bound=27699889 pruned_infeasible=85
[tsp-debug] progress: expanded=27800000 created=55599914 depth=30 bound=1561 best=1618 pruned_bound=27799897 pruned_infeasible=85
[tsp-debug] progress: expanded=27900000 created=55799914 depth=39 bound=1579 best=1618 pruned_bound=27899891 pruned_infeasible=85
[tsp-debug] progress: expanded=28000000 created=55999914 depth=35 bound=1612 best=1618 pruned_bound=27999894 pruned_infeasible=85
[tsp-debug] progress: expanded=28100000 created=56199914 depth=18 bound=1505 best=1618 pruned_bound=28099904 pruned_infeasible=85
[tsp-debug] progress: expanded=28200000 created=56399914 depth=45 bound=1613 best=1618 pruned_bound=28199888 pruned_infeasible=85
[tsp-debug] progress: expanded=28300000 created=56599914 depth=28 bound=1590 best=1618 pruned_bound=28299896 pruned_infeasible=85
[tsp-debug] progress: expanded=28400000 created=56799914 depth=33 bound=1612 best=1618 pruned_bound=28399894 pruned_infeasible=85
[tsp-debug] progress: expanded=28500000 created=56999914 depth=32 bound=1611 best=1618 pruned_bound=28499896 pruned_infeasible=85
[tsp-debug] progress: expanded=28600000 created=57199914 depth=36 bound=1594 best=1618 pruned_bound=28599891 pruned_infeasible=85
[tsp-debug] progress: expanded=28700000 created=57399914 depth=40 bound=1617 best=1618 pruned_bound=28699886 pruned_infeasible=85
[tsp-debug] progress: expanded=28800000 created=57599914 depth=35 bound=1603 best=1618 pruned_bound=28799893 pruned_infeasible=85
[tsp-debug] progress: expanded=28900000 created=57799914 depth=30 bound=1614 best=1618 pruned_bound=28899904 pruned_infeasible=85
[tsp-debug] progress: expanded=29000000 created=57999914 depth=47 bound=1613 best=1618 pruned_bound=28999896 pruned_infeasible=85
[tsp-debug] progress: expanded=29100000 created=58199914 depth=35 bound=1597 best=1618 pruned_bound=29099890 pruned_infeasible=85
[tsp-debug] progress: expanded=29200000 created=58399914 depth=30 bound=1590 best=1618 pruned_bound=29199895 pruned_infeasible=85
[tsp-debug] progress: expanded=29300000 created=58599914 depth=41 bound=1608 best=1618 pruned_bound=29299887 pruned_infeasible=85
[tsp-debug] progress: expanded=29400000 created=58799914 depth=40 bound=1611 best=1618 pruned_bound=29399887 pruned_infeasible=85
[tsp-debug] progress: expanded=29500000 created=58999914 depth=32 bound=1616 best=1618 pruned_bound=29499897 pruned_infeasible=85
[tsp-debug] progress: expanded=29600000 created=59199914 depth=39 bound=1616 best=1618 pruned_bound=29599895 pruned_infeasible=85
[tsp-debug] progress: expanded=29700000 created=59399914 depth=26 bound=1565 best=1618 pruned_bound=29699901 pruned_infeasible=85
[tsp-debug] progress: expanded=29800000 created=59599914 depth=25 bound=1610 best=1618 pruned_bound=29799899 pruned_infeasible=85
[tsp-debug] progress: expanded=29900000 created=59799914 depth=37 bound=1614 best=1618 pruned_bound=29899888 pruned_infeasible=85
[tsp-debug] progress: expanded=30000000 created=59999914 depth=46 bound=1590 best=1618 pruned_bound=29999885 pruned_infeasible=85
[tsp-debug] progress: expanded=30100000 created=60199914 depth=29 bound=1597 best=1618 pruned_bound=30099901 pruned_infeasible=85
[tsp-debug] progress: expanded=30200000 created=60399914 depth=26 bound=1605 best=1618 pruned_bound=30199896 pruned_infeasible=85
[tsp-debug] progress: expanded=30300000 created=60599914 depth=28 bound=1571 best=1618 pruned_bound=30299898 pruned_infeasible=85
[tsp-debug] progress: expanded=30400000 created=60799914 depth=39 bound=1609 best=1618 pruned_bound=30399890 pruned_infeasible=85
[tsp-debug] progress: expanded=30500000 created=60999914 depth=35 bound=1565 best=1618 pruned_bound=30499894 pruned_infeasible=85
[tsp-debug] progress: expanded=30600000 created=61199914 depth=28 bound=1615 best=1618 pruned_bound=30599900 pruned_infeasible=85
[tsp-debug] progress: expanded=30700000 created=61399914 depth=25 bound=1545 best=1618 pruned_bound=30699903 pruned_infeasible=85
[tsp-debug] progress: expanded=30800000 created=61599914 depth=34 bound=1572 best=1618 pruned_bound=30799900 pruned_infeasible=85
[tsp-debug] progress: expanded=30900000 created=61799914 depth=31 bound=1616 best=1618 pruned_bound=30899894 pruned_infeasible=85
[tsp-debug] progress: expanded=31000000 created=61999914 depth=37 bound=1594 best=1618 pruned_bound=30999895 pruned_infeasible=85
[tsp-debug] progress: expanded=31100000 created=62199914 depth=35 bound=1607 best=1618 pruned_bound=31099892 pruned_infeasible=85
[tsp-debug] progress: expanded=31200000 created=62399914 depth=44 bound=1603 best=1618 pruned_bound=31199886 pruned_infeasible=85
[tsp-debug] progress: expanded=31300000 created=62599914 depth=49 bound=1606 best=1618 pruned_bound=31299890 pruned_infeasible=85
[tsp-debug] progress: expanded=31400000 created=62799914 depth=33 bound=1603 best=1618 pruned_bound=31399898 pruned_infeasible=85
[tsp-debug] progress: expanded=31500000 created=62999914 depth=48 bound=1613 best=1618 pruned_bound=31499884 pruned_infeasible=85
[tsp-debug] progress: expanded=31600000 created=63199914 depth=44 bound=1600 best=1618 pruned_bound=31599893 pruned_infeasible=85
[tsp-debug] progress: expanded=31700000 created=63399914 depth=36 bound=1607 best=1618 pruned_bound=31699892 pruned_infeasible=85
[tsp-debug] progress: expanded=31800000 created=63599914 depth=39 bound=1611 best=1618 pruned_bound=31799891 pruned_infeasible=85
[tsp-debug] progress: expanded=31900000 created=63799914 depth=35 bound=1577 best=1618 pruned_bound=31899894 pruned_infeasible=85
[tsp-debug] progress: expanded=32000000 created=63999914 depth=41 bound=1615 best=1618 pruned_bound=31999900 pruned_infeasible=85
[tsp-debug] progress: expanded=32100000 created=64199914 depth=37 bound=1587 best=1618 pruned_bound=32099893 pruned_infeasible=85
[tsp-debug] progress: expanded=32200000 created=64399914 depth=27 bound=1586 best=1618 pruned_bound=32199899 pruned_infeasible=85
[tsp-debug] progress: expanded=32300000 created=64599914 depth=34 bound=1605 best=1618 pruned_bound=32299898 pruned_infeasible=85
[tsp-debug] progress: expanded=32400000 created=64799914 depth=28 bound=1578 best=1618 pruned_bound=32399895 pruned_infeasible=85
[tsp-debug] progress: expanded=32500000 created=64999914 depth=27 bound=1612 best=1618 pruned_bound=32499900 pruned_infeasible=85
[tsp-debug] progress: expanded=32600000 created=65199914 depth=41 bound=1605 best=1618 pruned_bound=32599892 pruned_infeasible=85
[tsp-debug] progress: expanded=32700000 created=65399914 depth=37 bound=1604 best=1618 pruned_bound=32699890 pruned_infeasible=85
[tsp-debug] progress: expanded=32800000 created=65599914 depth=33 bound=1615 best=1618 pruned_bound=32799895 pruned_infeasible=85
[tsp-debug] progress: expanded=32900000 created=65799914 depth=24 bound=1597 best=1618 pruned_bound=32899900 pruned_infeasible=85
[tsp-debug] progress: expanded=33000000 created=65999914 depth=34 bound=1534 best=1618 pruned_bound=32999894 pruned_infeasible=85
[tsp-debug] progress: expanded=33100000 created=66199914 depth=38 bound=1576 best=1618 pruned_bound=33099896 pruned_infeasible=85
[tsp-debug] progress: expanded=33200000 created=66399914 depth=29 bound=1589 best=1618 pruned_bound=33199897 pruned_infeasible=85
[tsp-debug] progress: expanded=33300000 created=66599914 depth=40 bound=1612 best=1618 pruned_bound=33299886 pruned_infeasible=85
[tsp-debug] progress: expanded=33400000 created=66799914 depth=35 bound=1605 best=1618 pruned_bound=33399898 pruned_infeasible=85
[tsp-debug] progress: expanded=33500000 created=66999914 depth=38 bound=1608 best=1618 pruned_bound=33499896 pruned_infeasible=85
[tsp-debug] progress: expanded=33600000 created=67199914 depth=36 bound=1607 best=1618 pruned_bound=33599891 pruned_infeasible=85
[tsp-debug] progress: expanded=33700000 created=67399914 depth=35 bound=1610 best=1618 pruned_bound=33699893 pruned_infeasible=85
[tsp-debug] progress: expanded=33800000 created=67599914 depth=25 bound=1594 best=1618 pruned_bound=33799903 pruned_infeasible=85
[tsp-debug] progress: expanded=33900000 created=67799914 depth=41 bound=1611 best=1618 pruned_bound=33899888 pruned_infeasible=85
[tsp-debug] progress: expanded=34000000 created=67999914 depth=26 bound=1559 best=1618 pruned_bound=33999903 pruned_infeasible=85
[tsp-debug] progress: expanded=34100000 created=68199914 depth=32 bound=1588 best=1618 pruned_bound=34099894 pruned_infeasible=85
[tsp-debug] progress: expanded=34200000 created=68399914 depth=44 bound=1597 best=1618 pruned_bound=34199892 pruned_infeasible=85
[tsp-debug] progress: expanded=34300000 created=68599914 depth=39 bound=1590 best=1618 pruned_bound=34299897 pruned_infeasible=85
[tsp-debug] progress: expanded=34400000 created=68799914 depth=29 bound=1568 best=1618 pruned_bound=34399904 pruned_infeasible=85
[tsp-debug] progress: expanded=34500000 created=68999914 depth=54 bound=1616 best=1618 pruned_bound=34499882 pruned_infeasible=85
[tsp-debug] progress: expanded=34600000 created=69199914 depth=38 bound=1577 best=1618 pruned_bound=34599891 pruned_infeasible=85
[tsp-debug] progress: expanded=34700000 created=69399914 depth=29 bound=1616 best=1618 pruned_bound=34699900 pruned_infeasible=85
[tsp-debug] progress: expanded=34800000 created=69599914 depth=35 bound=1606 best=1618 pruned_bound=34799897 pruned_infeasible=85
[tsp-debug] progress: expanded=34900000 created=69799914 depth=29 bound=1584 best=1618 pruned_bound=34899903 pruned_infeasible=85
[tsp-debug] progress: expanded=35000000 created=69999914 depth=33 bound=1588 best=1618 pruned_bound=34999896 pruned_infeasible=85
[tsp-debug] progress: expanded=35100000 created=70199914 depth=33 bound=1608 best=1618 pruned_bound=35099896 pruned_infeasible=85
[tsp-debug] progress: expanded=35200000 created=70399914 depth=40 bound=1596 best=1618 pruned_bound=35199894 pruned_infeasible=85
[tsp-debug] progress: expanded=35300000 created=70599914 depth=41 bound=1568 best=1618 pruned_bound=35299893 pruned_infeasible=85
[tsp-debug] progress: expanded=35400000 created=70799914 depth=38 bound=1601 best=1618 pruned_bound=35399905 pruned_infeasible=85
[tsp-debug] progress: expanded=35500000 created=70999914 depth=28 bound=1614 best=1618 pruned_bound=35499900 pruned_infeasible=85
[tsp-debug] progress: expanded=35600000 created=71199914 depth=28 bound=1587 best=1618 pruned_bound=35599900 pruned_infeasible=85
[tsp-debug] progress: expanded=35700000 created=71399914 depth=40 bound=1597 best=1618 pruned_bound=35699899 pruned_infeasible=85
[tsp-debug] progress: expanded=35800000 created=71599914 depth=33 bound=1597 best=1618 pruned_bound=35799897 pruned_infeasible=85
[tsp-debug] progress: expanded=35900000 created=71799914 depth=32 bound=1616 best=1618 pruned_bound=35899897 pruned_infeasible=85
[tsp-debug] progress: expanded=36000000 created=71999914 depth=29 bound=1611 best=1618 pruned_bound=35999905 pruned_infeasible=85
[tsp-debug] progress: expanded=36100000 created=72199914 depth=32 bound=1607 best=1618 pruned_bound=36099903 pruned_infeasible=85
[tsp-debug] progress: expanded=36200000 created=72399914 depth=38 bound=1611 best=1618 pruned_bound=36199896 pruned_infeasible=85
[tsp-debug] progress: expanded=36300000 created=72599914 depth=41 bound=1609 best=1618 pruned_bound=36299897 pruned_infeasible=85
[tsp-debug] progress: expanded=36400000 created=72799914 depth=44 bound=1615 best=1618 pruned_bound=36399905 pruned_infeasible=85
[tsp-debug] progress: expanded=36500000 created=72999914 depth=47 bound=1615 best=1618 pruned_bound=36499897 pruned_infeasible=85
[tsp-debug] progress: expanded=36600000 created=73199914 depth=30 bound=1608 best=1618 pruned_bound=36599889 pruned_infeasible=85
[tsp-debug] progress: expanded=36700000 created=73399914 depth=32 bound=1612 best=1618 pruned_bound=36699893 pruned_infeasible=85
[tsp-debug] progress: expanded=36800000 created=73599914 depth=24 bound=1570 best=1618 pruned_bound=36799898 pruned_infeasible=85
[tsp-debug] progress: expanded=36900000 created=73799913 depth=43 bound=1616 best=1618 pruned_bound=36899885 pruned_infeasible=86
[tsp-debug] progress: expanded=37000000 created=73999913 depth=18 bound=1499 best=1618 pruned_bound=36999904 pruned_infeasible=86
[tsp-debug] progress: expanded=37100000 created=74199897 depth=25 bound=1560 best=1618 pruned_bound=37099880 pruned_infeasible=102
[tsp-debug] progress: expanded=37200000 created=74399896 depth=35 bound=1607 best=1618 pruned_bound=37199877 pruned_infeasible=103
[tsp-debug] progress: expanded=37300000 created=74599893 depth=34 bound=1610 best=1618 pruned_bound=37299870 pruned_infeasible=106
[tsp-debug] progress: expanded=37400000 created=74799893 depth=31 bound=1612 best=1618 pruned_bound=37399873 pruned_infeasible=106
[tsp-debug] progress: expanded=37500000 created=74999891 depth=30 bound=1612 best=1618 pruned_bound=37499871 pruned_infeasible=108
[tsp-debug] progress: expanded=37600000 created=75199891 depth=26 bound=1601 best=1618 pruned_bound=37599874 pruned_infeasible=108
[tsp-debug] progress: expanded=37700000 created=75399891 depth=20 bound=1543 best=1618 pruned_bound=37699881 pruned_infeasible=108
[tsp-debug] new incumbent: cost=1615 source=recursive-node depth=39
[tsp-debug] progress: expanded=37800000 created=75599887 depth=32 bound=1599 best=1615 pruned_bound=37799866 pruned_infeasible=110
[tsp-debug] progress: expanded=37900000 created=75799887 depth=30 bound=1587 best=1615 pruned_bound=37899869 pruned_infeasible=110
[tsp-debug] progress: expanded=38000000 created=75999887 depth=41 bound=1601 best=1615 pruned_bound=37999864 pruned_infeasible=110
[tsp-debug] progress: expanded=38100000 created=76199887 depth=20 bound=1564 best=1615 pruned_bound=38099876 pruned_infeasible=110
[tsp-debug] progress: expanded=38200000 created=76399887 depth=25 bound=1595 best=1615 pruned_bound=38199877 pruned_infeasible=110
[tsp-debug] progress: expanded=38300000 created=76599887 depth=32 bound=1595 best=1615 pruned_bound=38299868 pruned_infeasible=110
[tsp-debug] progress: expanded=38400000 created=76799887 depth=31 bound=1609 best=1615 pruned_bound=38399864 pruned_infeasible=110
[tsp-debug] progress: expanded=38500000 created=76999887 depth=23 bound=1554 best=1615 pruned_bound=38499872 pruned_infeasible=110
[tsp-debug] progress: expanded=38600000 created=77199887 depth=23 bound=1588 best=1615 pruned_bound=38599873 pruned_infeasible=110
[tsp-debug] progress: expanded=38700000 created=77399887 depth=27 bound=1606 best=1615 pruned_bound=38699873 pruned_infeasible=110
[tsp-debug] progress: expanded=38800000 created=77599887 depth=29 bound=1599 best=1615 pruned_bound=38799870 pruned_infeasible=110
[tsp-debug] progress: expanded=38900000 created=77799887 depth=23 bound=1610 best=1615 pruned_bound=38899880 pruned_infeasible=110
[tsp-debug] progress: expanded=39000000 created=77999887 depth=29 bound=1614 best=1615 pruned_bound=38999872 pruned_infeasible=110
[tsp-debug] progress: expanded=39100000 created=78199887 depth=25 bound=1599 best=1615 pruned_bound=39099872 pruned_infeasible=110
[tsp-debug] progress: expanded=39200000 created=78399887 depth=32 bound=1601 best=1615 pruned_bound=39199872 pruned_infeasible=110
[tsp-debug] progress: expanded=39300000 created=78599887 depth=24 bound=1602 best=1615 pruned_bound=39299874 pruned_infeasible=110
[tsp-debug] progress: expanded=39400000 created=78799887 depth=25 bound=1573 best=1615 pruned_bound=39399873 pruned_infeasible=110
[tsp-debug] progress: expanded=39500000 created=78999887 depth=36 bound=1610 best=1615 pruned_bound=39499868 pruned_infeasible=110
[tsp-debug] progress: expanded=39600000 created=79199887 depth=27 bound=1612 best=1615 pruned_bound=39599870 pruned_infeasible=110
[tsp-debug] progress: expanded=39700000 created=79399887 depth=28 bound=1612 best=1615 pruned_bound=39699873 pruned_infeasible=110
[tsp-debug] progress: expanded=39800000 created=79599887 depth=39 bound=1612 best=1615 pruned_bound=39799862 pruned_infeasible=110
[tsp-debug] progress: expanded=39900000 created=79799887 depth=35 bound=1600 best=1615 pruned_bound=39899871 pruned_infeasible=110
[tsp-debug] progress: expanded=40000000 created=79999887 depth=42 bound=1607 best=1615 pruned_bound=39999876 pruned_infeasible=110
[tsp-debug] progress: expanded=40100000 created=80199887 depth=37 bound=1588 best=1615 pruned_bound=40099860 pruned_infeasible=110
[tsp-debug] progress: expanded=40200000 created=80399887 depth=32 bound=1608 best=1615 pruned_bound=40199873 pruned_infeasible=110
[tsp-debug] progress: expanded=40300000 created=80599887 depth=26 bound=1548 best=1615 pruned_bound=40299872 pruned_infeasible=110
[tsp-debug] progress: expanded=40400000 created=80799887 depth=40 bound=1606 best=1615 pruned_bound=40399862 pruned_infeasible=110
[tsp-debug] progress: expanded=40500000 created=80999887 depth=50 bound=1611 best=1615 pruned_bound=40499856 pruned_infeasible=110
[tsp-debug] progress: expanded=40600000 created=81199887 depth=28 bound=1591 best=1615 pruned_bound=40599870 pruned_infeasible=110
[tsp-debug] progress: expanded=40700000 created=81399887 depth=25 bound=1539 best=1615 pruned_bound=40699872 pruned_infeasible=110
[tsp-debug] progress: expanded=40800000 created=81599887 depth=42 bound=1611 best=1615 pruned_bound=40799857 pruned_infeasible=110
[tsp-debug] progress: expanded=40900000 created=81799886 depth=25 bound=1575 best=1615 pruned_bound=40899873 pruned_infeasible=111
[tsp-debug] progress: expanded=41000000 created=81999885 depth=39 bound=1597 best=1615 pruned_bound=40999859 pruned_infeasible=112
[tsp-debug] progress: expanded=41100000 created=82199885 depth=30 bound=1596 best=1615 pruned_bound=41099871 pruned_infeasible=112
[tsp-debug] progress: expanded=41200000 created=82399885 depth=29 bound=1593 best=1615 pruned_bound=41199866 pruned_infeasible=112
[tsp-debug] progress: expanded=41300000 created=82599885 depth=31 bound=1612 best=1615 pruned_bound=41299866 pruned_infeasible=112
[tsp-debug] progress: expanded=41400000 created=82799885 depth=35 bound=1587 best=1615 pruned_bound=41399868 pruned_infeasible=112
[tsp-debug] progress: expanded=41500000 created=82999885 depth=29 bound=1596 best=1615 pruned_bound=41499870 pruned_infeasible=112
[tsp-debug] progress: expanded=41600000 created=83199885 depth=39 bound=1614 best=1615 pruned_bound=41599860 pruned_infeasible=112
[tsp-debug] progress: expanded=41700000 created=83399885 depth=26 bound=1573 best=1615 pruned_bound=41699876 pruned_infeasible=112
[tsp-debug] progress: expanded=41800000 created=83599885 depth=32 bound=1610 best=1615 pruned_bound=41799863 pruned_infeasible=112
[tsp-debug] progress: expanded=41900000 created=83799885 depth=29 bound=1594 best=1615 pruned_bound=41899862 pruned_infeasible=112
[tsp-debug] progress: expanded=42000000 created=83999885 depth=49 bound=1605 best=1615 pruned_bound=41999846 pruned_infeasible=112
[tsp-debug] progress: expanded=42100000 created=84199883 depth=31 bound=1614 best=1615 pruned_bound=42099866 pruned_infeasible=114
[tsp-debug] progress: expanded=42200000 created=84399883 depth=32 bound=1602 best=1615 pruned_bound=42199868 pruned_infeasible=114
[tsp-debug] progress: expanded=42300000 created=84599883 depth=31 bound=1583 best=1615 pruned_bound=42299863 pruned_infeasible=114
[tsp-debug] progress: expanded=42400000 created=84799883 depth=33 bound=1612 best=1615 pruned_bound=42399865 pruned_infeasible=114
[tsp-debug] progress: expanded=42500000 created=84999883 depth=37 bound=1595 best=1615 pruned_bound=42499864 pruned_infeasible=114
[tsp-debug] progress: expanded=42600000 created=85199881 depth=25 bound=1551 best=1615 pruned_bound=42599866 pruned_infeasible=116
[tsp-debug] progress: expanded=42700000 created=85399881 depth=79 bound=1607 best=1615 pruned_bound=42699836 pruned_infeasible=116
[tsp-debug] progress: expanded=42800000 created=85599881 depth=31 bound=1536 best=1615 pruned_bound=42799861 pruned_infeasible=116
[tsp-debug] progress: expanded=42900000 created=85799881 depth=41 bound=1605 best=1615 pruned_bound=42899855 pruned_infeasible=116
[tsp-debug] progress: expanded=43000000 created=85999881 depth=44 bound=1604 best=1615 pruned_bound=42999857 pruned_infeasible=116
[tsp-debug] progress: expanded=43100000 created=86199881 depth=38 bound=1599 best=1615 pruned_bound=43099863 pruned_infeasible=116
[tsp-debug] progress: expanded=43200000 created=86399881 depth=28 bound=1573 best=1615 pruned_bound=43199867 pruned_infeasible=116
[tsp-debug] progress: expanded=43300000 created=86599881 depth=31 bound=1605 best=1615 pruned_bound=43299862 pruned_infeasible=116
[tsp-debug] progress: expanded=43400000 created=86799880 depth=26 bound=1609 best=1615 pruned_bound=43399867 pruned_infeasible=117
[tsp-debug] progress: expanded=43500000 created=86999880 depth=39 bound=1612 best=1615 pruned_bound=43499859 pruned_infeasible=117
[tsp-debug] progress: expanded=43600000 created=87199880 depth=29 bound=1611 best=1615 pruned_bound=43599863 pruned_infeasible=117
[tsp-debug] progress: expanded=43700000 created=87399880 depth=32 bound=1610 best=1615 pruned_bound=43699860 pruned_infeasible=117
[tsp-debug] progress: expanded=43800000 created=87599880 depth=32 bound=1597 best=1615 pruned_bound=43799862 pruned_infeasible=117
[tsp-debug] progress: expanded=43900000 created=87799880 depth=35 bound=1594 best=1615 pruned_bound=43899859 pruned_infeasible=117
[tsp-debug] progress: expanded=44000000 created=87999880 depth=25 bound=1605 best=1615 pruned_bound=43999868 pruned_infeasible=117
[tsp-debug] progress: expanded=44100000 created=88199880 depth=37 bound=1612 best=1615 pruned_bound=44099851 pruned_infeasible=117
[tsp-debug] progress: expanded=44200000 created=88399879 depth=28 bound=1608 best=1615 pruned_bound=44199862 pruned_infeasible=118
[tsp-debug] progress: expanded=44300000 created=88599876 depth=27 bound=1593 best=1615 pruned_bound=44299867 pruned_infeasible=121
[tsp-debug] progress: expanded=44400000 created=88799875 depth=25 bound=1585 best=1615 pruned_bound=44399860 pruned_infeasible=122
[tsp-debug] progress: expanded=44500000 created=88999875 depth=36 bound=1590 best=1615 pruned_bound=44499858 pruned_infeasible=122
[tsp-debug] progress: expanded=44600000 created=89199874 depth=37 bound=1609 best=1615 pruned_bound=44599858 pruned_infeasible=123
[tsp-debug] progress: expanded=44700000 created=89399874 depth=27 bound=1590 best=1615 pruned_bound=44699863 pruned_infeasible=123
[tsp-debug] progress: expanded=44800000 created=89599874 depth=28 bound=1607 best=1615 pruned_bound=44799870 pruned_infeasible=123
[tsp-debug] progress: expanded=44900000 created=89799874 depth=25 bound=1584 best=1615 pruned_bound=44899859 pruned_infeasible=123
[tsp-debug] progress: expanded=45000000 created=89999874 depth=40 bound=1607 best=1615 pruned_bound=44999850 pruned_infeasible=123
[tsp-debug] progress: expanded=45100000 created=90199874 depth=35 bound=1605 best=1615 pruned_bound=45099851 pruned_infeasible=123
[tsp-debug] progress: expanded=45200000 created=90399874 depth=37 bound=1614 best=1615 pruned_bound=45199849 pruned_infeasible=123
[tsp-debug] progress: expanded=45300000 created=90599874 depth=25 bound=1548 best=1615 pruned_bound=45299863 pruned_infeasible=123
[tsp-debug] progress: expanded=45400000 created=90799874 depth=36 bound=1599 best=1615 pruned_bound=45399857 pruned_infeasible=123
[tsp-debug] progress: expanded=45500000 created=90999874 depth=30 bound=1586 best=1615 pruned_bound=45499857 pruned_infeasible=123
[tsp-debug] progress: expanded=45600000 created=91199874 depth=29 bound=1612 best=1615 pruned_bound=45599852 pruned_infeasible=123
[tsp-debug] progress: expanded=45700000 created=91399873 depth=37 bound=1608 best=1615 pruned_bound=45699853 pruned_infeasible=124
[tsp-debug] progress: expanded=45800000 created=91599873 depth=28 bound=1600 best=1615 pruned_bound=45799860 pruned_infeasible=124
[tsp-debug] progress: expanded=45900000 created=91799873 depth=23 bound=1599 best=1615 pruned_bound=45899863 pruned_infeasible=124
[tsp-debug] progress: expanded=46000000 created=91999851 depth=41 bound=1606 best=1615 pruned_bound=45999834 pruned_infeasible=146
[tsp-debug] progress: expanded=46100000 created=92199851 depth=31 bound=1603 best=1615 pruned_bound=46099831 pruned_infeasible=146
[tsp-debug] progress: expanded=46200000 created=92399851 depth=34 bound=1591 best=1615 pruned_bound=46199829 pruned_infeasible=146
[tsp-debug] progress: expanded=46300000 created=92599851 depth=26 bound=1603 best=1615 pruned_bound=46299839 pruned_infeasible=146
[tsp-debug] progress: expanded=46400000 created=92799851 depth=27 bound=1609 best=1615 pruned_bound=46399837 pruned_infeasible=146
[tsp-debug] progress: expanded=46500000 created=92999851 depth=27 bound=1527 best=1615 pruned_bound=46499835 pruned_infeasible=146
[tsp-debug] progress: expanded=46600000 created=93199846 depth=29 bound=1606 best=1615 pruned_bound=46599835 pruned_infeasible=151
[tsp-debug] progress: expanded=46700000 created=93399846 depth=34 bound=1585 best=1615 pruned_bound=46699829 pruned_infeasible=151
[tsp-debug] progress: expanded=46800000 created=93599844 depth=33 bound=1599 best=1615 pruned_bound=46799833 pruned_infeasible=153
[tsp-debug] progress: expanded=46900000 created=93799844 depth=28 bound=1607 best=1615 pruned_bound=46899827 pruned_infeasible=153
[tsp-debug] progress: expanded=47000000 created=93999844 depth=30 bound=1594 best=1615 pruned_bound=46999823 pruned_infeasible=153
[tsp-debug] progress: expanded=47100000 created=94199844 depth=40 bound=1612 best=1615 pruned_bound=47099822 pruned_infeasible=153
[tsp-debug] progress: expanded=47200000 created=94399844 depth=23 bound=1560 best=1615 pruned_bound=47199834 pruned_infeasible=153
[tsp-debug] progress: expanded=47300000 created=94599844 depth=31 bound=1611 best=1615 pruned_bound=47299824 pruned_infeasible=153
[tsp-debug] progress: expanded=47400000 created=94799844 depth=22 bound=1598 best=1615 pruned_bound=47399836 pruned_infeasible=153
[tsp-debug] progress: expanded=47500000 created=94999843 depth=29 bound=1612 best=1615 pruned_bound=47499826 pruned_infeasible=154
[tsp-debug] progress: expanded=47600000 created=95199843 depth=48 bound=1614 best=1615 pruned_bound=47599815 pruned_infeasible=154
[tsp-debug] progress: expanded=47700000 created=95399843 depth=36 bound=1596 best=1615 pruned_bound=47699823 pruned_infeasible=154
[tsp-debug] progress: expanded=47800000 created=95599843 depth=36 bound=1599 best=1615 pruned_bound=47799819 pruned_infeasible=154
[tsp-debug] progress: expanded=47900000 created=95799843 depth=39 bound=1596 best=1615 pruned_bound=47899817 pruned_infeasible=154
[tsp-debug] progress: expanded=48000000 created=95999843 depth=24 bound=1608 best=1615 pruned_bound=47999830 pruned_infeasible=154
[tsp-debug] progress: expanded=48100000 created=96199843 depth=32 bound=1607 best=1615 pruned_bound=48099829 pruned_infeasible=154
[tsp-debug] progress: expanded=48200000 created=96399843 depth=35 bound=1571 best=1615 pruned_bound=48199830 pruned_infeasible=154
[tsp-debug] progress: expanded=48300000 created=96599843 depth=31 bound=1589 best=1615 pruned_bound=48299827 pruned_infeasible=154
[tsp-debug] progress: expanded=48400000 created=96799843 depth=39 bound=1605 best=1615 pruned_bound=48399822 pruned_infeasible=154
[tsp-debug] progress: expanded=48500000 created=96999843 depth=32 bound=1596 best=1615 pruned_bound=48499821 pruned_infeasible=154
[tsp-debug] progress: expanded=48600000 created=97199843 depth=28 bound=1596 best=1615 pruned_bound=48599828 pruned_infeasible=154
[tsp-debug] progress: expanded=48700000 created=97399843 depth=27 bound=1598 best=1615 pruned_bound=48699829 pruned_infeasible=154
[tsp-debug] progress: expanded=48800000 created=97599843 depth=26 bound=1614 best=1615 pruned_bound=48799826 pruned_infeasible=154
[tsp-debug] progress: expanded=48900000 created=97799843 depth=34 bound=1580 best=1615 pruned_bound=48899819 pruned_infeasible=154
[tsp-debug] progress: expanded=49000000 created=97999843 depth=28 bound=1609 best=1615 pruned_bound=48999828 pruned_infeasible=154
[tsp-debug] progress: expanded=49100000 created=98199843 depth=27 bound=1599 best=1615 pruned_bound=49099829 pruned_infeasible=154
[tsp-debug] progress: expanded=49200000 created=98399843 depth=29 bound=1594 best=1615 pruned_bound=49199824 pruned_infeasible=154
[tsp-debug] progress: expanded=49300000 created=98599843 depth=33 bound=1614 best=1615 pruned_bound=49299824 pruned_infeasible=154
[tsp-debug] progress: expanded=49400000 created=98799843 depth=27 bound=1593 best=1615 pruned_bound=49399833 pruned_infeasible=154
[tsp-debug] progress: expanded=49500000 created=98999843 depth=28 bound=1612 best=1615 pruned_bound=49499828 pruned_infeasible=154
[tsp-debug] progress: expanded=49600000 created=99199843 depth=26 bound=1585 best=1615 pruned_bound=49599830 pruned_infeasible=154
[tsp-debug] progress: expanded=49700000 created=99399838 depth=36 bound=1608 best=1615 pruned_bound=49699821 pruned_infeasible=159
[tsp-debug] progress: expanded=49800000 created=99599837 depth=27 bound=1599 best=1615 pruned_bound=49799822 pruned_infeasible=160
[tsp-debug] progress: expanded=49900000 created=99799837 depth=31 bound=1603 best=1615 pruned_bound=49899821 pruned_infeasible=160
[tsp-debug] progress: expanded=50000000 created=99999837 depth=20 bound=1612 best=1615 pruned_bound=49999828 pruned_infeasible=160
[tsp-debug] progress: expanded=50100000 created=100199825 depth=32 bound=1581 best=1615 pruned_bound=50099801 pruned_infeasible=172
[tsp-debug] progress: expanded=50200000 created=100399825 depth=21 bound=1594 best=1615 pruned_bound=50199812 pruned_infeasible=172
[tsp-debug] progress: expanded=50300000 created=100599825 depth=34 bound=1614 best=1615 pruned_bound=50299804 pruned_infeasible=172
[tsp-debug] progress: expanded=50400000 created=100799825 depth=29 bound=1601 best=1615 pruned_bound=50399810 pruned_infeasible=172
[tsp-debug] progress: expanded=50500000 created=100999825 depth=26 bound=1613 best=1615 pruned_bound=50499812 pruned_infeasible=172
[tsp-debug] progress: expanded=50600000 created=101199824 depth=26 bound=1586 best=1615 pruned_bound=50599813 pruned_infeasible=173
[tsp-debug] progress: expanded=50700000 created=101399824 depth=54 bound=1604 best=1615 pruned_bound=50699786 pruned_infeasible=173
[tsp-debug] progress: expanded=50800000 created=101599824 depth=37 bound=1594 best=1615 pruned_bound=50799803 pruned_infeasible=173
[tsp-debug] progress: expanded=50900000 created=101799824 depth=30 bound=1608 best=1615 pruned_bound=50899815 pruned_infeasible=173
[tsp-debug] progress: expanded=51000000 created=101999823 depth=29 bound=1602 best=1615 pruned_bound=50999806 pruned_infeasible=174
[tsp-debug] progress: expanded=51100000 created=102199823 depth=24 bound=1592 best=1615 pruned_bound=51099810 pruned_infeasible=174
[tsp-debug] progress: expanded=51200000 created=102399823 depth=33 bound=1594 best=1615 pruned_bound=51199806 pruned_infeasible=174
[tsp-debug] progress: expanded=51300000 created=102599823 depth=36 bound=1576 best=1615 pruned_bound=51299802 pruned_infeasible=174
[tsp-debug] progress: expanded=51400000 created=102799823 depth=36 bound=1603 best=1615 pruned_bound=51399809 pruned_infeasible=174
[tsp-debug] progress: expanded=51500000 created=102999820 depth=30 bound=1564 best=1615 pruned_bound=51499805 pruned_infeasible=177
[tsp-debug] progress: expanded=51600000 created=103199820 depth=48 bound=1606 best=1615 pruned_bound=51599791 pruned_infeasible=177
[tsp-debug] progress: expanded=51700000 created=103399820 depth=22 bound=1588 best=1615 pruned_bound=51699809 pruned_infeasible=177
[tsp-debug] progress: expanded=51800000 created=103599820 depth=36 bound=1600 best=1615 pruned_bound=51799799 pruned_infeasible=177
[tsp-debug] new incumbent: cost=1610 source=recursive-node depth=33
[tsp-debug] progress: expanded=51900000 created=103799817 depth=37 bound=1602 best=1610 pruned_bound=51899799 pruned_infeasible=178
[tsp-debug] progress: expanded=52000000 created=103999815 depth=37 bound=1570 best=1610 pruned_bound=51999794 pruned_infeasible=180
[tsp-debug] progress: expanded=52100000 created=104199815 depth=45 bound=1599 best=1610 pruned_bound=52099789 pruned_infeasible=180
[tsp-debug] progress: expanded=52200000 created=104399815 depth=36 bound=1604 best=1610 pruned_bound=52199796 pruned_infeasible=180
[tsp-debug] progress: expanded=52300000 created=104599815 depth=31 bound=1601 best=1610 pruned_bound=52299801 pruned_infeasible=180
[tsp-debug] progress: expanded=52400000 created=104799815 depth=31 bound=1576 best=1610 pruned_bound=52399797 pruned_infeasible=180
[tsp-debug] progress: expanded=52500000 created=104999815 depth=48 bound=1601 best=1610 pruned_bound=52499789 pruned_infeasible=180
[tsp-debug] progress: expanded=52600000 created=105199815 depth=36 bound=1599 best=1610 pruned_bound=52599800 pruned_infeasible=180
[tsp-debug] progress: expanded=52700000 created=105399815 depth=46 bound=1600 best=1610 pruned_bound=52699796 pruned_infeasible=180
[tsp-debug] progress: expanded=52800000 created=105599815 depth=34 bound=1605 best=1610 pruned_bound=52799800 pruned_infeasible=180
[tsp-debug] progress: expanded=52900000 created=105799814 depth=30 bound=1605 best=1610 pruned_bound=52899798 pruned_infeasible=181
[tsp-debug] progress: expanded=53000000 created=105999814 depth=41 bound=1606 best=1610 pruned_bound=52999798 pruned_infeasible=181
[tsp-debug] progress: expanded=53100000 created=106199814 depth=42 bound=1609 best=1610 pruned_bound=53099786 pruned_infeasible=181
[tsp-debug] progress: expanded=53200000 created=106399814 depth=47 bound=1594 best=1610 pruned_bound=53199792 pruned_infeasible=181
[tsp-debug] progress: expanded=53300000 created=106599814 depth=41 bound=1603 best=1610 pruned_bound=53299790 pruned_infeasible=181
[tsp-debug] progress: expanded=53400000 created=106799814 depth=30 bound=1599 best=1610 pruned_bound=53399799 pruned_infeasible=181
[tsp-debug] progress: expanded=53500000 created=106999814 depth=28 bound=1607 best=1610 pruned_bound=53499803 pruned_infeasible=181
[tsp-debug] progress: expanded=53600000 created=107199814 depth=39 bound=1589 best=1610 pruned_bound=53599799 pruned_infeasible=181
[tsp-debug] progress: expanded=53700000 created=107399814 depth=43 bound=1605 best=1610 pruned_bound=53699786 pruned_infeasible=181
[tsp-debug] progress: expanded=53800000 created=107599814 depth=27 bound=1609 best=1610 pruned_bound=53799796 pruned_infeasible=181
[tsp-debug] progress: expanded=53900000 created=107799814 depth=35 bound=1590 best=1610 pruned_bound=53899796 pruned_infeasible=181
[tsp-debug] progress: expanded=54000000 created=107999814 depth=36 bound=1603 best=1610 pruned_bound=53999801 pruned_infeasible=181
[tsp-debug] progress: expanded=54100000 created=108199814 depth=29 bound=1561 best=1610 pruned_bound=54099803 pruned_infeasible=181
[tsp-debug] progress: expanded=54200000 created=108399814 depth=43 bound=1588 best=1610 pruned_bound=54199792 pruned_infeasible=181
[tsp-debug] progress: expanded=54300000 created=108599814 depth=27 bound=1566 best=1610 pruned_bound=54299801 pruned_infeasible=181
[tsp-debug] progress: expanded=54400000 created=108799811 depth=38 bound=1603 best=1610 pruned_bound=54399789 pruned_infeasible=184
[tsp-debug] progress: expanded=54500000 created=108999811 depth=31 bound=1572 best=1610 pruned_bound=54499801 pruned_infeasible=184
[tsp-debug] progress: expanded=54600000 created=109199811 depth=40 bound=1605 best=1610 pruned_bound=54599792 pruned_infeasible=184
[tsp-debug] progress: expanded=54700000 created=109399810 depth=33 bound=1603 best=1610 pruned_bound=54699797 pruned_infeasible=185
[tsp-debug] progress: expanded=54800000 created=109599810 depth=38 bound=1588 best=1610 pruned_bound=54799790 pruned_infeasible=185
[tsp-debug] progress: expanded=54900000 created=109799810 depth=40 bound=1592 best=1610 pruned_bound=54899796 pruned_infeasible=185
[tsp-debug] progress: expanded=55000000 created=109999809 depth=38 bound=1608 best=1610 pruned_bound=54999797 pruned_infeasible=186
[tsp-debug] progress: expanded=55100000 created=110199809 depth=24 bound=1535 best=1610 pruned_bound=55099802 pruned_infeasible=186
[tsp-debug] progress: expanded=55200000 created=110399809 depth=28 bound=1558 best=1610 pruned_bound=55199802 pruned_infeasible=186
[tsp-debug] progress: expanded=55300000 created=110599809 depth=49 bound=1607 best=1610 pruned_bound=55299802 pruned_infeasible=186
[tsp-debug] exact solve finished: feasible=yes cost=1610 expanded=55367726 created=110735263 pruned_bound=55367537 pruned_infeasible=186
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=bayg29 dimension=29 method=exact
[tsp-debug] exact solve started: vertices=29
[tsp-debug] initial incumbent: cost=1618
[tsp-debug] root: lower_bound=1375 search=recursive-dfs strategy=simple
[tsp-debug] progress: expanded=100000 created=112458 depth=370 bound=1558 best=1618 pruned_bound=12101 pruned_infeasible=87541
[tsp-debug] progress: expanded=200000 created=228337 depth=379 bound=1611 best=1618 pruned_bound=27979 pruned_infeasible=171662
[tsp-debug] progress: expanded=300000 created=350379 depth=368 bound=1543 best=1618 pruned_bound=50021 pruned_infeasible=249620
[tsp-debug] progress: expanded=400000 created=471614 depth=365 bound=1570 best=1618 pruned_bound=71262 pruned_infeasible=328385
[tsp-debug] new incumbent: cost=1615 source=recursive-node depth=392
[tsp-debug] progress: expanded=500000 created=589656 depth=375 bound=1603 best=1615 pruned_bound=89299 pruned_infeasible=410341
[tsp-debug] progress: expanded=600000 created=716226 depth=361 bound=1600 best=1615 pruned_bound=115877 pruned_infeasible=483771
[tsp-debug] progress: expanded=700000 created=842098 depth=378 bound=1599 best=1615 pruned_bound=141738 pruned_infeasible=557899
[tsp-debug] progress: expanded=800000 created=969275 depth=361 bound=1599 best=1615 pruned_bound=168924 pruned_infeasible=630722
[tsp-debug] progress: expanded=900000 created=1098535 depth=362 bound=1585 best=1615 pruned_bound=198184 pruned_infeasible=701462
[tsp-debug] progress: expanded=1000000 created=1229829 depth=362 bound=1588 best=1615 pruned_bound=229481 pruned_infeasible=770168
[tsp-debug] progress: expanded=1100000 created=1359285 depth=371 bound=1611 best=1615 pruned_bound=258928 pruned_infeasible=840712
[tsp-debug] progress: expanded=1200000 created=1489515 depth=363 bound=1599 best=1615 pruned_bound=289162 pruned_infeasible=910482
[tsp-debug] progress: expanded=1300000 created=1622929 depth=355 bound=1571 best=1615 pruned_bound=322583 pruned_infeasible=977068
[tsp-debug] progress: expanded=1400000 created=1762343 depth=369 bound=1590 best=1615 pruned_bound=361987 pruned_infeasible=1037654
[tsp-debug] progress: expanded=1500000 created=1896134 depth=375 bound=1597 best=1615 pruned_bound=395778 pruned_infeasible=1103863
[tsp-debug] progress: expanded=1600000 created=2033089 depth=359 bound=1608 best=1615 pruned_bound=432744 pruned_infeasible=1166908
[tsp-debug] progress: expanded=1700000 created=2166516 depth=368 bound=1588 best=1615 pruned_bound=466161 pruned_infeasible=1233481
[tsp-debug] progress: expanded=1800000 created=2299523 depth=369 bound=1597 best=1615 pruned_bound=499168 pruned_infeasible=1300474
[tsp-debug] progress: expanded=1900000 created=2437813 depth=367 bound=1614 best=1615 pruned_bound=537461 pruned_infeasible=1362184
[tsp-debug] progress: expanded=2000000 created=2579518 depth=359 bound=1544 best=1615 pruned_bound=579169 pruned_infeasible=1420479
[tsp-debug] progress: expanded=2100000 created=2708544 depth=366 bound=1602 best=1615 pruned_bound=608192 pruned_infeasible=1491453
[tsp-debug] progress: expanded=2200000 created=2841096 depth=378 bound=1602 best=1615 pruned_bound=640734 pruned_infeasible=1558901
[tsp-debug] progress: expanded=2300000 created=2979342 depth=371 bound=1607 best=1615 pruned_bound=678985 pruned_infeasible=1620655
[tsp-debug] progress: expanded=2400000 created=3112490 depth=374 bound=1597 best=1615 pruned_bound=712129 pruned_infeasible=1687507
[tsp-debug] progress: expanded=2500000 created=3241749 depth=364 bound=1604 best=1615 pruned_bound=741395 pruned_infeasible=1758248
[tsp-debug] progress: expanded=2600000 created=3377086 depth=362 bound=1610 best=1615 pruned_bound=776734 pruned_infeasible=1822911
[tsp-debug] progress: expanded=2700000 created=3518179 depth=375 bound=1613 best=1615 pruned_bound=817818 pruned_infeasible=1881818
[tsp-debug] progress: expanded=2800000 created=3649718 depth=372 bound=1611 best=1615 pruned_bound=849360 pruned_infeasible=1950279
[tsp-debug] progress: expanded=2900000 created=3775010 depth=362 bound=1593 best=1615 pruned_bound=874659 pruned_infeasible=2024987
[tsp-debug] progress: expanded=3000000 created=3895456 depth=372 bound=1601 best=1615 pruned_bound=895101 pruned_infeasible=2104541
[tsp-debug] progress: expanded=3100000 created=4014051 depth=376 bound=1580 best=1615 pruned_bound=913690 pruned_infeasible=2185946
[tsp-debug] progress: expanded=3200000 created=4142311 depth=370 bound=1610 best=1615 pruned_bound=941956 pruned_infeasible=2257686
[tsp-debug] progress: expanded=3300000 created=4271283 depth=371 bound=1603 best=1615 pruned_bound=970928 pruned_infeasible=2328714
[tsp-debug] progress: expanded=3400000 created=4405063 depth=359 bound=1595 best=1615 pruned_bound=1004714 pruned_infeasible=2394934
[tsp-debug] progress: expanded=3500000 created=4539332 depth=359 bound=1604 best=1615 pruned_bound=1038984 pruned_infeasible=2460665
[tsp-debug] progress: expanded=3600000 created=4675702 depth=362 bound=1597 best=1615 pruned_bound=1075352 pruned_infeasible=2524295
[tsp-debug] progress: expanded=3700000 created=4810159 depth=377 bound=1586 best=1615 pruned_bound=1109798 pruned_infeasible=2589838
[tsp-debug] progress: expanded=3800000 created=4945756 depth=367 bound=1598 best=1615 pruned_bound=1145403 pruned_infeasible=2654241
[tsp-debug] progress: expanded=3900000 created=5076423 depth=350 bound=1591 best=1615 pruned_bound=1176080 pruned_infeasible=2723574
[tsp-debug] progress: expanded=4000000 created=5208653 depth=368 bound=1596 best=1615 pruned_bound=1208298 pruned_infeasible=2791344
[tsp-debug] progress: expanded=4100000 created=5342247 depth=359 bound=1612 best=1615 pruned_bound=1241899 pruned_infeasible=2857750
[tsp-debug] progress: expanded=4200000 created=5471652 depth=372 bound=1581 best=1615 pruned_bound=1271292 pruned_infeasible=2928345
[tsp-debug] progress: expanded=4300000 created=5595132 depth=372 bound=1598 best=1615 pruned_bound=1294774 pruned_infeasible=3004865
[tsp-debug] progress: expanded=4400000 created=5727883 depth=368 bound=1586 best=1615 pruned_bound=1327525 pruned_infeasible=3072114
[tsp-debug] progress: expanded=4500000 created=5868140 depth=366 bound=1599 best=1615 pruned_bound=1367787 pruned_infeasible=3131857
[tsp-debug] progress: expanded=4600000 created=6010556 depth=365 bound=1596 best=1615 pruned_bound=1410202 pruned_infeasible=3189441
[tsp-debug] progress: expanded=4700000 created=6152658 depth=363 bound=1596 best=1615 pruned_bound=1452305 pruned_infeasible=3247339
[tsp-debug] progress: expanded=4800000 created=6295600 depth=344 bound=1549 best=1615 pruned_bound=1495264 pruned_infeasible=3304397
[tsp-debug] progress: expanded=4900000 created=6426640 depth=364 bound=1590 best=1615 pruned_bound=1526286 pruned_infeasible=3373357
[tsp-debug] progress: expanded=5000000 created=6553770 depth=373 bound=1594 best=1615 pruned_bound=1553416 pruned_infeasible=3446227
[tsp-debug] progress: expanded=5100000 created=6689188 depth=364 bound=1600 best=1615 pruned_bound=1588838 pruned_infeasible=3510809
[tsp-debug] progress: expanded=5200000 created=6830387 depth=370 bound=1590 best=1615 pruned_bound=1630032 pruned_infeasible=3569610
[tsp-debug] progress: expanded=5300000 created=6971948 depth=374 bound=1600 best=1615 pruned_bound=1671589 pruned_infeasible=3628049
[tsp-debug] progress: expanded=5400000 created=7099657 depth=375 bound=1614 best=1615 pruned_bound=1699297 pruned_infeasible=3700340
[tsp-debug] progress: expanded=5500000 created=7236353 depth=358 bound=1595 best=1615 pruned_bound=1736006 pruned_infeasible=3763644
[tsp-debug] progress: expanded=5600000 created=7380036 depth=365 bound=1576 best=1615 pruned_bound=1779683 pruned_infeasible=3819961
[tsp-debug] progress: expanded=5700000 created=7512113 depth=369 bound=1603 best=1615 pruned_bound=1811755 pruned_infeasible=3887884
[tsp-debug] progress: expanded=5800000 created=7652253 depth=361 bound=1588 best=1615 pruned_bound=1851902 pruned_infeasible=3947744
[tsp-debug] progress: expanded=5900000 created=7790547 depth=370 bound=1604 best=1615 pruned_bound=1890189 pruned_infeasible=4009450
[tsp-debug] progress: expanded=6000000 created=7926960 depth=365 bound=1583 best=1615 pruned_bound=1926607 pruned_infeasible=4073037
[tsp-debug] progress: expanded=6100000 created=8070248 depth=367 bound=1605 best=1615 pruned_bound=1969891 pruned_infeasible=4129749
[tsp-debug] progress: expanded=6200000 created=8216248 depth=359 bound=1563 best=1615 pruned_bound=2015895 pruned_infeasible=4183749
[tsp-debug] progress: expanded=6300000 created=8355597 depth=357 bound=1591 best=1615 pruned_bound=2055249 pruned_infeasible=4244400
[tsp-debug] progress: expanded=6400000 created=8503378 depth=361 bound=1610 best=1615 pruned_bound=2103028 pruned_infeasible=4296619
[tsp-debug] progress: expanded=6500000 created=8647230 depth=378 bound=1600 best=1615 pruned_bound=2146866 pruned_infeasible=4352767
[tsp-debug] progress: expanded=6600000 created=8778606 depth=359 bound=1591 best=1615 pruned_bound=2178258 pruned_infeasible=4421391
[tsp-debug] progress: expanded=6700000 created=8912388 depth=372 bound=1580 best=1615 pruned_bound=2212032 pruned_infeasible=4487609
[tsp-debug] progress: expanded=6800000 created=9057123 depth=364 bound=1567 best=1615 pruned_bound=2256773 pruned_infeasible=4542874
[tsp-debug] progress: expanded=6900000 created=9199975 depth=370 bound=1601 best=1615 pruned_bound=2299622 pruned_infeasible=4600022
[tsp-debug] progress: expanded=7000000 created=9338391 depth=373 bound=1584 best=1615 pruned_bound=2338033 pruned_infeasible=4661606
[tsp-debug] progress: expanded=7100000 created=9481963 depth=363 bound=1610 best=1615 pruned_bound=2381613 pruned_infeasible=4718034
[tsp-debug] progress: expanded=7200000 created=9628924 depth=367 bound=1581 best=1615 pruned_bound=2428567 pruned_infeasible=4771073
[tsp-debug] progress: expanded=7300000 created=9764601 depth=361 bound=1597 best=1615 pruned_bound=2464253 pruned_infeasible=4835396
[tsp-debug] progress: expanded=7400000 created=9909010 depth=370 bound=1610 best=1615 pruned_bound=2508656 pruned_infeasible=4890987
[tsp-debug] progress: expanded=7500000 created=10049150 depth=357 bound=1599 best=1615 pruned_bound=2548803 pruned_infeasible=4950847
[tsp-debug] progress: expanded=7600000 created=10195674 depth=367 bound=1614 best=1615 pruned_bound=2595324 pruned_infeasible=5004323
[tsp-debug] progress: expanded=7700000 created=10344546 depth=361 bound=1614 best=1615 pruned_bound=2644197 pruned_infeasible=5055451
[tsp-debug] progress: expanded=7800000 created=10473659 depth=368 bound=1609 best=1615 pruned_bound=2673307 pruned_infeasible=5126338
[tsp-debug] progress: expanded=7900000 created=10620378 depth=364 bound=1607 best=1615 pruned_bound=2720029 pruned_infeasible=5179619
[tsp-debug] progress: expanded=8000000 created=10763222 depth=361 bound=1586 best=1615 pruned_bound=2762873 pruned_infeasible=5236775
[tsp-debug] progress: expanded=8100000 created=10912600 depth=357 bound=1594 best=1615 pruned_bound=2812253 pruned_infeasible=5287397
[tsp-debug] progress: expanded=8200000 created=11064140 depth=348 bound=1584 best=1615 pruned_bound=2863800 pruned_infeasible=5335857
[tsp-debug] progress: expanded=8300000 created=11200703 depth=369 bound=1566 best=1615 pruned_bound=2900346 pruned_infeasible=5399294
[tsp-debug] progress: expanded=8400000 created=11323615 depth=364 bound=1599 best=1615 pruned_bound=2923262 pruned_infeasible=5476382
[tsp-debug] new incumbent: cost=1610 source=recursive-node depth=384
[tsp-debug] progress: expanded=8500000 created=11453527 depth=358 bound=1587 best=1610 pruned_bound=2953178 pruned_infeasible=5546468
[tsp-debug] progress: expanded=8600000 created=11591682 depth=374 bound=1598 best=1610 pruned_bound=2991323 pruned_infeasible=5608313
[tsp-debug] progress: expanded=8700000 created=11725640 depth=365 bound=1597 best=1610 pruned_bound=3025288 pruned_infeasible=5674355
[tsp-debug] progress: expanded=8800000 created=11865191 depth=365 bound=1606 best=1610 pruned_bound=3064838 pruned_infeasible=5734804
[tsp-debug] progress: expanded=8900000 created=12005173 depth=363 bound=1582 best=1610 pruned_bound=3104824 pruned_infeasible=5794822
[tsp-debug] progress: expanded=9000000 created=12148925 depth=360 bound=1533 best=1610 pruned_bound=3148573 pruned_infeasible=5851070
[tsp-debug] progress: expanded=9100000 created=12287759 depth=344 bound=1570 best=1610 pruned_bound=3187422 pruned_infeasible=5912236
[tsp-debug] progress: expanded=9200000 created=12428774 depth=363 bound=1594 best=1610 pruned_bound=3228420 pruned_infeasible=5971221
[tsp-debug] progress: expanded=9300000 created=12564337 depth=372 bound=1581 best=1610 pruned_bound=3263978 pruned_infeasible=6035658
[tsp-debug] progress: expanded=9400000 created=12692881 depth=372 bound=1606 best=1610 pruned_bound=3292521 pruned_infeasible=6107114
[tsp-debug] progress: expanded=9500000 created=12831969 depth=346 bound=1597 best=1610 pruned_bound=3331632 pruned_infeasible=6168026
[tsp-debug] progress: expanded=9600000 created=12972691 depth=360 bound=1604 best=1610 pruned_bound=3372339 pruned_infeasible=6227304
[tsp-debug] progress: expanded=9700000 created=13113255 depth=372 bound=1605 best=1610 pruned_bound=3412895 pruned_infeasible=6286740
[tsp-debug] progress: expanded=9800000 created=13263509 depth=375 bound=1609 best=1610 pruned_bound=3463151 pruned_infeasible=6336486
[tsp-debug] progress: expanded=9900000 created=13408387 depth=366 bound=1581 best=1610 pruned_bound=3508033 pruned_infeasible=6391608
[tsp-debug] progress: expanded=10000000 created=13552870 depth=364 bound=1597 best=1610 pruned_bound=3552516 pruned_infeasible=6447125
[tsp-debug] progress: expanded=10100000 created=13706381 depth=365 bound=1601 best=1610 pruned_bound=3606027 pruned_infeasible=6493614
[tsp-debug] progress: expanded=10200000 created=13850485 depth=362 bound=1591 best=1610 pruned_bound=3650135 pruned_infeasible=6549510
[tsp-debug] progress: expanded=10300000 created=14005527 depth=378 bound=1602 best=1610 pruned_bound=3705167 pruned_infeasible=6594468
[tsp-debug] progress: expanded=10400000 created=14146878 depth=365 bound=1586 best=1610 pruned_bound=3746524 pruned_infeasible=6653117
[tsp-debug] progress: expanded=10500000 created=14288901 depth=353 bound=1601 best=1610 pruned_bound=3788557 pruned_infeasible=6711094
[tsp-debug] progress: expanded=10600000 created=14443879 depth=366 bound=1602 best=1610 pruned_bound=3843526 pruned_infeasible=6756116
[tsp-debug] progress: expanded=10700000 created=14597853 depth=348 bound=1608 best=1610 pruned_bound=3897516 pruned_infeasible=6802142
[tsp-debug] progress: expanded=10800000 created=14729347 depth=356 bound=1603 best=1610 pruned_bound=3929004 pruned_infeasible=6870648
[tsp-debug] progress: expanded=10900000 created=14876620 depth=367 bound=1577 best=1610 pruned_bound=3976268 pruned_infeasible=6923375
[tsp-debug] progress: expanded=11000000 created=15024915 depth=370 bound=1593 best=1610 pruned_bound=4024560 pruned_infeasible=6975080
[tsp-debug] progress: expanded=11100000 created=15175062 depth=368 bound=1602 best=1610 pruned_bound=4074708 pruned_infeasible=7024933
[tsp-debug] progress: expanded=11200000 created=15324393 depth=345 bound=1591 best=1610 pruned_bound=4124055 pruned_infeasible=7075602
[tsp-debug] progress: expanded=11300000 created=15476175 depth=359 bound=1607 best=1610 pruned_bound=4175826 pruned_infeasible=7123820
[tsp-debug] progress: expanded=11400000 created=15626427 depth=374 bound=1605 best=1610 pruned_bound=4226072 pruned_infeasible=7173568
[tsp-debug] progress: expanded=11500000 created=15780847 depth=358 bound=1605 best=1610 pruned_bound=4280499 pruned_infeasible=7219148
[tsp-debug] progress: expanded=11600000 created=15941978 depth=353 bound=1604 best=1610 pruned_bound=4341637 pruned_infeasible=7258017
[tsp-debug] progress: expanded=11700000 created=16085292 depth=360 bound=1598 best=1610 pruned_bound=4384943 pruned_infeasible=7314703
[tsp-debug] progress: expanded=11800000 created=16234155 depth=350 bound=1580 best=1610 pruned_bound=4433813 pruned_infeasible=7365840
[tsp-debug] progress: expanded=11900000 created=16388960 depth=365 bound=1601 best=1610 pruned_bound=4488609 pruned_infeasible=7411035
[tsp-debug] progress: expanded=12000000 created=16543602 depth=360 bound=1602 best=1610 pruned_bound=4543253 pruned_infeasible=7456393
[tsp-debug] progress: expanded=12100000 created=16693892 depth=359 bound=1577 best=1610 pruned_bound=4593544 pruned_infeasible=7506103
[tsp-debug] progress: expanded=12200000 created=16847864 depth=371 bound=1609 best=1610 pruned_bound=4647504 pruned_infeasible=7552131
[tsp-debug] progress: expanded=12300000 created=17011163 depth=331 bound=1603 best=1610 pruned_bound=4710837 pruned_infeasible=7588832
[tsp-debug] progress: expanded=12400000 created=17143250 depth=370 bound=1596 best=1610 pruned_bound=4742896 pruned_infeasible=7656745
[tsp-debug] progress: expanded=12500000 created=17284965 depth=366 bound=1596 best=1610 pruned_bound=4784613 pruned_infeasible=7715030
[tsp-debug] progress: expanded=12600000 created=17430251 depth=369 bound=1590 best=1610 pruned_bound=4829894 pruned_infeasible=7769744
[tsp-debug] progress: expanded=12700000 created=17567449 depth=378 bound=1594 best=1610 pruned_bound=4867090 pruned_infeasible=7832546
[tsp-debug] progress: expanded=12800000 created=17717448 depth=355 bound=1608 best=1610 pruned_bound=4917102 pruned_infeasible=7882547
[tsp-debug] progress: expanded=12900000 created=17868954 depth=374 bound=1598 best=1610 pruned_bound=4968595 pruned_infeasible=7931041
[tsp-debug] progress: expanded=13000000 created=18015301 depth=373 bound=1608 best=1610 pruned_bound=5014944 pruned_infeasible=7984694
[tsp-debug] progress: expanded=13100000 created=18166673 depth=344 bound=1558 best=1610 pruned_bound=5066335 pruned_infeasible=8033322
[tsp-debug] progress: expanded=13200000 created=18325040 depth=346 bound=1572 best=1610 pruned_bound=5124700 pruned_infeasible=8074955
[tsp-debug] progress: expanded=13300000 created=18477075 depth=368 bound=1600 best=1610 pruned_bound=5176722 pruned_infeasible=8122920
[tsp-debug] progress: expanded=13400000 created=18620916 depth=364 bound=1571 best=1610 pruned_bound=5220561 pruned_infeasible=8179079
[tsp-debug] progress: expanded=13500000 created=18772780 depth=371 bound=1597 best=1610 pruned_bound=5272424 pruned_infeasible=8227215
[tsp-debug] progress: expanded=13600000 created=18935233 depth=364 bound=1592 best=1610 pruned_bound=5334879 pruned_infeasible=8264762
[tsp-debug] progress: expanded=13700000 created=19078424 depth=364 bound=1594 best=1610 pruned_bound=5378072 pruned_infeasible=8321571
[tsp-debug] progress: expanded=13800000 created=19225269 depth=343 bound=1609 best=1610 pruned_bound=5424933 pruned_infeasible=8374726
[tsp-debug] progress: expanded=13900000 created=19372362 depth=359 bound=1603 best=1610 pruned_bound=5472013 pruned_infeasible=8427633
[tsp-debug] progress: expanded=14000000 created=19528588 depth=343 bound=1586 best=1610 pruned_bound=5528253 pruned_infeasible=8471407
[tsp-debug] progress: expanded=14100000 created=19682676 depth=353 bound=1596 best=1610 pruned_bound=5582333 pruned_infeasible=8517319
[tsp-debug] progress: expanded=14200000 created=19847233 depth=361 bound=1606 best=1610 pruned_bound=5646882 pruned_infeasible=8552762
[tsp-debug] progress: expanded=14300000 created=20006900 depth=365 bound=1584 best=1610 pruned_bound=5706545 pruned_infeasible=8593095
[tsp-debug] progress: expanded=14400000 created=20151724 depth=363 bound=1560 best=1610 pruned_bound=5751370 pruned_infeasible=8648271
[tsp-debug] progress: expanded=14500000 created=20300602 depth=363 bound=1605 best=1610 pruned_bound=5800250 pruned_infeasible=8699393
[tsp-debug] progress: expanded=14600000 created=20454882 depth=360 bound=1609 best=1610 pruned_bound=5854531 pruned_infeasible=8745113
[tsp-debug] progress: expanded=14700000 created=20614690 depth=344 bound=1590 best=1610 pruned_bound=5914352 pruned_infeasible=8785305
[tsp-debug] progress: expanded=14800000 created=20776843 depth=374 bound=1593 best=1610 pruned_bound=5976483 pruned_infeasible=8823152
[tsp-debug] progress: expanded=14900000 created=20901662 depth=375 bound=1597 best=1610 pruned_bound=6001304 pruned_infeasible=8898333
[tsp-debug] progress: expanded=15000000 created=21037750 depth=371 bound=1608 best=1610 pruned_bound=6037395 pruned_infeasible=8962245
[tsp-debug] progress: expanded=15100000 created=21173015 depth=371 bound=1599 best=1610 pruned_bound=6072660 pruned_infeasible=9026980
[tsp-debug] progress: expanded=15200000 created=21315141 depth=356 bound=1604 best=1610 pruned_bound=6114796 pruned_infeasible=9084854
[tsp-debug] progress: expanded=15300000 created=21456093 depth=361 bound=1599 best=1610 pruned_bound=6155744 pruned_infeasible=9143902
[tsp-debug] progress: expanded=15400000 created=21596799 depth=363 bound=1597 best=1610 pruned_bound=6196450 pruned_infeasible=9203196
[tsp-debug] progress: expanded=15500000 created=21734146 depth=371 bound=1570 best=1610 pruned_bound=6233792 pruned_infeasible=9265849
[tsp-debug] progress: expanded=15600000 created=21865685 depth=354 bound=1574 best=1610 pruned_bound=6265339 pruned_infeasible=9334310
[tsp-debug] progress: expanded=15700000 created=22008574 depth=358 bound=1606 best=1610 pruned_bound=6308226 pruned_infeasible=9391421
[tsp-debug] progress: expanded=15800000 created=22148685 depth=353 bound=1591 best=1610 pruned_bound=6348340 pruned_infeasible=9451310
[tsp-debug] progress: expanded=15900000 created=22298270 depth=356 bound=1605 best=1610 pruned_bound=6397926 pruned_infeasible=9501725
[tsp-debug] progress: expanded=16000000 created=22444930 depth=364 bound=1598 best=1610 pruned_bound=6444577 pruned_infeasible=9555065
[tsp-debug] progress: expanded=16100000 created=22593425 depth=359 bound=1591 best=1610 pruned_bound=6493079 pruned_infeasible=9606570
[tsp-debug] progress: expanded=16200000 created=22742063 depth=362 bound=1597 best=1610 pruned_bound=6541709 pruned_infeasible=9657932
[tsp-debug] progress: expanded=16300000 created=22895761 depth=361 bound=1574 best=1610 pruned_bound=6595409 pruned_infeasible=9704234
[tsp-debug] progress: expanded=16400000 created=23049882 depth=366 bound=1593 best=1610 pruned_bound=6649531 pruned_infeasible=9750113
[tsp-debug] progress: expanded=16500000 created=23207691 depth=331 bound=1599 best=1610 pruned_bound=6707366 pruned_infeasible=9792304
[tsp-debug] progress: expanded=16600000 created=23365645 depth=360 bound=1600 best=1610 pruned_bound=6765299 pruned_infeasible=9834350
[tsp-debug] progress: expanded=16700000 created=23515869 depth=366 bound=1596 best=1610 pruned_bound=6815514 pruned_infeasible=9884126
[tsp-debug] progress: expanded=16800000 created=23673720 depth=361 bound=1581 best=1610 pruned_bound=6873368 pruned_infeasible=9926275
[tsp-debug] progress: expanded=16900000 created=23818868 depth=367 bound=1597 best=1610 pruned_bound=6918516 pruned_infeasible=9981127
[tsp-debug] progress: expanded=17000000 created=23962155 depth=368 bound=1577 best=1610 pruned_bound=6961801 pruned_infeasible=10037840
[tsp-debug] progress: expanded=17100000 created=24105934 depth=368 bound=1596 best=1610 pruned_bound=7005581 pruned_infeasible=10094061
[tsp-debug] progress: expanded=17200000 created=24258789 depth=345 bound=1597 best=1610 pruned_bound=7058452 pruned_infeasible=10141206
[tsp-debug] progress: expanded=17300000 created=24420325 depth=369 bound=1606 best=1610 pruned_bound=7119972 pruned_infeasible=10179670
[tsp-debug] progress: expanded=17400000 created=24576631 depth=356 bound=1609 best=1610 pruned_bound=7176285 pruned_infeasible=10223364
[tsp-debug] progress: expanded=17500000 created=24724056 depth=353 bound=1609 best=1610 pruned_bound=7223712 pruned_infeasible=10275939
[tsp-debug] progress: expanded=17600000 created=24872612 depth=363 bound=1580 best=1610 pruned_bound=7272260 pruned_infeasible=10327383
[tsp-debug] progress: expanded=17700000 created=25029512 depth=344 bound=1607 best=1610 pruned_bound=7329176 pruned_infeasible=10370483
[tsp-debug] progress: expanded=17800000 created=25192120 depth=346 bound=1590 best=1610 pruned_bound=7391781 pruned_infeasible=10407875
[tsp-debug] progress: expanded=17900000 created=25342612 depth=370 bound=1605 best=1610 pruned_bound=7442253 pruned_infeasible=10457383
[tsp-debug] progress: expanded=18000000 created=25483838 depth=347 bound=1568 best=1610 pruned_bound=7483498 pruned_infeasible=10516157
[tsp-debug] progress: expanded=18100000 created=25627979 depth=366 bound=1586 best=1610 pruned_bound=7527623 pruned_infeasible=10572016
[tsp-debug] progress: expanded=18200000 created=25780604 depth=370 bound=1600 best=1610 pruned_bound=7580246 pruned_infeasible=10619391
[tsp-debug] progress: expanded=18300000 created=25931250 depth=360 bound=1599 best=1610 pruned_bound=7630903 pruned_infeasible=10668745
[tsp-debug] progress: expanded=18400000 created=26092051 depth=359 bound=1590 best=1610 pruned_bound=7691702 pruned_infeasible=10707944
[tsp-debug] progress: expanded=18500000 created=26251575 depth=355 bound=1606 best=1610 pruned_bound=7751229 pruned_infeasible=10748420
[tsp-debug] progress: expanded=18600000 created=26410325 depth=346 bound=1570 best=1610 pruned_bound=7809986 pruned_infeasible=10789670
[tsp-debug] progress: expanded=18700000 created=26573160 depth=357 bound=1594 best=1610 pruned_bound=7872812 pruned_infeasible=10826835
[tsp-debug] progress: expanded=18800000 created=26722266 depth=349 bound=1586 best=1610 pruned_bound=7921924 pruned_infeasible=10877729
[tsp-debug] progress: expanded=18900000 created=26874028 depth=347 bound=1588 best=1610 pruned_bound=7973689 pruned_infeasible=10925967
[tsp-debug] progress: expanded=19000000 created=27033327 depth=355 bound=1573 best=1610 pruned_bound=8032979 pruned_infeasible=10966668
[tsp-debug] progress: expanded=19100000 created=27199173 depth=359 bound=1593 best=1610 pruned_bound=8098825 pruned_infeasible=11000822
[tsp-debug] progress: expanded=19200000 created=27364583 depth=368 bound=1605 best=1610 pruned_bound=8164226 pruned_infeasible=11035412
[tsp-debug] progress: expanded=19300000 created=27512868 depth=357 bound=1598 best=1610 pruned_bound=8212519 pruned_infeasible=11087127
[tsp-debug] progress: expanded=19400000 created=27661094 depth=359 bound=1609 best=1610 pruned_bound=8260745 pruned_infeasible=11138901
[tsp-debug] progress: expanded=19500000 created=27818349 depth=370 bound=1597 best=1610 pruned_bound=8317993 pruned_infeasible=11181646
[tsp-debug] progress: expanded=19600000 created=27980840 depth=349 bound=1607 best=1610 pruned_bound=8380499 pruned_infeasible=11219155
[tsp-debug] progress: expanded=19700000 created=28141562 depth=367 bound=1588 best=1610 pruned_bound=8441207 pruned_infeasible=11258433
[tsp-debug] progress: expanded=19800000 created=28280659 depth=366 bound=1599 best=1610 pruned_bound=8480305 pruned_infeasible=11319336
[tsp-debug] progress: expanded=19900000 created=28425146 depth=349 bound=1608 best=1610 pruned_bound=8524806 pruned_infeasible=11374849
[tsp-debug] progress: expanded=20000000 created=28576902 depth=339 bound=1592 best=1610 pruned_bound=8576569 pruned_infeasible=11423093
[tsp-debug] progress: expanded=20100000 created=28735850 depth=357 bound=1596 best=1610 pruned_bound=8635501 pruned_infeasible=11464145
[tsp-debug] progress: expanded=20200000 created=28892462 depth=338 bound=1594 best=1610 pruned_bound=8692130 pruned_infeasible=11507533
[tsp-debug] progress: expanded=20300000 created=29060364 depth=331 bound=1583 best=1610 pruned_bound=8760038 pruned_infeasible=11539631
[tsp-debug] progress: expanded=20400000 created=29219253 depth=345 bound=1605 best=1610 pruned_bound=8818915 pruned_infeasible=11580742
[tsp-debug] progress: expanded=20500000 created=29385912 depth=350 bound=1584 best=1610 pruned_bound=8885568 pruned_infeasible=11614083
[tsp-debug] progress: expanded=20600000 created=29548649 depth=339 bound=1608 best=1610 pruned_bound=8948317 pruned_infeasible=11651346
[tsp-debug] progress: expanded=20700000 created=29703965 depth=365 bound=1597 best=1610 pruned_bound=9003610 pruned_infeasible=11696030
[tsp-debug] progress: expanded=20800000 created=29851689 depth=363 bound=1603 best=1610 pruned_bound=9051340 pruned_infeasible=11748306
[tsp-debug] progress: expanded=20900000 created=30011136 depth=341 bound=1585 best=1610 pruned_bound=9110800 pruned_infeasible=11788859
[tsp-debug] progress: expanded=21000000 created=30174923 depth=342 bound=1558 best=1610 pruned_bound=9174586 pruned_infeasible=11825072
[tsp-debug] progress: expanded=21100000 created=30337914 depth=368 bound=1578 best=1610 pruned_bound=9237557 pruned_infeasible=11862081
[tsp-debug] progress: expanded=21200000 created=30484220 depth=342 bound=1608 best=1610 pruned_bound=9283884 pruned_infeasible=11915775
[tsp-debug] progress: expanded=21300000 created=30638920 depth=348 bound=1605 best=1610 pruned_bound=9338580 pruned_infeasible=11961075
[tsp-debug] progress: expanded=21400000 created=30806153 depth=357 bound=1600 best=1610 pruned_bound=9405805 pruned_infeasible=11993842
[tsp-debug] progress: expanded=21500000 created=30977122 depth=337 bound=1609 best=1610 pruned_bound=9476794 pruned_infeasible=12022873
[tsp-debug] progress: expanded=21600000 created=31148650 depth=366 bound=1583 best=1610 pruned_bound=9548296 pruned_infeasible=12051345
[tsp-debug] progress: expanded=21700000 created=31289549 depth=359 bound=1562 best=1610 pruned_bound=9589197 pruned_infeasible=12110446
[tsp-debug] progress: expanded=21800000 created=31440820 depth=351 bound=1598 best=1610 pruned_bound=9640477 pruned_infeasible=12159175
[tsp-debug] progress: expanded=21900000 created=31598949 depth=356 bound=1607 best=1610 pruned_bound=9698602 pruned_infeasible=12201046
[tsp-debug] progress: expanded=22000000 created=31765082 depth=362 bound=1604 best=1610 pruned_bound=9764733 pruned_infeasible=12234913
[tsp-debug] progress: expanded=22100000 created=31936441 depth=356 bound=1599 best=1610 pruned_bound=9836093 pruned_infeasible=12263554
[tsp-debug] progress: expanded=22200000 created=32079450 depth=363 bound=1597 best=1610 pruned_bound=9879098 pruned_infeasible=12320545
[tsp-debug] progress: expanded=22300000 created=32222512 depth=366 bound=1602 best=1610 pruned_bound=9922159 pruned_infeasible=12377483
[tsp-debug] progress: expanded=22400000 created=32374514 depth=345 bound=1608 best=1610 pruned_bound=9974178 pruned_infeasible=12425481
[tsp-debug] progress: expanded=22500000 created=32527170 depth=339 bound=1592 best=1610 pruned_bound=10026837 pruned_infeasible=12472825
[tsp-debug] progress: expanded=22600000 created=32689836 depth=351 bound=1589 best=1610 pruned_bound=10089491 pruned_infeasible=12510159
[tsp-debug] progress: expanded=22700000 created=32856431 depth=373 bound=1608 best=1610 pruned_bound=10156075 pruned_infeasible=12543564
[tsp-debug] progress: expanded=22800000 created=33024587 depth=362 bound=1608 best=1610 pruned_bound=10224235 pruned_infeasible=12575408
[tsp-debug] progress: expanded=22900000 created=33178419 depth=360 bound=1605 best=1610 pruned_bound=10278068 pruned_infeasible=12621576
[tsp-debug] progress: expanded=23000000 created=33341278 depth=339 bound=1607 best=1610 pruned_bound=10340946 pruned_infeasible=12658717
[tsp-debug] progress: expanded=23100000 created=33511171 depth=357 bound=1594 best=1610 pruned_bound=10410824 pruned_infeasible=12688824
[tsp-debug] progress: expanded=23200000 created=33686990 depth=339 bound=1589 best=1610 pruned_bound=10486656 pruned_infeasible=12713005
[tsp-debug] progress: expanded=23300000 created=33817090 depth=365 bound=1608 best=1610 pruned_bound=10516736 pruned_infeasible=12782905
[tsp-debug] progress: expanded=23400000 created=33964944 depth=350 bound=1597 best=1610 pruned_bound=10564602 pruned_infeasible=12835051
[tsp-debug] progress: expanded=23500000 created=34114475 depth=356 bound=1603 best=1610 pruned_bound=10614127 pruned_infeasible=12885520
[tsp-debug] progress: expanded=23600000 created=34268405 depth=373 bound=1598 best=1610 pruned_bound=10668042 pruned_infeasible=12931590
[tsp-debug] progress: expanded=23700000 created=34429376 depth=366 bound=1592 best=1610 pruned_bound=10729024 pruned_infeasible=12970619
[tsp-debug] progress: expanded=23800000 created=34594381 depth=327 bound=1556 best=1610 pruned_bound=10794059 pruned_infeasible=13005614
[tsp-debug] progress: expanded=23900000 created=34758647 depth=353 bound=1606 best=1610 pruned_bound=10858303 pruned_infeasible=13041348
[tsp-debug] progress: expanded=24000000 created=34925265 depth=345 bound=1597 best=1610 pruned_bound=10924927 pruned_infeasible=13074730
[tsp-debug] progress: expanded=24100000 created=35098401 depth=354 bound=1597 best=1610 pruned_bound=10998054 pruned_infeasible=13101594
[tsp-debug] progress: expanded=24200000 created=35273910 depth=339 bound=1607 best=1610 pruned_bound=11073577 pruned_infeasible=13126085
[tsp-debug] progress: expanded=24300000 created=35431630 depth=361 bound=1604 best=1610 pruned_bound=11131278 pruned_infeasible=13168365
[tsp-debug] progress: expanded=24400000 created=35598081 depth=340 bound=1603 best=1610 pruned_bound=11197748 pruned_infeasible=13201914
[tsp-debug] progress: expanded=24500000 created=35772877 depth=336 bound=1586 best=1610 pruned_bound=11272547 pruned_infeasible=13227118
[tsp-debug] progress: expanded=24600000 created=35940620 depth=363 bound=1603 best=1610 pruned_bound=11340268 pruned_infeasible=13259375
[tsp-debug] progress: expanded=24700000 created=36094768 depth=367 bound=1584 best=1610 pruned_bound=11394415 pruned_infeasible=13305227
[tsp-debug] progress: expanded=24800000 created=36262183 depth=355 bound=1605 best=1610 pruned_bound=11461837 pruned_infeasible=13337812
[tsp-debug] progress: expanded=24900000 created=36439346 depth=330 bound=1609 best=1610 pruned_bound=11539021 pruned_infeasible=13360649
[tsp-debug] progress: expanded=25000000 created=36595247 depth=360 bound=1585 best=1610 pruned_bound=11594901 pruned_infeasible=13404748
[tsp-debug] progress: expanded=25100000 created=36748203 depth=337 bound=1594 best=1610 pruned_bound=11647873 pruned_infeasible=13451792
[tsp-debug] progress: expanded=25200000 created=36915003 depth=362 bound=1603 best=1610 pruned_bound=11714654 pruned_infeasible=13484992
[tsp-debug] progress: expanded=25300000 created=37089464 depth=339 bound=1471 best=1610 pruned_bound=11789127 pruned_infeasible=13510531
[tsp-debug] progress: expanded=25400000 created=37242358 depth=343 bound=1567 best=1610 pruned_bound=11842021 pruned_infeasible=13557637
[tsp-debug] progress: expanded=25500000 created=37410266 depth=358 bound=1604 best=1610 pruned_bound=11909918 pruned_infeasible=13589729
[tsp-debug] progress: expanded=25600000 created=37587968 depth=338 bound=1596 best=1610 pruned_bound=11987635 pruned_infeasible=13612027
[tsp-debug] progress: expanded=25700000 created=37743961 depth=361 bound=1567 best=1610 pruned_bound=12043609 pruned_infeasible=13656034
[tsp-debug] progress: expanded=25800000 created=37897065 depth=343 bound=1586 best=1610 pruned_bound=12096728 pruned_infeasible=13702930
[tsp-debug] progress: expanded=25900000 created=38063346 depth=354 bound=1607 best=1610 pruned_bound=12163000 pruned_infeasible=13736649
[tsp-debug] progress: expanded=26000000 created=38233140 depth=337 bound=1593 best=1610 pruned_bound=12232808 pruned_infeasible=13766855
[tsp-debug] progress: expanded=26100000 created=38412686 depth=343 bound=1606 best=1610 pruned_bound=12312349 pruned_infeasible=13787309
[tsp-debug] progress: expanded=26200000 created=38565225 depth=363 bound=1608 best=1610 pruned_bound=12364872 pruned_infeasible=13834770
[tsp-debug] progress: expanded=26300000 created=38730043 depth=363 bound=1602 best=1610 pruned_bound=12429690 pruned_infeasible=13869952
[tsp-debug] progress: expanded=26400000 created=38906212 depth=347 bound=1576 best=1610 pruned_bound=12505872 pruned_infeasible=13893783
[tsp-debug] progress: expanded=26500000 created=39051334 depth=364 bound=1609 best=1610 pruned_bound=12550982 pruned_infeasible=13948661
[tsp-debug] progress: expanded=26600000 created=39192260 depth=359 bound=1602 best=1610 pruned_bound=12591913 pruned_infeasible=14007735
[tsp-debug] progress: expanded=26700000 created=39338731 depth=366 bound=1609 best=1610 pruned_bound=12638379 pruned_infeasible=14061264
[tsp-debug] progress: expanded=26800000 created=39484303 depth=363 bound=1608 best=1610 pruned_bound=12683949 pruned_infeasible=14115692
[tsp-debug] progress: expanded=26900000 created=39622352 depth=369 bound=1591 best=1610 pruned_bound=12721993 pruned_infeasible=14177643
[tsp-debug] progress: expanded=27000000 created=39771696 depth=356 bound=1607 best=1610 pruned_bound=12771348 pruned_infeasible=14228299
[tsp-debug] progress: expanded=27100000 created=39927599 depth=360 bound=1588 best=1610 pruned_bound=12827248 pruned_infeasible=14272396
[tsp-debug] progress: expanded=27200000 created=40090563 depth=337 bound=1595 best=1610 pruned_bound=12890232 pruned_infeasible=14309432
[tsp-debug] progress: expanded=27300000 created=40251930 depth=347 bound=1579 best=1610 pruned_bound=12951590 pruned_infeasible=14348065
[tsp-debug] progress: expanded=27400000 created=40421024 depth=342 bound=1601 best=1610 pruned_bound=13020691 pruned_infeasible=14378971
[tsp-debug] progress: expanded=27500000 created=40588899 depth=342 bound=1593 best=1610 pruned_bound=13088564 pruned_infeasible=14411096
[tsp-debug] progress: expanded=27600000 created=40760255 depth=339 bound=1583 best=1610 pruned_bound=13159921 pruned_infeasible=14439740
[tsp-debug] progress: expanded=27700000 created=40933700 depth=333 bound=1606 best=1610 pruned_bound=13233373 pruned_infeasible=14466295
[tsp-debug] progress: expanded=27800000 created=41098975 depth=341 bound=1598 best=1610 pruned_bound=13298641 pruned_infeasible=14501020
[tsp-debug] progress: expanded=27900000 created=41260923 depth=369 bound=1602 best=1610 pruned_bound=13360564 pruned_infeasible=14539072
[tsp-debug] progress: expanded=28000000 created=41436007 depth=341 bound=1607 best=1610 pruned_bound=13435672 pruned_infeasible=14563988
[tsp-debug] progress: expanded=28100000 created=41604441 depth=357 bound=1594 best=1610 pruned_bound=13504096 pruned_infeasible=14595554
[tsp-debug] progress: expanded=28200000 created=41758193 depth=337 bound=1605 best=1610 pruned_bound=13557861 pruned_infeasible=14641802
[tsp-debug] progress: expanded=28300000 created=41918872 depth=336 bound=1567 best=1610 pruned_bound=13618541 pruned_infeasible=14681123
[tsp-debug] progress: expanded=28400000 created=42089335 depth=348 bound=1598 best=1610 pruned_bound=13688997 pruned_infeasible=14710660
[tsp-debug] progress: expanded=28500000 created=42255865 depth=342 bound=1607 best=1610 pruned_bound=13755532 pruned_infeasible=14744130
[tsp-debug] progress: expanded=28600000 created=42433798 depth=335 bound=1585 best=1610 pruned_bound=13833469 pruned_infeasible=14766197
[tsp-debug] progress: expanded=28700000 created=42609491 depth=327 bound=1609 best=1610 pruned_bound=13909169 pruned_infeasible=14790504
[tsp-debug] progress: expanded=28800000 created=42792712 depth=335 bound=1609 best=1610 pruned_bound=13992384 pruned_infeasible=14807283
[tsp-debug] progress: expanded=28900000 created=42948496 depth=362 bound=1599 best=1610 pruned_bound=14048144 pruned_infeasible=14851499
[tsp-debug] progress: expanded=29000000 created=43092100 depth=376 bound=1588 best=1610 pruned_bound=14091736 pruned_infeasible=14907895
[tsp-debug] progress: expanded=29100000 created=43243526 depth=359 bound=1609 best=1610 pruned_bound=14143177 pruned_infeasible=14956469
[tsp-debug] progress: expanded=29200000 created=43403501 depth=338 bound=1591 best=1610 pruned_bound=14203170 pruned_infeasible=14996494
[tsp-debug] progress: expanded=29300000 created=43574759 depth=324 bound=1606 best=1610 pruned_bound=14274439 pruned_infeasible=15025236
[tsp-debug] progress: expanded=29400000 created=43747480 depth=359 bound=1591 best=1610 pruned_bound=14347133 pruned_infeasible=15052515
[tsp-debug] progress: expanded=29500000 created=43891016 depth=360 bound=1606 best=1610 pruned_bound=14390669 pruned_infeasible=15108979
[tsp-debug] progress: expanded=29600000 created=44047896 depth=330 bound=1557 best=1610 pruned_bound=14447572 pruned_infeasible=15152099
[tsp-debug] progress: expanded=29700000 created=44212488 depth=331 bound=1594 best=1610 pruned_bound=14512166 pruned_infeasible=15187507
[tsp-debug] progress: expanded=29800000 created=44385963 depth=296 bound=1576 best=1610 pruned_bound=14585671 pruned_infeasible=15214032
[tsp-debug] progress: expanded=29900000 created=44539151 depth=358 bound=1589 best=1610 pruned_bound=14638801 pruned_infeasible=15260844
[tsp-debug] progress: expanded=30000000 created=44703742 depth=353 bound=1599 best=1610 pruned_bound=14703396 pruned_infeasible=15296253
[tsp-debug] progress: expanded=30100000 created=44876445 depth=329 bound=1598 best=1610 pruned_bound=14776121 pruned_infeasible=15323550
[tsp-debug] progress: expanded=30200000 created=45057904 depth=322 bound=1606 best=1610 pruned_bound=14857587 pruned_infeasible=15342091
[tsp-debug] progress: expanded=30300000 created=45221070 depth=344 bound=1606 best=1610 pruned_bound=14920737 pruned_infeasible=15378925
[tsp-debug] progress: expanded=30400000 created=45389270 depth=331 bound=1596 best=1610 pruned_bound=14988947 pruned_infeasible=15410725
[tsp-debug] progress: expanded=30500000 created=45564490 depth=343 bound=1604 best=1610 pruned_bound=15064158 pruned_infeasible=15435505
[tsp-debug] progress: expanded=30600000 created=45727757 depth=342 bound=1568 best=1610 pruned_bound=15127420 pruned_infeasible=15472238
[tsp-debug] progress: expanded=30700000 created=45888555 depth=332 bound=1575 best=1610 pruned_bound=15188227 pruned_infeasible=15511440
[tsp-debug] progress: expanded=30800000 created=46060502 depth=323 bound=1602 best=1610 pruned_bound=15260184 pruned_infeasible=15539493
[tsp-debug] progress: expanded=30900000 created=46231719 depth=354 bound=1609 best=1610 pruned_bound=15331379 pruned_infeasible=15568276
[tsp-debug] progress: expanded=31000000 created=46405848 depth=364 bound=1591 best=1610 pruned_bound=15405493 pruned_infeasible=15594147
[tsp-debug] progress: expanded=31100000 created=46548517 depth=356 bound=1599 best=1610 pruned_bound=15448169 pruned_infeasible=15651478
[tsp-debug] progress: expanded=31200000 created=46706983 depth=354 bound=1607 best=1610 pruned_bound=15506637 pruned_infeasible=15693012
[tsp-debug] progress: expanded=31300000 created=46877527 depth=363 bound=1601 best=1610 pruned_bound=15577176 pruned_infeasible=15722468
[tsp-debug] progress: expanded=31400000 created=47057266 depth=328 bound=1606 best=1610 pruned_bound=15656945 pruned_infeasible=15742729
[tsp-debug] progress: expanded=31500000 created=47232916 depth=368 bound=1605 best=1610 pruned_bound=15732559 pruned_infeasible=15767079
[tsp-debug] progress: expanded=31600000 created=47386599 depth=370 bound=1601 best=1610 pruned_bound=15786241 pruned_infeasible=15813396
[tsp-debug] progress: expanded=31700000 created=47553940 depth=332 bound=1563 best=1610 pruned_bound=15853613 pruned_infeasible=15846055
[tsp-debug] progress: expanded=31800000 created=47729021 depth=349 bound=1599 best=1610 pruned_bound=15928679 pruned_infeasible=15870974
[tsp-debug] progress: expanded=31900000 created=47897669 depth=350 bound=1605 best=1610 pruned_bound=15997330 pruned_infeasible=15902326
[tsp-debug] progress: expanded=32000000 created=48075636 depth=352 bound=1606 best=1610 pruned_bound=16075294 pruned_infeasible=15924359
[tsp-debug] progress: expanded=32100000 created=48246914 depth=340 bound=1599 best=1610 pruned_bound=16146581 pruned_infeasible=15953081
[tsp-debug] progress: expanded=32200000 created=48414921 depth=331 bound=1605 best=1610 pruned_bound=16214597 pruned_infeasible=15985074
[tsp-debug] progress: expanded=32300000 created=48595181 depth=356 bound=1595 best=1610 pruned_bound=16294833 pruned_infeasible=16004814
[tsp-debug] progress: expanded=32400000 created=48760078 depth=367 bound=1604 best=1610 pruned_bound=16359722 pruned_infeasible=16039917
[tsp-debug] progress: expanded=32500000 created=48938426 depth=350 bound=1606 best=1610 pruned_bound=16438083 pruned_infeasible=16061569
[tsp-debug] progress: expanded=32600000 created=49116494 depth=335 bound=1584 best=1610 pruned_bound=16516164 pruned_infeasible=16083501
[tsp-debug] progress: expanded=32700000 created=49296372 depth=306 bound=1603 best=1610 pruned_bound=16596070 pruned_infeasible=16103623
[tsp-debug] progress: expanded=32800000 created=49457005 depth=351 bound=1604 best=1610 pruned_bound=16656661 pruned_infeasible=16142990
[tsp-debug] progress: expanded=32900000 created=49634569 depth=342 bound=1565 best=1610 pruned_bound=16734232 pruned_infeasible=16165426
[tsp-debug] progress: expanded=33000000 created=49809582 depth=345 bound=1580 best=1610 pruned_bound=16809248 pruned_infeasible=16190413
[tsp-debug] progress: expanded=33100000 created=49986211 depth=342 bound=1604 best=1610 pruned_bound=16885879 pruned_infeasible=16213784
[tsp-debug] progress: expanded=33200000 created=50144409 depth=345 bound=1586 best=1610 pruned_bound=16944074 pruned_infeasible=16255586
[tsp-debug] progress: expanded=33300000 created=50318411 depth=335 bound=1608 best=1610 pruned_bound=17018084 pruned_infeasible=16281584
[tsp-debug] progress: expanded=33400000 created=50495240 depth=335 bound=1582 best=1610 pruned_bound=17094910 pruned_infeasible=16304755
[tsp-debug] progress: expanded=33500000 created=50670270 depth=357 bound=1594 best=1610 pruned_bound=17169922 pruned_infeasible=16329725
[tsp-debug] progress: expanded=33600000 created=50839318 depth=322 bound=1575 best=1610 pruned_bound=17239000 pruned_infeasible=16360677
[tsp-debug] progress: expanded=33700000 created=51024683 depth=336 bound=1602 best=1610 pruned_bound=17324351 pruned_infeasible=16375312
[tsp-debug] progress: expanded=33800000 created=51173577 depth=369 bound=1603 best=1610 pruned_bound=17373221 pruned_infeasible=16426418
[tsp-debug] progress: expanded=33900000 created=51329811 depth=345 bound=1510 best=1610 pruned_bound=17429470 pruned_infeasible=16470184
[tsp-debug] progress: expanded=34000000 created=51490277 depth=348 bound=1589 best=1610 pruned_bound=17489938 pruned_infeasible=16509718
[tsp-debug] progress: expanded=34100000 created=51656507 depth=343 bound=1585 best=1610 pruned_bound=17556170 pruned_infeasible=16543488
[tsp-debug] progress: expanded=34200000 created=51804828 depth=359 bound=1595 best=1610 pruned_bound=17604479 pruned_infeasible=16595167
[tsp-debug] progress: expanded=34300000 created=51957224 depth=352 bound=1602 best=1610 pruned_bound=17656880 pruned_infeasible=16642771
[tsp-debug] progress: expanded=34400000 created=52117109 depth=339 bound=1584 best=1610 pruned_bound=17716776 pruned_infeasible=16682886
[tsp-debug] progress: expanded=34500000 created=52279516 depth=357 bound=1609 best=1610 pruned_bound=17779170 pruned_infeasible=16720479
[tsp-debug] progress: expanded=34600000 created=52448283 depth=324 bound=1604 best=1610 pruned_bound=17847964 pruned_infeasible=16751712
[tsp-debug] progress: expanded=34700000 created=52619824 depth=343 bound=1595 best=1610 pruned_bound=17919488 pruned_infeasible=16780171
[tsp-debug] progress: expanded=34800000 created=52797321 depth=340 bound=1609 best=1610 pruned_bound=17996987 pruned_infeasible=16802674
[tsp-debug] progress: expanded=34900000 created=52973131 depth=342 bound=1576 best=1610 pruned_bound=18072794 pruned_infeasible=16826864
[tsp-debug] progress: expanded=35000000 created=53150940 depth=323 bound=1589 best=1610 pruned_bound=18150621 pruned_infeasible=16849055
[tsp-debug] progress: expanded=35100000 created=53308347 depth=347 bound=1530 best=1610 pruned_bound=18208005 pruned_infeasible=16891648
[tsp-debug] progress: expanded=35200000 created=53476514 depth=364 bound=1608 best=1610 pruned_bound=18276160 pruned_infeasible=16923481
[tsp-debug] progress: expanded=35300000 created=53650910 depth=342 bound=1596 best=1610 pruned_bound=18350575 pruned_infeasible=16949085
[tsp-debug] progress: expanded=35400000 created=53832449 depth=334 bound=1588 best=1610 pruned_bound=18432120 pruned_infeasible=16967546
[tsp-debug] progress: expanded=35500000 created=54011118 depth=334 bound=1590 best=1610 pruned_bound=18510789 pruned_infeasible=16988877
[tsp-debug] progress: expanded=35600000 created=54174757 depth=338 bound=1574 best=1610 pruned_bound=18574424 pruned_infeasible=17025238
[tsp-debug] progress: expanded=35700000 created=54350544 depth=326 bound=1571 best=1610 pruned_bound=18650222 pruned_infeasible=17049451
[tsp-debug] progress: expanded=35800000 created=54530467 depth=340 bound=1600 best=1610 pruned_bound=18730134 pruned_infeasible=17069528
[tsp-debug] progress: expanded=35900000 created=54708026 depth=339 bound=1596 best=1610 pruned_bound=18807696 pruned_infeasible=17091969
[tsp-debug] progress: expanded=36000000 created=54886405 depth=313 bound=1569 best=1610 pruned_bound=18886096 pruned_infeasible=17113590
[tsp-debug] progress: expanded=36100000 created=55067510 depth=354 bound=1606 best=1610 pruned_bound=18967163 pruned_infeasible=17132485
[tsp-debug] progress: expanded=36200000 created=55251150 depth=330 bound=1608 best=1610 pruned_bound=19050825 pruned_infeasible=17148845
[tsp-debug] progress: expanded=36300000 created=55435647 depth=342 bound=1609 best=1610 pruned_bound=19135310 pruned_infeasible=17164348
[tsp-debug] progress: expanded=36400000 created=55615418 depth=343 bound=1604 best=1610 pruned_bound=19215080 pruned_infeasible=17184577
[tsp-debug] progress: expanded=36500000 created=55764685 depth=360 bound=1597 best=1610 pruned_bound=19264332 pruned_infeasible=17235310
[tsp-debug] progress: expanded=36600000 created=55922693 depth=341 bound=1590 best=1610 pruned_bound=19322358 pruned_infeasible=17277302
[tsp-debug] progress: expanded=36700000 created=56084986 depth=359 bound=1597 best=1610 pruned_bound=19384636 pruned_infeasible=17315009
[tsp-debug] progress: expanded=36800000 created=56252931 depth=355 bound=1574 best=1610 pruned_bound=19452583 pruned_infeasible=17347064
[tsp-debug] progress: expanded=36900000 created=56399205 depth=360 bound=1596 best=1610 pruned_bound=19498852 pruned_infeasible=17400790
[tsp-debug] progress: expanded=37000000 created=56557051 depth=351 bound=1602 best=1610 pruned_bound=19556708 pruned_infeasible=17442944
[tsp-debug] progress: expanded=37100000 created=56721301 depth=341 bound=1531 best=1610 pruned_bound=19620965 pruned_infeasible=17478694
[tsp-debug] progress: expanded=37200000 created=56892452 depth=355 bound=1596 best=1610 pruned_bound=19692107 pruned_infeasible=17507543
[tsp-debug] progress: expanded=37300000 created=57063149 depth=330 bound=1606 best=1610 pruned_bound=19762827 pruned_infeasible=17536846
[tsp-debug] progress: expanded=37400000 created=57238336 depth=340 bound=1598 best=1610 pruned_bound=19838003 pruned_infeasible=17561659
[tsp-debug] progress: expanded=37500000 created=57416005 depth=343 bound=1567 best=1610 pruned_bound=19915669 pruned_infeasible=17583990
[tsp-debug] progress: expanded=37600000 created=57593330 depth=345 bound=1597 best=1610 pruned_bound=19992992 pruned_infeasible=17606665
[tsp-debug] progress: expanded=37700000 created=57771770 depth=337 bound=1604 best=1610 pruned_bound=20071441 pruned_infeasible=17628225
[tsp-debug] progress: expanded=37800000 created=57937543 depth=358 bound=1601 best=1610 pruned_bound=20137195 pruned_infeasible=17662452
[tsp-debug] progress: expanded=37900000 created=58096062 depth=339 bound=1568 best=1610 pruned_bound=20195728 pruned_infeasible=17703933
[tsp-debug] progress: expanded=38000000 created=58268130 depth=343 bound=1607 best=1610 pruned_bound=20267794 pruned_infeasible=17731865
[tsp-debug] progress: expanded=38100000 created=58446598 depth=335 bound=1594 best=1610 pruned_bound=20346268 pruned_infeasible=17753397
[tsp-debug] progress: expanded=38200000 created=58628908 depth=346 bound=1601 best=1610 pruned_bound=20428568 pruned_infeasible=17771087
[tsp-debug] progress: expanded=38300000 created=58795479 depth=344 bound=1572 best=1610 pruned_bound=20495141 pruned_infeasible=17804516
[tsp-debug] progress: expanded=38400000 created=58968666 depth=315 bound=1577 best=1610 pruned_bound=20568355 pruned_infeasible=17831329
[tsp-debug] progress: expanded=38500000 created=59150770 depth=340 bound=1570 best=1610 pruned_bound=20650435 pruned_infeasible=17849225
[tsp-debug] progress: expanded=38600000 created=59333679 depth=333 bound=1608 best=1610 pruned_bound=20733350 pruned_infeasible=17866316
[tsp-debug] progress: expanded=38700000 created=59512275 depth=329 bound=1607 best=1610 pruned_bound=20811951 pruned_infeasible=17887720
[tsp-debug] progress: expanded=38800000 created=59693300 depth=347 bound=1606 best=1610 pruned_bound=20892959 pruned_infeasible=17906695
[tsp-debug] progress: expanded=38900000 created=59877602 depth=318 bound=1556 best=1610 pruned_bound=20977287 pruned_infeasible=17922393
[tsp-debug] progress: expanded=39000000 created=60063041 depth=304 bound=1605 best=1610 pruned_bound=21062741 pruned_infeasible=17936954
[tsp-debug] progress: expanded=39100000 created=60243873 depth=333 bound=1587 best=1610 pruned_bound=21143549 pruned_infeasible=17956122
[tsp-debug] progress: expanded=39200000 created=60426900 depth=309 bound=1608 best=1610 pruned_bound=21226595 pruned_infeasible=17973095
[tsp-debug] progress: expanded=39300000 created=60577914 depth=371 bound=1600 best=1610 pruned_bound=21277559 pruned_infeasible=18022081
[tsp-debug] progress: expanded=39400000 created=60740250 depth=332 bound=1598 best=1610 pruned_bound=21339926 pruned_infeasible=18059745
[tsp-debug] progress: expanded=39500000 created=60921710 depth=344 bound=1550 best=1610 pruned_bound=21421373 pruned_infeasible=18078285
[tsp-debug] progress: expanded=39600000 created=61094405 depth=344 bound=1594 best=1610 pruned_bound=21494067 pruned_infeasible=18105590
[tsp-debug] progress: expanded=39700000 created=61275257 depth=327 bound=1598 best=1610 pruned_bound=21574935 pruned_infeasible=18124738
[tsp-debug] progress: expanded=39800000 created=61446876 depth=380 bound=1600 best=1610 pruned_bound=21646516 pruned_infeasible=18153119
[tsp-debug] progress: expanded=39900000 created=61573510 depth=379 bound=1579 best=1610 pruned_bound=21673149 pruned_infeasible=18226485
[tsp-debug] progress: expanded=40000000 created=61714644 depth=355 bound=1595 best=1610 pruned_bound=21714296 pruned_infeasible=18285351
[tsp-debug] progress: expanded=40100000 created=61856874 depth=365 bound=1603 best=1610 pruned_bound=21756518 pruned_infeasible=18343121
[tsp-debug] progress: expanded=40200000 created=62008196 depth=354 bound=1601 best=1610 pruned_bound=21807851 pruned_infeasible=18391799
[tsp-debug] progress: expanded=40300000 created=62152649 depth=377 bound=1600 best=1610 pruned_bound=21852286 pruned_infeasible=18447346
[tsp-debug] progress: expanded=40400000 created=62314679 depth=369 bound=1598 best=1610 pruned_bound=21914321 pruned_infeasible=18485316
[tsp-debug] progress: expanded=40500000 created=62469608 depth=359 bound=1584 best=1610 pruned_bound=21969257 pruned_infeasible=18530387
[tsp-debug] progress: expanded=40600000 created=62636333 depth=370 bound=1608 best=1610 pruned_bound=22035980 pruned_infeasible=18563662
[tsp-debug] progress: expanded=40700000 created=62807061 depth=338 bound=1588 best=1610 pruned_bound=22106728 pruned_infeasible=18592934
[tsp-debug] progress: expanded=40800000 created=62980665 depth=348 bound=1593 best=1610 pruned_bound=22180322 pruned_infeasible=18619330
[tsp-debug] progress: expanded=40900000 created=63160547 depth=333 bound=1580 best=1610 pruned_bound=22260218 pruned_infeasible=18639448
[tsp-debug] progress: expanded=41000000 created=63338896 depth=336 bound=1592 best=1610 pruned_bound=22338565 pruned_infeasible=18661099
[tsp-debug] progress: expanded=41100000 created=63515643 depth=324 bound=1599 best=1610 pruned_bound=22415324 pruned_infeasible=18684352
[tsp-debug] progress: expanded=41200000 created=63695170 depth=323 bound=1608 best=1610 pruned_bound=22494851 pruned_infeasible=18704825
[tsp-debug] progress: expanded=41300000 created=63857715 depth=325 bound=1588 best=1610 pruned_bound=22557395 pruned_infeasible=18742280
[tsp-debug] progress: expanded=41400000 created=64037589 depth=327 bound=1604 best=1610 pruned_bound=22637267 pruned_infeasible=18762406
[tsp-debug] progress: expanded=41500000 created=64217649 depth=332 bound=1605 best=1610 pruned_bound=22717323 pruned_infeasible=18782346
[tsp-debug] progress: expanded=41600000 created=64396392 depth=313 bound=1605 best=1610 pruned_bound=22796085 pruned_infeasible=18803603
[tsp-debug] progress: expanded=41700000 created=64570079 depth=332 bound=1586 best=1610 pruned_bound=22869751 pruned_infeasible=18829916
[tsp-debug] progress: expanded=41800000 created=64754103 depth=303 bound=1573 best=1610 pruned_bound=22953803 pruned_infeasible=18845892
[tsp-debug] progress: expanded=41900000 created=64922826 depth=359 bound=1600 best=1610 pruned_bound=23022476 pruned_infeasible=18877169
[tsp-debug] progress: expanded=42000000 created=65102038 depth=326 bound=1602 best=1610 pruned_bound=23101718 pruned_infeasible=18897957
[tsp-debug] progress: expanded=42100000 created=65285666 depth=364 bound=1597 best=1610 pruned_bound=23185314 pruned_infeasible=18914329
[tsp-debug] progress: expanded=42200000 created=65456694 depth=333 bound=1605 best=1610 pruned_bound=23256366 pruned_infeasible=18943301
[tsp-debug] progress: expanded=42300000 created=65642861 depth=329 bound=1583 best=1610 pruned_bound=23342537 pruned_infeasible=18957134
[tsp-debug] progress: expanded=42400000 created=65817596 depth=323 bound=1592 best=1610 pruned_bound=23417278 pruned_infeasible=18982399
[tsp-debug] progress: expanded=42500000 created=66001421 depth=345 bound=1594 best=1610 pruned_bound=23501084 pruned_infeasible=18998574
[tsp-debug] progress: expanded=42600000 created=66166238 depth=363 bound=1608 best=1610 pruned_bound=23565885 pruned_infeasible=19033757
[tsp-debug] progress: expanded=42700000 created=66334125 depth=333 bound=1608 best=1610 pruned_bound=23633797 pruned_infeasible=19065870
[tsp-debug] progress: expanded=42800000 created=66517286 depth=325 bound=1606 best=1610 pruned_bound=23716966 pruned_infeasible=19082709
[tsp-debug] progress: expanded=42900000 created=66692739 depth=362 bound=1598 best=1610 pruned_bound=23792387 pruned_infeasible=19107256
[tsp-debug] progress: expanded=43000000 created=66870131 depth=331 bound=1604 best=1610 pruned_bound=23869805 pruned_infeasible=19129864
[tsp-debug] progress: expanded=43100000 created=67053814 depth=378 bound=1583 best=1610 pruned_bound=23953451 pruned_infeasible=19146181
[tsp-debug] progress: expanded=43200000 created=67181016 depth=375 bound=1600 best=1610 pruned_bound=23980654 pruned_infeasible=19218979
[tsp-debug] progress: expanded=43300000 created=67325055 depth=376 bound=1608 best=1610 pruned_bound=24024692 pruned_infeasible=19274940
[tsp-debug] progress: expanded=43400000 created=67472444 depth=369 bound=1580 best=1610 pruned_bound=24072086 pruned_infeasible=19327551
[tsp-debug] progress: expanded=43500000 created=67622908 depth=380 bound=1608 best=1610 pruned_bound=24122545 pruned_infeasible=19377087
[tsp-debug] progress: expanded=43600000 created=67787179 depth=363 bound=1609 best=1610 pruned_bound=24186825 pruned_infeasible=19412816
[tsp-debug] progress: expanded=43700000 created=67947487 depth=348 bound=1595 best=1610 pruned_bound=24247147 pruned_infeasible=19452508
[tsp-debug] progress: expanded=43800000 created=68119081 depth=344 bound=1596 best=1610 pruned_bound=24318742 pruned_infeasible=19480914
[tsp-debug] progress: expanded=43900000 created=68293799 depth=317 bound=1608 best=1610 pruned_bound=24393486 pruned_infeasible=19506196
[tsp-debug] progress: expanded=44000000 created=68472086 depth=341 bound=1606 best=1610 pruned_bound=24471750 pruned_infeasible=19527909
[tsp-debug] progress: expanded=44100000 created=68657013 depth=367 bound=1598 best=1610 pruned_bound=24556656 pruned_infeasible=19542982
[tsp-debug] progress: expanded=44200000 created=68825522 depth=333 bound=1575 best=1610 pruned_bound=24625194 pruned_infeasible=19574473
[tsp-debug] progress: expanded=44300000 created=69010941 depth=324 bound=1608 best=1610 pruned_bound=24710622 pruned_infeasible=19589054
[tsp-debug] progress: expanded=44400000 created=69186857 depth=340 bound=1600 best=1610 pruned_bound=24786527 pruned_infeasible=19613138
[tsp-debug] progress: expanded=44500000 created=69359512 depth=341 bound=1536 best=1610 pruned_bound=24859178 pruned_infeasible=19640483
[tsp-debug] progress: expanded=44600000 created=69538355 depth=339 bound=1578 best=1610 pruned_bound=24938023 pruned_infeasible=19661640
[tsp-debug] progress: expanded=44700000 created=69713812 depth=363 bound=1591 best=1610 pruned_bound=25013463 pruned_infeasible=19686183
[tsp-debug] progress: expanded=44800000 created=69891076 depth=335 bound=1600 best=1610 pruned_bound=25090752 pruned_infeasible=19708919
[tsp-debug] progress: expanded=44900000 created=70071809 depth=344 bound=1592 best=1610 pruned_bound=25171473 pruned_infeasible=19728186
[tsp-debug] progress: expanded=45000000 created=70238570 depth=370 bound=1592 best=1610 pruned_bound=25238211 pruned_infeasible=19761425
[tsp-debug] progress: expanded=45100000 created=70414019 depth=328 bound=1590 best=1610 pruned_bound=25313697 pruned_infeasible=19785976
[tsp-debug] progress: expanded=45200000 created=70598134 depth=356 bound=1608 best=1610 pruned_bound=25397786 pruned_infeasible=19801861
[tsp-debug] progress: expanded=45300000 created=70774086 depth=315 bound=1588 best=1610 pruned_bound=25473775 pruned_infeasible=19825909
[tsp-debug] progress: expanded=45400000 created=70956369 depth=301 bound=1560 best=1610 pruned_bound=25556071 pruned_infeasible=19843626
[tsp-debug] progress: expanded=45500000 created=71122913 depth=337 bound=1596 best=1610 pruned_bound=25622581 pruned_infeasible=19877082
[tsp-debug] progress: expanded=45600000 created=71304700 depth=336 bound=1579 best=1610 pruned_bound=25704368 pruned_infeasible=19895295
[tsp-debug] progress: expanded=45700000 created=71476157 depth=368 bound=1602 best=1610 pruned_bound=25775799 pruned_infeasible=19923838
[tsp-debug] progress: expanded=45800000 created=71651397 depth=302 bound=1576 best=1610 pruned_bound=25851098 pruned_infeasible=19948598
[tsp-debug] progress: expanded=45900000 created=71828391 depth=370 bound=1593 best=1610 pruned_bound=25928031 pruned_infeasible=19971604
[tsp-debug] progress: expanded=46000000 created=71972643 depth=361 bound=1605 best=1610 pruned_bound=25972290 pruned_infeasible=20027352
[tsp-debug] progress: expanded=46100000 created=72125440 depth=377 bound=1603 best=1610 pruned_bound=26025077 pruned_infeasible=20074555
[tsp-debug] progress: expanded=46200000 created=72289063 depth=357 bound=1602 best=1610 pruned_bound=26088712 pruned_infeasible=20110932
[tsp-debug] progress: expanded=46300000 created=72459612 depth=351 bound=1606 best=1610 pruned_bound=26159268 pruned_infeasible=20140383
[tsp-debug] progress: expanded=46400000 created=72637314 depth=349 bound=1599 best=1610 pruned_bound=26236973 pruned_infeasible=20162681
[tsp-debug] progress: expanded=46500000 created=72814645 depth=356 bound=1609 best=1610 pruned_bound=26314296 pruned_infeasible=20185350
[tsp-debug] progress: expanded=46600000 created=72999782 depth=298 bound=1600 best=1610 pruned_bound=26399487 pruned_infeasible=20200213
[tsp-debug] progress: expanded=46700000 created=73166940 depth=325 bound=1534 best=1610 pruned_bound=26466618 pruned_infeasible=20233055
[tsp-debug] progress: expanded=46800000 created=73334923 depth=310 bound=1607 best=1610 pruned_bound=26534618 pruned_infeasible=20265072
[tsp-debug] progress: expanded=46900000 created=73518239 depth=333 bound=1608 best=1610 pruned_bound=26617913 pruned_infeasible=20281756
[tsp-debug] progress: expanded=47000000 created=73693664 depth=347 bound=1579 best=1610 pruned_bound=26693323 pruned_infeasible=20306331
[tsp-debug] progress: expanded=47100000 created=73873460 depth=344 bound=1606 best=1610 pruned_bound=26773123 pruned_infeasible=20326535
[tsp-debug] progress: expanded=47200000 created=74056489 depth=340 bound=1603 best=1610 pruned_bound=26856154 pruned_infeasible=20343506
[tsp-debug] progress: expanded=47300000 created=74233781 depth=343 bound=1577 best=1610 pruned_bound=26933448 pruned_infeasible=20366214
[tsp-debug] progress: expanded=47400000 created=74415771 depth=342 bound=1609 best=1610 pruned_bound=27015439 pruned_infeasible=20384224
[tsp-debug] progress: expanded=47500000 created=74600086 depth=367 bound=1603 best=1610 pruned_bound=27099728 pruned_infeasible=20399909
[tsp-debug] progress: expanded=47600000 created=74784990 depth=344 bound=1593 best=1610 pruned_bound=27184654 pruned_infeasible=20415005
[tsp-debug] progress: expanded=47700000 created=74963876 depth=313 bound=1607 best=1610 pruned_bound=27263568 pruned_infeasible=20436119
[tsp-debug] progress: expanded=47800000 created=75143637 depth=330 bound=1565 best=1610 pruned_bound=27343313 pruned_infeasible=20456358
[tsp-debug] progress: expanded=47900000 created=75329630 depth=310 bound=1599 best=1610 pruned_bound=27429324 pruned_infeasible=20470365
[tsp-debug] progress: expanded=48000000 created=75510907 depth=319 bound=1571 best=1610 pruned_bound=27510592 pruned_infeasible=20489088
[tsp-debug] progress: expanded=48100000 created=75684794 depth=341 bound=1575 best=1610 pruned_bound=27584457 pruned_infeasible=20515201
[tsp-debug] progress: expanded=48200000 created=75870094 depth=365 bound=1604 best=1610 pruned_bound=27669738 pruned_infeasible=20529901
[tsp-debug] progress: expanded=48300000 created=76045728 depth=336 bound=1601 best=1610 pruned_bound=27745396 pruned_infeasible=20554267
[tsp-debug] progress: expanded=48400000 created=76225238 depth=332 bound=1599 best=1610 pruned_bound=27824910 pruned_infeasible=20574757
[tsp-debug] progress: expanded=48500000 created=76399669 depth=347 bound=1576 best=1610 pruned_bound=27899330 pruned_infeasible=20600326
[tsp-debug] progress: expanded=48600000 created=76562697 depth=341 bound=1597 best=1610 pruned_bound=27962361 pruned_infeasible=20637298
[tsp-debug] progress: expanded=48700000 created=76734724 depth=340 bound=1582 best=1610 pruned_bound=28034390 pruned_infeasible=20665271
[tsp-debug] progress: expanded=48800000 created=76911016 depth=337 bound=1603 best=1610 pruned_bound=28110685 pruned_infeasible=20688979
[tsp-debug] progress: expanded=48900000 created=77087288 depth=323 bound=1599 best=1610 pruned_bound=28186970 pruned_infeasible=20712707
[tsp-debug] progress: expanded=49000000 created=77266838 depth=338 bound=1606 best=1610 pruned_bound=28266506 pruned_infeasible=20733157
[tsp-debug] progress: expanded=49100000 created=77447249 depth=343 bound=1607 best=1610 pruned_bound=28346911 pruned_infeasible=20752746
[tsp-debug] progress: expanded=49200000 created=77628276 depth=280 bound=1596 best=1610 pruned_bound=28427999 pruned_infeasible=20771719
[tsp-debug] progress: expanded=49300000 created=77810902 depth=354 bound=1599 best=1610 pruned_bound=28510562 pruned_infeasible=20789093
[tsp-debug] progress: expanded=49400000 created=77995667 depth=289 bound=1595 best=1610 pruned_bound=28595384 pruned_infeasible=20804328
[tsp-debug] progress: expanded=49500000 created=78166767 depth=329 bound=1604 best=1610 pruned_bound=28666443 pruned_infeasible=20833228
[tsp-debug] progress: expanded=49600000 created=78352608 depth=328 bound=1571 best=1610 pruned_bound=28752285 pruned_infeasible=20847387
[tsp-debug] progress: expanded=49700000 created=78534344 depth=333 bound=1599 best=1610 pruned_bound=28834016 pruned_infeasible=20865651
[tsp-debug] progress: expanded=49800000 created=78721678 depth=344 bound=1587 best=1610 pruned_bound=28921344 pruned_infeasible=20878317
[tsp-debug] progress: expanded=49900000 created=78899475 depth=343 bound=1595 best=1610 pruned_bound=28999137 pruned_infeasible=20900520
[tsp-debug] progress: expanded=50000000 created=79073285 depth=363 bound=1604 best=1610 pruned_bound=29072935 pruned_infeasible=20926710
[tsp-debug] progress: expanded=50100000 created=79238101 depth=343 bound=1603 best=1610 pruned_bound=29137766 pruned_infeasible=20961894
[tsp-debug] progress: expanded=50200000 created=79412162 depth=321 bound=1582 best=1610 pruned_bound=29211845 pruned_infeasible=20987833
[tsp-debug] progress: expanded=50300000 created=79593748 depth=319 bound=1559 best=1610 pruned_bound=29293432 pruned_infeasible=21006247
[tsp-debug] progress: expanded=50400000 created=79767997 depth=340 bound=1586 best=1610 pruned_bound=29367662 pruned_infeasible=21031998
[tsp-debug] progress: expanded=50500000 created=79948683 depth=332 bound=1599 best=1610 pruned_bound=29448356 pruned_infeasible=21051312
[tsp-debug] progress: expanded=50600000 created=80132839 depth=327 bound=1608 best=1610 pruned_bound=29532518 pruned_infeasible=21067156
[tsp-debug] progress: expanded=50700000 created=80318486 depth=322 bound=1572 best=1610 pruned_bound=29618168 pruned_infeasible=21081509
[tsp-debug] progress: expanded=50800000 created=80504136 depth=350 bound=1587 best=1610 pruned_bound=29703795 pruned_infeasible=21095859
[tsp-debug] progress: expanded=50900000 created=80693303 depth=332 bound=1593 best=1610 pruned_bound=29792976 pruned_infeasible=21106692
[tsp-debug] progress: expanded=51000000 created=80876806 depth=333 bound=1606 best=1610 pruned_bound=29876480 pruned_infeasible=21123189
[tsp-debug] progress: expanded=51100000 created=81056834 depth=344 bound=1603 best=1610 pruned_bound=29956495 pruned_infeasible=21143161
[tsp-debug] progress: expanded=51200000 created=81242981 depth=276 bound=1554 best=1610 pruned_bound=30042707 pruned_infeasible=21157014
[tsp-debug] progress: expanded=51300000 created=81412540 depth=338 bound=1596 best=1610 pruned_bound=30112209 pruned_infeasible=21187455
[tsp-debug] progress: expanded=51400000 created=81595496 depth=327 bound=1603 best=1610 pruned_bound=30195173 pruned_infeasible=21204499
[tsp-debug] progress: expanded=51500000 created=81771668 depth=364 bound=1609 best=1610 pruned_bound=30271319 pruned_infeasible=21228327
[tsp-debug] progress: expanded=51600000 created=81941135 depth=321 bound=1607 best=1610 pruned_bound=30340822 pruned_infeasible=21258860
[tsp-debug] progress: expanded=51700000 created=82122691 depth=331 bound=1604 best=1610 pruned_bound=30422366 pruned_infeasible=21277304
[tsp-debug] progress: expanded=51800000 created=82306296 depth=328 bound=1602 best=1610 pruned_bound=30505972 pruned_infeasible=21293699
[tsp-debug] progress: expanded=51900000 created=82492779 depth=283 bound=1607 best=1610 pruned_bound=30592499 pruned_infeasible=21307216
[tsp-debug] progress: expanded=52000000 created=82667225 depth=357 bound=1604 best=1610 pruned_bound=30666876 pruned_infeasible=21332770
[tsp-debug] progress: expanded=52100000 created=82839244 depth=328 bound=1605 best=1610 pruned_bound=30738920 pruned_infeasible=21360751
[tsp-debug] progress: expanded=52200000 created=83027430 depth=373 bound=1609 best=1610 pruned_bound=30827070 pruned_infeasible=21372565
[tsp-debug] progress: expanded=52300000 created=83205281 depth=326 bound=1552 best=1610 pruned_bound=30904959 pruned_infeasible=21394714
[tsp-debug] progress: expanded=52400000 created=83388945 depth=328 bound=1608 best=1610 pruned_bound=30988621 pruned_infeasible=21411050
[tsp-debug] progress: expanded=52500000 created=83569405 depth=280 bound=1608 best=1610 pruned_bound=31069130 pruned_infeasible=21430590
[tsp-debug] progress: expanded=52600000 created=83756910 depth=295 bound=1554 best=1610 pruned_bound=31156617 pruned_infeasible=21443085
[tsp-debug] progress: expanded=52700000 created=83939000 depth=326 bound=1605 best=1610 pruned_bound=31238679 pruned_infeasible=21460995
[tsp-debug] progress: expanded=52800000 created=84125781 depth=267 bound=1600 best=1610 pruned_bound=31325517 pruned_infeasible=21474214
[tsp-debug] progress: expanded=52900000 created=84311871 depth=294 bound=1576 best=1610 pruned_bound=31411580 pruned_infeasible=21488124
[tsp-debug] progress: expanded=53000000 created=84498141 depth=318 bound=1587 best=1610 pruned_bound=31497827 pruned_infeasible=21501854
[tsp-debug] progress: expanded=53100000 created=84683466 depth=310 bound=1605 best=1610 pruned_bound=31583159 pruned_infeasible=21516529
[tsp-debug] progress: expanded=53200000 created=84869751 depth=341 bound=1592 best=1610 pruned_bound=31669414 pruned_infeasible=21530244
[tsp-debug] progress: expanded=53300000 created=85058602 depth=264 bound=1502 best=1610 pruned_bound=31758341 pruned_infeasible=21541393
[tsp-debug] progress: expanded=53400000 created=85237907 depth=330 bound=1606 best=1610 pruned_bound=31837583 pruned_infeasible=21562088
[tsp-debug] progress: expanded=53500000 created=85423319 depth=340 bound=1604 best=1610 pruned_bound=31922986 pruned_infeasible=21576676
[tsp-debug] progress: expanded=53600000 created=85611760 depth=300 bound=1571 best=1610 pruned_bound=32011463 pruned_infeasible=21588235
[tsp-debug] progress: expanded=53700000 created=85799892 depth=337 bound=1600 best=1610 pruned_bound=32099558 pruned_infeasible=21600103
[tsp-debug] progress: expanded=53800000 created=85985091 depth=344 bound=1541 best=1610 pruned_bound=32184752 pruned_infeasible=21614904
[tsp-debug] progress: expanded=53900000 created=86169088 depth=339 bound=1609 best=1610 pruned_bound=32268756 pruned_infeasible=21630907
[tsp-debug] progress: expanded=54000000 created=86359844 depth=360 bound=1603 best=1610 pruned_bound=32359492 pruned_infeasible=21640151
[tsp-debug] progress: expanded=54100000 created=86541517 depth=335 bound=1546 best=1610 pruned_bound=32441185 pruned_infeasible=21658478
[tsp-debug] progress: expanded=54200000 created=86730218 depth=306 bound=1599 best=1610 pruned_bound=32529915 pruned_infeasible=21669777
[tsp-debug] progress: expanded=54300000 created=86917240 depth=319 bound=1599 best=1610 pruned_bound=32616924 pruned_infeasible=21682755
[tsp-debug] progress: expanded=54400000 created=87106728 depth=328 bound=1548 best=1610 pruned_bound=32706403 pruned_infeasible=21693267
[tsp-debug] progress: expanded=54500000 created=87296423 depth=219 bound=1600 best=1610 pruned_bound=32796206 pruned_infeasible=21703572
[tsp-debug] progress: expanded=54600000 created=87482524 depth=289 bound=1601 best=1610 pruned_bound=32882238 pruned_infeasible=21717471
[tsp-debug] progress: expanded=54700000 created=87674553 depth=262 bound=1592 best=1610 pruned_bound=32974293 pruned_infeasible=21725442
[tsp-debug] progress: expanded=54800000 created=87866946 depth=254 bound=1596 best=1610 pruned_bound=33066694 pruned_infeasible=21733049
[tsp-debug] progress: expanded=54900000 created=88058735 depth=330 bound=1606 best=1610 pruned_bound=33158409 pruned_infeasible=21741260
[tsp-debug] progress: expanded=55000000 created=88251117 depth=262 bound=1597 best=1610 pruned_bound=33250856 pruned_infeasible=21748878
[tsp-debug] progress: expanded=55100000 created=88379634 depth=364 bound=1599 best=1610 pruned_bound=33279282 pruned_infeasible=21820361
[tsp-debug] progress: expanded=55200000 created=88514703 depth=368 bound=1603 best=1610 pruned_bound=33314349 pruned_infeasible=21885292
[tsp-debug] progress: expanded=55300000 created=88654319 depth=354 bound=1571 best=1610 pruned_bound=33353973 pruned_infeasible=21945676
[tsp-debug] progress: expanded=55400000 created=88800388 depth=369 bound=1607 best=1610 pruned_bound=33400032 pruned_infeasible=21999607
[tsp-debug] progress: expanded=55500000 created=88946893 depth=361 bound=1598 best=1610 pruned_bound=33446545 pruned_infeasible=22053102
[tsp-debug] progress: expanded=55600000 created=89090638 depth=367 bound=1607 best=1610 pruned_bound=33490282 pruned_infeasible=22109357
[tsp-debug] progress: expanded=55700000 created=89220445 depth=370 bound=1607 best=1610 pruned_bound=33520093 pruned_infeasible=22179550
[tsp-debug] progress: expanded=55800000 created=89363541 depth=370 bound=1604 best=1610 pruned_bound=33563187 pruned_infeasible=22236454
[tsp-debug] progress: expanded=55900000 created=89510235 depth=355 bound=1606 best=1610 pruned_bound=33609891 pruned_infeasible=22289760
[tsp-debug] progress: expanded=56000000 created=89658020 depth=350 bound=1583 best=1610 pruned_bound=33657679 pruned_infeasible=22341975
[tsp-debug] progress: expanded=56100000 created=89806725 depth=365 bound=1574 best=1610 pruned_bound=33706370 pruned_infeasible=22393270
[tsp-debug] progress: expanded=56200000 created=89955992 depth=369 bound=1592 best=1610 pruned_bound=33755636 pruned_infeasible=22444003
[tsp-debug] progress: expanded=56300000 created=90111981 depth=372 bound=1597 best=1610 pruned_bound=33811622 pruned_infeasible=22488014
[tsp-debug] progress: expanded=56400000 created=90253292 depth=364 bound=1604 best=1610 pruned_bound=33852941 pruned_infeasible=22546703
[tsp-debug] progress: expanded=56500000 created=90404206 depth=355 bound=1572 best=1610 pruned_bound=33903858 pruned_infeasible=22595789
[tsp-debug] progress: expanded=56600000 created=90554868 depth=345 bound=1600 best=1610 pruned_bound=33954530 pruned_infeasible=22645127
[tsp-debug] progress: expanded=56700000 created=90714111 depth=364 bound=1599 best=1610 pruned_bound=34013760 pruned_infeasible=22685884
[tsp-debug] progress: expanded=56800000 created=90874190 depth=367 bound=1604 best=1610 pruned_bound=34073834 pruned_infeasible=22725805
[tsp-debug] progress: expanded=56900000 created=91034719 depth=369 bound=1602 best=1610 pruned_bound=34134362 pruned_infeasible=22765276
[tsp-debug] progress: expanded=57000000 created=91196143 depth=348 bound=1583 best=1610 pruned_bound=34195804 pruned_infeasible=22803852
[tsp-debug] progress: expanded=57100000 created=91359404 depth=334 bound=1599 best=1610 pruned_bound=34259077 pruned_infeasible=22840591
[tsp-debug] progress: expanded=57200000 created=91509395 depth=362 bound=1608 best=1610 pruned_bound=34309042 pruned_infeasible=22890600
[tsp-debug] progress: expanded=57300000 created=91671714 depth=337 bound=1608 best=1610 pruned_bound=34371384 pruned_infeasible=22928281
[tsp-debug] progress: expanded=57400000 created=91833407 depth=340 bound=1604 best=1610 pruned_bound=34433073 pruned_infeasible=22966588
[tsp-debug] progress: expanded=57500000 created=91999522 depth=363 bound=1604 best=1610 pruned_bound=34499169 pruned_infeasible=23000473
[tsp-debug] progress: expanded=57600000 created=92149450 depth=371 bound=1608 best=1610 pruned_bound=34549097 pruned_infeasible=23050545
[tsp-debug] progress: expanded=57700000 created=92295301 depth=363 bound=1585 best=1610 pruned_bound=34594949 pruned_infeasible=23104694
[tsp-debug] progress: expanded=57800000 created=92454305 depth=353 bound=1599 best=1610 pruned_bound=34653961 pruned_infeasible=23145690
[tsp-debug] progress: expanded=57900000 created=92621579 depth=368 bound=1606 best=1610 pruned_bound=34721227 pruned_infeasible=23178416
[tsp-debug] progress: expanded=58000000 created=92785001 depth=365 bound=1606 best=1610 pruned_bound=34784646 pruned_infeasible=23214994
[tsp-debug] progress: expanded=58100000 created=92951973 depth=367 bound=1606 best=1610 pruned_bound=34851618 pruned_infeasible=23248022
[tsp-debug] progress: expanded=58200000 created=93112919 depth=326 bound=1569 best=1610 pruned_bound=34912598 pruned_infeasible=23287076
[tsp-debug] progress: expanded=58300000 created=93282365 depth=336 bound=1606 best=1610 pruned_bound=34982035 pruned_infeasible=23317630
[tsp-debug] progress: expanded=58400000 created=93450040 depth=365 bound=1606 best=1610 pruned_bound=35049687 pruned_infeasible=23349955
[tsp-debug] progress: expanded=58500000 created=93617109 depth=359 bound=1604 best=1610 pruned_bound=35116758 pruned_infeasible=23382886
[tsp-debug] progress: expanded=58600000 created=93789001 depth=367 bound=1600 best=1610 pruned_bound=35188645 pruned_infeasible=23410994
[tsp-debug] progress: expanded=58700000 created=93962597 depth=345 bound=1600 best=1610 pruned_bound=35262260 pruned_infeasible=23437398
[tsp-debug] progress: expanded=58800000 created=94136340 depth=342 bound=1602 best=1610 pruned_bound=35336004 pruned_infeasible=23463655
[tsp-debug] progress: expanded=58900000 created=94305199 depth=364 bound=1609 best=1610 pruned_bound=35404848 pruned_infeasible=23494796
[tsp-debug] progress: expanded=59000000 created=94473431 depth=358 bound=1602 best=1610 pruned_bound=35473080 pruned_infeasible=23526564
[tsp-debug] progress: expanded=59100000 created=94641272 depth=360 bound=1604 best=1610 pruned_bound=35540921 pruned_infeasible=23558723
[tsp-debug] progress: expanded=59200000 created=94816668 depth=330 bound=1591 best=1610 pruned_bound=35616344 pruned_infeasible=23583327
[tsp-debug] progress: expanded=59300000 created=94995202 depth=366 bound=1597 best=1610 pruned_bound=35694846 pruned_infeasible=23604793
[tsp-debug] progress: expanded=59400000 created=95171509 depth=344 bound=1594 best=1610 pruned_bound=35771173 pruned_infeasible=23628486
[tsp-debug] progress: expanded=59500000 created=95345171 depth=336 bound=1596 best=1610 pruned_bound=35844842 pruned_infeasible=23654824
[tsp-debug] progress: expanded=59600000 created=95523063 depth=334 bound=1607 best=1610 pruned_bound=35922734 pruned_infeasible=23676932
[tsp-debug] progress: expanded=59700000 created=95678617 depth=340 bound=1584 best=1610 pruned_bound=35978283 pruned_infeasible=23721378
[tsp-debug] progress: expanded=59800000 created=95859298 depth=341 bound=1609 best=1610 pruned_bound=36058963 pruned_infeasible=23740697
[tsp-debug] progress: expanded=59900000 created=96030122 depth=341 bound=1581 best=1610 pruned_bound=36129786 pruned_infeasible=23769873
[tsp-debug] progress: expanded=60000000 created=96205494 depth=330 bound=1608 best=1610 pruned_bound=36205169 pruned_infeasible=23794501
[tsp-debug] progress: expanded=60100000 created=96378576 depth=326 bound=1580 best=1610 pruned_bound=36278255 pruned_infeasible=23821419
[tsp-debug] progress: expanded=60200000 created=96556448 depth=344 bound=1607 best=1610 pruned_bound=36356111 pruned_infeasible=23843547
[tsp-debug] progress: expanded=60300000 created=96735856 depth=304 bound=1603 best=1610 pruned_bound=36435557 pruned_infeasible=23864139
[tsp-debug] progress: expanded=60400000 created=96914480 depth=333 bound=1579 best=1610 pruned_bound=36514152 pruned_infeasible=23885515
[tsp-debug] progress: expanded=60500000 created=97096078 depth=325 bound=1576 best=1610 pruned_bound=36595758 pruned_infeasible=23903917
[tsp-debug] progress: expanded=60600000 created=97276628 depth=346 bound=1605 best=1610 pruned_bound=36676290 pruned_infeasible=23923367
[tsp-debug] progress: expanded=60700000 created=97457924 depth=332 bound=1607 best=1610 pruned_bound=36757602 pruned_infeasible=23942071
[tsp-debug] progress: expanded=60800000 created=97637524 depth=357 bound=1605 best=1610 pruned_bound=36837176 pruned_infeasible=23962471
[tsp-debug] progress: expanded=60900000 created=97810286 depth=326 bound=1561 best=1610 pruned_bound=36909965 pruned_infeasible=23989709
[tsp-debug] progress: expanded=61000000 created=97981311 depth=332 bound=1609 best=1610 pruned_bound=36980985 pruned_infeasible=24018684
[tsp-debug] progress: expanded=61100000 created=98161535 depth=328 bound=1567 best=1610 pruned_bound=37061213 pruned_infeasible=24038460
[tsp-debug] progress: expanded=61200000 created=98342558 depth=343 bound=1608 best=1610 pruned_bound=37142221 pruned_infeasible=24057437
[tsp-debug] progress: expanded=61300000 created=98529244 depth=337 bound=1607 best=1610 pruned_bound=37228912 pruned_infeasible=24070751
[tsp-debug] progress: expanded=61400000 created=98703636 depth=340 bound=1609 best=1610 pruned_bound=37303303 pruned_infeasible=24096359
[tsp-debug] progress: expanded=61500000 created=98875888 depth=326 bound=1602 best=1610 pruned_bound=37375569 pruned_infeasible=24124107
[tsp-debug] progress: expanded=61600000 created=99060064 depth=344 bound=1583 best=1610 pruned_bound=37459727 pruned_infeasible=24139931
[tsp-debug] progress: expanded=61700000 created=99242269 depth=345 bound=1607 best=1610 pruned_bound=37541931 pruned_infeasible=24157726
[tsp-debug] progress: expanded=61800000 created=99426235 depth=329 bound=1575 best=1610 pruned_bound=37625911 pruned_infeasible=24173760
[tsp-debug] progress: expanded=61900000 created=99610885 depth=325 bound=1602 best=1610 pruned_bound=37710566 pruned_infeasible=24189110
[tsp-debug] progress: expanded=62000000 created=99782611 depth=346 bound=1606 best=1610 pruned_bound=37782271 pruned_infeasible=24217384
[tsp-debug] progress: expanded=62100000 created=99961286 depth=344 bound=1565 best=1610 pruned_bound=37860948 pruned_infeasible=24238709
[tsp-debug] progress: expanded=62200000 created=100145985 depth=328 bound=1585 best=1610 pruned_bound=37945661 pruned_infeasible=24254010
[tsp-debug] progress: expanded=62300000 created=100331561 depth=329 bound=1594 best=1610 pruned_bound=38031238 pruned_infeasible=24268434
[tsp-debug] progress: expanded=62400000 created=100509619 depth=335 bound=1605 best=1610 pruned_bound=38109290 pruned_infeasible=24290376
[tsp-debug] progress: expanded=62500000 created=100690683 depth=340 bound=1604 best=1610 pruned_bound=38190349 pruned_infeasible=24309312
[tsp-debug] progress: expanded=62600000 created=100873007 depth=338 bound=1608 best=1610 pruned_bound=38272674 pruned_infeasible=24326988
[tsp-debug] progress: expanded=62700000 created=101058385 depth=363 bound=1596 best=1610 pruned_bound=38358032 pruned_infeasible=24341610
[tsp-debug] progress: expanded=62800000 created=101240034 depth=344 bound=1602 best=1610 pruned_bound=38439696 pruned_infeasible=24359961
[tsp-debug] progress: expanded=62900000 created=101425595 depth=331 bound=1597 best=1610 pruned_bound=38525272 pruned_infeasible=24374400
[tsp-debug] progress: expanded=63000000 created=101612005 depth=255 bound=1571 best=1610 pruned_bound=38611753 pruned_infeasible=24387990
[tsp-debug] progress: expanded=63100000 created=101798231 depth=339 bound=1601 best=1610 pruned_bound=38697897 pruned_infeasible=24401764
[tsp-debug] progress: expanded=63200000 created=101977243 depth=335 bound=1602 best=1610 pruned_bound=38776915 pruned_infeasible=24422752
[tsp-debug] progress: expanded=63300000 created=102160124 depth=266 bound=1566 best=1610 pruned_bound=38859861 pruned_infeasible=24439871
[tsp-debug] progress: expanded=63400000 created=102345113 depth=345 bound=1576 best=1610 pruned_bound=38944774 pruned_infeasible=24454882
[tsp-debug] progress: expanded=63500000 created=102530932 depth=340 bound=1600 best=1610 pruned_bound=39030602 pruned_infeasible=24469063
[tsp-debug] progress: expanded=63600000 created=102720122 depth=325 bound=1607 best=1610 pruned_bound=39119803 pruned_infeasible=24479873
[tsp-debug] progress: expanded=63700000 created=102908330 depth=336 bound=1597 best=1610 pruned_bound=39207999 pruned_infeasible=24491665
[tsp-debug] progress: expanded=63800000 created=103099425 depth=308 bound=1560 best=1610 pruned_bound=39299120 pruned_infeasible=24500570
[tsp-debug] progress: expanded=63900000 created=103291082 depth=249 bound=1605 best=1610 pruned_bound=39390836 pruned_infeasible=24508913
[tsp-debug] progress: expanded=64000000 created=103481542 depth=260 bound=1609 best=1610 pruned_bound=39481285 pruned_infeasible=24518453
[tsp-debug] progress: expanded=64100000 created=103672643 depth=304 bound=1600 best=1610 pruned_bound=39572342 pruned_infeasible=24527352
[tsp-debug] progress: expanded=64200000 created=103865757 depth=330 bound=1605 best=1610 pruned_bound=39665432 pruned_infeasible=24534238
[tsp-debug] exact solve finished: feasible=yes cost=1610 expanded=64228007 created=103920892 pruned_bound=39692885 pruned_infeasible=24535119
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
| Simple | 2020 | :white_check_mark: | 20.1m | 72136173 | 39512657 | 32623516 | 6889142 | - |

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
[tsp-debug] progress: expanded=100000 created=177914 depth=24 bound=1897 best=2020 pruned_bound=77896 pruned_infeasible=22085
[tsp-debug] progress: expanded=200000 created=354188 depth=32 bound=2001 best=2020 pruned_bound=154168 pruned_infeasible=45811
[tsp-debug] progress: expanded=300000 created=541594 depth=17 bound=1817 best=2020 pruned_bound=241581 pruned_infeasible=58405
[tsp-debug] progress: expanded=400000 created=729561 depth=27 bound=1998 best=2020 pruned_bound=329543 pruned_infeasible=70438
[tsp-debug] progress: expanded=500000 created=914731 depth=29 bound=1939 best=2020 pruned_bound=414710 pruned_infeasible=85268
[tsp-debug] progress: expanded=600000 created=1099414 depth=30 bound=1972 best=2020 pruned_bound=499391 pruned_infeasible=100585
[tsp-debug] progress: expanded=700000 created=1279377 depth=33 bound=2012 best=2020 pruned_bound=579356 pruned_infeasible=120622
[tsp-debug] progress: expanded=800000 created=1463303 depth=26 bound=1971 best=2020 pruned_bound=663287 pruned_infeasible=136696
[tsp-debug] progress: expanded=900000 created=1639182 depth=28 bound=1952 best=2020 pruned_bound=739164 pruned_infeasible=160817
[tsp-debug] progress: expanded=1000000 created=1826305 depth=28 bound=1946 best=2020 pruned_bound=826286 pruned_infeasible=173694
[tsp-debug] progress: expanded=1100000 created=2006367 depth=30 bound=1988 best=2020 pruned_bound=906351 pruned_infeasible=193632
[tsp-debug] progress: expanded=1200000 created=2188486 depth=29 bound=1953 best=2020 pruned_bound=988465 pruned_infeasible=211513
[tsp-debug] progress: expanded=1300000 created=2370362 depth=30 bound=1973 best=2020 pruned_bound=1070338 pruned_infeasible=229637
[tsp-debug] progress: expanded=1400000 created=2551603 depth=28 bound=1995 best=2020 pruned_bound=1151585 pruned_infeasible=248396
[tsp-debug] progress: expanded=1500000 created=2730719 depth=29 bound=2001 best=2020 pruned_bound=1230704 pruned_infeasible=269280
[tsp-debug] progress: expanded=1600000 created=2909590 depth=31 bound=1962 best=2020 pruned_bound=1309571 pruned_infeasible=290409
[tsp-debug] progress: expanded=1700000 created=3085554 depth=23 bound=1926 best=2020 pruned_bound=1385540 pruned_infeasible=314445
[tsp-debug] progress: expanded=1800000 created=3260841 depth=28 bound=1999 best=2020 pruned_bound=1460829 pruned_infeasible=339158
[tsp-debug] progress: expanded=1900000 created=3442285 depth=24 bound=1940 best=2020 pruned_bound=1542269 pruned_infeasible=357714
[tsp-debug] progress: expanded=2000000 created=3623258 depth=26 bound=2004 best=2020 pruned_bound=1623245 pruned_infeasible=376741
[tsp-debug] progress: expanded=2100000 created=3802157 depth=26 bound=1944 best=2020 pruned_bound=1702142 pruned_infeasible=397842
[tsp-debug] progress: expanded=2200000 created=3987373 depth=23 bound=1886 best=2020 pruned_bound=1787361 pruned_infeasible=412626
[tsp-debug] progress: expanded=2300000 created=4169563 depth=26 bound=1985 best=2020 pruned_bound=1869550 pruned_infeasible=430436
[tsp-debug] progress: expanded=2400000 created=4349262 depth=30 bound=1978 best=2020 pruned_bound=1949244 pruned_infeasible=450737
[tsp-debug] progress: expanded=2500000 created=4529509 depth=26 bound=2002 best=2020 pruned_bound=2029494 pruned_infeasible=470490
[tsp-debug] progress: expanded=2600000 created=4711297 depth=23 bound=1979 best=2020 pruned_bound=2111285 pruned_infeasible=488702
[tsp-debug] progress: expanded=2700000 created=4894255 depth=31 bound=1955 best=2020 pruned_bound=2194235 pruned_infeasible=505744
[tsp-debug] progress: expanded=2800000 created=5074557 depth=31 bound=1993 best=2020 pruned_bound=2274537 pruned_infeasible=525442
[tsp-debug] progress: expanded=2900000 created=5250027 depth=31 bound=1968 best=2020 pruned_bound=2350009 pruned_infeasible=549972
[tsp-debug] progress: expanded=3000000 created=5429179 depth=26 bound=1926 best=2020 pruned_bound=2429160 pruned_infeasible=570820
[tsp-debug] progress: expanded=3100000 created=5615219 depth=29 bound=2013 best=2020 pruned_bound=2515197 pruned_infeasible=584780
[tsp-debug] progress: expanded=3200000 created=5803692 depth=32 bound=1989 best=2020 pruned_bound=2603668 pruned_infeasible=596307
[tsp-debug] progress: expanded=3300000 created=5984223 depth=30 bound=1976 best=2020 pruned_bound=2684205 pruned_infeasible=615776
[tsp-debug] progress: expanded=3400000 created=6160981 depth=30 bound=1975 best=2020 pruned_bound=2760962 pruned_infeasible=639018
[tsp-debug] progress: expanded=3500000 created=6340541 depth=27 bound=1905 best=2020 pruned_bound=2840526 pruned_infeasible=659458
[tsp-debug] progress: expanded=3600000 created=6523241 depth=28 bound=2014 best=2020 pruned_bound=2923222 pruned_infeasible=676758
[tsp-debug] progress: expanded=3700000 created=6706451 depth=30 bound=2009 best=2020 pruned_bound=3006432 pruned_infeasible=693548
[tsp-debug] progress: expanded=3800000 created=6889747 depth=31 bound=2005 best=2020 pruned_bound=3089728 pruned_infeasible=710252
[tsp-debug] progress: expanded=3900000 created=7070175 depth=27 bound=2013 best=2020 pruned_bound=3170159 pruned_infeasible=729824
[tsp-debug] progress: expanded=4000000 created=7254178 depth=28 bound=1969 best=2020 pruned_bound=3254160 pruned_infeasible=745821
[tsp-debug] progress: expanded=4100000 created=7441036 depth=27 bound=2014 best=2020 pruned_bound=3341021 pruned_infeasible=758963
[tsp-debug] progress: expanded=4200000 created=7619293 depth=27 bound=1999 best=2020 pruned_bound=3419279 pruned_infeasible=780706
[tsp-debug] progress: expanded=4300000 created=7803734 depth=30 bound=1962 best=2020 pruned_bound=3503717 pruned_infeasible=796265
[tsp-debug] progress: expanded=4400000 created=7983772 depth=30 bound=1974 best=2020 pruned_bound=3583754 pruned_infeasible=816227
[tsp-debug] progress: expanded=4500000 created=8167098 depth=33 bound=2012 best=2020 pruned_bound=3667077 pruned_infeasible=832901
[tsp-debug] progress: expanded=4600000 created=8346587 depth=27 bound=1927 best=2020 pruned_bound=3746571 pruned_infeasible=853412
[tsp-debug] progress: expanded=4700000 created=8529114 depth=25 bound=1984 best=2020 pruned_bound=3829098 pruned_infeasible=870885
[tsp-debug] progress: expanded=4800000 created=8711486 depth=29 bound=1971 best=2020 pruned_bound=3911469 pruned_infeasible=888513
[tsp-debug] progress: expanded=4900000 created=8889104 depth=30 bound=2008 best=2020 pruned_bound=3989085 pruned_infeasible=910895
[tsp-debug] progress: expanded=5000000 created=9073114 depth=26 bound=1926 best=2020 pruned_bound=4073097 pruned_infeasible=926885
[tsp-debug] progress: expanded=5100000 created=9257400 depth=18 bound=1864 best=2020 pruned_bound=4157390 pruned_infeasible=942599
[tsp-debug] progress: expanded=5200000 created=9443358 depth=30 bound=1979 best=2020 pruned_bound=4243340 pruned_infeasible=956641
[tsp-debug] progress: expanded=5300000 created=9621951 depth=27 bound=2007 best=2020 pruned_bound=4321938 pruned_infeasible=978048
[tsp-debug] progress: expanded=5400000 created=9802700 depth=29 bound=1981 best=2020 pruned_bound=4402688 pruned_infeasible=997299
[tsp-debug] progress: expanded=5500000 created=9986374 depth=25 bound=1980 best=2020 pruned_bound=4486359 pruned_infeasible=1013625
[tsp-debug] progress: expanded=5600000 created=10161652 depth=23 bound=1975 best=2020 pruned_bound=4561640 pruned_infeasible=1038347
[tsp-debug] progress: expanded=5700000 created=10339229 depth=24 bound=1984 best=2020 pruned_bound=4639219 pruned_infeasible=1060770
[tsp-debug] progress: expanded=5800000 created=10522470 depth=27 bound=2016 best=2020 pruned_bound=4722455 pruned_infeasible=1077529
[tsp-debug] progress: expanded=5900000 created=10700571 depth=27 bound=1979 best=2020 pruned_bound=4800555 pruned_infeasible=1099428
[tsp-debug] progress: expanded=6000000 created=10882757 depth=28 bound=1992 best=2020 pruned_bound=4882740 pruned_infeasible=1117242
[tsp-debug] progress: expanded=6100000 created=11062278 depth=29 bound=2005 best=2020 pruned_bound=4962264 pruned_infeasible=1137721
[tsp-debug] progress: expanded=6200000 created=11242069 depth=27 bound=2014 best=2020 pruned_bound=5042053 pruned_infeasible=1157930
[tsp-debug] progress: expanded=6300000 created=11424272 depth=29 bound=1964 best=2020 pruned_bound=5124262 pruned_infeasible=1175727
[tsp-debug] progress: expanded=6400000 created=11604456 depth=27 bound=2002 best=2020 pruned_bound=5204439 pruned_infeasible=1195543
[tsp-debug] progress: expanded=6500000 created=11788081 depth=26 bound=2007 best=2020 pruned_bound=5288063 pruned_infeasible=1211918
[tsp-debug] progress: expanded=6600000 created=11973501 depth=27 bound=2006 best=2020 pruned_bound=5373486 pruned_infeasible=1226498
[tsp-debug] progress: expanded=6700000 created=12156636 depth=32 bound=2003 best=2020 pruned_bound=5456618 pruned_infeasible=1243363
[tsp-debug] progress: expanded=6800000 created=12337030 depth=28 bound=1999 best=2020 pruned_bound=5537014 pruned_infeasible=1262969
[tsp-debug] progress: expanded=6900000 created=12519876 depth=25 bound=2004 best=2020 pruned_bound=5619860 pruned_infeasible=1280123
[tsp-debug] progress: expanded=7000000 created=12698886 depth=31 bound=1999 best=2020 pruned_bound=5698866 pruned_infeasible=1301113
[tsp-debug] progress: expanded=7100000 created=12876483 depth=27 bound=1979 best=2020 pruned_bound=5776470 pruned_infeasible=1323516
[tsp-debug] progress: expanded=7200000 created=13058230 depth=28 bound=2007 best=2020 pruned_bound=5858213 pruned_infeasible=1341769
[tsp-debug] progress: expanded=7300000 created=13244410 depth=25 bound=1969 best=2020 pruned_bound=5944392 pruned_infeasible=1355589
[tsp-debug] progress: expanded=7400000 created=13431954 depth=27 bound=2015 best=2020 pruned_bound=6031938 pruned_infeasible=1368045
[tsp-debug] progress: expanded=7500000 created=13615643 depth=27 bound=2011 best=2020 pruned_bound=6115630 pruned_infeasible=1384356
[tsp-debug] progress: expanded=7600000 created=13800554 depth=24 bound=1982 best=2020 pruned_bound=6200540 pruned_infeasible=1399445
[tsp-debug] progress: expanded=7700000 created=13979858 depth=24 bound=1934 best=2020 pruned_bound=6279842 pruned_infeasible=1420141
[tsp-debug] progress: expanded=7800000 created=14160534 depth=23 bound=2001 best=2020 pruned_bound=6360522 pruned_infeasible=1439465
[tsp-debug] progress: expanded=7900000 created=14339595 depth=29 bound=2012 best=2020 pruned_bound=6439579 pruned_infeasible=1460404
[tsp-debug] progress: expanded=8000000 created=14519186 depth=25 bound=1997 best=2020 pruned_bound=6519171 pruned_infeasible=1480813
[tsp-debug] progress: expanded=8100000 created=14700010 depth=17 bound=1964 best=2020 pruned_bound=6600003 pruned_infeasible=1499989
[tsp-debug] progress: expanded=8200000 created=14880896 depth=28 bound=2010 best=2020 pruned_bound=6680881 pruned_infeasible=1519103
[tsp-debug] progress: expanded=8300000 created=15060602 depth=25 bound=1988 best=2020 pruned_bound=6760587 pruned_infeasible=1539397
[tsp-debug] progress: expanded=8400000 created=15249446 depth=25 bound=2010 best=2020 pruned_bound=6849433 pruned_infeasible=1550553
[tsp-debug] progress: expanded=8500000 created=15431945 depth=30 bound=1998 best=2020 pruned_bound=6931925 pruned_infeasible=1568054
[tsp-debug] progress: expanded=8600000 created=15617818 depth=28 bound=1949 best=2020 pruned_bound=7017802 pruned_infeasible=1582181
[tsp-debug] progress: expanded=8700000 created=15799972 depth=25 bound=1931 best=2020 pruned_bound=7099955 pruned_infeasible=1600027
[tsp-debug] progress: expanded=8800000 created=15989102 depth=32 bound=1981 best=2020 pruned_bound=7189082 pruned_infeasible=1610897
[tsp-debug] progress: expanded=8900000 created=16176556 depth=25 bound=1958 best=2020 pruned_bound=7276542 pruned_infeasible=1623443
[tsp-debug] progress: expanded=9000000 created=16353638 depth=26 bound=2009 best=2020 pruned_bound=7353620 pruned_infeasible=1646361
[tsp-debug] progress: expanded=9100000 created=16538238 depth=25 bound=1971 best=2020 pruned_bound=7438223 pruned_infeasible=1661761
[tsp-debug] progress: expanded=9200000 created=16719556 depth=25 bound=2001 best=2020 pruned_bound=7519542 pruned_infeasible=1680443
[tsp-debug] progress: expanded=9300000 created=16901195 depth=28 bound=1953 best=2020 pruned_bound=7601176 pruned_infeasible=1698804
[tsp-debug] progress: expanded=9400000 created=17081448 depth=28 bound=1996 best=2020 pruned_bound=7681431 pruned_infeasible=1718551
[tsp-debug] progress: expanded=9500000 created=17268066 depth=25 bound=1970 best=2020 pruned_bound=7768054 pruned_infeasible=1731933
[tsp-debug] progress: expanded=9600000 created=17452267 depth=25 bound=1951 best=2020 pruned_bound=7852254 pruned_infeasible=1747732
[tsp-debug] progress: expanded=9700000 created=17634132 depth=24 bound=1916 best=2020 pruned_bound=7934121 pruned_infeasible=1765867
[tsp-debug] progress: expanded=9800000 created=17819454 depth=22 bound=1909 best=2020 pruned_bound=8019446 pruned_infeasible=1780545
[tsp-debug] progress: expanded=9900000 created=18003009 depth=26 bound=1990 best=2020 pruned_bound=8102993 pruned_infeasible=1796990
[tsp-debug] progress: expanded=10000000 created=18188393 depth=27 bound=1996 best=2020 pruned_bound=8188377 pruned_infeasible=1811606
[tsp-debug] progress: expanded=10100000 created=18368192 depth=26 bound=1998 best=2020 pruned_bound=8268177 pruned_infeasible=1831807
[tsp-debug] progress: expanded=10200000 created=18551580 depth=26 bound=1998 best=2020 pruned_bound=8351564 pruned_infeasible=1848419
[tsp-debug] progress: expanded=10300000 created=18734653 depth=24 bound=2010 best=2020 pruned_bound=8434640 pruned_infeasible=1865346
[tsp-debug] progress: expanded=10400000 created=18916495 depth=28 bound=1966 best=2020 pruned_bound=8516475 pruned_infeasible=1883504
[tsp-debug] progress: expanded=10500000 created=19098366 depth=18 bound=1898 best=2020 pruned_bound=8598355 pruned_infeasible=1901633
[tsp-debug] progress: expanded=10600000 created=19281547 depth=24 bound=1999 best=2020 pruned_bound=8681534 pruned_infeasible=1918452
[tsp-debug] progress: expanded=10700000 created=19463026 depth=20 bound=1909 best=2020 pruned_bound=8763016 pruned_infeasible=1936973
[tsp-debug] progress: expanded=10800000 created=19645507 depth=31 bound=1985 best=2020 pruned_bound=8845486 pruned_infeasible=1954492
[tsp-debug] progress: expanded=10900000 created=19825721 depth=28 bound=1988 best=2020 pruned_bound=8925703 pruned_infeasible=1974278
[tsp-debug] progress: expanded=11000000 created=20009366 depth=34 bound=2015 best=2020 pruned_bound=9009341 pruned_infeasible=1990633
[tsp-debug] progress: expanded=11100000 created=20200254 depth=28 bound=1955 best=2020 pruned_bound=9100237 pruned_infeasible=1999745
[tsp-debug] progress: expanded=11200000 created=20380201 depth=28 bound=1972 best=2020 pruned_bound=9180185 pruned_infeasible=2019798
[tsp-debug] progress: expanded=11300000 created=20562817 depth=31 bound=2006 best=2020 pruned_bound=9262795 pruned_infeasible=2037182
[tsp-debug] progress: expanded=11400000 created=20745097 depth=26 bound=1984 best=2020 pruned_bound=9345080 pruned_infeasible=2054902
[tsp-debug] progress: expanded=11500000 created=20926595 depth=34 bound=2005 best=2020 pruned_bound=9426572 pruned_infeasible=2073404
[tsp-debug] progress: expanded=11600000 created=21107762 depth=26 bound=1909 best=2020 pruned_bound=9507745 pruned_infeasible=2092237
[tsp-debug] progress: expanded=11700000 created=21288997 depth=24 bound=2002 best=2020 pruned_bound=9588984 pruned_infeasible=2111002
[tsp-debug] progress: expanded=11800000 created=21467497 depth=31 bound=2016 best=2020 pruned_bound=9667477 pruned_infeasible=2132502
[tsp-debug] progress: expanded=11900000 created=21652713 depth=18 bound=1875 best=2020 pruned_bound=9752703 pruned_infeasible=2147286
[tsp-debug] progress: expanded=12000000 created=21836397 depth=25 bound=1968 best=2020 pruned_bound=9836384 pruned_infeasible=2163602
[tsp-debug] progress: expanded=12100000 created=22019051 depth=30 bound=2018 best=2020 pruned_bound=9919031 pruned_infeasible=2180948
[tsp-debug] progress: expanded=12200000 created=22198424 depth=25 bound=1998 best=2020 pruned_bound=9998410 pruned_infeasible=2201575
[tsp-debug] progress: expanded=12300000 created=22378586 depth=24 bound=1908 best=2020 pruned_bound=10078571 pruned_infeasible=2221413
[tsp-debug] progress: expanded=12400000 created=22558180 depth=27 bound=1957 best=2020 pruned_bound=10158168 pruned_infeasible=2241819
[tsp-debug] progress: expanded=12500000 created=22740763 depth=27 bound=1968 best=2020 pruned_bound=10240750 pruned_infeasible=2259236
[tsp-debug] progress: expanded=12600000 created=22921582 depth=31 bound=1992 best=2020 pruned_bound=10321568 pruned_infeasible=2278417
[tsp-debug] progress: expanded=12700000 created=23104013 depth=31 bound=1984 best=2020 pruned_bound=10403994 pruned_infeasible=2295986
[tsp-debug] progress: expanded=12800000 created=23285776 depth=29 bound=1951 best=2020 pruned_bound=10485759 pruned_infeasible=2314223
[tsp-debug] progress: expanded=12900000 created=23467904 depth=26 bound=1991 best=2020 pruned_bound=10567887 pruned_infeasible=2332095
[tsp-debug] progress: expanded=13000000 created=23650054 depth=26 bound=1994 best=2020 pruned_bound=10650040 pruned_infeasible=2349945
[tsp-debug] progress: expanded=13100000 created=23831065 depth=26 bound=2018 best=2020 pruned_bound=10731050 pruned_infeasible=2368934
[tsp-debug] progress: expanded=13200000 created=24018267 depth=29 bound=2016 best=2020 pruned_bound=10818248 pruned_infeasible=2381732
[tsp-debug] progress: expanded=13300000 created=24202667 depth=26 bound=1962 best=2020 pruned_bound=10902654 pruned_infeasible=2397332
[tsp-debug] progress: expanded=13400000 created=24384768 depth=26 bound=1991 best=2020 pruned_bound=10984753 pruned_infeasible=2415231
[tsp-debug] progress: expanded=13500000 created=24569155 depth=25 bound=1964 best=2020 pruned_bound=11069142 pruned_infeasible=2430844
[tsp-debug] progress: expanded=13600000 created=24748570 depth=29 bound=1980 best=2020 pruned_bound=11148554 pruned_infeasible=2451429
[tsp-debug] progress: expanded=13700000 created=24933152 depth=26 bound=2007 best=2020 pruned_bound=11233135 pruned_infeasible=2466847
[tsp-debug] progress: expanded=13800000 created=25117158 depth=28 bound=1974 best=2020 pruned_bound=11317142 pruned_infeasible=2482841
[tsp-debug] progress: expanded=13900000 created=25304337 depth=27 bound=2018 best=2020 pruned_bound=11404322 pruned_infeasible=2495662
[tsp-debug] progress: expanded=14000000 created=25487605 depth=26 bound=1996 best=2020 pruned_bound=11487592 pruned_infeasible=2512394
[tsp-debug] progress: expanded=14100000 created=25670153 depth=28 bound=1946 best=2020 pruned_bound=11570140 pruned_infeasible=2529846
[tsp-debug] progress: expanded=14200000 created=25853130 depth=26 bound=2007 best=2020 pruned_bound=11653115 pruned_infeasible=2546869
[tsp-debug] progress: expanded=14300000 created=26034636 depth=23 bound=1889 best=2020 pruned_bound=11734626 pruned_infeasible=2565363
[tsp-debug] progress: expanded=14400000 created=26216202 depth=30 bound=1931 best=2020 pruned_bound=11816180 pruned_infeasible=2583797
[tsp-debug] progress: expanded=14500000 created=26392587 depth=28 bound=1989 best=2020 pruned_bound=11892569 pruned_infeasible=2607412
[tsp-debug] progress: expanded=14600000 created=26572298 depth=25 bound=1888 best=2020 pruned_bound=11972281 pruned_infeasible=2627701
[tsp-debug] progress: expanded=14700000 created=26759218 depth=32 bound=1995 best=2020 pruned_bound=12059196 pruned_infeasible=2640781
[tsp-debug] progress: expanded=14800000 created=26946011 depth=23 bound=1927 best=2020 pruned_bound=12145996 pruned_infeasible=2653988
[tsp-debug] progress: expanded=14900000 created=27136793 depth=25 bound=1991 best=2020 pruned_bound=12236777 pruned_infeasible=2663206
[tsp-debug] progress: expanded=15000000 created=27323433 depth=34 bound=2016 best=2020 pruned_bound=12323408 pruned_infeasible=2676566
[tsp-debug] progress: expanded=15100000 created=27509685 depth=30 bound=1992 best=2020 pruned_bound=12409664 pruned_infeasible=2690314
[tsp-debug] progress: expanded=15200000 created=27690962 depth=32 bound=1936 best=2020 pruned_bound=12490941 pruned_infeasible=2709037
[tsp-debug] progress: expanded=15300000 created=27868959 depth=31 bound=1996 best=2020 pruned_bound=12568941 pruned_infeasible=2731040
[tsp-debug] progress: expanded=15400000 created=28051469 depth=31 bound=2001 best=2020 pruned_bound=12651448 pruned_infeasible=2748530
[tsp-debug] progress: expanded=15500000 created=28230691 depth=29 bound=2006 best=2020 pruned_bound=12730671 pruned_infeasible=2769308
[tsp-debug] progress: expanded=15600000 created=28413111 depth=23 bound=2017 best=2020 pruned_bound=12813097 pruned_infeasible=2786888
[tsp-debug] progress: expanded=15700000 created=28597332 depth=29 bound=1996 best=2020 pruned_bound=12897313 pruned_infeasible=2802667
[tsp-debug] progress: expanded=15800000 created=28779243 depth=31 bound=1986 best=2020 pruned_bound=12979225 pruned_infeasible=2820756
[tsp-debug] progress: expanded=15900000 created=28962300 depth=19 bound=1872 best=2020 pruned_bound=13062288 pruned_infeasible=2837699
[tsp-debug] progress: expanded=16000000 created=29145464 depth=26 bound=1990 best=2020 pruned_bound=13145451 pruned_infeasible=2854535
[tsp-debug] progress: expanded=16100000 created=29327490 depth=30 bound=2012 best=2020 pruned_bound=13227470 pruned_infeasible=2872509
[tsp-debug] progress: expanded=16200000 created=29502888 depth=25 bound=2007 best=2020 pruned_bound=13302873 pruned_infeasible=2897111
[tsp-debug] progress: expanded=16300000 created=29689197 depth=28 bound=1960 best=2020 pruned_bound=13389180 pruned_infeasible=2910802
[tsp-debug] progress: expanded=16400000 created=29869229 depth=28 bound=2006 best=2020 pruned_bound=13469214 pruned_infeasible=2930770
[tsp-debug] progress: expanded=16500000 created=30051829 depth=20 bound=1930 best=2020 pruned_bound=13551820 pruned_infeasible=2948170
[tsp-debug] progress: expanded=16600000 created=30230860 depth=18 bound=1849 best=2020 pruned_bound=13630849 pruned_infeasible=2969139
[tsp-debug] progress: expanded=16700000 created=30413258 depth=31 bound=1986 best=2020 pruned_bound=13713240 pruned_infeasible=2986741
[tsp-debug] progress: expanded=16800000 created=30593949 depth=20 bound=1891 best=2020 pruned_bound=13793940 pruned_infeasible=3006050
[tsp-debug] progress: expanded=16900000 created=30775716 depth=32 bound=1995 best=2020 pruned_bound=13875693 pruned_infeasible=3024283
[tsp-debug] progress: expanded=17000000 created=30957634 depth=29 bound=2002 best=2020 pruned_bound=13957619 pruned_infeasible=3042365
[tsp-debug] progress: expanded=17100000 created=31138947 depth=27 bound=1951 best=2020 pruned_bound=14038929 pruned_infeasible=3061052
[tsp-debug] progress: expanded=17200000 created=31324199 depth=30 bound=1997 best=2020 pruned_bound=14124179 pruned_infeasible=3075800
[tsp-debug] progress: expanded=17300000 created=31510440 depth=27 bound=1953 best=2020 pruned_bound=14210425 pruned_infeasible=3089559
[tsp-debug] progress: expanded=17400000 created=31694301 depth=27 bound=1911 best=2020 pruned_bound=14294282 pruned_infeasible=3105698
[tsp-debug] progress: expanded=17500000 created=31879980 depth=25 bound=2003 best=2020 pruned_bound=14379971 pruned_infeasible=3120019
[tsp-debug] progress: expanded=17600000 created=32062887 depth=32 bound=1940 best=2020 pruned_bound=14462866 pruned_infeasible=3137112
[tsp-debug] progress: expanded=17700000 created=32250202 depth=30 bound=2002 best=2020 pruned_bound=14550183 pruned_infeasible=3149797
[tsp-debug] progress: expanded=17800000 created=32440773 depth=22 bound=1988 best=2020 pruned_bound=14640761 pruned_infeasible=3159226
[tsp-debug] progress: expanded=17900000 created=32618934 depth=32 bound=1968 best=2020 pruned_bound=14718913 pruned_infeasible=3181065
[tsp-debug] progress: expanded=18000000 created=32802827 depth=32 bound=1994 best=2020 pruned_bound=14802807 pruned_infeasible=3197172
[tsp-debug] progress: expanded=18100000 created=32982866 depth=28 bound=1999 best=2020 pruned_bound=14882852 pruned_infeasible=3217133
[tsp-debug] progress: expanded=18200000 created=33164959 depth=29 bound=1982 best=2020 pruned_bound=14964943 pruned_infeasible=3235040
[tsp-debug] progress: expanded=18300000 created=33347160 depth=28 bound=1985 best=2020 pruned_bound=15047147 pruned_infeasible=3252839
[tsp-debug] progress: expanded=18400000 created=33528194 depth=30 bound=1985 best=2020 pruned_bound=15128177 pruned_infeasible=3271805
[tsp-debug] progress: expanded=18500000 created=33708853 depth=29 bound=2017 best=2020 pruned_bound=15208837 pruned_infeasible=3291146
[tsp-debug] progress: expanded=18600000 created=33892007 depth=29 bound=2014 best=2020 pruned_bound=15291991 pruned_infeasible=3307992
[tsp-debug] progress: expanded=18700000 created=34070704 depth=24 bound=1930 best=2020 pruned_bound=15370690 pruned_infeasible=3329295
[tsp-debug] progress: expanded=18800000 created=34252964 depth=34 bound=1999 best=2020 pruned_bound=15452945 pruned_infeasible=3347035
[tsp-debug] progress: expanded=18900000 created=34433920 depth=28 bound=2001 best=2020 pruned_bound=15533906 pruned_infeasible=3366079
[tsp-debug] progress: expanded=19000000 created=34617249 depth=26 bound=2019 best=2020 pruned_bound=15617236 pruned_infeasible=3382750
[tsp-debug] progress: expanded=19100000 created=34796992 depth=27 bound=1997 best=2020 pruned_bound=15696976 pruned_infeasible=3403007
[tsp-debug] progress: expanded=19200000 created=34980827 depth=26 bound=1994 best=2020 pruned_bound=15780816 pruned_infeasible=3419172
[tsp-debug] progress: expanded=19300000 created=35159764 depth=25 bound=1973 best=2020 pruned_bound=15859748 pruned_infeasible=3440235
[tsp-debug] progress: expanded=19400000 created=35342714 depth=26 bound=1991 best=2020 pruned_bound=15942701 pruned_infeasible=3457285
[tsp-debug] progress: expanded=19500000 created=35526857 depth=24 bound=2011 best=2020 pruned_bound=16026847 pruned_infeasible=3473142
[tsp-debug] progress: expanded=19600000 created=35707378 depth=30 bound=2002 best=2020 pruned_bound=16107363 pruned_infeasible=3492621
[tsp-debug] progress: expanded=19700000 created=35889577 depth=21 bound=1857 best=2020 pruned_bound=16189565 pruned_infeasible=3510422
[tsp-debug] progress: expanded=19800000 created=36071314 depth=23 bound=1967 best=2020 pruned_bound=16271301 pruned_infeasible=3528685
[tsp-debug] progress: expanded=19900000 created=36256058 depth=29 bound=1970 best=2020 pruned_bound=16356042 pruned_infeasible=3543941
[tsp-debug] progress: expanded=20000000 created=36440024 depth=23 bound=1919 best=2020 pruned_bound=16440012 pruned_infeasible=3559975
[tsp-debug] progress: expanded=20100000 created=36624644 depth=28 bound=1977 best=2020 pruned_bound=16524629 pruned_infeasible=3575355
[tsp-debug] progress: expanded=20200000 created=36810265 depth=31 bound=2010 best=2020 pruned_bound=16610242 pruned_infeasible=3589734
[tsp-debug] progress: expanded=20300000 created=36988943 depth=28 bound=1947 best=2020 pruned_bound=16688928 pruned_infeasible=3611056
[tsp-debug] progress: expanded=20400000 created=37170563 depth=29 bound=1999 best=2020 pruned_bound=16770545 pruned_infeasible=3629436
[tsp-debug] progress: expanded=20500000 created=37352812 depth=23 bound=1966 best=2020 pruned_bound=16852798 pruned_infeasible=3647187
[tsp-debug] progress: expanded=20600000 created=37533402 depth=29 bound=2007 best=2020 pruned_bound=16933386 pruned_infeasible=3666597
[tsp-debug] progress: expanded=20700000 created=37714122 depth=26 bound=1990 best=2020 pruned_bound=17014108 pruned_infeasible=3685877
[tsp-debug] progress: expanded=20800000 created=37891586 depth=31 bound=2006 best=2020 pruned_bound=17091565 pruned_infeasible=3708413
[tsp-debug] progress: expanded=20900000 created=38070100 depth=31 bound=2003 best=2020 pruned_bound=17170082 pruned_infeasible=3729899
[tsp-debug] progress: expanded=21000000 created=38249642 depth=25 bound=1993 best=2020 pruned_bound=17249627 pruned_infeasible=3750357
[tsp-debug] progress: expanded=21100000 created=38426365 depth=29 bound=2004 best=2020 pruned_bound=17326348 pruned_infeasible=3773634
[tsp-debug] progress: expanded=21200000 created=38602522 depth=31 bound=2019 best=2020 pruned_bound=17402507 pruned_infeasible=3797477
[tsp-debug] progress: expanded=21300000 created=38781616 depth=32 bound=1980 best=2020 pruned_bound=17481597 pruned_infeasible=3818383
[tsp-debug] progress: expanded=21400000 created=38964617 depth=22 bound=1968 best=2020 pruned_bound=17564603 pruned_infeasible=3835382
[tsp-debug] progress: expanded=21500000 created=39144150 depth=30 bound=2012 best=2020 pruned_bound=17644129 pruned_infeasible=3855849
[tsp-debug] progress: expanded=21600000 created=39323071 depth=27 bound=1965 best=2020 pruned_bound=17723059 pruned_infeasible=3876928
[tsp-debug] progress: expanded=21700000 created=39503732 depth=30 bound=2011 best=2020 pruned_bound=17803713 pruned_infeasible=3896267
[tsp-debug] progress: expanded=21800000 created=39681419 depth=26 bound=1990 best=2020 pruned_bound=17881407 pruned_infeasible=3918580
[tsp-debug] progress: expanded=21900000 created=39857434 depth=27 bound=2012 best=2020 pruned_bound=17957420 pruned_infeasible=3942565
[tsp-debug] progress: expanded=22000000 created=40037502 depth=29 bound=1998 best=2020 pruned_bound=18037486 pruned_infeasible=3962497
[tsp-debug] progress: expanded=22100000 created=40219564 depth=26 bound=1961 best=2020 pruned_bound=18119550 pruned_infeasible=3980435
[tsp-debug] progress: expanded=22200000 created=40400083 depth=32 bound=2004 best=2020 pruned_bound=18200067 pruned_infeasible=3999916
[tsp-debug] progress: expanded=22300000 created=40578348 depth=27 bound=1989 best=2020 pruned_bound=18278337 pruned_infeasible=4021651
[tsp-debug] progress: expanded=22400000 created=40756260 depth=24 bound=1909 best=2020 pruned_bound=18356247 pruned_infeasible=4043739
[tsp-debug] progress: expanded=22500000 created=40933753 depth=27 bound=1989 best=2020 pruned_bound=18433742 pruned_infeasible=4066246
[tsp-debug] progress: expanded=22600000 created=41111508 depth=26 bound=1964 best=2020 pruned_bound=18511497 pruned_infeasible=4088491
[tsp-debug] progress: expanded=22700000 created=41290402 depth=27 bound=1986 best=2020 pruned_bound=18590385 pruned_infeasible=4109597
[tsp-debug] progress: expanded=22800000 created=41476931 depth=25 bound=1948 best=2020 pruned_bound=18676914 pruned_infeasible=4123068
[tsp-debug] progress: expanded=22900000 created=41665021 depth=32 bound=1988 best=2020 pruned_bound=18765001 pruned_infeasible=4134978
[tsp-debug] progress: expanded=23000000 created=41849809 depth=28 bound=1996 best=2020 pruned_bound=18849795 pruned_infeasible=4150190
[tsp-debug] progress: expanded=23100000 created=42035302 depth=30 bound=1995 best=2020 pruned_bound=18935284 pruned_infeasible=4164697
[tsp-debug] progress: expanded=23200000 created=42222179 depth=29 bound=1989 best=2020 pruned_bound=19022160 pruned_infeasible=4177820
[tsp-debug] progress: expanded=23300000 created=42408931 depth=27 bound=1977 best=2020 pruned_bound=19108915 pruned_infeasible=4191068
[tsp-debug] progress: expanded=23400000 created=42595655 depth=31 bound=2001 best=2020 pruned_bound=19195636 pruned_infeasible=4204344
[tsp-debug] progress: expanded=23500000 created=42779224 depth=29 bound=1972 best=2020 pruned_bound=19279206 pruned_infeasible=4220775
[tsp-debug] progress: expanded=23600000 created=42965122 depth=28 bound=1969 best=2020 pruned_bound=19365106 pruned_infeasible=4234877
[tsp-debug] progress: expanded=23700000 created=43152737 depth=29 bound=1964 best=2020 pruned_bound=19452719 pruned_infeasible=4247262
[tsp-debug] progress: expanded=23800000 created=43341403 depth=29 bound=2012 best=2020 pruned_bound=19541384 pruned_infeasible=4258596
[tsp-debug] progress: expanded=23900000 created=43530959 depth=30 bound=1990 best=2020 pruned_bound=19630942 pruned_infeasible=4269040
[tsp-debug] progress: expanded=24000000 created=43720582 depth=27 bound=2009 best=2020 pruned_bound=19720566 pruned_infeasible=4279417
[tsp-debug] progress: expanded=24100000 created=43910149 depth=26 bound=1984 best=2020 pruned_bound=19810137 pruned_infeasible=4289850
[tsp-debug] progress: expanded=24200000 created=44098391 depth=30 bound=1991 best=2020 pruned_bound=19898377 pruned_infeasible=4301608
[tsp-debug] progress: expanded=24300000 created=44286976 depth=23 bound=2011 best=2020 pruned_bound=19986966 pruned_infeasible=4313023
[tsp-debug] progress: expanded=24400000 created=44475640 depth=27 bound=2006 best=2020 pruned_bound=20075629 pruned_infeasible=4324359
[tsp-debug] progress: expanded=24500000 created=44662048 depth=26 bound=1995 best=2020 pruned_bound=20162033 pruned_infeasible=4337951
[tsp-debug] progress: expanded=24600000 created=44849918 depth=25 bound=1982 best=2020 pruned_bound=20249904 pruned_infeasible=4350081
[tsp-debug] progress: expanded=24700000 created=45035595 depth=27 bound=2003 best=2020 pruned_bound=20335579 pruned_infeasible=4364404
[tsp-debug] progress: expanded=24800000 created=45223088 depth=22 bound=1943 best=2020 pruned_bound=20423076 pruned_infeasible=4376911
[tsp-debug] progress: expanded=24900000 created=45412221 depth=26 bound=1995 best=2020 pruned_bound=20512203 pruned_infeasible=4387778
[tsp-debug] progress: expanded=25000000 created=45600651 depth=31 bound=1963 best=2020 pruned_bound=20600637 pruned_infeasible=4399348
[tsp-debug] progress: expanded=25100000 created=45788216 depth=28 bound=1999 best=2020 pruned_bound=20688201 pruned_infeasible=4411783
[tsp-debug] progress: expanded=25200000 created=45971896 depth=27 bound=2014 best=2020 pruned_bound=20771881 pruned_infeasible=4428103
[tsp-debug] progress: expanded=25300000 created=46164239 depth=24 bound=1984 best=2020 pruned_bound=20864232 pruned_infeasible=4435760
[tsp-debug] progress: expanded=25400000 created=46351578 depth=19 bound=1914 best=2020 pruned_bound=20951571 pruned_infeasible=4448421
[tsp-debug] progress: expanded=25500000 created=46539001 depth=26 bound=2019 best=2020 pruned_bound=21038989 pruned_infeasible=4460998
[tsp-debug] progress: expanded=25600000 created=46725269 depth=26 bound=1984 best=2020 pruned_bound=21125254 pruned_infeasible=4474730
[tsp-debug] progress: expanded=25700000 created=46910922 depth=25 bound=1968 best=2020 pruned_bound=21210905 pruned_infeasible=4489077
[tsp-debug] progress: expanded=25800000 created=47095390 depth=29 bound=2006 best=2020 pruned_bound=21295371 pruned_infeasible=4504609
[tsp-debug] progress: expanded=25900000 created=47280229 depth=28 bound=1979 best=2020 pruned_bound=21380209 pruned_infeasible=4519770
[tsp-debug] progress: expanded=26000000 created=47462876 depth=25 bound=1995 best=2020 pruned_bound=21462867 pruned_infeasible=4537123
[tsp-debug] progress: expanded=26100000 created=47644159 depth=27 bound=1958 best=2020 pruned_bound=21544144 pruned_infeasible=4555840
[tsp-debug] progress: expanded=26200000 created=47831078 depth=28 bound=1978 best=2020 pruned_bound=21631061 pruned_infeasible=4568921
[tsp-debug] progress: expanded=26300000 created=48014218 depth=25 bound=1973 best=2020 pruned_bound=21714203 pruned_infeasible=4585781
[tsp-debug] progress: expanded=26400000 created=48196510 depth=24 bound=1998 best=2020 pruned_bound=21796496 pruned_infeasible=4603489
[tsp-debug] progress: expanded=26500000 created=48380221 depth=22 bound=1968 best=2020 pruned_bound=21880209 pruned_infeasible=4619778
[tsp-debug] progress: expanded=26600000 created=48564167 depth=26 bound=2011 best=2020 pruned_bound=21964153 pruned_infeasible=4635832
[tsp-debug] progress: expanded=26700000 created=48749557 depth=34 bound=1983 best=2020 pruned_bound=22049535 pruned_infeasible=4650442
[tsp-debug] progress: expanded=26800000 created=48930112 depth=28 bound=1958 best=2020 pruned_bound=22130097 pruned_infeasible=4669887
[tsp-debug] progress: expanded=26900000 created=49115440 depth=25 bound=2014 best=2020 pruned_bound=22215427 pruned_infeasible=4684559
[tsp-debug] progress: expanded=27000000 created=49298704 depth=29 bound=2008 best=2020 pruned_bound=22298688 pruned_infeasible=4701295
[tsp-debug] progress: expanded=27100000 created=49478855 depth=27 bound=1949 best=2020 pruned_bound=22378838 pruned_infeasible=4721144
[tsp-debug] progress: expanded=27200000 created=49659438 depth=26 bound=1983 best=2020 pruned_bound=22459422 pruned_infeasible=4740561
[tsp-debug] progress: expanded=27300000 created=49842027 depth=29 bound=1983 best=2020 pruned_bound=22542010 pruned_infeasible=4757972
[tsp-debug] progress: expanded=27400000 created=50020558 depth=33 bound=2005 best=2020 pruned_bound=22620538 pruned_infeasible=4779441
[tsp-debug] progress: expanded=27500000 created=50204054 depth=21 bound=1944 best=2020 pruned_bound=22704041 pruned_infeasible=4795945
[tsp-debug] progress: expanded=27600000 created=50383551 depth=27 bound=2019 best=2020 pruned_bound=22783540 pruned_infeasible=4816448
[tsp-debug] progress: expanded=27700000 created=50568226 depth=24 bound=2000 best=2020 pruned_bound=22868215 pruned_infeasible=4831773
[tsp-debug] progress: expanded=27800000 created=50751728 depth=29 bound=1999 best=2020 pruned_bound=22951711 pruned_infeasible=4848271
[tsp-debug] progress: expanded=27900000 created=50933740 depth=25 bound=1958 best=2020 pruned_bound=23033725 pruned_infeasible=4866259
[tsp-debug] progress: expanded=28000000 created=51116836 depth=25 bound=1977 best=2020 pruned_bound=23116820 pruned_infeasible=4883163
[tsp-debug] progress: expanded=28100000 created=51297466 depth=24 bound=2016 best=2020 pruned_bound=23197454 pruned_infeasible=4902533
[tsp-debug] progress: expanded=28200000 created=51479634 depth=17 bound=1959 best=2020 pruned_bound=23279629 pruned_infeasible=4920365
[tsp-debug] progress: expanded=28300000 created=51663291 depth=23 bound=1969 best=2020 pruned_bound=23363278 pruned_infeasible=4936708
[tsp-debug] progress: expanded=28400000 created=51846245 depth=21 bound=1914 best=2020 pruned_bound=23446235 pruned_infeasible=4953754
[tsp-debug] progress: expanded=28500000 created=52028653 depth=23 bound=2009 best=2020 pruned_bound=23528639 pruned_infeasible=4971346
[tsp-debug] progress: expanded=28600000 created=52209913 depth=22 bound=1906 best=2020 pruned_bound=23609899 pruned_infeasible=4990086
[tsp-debug] progress: expanded=28700000 created=52389869 depth=30 bound=1989 best=2020 pruned_bound=23689848 pruned_infeasible=5010130
[tsp-debug] progress: expanded=28800000 created=52569371 depth=29 bound=1990 best=2020 pruned_bound=23769357 pruned_infeasible=5030628
[tsp-debug] progress: expanded=28900000 created=52751379 depth=26 bound=1928 best=2020 pruned_bound=23851361 pruned_infeasible=5048620
[tsp-debug] progress: expanded=29000000 created=52933873 depth=23 bound=2001 best=2020 pruned_bound=23933861 pruned_infeasible=5066126
[tsp-debug] progress: expanded=29100000 created=53114884 depth=27 bound=1993 best=2020 pruned_bound=24014868 pruned_infeasible=5085115
[tsp-debug] progress: expanded=29200000 created=53297432 depth=33 bound=2015 best=2020 pruned_bound=24097409 pruned_infeasible=5102567
[tsp-debug] progress: expanded=29300000 created=53480601 depth=24 bound=2000 best=2020 pruned_bound=24180585 pruned_infeasible=5119398
[tsp-debug] progress: expanded=29400000 created=53662224 depth=31 bound=1989 best=2020 pruned_bound=24262204 pruned_infeasible=5137775
[tsp-debug] progress: expanded=29500000 created=53843068 depth=28 bound=1998 best=2020 pruned_bound=24343052 pruned_infeasible=5156931
[tsp-debug] progress: expanded=29600000 created=54024411 depth=23 bound=1955 best=2020 pruned_bound=24424401 pruned_infeasible=5175588
[tsp-debug] progress: expanded=29700000 created=54205358 depth=31 bound=2018 best=2020 pruned_bound=24505337 pruned_infeasible=5194641
[tsp-debug] progress: expanded=29800000 created=54386890 depth=30 bound=1995 best=2020 pruned_bound=24586872 pruned_infeasible=5213109
[tsp-debug] progress: expanded=29900000 created=54568977 depth=22 bound=1928 best=2020 pruned_bound=24668965 pruned_infeasible=5231022
[tsp-debug] progress: expanded=30000000 created=54750463 depth=32 bound=1977 best=2020 pruned_bound=24750444 pruned_infeasible=5249536
[tsp-debug] progress: expanded=30100000 created=54930248 depth=21 bound=1903 best=2020 pruned_bound=24830236 pruned_infeasible=5269751
[tsp-debug] progress: expanded=30200000 created=55108833 depth=24 bound=2001 best=2020 pruned_bound=24908821 pruned_infeasible=5291166
[tsp-debug] progress: expanded=30300000 created=55287789 depth=24 bound=1986 best=2020 pruned_bound=24987779 pruned_infeasible=5312210
[tsp-debug] progress: expanded=30400000 created=55469255 depth=31 bound=1963 best=2020 pruned_bound=25069239 pruned_infeasible=5330744
[tsp-debug] progress: expanded=30500000 created=55649922 depth=24 bound=1952 best=2020 pruned_bound=25149906 pruned_infeasible=5350077
[tsp-debug] progress: expanded=30600000 created=55833667 depth=25 bound=1961 best=2020 pruned_bound=25233649 pruned_infeasible=5366332
[tsp-debug] progress: expanded=30700000 created=56015704 depth=20 bound=1956 best=2020 pruned_bound=25315691 pruned_infeasible=5384295
[tsp-debug] progress: expanded=30800000 created=56197902 depth=22 bound=1985 best=2020 pruned_bound=25397890 pruned_infeasible=5402097
[tsp-debug] progress: expanded=30900000 created=56378554 depth=24 bound=2018 best=2020 pruned_bound=25478542 pruned_infeasible=5421445
[tsp-debug] progress: expanded=31000000 created=56558468 depth=26 bound=1941 best=2020 pruned_bound=25558450 pruned_infeasible=5441531
[tsp-debug] progress: expanded=31100000 created=56747735 depth=22 bound=1993 best=2020 pruned_bound=25647723 pruned_infeasible=5452264
[tsp-debug] progress: expanded=31200000 created=56931042 depth=20 bound=1948 best=2020 pruned_bound=25731029 pruned_infeasible=5468957
[tsp-debug] progress: expanded=31300000 created=57112887 depth=22 bound=1969 best=2020 pruned_bound=25812874 pruned_infeasible=5487112
[tsp-debug] progress: expanded=31400000 created=57294450 depth=27 bound=1987 best=2020 pruned_bound=25894431 pruned_infeasible=5505549
[tsp-debug] progress: expanded=31500000 created=57479806 depth=27 bound=1989 best=2020 pruned_bound=25979791 pruned_infeasible=5520193
[tsp-debug] progress: expanded=31600000 created=57660781 depth=27 bound=2002 best=2020 pruned_bound=26060766 pruned_infeasible=5539218
[tsp-debug] progress: expanded=31700000 created=57842482 depth=26 bound=1994 best=2020 pruned_bound=26142468 pruned_infeasible=5557517
[tsp-debug] progress: expanded=31800000 created=58026890 depth=26 bound=2006 best=2020 pruned_bound=26226875 pruned_infeasible=5573109
[tsp-debug] progress: expanded=31900000 created=58209642 depth=19 bound=1986 best=2020 pruned_bound=26309633 pruned_infeasible=5590357
[tsp-debug] progress: expanded=32000000 created=58386240 depth=29 bound=2009 best=2020 pruned_bound=26386227 pruned_infeasible=5613759
[tsp-debug] progress: expanded=32100000 created=58572528 depth=21 bound=1917 best=2020 pruned_bound=26472516 pruned_infeasible=5627471
[tsp-debug] progress: expanded=32200000 created=58753340 depth=25 bound=1961 best=2020 pruned_bound=26553325 pruned_infeasible=5646659
[tsp-debug] progress: expanded=32300000 created=58936155 depth=21 bound=2011 best=2020 pruned_bound=26636146 pruned_infeasible=5663844
[tsp-debug] progress: expanded=32400000 created=59111716 depth=20 bound=1898 best=2020 pruned_bound=26711708 pruned_infeasible=5688283
[tsp-debug] progress: expanded=32500000 created=59296336 depth=30 bound=1992 best=2020 pruned_bound=26796318 pruned_infeasible=5703663
[tsp-debug] progress: expanded=32600000 created=59477376 depth=28 bound=2016 best=2020 pruned_bound=26877363 pruned_infeasible=5722623
[tsp-debug] progress: expanded=32700000 created=59658099 depth=24 bound=1998 best=2020 pruned_bound=26958085 pruned_infeasible=5741900
[tsp-debug] progress: expanded=32800000 created=59839105 depth=22 bound=1938 best=2020 pruned_bound=27039095 pruned_infeasible=5760894
[tsp-debug] progress: expanded=32900000 created=60018919 depth=29 bound=2018 best=2020 pruned_bound=27118906 pruned_infeasible=5781080
[tsp-debug] progress: expanded=33000000 created=60202936 depth=26 bound=1994 best=2020 pruned_bound=27202923 pruned_infeasible=5797063
[tsp-debug] progress: expanded=33100000 created=60383881 depth=23 bound=2015 best=2020 pruned_bound=27283870 pruned_infeasible=5816118
[tsp-debug] progress: expanded=33200000 created=60566097 depth=30 bound=1998 best=2020 pruned_bound=27366078 pruned_infeasible=5833902
[tsp-debug] progress: expanded=33300000 created=60745571 depth=24 bound=1989 best=2020 pruned_bound=27445557 pruned_infeasible=5854428
[tsp-debug] progress: expanded=33400000 created=60931206 depth=29 bound=1944 best=2020 pruned_bound=27531189 pruned_infeasible=5868793
[tsp-debug] progress: expanded=33500000 created=61117448 depth=27 bound=2015 best=2020 pruned_bound=27617431 pruned_infeasible=5882551
[tsp-debug] progress: expanded=33600000 created=61299390 depth=26 bound=1999 best=2020 pruned_bound=27699375 pruned_infeasible=5900609
[tsp-debug] progress: expanded=33700000 created=61484117 depth=24 bound=1997 best=2020 pruned_bound=27784102 pruned_infeasible=5915882
[tsp-debug] progress: expanded=33800000 created=61665409 depth=23 bound=1992 best=2020 pruned_bound=27865399 pruned_infeasible=5934590
[tsp-debug] progress: expanded=33900000 created=61847281 depth=30 bound=2004 best=2020 pruned_bound=27947266 pruned_infeasible=5952718
[tsp-debug] progress: expanded=34000000 created=62031723 depth=28 bound=1950 best=2020 pruned_bound=28031705 pruned_infeasible=5968276
[tsp-debug] progress: expanded=34100000 created=62220145 depth=27 bound=2002 best=2020 pruned_bound=28120129 pruned_infeasible=5979854
[tsp-debug] progress: expanded=34200000 created=62407916 depth=20 bound=1973 best=2020 pruned_bound=28207907 pruned_infeasible=5992083
[tsp-debug] progress: expanded=34300000 created=62591327 depth=27 bound=1994 best=2020 pruned_bound=28291315 pruned_infeasible=6008672
[tsp-debug] progress: expanded=34400000 created=62773456 depth=25 bound=1999 best=2020 pruned_bound=28373438 pruned_infeasible=6026543
[tsp-debug] progress: expanded=34500000 created=62954187 depth=25 bound=1994 best=2020 pruned_bound=28454173 pruned_infeasible=6045812
[tsp-debug] progress: expanded=34600000 created=63133589 depth=22 bound=1995 best=2020 pruned_bound=28533580 pruned_infeasible=6066410
[tsp-debug] progress: expanded=34700000 created=63314369 depth=29 bound=1977 best=2020 pruned_bound=28614350 pruned_infeasible=6085630
[tsp-debug] progress: expanded=34800000 created=63497601 depth=26 bound=1977 best=2020 pruned_bound=28697587 pruned_infeasible=6102398
[tsp-debug] progress: expanded=34900000 created=63681058 depth=26 bound=2006 best=2020 pruned_bound=28781045 pruned_infeasible=6118941
[tsp-debug] progress: expanded=35000000 created=63862558 depth=26 bound=1994 best=2020 pruned_bound=28862544 pruned_infeasible=6137441
[tsp-debug] progress: expanded=35100000 created=64044122 depth=18 bound=1950 best=2020 pruned_bound=28944114 pruned_infeasible=6155877
[tsp-debug] progress: expanded=35200000 created=64225866 depth=26 bound=1947 best=2020 pruned_bound=29025854 pruned_infeasible=6174133
[tsp-debug] progress: expanded=35300000 created=64405862 depth=25 bound=1911 best=2020 pruned_bound=29105845 pruned_infeasible=6194137
[tsp-debug] progress: expanded=35400000 created=64591702 depth=27 bound=1961 best=2020 pruned_bound=29191689 pruned_infeasible=6208297
[tsp-debug] progress: expanded=35500000 created=64777628 depth=26 bound=2016 best=2020 pruned_bound=29277616 pruned_infeasible=6222371
[tsp-debug] progress: expanded=35600000 created=64961561 depth=20 bound=1891 best=2020 pruned_bound=29361553 pruned_infeasible=6238438
[tsp-debug] progress: expanded=35700000 created=65144737 depth=25 bound=2014 best=2020 pruned_bound=29444725 pruned_infeasible=6255262
[tsp-debug] progress: expanded=35800000 created=65328645 depth=31 bound=1983 best=2020 pruned_bound=29528624 pruned_infeasible=6271354
[tsp-debug] progress: expanded=35900000 created=65510379 depth=30 bound=2018 best=2020 pruned_bound=29610359 pruned_infeasible=6289620
[tsp-debug] progress: expanded=36000000 created=65696678 depth=26 bound=2014 best=2020 pruned_bound=29696665 pruned_infeasible=6303321
[tsp-debug] progress: expanded=36100000 created=65876340 depth=24 bound=1962 best=2020 pruned_bound=29776330 pruned_infeasible=6323659
[tsp-debug] progress: expanded=36200000 created=66066125 depth=22 bound=2019 best=2020 pruned_bound=29866116 pruned_infeasible=6333874
[tsp-debug] progress: expanded=36300000 created=66250340 depth=27 bound=1995 best=2020 pruned_bound=29950327 pruned_infeasible=6349659
[tsp-debug] progress: expanded=36400000 created=66434279 depth=26 bound=2016 best=2020 pruned_bound=30034266 pruned_infeasible=6365720
[tsp-debug] progress: expanded=36500000 created=66616557 depth=28 bound=2015 best=2020 pruned_bound=30116543 pruned_infeasible=6383442
[tsp-debug] progress: expanded=36600000 created=66800386 depth=27 bound=2014 best=2020 pruned_bound=30200372 pruned_infeasible=6399613
[tsp-debug] progress: expanded=36700000 created=66986137 depth=29 bound=2013 best=2020 pruned_bound=30286119 pruned_infeasible=6413862
[tsp-debug] progress: expanded=36800000 created=67174347 depth=22 bound=1965 best=2020 pruned_bound=30374337 pruned_infeasible=6425652
[tsp-debug] progress: expanded=36900000 created=67361389 depth=27 bound=2002 best=2020 pruned_bound=30461374 pruned_infeasible=6438610
[tsp-debug] progress: expanded=37000000 created=67544275 depth=21 bound=1932 best=2020 pruned_bound=30544264 pruned_infeasible=6455724
[tsp-debug] progress: expanded=37100000 created=67730788 depth=28 bound=2003 best=2020 pruned_bound=30630773 pruned_infeasible=6469211
[tsp-debug] progress: expanded=37200000 created=67912768 depth=26 bound=1990 best=2020 pruned_bound=30712754 pruned_infeasible=6487231
[tsp-debug] progress: expanded=37300000 created=68096953 depth=29 bound=2008 best=2020 pruned_bound=30796944 pruned_infeasible=6503046
[tsp-debug] progress: expanded=37400000 created=68280785 depth=25 bound=2001 best=2020 pruned_bound=30880776 pruned_infeasible=6519214
[tsp-debug] progress: expanded=37500000 created=68464607 depth=31 bound=1998 best=2020 pruned_bound=30964590 pruned_infeasible=6535392
[tsp-debug] progress: expanded=37600000 created=68643049 depth=25 bound=1992 best=2020 pruned_bound=31043036 pruned_infeasible=6556950
[tsp-debug] progress: expanded=37700000 created=68828634 depth=25 bound=2005 best=2020 pruned_bound=31128623 pruned_infeasible=6571365
[tsp-debug] progress: expanded=37800000 created=69010772 depth=26 bound=1973 best=2020 pruned_bound=31210756 pruned_infeasible=6589227
[tsp-debug] progress: expanded=37900000 created=69192122 depth=26 bound=1998 best=2020 pruned_bound=31292111 pruned_infeasible=6607877
[tsp-debug] progress: expanded=38000000 created=69374285 depth=27 bound=1999 best=2020 pruned_bound=31374270 pruned_infeasible=6625714
[tsp-debug] progress: expanded=38100000 created=69556546 depth=21 bound=1988 best=2020 pruned_bound=31456535 pruned_infeasible=6643453
[tsp-debug] progress: expanded=38200000 created=69739420 depth=26 bound=1989 best=2020 pruned_bound=31539407 pruned_infeasible=6660579
[tsp-debug] progress: expanded=38300000 created=69921946 depth=26 bound=2019 best=2020 pruned_bound=31621935 pruned_infeasible=6678053
[tsp-debug] progress: expanded=38400000 created=70103887 depth=27 bound=2002 best=2020 pruned_bound=31703875 pruned_infeasible=6696112
[tsp-debug] progress: expanded=38500000 created=70285782 depth=25 bound=2015 best=2020 pruned_bound=31785766 pruned_infeasible=6714217
[tsp-debug] progress: expanded=38600000 created=70469611 depth=21 bound=2001 best=2020 pruned_bound=31869599 pruned_infeasible=6730388
[tsp-debug] progress: expanded=38700000 created=70652807 depth=27 bound=2011 best=2020 pruned_bound=31952792 pruned_infeasible=6747192
[tsp-debug] progress: expanded=38800000 created=70836619 depth=17 bound=2011 best=2020 pruned_bound=32036613 pruned_infeasible=6763380
[tsp-debug] progress: expanded=38900000 created=71017256 depth=21 bound=1957 best=2020 pruned_bound=32117245 pruned_infeasible=6782743
[tsp-debug] progress: expanded=39000000 created=71201328 depth=28 bound=2016 best=2020 pruned_bound=32201315 pruned_infeasible=6798671
[tsp-debug] progress: expanded=39100000 created=71383191 depth=24 bound=2007 best=2020 pruned_bound=32283178 pruned_infeasible=6816808
[tsp-debug] progress: expanded=39200000 created=71564968 depth=21 bound=1967 best=2020 pruned_bound=32364960 pruned_infeasible=6835031
[tsp-debug] progress: expanded=39300000 created=71748198 depth=24 bound=1986 best=2020 pruned_bound=32448189 pruned_infeasible=6851801
[tsp-debug] progress: expanded=39400000 created=71932063 depth=20 bound=1937 best=2020 pruned_bound=32532054 pruned_infeasible=6867936
[tsp-debug] progress: expanded=39500000 created=72113099 depth=19 bound=1939 best=2020 pruned_bound=32613091 pruned_infeasible=6886900
[tsp-debug] exact solve finished: feasible=yes cost=2020 expanded=39512657 created=72136173 pruned_bound=32623516 pruned_infeasible=6889142
```
</details>

---

## 30. `data/classic/national/wi29.tsp` (n=29)

**Concorde optimal cost:** 27603  
**Concorde time:** 37ms  

**Concorde (reference) tour:** `0 -> 4 -> 7 -> 3 -> 2 -> 6 -> 8 -> 12 -> 13 -> 15 -> 23 -> 26 -> 24 -> 19 -> 25 -> 27 -> 28 -> 22 -> 21 -> 20 -> 16 -> 17 -> 18 -> 14 -> 11 -> 10 -> 9 -> 5 -> 1 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 27603 | ref | 37ms | - | - | - | - | 1 |
| Smart | 27603 | :white_check_mark: | 1.2s | 89464 | 47581 | 41883 | 5699 | - |
| Simple | 27603 | :white_check_mark: | 1.1m | 4683527 | 2989038 | 1694489 | 1294550 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=27603 expanded=47581 created=89464 pruned_bound=41883 pruned_infeasible=5699
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=wi29 dimension=29 method=exact
[tsp-debug] exact solve started: vertices=29
[tsp-debug] initial incumbent: cost=27603
[tsp-debug] root: lower_bound=22727 search=recursive-dfs strategy=simple
[tsp-debug] progress: expanded=100000 created=147232 depth=26 bound=27444 best=27603 pruned_bound=47216 pruned_infeasible=52767
[tsp-debug] progress: expanded=200000 created=295736 depth=24 bound=27389 best=27603 pruned_bound=95720 pruned_infeasible=104263
[tsp-debug] progress: expanded=300000 created=442582 depth=22 bound=25718 best=27603 pruned_bound=142567 pruned_infeasible=157417
[tsp-debug] progress: expanded=400000 created=589788 depth=24 bound=27264 best=27603 pruned_bound=189771 pruned_infeasible=210211
[tsp-debug] progress: expanded=500000 created=738088 depth=26 bound=27203 best=27603 pruned_bound=238068 pruned_infeasible=261911
[tsp-debug] progress: expanded=600000 created=890459 depth=24 bound=27376 best=27603 pruned_bound=290446 pruned_infeasible=309540
[tsp-debug] progress: expanded=700000 created=1037747 depth=21 bound=25000 best=27603 pruned_bound=337735 pruned_infeasible=362252
[tsp-debug] progress: expanded=800000 created=1185145 depth=27 bound=27311 best=27603 pruned_bound=385129 pruned_infeasible=414854
[tsp-debug] progress: expanded=900000 created=1334177 depth=19 bound=24777 best=27603 pruned_bound=434167 pruned_infeasible=465822
[tsp-debug] progress: expanded=1000000 created=1486814 depth=21 bound=27014 best=27603 pruned_bound=486806 pruned_infeasible=513185
[tsp-debug] progress: expanded=1100000 created=1637268 depth=16 bound=25557 best=27603 pruned_bound=537259 pruned_infeasible=562731
[tsp-debug] progress: expanded=1200000 created=1789166 depth=21 bound=26481 best=27603 pruned_bound=589156 pruned_infeasible=610833
[tsp-debug] progress: expanded=1300000 created=1943510 depth=24 bound=26464 best=27603 pruned_bound=643497 pruned_infeasible=656489
[tsp-debug] progress: expanded=1400000 created=2097103 depth=28 bound=27001 best=27603 pruned_bound=697090 pruned_infeasible=702896
[tsp-debug] progress: expanded=1500000 created=2256900 depth=21 bound=26795 best=27603 pruned_bound=756892 pruned_infeasible=743099
[tsp-debug] progress: expanded=1600000 created=2419040 depth=19 bound=26956 best=27603 pruned_bound=819029 pruned_infeasible=780959
[tsp-debug] progress: expanded=1700000 created=2581160 depth=19 bound=27288 best=27603 pruned_bound=881150 pruned_infeasible=818839
[tsp-debug] progress: expanded=1800000 created=2745129 depth=16 bound=25773 best=27603 pruned_bound=945120 pruned_infeasible=854870
[tsp-debug] progress: expanded=1900000 created=2912634 depth=22 bound=27144 best=27603 pruned_bound=1012620 pruned_infeasible=887365
[tsp-debug] progress: expanded=2000000 created=3066738 depth=21 bound=25670 best=27603 pruned_bound=1066723 pruned_infeasible=933261
[tsp-debug] progress: expanded=2100000 created=3220330 depth=17 bound=26355 best=27603 pruned_bound=1120321 pruned_infeasible=979669
[tsp-debug] progress: expanded=2200000 created=3373187 depth=22 bound=25739 best=27603 pruned_bound=1173173 pruned_infeasible=1026812
[tsp-debug] progress: expanded=2300000 created=3531847 depth=22 bound=27193 best=27603 pruned_bound=1231835 pruned_infeasible=1068152
[tsp-debug] progress: expanded=2400000 created=3684116 depth=23 bound=27043 best=27603 pruned_bound=1284101 pruned_infeasible=1115883
[tsp-debug] progress: expanded=2500000 created=3841139 depth=22 bound=26168 best=27603 pruned_bound=1341127 pruned_infeasible=1158860
[tsp-debug] progress: expanded=2600000 created=4001564 depth=25 bound=26777 best=27603 pruned_bound=1401551 pruned_infeasible=1198435
[tsp-debug] progress: expanded=2700000 created=4164319 depth=20 bound=27057 best=27603 pruned_bound=1464310 pruned_infeasible=1235680
[tsp-debug] progress: expanded=2800000 created=4331997 depth=17 bound=26761 best=27603 pruned_bound=1531987 pruned_infeasible=1268002
[tsp-debug] progress: expanded=2900000 created=4506841 depth=20 bound=27208 best=27603 pruned_bound=1606831 pruned_infeasible=1293158
[tsp-debug] exact solve finished: feasible=yes cost=27603 expanded=2989038 created=4683527 pruned_bound=1694489 pruned_infeasible=1294550
```
</details>

---

## 31. `data/classic/national/dj38.tsp` (n=38)

**Concorde optimal cost:** 6656  
**Concorde time:** 34ms  

**Concorde (reference) tour:** `0 -> 9 -> 13 -> 20 -> 28 -> 29 -> 31 -> 34 -> 36 -> 37 -> 32 -> 33 -> 35 -> 30 -> 26 -> 27 -> 23 -> 21 -> 24 -> 25 -> 22 -> 19 -> 14 -> 12 -> 15 -> 16 -> 17 -> 18 -> 10 -> 11 -> 8 -> 7 -> 6 -> 5 -> 4 -> 2 -> 3 -> 1 -> 0`  

| Algorithm | Cost | Match Ref | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|-----------|------|---------------|----------------|---------------|----------------|---------|
| Concorde | 6656 | ref | 34ms | - | - | - | - | 1 |
| Smart | 6656 | :white_check_mark: | 2.0s | 83045 | 41522 | 41523 | 0 | - |
| Simple | 6656 | :white_check_mark: | 1.2s | 70447 | 35223 | 35224 | 0 | - |

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
[tsp-debug] exact solve finished: feasible=yes cost=6656 expanded=41522 created=83045 pruned_bound=41523 pruned_infeasible=0
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=dj38 dimension=38 method=exact
[tsp-debug] exact solve started: vertices=38
[tsp-debug] initial incumbent: cost=6656
[tsp-debug] root: lower_bound=6285 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=6656 expanded=35223 created=70447 pruned_bound=35224 pruned_infeasible=0
```
</details>

---
