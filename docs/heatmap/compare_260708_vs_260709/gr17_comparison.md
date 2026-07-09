# gr17 (n=17): 260708 vs 260709

## Performance

- Old (260708): 2.474s
- New (260709): 3.486s
- Delta: 1.012s (+40.9%)
- Old Ir: 15,290,897,525, New Ir: 21,761,551,205
- Solver self%: 60.5% → 59.1% (-1.3%)
- Runtime self%: 35.3% → 36.5% (+1.2%)

## Top Changes

| # | Function | Kind | Old Self% | New Self% | Delta |
|---:|---|---|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 20.66% | 19.60% | -1.06% |
| 2 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 5.92% | 6.28% | +0.36% |
| 3 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 8.20% | 8.41% | +0.21% |
| 4 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 5.00% | 4.82% | -0.18% |
| 5 | `./malloc/./malloc/malloc.c:malloc` | runtime | 6.70% | 6.87% | +0.17% |
| 6 | `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::o` | solver | 13.57% | 13.42% | -0.15% |
| 7 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unalig` | runtime | 3.80% | 3.93% | +0.13% |
| 8 | `./malloc/./malloc/malloc.c:free` | runtime | 4.18% | 4.29% | +0.11% |
| 9 | `operator new(unsigned long)` | runtime | 2.39% | 2.45% | +0.06% |
| 10 | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | runtime | 1.06% | 1.12% | +0.06% |
| 11 | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | runtime | 0.80% | 0.85% | +0.05% |
| 12 | `void std::vector<tsp::BranchBoundSolver::Edge, std::all` | solver | 1.63% | 1.67% | +0.04% |
| 13 | `tsp::BranchBoundSolver::buildBranchCandidates(...) cons` | solver | 17.10% | 17.07% | -0.03% |
| 14 | `tsp::BranchBoundSolver::search(...)::{lambda(...)::Leve` | solver | 1.11% | 1.14% | +0.03% |
| 15 | `./malloc/./malloc/malloc.c:malloc_consolidate` | runtime | 0.86% | 0.88% | +0.02% |
| 16 | `tsp::BranchBoundSolver::search(...)` | solver | 0.79% | 0.81% | +0.02% |
| 17 | `std::vector<int, std::allocator<> >::_M_fill_assign(...` | runtime | 0.80% | 0.82% | +0.02% |
| 18 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | solver | 0.60% | 0.61% | +0.01% |
| 19 | `operator delete(void*)` | runtime | 0.30% | 0.31% | +0.01% |
| 20 | `operator delete(...)` | runtime | 0.30% | 0.31% | +0.01% |
| 21 | `(anonymous namespace)::solveInput(...)::CliOptions cons` | solver | 0.00% | 0.00% | +0.00% |
| 22 | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below mai` | solver | 0.00% | 0.00% | +0.00% |
| 23 | `(below main)` | solver | 0.00% | 0.00% | +0.00% |
| 24 | `std::_Function_handler<void (int), tsp::BranchBoundSolv` | solver | 0.00% | 0.00% | +0.00% |
| 25 | `main` | solver | 0.00% | 0.00% | +0.00% |
