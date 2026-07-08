# Heatmap Comparison: gr24 (n=24)

## Overall Performance

| Metric | Old (0702) | New (0708) | Delta |
|---|---|---|---|
| Native elapsed | 1.126s | 1.256s | 130.1ms (+11.6%) |
| `cost` | `1272` | `1272` | |
| `initial_upper_bound` | `1272` | `1272` | |
| `method` | `exact` | `exact` | |
| `nodes_created` | `571607` | `571607` | |
| `nodes_expanded` | `287023` | `158220` | |
| `pruned_bound` | `284584` | `284581` | |
| `pruned_infeasible` | `2440` | `2440` | |
| `root_lower_bound` | `1081` | `1081` | |

## Top Changes by Self Time %

| # | Function | Kind | Old Self% | New Self% | Delta | Old ms | New ms |
|---:|---|---|---:|---:|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::opera` | solver | 21.57% | 0.00% | -21.57% | 242.88ms | - |
| 2 | `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::operat` | solver | 0.00% | 20.03% | +20.03% | - | 251.60ms |
| 3 | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | solver | 32.82% | 26.81% | -6.01% | 369.56ms | 336.76ms |
| 4 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 0.00% | 4.44% | +4.44% | - | 55.77ms |
| 5 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 2.46% | 5.28% | +2.82% | 27.70ms | 66.32ms |
| 6 | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | solver | 1.95% | 0.00% | -1.95% | 21.96ms | - |
| 7 | `void std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | solver | 2.82% | 0.93% | -1.89% | 31.75ms | 11.68ms |
| 8 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-e` | runtime | 2.06% | 3.61% | +1.55% | 23.20ms | 45.35ms |
| 9 | `tsp::BranchBoundSolver::search(...)::{lambda(...)::LevelChan` | solver | 0.00% | 0.63% | +0.63% | - | 7.91ms |
| 10 | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | runtime | 0.57% | 1.06% | +0.49% | 6.42ms | 13.31ms |
| 11 | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | runtime | 0.38% | 0.79% | +0.41% | 4.28ms | 9.92ms |
| 12 | `tsp::BranchBoundSolver::search(...)` | solver | 0.87% | 0.47% | -0.40% | 9.80ms | 5.90ms |
| 13 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 19.13% | 18.74% | -0.39% | 215.41ms | 235.40ms |
| 14 | `./malloc/./malloc/malloc.c:malloc_consolidate` | runtime | 0.25% | 0.60% | +0.35% | 2.82ms | 7.54ms |
| 15 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | solver | 0.00% | 0.35% | +0.35% | - | 4.40ms |
| 16 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 4.56% | 4.75% | +0.19% | 51.35ms | 59.67ms |
| 17 | `operator delete(...)` | runtime | 0.00% | 0.18% | +0.18% | - | 2.26ms |
| 18 | `./malloc/./malloc/malloc.c:malloc` | runtime | 3.81% | 3.91% | +0.10% | 42.90ms | 49.11ms |
| 19 | `./malloc/./malloc/malloc.c:free` | runtime | 2.38% | 2.45% | +0.07% | 26.80ms | 30.77ms |
| 20 | `operator new(unsigned long)` | runtime | 1.36% | 1.40% | +0.04% | 15.31ms | 17.59ms |
| 21 | `std::vector<int, std::allocator<> >::_M_fill_assign(...)` | runtime | 0.48% | 0.46% | -0.02% | 5.40ms | 5.78ms |
| 22 | `operator delete(void*)` | runtime | 0.17% | 0.18% | +0.01% | 1.91ms | 2.26ms |
| 23 | `tsp::BranchBoundSolver::solve(tsp::BranchStrategy)` | solver | 0.00% | 0.00% | +0.00% | - | - |
| 24 | `(below main)` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |
| 25 | `main` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |

## New Functions in 0708

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::operat` | 20.03% | 1862.80% |
| `tsp::BranchBoundSolver::bpPartition(...)` | 4.44% | 25.49% |
| `tsp::BranchBoundSolver::search(...)::{lambda(...)::LevelChan` | 0.63% | 0.00% |
| `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | 0.35% | 1.68% |
| `operator delete(...)` | 0.18% | 9.81% |

## Removed Functions (was in 0702)

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::opera` | 21.57% | 1906.60% |
| `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | 1.95% | 1.95% |

## Summary

- Old version functions: 28
- New version functions: 32
- Solver self%: 79.2% → 72.4% (-6.8%)
- Runtime self%: 18.5% → 24.7% (+6.2%)
