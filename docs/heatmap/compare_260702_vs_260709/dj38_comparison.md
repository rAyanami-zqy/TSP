# dj38 (n=38): 260702 vs 260709

## Performance

- Old (260702): 113.7ms
- New (260709): 8.948s
- Delta: 8.834s (+7767.1%)
- Old Ir: 870,679,889, New Ir: 67,723,820,524
- Solver self%: 85.3% → 76.4% (-8.9%)
- Runtime self%: 12.6% → 20.7% (+8.1%)

## Top Changes

| # | Function | Kind | Old Self% | New Self% | Delta |
|---:|---|---|---:|---:|---:|
| 1 | `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::` | solver | 23.34% | 0.00% | -23.34% |
| 2 | `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::o` | solver | 0.00% | 22.21% | +22.21% |
| 3 | `tsp::BranchBoundSolver::buildBranchCandidates(...) cons` | solver | 37.23% | 30.81% | -6.42% |
| 4 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 0.00% | 4.04% | +4.04% |
| 5 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 1.78% | 5.43% | +3.65% |
| 6 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 20.25% | 18.37% | -1.88% |
| 7 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unalig` | runtime | 4.44% | 5.91% | +1.47% |
| 8 | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | solver | 1.18% | 0.00% | -1.18% |
| 9 | `std::_Function_handler<void (int, double, int), tsp::Br` | solver | 1.14% | 0.00% | -1.14% |
| 10 | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | runtime | 0.48% | 1.42% | +0.94% |
| 11 | `void std::vector<tsp::BranchBoundSolver::Edge, std::all` | solver | 1.29% | 0.53% | -0.76% |
| 12 | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | runtime | 0.29% | 0.88% | +0.59% |
| 13 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 2.05% | 2.58% | +0.53% |
| 14 | `tsp::BranchBoundSolver::twoOpt(...) const` | solver | 0.53% | 0.00% | -0.53% |
| 15 | `./malloc/./malloc/malloc.c:malloc` | runtime | 1.72% | 2.24% | +0.52% |
| 16 | `./malloc/./malloc/malloc.c:free` | runtime | 1.09% | 1.42% | +0.33% |
| 17 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | solver | 0.00% | 0.20% | +0.20% |
| 18 | `operator new(unsigned long)` | runtime | 0.62% | 0.81% | +0.19% |
| 19 | `operator delete(void*)` | runtime | 0.08% | 0.00% | -0.08% |
| 20 | `tsp::BranchBoundSolver::search(...)` | solver | 0.36% | 0.28% | -0.08% |
| 21 | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | solver | 0.00% | 0.00% | +0.00% |
| 22 | `(anonymous namespace)::solveInput(...)::CliOptions cons` | solver | 0.00% | 0.00% | +0.00% |
| 23 | `tsp::BranchBoundSolver::solve(tsp::BranchStrategy)` | solver | 0.00% | 0.00% | +0.00% |
| 24 | `tsp::BranchBoundSolver::lkSearch(...) const` | solver | 0.00% | 0.00% | +0.00% |
| 25 | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below mai` | solver | 0.00% | 0.00% | +0.00% |
