# Function Time Share - wi29

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `1,490,626,131` Ir.
- Native elapsed: `201.8ms`.
- Callgrind elapsed: `10.876s`.

## Solver Stats

- `cost`: `27603`
- `initial_upper_bound`: `27603`
- `method`: `exact`
- `nodes_created`: `77709`
- `nodes_expanded`: `38866`
- `pruned_bound`: `38843`
- `pruned_infeasible`: `24`
- `root_lower_bound`: `22727`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 32.63% / 65.8ms | 37.14% / 74.9ms |  |
| 2 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 24.38% / 49.2ms | 30.08% / 60.7ms |  |
| 3 | solver | `{lambda(bool)#1}::operator()(bool) const` | 21.32% / 43.0ms | 1850.20% / 3.733s |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 3.48% / 7.0ms | 4.22% / 8.5ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:malloc` | 2.91% / 5.9ms | 5.01% / 10.1ms |  |
| 6 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 2.21% / 4.5ms | 5.39% / 10.9ms |  |
| 7 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 1.83% / 3.7ms | 1.83% / 3.7ms |  |
| 8 | runtime | `./malloc/./malloc/malloc.c:free` | 1.82% / 3.7ms | 6.24% / 12.6ms |  |
| 9 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 1.82% / 3.7ms | 2.09% / 4.2ms |  |
| 10 | solver | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | 1.66% / 3.3ms | 1.66% / 3.3ms |  |
| 11 | runtime | `operator new(unsigned long)` | 1.04% / 2.1ms | 6.18% / 12.5ms |  |
| 12 | solver | `tsp::BranchBoundSolver::search(...)` | 0.64% / 1.3ms | 1883.70% / 3.801s |  |
| 13 | solver | `{lambda(int, double, int)#1}>::_M_invoke(...)` | 0.60% / 1.2ms | 1.70% / 3.4ms |  |
| 14 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 0.47% / 948.4us | 0.00% / 0.0us |  |
| 15 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.40% / 807.1us | 0.75% / 1.5ms |  |
| 16 | runtime | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.29% / 585.2us | 0.00% / 0.0us |  |
| 17 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.22% / 443.9us | 0.00% / 0.0us |  |
| 18 | runtime | `./malloc/./malloc/arena.c:free` | 0.20% / 403.6us | 0.00% / 0.0us |  |
| 19 | solver | `operator()(...)::LevelChanges const&) const` | 0.17% / 343.0us | 0.00% / 0.0us |  |
| 20 | runtime | `./malloc/./malloc/malloc.c:_int_free_maybe_consolidate` | 0.17% / 343.0us | 0.00% / 0.0us |  |
| 21 | solver | `tsp::BranchBoundSolver::twoOpt(std::vector<>&, double&) const` | 0.16% / 322.9us | 0.00% / 0.0us |  |
| 22 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.15% / 302.7us | 0.00% / 0.0us |  |
| 23 | runtime | `operator delete(void*)` | 0.13% / 262.3us | 6.50% / 13.1ms |  |
| 24 | solver | `(below main)` | 0.00% / 0.0us | 99.87% / 201.5ms |  |
| 25 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 99.87% / 201.5ms |  |
| 26 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 99.87% / 201.5ms |  |
| 27 | solver | `main` | 0.00% / 0.0us | 99.87% / 201.5ms |  |
| 28 | solver | `(anonymous namespace)::solveInput(...)::CliOptions const&)` | 0.00% / 0.0us | 99.86% / 201.5ms |  |
| 29 | solver | `tsp::BranchBoundSolver::solve(tsp::BranchStrategy)` | 0.00% / 0.0us | 99.83% / 201.4ms |  |
| 30 | runtime | `operator delete(void*, unsigned long)` | 0.00% / 0.0us | 6.76% / 13.6ms |  |

## Scope Total

- Selected function self share: `98.70%` of program Ir.
- Selected function estimated self time: `199.2ms`.
- Full CSV: `function_time_breakdown.csv`.
