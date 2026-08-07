# Heatmap Comparison: burma14 (n=14)

## Overall Performance

| Metric | Old (0702) | New (0708) | Delta |
|---|---|---|---|
| Native elapsed | 36.1ms | 36.2ms | 109.1us (+0.3%) |
| `cost` | `3323` | `3323` | |
| `initial_upper_bound` | `3323` | `3323` | |
| `method` | `exact` | `exact` | |
| `nodes_created` | `8949` | `8949` | |
| `nodes_expanded` | `4582` | `4582` | |
| `pruned_bound` | `4367` | `4367` | |
| `pruned_infeasible` | `214` | `214` | |
| `root_lower_bound` | `2542` | `2542` | |

## Top Changes by Self Time %

| # | Function | Kind | Old Self% | New Self% | Delta | Old ms | New ms |
|---:|---|---|---:|---:|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 0.00% | 27.59% | +27.59% | - | 9.99ms |
| 2 | `tsp::BranchBoundSolver::updateOneTreeAfterCandidateRemoval(.` | solver | 23.69% | 0.00% | -23.69% | 8.55ms | - |
| 3 | `tsp::BranchBoundSolver::updateOneTreeAfterForbid(...) const` | solver | 6.90% | 0.00% | -6.90% | 2.49ms | - |
| 4 | `tsp::(anonymous namespace)::DisjointSet::unite(int, int)` | solver | 4.05% | 0.00% | -4.05% | 1.46ms | - |
| 5 | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | solver | 19.52% | 20.86% | +1.34% | 7.05ms | 7.55ms |
| 6 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 7.60% | 8.94% | +1.34% | 2.74ms | 3.24ms |
| 7 | `std::vector<int, std::allocator<> >::_M_fill_assign(...)` | runtime | 0.00% | 1.15% | +1.15% | - | 0.42ms |
| 8 | `./malloc/./malloc/malloc.c:malloc` | runtime | 6.21% | 7.31% | +1.10% | 2.24ms | 2.65ms |
| 9 | `./malloc/./malloc/malloc.c:free` | runtime | 3.89% | 4.57% | +0.68% | 1.40ms | 1.65ms |
| 10 | `tsp::(anonymous namespace)::DisjointSet::DisjointSet(int)` | solver | 0.68% | 0.00% | -0.68% | 0.25ms | - |
| 11 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | solver | 0.00% | 0.65% | +0.65% | - | 0.24ms |
| 12 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 4.66% | 5.26% | +0.60% | 1.68ms | 1.90ms |
| 13 | `operator new(unsigned long)` | runtime | 2.22% | 2.61% | +0.39% | 0.80ms | 0.95ms |
| 14 | `tsp::BranchBoundSolver::search(...)` | solver | 6.10% | 5.76% | -0.34% | 2.20ms | 2.09ms |
| 15 | `std::_Function_handler<void (int, double, int), tsp::BranchB` | solver | 2.33% | 2.49% | +0.16% | 0.84ms | 0.90ms |
| 16 | `void std::vector<unsigned long, std::allocator<> >::_M_reall` | runtime | 2.21% | 2.36% | +0.15% | 0.80ms | 0.85ms |
| 17 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 0.73% | 0.85% | +0.12% | 0.26ms | 0.31ms |
| 18 | `std::vector<int, std::allocator<> >::~vector()` | runtime | 0.21% | 0.10% | -0.11% | 0.08ms | 0.04ms |
| 19 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | solver | 0.14% | 0.23% | +0.09% | 0.05ms | 0.08ms |
| 20 | `operator delete(void*)` | runtime | 0.28% | 0.33% | +0.05% | 0.10ms | 0.12ms |
| 21 | `operator delete(...)` | runtime | 0.28% | 0.33% | +0.05% | 0.10ms | 0.12ms |
| 22 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-e` | runtime | 1.71% | 1.73% | +0.02% | 0.62ms | 0.63ms |
| 23 | `tsp::BranchBoundSolver::lkSearch(...) const` | solver | 0.21% | 0.22% | +0.01% | 0.08ms | 0.08ms |
| 24 | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |
| 25 | `tsp::BranchBoundSolver::linKernighanImprove(...) const` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |

## New Functions in 0708

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::computeOneTree(...) const` | 27.59% | 37.57% |
| `std::vector<int, std::allocator<> >::_M_fill_assign(...)` | 1.15% | 2.21% |
| `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | 0.65% | 1.77% |

## Removed Functions (was in 0702)

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::updateOneTreeAfterCandidateRemoval(.` | 23.69% | 27.05% |
| `tsp::BranchBoundSolver::updateOneTreeAfterForbid(...) const` | 6.90% | 14.10% |
| `tsp::(anonymous namespace)::DisjointSet::unite(int, int)` | 4.05% | 4.05% |
| `tsp::(anonymous namespace)::DisjointSet::DisjointSet(int)` | 0.68% | 1.81% |

## Summary

- Old version functions: 40
- New version functions: 38
- Solver self%: 68.3% → 63.1% (-5.2%)
- Runtime self%: 25.3% → 30.3% (+4.9%)
