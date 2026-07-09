# gr24 (n=24): 260708 vs 260709

## Performance

- Old (260708): 1.256s
- New (260709): 2.313s
- Delta: 1.057s (+84.1%)
- Old Ir: 8,375,907,163, New Ir: 15,566,725,552
- Solver self%: 72.4% → 71.9% (-0.5%)
- Runtime self%: 24.7% → 25.2% (+0.5%)

## Top Changes

| # | Function | Kind | Old Self% | New Self% | Delta |
|---:|---|---|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 18.74% | 18.24% | -0.50% |
| 2 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 5.28% | 5.62% | +0.34% |
| 3 | `tsp::BranchBoundSolver::buildBranchCandidates(...) cons` | solver | 26.81% | 27.00% | +0.19% |
| 4 | `operator delete(...)` | runtime | 0.18% | 0.00% | -0.18% |
| 5 | `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::o` | solver | 20.03% | 19.88% | -0.15% |
| 6 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 4.44% | 4.33% | -0.11% |
| 7 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unalig` | runtime | 3.61% | 3.69% | +0.08% |
| 8 | `./malloc/./malloc/malloc.c:malloc_consolidate` | runtime | 0.60% | 0.65% | +0.05% |
| 9 | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | runtime | 0.79% | 0.84% | +0.05% |
| 10 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 4.75% | 4.79% | +0.04% |
| 11 | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | runtime | 1.06% | 1.10% | +0.04% |
| 12 | `./malloc/./malloc/malloc.c:malloc` | runtime | 3.91% | 3.94% | +0.03% |
| 13 | `./malloc/./malloc/malloc.c:free` | runtime | 2.45% | 2.47% | +0.02% |
| 14 | `operator new(unsigned long)` | runtime | 1.40% | 1.41% | +0.01% |
| 15 | `tsp::BranchBoundSolver::search(...)::{lambda(...)::Leve` | solver | 0.63% | 0.64% | +0.01% |
| 16 | `tsp::BranchBoundSolver::search(...)` | solver | 0.47% | 0.48% | +0.01% |
| 17 | `std::vector<int, std::allocator<> >::_M_fill_assign(...` | runtime | 0.46% | 0.47% | +0.01% |
| 18 | `void std::vector<tsp::BranchBoundSolver::Edge, std::all` | solver | 0.93% | 0.94% | +0.01% |
| 19 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | solver | 0.35% | 0.35% | +0.00% |
| 20 | `operator delete(void*)` | runtime | 0.18% | 0.18% | +0.00% |
| 21 | `(anonymous namespace)::solveInput(...)::CliOptions cons` | solver | 0.00% | 0.00% | +0.00% |
| 22 | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below mai` | solver | 0.00% | 0.00% | +0.00% |
| 23 | `(below main)` | solver | 0.00% | 0.00% | +0.00% |
| 24 | `std::_Function_handler<void (int), tsp::BranchBoundSolv` | solver | 0.00% | 0.00% | +0.00% |
| 25 | `main` | solver | 0.00% | 0.00% | +0.00% |
