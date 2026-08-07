# Function Time Share - burma14

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `119,115,966` Ir.
- Native elapsed: `36.1ms`.
- Callgrind elapsed: `2.096s`.

## Solver Stats

- `cost`: `3323`
- `initial_upper_bound`: `3323`
- `method`: `exact`
- `nodes_created`: `8949`
- `nodes_expanded`: `4582`
- `pruned_bound`: `4367`
- `pruned_infeasible`: `214`
- `root_lower_bound`: `2542`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `BranchBoundSolver::updateOneTreeAfterCandidateRemoval(...) const` | 23.69% / 8.6ms | 27.05% / 9.8ms |  |
| 2 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 19.52% / 7.0ms | 29.03% / 10.5ms |  |
| 3 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 7.60% / 2.7ms | 7.87% / 2.8ms |  |
| 4 | solver | `tsp::BranchBoundSolver::updateOneTreeAfterForbid(...) const` | 6.90% / 2.5ms | 14.10% / 5.1ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:malloc` | 6.21% / 2.2ms | 7.04% / 2.5ms |  |
| 6 | solver | `tsp::BranchBoundSolver::search(...)` | 6.10% / 2.2ms | 571.30% / 206.3ms |  |
| 7 | solver | `tsp::BranchBoundSolver::bpPartition(...)` | 4.66% / 1.7ms | 21.50% / 7.8ms |  |
| 8 | solver | `tsp::(anonymous namespace)::DisjointSet::unite(int, int)` | 4.05% / 1.5ms | 4.05% / 1.5ms |  |
| 9 | runtime | `./malloc/./malloc/malloc.c:free` | 3.89% / 1.4ms | 12.17% / 4.4ms |  |
| 10 | solver | `{lambda(int, double, int)#1}>::_M_invoke(...)` | 2.33% / 841.2us | 6.16% / 2.2ms |  |
| 11 | runtime | `operator new(unsigned long)` | 2.22% / 801.5us | 9.53% / 3.4ms |  |
| 12 | runtime | `void std::vector<>::_M_realloc_insert<>(...)` | 2.21% / 797.9us | 6.93% / 2.5ms |  |
| 13 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 1.71% / 617.4us | 1.71% / 617.4us |  |
| 14 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 0.73% / 263.6us | 0.00% / 0.0us |  |
| 15 | solver | `tsp::(anonymous namespace)::DisjointSet::DisjointSet(int)` | 0.68% / 245.5us | 1.81% / 653.5us |  |
| 16 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.48% / 173.3us | 0.00% / 0.0us |  |
| 17 | runtime | `./malloc/./malloc/arena.c:free` | 0.42% / 151.6us | 0.00% / 0.0us |  |
| 18 | runtime | `std::vector<>::operator=(std::vector<> const&)` | 0.32% / 115.5us | 0.92% / 332.2us |  |
| 19 | runtime | `void std::vector<>::_M_realloc_insert<>(...)` | 0.29% / 104.7us | 0.00% / 0.0us |  |
| 20 | runtime | `operator delete(void*)` | 0.28% / 101.1us | 12.73% / 4.6ms |  |
| 21 | runtime | `operator delete(void*, unsigned long)` | 0.28% / 101.1us | 13.28% / 4.8ms |  |
| 22 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.27% / 97.5us | 0.00% / 0.0us |  |
| 23 | runtime | `std::vector<int, std::allocator<int> >::~vector()` | 0.21% / 75.8us | 2.60% / 938.7us |  |
| 24 | solver | `tsp::BranchBoundSolver::lkSearch(...) const` | 0.21% / 75.8us | 4.14% / 1.5ms |  |
| 25 | runtime | `std::vector<>::reserve(unsigned long)` | 0.15% / 54.2us | 0.00% / 0.0us |  |
| 26 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 0.14% / 50.5us | 0.00% / 0.0us |  |
| 27 | solver | `tsp::BranchBoundSolver::shouldPrune(double, double) const` | 0.14% / 50.5us | 0.00% / 0.0us |  |
| 28 | solver | `std::vector<>::~vector()` | 0.14% / 50.5us | 1.74% / 628.2us |  |
| 29 | solver | `tsp::BranchBoundSolver::twoOpt(std::vector<>&, double&) const` | 0.12% / 43.3us | 0.00% / 0.0us |  |
| 30 | runtime | `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.11% / 39.7us | 0.00% / 0.0us |  |

## Scope Total

- Selected function self share: `96.06%` of program Ir.
- Selected function estimated self time: `34.7ms`.
- Full CSV: `function_time_breakdown.csv`.
