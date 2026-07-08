# Heatmap Comparison: dj38 (n=38)

## Overall Performance

| Metric | Old (0702) | New (0708) | Delta |
|---|---|---|---|
| Native elapsed | 113.7ms | 125.6ms | 11.8ms (+10.4%) |
| `cost` | `6656` | `6656` | |
| `initial_upper_bound` | `6656` | `6656` | |
| `method` | `exact` | `exact` | |
| `nodes_created` | `25733` | `25733` | |
| `nodes_expanded` | `12866` | `7990` | |
| `pruned_bound` | `12867` | `12867` | |
| `pruned_infeasible` | `0` | `0` | |
| `root_lower_bound` | `6285` | `6285` | |

## Top Changes by Self Time %

| # | Function | Kind | Old Self% | New Self% | Delta | Old ms | New ms |
|---:|---|---|---:|---:|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::opera` | solver | 23.34% | 0.00% | -23.34% | 26.55ms | - |
| 2 | `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::operat` | solver | 0.00% | 21.93% | +21.93% | - | 27.54ms |
| 3 | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | solver | 37.23% | 30.13% | -7.10% | 42.35ms | 37.83ms |
| 4 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-e` | runtime | 4.44% | 8.30% | +3.86% | 5.05ms | 10.42ms |
| 5 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 0.00% | 3.58% | +3.58% | - | 4.50ms |
| 6 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 1.78% | 3.88% | +2.10% | 2.02ms | 4.87ms |
| 7 | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | solver | 1.18% | 0.00% | -1.18% | 1.34ms | - |
| 8 | `void std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | solver | 1.29% | 0.39% | -0.90% | 1.47ms | 0.49ms |
| 9 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 20.25% | 19.68% | -0.57% | 23.03ms | 24.71ms |
| 10 | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | runtime | 0.48% | 1.04% | +0.56% | 0.55ms | 1.31ms |
| 11 | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | runtime | 0.29% | 0.61% | +0.32% | 0.33ms | 0.77ms |
| 12 | `tsp::BranchBoundSolver::search(...)` | solver | 0.36% | 0.21% | -0.15% | 0.41ms | 0.26ms |
| 13 | `std::_Function_handler<void (int, double, int), tsp::BranchB` | solver | 1.14% | 1.09% | -0.05% | 1.30ms | 1.37ms |
| 14 | `./malloc/./malloc/malloc.c:malloc` | runtime | 1.72% | 1.76% | +0.04% | 1.96ms | 2.21ms |
| 15 | `./malloc/./malloc/malloc.c:free` | runtime | 1.09% | 1.12% | +0.03% | 1.24ms | 1.41ms |
| 16 | `operator new(unsigned long)` | runtime | 0.62% | 0.64% | +0.02% | 0.71ms | 0.80ms |
| 17 | `tsp::BranchBoundSolver::twoOpt(...) const` | solver | 0.53% | 0.51% | -0.02% | 0.60ms | 0.64ms |
| 18 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 2.05% | 2.06% | +0.01% | 2.33ms | 2.59ms |
| 19 | `operator delete(void*)` | runtime | 0.08% | 0.08% | +0.00% | 0.09ms | 0.10ms |
| 20 | `tsp::BranchBoundSolver::linKernighan(...) const` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |
| 21 | `tsp::BranchBoundSolver::linKernighanImprove(...) const` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |
| 22 | `operator delete(...)` | runtime | 0.00% | 0.00% | +0.00% | - | 0.00ms |
| 23 | `tsp::BranchBoundSolver::solve(tsp::BranchStrategy)` | solver | 0.00% | 0.00% | +0.00% | - | - |
| 24 | `(below main)` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |
| 25 | `tsp::BranchBoundSolver::findInitialTour(...)` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |

## New Functions in 0708

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::operat` | 21.93% | 1670.30% |
| `tsp::BranchBoundSolver::bpPartition(...)` | 3.58% | 23.27% |

## Removed Functions (was in 0702)

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::opera` | 23.34% | 1708.90% |
| `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | 1.18% | 1.18% |

## Summary

- Old version functions: 34
- New version functions: 38
- Solver self%: 85.3% → 77.5% (-7.8%)
- Runtime self%: 12.6% → 19.5% (+6.9%)
