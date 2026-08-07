# Heatmap Comparison: burma14 (n=14)

## Overall Performance

| Metric | Old (0702) | New (0708) | Delta |
|---|---|---|---|
| Native elapsed | 35.2ms | 34.3ms | -889.0us (-2.5%) |
| `cost` | `3323` | `3323` | |
| `initial_upper_bound` | `3323` | `3323` | |
| `method` | `exact` | `exact` | |
| `nodes_created` | `18111` | `18111` | |
| `nodes_expanded` | `9162` | `9162` | |
| `pruned_bound` | `8949` | `8949` | |
| `pruned_infeasible` | `214` | `214` | |
| `root_lower_bound` | `2542` | `2542` | |

## Top Changes by Self Time %

| # | Function | Kind | Old Self% | New Self% | Delta | Old ms | New ms |
|---:|---|---|---:|---:|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 0.00% | 26.99% | +26.99% | - | 9.27ms |
| 2 | `tsp::BranchBoundSolver::updateOneTreeAfterCandidateRemoval(.` | solver | 22.64% | 0.00% | -22.64% | 7.98ms | - |
| 3 | `tsp::BranchBoundSolver::updateOneTreeAfterForbid(...) const` | solver | 6.37% | 0.00% | -6.37% | 2.24ms | - |
| 4 | `tsp::(anonymous namespace)::DisjointSet::unite(int, int)` | solver | 3.87% | 0.00% | -3.87% | 1.36ms | - |
| 5 | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | solver | 18.65% | 20.67% | +2.02% | 6.57ms | 7.10ms |
| 6 | `std::vector<int, std::allocator<> >::_M_fill_assign(...)` | runtime | 0.00% | 1.14% | +1.14% | - | 0.39ms |
| 7 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 7.33% | 8.06% | +0.73% | 2.58ms | 2.77ms |
| 8 | `tsp::(anonymous namespace)::DisjointSet::DisjointSet(int)` | solver | 0.65% | 0.00% | -0.65% | 0.23ms | - |
| 9 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | solver | 0.00% | 0.64% | +0.64% | - | 0.22ms |
| 10 | `std::vector<int, std::allocator<> >::operator=(...)` | runtime | 0.62% | 0.00% | -0.62% | 0.22ms | - |
| 11 | `./malloc/./malloc/malloc.c:malloc` | runtime | 5.91% | 6.49% | +0.58% | 2.08ms | 2.23ms |
| 12 | `./malloc/./malloc/malloc.c:free` | runtime | 3.70% | 4.06% | +0.36% | 1.30ms | 1.39ms |
| 13 | `tsp::BranchBoundSolver::chooseSmartBranchEdge(...) const` | solver | 2.36% | 2.62% | +0.26% | 0.83ms | 0.90ms |
| 14 | `std::_Function_handler<void (int, double, int), tsp::BranchB` | solver | 2.23% | 2.47% | +0.24% | 0.79ms | 0.85ms |
| 15 | `void std::vector<unsigned long, std::allocator<> >::_M_reall` | runtime | 2.09% | 2.32% | +0.23% | 0.74ms | 0.80ms |
| 16 | `operator new(unsigned long)` | runtime | 2.11% | 2.32% | +0.21% | 0.74ms | 0.80ms |
| 17 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | solver | 0.27% | 0.46% | +0.19% | 0.10ms | 0.16ms |
| 18 | `tsp::BranchBoundSolver::searchSmart(...)` | solver | 12.04% | 11.92% | -0.12% | 4.24ms | 4.09ms |
| 19 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 1.01% | 1.11% | +0.10% | 0.36ms | 0.38ms |
| 20 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-e` | runtime | 1.34% | 1.30% | -0.04% | 0.47ms | 0.45ms |
| 21 | `operator delete(...)` | runtime | 0.26% | 0.29% | +0.03% | 0.09ms | 0.10ms |
| 22 | `operator delete(void*)` | runtime | 0.26% | 0.29% | +0.03% | 0.09ms | 0.10ms |
| 23 | `tsp::BranchBoundSolver::lkSearch(...) const` | solver | 0.20% | 0.22% | +0.02% | 0.07ms | 0.08ms |
| 24 | `std::vector<int, std::allocator<> >::~vector()` | runtime | 0.31% | 0.32% | +0.01% | 0.11ms | 0.11ms |
| 25 | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |

## New Functions in 0708

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::computeOneTree(...) const` | 26.99% | 37.25% |
| `std::vector<int, std::allocator<> >::_M_fill_assign(...)` | 1.14% | 2.19% |
| `std::vector<tsp::BranchBoundSolver::Edge, std::allocator<> >` | 0.64% | 1.84% |

## Removed Functions (was in 0702)

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::updateOneTreeAfterCandidateRemoval(.` | 22.64% | 25.97% |
| `tsp::BranchBoundSolver::updateOneTreeAfterForbid(...) const` | 6.37% | 13.38% |
| `tsp::(anonymous namespace)::DisjointSet::unite(int, int)` | 3.87% | 3.87% |
| `tsp::(anonymous namespace)::DisjointSet::DisjointSet(int)` | 0.65% | 1.86% |
| `std::vector<int, std::allocator<> >::operator=(...)` | 0.62% | 1.77% |

## Summary

- Old version functions: 39
- New version functions: 37
- Solver self%: 69.3% → 66.0% (-3.3%)
- Runtime self%: 24.9% → 27.7% (+2.8%)
