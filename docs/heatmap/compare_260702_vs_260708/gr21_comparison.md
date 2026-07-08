# Heatmap Comparison: gr21 (n=21)

## Overall Performance

| Metric | Old (0702) | New (0708) | Delta |
|---|---|---|---|
| Native elapsed | 80.6ms | 89.6ms | 9.0ms (+11.2%) |
| `cost` | `2707` | `2707` | |
| `initial_upper_bound` | `2707` | `2707` | |
| `method` | `exact` | `exact` | |
| `nodes_created` | `34213` | `34213` | |
| `nodes_expanded` | `17314` | `9911` | |
| `pruned_bound` | `16899` | `16899` | |
| `pruned_infeasible` | `416` | `416` | |
| `root_lower_bound` | `2252` | `2252` | |

## Top Changes by Self Time %

| # | Function | Kind | Old Self% | New Self% | Delta | Old ms | New ms |
|---:|---|---|---:|---:|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::opera` | solver | 19.18% | 0.00% | -19.18% | 15.46ms | - |
| 2 | `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::operat` | solver | 0.00% | 17.07% | +17.07% | - | 15.30ms |
| 3 | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | solver | 28.51% | 22.50% | -6.01% | 22.98ms | 20.16ms |
| 4 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 0.00% | 4.27% | +4.27% | - | 3.83ms |
| 5 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 2.69% | 6.59% | +3.90% | 2.17ms | 5.91ms |
| 6 | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | solver | 1.97% | 0.00% | -1.97% | 1.59ms | - |
| 7 | `void std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | solver | 2.71% | 1.00% | -1.71% | 2.18ms | 0.90ms |
| 8 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-e` | runtime | 2.01% | 3.38% | +1.37% | 1.62ms | 3.03ms |
| 9 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 22.50% | 21.29% | -1.21% | 18.14ms | 19.08ms |
| 10 | `./malloc/./malloc/malloc.c:malloc_consolidate` | runtime | 0.21% | 0.88% | +0.67% | 0.17ms | 0.79ms |
| 11 | `tsp::BranchBoundSolver::search(...)::{lambda(...)::LevelChan` | solver | 0.00% | 0.67% | +0.67% | - | 0.60ms |
| 12 | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | runtime | 0.42% | 0.96% | +0.54% | 0.34ms | 0.86ms |
| 13 | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | runtime | 0.64% | 1.17% | +0.53% | 0.52ms | 1.05ms |
| 14 | `tsp::BranchBoundSolver::search(...)` | solver | 0.97% | 0.51% | -0.46% | 0.78ms | 0.46ms |
| 15 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | solver | 0.00% | 0.38% | +0.38% | - | 0.34ms |
| 16 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 4.99% | 5.28% | +0.29% | 4.02ms | 4.73ms |
| 17 | `./malloc/./malloc/malloc.c:malloc` | runtime | 4.17% | 4.29% | +0.12% | 3.36ms | 3.84ms |
| 18 | `./malloc/./malloc/malloc.c:free` | runtime | 2.61% | 2.69% | +0.08% | 2.10ms | 2.41ms |
| 19 | `std::_Function_handler<void (int, double, int), tsp::BranchB` | solver | 0.92% | 0.86% | -0.06% | 0.74ms | 0.77ms |
| 20 | `operator new(unsigned long)` | runtime | 1.49% | 1.54% | +0.05% | 1.20ms | 1.38ms |
| 21 | `std::vector<int, std::allocator<> >::_M_fill_assign(...)` | runtime | 0.56% | 0.53% | -0.03% | 0.45ms | 0.47ms |
| 22 | `operator delete(...)` | runtime | 0.19% | 0.19% | +0.00% | 0.15ms | 0.17ms |
| 23 | `operator delete(void*)` | runtime | 0.19% | 0.19% | +0.00% | 0.15ms | 0.17ms |
| 24 | `tsp::BranchBoundSolver::linKernighan(...) const` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |
| 25 | `tsp::BranchBoundSolver::linKernighanImprove(...) const` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |

## New Functions in 0708

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::operat` | 17.07% | 1381.30% |
| `tsp::BranchBoundSolver::bpPartition(...)` | 4.27% | 28.28% |
| `tsp::BranchBoundSolver::search(...)::{lambda(...)::LevelChan` | 0.67% | 0.00% |
| `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | 0.38% | 1.85% |

## Removed Functions (was in 0702)

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::opera` | 19.18% | 1421.70% |
| `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | 1.97% | 1.97% |

## Summary

- Old version functions: 34
- New version functions: 39
- Solver self%: 76.8% → 68.5% (-8.2%)
- Runtime self%: 20.2% → 27.7% (+7.5%)
