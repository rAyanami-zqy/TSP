# burma14 (n=14): 260702 vs 260709

## Performance

- Old (260702): 38.3ms
- New (260709): 58.3ms
- Delta: 20.0ms (+52.4%)
- Old Ir: 125,968,557, New Ir: 258,068,469
- Solver self%: 64.7% → 55.7% (-9.0%)
- Runtime self%: 29.7% → 38.9% (+9.3%)

## Top Changes

| # | Function | Kind | Old Self% | New Self% | Delta |
|---:|---|---|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::` | solver | 14.04% | 0.00% | -14.04% |
| 2 | `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::o` | solver | 0.00% | 11.26% | +11.26% |
| 3 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 0.00% | 4.85% | +4.85% |
| 4 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 0.85% | 5.31% | +4.46% |
| 5 | `tsp::BranchBoundSolver::buildBranchCandidates(...) cons` | solver | 18.14% | 13.93% | -4.21% |
| 6 | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | solver | 2.73% | 0.00% | -2.73% |
| 7 | `void std::vector<tsp::BranchBoundSolver::Edge, std::all` | solver | 4.54% | 1.96% | -2.58% |
| 8 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 21.12% | 19.37% | -1.75% |
| 9 | `tsp::BranchBoundSolver::search(...)::{lambda(...)::Leve` | solver | 0.00% | 1.34% | +1.34% |
| 10 | `std::_Function_handler<void (int, double, int), tsp::Br` | solver | 2.20% | 1.08% | -1.12% |
| 11 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unalig` | runtime | 1.65% | 2.70% | +1.05% |
| 12 | `./malloc/./malloc/malloc.c:malloc_consolidate` | runtime | 0.00% | 0.93% | +0.93% |
| 13 | `tsp::BranchBoundSolver::search(...)` | solver | 1.77% | 0.95% | -0.82% |
| 14 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | solver | 0.00% | 0.71% | +0.71% |
| 15 | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | runtime | 0.20% | 0.88% | +0.68% |
| 16 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 9.39% | 10.05% | +0.66% |
| 17 | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | runtime | 0.00% | 0.63% | +0.63% |
| 18 | `./malloc/./malloc/malloc.c:malloc` | runtime | 7.75% | 8.16% | +0.41% |
| 19 | `./malloc/./malloc/malloc.c:free` | runtime | 4.83% | 5.09% | +0.26% |
| 20 | `std::vector<tsp::BranchBoundSolver::OneTree, std::alloc` | solver | 0.00% | 0.18% | +0.18% |
| 21 | `operator new(unsigned long)` | runtime | 2.76% | 2.91% | +0.15% |
| 22 | `tsp::BranchBoundSolver::lkSearch(...) const` | solver | 0.20% | 0.09% | -0.11% |
| 23 | `./malloc/./malloc/arena.c:free` | runtime | 0.52% | 0.55% | +0.03% |
| 24 | `std::vector<int, std::allocator<> >::_M_fill_assign(...` | runtime | 1.02% | 1.01% | -0.01% |
| 25 | `operator delete(void*)` | runtime | 0.35% | 0.36% | +0.01% |
