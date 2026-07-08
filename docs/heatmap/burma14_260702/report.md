# Function Time Share - burma14

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `125,968,557` Ir.
- Native elapsed: `38.3ms`.
- Callgrind elapsed: `1.502s`.

## Solver Stats

- `cost`: `3323`
- `initial_upper_bound`: `3323`
- `method`: `exact`
- `nodes_created`: `18111`
- `nodes_expanded`: `9162`
- `pruned_bound`: `8949`
- `pruned_infeasible`: `214`
- `root_lower_bound`: `2542`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 21.12% / 8.1ms | 35.88% / 13.7ms |  |
| 2 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 18.14% / 6.9ms | 27.30% / 10.4ms |  |
| 3 | solver | `{lambda(bool)#1}::operator()(bool) const` | 14.04% / 5.4ms | 1212.40% / 464.0ms |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 9.39% / 3.6ms | 9.70% / 3.7ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:malloc` | 7.75% / 3.0ms | 8.74% / 3.3ms |  |
| 6 | runtime | `./malloc/./malloc/malloc.c:free` | 4.83% / 1.8ms | 15.05% / 5.8ms |  |
| 7 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 4.54% / 1.7ms | 11.90% / 4.6ms |  |
| 8 | runtime | `operator new(unsigned long)` | 2.76% / 1.1ms | 11.84% / 4.5ms |  |
| 9 | solver | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | 2.73% / 1.0ms | 2.73% / 1.0ms |  |
| 10 | solver | `{lambda(int, double, int)#1}>::_M_invoke(...)` | 2.20% / 841.9us | 5.82% / 2.2ms |  |
| 11 | solver | `tsp::BranchBoundSolver::search(...)` | 1.77% / 677.3us | 1255.80% / 480.6ms |  |
| 12 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 1.65% / 631.4us | 1.65% / 631.4us |  |
| 13 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 1.02% / 390.3us | 1.96% / 750.0us |  |
| 14 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 0.85% / 325.3us | 0.97% / 371.2us |  |
| 15 | runtime | `./malloc/./malloc/arena.c:free` | 0.52% / 199.0us | 0.00% / 0.0us |  |
| 16 | solver | `operator()(...)::LevelChanges const&) const` | 0.46% / 176.0us | 0.00% / 0.0us |  |
| 17 | runtime | `operator delete(void*)` | 0.35% / 133.9us | 15.75% / 6.0ms |  |
| 18 | runtime | `operator delete(void*, unsigned long)` | 0.35% / 133.9us | 16.43% / 6.3ms |  |
| 19 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.28% / 107.1us | 0.00% / 0.0us |  |
| 20 | runtime | `void std::vector<>::_M_realloc_insert<>(...)` | 0.27% / 103.3us | 0.00% / 0.0us |  |
| 21 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 0.20% / 76.5us | 0.00% / 0.0us |  |
| 22 | solver | `tsp::BranchBoundSolver::lkSearch(...) const` | 0.20% / 76.5us | 3.91% / 1.5ms |  |
| 23 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.17% / 65.1us | 0.00% / 0.0us |  |
| 24 | solver | `tsp::BranchBoundSolver::twoOpt(std::vector<>&, double&) const` | 0.11% / 42.1us | 0.00% / 0.0us |  |
| 25 | solver | `(below main)` | 0.00% / 0.0us | 98.42% / 37.7ms |  |
| 26 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 98.42% / 37.7ms |  |
| 27 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 98.42% / 37.7ms |  |
| 28 | solver | `main` | 0.00% / 0.0us | 98.42% / 37.7ms |  |
| 29 | solver | `(anonymous namespace)::solveInput(...)::CliOptions const&)` | 0.00% / 0.0us | 98.38% / 37.6ms |  |
| 30 | solver | `tsp::BranchBoundSolver::solve(tsp::BranchStrategy)` | 0.00% / 0.0us | 98.13% / 37.6ms |  |

## Scope Total

- Selected function self share: `95.70%` of program Ir.
- Selected function estimated self time: `36.6ms`.
- Full CSV: `function_time_breakdown.csv`.
