# Heatmap Comparison: wi29 (n=29)

## Overall Performance

| Metric | Old (0702) | New (0708) | Delta |
|---|---|---|---|
| Native elapsed | 201.8ms | 225.4ms | 23.6ms (+11.7%) |
| `cost` | `27603` | `27603` | |
| `initial_upper_bound` | `27603` | `27603` | |
| `method` | `exact` | `exact` | |
| `nodes_created` | `77709` | `77709` | |
| `nodes_expanded` | `38866` | `22423` | |
| `pruned_bound` | `38843` | `38843` | |
| `pruned_infeasible` | `24` | `24` | |
| `root_lower_bound` | `22727` | `22727` | |

## Top Changes by Self Time %

| # | Function | Kind | Old Self% | New Self% | Delta | Old ms | New ms |
|---:|---|---|---:|---:|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::opera` | solver | 21.32% | 0.00% | -21.32% | 43.02ms | - |
| 2 | `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::operat` | solver | 0.00% | 20.04% | +20.04% | - | 45.17ms |
| 3 | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | solver | 32.63% | 26.82% | -5.81% | 65.84ms | 60.45ms |
| 4 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 0.00% | 4.22% | +4.22% | - | 9.51ms |
| 5 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 1.82% | 4.22% | +2.40% | 3.67ms | 9.51ms |
| 6 | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | solver | 1.66% | 0.00% | -1.66% | 3.35ms | - |
| 7 | `void std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | solver | 2.21% | 0.70% | -1.51% | 4.46ms | 1.58ms |
| 8 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-e` | runtime | 1.83% | 3.28% | +1.45% | 3.69ms | 7.39ms |
| 9 | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | runtime | 0.47% | 0.89% | +0.42% | 0.95ms | 2.01ms |
| 10 | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | runtime | 0.29% | 0.64% | +0.35% | 0.59ms | 1.44ms |
| 11 | `tsp::BranchBoundSolver::search(...)` | solver | 0.64% | 0.36% | -0.28% | 1.29ms | 0.81ms |
| 12 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | solver | 0.00% | 0.26% | +0.26% | - | 0.59ms |
| 13 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 24.38% | 24.13% | -0.25% | 49.19ms | 54.38ms |
| 14 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 3.48% | 3.63% | +0.15% | 7.02ms | 8.18ms |
| 15 | `operator delete(...)` | runtime | 0.00% | 0.13% | +0.13% | - | 0.29ms |
| 16 | `./malloc/./malloc/malloc.c:malloc` | runtime | 2.91% | 3.00% | +0.09% | 5.87ms | 6.76ms |
| 17 | `./malloc/./malloc/malloc.c:free` | runtime | 1.82% | 1.88% | +0.06% | 3.67ms | 4.24ms |
| 18 | `operator new(unsigned long)` | runtime | 1.04% | 1.08% | +0.04% | 2.10ms | 2.43ms |
| 19 | `std::_Function_handler<void (int, double, int), tsp::BranchB` | solver | 0.60% | 0.58% | -0.02% | 1.21ms | 1.31ms |
| 20 | `operator delete(void*)` | runtime | 0.13% | 0.13% | +0.00% | 0.26ms | 0.29ms |
| 21 | `tsp::BranchBoundSolver::linKernighan(...) const` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |
| 22 | `tsp::BranchBoundSolver::linKernighanImprove(...) const` | solver | 0.00% | 0.00% | +0.00% | - | - |
| 23 | `tsp::BranchBoundSolver::solve(tsp::BranchStrategy)` | solver | 0.00% | 0.00% | +0.00% | - | - |
| 24 | `(below main)` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |
| 25 | `tsp::BranchBoundSolver::findInitialTour(...)` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |

## New Functions in 0708

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::operat` | 20.04% | 1813.10% |
| `tsp::BranchBoundSolver::bpPartition(...)` | 4.22% | 25.74% |
| `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | 0.26% | 1.41% |
| `operator delete(...)` | 0.13% | 7.65% |

## Removed Functions (was in 0702)

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::opera` | 21.32% | 1850.20% |
| `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | 1.66% | 1.66% |

## Summary

- Old version functions: 34
- New version functions: 36
- Solver self%: 83.4% → 77.1% (-6.3%)
- Runtime self%: 13.8% → 18.9% (+5.1%)
