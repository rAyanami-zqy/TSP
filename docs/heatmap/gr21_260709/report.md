# Function Time Share - gr21

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `1,049,585,792` Ir.
- Native elapsed: `173.2ms`.
- Callgrind elapsed: `8.731s`.

## Solver Stats

- `cost`: `2707`
- `initial_upper_bound`: `2707`
- `method`: `exact`
- `nodes_created`: `78479`
- `nodes_expanded`: `24876`
- `pruned_bound`: `38775`
- `pruned_infeasible`: `926`
- `root_lower_bound`: `2252`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 21.93% / 38.0ms | 23.35% / 40.4ms |  |
| 2 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 21.55% / 37.3ms | 32.04% / 55.5ms |  |
| 3 | solver | `{lambda(int)#2}::operator()(int) const` | 16.47% / 28.5ms | 1514.70% / 2.623s |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 7.08% / 12.3ms | 8.87% / 15.4ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 5.52% / 9.6ms | 7.47% / 12.9ms |  |
| 6 | runtime | `./malloc/./malloc/malloc.c:malloc` | 4.47% / 7.7ms | 13.39% / 23.2ms |  |
| 7 | solver | `tsp::BranchBoundSolver::bpPartition(...)` | 4.40% / 7.6ms | 29.61% / 51.3ms |  |
| 8 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 3.49% / 6.0ms | 3.49% / 6.0ms |  |
| 9 | runtime | `./malloc/./malloc/malloc.c:free` | 2.80% / 4.8ms | 10.57% / 18.3ms |  |
| 10 | runtime | `operator new(unsigned long)` | 1.60% / 2.8ms | 15.20% / 26.3ms |  |
| 11 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 1.25% / 2.2ms | 1.95% / 3.4ms |  |
| 12 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 1.02% / 1.8ms | 2.71% / 4.7ms |  |
| 13 | runtime | `./malloc/./malloc/malloc.c:malloc_consolidate` | 1.02% / 1.8ms | 1.14% / 2.0ms |  |
| 14 | runtime | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 1.02% / 1.8ms | 1.02% / 1.8ms |  |
| 15 | solver | `operator()(...)::LevelChanges&) const` | 0.69% / 1.2ms | 0.00% / 0.0us |  |
| 16 | solver | `tsp::BranchBoundSolver::search(...)` | 0.55% / 952.4us | 860.80% / 1.491s |  |
| 17 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.54% / 935.1us | 1.40% / 2.4ms |  |
| 18 | runtime | `./malloc/./malloc/malloc.c:_int_free_maybe_consolidate` | 0.42% / 727.3us | 0.00% / 0.0us |  |
| 19 | solver | `std::vector<>::reserve(unsigned long)` | 0.39% / 675.3us | 2.09% / 3.6ms |  |
| 20 | solver | `{lambda(int, double, int)#1}>::_M_invoke(...)` | 0.38% / 658.0us | 1.06% / 1.8ms |  |
| 21 | runtime | `./malloc/./malloc/arena.c:free` | 0.30% / 519.5us | 0.00% / 0.0us |  |
| 22 | solver | `operator()(...)::LevelChanges const&) const` | 0.24% / 415.6us | 0.00% / 0.0us |  |
| 23 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.23% / 398.3us | 0.00% / 0.0us |  |
| 24 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.22% / 381.0us | 0.00% / 0.0us |  |
| 25 | runtime | `operator delete(void*)` | 0.20% / 346.3us | 10.97% / 19.0ms |  |
| 26 | runtime | `operator delete(void*, unsigned long)` | 0.20% / 346.3us | 11.37% / 19.7ms |  |
| 27 | runtime | `std::vector<>::vector(std::vector<> const&)` | 0.15% / 259.7us | 0.00% / 0.0us |  |
| 28 | solver | `std::_Function_handler<>::_M_invoke(std::_Any_data const&, int&&)` | 0.00% / 0.0us | 1514.70% / 2.623s |  |
| 29 | solver | `(below main)` | 0.00% / 0.0us | 99.81% / 172.8ms |  |
| 30 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 99.81% / 172.8ms |  |

## Scope Total

- Selected function self share: `98.13%` of program Ir.
- Selected function estimated self time: `169.9ms`.
- Full CSV: `function_time_breakdown.csv`.
