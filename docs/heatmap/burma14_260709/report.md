# Function Time Share - burma14

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `258,068,469` Ir.
- Native elapsed: `58.3ms`.
- Callgrind elapsed: `2.643s`.

## Solver Stats

- `cost`: `3323`
- `initial_upper_bound`: `3323`
- `method`: `exact`
- `nodes_created`: `36847`
- `nodes_expanded`: `9217`
- `pruned_bound`: `18399`
- `pruned_infeasible`: `28`
- `root_lower_bound`: `2542`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 19.37% / 11.3ms | 37.33% / 21.8ms |  |
| 2 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 13.93% / 8.1ms | 16.42% / 9.6ms |  |
| 3 | solver | `{lambda(int)#2}::operator()(int) const` | 11.26% / 6.6ms | 1303.20% / 760.0ms |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 10.05% / 5.9ms | 11.37% / 6.6ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:malloc` | 8.16% / 4.8ms | 14.93% / 8.7ms |  |
| 6 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 5.31% / 3.1ms | 6.73% / 3.9ms |  |
| 7 | runtime | `./malloc/./malloc/malloc.c:free` | 5.09% / 3.0ms | 17.01% / 9.9ms |  |
| 8 | solver | `tsp::BranchBoundSolver::bpPartition(...)` | 4.85% / 2.8ms | 35.22% / 20.5ms |  |
| 9 | runtime | `operator new(unsigned long)` | 2.91% / 1.7ms | 18.20% / 10.6ms |  |
| 10 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 2.70% / 1.6ms | 2.70% / 1.6ms |  |
| 11 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 1.96% / 1.1ms | 5.17% / 3.0ms |  |
| 12 | solver | `operator()(...)::LevelChanges&) const` | 1.34% / 781.4us | 1.34% / 781.4us |  |
| 13 | solver | `{lambda(int, double, int)#1}>::_M_invoke(...)` | 1.08% / 629.8us | 2.84% / 1.7ms |  |
| 14 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 1.01% / 589.0us | 2.14% / 1.2ms |  |
| 15 | solver | `tsp::BranchBoundSolver::search(...)` | 0.95% / 554.0us | 696.40% / 406.1ms |  |
| 16 | runtime | `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.93% / 542.3us | 1.06% / 618.1us |  |
| 17 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 0.88% / 513.2us | 1.33% / 775.6us |  |
| 18 | solver | `std::vector<>::reserve(unsigned long)` | 0.71% / 414.0us | 3.06% / 1.8ms |  |
| 19 | runtime | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.63% / 367.4us | 0.00% / 0.0us |  |
| 20 | runtime | `./malloc/./malloc/arena.c:free` | 0.55% / 320.7us | 0.00% / 0.0us |  |
| 21 | solver | `operator()(...)::LevelChanges const&) const` | 0.47% / 274.1us | 0.00% / 0.0us |  |
| 22 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.42% / 244.9us | 0.00% / 0.0us |  |
| 23 | runtime | `operator delete(void*)` | 0.36% / 209.9us | 17.74% / 10.3ms |  |
| 24 | runtime | `operator delete(void*, unsigned long)` | 0.36% / 209.9us | 18.46% / 10.8ms |  |
| 25 | runtime | `./malloc/./malloc/malloc.c:_int_free_maybe_consolidate` | 0.30% / 174.9us | 0.00% / 0.0us |  |
| 26 | runtime | `std::vector<>::vector(std::vector<> const&)` | 0.28% / 163.3us | 0.84% / 489.8us |  |
| 27 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.27% / 157.4us | 0.00% / 0.0us |  |
| 28 | solver | `std::vector<>::reserve(unsigned long)` | 0.18% / 105.0us | 2.91% / 1.7ms |  |
| 29 | runtime | `void std::vector<>::_M_realloc_insert<>(...)` | 0.15% / 87.5us | 0.00% / 0.0us |  |
| 30 | solver | `tsp::BranchBoundSolver::lkSearch(...) const` | 0.09% / 52.5us | 1.91% / 1.1ms |  |

## Scope Total

- Selected function self share: `96.64%` of program Ir.
- Selected function estimated self time: `56.4ms`.
- Full CSV: `function_time_breakdown.csv`.
