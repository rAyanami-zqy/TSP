# Heatmap Comparison: fri26 (n=26)

## Overall Performance

| Metric | Old (0702) | New (0708) | Delta |
|---|---|---|---|
| Native elapsed | 1.937s | 2.198s | 260.9ms (+13.5%) |
| `cost` | `937` | `937` | |
| `initial_upper_bound` | `937` | `937` | |
| `method` | `exact` | `exact` | |
| `nodes_created` | `984246` | `984246` | |
| `nodes_expanded` | `492159` | `264044` | |
| `pruned_bound` | `492087` | `492087` | |
| `pruned_infeasible` | `73` | `73` | |
| `root_lower_bound` | `824` | `824` | |

## Top Changes by Self Time %

| # | Function | Kind | Old Self% | New Self% | Delta | Old ms | New ms |
|---:|---|---|---:|---:|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::opera` | solver | 21.60% | 0.00% | -21.60% | 418.34ms | - |
| 2 | `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::operat` | solver | 0.00% | 20.11% | +20.11% | - | 441.94ms |
| 3 | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | solver | 33.04% | 27.01% | -6.03% | 639.91ms | 593.58ms |
| 4 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 0.00% | 4.38% | +4.38% | - | 96.26ms |
| 5 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 2.04% | 5.11% | +3.07% | 39.51ms | 112.30ms |
| 6 | `void std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | solver | 2.86% | 0.89% | -1.97% | 55.39ms | 19.56ms |
| 7 | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | solver | 1.95% | 0.00% | -1.95% | 37.77ms | - |
| 8 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-e` | runtime | 2.07% | 3.62% | +1.55% | 40.09ms | 79.55ms |
| 9 | `./malloc/./malloc/malloc.c:malloc_consolidate` | runtime | 0.00% | 0.62% | +0.62% | - | 13.63ms |
| 10 | `tsp::BranchBoundSolver::search(...)::{lambda(...)::LevelChan` | solver | 0.00% | 0.61% | +0.61% | - | 13.41ms |
| 11 | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | runtime | 0.53% | 1.00% | +0.47% | 10.26ms | 21.98ms |
| 12 | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | runtime | 0.32% | 0.77% | +0.45% | 6.20ms | 16.92ms |
| 13 | `tsp::BranchBoundSolver::search(...)` | solver | 0.82% | 0.44% | -0.38% | 15.88ms | 9.67ms |
| 14 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 20.05% | 19.70% | -0.35% | 388.32ms | 432.93ms |
| 15 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | solver | 0.00% | 0.33% | +0.33% | - | 7.25ms |
| 16 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 4.38% | 4.49% | +0.11% | 84.83ms | 98.67ms |
| 17 | `./malloc/./malloc/malloc.c:malloc` | runtime | 3.67% | 3.69% | +0.02% | 71.08ms | 81.09ms |
| 18 | `std::vector<int, std::allocator<> >::_M_fill_assign(...)` | runtime | 0.51% | 0.49% | -0.02% | 9.88ms | 10.77ms |
| 19 | `./malloc/./malloc/malloc.c:free` | runtime | 2.30% | 2.31% | +0.01% | 44.55ms | 50.77ms |
| 20 | `operator new(unsigned long)` | runtime | 1.31% | 1.32% | +0.01% | 25.37ms | 29.01ms |
| 21 | `operator delete(void*)` | runtime | 0.16% | 0.17% | +0.01% | 3.10ms | 3.74ms |
| 22 | `operator delete(...)` | runtime | 0.00% | 0.00% | +0.00% | - | 0.00ms |
| 23 | `tsp::BranchBoundSolver::solve(tsp::BranchStrategy)` | solver | 0.00% | 0.00% | +0.00% | - | - |
| 24 | `(below main)` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |
| 25 | `main` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |

## New Functions in 0708

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::operat` | 20.11% | 1974.10% |
| `tsp::BranchBoundSolver::bpPartition(...)` | 4.38% | 25.35% |
| `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.62% | 0.00% |
| `tsp::BranchBoundSolver::search(...)::{lambda(...)::LevelChan` | 0.61% | 0.00% |
| `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | 0.33% | 1.63% |

## Removed Functions (was in 0702)

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::opera` | 21.60% | 2015.80% |
| `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | 1.95% | 1.95% |

## Summary

- Old version functions: 27
- New version functions: 32
- Solver self%: 80.3% → 73.5% (-6.9%)
- Runtime self%: 17.3% → 23.6% (+6.3%)
