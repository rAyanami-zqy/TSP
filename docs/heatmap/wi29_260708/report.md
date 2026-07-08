# Function Time Share - wi29

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `1,528,799,306` Ir.
- Native elapsed: `225.4ms`.
- Callgrind elapsed: `11.957s`.

## Solver Stats

- `cost`: `27603`
- `initial_upper_bound`: `27603`
- `method`: `exact`
- `nodes_created`: `77709`
- `nodes_expanded`: `22423`
- `pruned_bound`: `38843`
- `pruned_infeasible`: `24`
- `root_lower_bound`: `22727`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 26.82% / 60.4ms | 27.74% / 62.5ms |  |
| 2 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 24.13% / 54.4ms | 31.01% / 69.9ms |  |
| 3 | solver | `{lambda(int)#2}::operator()(int) const` | 20.04% / 45.2ms | 1813.10% / 4.086s |  |
| 4 | solver | `tsp::BranchBoundSolver::bpPartition(...)` | 4.22% / 9.5ms | 25.74% / 58.0ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 4.22% / 9.5ms | 4.99% / 11.2ms |  |
| 6 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 3.63% / 8.2ms | 5.03% / 11.3ms |  |
| 7 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 3.28% / 7.4ms | 3.28% / 7.4ms |  |
| 8 | runtime | `./malloc/./malloc/malloc.c:malloc` | 3.00% / 6.8ms | 8.03% / 18.1ms |  |
| 9 | runtime | `./malloc/./malloc/malloc.c:free` | 1.88% / 4.2ms | 7.12% / 16.0ms |  |
| 10 | runtime | `operator new(unsigned long)` | 1.08% / 2.4ms | 9.24% / 20.8ms |  |
| 11 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 0.89% / 2.0ms | 1.40% / 3.2ms |  |
| 12 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 0.70% / 1.6ms | 1.84% / 4.1ms |  |
| 13 | runtime | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.64% / 1.4ms | 0.00% / 0.0us |  |
| 14 | solver | `{lambda(int, double, int)#1}>::_M_invoke(...)` | 0.58% / 1.3ms | 1.65% / 3.7ms |  |
| 15 | solver | `operator()(...)::LevelChanges&) const` | 0.47% / 1.1ms | 0.00% / 0.0us |  |
| 16 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.39% / 879.0us | 0.00% / 0.0us |  |
| 17 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.36% / 811.4us | 0.00% / 0.0us |  |
| 18 | solver | `tsp::BranchBoundSolver::search(...)` | 0.36% / 811.4us | 1017.00% / 2.292s |  |
| 19 | runtime | `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.34% / 766.3us | 0.00% / 0.0us |  |
| 20 | runtime | `./malloc/./malloc/malloc.c:_int_free_maybe_consolidate` | 0.30% / 676.1us | 0.00% / 0.0us |  |
| 21 | solver | `std::vector<>::reserve(unsigned long)` | 0.26% / 586.0us | 1.41% / 3.2ms |  |
| 22 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.21% / 473.3us | 0.00% / 0.0us |  |
| 23 | runtime | `./malloc/./malloc/arena.c:free` | 0.20% / 450.8us | 0.00% / 0.0us |  |
| 24 | solver | `operator()(...)::LevelChanges const&) const` | 0.17% / 383.1us | 0.00% / 0.0us |  |
| 25 | solver | `tsp::BranchBoundSolver::twoOpt(std::vector<>&, double&) const` | 0.15% / 338.1us | 0.00% / 0.0us |  |
| 26 | runtime | `operator delete(void*)` | 0.13% / 293.0us | 7.38% / 16.6ms |  |
| 27 | runtime | `operator delete(void*, unsigned long)` | 0.13% / 293.0us | 7.65% / 17.2ms |  |
| 28 | solver | `std::_Function_handler<>::_M_invoke(std::_Any_data const&, int&&)` | 0.00% / 0.0us | 1813.10% / 4.086s |  |
| 29 | solver | `(below main)` | 0.00% / 0.0us | 99.87% / 225.1ms |  |
| 30 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 99.87% / 225.1ms |  |

## Scope Total

- Selected function self share: `98.58%` of program Ir.
- Selected function estimated self time: `222.2ms`.
- Full CSV: `function_time_breakdown.csv`.
