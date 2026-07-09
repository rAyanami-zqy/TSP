# gr17 (n=17): 260702 vs 260709

## Performance

- Old (260702): 2.210s
- New (260709): 3.486s
- Delta: 1.276s (+57.7%)
- Old Ir: 14,216,783,384, New Ir: 21,761,551,205
- Solver self%: 68.7% → 59.1% (-9.5%)
- Runtime self%: 27.7% → 36.5% (+8.8%)

## Top Changes

| # | Function | Kind | Old Self% | New Self% | Delta |
|---:|---|---|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::` | solver | 16.01% | 0.00% | -16.01% |
| 2 | `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::o` | solver | 0.00% | 13.42% | +13.42% |
| 3 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 0.00% | 4.82% | +4.82% |
| 4 | `tsp::BranchBoundSolver::buildBranchCandidates(...) cons` | solver | 21.82% | 17.07% | -4.75% |
| 5 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 1.77% | 6.28% | +4.51% |
| 6 | `void std::vector<tsp::BranchBoundSolver::Edge, std::all` | solver | 4.44% | 1.67% | -2.77% |
| 7 | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | solver | 2.75% | 0.00% | -2.75% |
| 8 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 22.05% | 19.60% | -2.45% |
| 9 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unalig` | runtime | 2.42% | 3.93% | +1.51% |
| 10 | `tsp::BranchBoundSolver::search(...)::{lambda(...)::Leve` | solver | 0.00% | 1.14% | +1.14% |
| 11 | `./malloc/./malloc/malloc.c:malloc_consolidate` | runtime | 0.00% | 0.88% | +0.88% |
| 12 | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | runtime | 0.00% | 0.85% | +0.85% |
| 13 | `tsp::BranchBoundSolver::search(...)` | solver | 1.58% | 0.81% | -0.77% |
| 14 | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | runtime | 0.45% | 1.12% | +0.67% |
| 15 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | solver | 0.00% | 0.61% | +0.61% |
| 16 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 8.14% | 8.41% | +0.27% |
| 17 | `./malloc/./malloc/malloc.c:malloc` | runtime | 6.79% | 6.87% | +0.08% |
| 18 | `./malloc/./malloc/malloc.c:free` | runtime | 4.23% | 4.29% | +0.06% |
| 19 | `std::vector<int, std::allocator<> >::_M_fill_assign(...` | runtime | 0.86% | 0.82% | -0.04% |
| 20 | `operator new(unsigned long)` | runtime | 2.42% | 2.45% | +0.03% |
| 21 | `operator delete(void*)` | runtime | 0.30% | 0.31% | +0.01% |
| 22 | `operator delete(...)` | runtime | 0.30% | 0.31% | +0.01% |
| 23 | `(anonymous namespace)::solveInput(...)::CliOptions cons` | solver | 0.00% | 0.00% | +0.00% |
| 24 | `tsp::BranchBoundSolver::solve(tsp::BranchStrategy)` | solver | 0.00% | 0.00% | +0.00% |
| 25 | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below mai` | solver | 0.00% | 0.00% | +0.00% |
