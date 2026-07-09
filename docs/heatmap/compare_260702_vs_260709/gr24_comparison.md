# gr24 (n=24): 260702 vs 260709

## Performance

- Old (260702): 1.126s
- New (260709): 2.313s
- Delta: 1.187s (+105.4%)
- Old Ir: 8,119,979,565, New Ir: 15,566,725,552
- Solver self%: 79.2% → 71.9% (-7.3%)
- Runtime self%: 18.5% → 25.2% (+6.7%)

## Top Changes

| # | Function | Kind | Old Self% | New Self% | Delta |
|---:|---|---|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::` | solver | 21.57% | 0.00% | -21.57% |
| 2 | `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::o` | solver | 0.00% | 19.88% | +19.88% |
| 3 | `tsp::BranchBoundSolver::buildBranchCandidates(...) cons` | solver | 32.82% | 27.00% | -5.82% |
| 4 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 0.00% | 4.33% | +4.33% |
| 5 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 2.46% | 5.62% | +3.16% |
| 6 | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | solver | 1.95% | 0.00% | -1.95% |
| 7 | `void std::vector<tsp::BranchBoundSolver::Edge, std::all` | solver | 2.82% | 0.94% | -1.88% |
| 8 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unalig` | runtime | 2.06% | 3.69% | +1.63% |
| 9 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 19.13% | 18.24% | -0.89% |
| 10 | `tsp::BranchBoundSolver::search(...)::{lambda(...)::Leve` | solver | 0.00% | 0.64% | +0.64% |
| 11 | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | runtime | 0.57% | 1.10% | +0.53% |
| 12 | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | runtime | 0.38% | 0.84% | +0.46% |
| 13 | `./malloc/./malloc/malloc.c:malloc_consolidate` | runtime | 0.25% | 0.65% | +0.40% |
| 14 | `tsp::BranchBoundSolver::search(...)` | solver | 0.87% | 0.48% | -0.39% |
| 15 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | solver | 0.00% | 0.35% | +0.35% |
| 16 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 4.56% | 4.79% | +0.23% |
| 17 | `./malloc/./malloc/malloc.c:malloc` | runtime | 3.81% | 3.94% | +0.13% |
| 18 | `./malloc/./malloc/malloc.c:free` | runtime | 2.38% | 2.47% | +0.09% |
| 19 | `operator new(unsigned long)` | runtime | 1.36% | 1.41% | +0.05% |
| 20 | `std::vector<int, std::allocator<> >::_M_fill_assign(...` | runtime | 0.48% | 0.47% | -0.01% |
| 21 | `operator delete(void*)` | runtime | 0.17% | 0.18% | +0.01% |
| 22 | `(anonymous namespace)::solveInput(...)::CliOptions cons` | solver | 0.00% | 0.00% | +0.00% |
| 23 | `tsp::BranchBoundSolver::solve(tsp::BranchStrategy)` | solver | 0.00% | 0.00% | +0.00% |
| 24 | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below mai` | solver | 0.00% | 0.00% | +0.00% |
| 25 | `(below main)` | solver | 0.00% | 0.00% | +0.00% |
