# gr21 (n=21): 260702 vs 260709

## Performance

- Old (260702): 80.6ms
- New (260709): 173.2ms
- Delta: 92.5ms (+114.8%)
- Old Ir: 437,536,663, New Ir: 1,049,585,792
- Solver self%: 76.8% → 67.4% (-9.4%)
- Runtime self%: 20.2% → 29.2% (+9.0%)

## Top Changes

| # | Function | Kind | Old Self% | New Self% | Delta |
|---:|---|---|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::` | solver | 19.18% | 0.00% | -19.18% |
| 2 | `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::o` | solver | 0.00% | 16.47% | +16.47% |
| 3 | `tsp::BranchBoundSolver::buildBranchCandidates(...) cons` | solver | 28.51% | 21.93% | -6.58% |
| 4 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 0.00% | 4.40% | +4.40% |
| 5 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 2.69% | 7.08% | +4.39% |
| 6 | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | solver | 1.97% | 0.00% | -1.97% |
| 7 | `void std::vector<tsp::BranchBoundSolver::Edge, std::all` | solver | 2.71% | 1.02% | -1.69% |
| 8 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unalig` | runtime | 2.01% | 3.49% | +1.48% |
| 9 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 22.50% | 21.55% | -0.95% |
| 10 | `./malloc/./malloc/malloc.c:malloc_consolidate` | runtime | 0.21% | 1.02% | +0.81% |
| 11 | `tsp::BranchBoundSolver::search(...)::{lambda(...)::Leve` | solver | 0.00% | 0.69% | +0.69% |
| 12 | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | runtime | 0.64% | 1.25% | +0.61% |
| 13 | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | runtime | 0.42% | 1.02% | +0.60% |
| 14 | `std::_Function_handler<void (int, double, int), tsp::Br` | solver | 0.92% | 0.38% | -0.54% |
| 15 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 4.99% | 5.52% | +0.53% |
| 16 | `tsp::BranchBoundSolver::search(...)` | solver | 0.97% | 0.55% | -0.42% |
| 17 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | solver | 0.00% | 0.39% | +0.39% |
| 18 | `./malloc/./malloc/malloc.c:malloc` | runtime | 4.17% | 4.47% | +0.30% |
| 19 | `./malloc/./malloc/malloc.c:free` | runtime | 2.61% | 2.80% | +0.19% |
| 20 | `operator new(unsigned long)` | runtime | 1.49% | 1.60% | +0.11% |
| 21 | `std::vector<int, std::allocator<> >::_M_fill_assign(...` | runtime | 0.56% | 0.54% | -0.02% |
| 22 | `operator delete(void*)` | runtime | 0.19% | 0.20% | +0.01% |
| 23 | `operator delete(...)` | runtime | 0.19% | 0.20% | +0.01% |
| 24 | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | solver | 0.00% | 0.00% | +0.00% |
| 25 | `(anonymous namespace)::solveInput(...)::CliOptions cons` | solver | 0.00% | 0.00% | +0.00% |
