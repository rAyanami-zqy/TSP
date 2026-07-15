# Heatmap Comparison: fri26 (n=26)

## Overall Performance

| Metric | Old (0702) | New (0708) | Delta |
|---|---|---|---|
| Native elapsed | 810.1ms | 1.552s | 742.2ms (+91.6%) |
| `cost` | `937` | `937` | |
| `initial_upper_bound` | `937` | `937` | |
| `method` | `exact` | `exact` | |
| `nodes_created` | `195433` | `565572` | |
| `nodes_expanded` | `95295` | `296513` | |
| `pruned_bound` | `100138` | `269059` | |
| `pruned_infeasible` | `33` | `73` | |
| `root_lower_bound` | `824` | `824` | |

## Top Changes by Self Time %

| # | Function | Kind | Old Self% | New Self% | Delta | Old ms | New ms |
|---:|---|---|---:|---:|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | solver | 31.92% | 0.00% | -31.92% | 258.58ms | - |
| 2 | `tsp::BranchBoundSolver::findMstReplacement(...) const` | solver | 0.00% | 30.27% | +30.27% | - | 469.89ms |
| 3 | `tsp::BranchBoundSolver::updateOneTreeAfterForbid(...) const` | solver | 8.44% | 0.90% | -7.54% | 68.37ms | 13.97ms |
| 4 | `tsp::BranchBoundSolver::deactivateCandidate(...) const` | solver | 0.00% | 6.58% | +6.58% | - | 102.14ms |
| 5 | `tsp::BranchBoundSolver::deactivateCandidatesAfterForce(...) ` | solver | 0.00% | 4.84% | +4.84% | - | 75.13ms |
| 6 | `tsp::BranchBoundSolver::restoreCandidates(...) const` | solver | 0.00% | 4.42% | +4.42% | - | 68.61ms |
| 7 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 2.18% | 4.01% | +1.83% | 17.66ms | 62.25ms |
| 8 | `tsp::BranchBoundSolver::updateOneTreeAfterCandidateRemoval(.` | solver | 35.36% | 33.79% | -1.57% | 286.44ms | 524.53ms |
| 9 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-e` | runtime | 1.98% | 0.41% | -1.57% | 16.04ms | 6.36ms |
| 10 | `void std::vector<unsigned long, std::allocator<> >::_M_reall` | runtime | 1.32% | 0.00% | -1.32% | 10.69ms | - |
| 11 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 1.42% | 0.44% | -0.98% | 11.50ms | 6.83ms |
| 12 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 3.42% | 2.58% | -0.84% | 27.70ms | 40.05ms |
| 13 | `./malloc/./malloc/malloc.c:malloc` | runtime | 2.85% | 2.09% | -0.76% | 23.09ms | 32.44ms |
| 14 | `tsp::BranchBoundSolver::replaceOneTreeEdge(...) const` | solver | 0.00% | 0.60% | +0.60% | - | 9.31ms |
| 15 | `./malloc/./malloc/malloc.c:free` | runtime | 1.79% | 1.30% | -0.49% | 14.50ms | 20.18ms |
| 16 | `std::vector<unsigned long, std::allocator<> >::reserve(unsig` | runtime | 0.00% | 0.31% | +0.31% | - | 4.81ms |
| 17 | `tsp::BranchBoundSolver::search(...)` | solver | 4.87% | 4.57% | -0.30% | 39.45ms | 70.94ms |
| 18 | `operator new(unsigned long)` | runtime | 1.02% | 0.74% | -0.28% | 8.26ms | 11.49ms |
| 19 | `operator delete(void*)` | runtime | 0.13% | 0.00% | -0.13% | 1.05ms | 0.00ms |
| 20 | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |
| 21 | `operator delete(...)` | runtime | 0.00% | 0.00% | +0.00% | - | 0.00ms |
| 22 | `(below main)` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |
| 23 | `main` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |
| 24 | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | solver | 0.00% | 0.00% | +0.00% | - | 0.00ms |
| 25 | `std::vector<int, std::allocator<> >::~vector()` | runtime | 0.00% | 0.00% | +0.00% | - | - |

## New Functions in 0708

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::findMstReplacement(...) const` | 30.27% | 33.61% |
| `tsp::BranchBoundSolver::deactivateCandidate(...) const` | 6.58% | 6.58% |
| `tsp::BranchBoundSolver::deactivateCandidatesAfterForce(...) ` | 4.84% | 11.30% |
| `tsp::BranchBoundSolver::restoreCandidates(...) const` | 4.42% | 4.42% |
| `tsp::BranchBoundSolver::replaceOneTreeEdge(...) const` | 0.60% | 0.60% |
| `std::vector<unsigned long, std::allocator<> >::reserve(unsig` | 0.31% | 1.05% |

## Removed Functions (was in 0702)

| Function | Self% | Incl% |
|---|---|---:|
| `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 31.92% | 36.92% |
| `void std::vector<unsigned long, std::allocator<> >::_M_reall` | 1.32% | 4.35% |
| `operator delete(void*)` | 0.13% | 6.20% |

## Summary

- Old version functions: 28
- New version functions: 29
- Solver self%: 82.8% → 90.0% (+7.2%)
- Runtime self%: 13.9% → 7.9% (-6.1%)
