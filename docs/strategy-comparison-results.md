# TSP Algorithm Comparison

## Algorithms Compared

| Algorithm | Description |
|---|---|
| **DP** | Held-Karp Dynamic Programming (exact, ground truth) |
| **Smart** | Branch & Bound with smart branching (1-tree degree + edge candidate) |
| **Simple** | Branch & Bound with simple branching (max-degree vertex) |
| **Concorde** | State-of-the-art Concorde TSP solver (exact, with QSopt LP) |

**Instances:** 19 from `examples/`  
**Timeout:** 3600s (1h) per method per instance  
**Ground truth:** Held-Karp DP  

---

## 1. `examples/five-city.txt` (n=5)

**DP optimal cost:** 26  
**DP time:** <1ms  

**DP optimal tour:** `0 -> 4 -> 2 -> 3 -> 1 -> 0`  

| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|
| DP | 26 | N/A | <1ms | - | - | - | - | - |
| Smart | 26 | :white_check_mark: | 4ms | 30 | 18 | 12 | 7 | - |
| Simple | 26 | :white_check_mark: | 4ms | 32 | 25 | 7 | 19 | - |
| Concorde | 26 | :white_check_mark: | 2ms | - | - | - | - | - |

**Tours found:**
- **DP (ground truth):** `0 -> 4 -> 2 -> 3 -> 1 -> 0` cost=26
- **Smart:** `0 -> 1 -> 3 -> 2 -> 4` cost=26 (=DP)
- **Simple:** `0 -> 1 -> 3 -> 2 -> 4` cost=26 (=DP)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=26
[tsp-debug] root: lower_bound=21 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=26 expanded=18 created=30 pruned_bound=12 pruned_infeasible=7
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=26
[tsp-debug] root: lower_bound=21 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=26 expanded=25 created=32 pruned_bound=7 pruned_infeasible=19
```
</details>

---

## 2. `examples/converted/five-node-euc.txt` (n=5)

**DP optimal cost:** 8  
**DP time:** <1ms  

**DP optimal tour:** `0 -> 4 -> 3 -> 2 -> 1 -> 0`  

| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|
| DP | 8 | N/A | <1ms | - | - | - | - | - |
| Smart | 8 | :white_check_mark: | 4ms | 16 | 10 | 6 | 5 | - |
| Simple | 8 | :white_check_mark: | 6ms | 56 | 43 | 13 | 31 | - |
| Concorde | 8 | :white_check_mark: | 3ms | - | - | - | - | - |

**Tours found:**
- **DP (ground truth):** `0 -> 4 -> 3 -> 2 -> 1 -> 0` cost=8
- **Smart:** `0 -> 4 -> 1 -> 2 -> 3` cost=8 (=DP)
- **Simple:** `0 -> 4 -> 1 -> 2 -> 3` cost=8 (=DP)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=8
[tsp-debug] root: lower_bound=6 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=8 expanded=10 created=16 pruned_bound=6 pruned_infeasible=5
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=8
[tsp-debug] root: lower_bound=6 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=8 expanded=43 created=56 pruned_bound=13 pruned_infeasible=31
```
</details>

---

## 3. `examples/converted/five-node-explicit.txt` (n=5)

**DP optimal cost:** 26  
**DP time:** <1ms  

**DP optimal tour:** `0 -> 4 -> 2 -> 3 -> 1 -> 0`  

| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|
| DP | 26 | N/A | <1ms | - | - | - | - | - |
| Smart | 26 | :white_check_mark: | 4ms | 30 | 18 | 12 | 7 | - |
| Simple | 26 | :white_check_mark: | 4ms | 32 | 25 | 7 | 19 | - |
| Concorde | 26 | :white_check_mark: | 2ms | - | - | - | - | - |

**Tours found:**
- **DP (ground truth):** `0 -> 4 -> 2 -> 3 -> 1 -> 0` cost=26
- **Smart:** `0 -> 1 -> 3 -> 2 -> 4` cost=26 (=DP)
- **Simple:** `0 -> 1 -> 3 -> 2 -> 4` cost=26 (=DP)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=26
[tsp-debug] root: lower_bound=21 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=26 expanded=18 created=30 pruned_bound=12 pruned_infeasible=7
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=26
[tsp-debug] root: lower_bound=21 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=26 expanded=25 created=32 pruned_bound=7 pruned_infeasible=19
```
</details>

---

## 4. `examples/random/complete/rnd-01-complete-n4.txt` (n=4)

**DP optimal cost:** 45  
**DP time:** <1ms  

**DP optimal tour:** `0 -> 3 -> 2 -> 1 -> 0`  

| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|
| DP | 45 | N/A | <1ms | - | - | - | - | - |
| Smart | 45 | :white_check_mark: | 3ms | 6 | 3 | 3 | 1 | - |
| Simple | 45 | :white_check_mark: | 3ms | 13 | 9 | 4 | 6 | - |
| Concorde | 45 | :white_check_mark: | 2ms | - | - | - | - | - |

**Tours found:**
- **DP (ground truth):** `0 -> 3 -> 2 -> 1 -> 0` cost=45
- **Smart:** `0 -> 3 -> 2 -> 1` cost=45 (=DP)
- **Simple:** `0 -> 3 -> 2 -> 1` cost=45 (=DP)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=4 method=exact
[tsp-debug] exact solve started: vertices=4
[tsp-debug] initial incumbent: cost=45
[tsp-debug] root: lower_bound=36 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=45 expanded=3 created=6 pruned_bound=3 pruned_infeasible=1
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=4 method=exact
[tsp-debug] exact solve started: vertices=4
[tsp-debug] initial incumbent: cost=45
[tsp-debug] root: lower_bound=36 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=45 expanded=9 created=13 pruned_bound=4 pruned_infeasible=6
```
</details>

---

## 5. `examples/random/complete/rnd-02-complete-n5.txt` (n=5)

**DP optimal cost:** 136  
**DP time:** <1ms  

**DP optimal tour:** `0 -> 2 -> 3 -> 4 -> 1 -> 0`  

| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|
| DP | 136 | N/A | <1ms | - | - | - | - | - |
| Smart | 136 | :white_check_mark: | 4ms | 11 | 6 | 5 | 2 | - |
| Simple | 136 | :white_check_mark: | 5ms | 35 | 26 | 9 | 18 | - |
| Concorde | 136 | :white_check_mark: | 3ms | - | - | - | - | - |

**Tours found:**
- **DP (ground truth):** `0 -> 2 -> 3 -> 4 -> 1 -> 0` cost=136
- **Smart:** `0 -> 2 -> 3 -> 4 -> 1` cost=136 (=DP)
- **Simple:** `0 -> 2 -> 3 -> 4 -> 1` cost=136 (=DP)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=136
[tsp-debug] root: lower_bound=127 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=136 expanded=6 created=11 pruned_bound=5 pruned_infeasible=2
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=136
[tsp-debug] root: lower_bound=127 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=136 expanded=26 created=35 pruned_bound=9 pruned_infeasible=18
```
</details>

---

## 6. `examples/random/complete/rnd-03-complete-n4.txt` (n=4)

**DP optimal cost:** 102  
**DP time:** <1ms  

**DP optimal tour:** `0 -> 2 -> 3 -> 1 -> 0`  

| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|
| DP | 102 | N/A | <1ms | - | - | - | - | - |
| Smart | 102 | :white_check_mark: | 2ms | 1 | 0 | 1 | 0 | - |
| Simple | 102 | :white_check_mark: | 4ms | 1 | 0 | 1 | 0 | - |
| Concorde | 102 | :white_check_mark: | 2ms | - | - | - | - | - |

**Tours found:**
- **DP (ground truth):** `0 -> 2 -> 3 -> 1 -> 0` cost=102
- **Smart:** `0 -> 1 -> 3 -> 2` cost=102 (=DP)
- **Simple:** `0 -> 1 -> 3 -> 2` cost=102 (=DP)

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

## 7. `examples/random/complete/rnd-04-complete-n5.txt` (n=5)

**DP optimal cost:** 88  
**DP time:** <1ms  

**DP optimal tour:** `0 -> 3 -> 2 -> 4 -> 1 -> 0`  

| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|
| DP | 88 | N/A | <1ms | - | - | - | - | - |
| Smart | 88 | :white_check_mark: | 2ms | 11 | 6 | 5 | 2 | - |
| Simple | 88 | :white_check_mark: | 2ms | 24 | 15 | 9 | 7 | - |
| Concorde | 88 | :white_check_mark: | 2ms | - | - | - | - | - |

**Tours found:**
- **DP (ground truth):** `0 -> 3 -> 2 -> 4 -> 1 -> 0` cost=88
- **Smart:** `0 -> 3 -> 2 -> 4 -> 1` cost=88 (=DP)
- **Simple:** `0 -> 3 -> 2 -> 4 -> 1` cost=88 (=DP)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=88
[tsp-debug] root: lower_bound=79 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=88 expanded=6 created=11 pruned_bound=5 pruned_infeasible=2
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=88
[tsp-debug] root: lower_bound=79 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=88 expanded=15 created=24 pruned_bound=9 pruned_infeasible=7
```
</details>

---

## 8. `examples/random/complete/rnd-05-complete-n7.txt` (n=7)

**DP optimal cost:** 74  
**DP time:** <1ms  

**DP optimal tour:** `0 -> 3 -> 6 -> 5 -> 1 -> 4 -> 2 -> 0`  

| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|
| DP | 74 | N/A | <1ms | - | - | - | - | - |
| Smart | 74 | :white_check_mark: | 2ms | 42 | 25 | 17 | 9 | - |
| Simple | 74 | :white_check_mark: | 7ms | 103 | 74 | 29 | 46 | - |
| Concorde | 74 | :white_check_mark: | 3ms | - | - | - | - | - |

**Tours found:**
- **DP (ground truth):** `0 -> 3 -> 6 -> 5 -> 1 -> 4 -> 2 -> 0` cost=74
- **Smart:** `0 -> 2 -> 4 -> 1 -> 5 -> 6 -> 3` cost=74 (=DP)
- **Simple:** `0 -> 2 -> 4 -> 1 -> 5 -> 6 -> 3` cost=74 (=DP)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=74
[tsp-debug] root: lower_bound=49 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=74 expanded=25 created=42 pruned_bound=17 pruned_infeasible=9
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=74
[tsp-debug] root: lower_bound=49 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=74 expanded=74 created=103 pruned_bound=29 pruned_infeasible=46
```
</details>

---

## 9. `examples/random/complete/rnd-06-complete-n5.txt` (n=5)

**DP optimal cost:** 51  
**DP time:** <1ms  

**DP optimal tour:** `0 -> 4 -> 1 -> 2 -> 3 -> 0`  

| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|
| DP | 51 | N/A | <1ms | - | - | - | - | - |
| Smart | 51 | :white_check_mark: | 2ms | 11 | 6 | 5 | 2 | - |
| Simple | 51 | :white_check_mark: | 4ms | 25 | 16 | 9 | 8 | - |
| Concorde | 51 | :white_check_mark: | 2ms | - | - | - | - | - |

**Tours found:**
- **DP (ground truth):** `0 -> 4 -> 1 -> 2 -> 3 -> 0` cost=51
- **Smart:** `0 -> 3 -> 2 -> 1 -> 4` cost=51 (=DP)
- **Simple:** `0 -> 3 -> 2 -> 1 -> 4` cost=51 (=DP)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=51
[tsp-debug] root: lower_bound=43 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=51 expanded=6 created=11 pruned_bound=5 pruned_infeasible=2
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=51
[tsp-debug] root: lower_bound=43 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=51 expanded=16 created=25 pruned_bound=9 pruned_infeasible=8
```
</details>

---

## 10. `examples/random/complete/rnd-07-complete-n8.txt` (n=8)

**DP optimal cost:** 64  
**DP time:** 2ms  

**DP optimal tour:** `0 -> 7 -> 4 -> 3 -> 2 -> 1 -> 6 -> 5 -> 0`  

| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|
| DP | 64 | N/A | 2ms | - | - | - | - | - |
| Smart | 64 | :white_check_mark: | 5ms | 44 | 27 | 17 | 11 | - |
| Simple | 64 | :white_check_mark: | 6ms | 310 | 208 | 102 | 107 | - |
| Concorde | 64 | :white_check_mark: | 2ms | - | - | - | - | - |

**Tours found:**
- **DP (ground truth):** `0 -> 7 -> 4 -> 3 -> 2 -> 1 -> 6 -> 5 -> 0` cost=64
- **Smart:** `2 -> 3 -> 4 -> 7 -> 0 -> 5 -> 6 -> 1` cost=64 (=DP)
- **Simple:** `2 -> 3 -> 4 -> 7 -> 0 -> 5 -> 6 -> 1` cost=64 (=DP)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=8 method=exact
[tsp-debug] exact solve started: vertices=8
[tsp-debug] initial incumbent: cost=64
[tsp-debug] root: lower_bound=46 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=64 expanded=27 created=44 pruned_bound=17 pruned_infeasible=11
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=8 method=exact
[tsp-debug] exact solve started: vertices=8
[tsp-debug] initial incumbent: cost=64
[tsp-debug] root: lower_bound=46 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=64 expanded=208 created=310 pruned_bound=102 pruned_infeasible=107
```
</details>

---

## 11. `examples/random/complete/rnd-08-complete-n7.txt` (n=7)

**DP optimal cost:** 73  
**DP time:** <1ms  

**DP optimal tour:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0`  

| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|
| DP | 73 | N/A | <1ms | - | - | - | - | - |
| Smart | 73 | :white_check_mark: | 5ms | 34 | 20 | 14 | 7 | - |
| Simple | 73 | :white_check_mark: | 9ms | 203 | 136 | 67 | 70 | - |
| Concorde | 73 | :white_check_mark: | 3ms | - | - | - | - | - |

**Tours found:**
- **DP (ground truth):** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5 -> 0` cost=73
- **Smart:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5` cost=73 (=DP)
- **Simple:** `0 -> 6 -> 3 -> 2 -> 1 -> 4 -> 5` cost=73 (=DP)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=73
[tsp-debug] root: lower_bound=50 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=73 expanded=20 created=34 pruned_bound=14 pruned_infeasible=7
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=73
[tsp-debug] root: lower_bound=50 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=73 expanded=136 created=203 pruned_bound=67 pruned_infeasible=70
```
</details>

---

## 12. `examples/random/complete/rnd-09-complete-n8.txt` (n=8)

**DP optimal cost:** 93  
**DP time:** 2ms  

**DP optimal tour:** `0 -> 7 -> 2 -> 3 -> 1 -> 6 -> 4 -> 5 -> 0`  

| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|
| DP | 93 | N/A | 2ms | - | - | - | - | - |
| Smart | 93 | :white_check_mark: | 8ms | 173 | 111 | 62 | 50 | - |
| Simple | 93 | :white_check_mark: | 30ms | 1113 | 769 | 344 | 426 | - |
| Concorde | 93 | :white_check_mark: | 3ms | - | - | - | - | - |

**Tours found:**
- **DP (ground truth):** `0 -> 7 -> 2 -> 3 -> 1 -> 6 -> 4 -> 5 -> 0` cost=93
- **Smart:** `1 -> 3 -> 2 -> 7 -> 0 -> 5 -> 4 -> 6` cost=93 (=DP)
- **Simple:** `1 -> 3 -> 2 -> 7 -> 0 -> 5 -> 4 -> 6` cost=93 (=DP)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=8 method=exact
[tsp-debug] exact solve started: vertices=8
[tsp-debug] initial incumbent: cost=93
[tsp-debug] root: lower_bound=40 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=93 expanded=111 created=173 pruned_bound=62 pruned_infeasible=50
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=8 method=exact
[tsp-debug] exact solve started: vertices=8
[tsp-debug] initial incumbent: cost=93
[tsp-debug] root: lower_bound=40 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=93 expanded=769 created=1113 pruned_bound=344 pruned_infeasible=426
```
</details>

---

## 13. `examples/random/complete/rnd-10-complete-n7.txt` (n=7)

**DP optimal cost:** 49  
**DP time:** <1ms  

**DP optimal tour:** `0 -> 6 -> 4 -> 5 -> 3 -> 2 -> 1 -> 0`  

| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|
| DP | 49 | N/A | <1ms | - | - | - | - | - |
| Smart | 49 | :white_check_mark: | 5ms | 22 | 13 | 9 | 5 | - |
| Simple | 49 | :white_check_mark: | 7ms | 106 | 68 | 38 | 31 | - |
| Concorde | 49 | :white_check_mark: | 3ms | - | - | - | - | - |

**Tours found:**
- **DP (ground truth):** `0 -> 6 -> 4 -> 5 -> 3 -> 2 -> 1 -> 0` cost=49
- **Smart:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6` cost=49 (=DP)
- **Simple:** `0 -> 1 -> 2 -> 3 -> 5 -> 4 -> 6` cost=49 (=DP)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=49
[tsp-debug] root: lower_bound=45 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=49 expanded=13 created=22 pruned_bound=9 pruned_infeasible=5
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=49
[tsp-debug] root: lower_bound=45 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=49 expanded=68 created=106 pruned_bound=38 pruned_infeasible=31
```
</details>

---

## 14. `examples/random/sparse/rnd-01-sparse-n5.txt` (n=5)

**DP optimal cost:** 110  
**DP time:** <1ms  

**DP optimal tour:** `0 -> 3 -> 4 -> 2 -> 1 -> 0`  

| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|
| DP | 110 | N/A | <1ms | - | - | - | - | - |
| Smart | 110 | :white_check_mark: | 4ms | 6 | 3 | 3 | 1 | - |
| Simple | 110 | :white_check_mark: | 4ms | 8 | 6 | 2 | 5 | - |
| Concorde | 110 | :white_check_mark: | 2ms | - | - | - | - | - |

**Tours found:**
- **DP (ground truth):** `0 -> 3 -> 4 -> 2 -> 1 -> 0` cost=110
- **Smart:** `3 -> 4 -> 2 -> 1 -> 0` cost=110 (=DP)
- **Simple:** `3 -> 4 -> 2 -> 1 -> 0` cost=110 (=DP)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=110
[tsp-debug] root: lower_bound=102 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=110 expanded=3 created=6 pruned_bound=3 pruned_infeasible=1
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=5 method=exact
[tsp-debug] exact solve started: vertices=5
[tsp-debug] initial incumbent: cost=110
[tsp-debug] root: lower_bound=102 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=110 expanded=6 created=8 pruned_bound=2 pruned_infeasible=5
```
</details>

---

## 15. `examples/random/sparse/rnd-02-sparse-n7.txt` (n=7)

**DP optimal cost:** 196  
**DP time:** <1ms  

**DP optimal tour:** `0 -> 4 -> 2 -> 6 -> 1 -> 5 -> 3 -> 0`  

| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|
| DP | 196 | N/A | <1ms | - | - | - | - | - |
| Smart | 196 | :white_check_mark: | 2ms | 32 | 22 | 10 | 13 | - |
| Simple | 196 | :white_check_mark: | 3ms | 92 | 71 | 21 | 51 | - |
| Concorde | 196 | :white_check_mark: | 2ms | - | - | - | - | - |

**Tours found:**
- **DP (ground truth):** `0 -> 4 -> 2 -> 6 -> 1 -> 5 -> 3 -> 0` cost=196
- **Smart:** `1 -> 5 -> 3 -> 0 -> 4 -> 2 -> 6` cost=196 (=DP)
- **Simple:** `1 -> 5 -> 3 -> 0 -> 4 -> 2 -> 6` cost=196 (=DP)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=196
[tsp-debug] root: lower_bound=161 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=196 expanded=22 created=32 pruned_bound=10 pruned_infeasible=13
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=196
[tsp-debug] root: lower_bound=161 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=196 expanded=71 created=92 pruned_bound=21 pruned_infeasible=51
```
</details>

---

## 16. `examples/random/sparse/rnd-03-sparse-n7.txt` (n=7)

**DP optimal cost:** 86  
**DP time:** <1ms  

**DP optimal tour:** `0 -> 2 -> 5 -> 4 -> 3 -> 6 -> 1 -> 0`  

| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|
| DP | 86 | N/A | <1ms | - | - | - | - | - |
| Smart | 86 | :white_check_mark: | 4ms | 11 | 7 | 4 | 4 | - |
| Simple | 86 | :white_check_mark: | 2ms | 31 | 21 | 10 | 12 | - |
| Concorde | 86 | :white_check_mark: | 2ms | - | - | - | - | - |

**Tours found:**
- **DP (ground truth):** `0 -> 2 -> 5 -> 4 -> 3 -> 6 -> 1 -> 0` cost=86
- **Smart:** `1 -> 6 -> 3 -> 4 -> 5 -> 2 -> 0` cost=86 (=DP)
- **Simple:** `1 -> 6 -> 3 -> 4 -> 5 -> 2 -> 0` cost=86 (=DP)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=86
[tsp-debug] root: lower_bound=78 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=86 expanded=7 created=11 pruned_bound=4 pruned_infeasible=4
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=86
[tsp-debug] root: lower_bound=78 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=86 expanded=21 created=31 pruned_bound=10 pruned_infeasible=12
```
</details>

---

## 17. `examples/random/sparse/rnd-04-sparse-n6.txt` (n=6)

**DP optimal cost:** 115  
**DP time:** <1ms  

**DP optimal tour:** `0 -> 4 -> 3 -> 5 -> 1 -> 2 -> 0`  

| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|
| DP | 115 | N/A | <1ms | - | - | - | - | - |
| Smart | 115 | :white_check_mark: | 3ms | 1 | 0 | 1 | 0 | - |
| Simple | 115 | :white_check_mark: | 2ms | 1 | 0 | 1 | 0 | - |
| Concorde | 115 | :white_check_mark: | 2ms | - | - | - | - | - |

**Tours found:**
- **DP (ground truth):** `0 -> 4 -> 3 -> 5 -> 1 -> 2 -> 0` cost=115
- **Smart:** `0 -> 4 -> 3 -> 5 -> 1 -> 2` cost=115 (=DP)
- **Simple:** `0 -> 4 -> 3 -> 5 -> 1 -> 2` cost=115 (=DP)

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

## 18. `examples/random/sparse/rnd-05-sparse-n6.txt` (n=6)

**DP optimal cost:** 100  
**DP time:** <1ms  

**DP optimal tour:** `0 -> 5 -> 2 -> 1 -> 4 -> 3 -> 0`  

| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|
| DP | 100 | N/A | <1ms | - | - | - | - | - |
| Smart | 100 | :white_check_mark: | 5ms | 49 | 36 | 13 | 24 | - |
| Simple | 100 | :white_check_mark: | 6ms | 83 | 68 | 15 | 54 | - |
| Concorde | 100 | :white_check_mark: | 2ms | - | - | - | - | - |

**Tours found:**
- **DP (ground truth):** `0 -> 5 -> 2 -> 1 -> 4 -> 3 -> 0` cost=100
- **Smart:** `0 -> 5 -> 2 -> 1 -> 4 -> 3` cost=100 (=DP)
- **Simple:** `0 -> 5 -> 2 -> 1 -> 4 -> 3` cost=100 (=DP)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=6 method=exact
[tsp-debug] exact solve started: vertices=6
[tsp-debug] initial incumbent: cost=100
[tsp-debug] root: lower_bound=53 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=100 expanded=36 created=49 pruned_bound=13 pruned_infeasible=24
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=6 method=exact
[tsp-debug] exact solve started: vertices=6
[tsp-debug] initial incumbent: cost=100
[tsp-debug] root: lower_bound=53 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=100 expanded=68 created=83 pruned_bound=15 pruned_infeasible=54
```
</details>

---

## 19. `examples/random/sparse/rnd-06-sparse-n7.txt` (n=7)

**DP optimal cost:** 199  
**DP time:** <1ms  

**DP optimal tour:** `0 -> 6 -> 2 -> 3 -> 5 -> 1 -> 4 -> 0`  

| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |
|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|
| DP | 199 | N/A | <1ms | - | - | - | - | - |
| Smart | 199 | :white_check_mark: | 5ms | 48 | 38 | 10 | 29 | - |
| Simple | 199 | :white_check_mark: | 6ms | 87 | 69 | 18 | 52 | - |
| Concorde | 199 | :white_check_mark: | 3ms | - | - | - | - | - |

**Tours found:**
- **DP (ground truth):** `0 -> 6 -> 2 -> 3 -> 5 -> 1 -> 4 -> 0` cost=199
- **Smart:** `4 -> 1 -> 5 -> 3 -> 2 -> 6 -> 0` cost=199 (=DP)
- **Simple:** `4 -> 1 -> 5 -> 3 -> 2 -> 6 -> 0` cost=199 (=DP)

<details>
<summary>Smart debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=199
[tsp-debug] root: lower_bound=169 search=recursive-dfs strategy=smart
[tsp-debug] exact solve finished: feasible=yes cost=199 expanded=38 created=48 pruned_bound=10 pruned_infeasible=29
```
</details>

<details>
<summary>Simple debug output</summary>

```
[tsp-debug] problem loaded: name=matrix dimension=7 method=exact
[tsp-debug] exact solve started: vertices=7
[tsp-debug] initial incumbent: cost=199
[tsp-debug] root: lower_bound=169 search=recursive-dfs strategy=simple
[tsp-debug] exact solve finished: feasible=yes cost=199 expanded=69 created=87 pruned_bound=18 pruned_infeasible=52
```
</details>

---


## Summary

| Algorithm | Solved | Timeout | Error | Wrong Cost | Total Time | Avg Time | Avg B&B Nodes | Total B&B Nodes |
|-----------|--------|---------|-------|------------|------------|----------|---------------|-----------------|
| Smart | 19 | 0 | 0 | 0 | 73ms | 4ms | 30 | 578 |
| Simple | 19 | 0 | 0 | 0 | 112ms | 6ms | 124 | 2355 |
| Concorde | 19 | 0 | 0 | 0 | 45ms | 2ms | 0 | 0 |

### Per-Instance Results Matrix

| # | Instance | n | DP Cost | Smart Cost | Simple Cost | Concorde Cost | DP Time | Smart Time | Simple Time | Concorde Time |
|---|---|---|---|---|---|---|---|---|---|---|
| 1 | `examples/five-city.txt` | 5 | 26 | 26 | 26 | 26 | <1ms | 4ms | 4ms | 2ms |
| 2 | `examples/converted/five-node-euc.txt` | 5 | 8 | 8 | 8 | 8 | <1ms | 4ms | 6ms | 3ms |
| 3 | `examples/converted/five-node-explicit.txt` | 5 | 26 | 26 | 26 | 26 | <1ms | 4ms | 4ms | 2ms |
| 4 | `examples/random/complete/rnd-01-complete-n4.txt` | 4 | 45 | 45 | 45 | 45 | <1ms | 3ms | 3ms | 2ms |
| 5 | `examples/random/complete/rnd-02-complete-n5.txt` | 5 | 136 | 136 | 136 | 136 | <1ms | 4ms | 5ms | 3ms |
| 6 | `examples/random/complete/rnd-03-complete-n4.txt` | 4 | 102 | 102 | 102 | 102 | <1ms | 2ms | 4ms | 2ms |
| 7 | `examples/random/complete/rnd-04-complete-n5.txt` | 5 | 88 | 88 | 88 | 88 | <1ms | 2ms | 2ms | 2ms |
| 8 | `examples/random/complete/rnd-05-complete-n7.txt` | 7 | 74 | 74 | 74 | 74 | <1ms | 2ms | 7ms | 3ms |
| 9 | `examples/random/complete/rnd-06-complete-n5.txt` | 5 | 51 | 51 | 51 | 51 | <1ms | 2ms | 4ms | 2ms |
| 10 | `examples/random/complete/rnd-07-complete-n8.txt` | 8 | 64 | 64 | 64 | 64 | 2ms | 5ms | 6ms | 2ms |
| 11 | `examples/random/complete/rnd-08-complete-n7.txt` | 7 | 73 | 73 | 73 | 73 | <1ms | 5ms | 9ms | 3ms |
| 12 | `examples/random/complete/rnd-09-complete-n8.txt` | 8 | 93 | 93 | 93 | 93 | 2ms | 8ms | 30ms | 3ms |
| 13 | `examples/random/complete/rnd-10-complete-n7.txt` | 7 | 49 | 49 | 49 | 49 | <1ms | 5ms | 7ms | 3ms |
| 14 | `examples/random/sparse/rnd-01-sparse-n5.txt` | 5 | 110 | 110 | 110 | 110 | <1ms | 4ms | 4ms | 2ms |
| 15 | `examples/random/sparse/rnd-02-sparse-n7.txt` | 7 | 196 | 196 | 196 | 196 | <1ms | 2ms | 3ms | 2ms |
| 16 | `examples/random/sparse/rnd-03-sparse-n7.txt` | 7 | 86 | 86 | 86 | 86 | <1ms | 4ms | 2ms | 2ms |
| 17 | `examples/random/sparse/rnd-04-sparse-n6.txt` | 6 | 115 | 115 | 115 | 115 | <1ms | 3ms | 2ms | 2ms |
| 18 | `examples/random/sparse/rnd-05-sparse-n6.txt` | 6 | 100 | 100 | 100 | 100 | <1ms | 5ms | 6ms | 2ms |
| 19 | `examples/random/sparse/rnd-06-sparse-n7.txt` | 7 | 199 | 199 | 199 | 199 | <1ms | 5ms | 6ms | 3ms |
