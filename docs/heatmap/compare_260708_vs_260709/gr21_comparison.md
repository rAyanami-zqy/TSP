# gr21 (n=21): 260708 vs 260709

## Performance

- Old (260708): 89.6ms
- New (260709): 173.2ms
- Delta: 83.5ms (+93.2%)
- Old Ir: 468,190,922, New Ir: 1,049,585,792
- Solver self%: 68.5% → 67.4% (-1.2%)
- Runtime self%: 27.7% → 29.2% (+1.5%)

## Top Changes

| # | Function | Kind | Old Self% | New Self% | Delta |
|---:|---|---|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::o` | solver | 17.07% | 16.47% | -0.60% |
| 2 | `tsp::BranchBoundSolver::buildBranchCandidates(...) cons` | solver | 22.50% | 21.93% | -0.57% |
| 3 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 6.59% | 7.08% | +0.49% |
| 4 | `std::_Function_handler<void (int, double, int), tsp::Br` | solver | 0.86% | 0.38% | -0.48% |
| 5 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 21.29% | 21.55% | +0.26% |
| 6 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 5.28% | 5.52% | +0.24% |
| 7 | `./malloc/./malloc/malloc.c:malloc` | runtime | 4.29% | 4.47% | +0.18% |
| 8 | `./malloc/./malloc/malloc.c:malloc_consolidate` | runtime | 0.88% | 1.02% | +0.14% |
| 9 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 4.27% | 4.40% | +0.13% |
| 10 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unalig` | runtime | 3.38% | 3.49% | +0.11% |
| 11 | `./malloc/./malloc/malloc.c:free` | runtime | 2.69% | 2.80% | +0.11% |
| 12 | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | runtime | 1.17% | 1.25% | +0.08% |
| 13 | `operator new(unsigned long)` | runtime | 1.54% | 1.60% | +0.06% |
| 14 | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | runtime | 0.96% | 1.02% | +0.06% |
| 15 | `tsp::BranchBoundSolver::search(...)` | solver | 0.51% | 0.55% | +0.04% |
| 16 | `void std::vector<tsp::BranchBoundSolver::Edge, std::all` | solver | 1.00% | 1.02% | +0.02% |
| 17 | `tsp::BranchBoundSolver::search(...)::{lambda(...)::Leve` | solver | 0.67% | 0.69% | +0.02% |
| 18 | `std::vector<int, std::allocator<> >::_M_fill_assign(...` | runtime | 0.53% | 0.54% | +0.01% |
| 19 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | solver | 0.38% | 0.39% | +0.01% |
| 20 | `operator delete(void*)` | runtime | 0.19% | 0.20% | +0.01% |
| 21 | `operator delete(...)` | runtime | 0.19% | 0.20% | +0.01% |
| 22 | `(anonymous namespace)::solveInput(...)::CliOptions cons` | solver | 0.00% | 0.00% | +0.00% |
| 23 | `tsp::BranchBoundSolver::lkSearch(...) const` | solver | 0.00% | 0.00% | +0.00% |
| 24 | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below mai` | solver | 0.00% | 0.00% | +0.00% |
| 25 | `(below main)` | solver | 0.00% | 0.00% | +0.00% |
