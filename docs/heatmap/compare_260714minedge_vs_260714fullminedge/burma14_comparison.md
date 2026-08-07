# Heatmap Comparison: burma14 (n=14)

## Overall Performance

| Metric | Old (0702) | New (0708) | Delta |
|---|---|---|---|
| Native elapsed | 55.7ms | 45.8ms | -9936.7us (-17.8%) |
| `cost` | `3323` | `3323` | |
| `initial_upper_bound` | `3323` | `3323` | |
| `method` | `exact` | `exact` | |
| `nodes_created` | `18362` | `18410` | |
| `nodes_expanded` | `9152` | `9217` | |
| `pruned_bound` | `9210` | `9193` | |
| `pruned_infeasible` | `28` | `28` | |
| `root_lower_bound` | `2542` | `2542` | |

## Top Changes by Self Time %

| # | Function | Kind | Old Self% | New Self% | Delta | Old ms | New ms |
|---:|---|---|---:|---:|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 0.00% | 27.36% | +27.36% | - | 12.52ms |
| 2 | `tsp::BranchBoundSolver::updateOneTreeAfterCandidateRemoval(.` | solver | 24.57% | 0.00% | -24.57% | 13.69ms | - |
| 3 | `tsp::BranchBoundSolver::updateOneTreeAfterForbid(...) const` | solver | 6.74% | 0.00% | -6.74% | 3.75ms | - |
| 4 | `tsp::(anonymous namespace)::DisjointSet::unite(int, int)` | solver | 4.91% | 0.00% | -4.91% | 2.74ms | - |
| 5 | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | solver | 18.87% | 20.83% | +1.96% | 10.51ms | 9.53ms |
| 6 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 7.95% | 9.57% | +1.62% | 4.43ms | 4.38ms |
| 7 | `./malloc/./malloc/malloc.c:malloc` | runtime | 6.51% | 7.85% | +1.34% | 3.63ms | 3.59ms |
| 8 | `std::vector<int, std::allocator<> >::_M_fill_assign(...)` | runtime | 0.00% | 1.26% | +1.26% | - | 0.58ms |
| 9 | `./malloc/./malloc/malloc.c:free` | runtime | 4.07% | 4.91% | +0.84% | 2.27ms | 2.25ms |
| 10 | `tsp::(anonymous namespace)::DisjointSet::DisjointSet(int)` | solver | 0.82% | 0.00% | -0.82% | 0.46ms | - |
| 11 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | solver | 0.00% | 0.71% | +0.71% | - | 0.32ms |
| 12 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 4.57% | 5.20% | +0.63% | 2.55ms | 2.38ms |
| 13 | `operator new(unsigned long)` | runtime | 2.33% | 2.80% | +0.47% | 1.30ms | 1.28ms |
| 14 | `void std::vector<unsigned long, std::allocator<> >::_M_reall` | runtime | 2.41% | 2.66% | +0.25% | 1.34ms | 1.22ms |
| 15 | `tsp::BranchBoundSolver::search(...)` | solver | 6.30% | 6.14% | -0.16% | 3.51ms | 2.81ms |
| 16 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 0.66% | 0.79% | +0.13% | 0.37ms | 0.36ms |
| 17 | `std::_Function_handler<void (int, double, int), tsp::BranchB` | solver | 1.21% | 1.33% | +0.12% | 0.67ms | 0.61ms |
| 18 | `std::vector<int, std::allocator<> >::~vector()` | runtime | 0.24% | 0.12% | -0.12% | 0.13ms | 0.05ms |
| 19 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | solver | 0.15% | 0.25% | +0.10% | 0.08ms | 0.11ms |
| 20 | `./malloc/./malloc/arena.c:free` | runtime | 0.44% | 0.53% | +0.09% | 0.25ms | 0.24ms |
| 21 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-e` | runtime | 1.72% | 1.79% | +0.07% | 0.96ms | 0.82ms |
| 22 | `operator delete(...)` | runtime | 0.29% | 0.35% | +0.06% | 0.16ms | 0.16ms |
| 23 | `operator delete(void*)` | runtime | 0.29% | 0.35% | +0.06% | 0.16ms | 0.16ms |
| 24 | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-er` | runtime | 0.52% | 0.54% | +0.02% | 0.29ms | 0.25ms |
| 25 | `tsp::BranchBoundSolver::lkSearch(...) const` | solver | 0.11% | 0.12% | +0.01% | 0.06ms | 0.05ms |

## New Functions in 0708

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::computeOneTree(...) const` | 27.36% | 38.26% |
| `std::vector<int, std::allocator<> >::_M_fill_assign(...)` | 1.26% | 2.41% |
| `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | 0.71% | 1.98% |

## Removed Functions (was in 0702)

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::updateOneTreeAfterCandidateRemoval(.` | 24.57% | 27.99% |
| `tsp::BranchBoundSolver::updateOneTreeAfterForbid(...) const` | 6.74% | 15.48% |
| `tsp::(anonymous namespace)::DisjointSet::unite(int, int)` | 4.91% | 4.91% |
| `tsp::(anonymous namespace)::DisjointSet::DisjointSet(int)` | 0.82% | 2.19% |

## Summary

- Old version functions: 38
- New version functions: 36
- Solver self%: 68.2% → 61.9% (-6.3%)
- Runtime self%: 27.4% → 33.5% (+6.1%)
