# Function Time Share - gr17

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `14,216,783,384` Ir.
- Native elapsed: `2.210s`.
- Callgrind elapsed: `1.82m`.

## Solver Stats

- `cost`: `2085`
- `initial_upper_bound`: `2085`
- `method`: `exact`
- `nodes_created`: `1817688`
- `nodes_expanded`: `920131`
- `pruned_bound`: `897557`
- `pruned_infeasible`: `22575`
- `root_lower_bound`: `1501`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 22.05% / 487.3ms | 35.41% / 782.5ms |  |
| 2 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 21.82% / 482.2ms | 30.94% / 683.7ms |  |
| 3 | solver | `{lambda(bool)#1}::operator()(bool) const` | 16.01% / 353.8ms | 2096.50% / 46.329s |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 8.14% / 179.9ms | 8.82% / 194.9ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:malloc` | 6.79% / 150.0ms | 8.90% / 196.7ms |  |
| 6 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 4.44% / 98.1ms | 11.48% / 253.7ms |  |
| 7 | runtime | `./malloc/./malloc/malloc.c:free` | 4.23% / 93.5ms | 13.51% / 298.5ms |  |
| 8 | solver | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | 2.75% / 60.8ms | 2.75% / 60.8ms |  |
| 9 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 2.42% / 53.5ms | 2.42% / 53.5ms |  |
| 10 | runtime | `operator new(unsigned long)` | 2.42% / 53.5ms | 11.62% / 256.8ms |  |
| 11 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 1.77% / 39.1ms | 2.10% / 46.4ms |  |
| 12 | solver | `tsp::BranchBoundSolver::search(...)` | 1.58% / 34.9ms | 2138.90% / 47.266s |  |
| 13 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.86% / 19.0ms | 1.69% / 37.3ms |  |
| 14 | runtime | `./malloc/./malloc/arena.c:free` | 0.45% / 9.9ms | 0.00% / 0.0us |  |
| 15 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 0.45% / 9.9ms | 0.68% / 15.0ms |  |
| 16 | solver | `operator()(...)::LevelChanges const&) const` | 0.41% / 9.1ms | 0.00% / 0.0us |  |
| 17 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.30% / 6.6ms | 0.00% / 0.0us |  |
| 18 | runtime | `operator delete(void*)` | 0.30% / 6.6ms | 14.11% / 311.8ms |  |
| 19 | runtime | `operator delete(void*, unsigned long)` | 0.30% / 6.6ms | 14.72% / 325.3ms |  |
| 20 | solver | `(below main)` | 0.00% / 0.0us | 99.99% / 2.210s |  |
| 21 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 99.99% / 2.210s |  |
| 22 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 99.99% / 2.210s |  |
| 23 | solver | `main` | 0.00% / 0.0us | 99.99% / 2.210s |  |
| 24 | solver | `(anonymous namespace)::solveInput(...)::CliOptions const&)` | 0.00% / 0.0us | 99.99% / 2.210s |  |
| 25 | solver | `tsp::BranchBoundSolver::solve(tsp::BranchStrategy)` | 0.00% / 0.0us | 99.98% / 2.209s |  |

## Scope Total

- Selected function self share: `97.49%` of program Ir.
- Selected function estimated self time: `2.154s`.
- Full CSV: `function_time_breakdown.csv`.
