# Function Time Share - fri26

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `69,541,150,846` Ir.
- Native elapsed: `10.453s`.
- Callgrind elapsed: `8.89m`.

## Solver Stats

- `cost`: `937`
- `initial_upper_bound`: `937`
- `method`: `exact`
- `nodes_created`: `5294723`
- `nodes_expanded`: `1470662`
- `pruned_bound`: `2647198`
- `pruned_infeasible`: `0`
- `root_lower_bound`: `824`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 24.67% / 2.579s | 26.06% / 2.724s |  |
| 2 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 19.51% / 2.039s | 29.68% / 3.102s |  |
| 3 | solver | `{lambda(int)#2}::operator()(int) const` | 18.13% / 1.895s | 2233.70% / 3.89m |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 5.86% / 612.5ms | 7.17% / 749.5ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 5.32% / 556.1ms | 7.10% / 742.1ms |  |
| 6 | solver | `tsp::BranchBoundSolver::bpPartition(...)` | 4.53% / 473.5ms | 27.45% / 2.869s |  |
| 7 | runtime | `./malloc/./malloc/malloc.c:malloc` | 4.37% / 456.8ms | 11.58% / 1.210s |  |
| 8 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 3.91% / 408.7ms | 3.91% / 408.7ms |  |
| 9 | runtime | `./malloc/./malloc/malloc.c:free` | 2.74% / 286.4ms | 10.13% / 1.059s |  |
| 10 | runtime | `operator new(unsigned long)` | 1.56% / 163.1ms | 13.34% / 1.394s |  |
| 11 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 1.13% / 118.1ms | 1.78% / 186.1ms |  |
| 12 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 1.04% / 108.7ms | 2.76% / 288.5ms |  |
| 13 | runtime | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.87% / 90.9ms | 0.87% / 90.9ms |  |
| 14 | solver | `operator()(...)::LevelChanges&) const` | 0.71% / 74.2ms | 0.00% / 0.0us |  |
| 15 | runtime | `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.70% / 73.2ms | 0.00% / 0.0us |  |
| 16 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.58% / 60.6ms | 1.26% / 131.7ms |  |
| 17 | solver | `tsp::BranchBoundSolver::search(...)` | 0.53% / 55.4ms | 1325.30% / 2.31m |  |
| 18 | solver | `std::vector<>::reserve(unsigned long)` | 0.39% / 40.8ms | 1.97% / 205.9ms |  |
| 19 | runtime | `./malloc/./malloc/malloc.c:_int_free_maybe_consolidate` | 0.38% / 39.7ms | 0.00% / 0.0us |  |
| 20 | runtime | `./malloc/./malloc/arena.c:free` | 0.29% / 30.3ms | 0.00% / 0.0us |  |
| 21 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.29% / 30.3ms | 0.00% / 0.0us |  |
| 22 | solver | `operator()(...)::LevelChanges const&) const` | 0.25% / 26.1ms | 0.00% / 0.0us |  |
| 23 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.25% / 26.1ms | 0.00% / 0.0us |  |
| 24 | runtime | `operator delete(void*)` | 0.20% / 20.9ms | 10.52% / 1.100s |  |
| 25 | runtime | `operator delete(void*, unsigned long)` | 0.20% / 20.9ms | 10.91% / 1.140s |  |
| 26 | solver | `std::_Function_handler<>::_M_invoke(std::_Any_data const&, int&&)` | 0.00% / 0.0us | 2233.70% / 3.89m |  |
| 27 | solver | `(below main)` | 0.00% / 0.0us | 100.00% / 10.453s |  |
| 28 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 100.00% / 10.453s |  |
| 29 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 100.00% / 10.453s |  |
| 30 | solver | `main` | 0.00% / 0.0us | 100.00% / 10.453s |  |

## Scope Total

- Selected function self share: `98.41%` of program Ir.
- Selected function estimated self time: `10.287s`.
- Full CSV: `function_time_breakdown.csv`.
