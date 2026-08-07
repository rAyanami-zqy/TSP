# Function Time Share - burma14

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `112,488,432` Ir.
- Native elapsed: `34.3ms`.
- Callgrind elapsed: `1.698s`.

## Solver Stats

- `cost`: `3323`
- `initial_upper_bound`: `3323`
- `method`: `exact`
- `nodes_created`: `18111`
- `nodes_expanded`: `9162`
- `pruned_bound`: `8949`
- `pruned_infeasible`: `214`
- `root_lower_bound`: `2542`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 26.99% / 9.3ms | 37.25% / 12.8ms |  |
| 2 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 20.67% / 7.1ms | 30.72% / 10.5ms |  |
| 3 | solver | `tsp::BranchBoundSolver::searchSmart(...)` | 11.92% / 4.1ms | 1206.80% / 414.4ms |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 8.06% / 2.8ms | 8.41% / 2.9ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:malloc` | 6.49% / 2.2ms | 7.74% / 2.7ms |  |
| 6 | runtime | `./malloc/./malloc/malloc.c:free` | 4.06% / 1.4ms | 12.90% / 4.4ms |  |
| 7 | solver | `tsp::BranchBoundSolver::chooseSmartBranchEdge(...) const` | 2.62% / 899.7us | 2.62% / 899.7us |  |
| 8 | solver | `{lambda(int, double, int)#1}>::_M_invoke(...)` | 2.47% / 848.2us | 6.52% / 2.2ms |  |
| 9 | runtime | `operator new(unsigned long)` | 2.32% / 796.7us | 10.34% / 3.6ms |  |
| 10 | runtime | `void std::vector<>::_M_realloc_insert<>(...)` | 2.32% / 796.7us | 7.27% / 2.5ms |  |
| 11 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 1.30% / 446.4us | 1.30% / 446.4us |  |
| 12 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 1.14% / 391.5us | 2.19% / 752.0us |  |
| 13 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 1.11% / 381.2us | 1.24% / 425.8us |  |
| 14 | solver | `std::vector<>::reserve(unsigned long)` | 0.64% / 219.8us | 1.84% / 631.9us |  |
| 15 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.49% / 168.3us | 0.00% / 0.0us |  |
| 16 | solver | `std::vector<>::~vector()` | 0.46% / 158.0us | 3.85% / 1.3ms |  |
| 17 | runtime | `./malloc/./malloc/arena.c:free` | 0.44% / 151.1us | 0.00% / 0.0us |  |
| 18 | runtime | `std::vector<int, std::allocator<int> >::~vector()` | 0.32% / 109.9us | 2.80% / 961.5us |  |
| 19 | solver | `tsp::BranchBoundSolver::shouldPrune(double, double) const` | 0.31% / 106.5us | 0.00% / 0.0us |  |
| 20 | runtime | `void std::vector<>::_M_realloc_insert<>(...)` | 0.30% / 103.0us | 0.93% / 319.4us |  |
| 21 | runtime | `operator delete(void*)` | 0.29% / 99.6us | 13.48% / 4.6ms |  |
| 22 | runtime | `operator delete(void*, unsigned long)` | 0.29% / 99.6us | 14.06% / 4.8ms |  |
| 23 | solver | `tsp::BranchBoundSolver::lkSearch(...) const` | 0.22% / 75.5us | 4.38% / 1.5ms |  |
| 24 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.19% / 65.2us | 0.00% / 0.0us |  |
| 25 | runtime | `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.19% / 65.2us | 0.00% / 0.0us |  |
| 26 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 0.15% / 51.5us | 0.00% / 0.0us |  |
| 27 | solver | `tsp::BranchBoundSolver::twoOpt(std::vector<>&, double&) const` | 0.13% / 44.6us | 0.00% / 0.0us |  |
| 28 | solver | `(below main)` | 0.00% / 0.0us | 98.23% / 33.7ms |  |
| 29 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 98.23% / 33.7ms |  |
| 30 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 98.23% / 33.7ms |  |

## Scope Total

- Selected function self share: `95.89%` of program Ir.
- Selected function estimated self time: `32.9ms`.
- Full CSV: `function_time_breakdown.csv`.
