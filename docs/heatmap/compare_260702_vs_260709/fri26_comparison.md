# fri26 (n=26): 260702 vs 260709

## Performance

- Old (260702): 1.937s
- New (260709): 10.453s
- Delta: 8.516s (+439.7%)
- Old Ir: 14,738,129,521, New Ir: 69,541,150,846
- Solver self%: 80.3% → 69.5% (-10.8%)
- Runtime self%: 17.3% → 27.4% (+10.2%)

## Top Changes

| # | Function | Kind | Old Self% | New Self% | Delta |
|---:|---|---|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::` | solver | 21.60% | 0.00% | -21.60% |
| 2 | `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::o` | solver | 0.00% | 18.13% | +18.13% |
| 3 | `tsp::BranchBoundSolver::buildBranchCandidates(...) cons` | solver | 33.04% | 24.67% | -8.37% |
| 4 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 0.00% | 4.53% | +4.53% |
| 5 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 2.04% | 5.86% | +3.82% |
| 6 | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | solver | 1.95% | 0.00% | -1.95% |
| 7 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unalig` | runtime | 2.07% | 3.91% | +1.84% |
| 8 | `void std::vector<tsp::BranchBoundSolver::Edge, std::all` | solver | 2.86% | 1.04% | -1.82% |
| 9 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 4.38% | 5.32% | +0.94% |
| 10 | `tsp::BranchBoundSolver::search(...)::{lambda(...)::Leve` | solver | 0.00% | 0.71% | +0.71% |
| 11 | `./malloc/./malloc/malloc.c:malloc` | runtime | 3.67% | 4.37% | +0.70% |
| 12 | `./malloc/./malloc/malloc.c:malloc_consolidate` | runtime | 0.00% | 0.70% | +0.70% |
| 13 | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | runtime | 0.53% | 1.13% | +0.60% |
| 14 | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | runtime | 0.32% | 0.87% | +0.55% |
| 15 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 20.05% | 19.51% | -0.54% |
| 16 | `./malloc/./malloc/malloc.c:free` | runtime | 2.30% | 2.74% | +0.44% |
| 17 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | solver | 0.00% | 0.39% | +0.39% |
| 18 | `tsp::BranchBoundSolver::search(...)` | solver | 0.82% | 0.53% | -0.29% |
| 19 | `operator new(unsigned long)` | runtime | 1.31% | 1.56% | +0.25% |
| 20 | `operator delete(...)` | runtime | 0.00% | 0.20% | +0.20% |
| 21 | `std::vector<int, std::allocator<> >::_M_fill_assign(...` | runtime | 0.51% | 0.58% | +0.07% |
| 22 | `operator delete(void*)` | runtime | 0.16% | 0.20% | +0.04% |
| 23 | `(anonymous namespace)::solveInput(...)::CliOptions cons` | solver | 0.00% | 0.00% | +0.00% |
| 24 | `tsp::BranchBoundSolver::solve(tsp::BranchStrategy)` | solver | 0.00% | 0.00% | +0.00% |
| 25 | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below mai` | solver | 0.00% | 0.00% | +0.00% |
