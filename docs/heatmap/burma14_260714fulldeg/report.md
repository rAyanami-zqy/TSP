# Function Time Share - burma14

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `111,472,960` Ir.
- Native elapsed: `36.2ms`.
- Callgrind elapsed: `1.693s`.

## Solver Stats

- `cost`: `3323`
- `initial_upper_bound`: `3323`
- `method`: `exact`
- `nodes_created`: `8949`
- `nodes_expanded`: `4582`
- `pruned_bound`: `4367`
- `pruned_infeasible`: `214`
- `root_lower_bound`: `2542`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 27.59% / 10.0ms | 37.57% / 13.6ms |  |
| 2 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 20.86% / 7.6ms | 31.00% / 11.2ms |  |
| 3 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 8.94% / 3.2ms | 9.20% / 3.3ms |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:malloc` | 7.31% / 2.6ms | 8.29% / 3.0ms |  |
| 5 | solver | `tsp::BranchBoundSolver::search(...)` | 5.76% / 2.1ms | 561.50% / 203.3ms |  |
| 6 | solver | `tsp::BranchBoundSolver::bpPartition(...)` | 5.26% / 1.9ms | 29.32% / 10.6ms |  |
| 7 | runtime | `./malloc/./malloc/malloc.c:free` | 4.57% / 1.7ms | 14.27% / 5.2ms |  |
| 8 | runtime | `operator new(unsigned long)` | 2.61% / 945.2us | 11.23% / 4.1ms |  |
| 9 | solver | `{lambda(int, double, int)#1}>::_M_invoke(...)` | 2.49% / 901.7us | 6.58% / 2.4ms |  |
| 10 | runtime | `void std::vector<>::_M_realloc_insert<>(...)` | 2.36% / 854.6us | 7.40% / 2.7ms |  |
| 11 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 1.73% / 626.5us | 1.73% / 626.5us |  |
| 12 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 1.15% / 416.4us | 2.21% / 800.3us |  |
| 13 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 0.85% / 307.8us | 0.97% / 351.3us |  |
| 14 | solver | `std::vector<>::reserve(unsigned long)` | 0.65% / 235.4us | 1.77% / 641.0us |  |
| 15 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.49% / 177.4us | 0.00% / 0.0us |  |
| 16 | runtime | `./malloc/./malloc/arena.c:free` | 0.49% / 177.4us | 0.00% / 0.0us |  |
| 17 | runtime | `operator delete(void*)` | 0.33% / 119.5us | 14.92% / 5.4ms |  |
| 18 | runtime | `operator delete(void*, unsigned long)` | 0.33% / 119.5us | 15.57% / 5.6ms |  |
| 19 | runtime | `void std::vector<>::_M_realloc_insert<>(...)` | 0.31% / 112.3us | 0.00% / 0.0us |  |
| 20 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.29% / 105.0us | 0.00% / 0.0us |  |
| 21 | solver | `std::vector<>::~vector()` | 0.23% / 83.3us | 1.06% / 383.9us |  |
| 22 | solver | `tsp::BranchBoundSolver::lkSearch(...) const` | 0.22% / 79.7us | 4.42% / 1.6ms |  |
| 23 | runtime | `std::vector<>::reserve(unsigned long)` | 0.16% / 57.9us | 0.00% / 0.0us |  |
| 24 | solver | `tsp::BranchBoundSolver::shouldPrune(double, double) const` | 0.15% / 54.3us | 0.00% / 0.0us |  |
| 25 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 0.15% / 54.3us | 0.00% / 0.0us |  |
| 26 | solver | `tsp::BranchBoundSolver::twoOpt(std::vector<>&, double&) const` | 0.13% / 47.1us | 0.00% / 0.0us |  |
| 27 | runtime | `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.11% / 39.8us | 0.00% / 0.0us |  |
| 28 | runtime | `std::vector<int, std::allocator<int> >::~vector()` | 0.10% / 36.2us | 0.00% / 0.0us |  |
| 29 | solver | `(below main)` | 0.00% / 0.0us | 98.22% / 35.6ms |  |
| 30 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 98.22% / 35.6ms |  |

## Scope Total

- Selected function self share: `95.62%` of program Ir.
- Selected function estimated self time: `34.6ms`.
- Full CSV: `function_time_breakdown.csv`.
