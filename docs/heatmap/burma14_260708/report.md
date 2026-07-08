# Function Time Share - burma14

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `137,241,840` Ir.
- Native elapsed: `40.7ms`.
- Callgrind elapsed: `1.656s`.

## Solver Stats

- `cost`: `3323`
- `initial_upper_bound`: `3323`
- `method`: `exact`
- `nodes_created`: `18111`
- `nodes_expanded`: `4582`
- `pruned_bound`: `8945`
- `pruned_infeasible`: `214`
- `root_lower_bound`: `2542`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 19.51% / 7.9ms | 36.23% / 14.7ms |  |
| 2 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 14.06% / 5.7ms | 16.32% / 6.6ms |  |
| 3 | solver | `{lambda(int)#2}::operator()(int) const` | 11.56% / 4.7ms | 1157.80% / 471.3ms |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 9.47% / 3.9ms | 10.76% / 4.4ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:malloc` | 7.68% / 3.1ms | 14.16% / 5.8ms |  |
| 6 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 5.08% / 2.1ms | 6.44% / 2.6ms |  |
| 7 | solver | `tsp::BranchBoundSolver::bpPartition(...)` | 4.94% / 2.0ms | 34.18% / 13.9ms |  |
| 8 | runtime | `./malloc/./malloc/malloc.c:free` | 4.79% / 1.9ms | 16.07% / 6.5ms |  |
| 9 | runtime | `operator new(unsigned long)` | 2.74% / 1.1ms | 17.24% / 7.0ms |  |
| 10 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 2.59% / 1.1ms | 2.59% / 1.1ms |  |
| 11 | solver | `{lambda(int, double, int)#1}>::_M_invoke(...)` | 2.02% / 822.3us | 5.35% / 2.2ms |  |
| 12 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 1.81% / 736.8us | 4.78% / 1.9ms |  |
| 13 | solver | `operator()(...)::LevelChanges&) const` | 1.22% / 496.6us | 1.23% / 500.7us |  |
| 14 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.94% / 382.7us | 1.99% / 810.1us |  |
| 15 | runtime | `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.89% / 362.3us | 1.02% / 415.2us |  |
| 16 | solver | `tsp::BranchBoundSolver::search(...)` | 0.88% / 358.2us | 587.90% / 239.3ms |  |
| 17 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 0.86% / 350.1us | 1.28% / 521.1us |  |
| 18 | solver | `std::vector<>::reserve(unsigned long)` | 0.66% / 268.7us | 2.82% / 1.1ms |  |
| 19 | runtime | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.58% / 236.1us | 0.00% / 0.0us |  |
| 20 | runtime | `./malloc/./malloc/arena.c:free` | 0.51% / 207.6us | 0.00% / 0.0us |  |
| 21 | solver | `operator()(...)::LevelChanges const&) const` | 0.42% / 171.0us | 0.00% / 0.0us |  |
| 22 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.40% / 162.8us | 0.00% / 0.0us |  |
| 23 | runtime | `operator delete(void*)` | 0.34% / 138.4us | 16.75% / 6.8ms |  |
| 24 | runtime | `operator delete(void*, unsigned long)` | 0.34% / 138.4us | 17.43% / 7.1ms |  |
| 25 | runtime | `./malloc/./malloc/malloc.c:_int_free_maybe_consolidate` | 0.30% / 122.1us | 0.00% / 0.0us |  |
| 26 | runtime | `void std::vector<>::_M_realloc_insert<>(...)` | 0.26% / 105.8us | 0.79% / 321.6us |  |
| 27 | runtime | `std::vector<>::vector(std::vector<> const&)` | 0.26% / 105.8us | 0.00% / 0.0us |  |
| 28 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.25% / 101.8us | 0.00% / 0.0us |  |
| 29 | solver | `tsp::BranchBoundSolver::lkSearch(...) const` | 0.18% / 73.3us | 3.59% / 1.5ms |  |
| 30 | solver | `std::vector<>::reserve(unsigned long)` | 0.16% / 65.1us | 2.51% / 1.0ms |  |

## Scope Total

- Selected function self share: `95.80%` of program Ir.
- Selected function estimated self time: `39.0ms`.
- Full CSV: `function_time_breakdown.csv`.
