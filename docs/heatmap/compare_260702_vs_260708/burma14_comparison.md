# Heatmap Comparison: burma14 (n=14)

## Overall Performance

| Metric | Old (0702) | New (0708) | Delta |
|---|---|---|---|
| Native elapsed | 38.3ms | 40.7ms | 2.4ms (+6.4%) |
| `cost` | `3323` | `3323` | |
| `initial_upper_bound` | `3323` | `3323` | |
| `method` | `exact` | `exact` | |
| `nodes_created` | `18111` | `18111` | |
| `nodes_expanded` | `9162` | `4582` | |
| `pruned_bound` | `8949` | `8945` | |
| `pruned_infeasible` | `214` | `214` | |
| `root_lower_bound` | `2542` | `2542` | |

## Top Changes by Self Time %

| # | Function | Kind | Old Self% | New Self% | Delta | Old ms | New ms |
|---:|---|---|---:|---:|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::opera` | solver | 14.04% | 0.00% | -14.04% | 5.37ms | - |
| 2 | `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::operat` | solver | 0.00% | 11.56% | +11.56% | - | 4.71ms |
| 3 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 0.00% | 4.94% | +4.94% | - | 2.01ms |
| 4 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 0.85% | 5.08% | +4.23% | 0.33ms | 2.07ms |
| 5 | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | solver | 18.14% | 14.06% | -4.08% | 6.94ms | 5.72ms |
| 6 | `void std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | solver | 4.54% | 1.81% | -2.73% | 1.74ms | 0.74ms |
| 7 | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | solver | 2.73% | 0.00% | -2.73% | 1.04ms | - |
| 8 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 21.12% | 19.51% | -1.61% | 8.08ms | 7.94ms |
| 9 | `tsp::BranchBoundSolver::search(...)::{lambda(...)::LevelChan` | solver | 0.00% | 1.22% | +1.22% | - | 0.50ms |
| 10 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-e` | runtime | 1.65% | 2.59% | +0.94% | 0.63ms | 1.05ms |
| 11 | `tsp::BranchBoundSolver::search(...)` | solver | 1.77% | 0.88% | -0.89% | 0.68ms | 0.36ms |
| 12 | `./malloc/./malloc/malloc.c:malloc_consolidate` | runtime | 0.00% | 0.89% | +0.89% | - | 0.36ms |
| 13 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | solver | 0.00% | 0.66% | +0.66% | - | 0.27ms |
| 14 | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | runtime | 0.20% | 0.86% | +0.66% | 0.08ms | 0.35ms |
| 15 | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | runtime | 0.00% | 0.58% | +0.58% | - | 0.24ms |
| 16 | `std::_Function_handler<void (int, double, int), tsp::BranchB` | solver | 2.20% | 2.02% | -0.18% | 0.84ms | 0.82ms |
| 17 | `std::vector<tsp::BranchBoundSolver::OneTree, std::allocator<` | solver | 0.00% | 0.16% | +0.16% | - | 0.07ms |
| 18 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 9.39% | 9.47% | +0.08% | 3.59ms | 3.86ms |
| 19 | `std::vector<int, std::allocator<> >::_M_fill_assign(...)` | runtime | 1.02% | 0.94% | -0.08% | 0.39ms | 0.38ms |
| 20 | `./malloc/./malloc/malloc.c:malloc` | runtime | 7.75% | 7.68% | -0.07% | 2.97ms | 3.13ms |
| 21 | `./malloc/./malloc/malloc.c:free` | runtime | 4.83% | 4.79% | -0.04% | 1.85ms | 1.95ms |
| 22 | `tsp::BranchBoundSolver::lkSearch(...) const` | solver | 0.20% | 0.18% | -0.02% | 0.08ms | 0.07ms |
| 23 | `operator new(unsigned long)` | runtime | 2.76% | 2.74% | -0.02% | 1.06ms | 1.12ms |
| 24 | `./malloc/./malloc/arena.c:free` | runtime | 0.52% | 0.51% | -0.01% | 0.20ms | 0.21ms |
| 25 | `operator delete(...)` | runtime | 0.35% | 0.34% | -0.01% | 0.13ms | 0.14ms |

## New Functions in 0708

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::operat` | 11.56% | 1157.80% |
| `tsp::BranchBoundSolver::bpPartition(...)` | 4.94% | 34.18% |
| `tsp::BranchBoundSolver::search(...)::{lambda(...)::LevelChan` | 1.22% | 1.23% |
| `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.89% | 1.02% |
| `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | 0.66% | 2.82% |
| `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.58% | 0.00% |
| `std::vector<tsp::BranchBoundSolver::OneTree, std::allocator<` | 0.16% | 2.51% |

## Removed Functions (was in 0702)

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::opera` | 14.04% | 1212.40% |
| `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | 2.73% | 2.73% |

## Summary

- Old version functions: 34
- New version functions: 42
- Solver self%: 64.7% → 57.0% (-7.7%)
- Runtime self%: 29.7% → 36.8% (+7.1%)
