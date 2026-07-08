# Heatmap Comparison: gr17 (n=17)

## Overall Performance

| Metric | Old (0702) | New (0708) | Delta |
|---|---|---|---|
| Native elapsed | 2.210s | 2.474s | 263.9ms (+11.9%) |
| `cost` | `2085` | `2085` | |
| `initial_upper_bound` | `2085` | `2085` | |
| `method` | `exact` | `exact` | |
| `nodes_created` | `1817688` | `1817688` | |
| `nodes_expanded` | `920131` | `463820` | |
| `pruned_bound` | `897557` | `897458` | |
| `pruned_infeasible` | `22575` | `22575` | |
| `root_lower_bound` | `1501` | `1501` | |

## Top Changes by Self Time %

| # | Function | Kind | Old Self% | New Self% | Delta | Old ms | New ms |
|---:|---|---|---:|---:|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::opera` | solver | 16.01% | 0.00% | -16.01% | 353.79ms | - |
| 2 | `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::operat` | solver | 0.00% | 13.57% | +13.57% | - | 335.68ms |
| 3 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 0.00% | 5.00% | +5.00% | - | 123.69ms |
| 4 | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | solver | 21.82% | 17.10% | -4.72% | 482.18ms | 423.00ms |
| 5 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 1.77% | 5.92% | +4.15% | 39.11ms | 146.44ms |
| 6 | `void std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | solver | 4.44% | 1.63% | -2.81% | 98.12ms | 40.32ms |
| 7 | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | solver | 2.75% | 0.00% | -2.75% | 60.77ms | - |
| 8 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 22.05% | 20.66% | -1.39% | 487.26ms | 511.07ms |
| 9 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-e` | runtime | 2.42% | 3.80% | +1.38% | 53.48ms | 94.00ms |
| 10 | `tsp::BranchBoundSolver::search(...)::{lambda(...)::LevelChan` | solver | 0.00% | 1.11% | +1.11% | - | 27.46ms |
| 11 | `./malloc/./malloc/malloc.c:malloc_consolidate` | runtime | 0.00% | 0.86% | +0.86% | - | 21.27ms |
| 12 | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | runtime | 0.00% | 0.80% | +0.80% | - | 19.79ms |
| 13 | `tsp::BranchBoundSolver::search(...)` | solver | 1.58% | 0.79% | -0.79% | 34.91ms | 19.54ms |
| 14 | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | runtime | 0.45% | 1.06% | +0.61% | 9.94ms | 26.22ms |
| 15 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | solver | 0.00% | 0.60% | +0.60% | - | 14.84ms |
| 16 | `./malloc/./malloc/malloc.c:malloc` | runtime | 6.79% | 6.70% | -0.09% | 150.05ms | 165.74ms |
| 17 | `std::vector<int, std::allocator<> >::_M_fill_assign(...)` | runtime | 0.86% | 0.80% | -0.06% | 19.00ms | 19.79ms |
| 18 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 8.14% | 8.20% | +0.06% | 179.88ms | 202.84ms |
| 19 | `./malloc/./malloc/malloc.c:free` | runtime | 4.23% | 4.18% | -0.05% | 93.47ms | 103.40ms |
| 20 | `operator new(unsigned long)` | runtime | 2.42% | 2.39% | -0.03% | 53.48ms | 59.12ms |
| 21 | `operator delete(...)` | runtime | 0.30% | 0.30% | +0.00% | 6.63ms | 7.42ms |
| 22 | `operator delete(void*)` | runtime | 0.30% | 0.30% | +0.00% | 6.63ms | 7.42ms |
| 23 | `tsp::BranchBoundSolver::solve(tsp::BranchStrategy)` | solver | 0.00% | 0.00% | +0.00% | - | - |
| 24 | `(below main)` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |
| 25 | `main` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |

## New Functions in 0708

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::operat` | 13.57% | 2032.00% |
| `tsp::BranchBoundSolver::bpPartition(...)` | 5.00% | 33.24% |
| `tsp::BranchBoundSolver::search(...)::{lambda(...)::LevelChan` | 1.11% | 1.11% |
| `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.86% | 0.98% |
| `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.80% | 0.00% |
| `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | 0.60% | 2.59% |

## Removed Functions (was in 0702)

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::opera` | 16.01% | 2096.50% |
| `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | 2.75% | 2.75% |

## Summary

- Old version functions: 25
- New version functions: 33
- Solver self%: 68.7% → 60.5% (-8.2%)
- Runtime self%: 27.7% → 35.3% (+7.6%)
