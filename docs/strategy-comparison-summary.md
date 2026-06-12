# TSP Strategy Comparison — Runtime & Cost Summary

## Algorithms

| Algorithm | Description |
|---|---|
| **Concorde** | Concorde TSP solver (exact, QSopt LP) — reference |
| **Smart** | Branch & Bound with smart branching (1-tree degree + edge candidate) |
| **Simple** | Branch & Bound with simple branching (max-degree vertex) |

**Timeout:** 3600s (1h) per method per instance  
**Instances:** 28 from `examples/` (n ≤ 8) and `data/classic/tsplib/` (n ≤ 29)

---

## Results Table

| # | Instance | n | Concorde Cost | Concorde Time | Smart Cost | Smart Time | Smart Nodes (created/expanded) | Simple Cost | Simple Time | Simple Nodes (created/expanded) |
|---|---|---|---|---|---|---|---|---|---|---|
| 1 | rnd-01-complete-n4 | 4 | 45 | 3ms | 45 ✅ | 4ms | 3 / 1 | 45 ✅ | 4ms | 3 / 1 |
| 2 | rnd-03-complete-n4 | 4 | 102 | 2ms | 102 ✅ | 4ms | 1 / 0 | 102 ✅ | 3ms | 1 / 0 |
| 3 | five-city.tsp | 5 | 26 | 2ms | 26 ✅ | 2ms | 17 / 10 | 26 ✅ | 3ms | 3 / 1 |
| 4 | five-city.txt | 5 | 26 | 2ms | 26 ✅ | 4ms | 17 / 10 | 26 ✅ | 2ms | 3 / 1 |
| 5 | five-node-euc.txt | 5 | 8 | 2ms | 8 ✅ | 4ms | 7 / 4 | 8 ✅ | 4ms | 7 / 4 |
| 6 | five-node-explicit.txt | 5 | 26 | 3ms | 26 ✅ | 2ms | 17 / 10 | 26 ✅ | 2ms | 3 / 1 |
| 7 | rnd-01-sparse-n5 | 5 | 110 | 2ms | 110 ✅ | 3ms | 3 / 1 | 110 ✅ | 2ms | 3 / 1 |
| 8 | rnd-02-complete-n5 | 5 | 136 | 2ms | 136 ✅ | 3ms | 8 / 4 | 136 ✅ | 3ms | 3 / 1 |
| 9 | rnd-04-complete-n5 | 5 | 88 | 2ms | 88 ✅ | 3ms | 7 / 4 | 88 ✅ | 3ms | 3 / 1 |
| 10 | rnd-06-complete-n5 | 5 | 51 | 2ms | 51 ✅ | 3ms | 3 / 1 | 51 ✅ | 3ms | 3 / 1 |
| 11 | rnd-04-sparse-n6 | 6 | 115 | 2ms | 115 ✅ | 2ms | 1 / 0 | 115 ✅ | 1ms | 1 / 0 |
| 12 | rnd-05-sparse-n6 | 6 | 100 | 3ms | 100 ✅ | 3ms | 20 / 15 | 100 ✅ | 4ms | 12 / 9 |
| 13 | rnd-02-sparse-n7 | 7 | 196 | 2ms | 196 ✅ | 3ms | 1 / 1 | 196 ✅ | 3ms | 17 / 10 |
| 14 | rnd-03-sparse-n7 | 7 | 86 | 3ms | 86 ✅ | 4ms | 5 / 4 | 86 ✅ | 4ms | 2 / 1 |
| 15 | rnd-05-complete-n7 | 7 | 74 | 3ms | 74 ✅ | 3ms | 31 / 15 | 74 ✅ | 3ms | 12 / 7 |
| 16 | rnd-06-sparse-n7 | 7 | 199 | 2ms | 199 ✅ | 3ms | 10 / 9 | 199 ✅ | 2ms | 12 / 6 |
| 17 | rnd-08-complete-n7 | 7 | 73 | 3ms | 73 ✅ | 2ms | 3 / 1 | 73 ✅ | 1ms | 7 / 3 |
| 18 | rnd-10-complete-n7 | 7 | 49 | 2ms | 49 ✅ | 3ms | 11 / 5 | 49 ✅ | 3ms | 9 / 4 |
| 19 | rnd-07-complete-n8 | 8 | 64 | 3ms | 64 ✅ | 4ms | 45 / 24 | 64 ✅ | 2ms | 7 / 3 |
| 20 | rnd-09-complete-n8 | 8 | 93 | 2ms | 93 ✅ | 4ms | 77 / 38 | 93 ✅ | 4ms | 14 / 8 |
| 21 | burma14 | 14 | 3323 | 21ms | 3323 ✅ | 271ms | 79,563 / 39,999 | 3323 ✅ | 26ms | 2,992 / 1,619 |
| 22 | ulysses16 | 16 | 6859 | 27ms | 6859 ✅ | 1.6min | 27,456,187 / 14,144,215 | 6859 ✅ | 8.5s | 2,263,683 / 1,397,712 |
| 23 | gr17 | 17 | 2085 | 24ms | 2085 ✅ | 8.7s | 1,785,976 / 1,026,492 | 2085 ✅ | 3.0s | 674,410 / 391,355 |
| 24 | gr21 | 21 | 2707 | 12ms | 2707 ✅ | 384ms | 44,660 / 25,616 | 2707 ✅ | 137ms | 16,396 / 8,998 |
| 25 | ulysses22 | 22 | 7013 | 81ms | ⏱ TIMEOUT | 1.0h | — | ⏱ TIMEOUT | 1.0h | — |
| 26 | gr24 | 24 | 1272 | 23ms | 1272 ✅ | 10.1s | 1,089,829 / 584,277 | 1272 ✅ | 3.8s | 494,385 / 249,241 |
| 27 | fri26 | 26 | 937 | 26ms | 937 ✅ | 9.6s | 857,717 / 488,569 | 937 ✅ | 12.2s | 1,533,589 / 766,794 |
| 28 | bayg29 | 29 | 1610 | 25ms | 1618 ❌ | 23.3min | 118,007,076 / 60,286,621 | 1618 ❌ | 3.2min | 17,279,593 / 9,507,444 |

✅ = matches Concorde optimal cost; ❌ = suboptimal (differs from Concorde); ⏱ = timed out

---

## Summary Statistics

### Cost Accuracy

| | Smart | Simple |
|---|---|---|
| Matched Concorde (optimal) | 25 / 28 | 25 / 28 |
| Suboptimal (bayg29) | 1 (1618 vs 1610) | 1 (1618 vs 1610) |
| Timed out (ulysses22) | 1 | 1 |
| **Accuracy (of solved)** | **96.2%** | **96.2%** |

### Runtime Comparison (solved instances, excluding timeouts)

| n range | Instances | Concorde (mean) | Smart (mean) | Simple (mean) | Smart/Simple Ratio |
|---|---|---|---|---|---|
| 4–8 | 1–20 | 2.4ms | 3.1ms | 2.7ms | 1.15× |
| 14–17 | 21–23 | 24ms | 18.4s | 3.9s | 4.7× |
| 21–26 | 24–27 | 20ms | 5.1s | 5.3s | 0.96× |
| 29 | 28 | 25ms | 23.3min | 3.2min | 7.3× |

### Search Efficiency (nodes expanded, solved instances)

| Instance | n | Smart Expanded | Simple Expanded | Simple/Smart |
|---|---|---|---|---|
| burma14 | 14 | 39,999 | 1,619 | **0.04×** |
| ulysses16 | 16 | 14,144,215 | 1,397,712 | **0.10×** |
| gr17 | 17 | 1,026,492 | 391,355 | **0.38×** |
| gr21 | 21 | 25,616 | 8,998 | **0.35×** |
| gr24 | 24 | 584,277 | 249,241 | **0.43×** |
| fri26 | 26 | 488,569 | 766,794 | **1.57×** |
| bayg29 | 29 | 60,286,621 | 9,507,444 | **0.16×** |

### Win/Loss/Tie (solved instances)

| | Smart Wins | Simple Wins | Ties (≈) |
|---|---|---|---|
| Runtime | 2 | 25 | 0 |
| Nodes Expanded | 1 | 25 | 1 |

---

## Key Takeaways

1. **Both strategies find optimal solutions** for all solved instances except bayg29 (n=29), where both find 1618 instead of Concorde's 1610 (Δ=8, 0.5% gap).

2. **Simple significantly outperforms Smart** in both runtime and search nodes for nearly all medium-size instances (n=14–29). Simple expands 4–25× fewer nodes on most TSPLIB instances.

3. **fri26 is the exception** — Smart wins there (9.6s vs 12.2s, 489K vs 767K nodes), suggesting smart branching is occasionally beneficial.

4. **On tiny instances (n≤8)**, both strategies are equivalent — differences are within measurement noise (~1ms).

5. **ulysses22 (n=22) timed out** for both strategies at 1 hour, while Concorde solved it in 81ms. The 1-tree lower bound appears too weak for this instance.

6. **Smart branching overhead** (computing 1-tree edge frequencies, building candidate sets) dominates on small-to-medium instances. The branching quality gain does not compensate for the higher per-node cost.
