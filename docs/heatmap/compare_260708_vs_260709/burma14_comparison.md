# burma14 (n=14): 260708 vs 260709

## Performance

- Old (260708): 40.7ms
- New (260709): 58.3ms
- Delta: 17.6ms (+43.2%)
- Old Ir: 137,241,840, New Ir: 258,068,469
- Solver self%: 57.0% → 55.7% (-1.3%)
- Runtime self%: 36.8% → 38.9% (+2.1%)

## Top Changes

| # | Function | Kind | Old Self% | New Self% | Delta |
|---:|---|---|---:|---:|---:|
| 1 | `std::_Function_handler<void (int, double, int), tsp::Br` | solver | 2.02% | 1.08% | -0.94% |
| 2 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 9.47% | 10.05% | +0.58% |
| 3 | `./malloc/./malloc/malloc.c:malloc` | runtime | 7.68% | 8.16% | +0.48% |
| 4 | `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::o` | solver | 11.56% | 11.26% | -0.30% |
| 5 | `./malloc/./malloc/malloc.c:free` | runtime | 4.79% | 5.09% | +0.30% |
| 6 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 5.08% | 5.31% | +0.23% |
| 7 | `operator new(unsigned long)` | runtime | 2.74% | 2.91% | +0.17% |
| 8 | `void std::vector<tsp::BranchBoundSolver::Edge, std::all` | solver | 1.81% | 1.96% | +0.15% |
| 9 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 19.51% | 19.37% | -0.14% |
| 10 | `tsp::BranchBoundSolver::buildBranchCandidates(...) cons` | solver | 14.06% | 13.93% | -0.13% |
| 11 | `tsp::BranchBoundSolver::search(...)::{lambda(...)::Leve` | solver | 1.22% | 1.34% | +0.12% |
| 12 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unalig` | runtime | 2.59% | 2.70% | +0.11% |
| 13 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 4.94% | 4.85% | -0.09% |
| 14 | `tsp::BranchBoundSolver::lkSearch(...) const` | solver | 0.18% | 0.09% | -0.09% |
| 15 | `std::vector<int, std::allocator<> >::_M_fill_assign(...` | runtime | 0.94% | 1.01% | +0.07% |
| 16 | `tsp::BranchBoundSolver::search(...)` | solver | 0.88% | 0.95% | +0.07% |
| 17 | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | runtime | 0.58% | 0.63% | +0.05% |
| 18 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | solver | 0.66% | 0.71% | +0.05% |
| 19 | `./malloc/./malloc/malloc.c:malloc_consolidate` | runtime | 0.89% | 0.93% | +0.04% |
| 20 | `./malloc/./malloc/arena.c:free` | runtime | 0.51% | 0.55% | +0.04% |
| 21 | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | runtime | 0.86% | 0.88% | +0.02% |
| 22 | `std::vector<tsp::BranchBoundSolver::OneTree, std::alloc` | solver | 0.16% | 0.18% | +0.02% |
| 23 | `operator delete(void*)` | runtime | 0.34% | 0.36% | +0.02% |
| 24 | `operator delete(...)` | runtime | 0.34% | 0.36% | +0.02% |
| 25 | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below mai` | solver | 0.00% | 0.00% | +0.00% |
