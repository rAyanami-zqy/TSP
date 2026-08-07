# Function Time Share - burma14

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `208,312,526` Ir.
- Native elapsed: `45.8ms`.
- Callgrind elapsed: `2.451s`.

## Solver Stats

- `cost`: `3323`
- `initial_upper_bound`: `3323`
- `method`: `exact`
- `nodes_created`: `18410`
- `nodes_expanded`: `9217`
- `pruned_bound`: `9193`
- `pruned_infeasible`: `28`
- `root_lower_bound`: `2542`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 27.36% / 12.5ms | 38.26% / 17.5ms |  |
| 2 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 20.83% / 9.5ms | 32.18% / 14.7ms |  |
| 3 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 9.57% / 4.4ms | 9.80% / 4.5ms |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:malloc` | 7.85% / 3.6ms | 8.79% / 4.0ms |  |
| 5 | solver | `tsp::BranchBoundSolver::search(...)` | 6.14% / 2.8ms | 671.30% / 307.3ms |  |
| 6 | solver | `tsp::BranchBoundSolver::bpPartition(...)` | 5.20% / 2.4ms | 29.73% / 13.6ms |  |
| 7 | runtime | `./malloc/./malloc/malloc.c:free` | 4.91% / 2.2ms | 15.24% / 7.0ms |  |
| 8 | runtime | `operator new(unsigned long)` | 2.80% / 1.3ms | 11.95% / 5.5ms |  |
| 9 | runtime | `void std::vector<>::_M_realloc_insert<>(...)` | 2.66% / 1.2ms | 8.36% / 3.8ms |  |
| 10 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 1.79% / 819.3us | 1.79% / 819.3us |  |
| 11 | solver | `{lambda(int, double, int)#1}>::_M_invoke(...)` | 1.33% / 608.8us | 3.52% / 1.6ms |  |
| 12 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 1.26% / 576.7us | 2.41% / 1.1ms |  |
| 13 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 0.79% / 361.6us | 0.00% / 0.0us |  |
| 14 | solver | `std::vector<>::reserve(unsigned long)` | 0.71% / 325.0us | 1.98% / 906.3us |  |
| 15 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.54% / 247.2us | 0.00% / 0.0us |  |
| 16 | runtime | `./malloc/./malloc/arena.c:free` | 0.53% / 242.6us | 0.00% / 0.0us |  |
| 17 | runtime | `operator delete(void*)` | 0.35% / 160.2us | 15.94% / 7.3ms |  |
| 18 | runtime | `operator delete(void*, unsigned long)` | 0.35% / 160.2us | 16.64% / 7.6ms |  |
| 19 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.31% / 141.9us | 0.00% / 0.0us |  |
| 20 | solver | `std::vector<>::~vector()` | 0.25% / 114.4us | 1.17% / 535.5us |  |
| 21 | runtime | `std::vector<>::reserve(unsigned long)` | 0.18% / 82.4us | 0.00% / 0.0us |  |
| 22 | runtime | `void std::vector<>::_M_realloc_insert<>(...)` | 0.17% / 77.8us | 0.00% / 0.0us |  |
| 23 | solver | `tsp::BranchBoundSolver::shouldPrune(double, double) const` | 0.17% / 77.8us | 0.00% / 0.0us |  |
| 24 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 0.15% / 68.7us | 0.00% / 0.0us |  |
| 25 | solver | `tsp::BranchBoundSolver::lkSearch(...) const` | 0.12% / 54.9us | 2.37% / 1.1ms |  |
| 26 | runtime | `std::vector<int, std::allocator<int> >::~vector()` | 0.12% / 54.9us | 1.00% / 457.7us |  |
| 27 | runtime | `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.10% / 45.8us | 0.00% / 0.0us |  |
| 28 | solver | `(below main)` | 0.00% / 0.0us | 99.05% / 45.3ms |  |
| 29 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 99.05% / 45.3ms |  |
| 30 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 99.05% / 45.3ms |  |

## Scope Total

- Selected function self share: `96.54%` of program Ir.
- Selected function estimated self time: `44.2ms`.
- Full CSV: `function_time_breakdown.csv`.
