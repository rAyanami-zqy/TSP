# Function Time Share - fri26

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `15,174,385,039` Ir.
- Native elapsed: `2.198s`.
- Callgrind elapsed: `1.92m`.

## Solver Stats

- `cost`: `937`
- `initial_upper_bound`: `937`
- `method`: `exact`
- `nodes_created`: `984246`
- `nodes_expanded`: `264044`
- `pruned_bound`: `492087`
- `pruned_infeasible`: `73`
- `root_lower_bound`: `824`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 27.01% / 593.6ms | 28.19% / 619.5ms |  |
| 2 | solver | `{lambda(int)#2}::operator()(int) const` | 20.11% / 441.9ms | 1974.10% / 43.383s |  |
| 3 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 19.70% / 432.9ms | 28.39% / 623.9ms |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 5.11% / 112.3ms | 6.29% / 138.2ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 4.49% / 98.7ms | 6.05% / 133.0ms |  |
| 6 | solver | `tsp::BranchBoundSolver::bpPartition(...)` | 4.38% / 96.3ms | 25.35% / 557.1ms |  |
| 7 | runtime | `./malloc/./malloc/malloc.c:malloc` | 3.69% / 81.1ms | 10.02% / 220.2ms |  |
| 8 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 3.62% / 79.6ms | 3.62% / 79.6ms |  |
| 9 | runtime | `./malloc/./malloc/malloc.c:free` | 2.31% / 50.8ms | 8.61% / 189.2ms |  |
| 10 | runtime | `operator new(unsigned long)` | 1.32% / 29.0ms | 11.51% / 252.9ms |  |
| 11 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 1.00% / 22.0ms | 1.56% / 34.3ms |  |
| 12 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 0.89% / 19.6ms | 2.35% / 51.6ms |  |
| 13 | runtime | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.77% / 16.9ms | 0.77% / 16.9ms |  |
| 14 | runtime | `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.62% / 13.6ms | 0.00% / 0.0us |  |
| 15 | solver | `operator()(...)::LevelChanges&) const` | 0.61% / 13.4ms | 0.00% / 0.0us |  |
| 16 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.49% / 10.8ms | 1.10% / 24.2ms |  |
| 17 | solver | `tsp::BranchBoundSolver::search(...)` | 0.44% / 9.7ms | 926.00% / 20.350s |  |
| 18 | runtime | `./malloc/./malloc/malloc.c:_int_free_maybe_consolidate` | 0.34% / 7.5ms | 0.00% / 0.0us |  |
| 19 | solver | `std::vector<>::reserve(unsigned long)` | 0.33% / 7.3ms | 1.63% / 35.8ms |  |
| 20 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.29% / 6.4ms | 0.00% / 0.0us |  |
| 21 | runtime | `./malloc/./malloc/arena.c:free` | 0.25% / 5.5ms | 0.00% / 0.0us |  |
| 22 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.25% / 5.5ms | 0.00% / 0.0us |  |
| 23 | solver | `operator()(...)::LevelChanges const&) const` | 0.21% / 4.6ms | 0.00% / 0.0us |  |
| 24 | runtime | `operator delete(void*)` | 0.17% / 3.7ms | 8.94% / 196.5ms |  |
| 25 | solver | `std::_Function_handler<>::_M_invoke(std::_Any_data const&, int&&)` | 0.00% / 0.0us | 1974.20% / 43.386s |  |
| 26 | solver | `(below main)` | 0.00% / 0.0us | 99.99% / 2.197s |  |
| 27 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 99.99% / 2.197s |  |
| 28 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 99.99% / 2.197s |  |
| 29 | solver | `main` | 0.00% / 0.0us | 99.99% / 2.197s |  |
| 30 | solver | `(anonymous namespace)::solveInput(...)::CliOptions const&)` | 0.00% / 0.0us | 99.99% / 2.197s |  |

## Scope Total

- Selected function self share: `98.40%` of program Ir.
- Selected function estimated self time: `2.162s`.
- Full CSV: `function_time_breakdown.csv`.
