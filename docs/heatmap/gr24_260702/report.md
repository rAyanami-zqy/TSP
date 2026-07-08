# Function Time Share - gr24

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `8,119,979,565` Ir.
- Native elapsed: `1.126s`.
- Callgrind elapsed: `58.224s`.

## Solver Stats

- `cost`: `1272`
- `initial_upper_bound`: `1272`
- `method`: `exact`
- `nodes_created`: `571607`
- `nodes_expanded`: `287023`
- `pruned_bound`: `284584`
- `pruned_infeasible`: `2440`
- `root_lower_bound`: `1081`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 32.82% / 369.6ms | 38.58% / 434.4ms |  |
| 2 | solver | `{lambda(bool)#1}::operator()(bool) const` | 21.57% / 242.9ms | 1906.60% / 21.469s |  |
| 3 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 19.13% / 215.4ms | 26.76% / 301.3ms |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 4.56% / 51.3ms | 5.47% / 61.6ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:malloc` | 3.81% / 42.9ms | 6.80% / 76.6ms |  |
| 6 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 2.82% / 31.8ms | 6.96% / 78.4ms |  |
| 7 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 2.46% / 27.7ms | 2.97% / 33.4ms |  |
| 8 | runtime | `./malloc/./malloc/malloc.c:free` | 2.38% / 26.8ms | 8.11% / 91.3ms |  |
| 9 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 2.06% / 23.2ms | 2.06% / 23.2ms |  |
| 10 | solver | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | 1.95% / 22.0ms | 1.95% / 22.0ms |  |
| 11 | runtime | `operator new(unsigned long)` | 1.36% / 15.3ms | 8.33% / 93.8ms |  |
| 12 | solver | `tsp::BranchBoundSolver::search(...)` | 0.87% / 9.8ms | 1937.70% / 21.819s |  |
| 13 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 0.57% / 6.4ms | 0.00% / 0.0us |  |
| 14 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.48% / 5.4ms | 0.94% / 10.6ms |  |
| 15 | runtime | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.38% / 4.3ms | 0.00% / 0.0us |  |
| 16 | runtime | `./malloc/./malloc/arena.c:free` | 0.26% / 2.9ms | 0.00% / 0.0us |  |
| 17 | runtime | `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.25% / 2.8ms | 0.00% / 0.0us |  |
| 18 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.24% / 2.7ms | 0.00% / 0.0us |  |
| 19 | solver | `operator()(...)::LevelChanges const&) const` | 0.23% / 2.6ms | 0.00% / 0.0us |  |
| 20 | runtime | `./malloc/./malloc/malloc.c:_int_free_maybe_consolidate` | 0.20% / 2.3ms | 0.00% / 0.0us |  |
| 21 | runtime | `operator delete(void*)` | 0.17% / 1.9ms | 8.45% / 95.1ms |  |
| 22 | solver | `(below main)` | 0.00% / 0.0us | 99.98% / 1.126s |  |
| 23 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 99.98% / 1.126s |  |
| 24 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 99.98% / 1.126s |  |
| 25 | solver | `main` | 0.00% / 0.0us | 99.98% / 1.126s |  |
| 26 | solver | `(anonymous namespace)::solveInput(...)::CliOptions const&)` | 0.00% / 0.0us | 99.97% / 1.126s |  |
| 27 | solver | `tsp::BranchBoundSolver::solve(tsp::BranchStrategy)` | 0.00% / 0.0us | 99.97% / 1.126s |  |
| 28 | runtime | `operator delete(void*, unsigned long)` | 0.00% / 0.0us | 8.79% / 99.0ms |  |

## Scope Total

- Selected function self share: `98.57%` of program Ir.
- Selected function estimated self time: `1.110s`.
- Full CSV: `function_time_breakdown.csv`.
