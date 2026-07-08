# Function Time Share - ulysses16

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `105,010,724,449` Ir.
- Native elapsed: `16.124s`.
- Callgrind elapsed: `13.80m`.

## Solver Stats

- `cost`: `6859`
- `initial_upper_bound`: `6859`
- `method`: `exact`
- `nodes_created`: `15447419`
- `nodes_expanded`: `7797474`
- `pruned_bound`: `7649945`
- `pruned_infeasible`: `147530`
- `root_lower_bound`: `4746`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 27.98% / 4.511s | 43.29% / 6.980s |  |
| 2 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 16.22% / 2.615s | 26.02% / 4.195s |  |
| 3 | solver | `{lambda(bool)#1}::operator()(bool) const` | 12.87% / 2.075s | 2506.70% / 6.74m |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 9.25% / 1.491s | 10.06% / 1.622s |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:malloc` | 7.68% / 1.238s | 9.32% / 1.503s |  |
| 6 | runtime | `./malloc/./malloc/malloc.c:free` | 4.78% / 770.7ms | 15.35% / 2.475s |  |
| 7 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 4.65% / 749.7ms | 12.59% / 2.030s |  |
| 8 | solver | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | 3.00% / 483.7ms | 3.00% / 483.7ms |  |
| 9 | runtime | `operator new(unsigned long)` | 2.73% / 440.2ms | 12.39% / 1.998s |  |
| 10 | solver | `tsp::BranchBoundSolver::search(...)` | 1.81% / 291.8ms | 2558.20% / 6.87m |  |
| 11 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 1.55% / 249.9ms | 1.55% / 249.9ms |  |
| 12 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 1.48% / 238.6ms | 1.63% / 262.8ms |  |
| 13 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.93% / 149.9ms | 1.88% / 303.1ms |  |
| 14 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 0.57% / 91.9ms | 0.82% / 132.2ms |  |
| 15 | runtime | `./malloc/./malloc/arena.c:free` | 0.51% / 82.2ms | 0.00% / 0.0us |  |
| 16 | solver | `operator()(...)::LevelChanges const&) const` | 0.47% / 75.8ms | 0.00% / 0.0us |  |
| 17 | runtime | `operator delete(void*)` | 0.34% / 54.8ms | 16.04% / 2.586s |  |
| 18 | runtime | `operator delete(void*, unsigned long)` | 0.34% / 54.8ms | 16.72% / 2.696s |  |
| 19 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.28% / 45.1ms | 0.00% / 0.0us |  |
| 20 | solver | `(below main)` | 0.00% / 0.0us | 100.00% / 16.124s |  |
| 21 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 100.00% / 16.124s |  |
| 22 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 100.00% / 16.124s |  |
| 23 | solver | `main` | 0.00% / 0.0us | 100.00% / 16.124s |  |
| 24 | solver | `(anonymous namespace)::solveInput(...)::CliOptions const&)` | 0.00% / 0.0us | 100.00% / 16.124s |  |
| 25 | solver | `tsp::BranchBoundSolver::solve(tsp::BranchStrategy)` | 0.00% / 0.0us | 100.00% / 16.124s |  |

## Scope Total

- Selected function self share: `97.44%` of program Ir.
- Selected function estimated self time: `15.711s`.
- Full CSV: `function_time_breakdown.csv`.
