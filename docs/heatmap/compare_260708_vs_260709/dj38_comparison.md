# dj38 (n=38): 260708 vs 260709

## Performance

- Old (260708): 125.6ms
- New (260709): 8.948s
- Delta: 8.823s (+7026.0%)
- Old Ir: 910,025,230, New Ir: 67,723,820,524
- Solver self%: 77.7% → 76.4% (-1.2%)
- Runtime self%: 19.5% → 20.7% (+1.2%)

## Top Changes

| # | Function | Kind | Old Self% | New Self% | Delta |
|---:|---|---|---:|---:|---:|
| 1 | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unalig` | runtime | 8.30% | 5.91% | -2.39% |
| 2 | `./malloc/./malloc/malloc.c:_int_malloc` | runtime | 3.88% | 5.43% | +1.55% |
| 3 | `tsp::BranchBoundSolver::computeOneTree(...) const` | solver | 19.68% | 18.37% | -1.31% |
| 4 | `std::_Function_handler<void (int, double, int), tsp::Br` | solver | 1.09% | 0.00% | -1.09% |
| 5 | `tsp::BranchBoundSolver::buildBranchCandidates(...) cons` | solver | 30.13% | 30.81% | +0.68% |
| 6 | `./malloc/./malloc/malloc.c:_int_free` | runtime | 2.06% | 2.58% | +0.52% |
| 7 | `tsp::BranchBoundSolver::twoOpt(...) const` | solver | 0.51% | 0.00% | -0.51% |
| 8 | `./malloc/./malloc/malloc.c:malloc` | runtime | 1.76% | 2.24% | +0.48% |
| 9 | `tsp::BranchBoundSolver::bpPartition(...)` | solver | 3.58% | 4.04% | +0.46% |
| 10 | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | runtime | 1.04% | 1.42% | +0.38% |
| 11 | `./malloc/./malloc/malloc.c:free` | runtime | 1.12% | 1.42% | +0.30% |
| 12 | `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::o` | solver | 21.93% | 22.21% | +0.28% |
| 13 | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | runtime | 0.61% | 0.88% | +0.27% |
| 14 | `operator new(unsigned long)` | runtime | 0.64% | 0.81% | +0.17% |
| 15 | `void std::vector<tsp::BranchBoundSolver::Edge, std::all` | solver | 0.39% | 0.53% | +0.14% |
| 16 | `operator delete(void*)` | runtime | 0.08% | 0.00% | -0.08% |
| 17 | `tsp::BranchBoundSolver::search(...)` | solver | 0.21% | 0.28% | +0.07% |
| 18 | `std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | solver | 0.15% | 0.20% | +0.05% |
| 19 | `(anonymous namespace)::solveInput(...)::CliOptions cons` | solver | 0.00% | 0.00% | +0.00% |
| 20 | `tsp::BranchBoundSolver::lkSearch(...) const` | solver | 0.00% | 0.00% | +0.00% |
| 21 | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below mai` | solver | 0.00% | 0.00% | +0.00% |
| 22 | `(below main)` | solver | 0.00% | 0.00% | +0.00% |
| 23 | `std::_Function_handler<void (int), tsp::BranchBoundSolv` | solver | 0.00% | 0.00% | +0.00% |
| 24 | `operator delete(...)` | runtime | 0.00% | 0.00% | +0.00% |
| 25 | `tsp::BranchBoundSolver::findInitialTour(...)` | solver | 0.00% | 0.00% | +0.00% |
