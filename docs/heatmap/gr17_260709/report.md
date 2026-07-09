# Function Time Share - gr17

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `21,761,551,205` Ir.
- Native elapsed: `3.486s`.
- Callgrind elapsed: `3.02m`.

## Solver Stats

- `cost`: `2085`
- `initial_upper_bound`: `2085`
- `method`: `exact`
- `nodes_created`: `2653019`
- `nodes_expanded`: `676329`
- `pruned_bound`: `1305260`
- `pruned_infeasible`: `41758`
- `root_lower_bound`: `1501`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 19.60% / 683.2ms | 35.27% / 1.229s |  |
| 2 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 17.07% / 595.0ms | 19.22% / 670.0ms |  |
| 3 | solver | `{lambda(int)#2}::operator()(int) const` | 13.42% / 467.8ms | 2073.60% / 1.20m |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 8.41% / 293.2ms | 10.19% / 355.2ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:malloc` | 6.87% / 239.5ms | 14.67% / 511.4ms |  |
| 6 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 6.28% / 218.9ms | 7.75% / 270.1ms |  |
| 7 | solver | `tsp::BranchBoundSolver::bpPartition(...)` | 4.82% / 168.0ms | 33.06% / 1.152s |  |
| 8 | runtime | `./malloc/./malloc/malloc.c:free` | 4.29% / 149.5ms | 14.94% / 520.8ms |  |
| 9 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 3.93% / 137.0ms | 3.93% / 137.0ms |  |
| 10 | runtime | `operator new(unsigned long)` | 2.45% / 85.4ms | 17.42% / 607.2ms |  |
| 11 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 1.67% / 58.2ms | 4.41% / 153.7ms |  |
| 12 | solver | `operator()(...)::LevelChanges&) const` | 1.14% / 39.7ms | 1.14% / 39.7ms |  |
| 13 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 1.12% / 39.0ms | 1.78% / 62.0ms |  |
| 14 | runtime | `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.88% / 30.7ms | 1.01% / 35.2ms |  |
| 15 | runtime | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.85% / 29.6ms | 0.85% / 29.6ms |  |
| 16 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.82% / 28.6ms | 1.81% / 63.1ms |  |
| 17 | solver | `tsp::BranchBoundSolver::search(...)` | 0.81% / 28.2ms | 873.30% / 30.441s |  |
| 18 | solver | `std::vector<>::reserve(unsigned long)` | 0.61% / 21.3ms | 2.72% / 94.8ms |  |
| 19 | runtime | `./malloc/./malloc/arena.c:free` | 0.46% / 16.0ms | 0.00% / 0.0us |  |
| 20 | solver | `operator()(...)::LevelChanges const&) const` | 0.39% / 13.6ms | 0.00% / 0.0us |  |
| 21 | runtime | `./malloc/./malloc/malloc.c:_int_free_maybe_consolidate` | 0.38% / 13.2ms | 0.00% / 0.0us |  |
| 22 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.33% / 11.5ms | 0.00% / 0.0us |  |
| 23 | runtime | `operator delete(void*)` | 0.31% / 10.8ms | 15.55% / 542.0ms |  |
| 24 | runtime | `operator delete(void*, unsigned long)` | 0.31% / 10.8ms | 16.17% / 563.6ms |  |
| 25 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.29% / 10.1ms | 0.00% / 0.0us |  |
| 26 | solver | `std::_Function_handler<>::_M_invoke(std::_Any_data const&, int&&)` | 0.00% / 0.0us | 2073.60% / 1.20m |  |
| 27 | solver | `(below main)` | 0.00% / 0.0us | 99.99% / 3.485s |  |
| 28 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 99.99% / 3.485s |  |
| 29 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 99.99% / 3.485s |  |
| 30 | solver | `main` | 0.00% / 0.0us | 99.99% / 3.485s |  |

## Scope Total

- Selected function self share: `97.51%` of program Ir.
- Selected function estimated self time: `3.399s`.
- Full CSV: `function_time_breakdown.csv`.
