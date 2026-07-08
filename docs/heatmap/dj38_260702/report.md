# Function Time Share - dj38

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `870,679,889` Ir.
- Native elapsed: `113.7ms`.
- Callgrind elapsed: `6.910s`.

## Solver Stats

- `cost`: `6656`
- `initial_upper_bound`: `6656`
- `method`: `exact`
- `nodes_created`: `25733`
- `nodes_expanded`: `12866`
- `pruned_bound`: `12867`
- `pruned_infeasible`: `0`
- `root_lower_bound`: `6285`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 37.23% / 42.3ms | 39.85% / 45.3ms |  |
| 2 | solver | `{lambda(bool)#1}::operator()(bool) const` | 23.34% / 26.5ms | 1708.90% / 1.944s |  |
| 3 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 20.25% / 23.0ms | 23.53% / 26.8ms |  |
| 4 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 4.44% / 5.1ms | 4.44% / 5.1ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 2.05% / 2.3ms | 2.80% / 3.2ms |  |
| 6 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 1.78% / 2.0ms | 1.98% / 2.3ms |  |
| 7 | runtime | `./malloc/./malloc/malloc.c:malloc` | 1.72% / 2.0ms | 3.72% / 4.2ms |  |
| 8 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 1.29% / 1.5ms | 3.11% / 3.5ms |  |
| 9 | solver | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | 1.18% / 1.3ms | 1.18% / 1.3ms |  |
| 10 | solver | `{lambda(int, double, int)#1}>::_M_invoke(...)` | 1.14% / 1.3ms | 3.22% / 3.7ms |  |
| 11 | runtime | `./malloc/./malloc/malloc.c:free` | 1.09% / 1.2ms | 4.00% / 4.5ms |  |
| 12 | runtime | `operator new(unsigned long)` | 0.62% / 705.2us | 4.41% / 5.0ms |  |
| 13 | solver | `tsp::BranchBoundSolver::twoOpt(std::vector<>&, double&) const` | 0.53% / 602.8us | 0.00% / 0.0us |  |
| 14 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 0.48% / 546.0us | 0.75% / 853.1us |  |
| 15 | solver | `tsp::BranchBoundSolver::search(...)` | 0.36% / 409.5us | 1734.70% / 1.973s |  |
| 16 | runtime | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.29% / 329.8us | 0.00% / 0.0us |  |
| 17 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.25% / 284.4us | 0.00% / 0.0us |  |
| 18 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.19% / 216.1us | 0.00% / 0.0us |  |
| 19 | runtime | `./malloc/./malloc/malloc.c:_int_free_maybe_consolidate` | 0.17% / 193.4us | 0.00% / 0.0us |  |
| 20 | runtime | `void std::vector<>::_M_realloc_insert<>(...)` | 0.13% / 147.9us | 0.00% / 0.0us |  |
| 21 | runtime | `./malloc/./malloc/arena.c:free` | 0.12% / 136.5us | 0.00% / 0.0us |  |
| 22 | solver | `operator()(...)::LevelChanges const&) const` | 0.10% / 113.7us | 0.00% / 0.0us |  |
| 23 | runtime | `operator delete(void*)` | 0.08% / 91.0us | 4.16% / 4.7ms |  |
| 24 | solver | `(below main)` | 0.00% / 0.0us | 99.77% / 113.5ms |  |
| 25 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 99.77% / 113.5ms |  |
| 26 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 99.77% / 113.5ms |  |
| 27 | solver | `main` | 0.00% / 0.0us | 99.77% / 113.5ms |  |
| 28 | solver | `(anonymous namespace)::solveInput(...)::CliOptions const&)` | 0.00% / 0.0us | 99.76% / 113.5ms |  |
| 29 | solver | `tsp::BranchBoundSolver::solve(tsp::BranchStrategy)` | 0.00% / 0.0us | 99.67% / 113.4ms |  |
| 30 | runtime | `operator delete(void*, unsigned long)` | 0.00% / 0.0us | 4.31% / 4.9ms |  |

## Scope Total

- Selected function self share: `98.83%` of program Ir.
- Selected function estimated self time: `112.4ms`.
- Full CSV: `function_time_breakdown.csv`.
