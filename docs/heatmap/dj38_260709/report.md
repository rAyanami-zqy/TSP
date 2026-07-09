# Function Time Share - dj38

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `67,723,820,524` Ir.
- Native elapsed: `8.948s`.
- Callgrind elapsed: `8.85m`.

## Solver Stats

- `cost`: `6656`
- `initial_upper_bound`: `6656`
- `method`: `exact`
- `nodes_created`: `2611955`
- `nodes_expanded`: `790892`
- `pruned_bound`: `1305978`
- `pruned_infeasible`: `0`
- `root_lower_bound`: `6285`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 30.81% / 2.757s | 31.54% / 2.822s |  |
| 2 | solver | `{lambda(int)#2}::operator()(int) const` | 22.21% / 1.987s | 2652.80% / 3.96m |  |
| 3 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 18.37% / 1.644s | 23.82% / 2.131s |  |
| 4 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 5.91% / 528.8ms | 5.91% / 528.8ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 5.43% / 485.9ms | 5.99% / 536.0ms |  |
| 6 | solver | `tsp::BranchBoundSolver::bpPartition(...)` | 4.04% / 361.5ms | 23.89% / 2.138s |  |
| 7 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 2.58% / 230.9ms | 4.82% / 431.3ms |  |
| 8 | runtime | `./malloc/./malloc/malloc.c:malloc` | 2.24% / 200.4ms | 8.27% / 740.0ms |  |
| 9 | runtime | `./malloc/./malloc/malloc.c:free` | 1.42% / 127.1ms | 6.39% / 571.8ms |  |
| 10 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 1.42% / 127.1ms | 2.24% / 200.4ms |  |
| 11 | runtime | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.88% / 78.7ms | 0.88% / 78.7ms |  |
| 12 | runtime | `operator new(unsigned long)` | 0.81% / 72.5ms | 9.18% / 821.4ms |  |
| 13 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 0.53% / 47.4ms | 1.40% / 125.3ms |  |
| 14 | runtime | `./malloc/./malloc/malloc.c:_int_free_maybe_consolidate` | 0.47% / 42.1ms | 0.00% / 0.0us |  |
| 15 | solver | `operator()(...)::LevelChanges&) const` | 0.36% / 32.2ms | 0.00% / 0.0us |  |
| 16 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.33% / 29.5ms | 0.00% / 0.0us |  |
| 17 | solver | `tsp::BranchBoundSolver::search(...)` | 0.28% / 25.1ms | 1857.70% / 2.77m |  |
| 18 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.24% / 21.5ms | 0.00% / 0.0us |  |
| 19 | solver | `std::vector<>::reserve(unsigned long)` | 0.20% / 17.9ms | 1.11% / 99.3ms |  |
| 20 | runtime | `./malloc/./malloc/arena.c:free` | 0.15% / 13.4ms | 0.00% / 0.0us |  |
| 21 | solver | `operator()(...)::LevelChanges const&) const` | 0.13% / 11.6ms | 0.00% / 0.0us |  |
| 22 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.11% / 9.8ms | 0.00% / 0.0us |  |
| 23 | solver | `std::_Function_handler<>::_M_invoke(std::_Any_data const&, int&&)` | 0.00% / 0.0us | 2652.80% / 3.96m |  |
| 24 | solver | `(below main)` | 0.00% / 0.0us | 100.00% / 8.948s |  |
| 25 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 100.00% / 8.948s |  |
| 26 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 100.00% / 8.948s |  |
| 27 | solver | `main` | 0.00% / 0.0us | 100.00% / 8.948s |  |
| 28 | solver | `(anonymous namespace)::solveInput(...)::CliOptions const&)` | 0.00% / 0.0us | 100.00% / 8.948s |  |
| 29 | solver | `tsp::BranchBoundSolver::solve()` | 0.00% / 0.0us | 100.00% / 8.948s |  |
| 30 | runtime | `operator delete(void*, unsigned long)` | 0.00% / 0.0us | 6.80% / 608.5ms |  |

## Scope Total

- Selected function self share: `98.92%` of program Ir.
- Selected function estimated self time: `8.851s`.
- Full CSV: `function_time_breakdown.csv`.
