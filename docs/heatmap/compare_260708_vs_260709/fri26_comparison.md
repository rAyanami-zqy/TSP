# fri26 (n=26): 260708 vs 260709

## Performance

- Old (260708): 2.198s
- New (260709): 10.453s
- Delta: 8.255s (+375.6%)
- Old Ir: 15,174,385,039, New Ir: 69,541,150,846
- Solver self%: 73.5% → 69.5% (-4.0%)
- Runtime self%: 23.6% → 27.4% (+3.9%)

## Top Changes

| # | Function | Kind | Old Self% | New Self% | Delta |
|---:|---|---|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::buildBranchCandidates(...) cons` | solver | 27.01% | 24.67% | -2.34% |
| 2 | `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::o` | solver | 20.11% | 18.13% | -1.98% |
| 3 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 4.49% | 5.32% | +0.83% |
| 4 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 5.11% | 5.86% | +0.75% |
| 5 | `./malloc/./malloc/malloc.c:malloc` | runtime | 3.69% | 4.37% | +0.68% |
| 6 | `./malloc/./malloc/malloc.c:free` | runtime | 2.31% | 2.74% | +0.43% |
| 7 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unalig` | runtime | 3.62% | 3.91% | +0.29% |
| 8 | `operator new(unsigned long)` | runtime | 1.32% | 1.56% | +0.24% |
| 9 | `operator delete(...)` | runtime | 0.00% | 0.20% | +0.20% |
| 10 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 19.70% | 19.51% | -0.19% |
| 11 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 4.38% | 4.53% | +0.15% |
| 12 | `void std::vector<tsp::BranchBoundSolver::Edge, std::all` | solver | 0.89% | 1.04% | +0.15% |
| 13 | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | runtime | 1.00% | 1.13% | +0.13% |
| 14 | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | runtime | 0.77% | 0.87% | +0.10% |
| 15 | `tsp::BranchBoundSolver::search(...)::{lambda(...)::Leve` | solver | 0.61% | 0.71% | +0.10% |
| 16 | `tsp::BranchBoundSolver::search(...)` | solver | 0.44% | 0.53% | +0.09% |
| 17 | `std::vector<int, std::allocator<> >::_M_fill_assign(...` | runtime | 0.49% | 0.58% | +0.09% |
| 18 | `./malloc/./malloc/malloc.c:malloc_consolidate` | runtime | 0.62% | 0.70% | +0.08% |
| 19 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | solver | 0.33% | 0.39% | +0.06% |
| 20 | `operator delete(void*)` | runtime | 0.17% | 0.20% | +0.03% |
| 21 | `(anonymous namespace)::solveInput(...)::CliOptions cons` | solver | 0.00% | 0.00% | +0.00% |
| 22 | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below mai` | solver | 0.00% | 0.00% | +0.00% |
| 23 | `(below main)` | solver | 0.00% | 0.00% | +0.00% |
| 24 | `std::_Function_handler<void (int), tsp::BranchBoundSolv` | solver | 0.00% | 0.00% | +0.00% |
| 25 | `main` | solver | 0.00% | 0.00% | +0.00% |
