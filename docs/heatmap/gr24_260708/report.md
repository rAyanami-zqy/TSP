# Function Time Share - gr24

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `8,375,907,163` Ir.
- Native elapsed: `1.256s`.
- Callgrind elapsed: `1.08m`.

## Solver Stats

- `cost`: `1272`
- `initial_upper_bound`: `1272`
- `method`: `exact`
- `nodes_created`: `571607`
- `nodes_expanded`: `158220`
- `pruned_bound`: `284581`
- `pruned_infeasible`: `2440`
- `root_lower_bound`: `1081`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 26.81% / 336.8ms | 28.05% / 352.3ms |  |
| 2 | solver | `{lambda(int)#2}::operator()(int) const` | 20.03% / 251.6ms | 1862.80% / 23.399s |  |
| 3 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 18.74% / 235.4ms | 27.77% / 348.8ms |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 5.28% / 66.3ms | 6.46% / 81.1ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 4.75% / 59.7ms | 6.39% / 80.3ms |  |
| 6 | solver | `tsp::BranchBoundSolver::bpPartition(...)` | 4.44% / 55.8ms | 25.49% / 320.2ms |  |
| 7 | runtime | `./malloc/./malloc/malloc.c:malloc` | 3.91% / 49.1ms | 10.42% / 130.9ms |  |
| 8 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 3.61% / 45.3ms | 3.61% / 45.3ms |  |
| 9 | runtime | `./malloc/./malloc/malloc.c:free` | 2.45% / 30.8ms | 9.11% / 114.4ms |  |
| 10 | runtime | `operator new(unsigned long)` | 1.40% / 17.6ms | 11.99% / 150.6ms |  |
| 11 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 1.06% / 13.3ms | 1.64% / 20.6ms |  |
| 12 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 0.93% / 11.7ms | 2.47% / 31.0ms |  |
| 13 | runtime | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.79% / 9.9ms | 0.79% / 9.9ms |  |
| 14 | solver | `operator()(...)::LevelChanges&) const` | 0.63% / 7.9ms | 0.00% / 0.0us |  |
| 15 | runtime | `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.60% / 7.5ms | 0.00% / 0.0us |  |
| 16 | solver | `tsp::BranchBoundSolver::search(...)` | 0.47% / 5.9ms | 831.70% / 10.447s |  |
| 17 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.46% / 5.8ms | 1.05% / 13.2ms |  |
| 18 | runtime | `./malloc/./malloc/malloc.c:_int_free_maybe_consolidate` | 0.36% / 4.5ms | 0.00% / 0.0us |  |
| 19 | solver | `std::vector<>::reserve(unsigned long)` | 0.35% / 4.4ms | 1.68% / 21.1ms |  |
| 20 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.28% / 3.5ms | 0.00% / 0.0us |  |
| 21 | runtime | `./malloc/./malloc/arena.c:free` | 0.26% / 3.3ms | 0.00% / 0.0us |  |
| 22 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.23% / 2.9ms | 0.00% / 0.0us |  |
| 23 | solver | `operator()(...)::LevelChanges const&) const` | 0.22% / 2.8ms | 0.00% / 0.0us |  |
| 24 | runtime | `operator delete(void*)` | 0.18% / 2.3ms | 9.46% / 118.8ms |  |
| 25 | runtime | `operator delete(void*, unsigned long)` | 0.18% / 2.3ms | 9.81% / 123.2ms |  |
| 26 | solver | `std::_Function_handler<>::_M_invoke(std::_Any_data const&, int&&)` | 0.00% / 0.0us | 1862.80% / 23.399s |  |
| 27 | solver | `(below main)` | 0.00% / 0.0us | 99.98% / 1.256s |  |
| 28 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 99.98% / 1.256s |  |
| 29 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 99.98% / 1.256s |  |
| 30 | solver | `main` | 0.00% / 0.0us | 99.98% / 1.256s |  |

## Scope Total

- Selected function self share: `98.42%` of program Ir.
- Selected function estimated self time: `1.236s`.
- Full CSV: `function_time_breakdown.csv`.
