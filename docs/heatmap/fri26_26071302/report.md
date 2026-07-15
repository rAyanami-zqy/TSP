# Function Time Share - fri26

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `6,211,515,375` Ir.
- Native elapsed: `810.1ms`.
- Callgrind elapsed: `43.974s`.

## Solver Stats

- `cost`: `937`
- `initial_upper_bound`: `937`
- `method`: `exact`
- `nodes_created`: `195433`
- `nodes_expanded`: `95295`
- `pruned_bound`: `100138`
- `pruned_infeasible`: `33`
- `root_lower_bound`: `824`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `BranchBoundSolver::updateOneTreeAfterCandidateRemoval(...) const` | 35.36% / 286.4ms | 37.12% / 300.7ms |  |
| 2 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 31.92% / 258.6ms | 36.92% / 299.1ms |  |
| 3 | solver | `tsp::BranchBoundSolver::updateOneTreeAfterForbid(...) const` | 8.44% / 68.4ms | 9.67% / 78.3ms |  |
| 4 | solver | `tsp::BranchBoundSolver::search(...)` | 4.87% / 39.5ms | 894.40% / 7.245s |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 3.42% / 27.7ms | 3.97% / 32.2ms |  |
| 6 | runtime | `./malloc/./malloc/malloc.c:malloc` | 2.85% / 23.1ms | 4.50% / 36.5ms |  |
| 7 | solver | `tsp::BranchBoundSolver::bpPartition(...)` | 2.18% / 17.7ms | 12.97% / 105.1ms |  |
| 8 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 1.98% / 16.0ms | 1.98% / 16.0ms |  |
| 9 | runtime | `./malloc/./malloc/malloc.c:free` | 1.79% / 14.5ms | 5.95% / 48.2ms |  |
| 10 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 1.42% / 11.5ms | 1.65% / 13.4ms |  |
| 11 | runtime | `void std::vector<>::_M_realloc_insert<>(...)` | 1.32% / 10.7ms | 4.35% / 35.2ms |  |
| 12 | runtime | `operator new(unsigned long)` | 1.02% / 8.3ms | 5.65% / 45.8ms |  |
| 13 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 0.41% / 3.3ms | 0.00% / 0.0us |  |
| 14 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.37% / 3.0ms | 0.00% / 0.0us |  |
| 15 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 0.34% / 2.8ms | 0.00% / 0.0us |  |
| 16 | runtime | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.24% / 1.9ms | 0.00% / 0.0us |  |
| 17 | runtime | `./malloc/./malloc/arena.c:free` | 0.19% / 1.5ms | 0.00% / 0.0us |  |
| 18 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.16% / 1.3ms | 0.00% / 0.0us |  |
| 19 | solver | `{lambda(int, double, int)#1}>::_M_invoke(...)` | 0.16% / 1.3ms | 0.00% / 0.0us |  |
| 20 | runtime | `operator delete(void*)` | 0.13% / 1.1ms | 6.20% / 50.2ms |  |
| 21 | solver | `(below main)` | 0.00% / 0.0us | 99.97% / 809.8ms |  |
| 22 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 99.97% / 809.8ms |  |
| 23 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 99.97% / 809.8ms |  |
| 24 | solver | `main` | 0.00% / 0.0us | 99.97% / 809.8ms |  |
| 25 | solver | `(anonymous namespace)::solveInput(...)::CliOptions const&)` | 0.00% / 0.0us | 99.97% / 809.8ms |  |
| 26 | solver | `tsp::BranchBoundSolver::solve()` | 0.00% / 0.0us | 99.94% / 809.6ms |  |
| 27 | runtime | `operator delete(void*, unsigned long)` | 0.00% / 0.0us | 6.46% / 52.3ms |  |
| 28 | runtime | `std::vector<int, std::allocator<int> >::~vector()` | 0.00% / 0.0us | 1.01% / 8.2ms |  |

## Scope Total

- Selected function self share: `98.57%` of program Ir.
- Selected function estimated self time: `798.5ms`.
- Full CSV: `function_time_breakdown.csv`.
