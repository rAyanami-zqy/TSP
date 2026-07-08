# Function Time Share - fri26

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `14,738,129,521` Ir.
- Native elapsed: `1.937s`.
- Callgrind elapsed: `1.74m`.

## Solver Stats

- `cost`: `937`
- `initial_upper_bound`: `937`
- `method`: `exact`
- `nodes_created`: `984246`
- `nodes_expanded`: `492159`
- `pruned_bound`: `492087`
- `pruned_infeasible`: `73`
- `root_lower_bound`: `824`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 33.04% / 639.9ms | 38.85% / 752.4ms |  |
| 2 | solver | `{lambda(bool)#1}::operator()(bool) const` | 21.60% / 418.3ms | 2015.80% / 39.042s |  |
| 3 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 20.05% / 388.3ms | 27.14% / 525.6ms |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 4.38% / 84.8ms | 5.23% / 101.3ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:malloc` | 3.67% / 71.1ms | 6.05% / 117.2ms |  |
| 6 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 2.86% / 55.4ms | 7.00% / 135.6ms |  |
| 7 | runtime | `./malloc/./malloc/malloc.c:free` | 2.30% / 44.5ms | 7.78% / 150.7ms |  |
| 8 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 2.07% / 40.1ms | 2.07% / 40.1ms |  |
| 9 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 2.04% / 39.5ms | 2.37% / 45.9ms |  |
| 10 | solver | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | 1.95% / 37.8ms | 1.95% / 37.8ms |  |
| 11 | runtime | `operator new(unsigned long)` | 1.31% / 25.4ms | 7.53% / 145.8ms |  |
| 12 | solver | `tsp::BranchBoundSolver::search(...)` | 0.82% / 15.9ms | 2047.10% / 39.648s |  |
| 13 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 0.53% / 10.3ms | 0.00% / 0.0us |  |
| 14 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.51% / 9.9ms | 0.94% / 18.2ms |  |
| 15 | runtime | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.32% / 6.2ms | 0.00% / 0.0us |  |
| 16 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.25% / 4.8ms | 0.00% / 0.0us |  |
| 17 | runtime | `./malloc/./malloc/arena.c:free` | 0.25% / 4.8ms | 0.00% / 0.0us |  |
| 18 | solver | `operator()(...)::LevelChanges const&) const` | 0.22% / 4.3ms | 0.00% / 0.0us |  |
| 19 | runtime | `./malloc/./malloc/malloc.c:_int_free_maybe_consolidate` | 0.20% / 3.9ms | 0.00% / 0.0us |  |
| 20 | runtime | `operator delete(void*)` | 0.16% / 3.1ms | 8.10% / 156.9ms |  |
| 21 | solver | `(below main)` | 0.00% / 0.0us | 99.99% / 1.937s |  |
| 22 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 99.99% / 1.937s |  |
| 23 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 99.99% / 1.937s |  |
| 24 | solver | `main` | 0.00% / 0.0us | 99.99% / 1.937s |  |
| 25 | solver | `(anonymous namespace)::solveInput(...)::CliOptions const&)` | 0.00% / 0.0us | 99.99% / 1.937s |  |
| 26 | solver | `tsp::BranchBoundSolver::solve(tsp::BranchStrategy)` | 0.00% / 0.0us | 99.97% / 1.936s |  |
| 27 | runtime | `operator delete(void*, unsigned long)` | 0.00% / 0.0us | 8.43% / 163.3ms |  |

## Scope Total

- Selected function self share: `98.53%` of program Ir.
- Selected function estimated self time: `1.908s`.
- Full CSV: `function_time_breakdown.csv`.
