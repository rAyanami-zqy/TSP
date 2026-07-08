# Function Time Share - gr17

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `15,290,897,525` Ir.
- Native elapsed: `2.474s`.
- Callgrind elapsed: `2.11m`.

## Solver Stats

- `cost`: `2085`
- `initial_upper_bound`: `2085`
- `method`: `exact`
- `nodes_created`: `1817688`
- `nodes_expanded`: `463820`
- `pruned_bound`: `897458`
- `pruned_infeasible`: `22575`
- `root_lower_bound`: `1501`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 20.66% / 511.1ms | 35.90% / 888.1ms |  |
| 2 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 17.10% / 423.0ms | 19.20% / 475.0ms |  |
| 3 | solver | `{lambda(int)#2}::operator()(int) const` | 13.57% / 335.7ms | 2032.00% / 50.266s |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 8.20% / 202.8ms | 9.88% / 244.4ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:malloc` | 6.70% / 165.7ms | 14.08% / 348.3ms |  |
| 6 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 5.92% / 146.4ms | 7.33% / 181.3ms |  |
| 7 | solver | `tsp::BranchBoundSolver::bpPartition(...)` | 5.00% / 123.7ms | 33.24% / 822.3ms |  |
| 8 | runtime | `./malloc/./malloc/malloc.c:free` | 4.18% / 103.4ms | 14.51% / 358.9ms |  |
| 9 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 3.80% / 94.0ms | 3.80% / 94.0ms |  |
| 10 | runtime | `operator new(unsigned long)` | 2.39% / 59.1ms | 16.77% / 414.8ms |  |
| 11 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 1.63% / 40.3ms | 4.30% / 106.4ms |  |
| 12 | solver | `operator()(...)::LevelChanges&) const` | 1.11% / 27.5ms | 1.11% / 27.5ms |  |
| 13 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 1.06% / 26.2ms | 1.68% / 41.6ms |  |
| 14 | runtime | `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.86% / 21.3ms | 0.98% / 24.2ms |  |
| 15 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.80% / 19.8ms | 1.77% / 43.8ms |  |
| 16 | runtime | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.80% / 19.8ms | 0.00% / 0.0us |  |
| 17 | solver | `tsp::BranchBoundSolver::search(...)` | 0.79% / 19.5ms | 770.10% / 19.050s |  |
| 18 | solver | `std::vector<>::reserve(unsigned long)` | 0.60% / 14.8ms | 2.59% / 64.1ms |  |
| 19 | runtime | `./malloc/./malloc/arena.c:free` | 0.45% / 11.1ms | 0.00% / 0.0us |  |
| 20 | solver | `operator()(...)::LevelChanges const&) const` | 0.38% / 9.4ms | 0.00% / 0.0us |  |
| 21 | runtime | `./malloc/./malloc/malloc.c:_int_free_maybe_consolidate` | 0.36% / 8.9ms | 0.00% / 0.0us |  |
| 22 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.32% / 7.9ms | 0.00% / 0.0us |  |
| 23 | runtime | `operator delete(void*)` | 0.30% / 7.4ms | 15.11% / 373.8ms |  |
| 24 | runtime | `operator delete(void*, unsigned long)` | 0.30% / 7.4ms | 15.70% / 388.4ms |  |
| 25 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.28% / 6.9ms | 0.00% / 0.0us |  |
| 26 | solver | `std::_Function_handler<>::_M_invoke(std::_Any_data const&, int&&)` | 0.00% / 0.0us | 2032.00% / 50.266s |  |
| 27 | solver | `(below main)` | 0.00% / 0.0us | 99.99% / 2.473s |  |
| 28 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 99.99% / 2.473s |  |
| 29 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 99.99% / 2.473s |  |
| 30 | solver | `main` | 0.00% / 0.0us | 99.99% / 2.473s |  |

## Scope Total

- Selected function self share: `97.56%` of program Ir.
- Selected function estimated self time: `2.413s`.
- Full CSV: `function_time_breakdown.csv`.
