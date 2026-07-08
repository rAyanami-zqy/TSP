# Function Time Share - dj38

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `910,025,230` Ir.
- Native elapsed: `125.6ms`.
- Callgrind elapsed: `8.318s`.

## Solver Stats

- `cost`: `6656`
- `initial_upper_bound`: `6656`
- `method`: `exact`
- `nodes_created`: `25733`
- `nodes_expanded`: `7990`
- `pruned_bound`: `12867`
- `pruned_infeasible`: `0`
- `root_lower_bound`: `6285`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 30.13% / 37.8ms | 30.66% / 38.5ms |  |
| 2 | solver | `{lambda(int)#2}::operator()(int) const` | 21.93% / 27.5ms | 1670.30% / 2.097s |  |
| 3 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 19.68% / 24.7ms | 23.65% / 29.7ms |  |
| 4 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 8.30% / 10.4ms | 8.30% / 10.4ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 3.88% / 4.9ms | 4.27% / 5.4ms |  |
| 6 | solver | `tsp::BranchBoundSolver::bpPartition(...)` | 3.58% / 4.5ms | 23.27% / 29.2ms |  |
| 7 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 2.06% / 2.6ms | 3.69% / 4.6ms |  |
| 8 | runtime | `./malloc/./malloc/malloc.c:malloc` | 1.76% / 2.2ms | 6.06% / 7.6ms |  |
| 9 | runtime | `./malloc/./malloc/malloc.c:free` | 1.12% / 1.4ms | 4.93% / 6.2ms |  |
| 10 | solver | `{lambda(int, double, int)#1}>::_M_invoke(...)` | 1.09% / 1.4ms | 3.08% / 3.9ms |  |
| 11 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 1.04% / 1.3ms | 1.63% / 2.0ms |  |
| 12 | runtime | `operator new(unsigned long)` | 0.64% / 803.6us | 6.78% / 8.5ms |  |
| 13 | runtime | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.61% / 766.0us | 0.00% / 0.0us |  |
| 14 | solver | `tsp::BranchBoundSolver::twoOpt(std::vector<>&, double&) const` | 0.51% / 640.4us | 0.00% / 0.0us |  |
| 15 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 0.39% / 489.7us | 1.02% / 1.3ms |  |
| 16 | runtime | `./malloc/./malloc/malloc.c:_int_free_maybe_consolidate` | 0.35% / 439.5us | 0.00% / 0.0us |  |
| 17 | solver | `operator()(...)::LevelChanges&) const` | 0.26% / 326.5us | 0.00% / 0.0us |  |
| 18 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.24% / 301.4us | 0.00% / 0.0us |  |
| 19 | solver | `tsp::BranchBoundSolver::search(...)` | 0.21% / 263.7us | 980.00% / 1.231s |  |
| 20 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.20% / 251.1us | 0.00% / 0.0us |  |
| 21 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.18% / 226.0us | 0.00% / 0.0us |  |
| 22 | solver | `std::vector<>::reserve(unsigned long)` | 0.15% / 188.4us | 0.77% / 966.9us |  |
| 23 | runtime | `void std::vector<>::_M_realloc_insert<>(...)` | 0.13% / 163.2us | 0.00% / 0.0us |  |
| 24 | runtime | `./malloc/./malloc/arena.c:free` | 0.12% / 150.7us | 0.00% / 0.0us |  |
| 25 | solver | `operator()(...)::LevelChanges const&) const` | 0.09% / 113.0us | 0.00% / 0.0us |  |
| 26 | runtime | `operator delete(void*)` | 0.08% / 100.5us | 5.09% / 6.4ms |  |
| 27 | solver | `std::_Function_handler<>::_M_invoke(std::_Any_data const&, int&&)` | 0.00% / 0.0us | 1670.30% / 2.097s |  |
| 28 | solver | `(below main)` | 0.00% / 0.0us | 99.78% / 125.3ms |  |
| 29 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 99.78% / 125.3ms |  |
| 30 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 99.78% / 125.3ms |  |

## Scope Total

- Selected function self share: `98.73%` of program Ir.
- Selected function estimated self time: `124.0ms`.
- Full CSV: `function_time_breakdown.csv`.
