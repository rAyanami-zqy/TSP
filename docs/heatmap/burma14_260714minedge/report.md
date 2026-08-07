# Function Time Share - burma14

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `229,601,724` Ir.
- Native elapsed: `55.7ms`.
- Callgrind elapsed: `2.558s`.

## Solver Stats

- `cost`: `3323`
- `initial_upper_bound`: `3323`
- `method`: `exact`
- `nodes_created`: `18362`
- `nodes_expanded`: `9152`
- `pruned_bound`: `9210`
- `pruned_infeasible`: `28`
- `root_lower_bound`: `2542`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `BranchBoundSolver::updateOneTreeAfterCandidateRemoval(...) const` | 24.57% / 13.7ms | 27.99% / 15.6ms |  |
| 2 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 18.87% / 10.5ms | 29.16% / 16.2ms |  |
| 3 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 7.95% / 4.4ms | 8.13% / 4.5ms |  |
| 4 | solver | `tsp::BranchBoundSolver::updateOneTreeAfterForbid(...) const` | 6.74% / 3.8ms | 15.48% / 8.6ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:malloc` | 6.51% / 3.6ms | 7.29% / 4.1ms |  |
| 6 | solver | `tsp::BranchBoundSolver::search(...)` | 6.30% / 3.5ms | 677.70% / 377.5ms |  |
| 7 | solver | `tsp::(anonymous namespace)::DisjointSet::unite(int, int)` | 4.91% / 2.7ms | 4.91% / 2.7ms |  |
| 8 | solver | `tsp::BranchBoundSolver::bpPartition(...)` | 4.57% / 2.5ms | 22.93% / 12.8ms |  |
| 9 | runtime | `./malloc/./malloc/malloc.c:free` | 4.07% / 2.3ms | 12.64% / 7.0ms |  |
| 10 | runtime | `void std::vector<>::_M_realloc_insert<>(...)` | 2.41% / 1.3ms | 7.56% / 4.2ms |  |
| 11 | runtime | `operator new(unsigned long)` | 2.33% / 1.3ms | 9.91% / 5.5ms |  |
| 12 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 1.72% / 958.2us | 1.72% / 958.2us |  |
| 13 | solver | `{lambda(int, double, int)#1}>::_M_invoke(...)` | 1.21% / 674.1us | 3.20% / 1.8ms |  |
| 14 | solver | `tsp::(anonymous namespace)::DisjointSet::DisjointSet(int)` | 0.82% / 456.8us | 2.19% / 1.2ms |  |
| 15 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 0.66% / 367.7us | 0.00% / 0.0us |  |
| 16 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.52% / 289.7us | 0.00% / 0.0us |  |
| 17 | runtime | `./malloc/./malloc/arena.c:free` | 0.44% / 245.1us | 0.00% / 0.0us |  |
| 18 | runtime | `std::vector<>::operator=(std::vector<> const&)` | 0.34% / 189.4us | 0.98% / 546.0us |  |
| 19 | runtime | `operator delete(void*)` | 0.29% / 161.6us | 13.23% / 7.4ms |  |
| 20 | runtime | `operator delete(void*, unsigned long)` | 0.29% / 161.6us | 13.81% / 7.7ms |  |
| 21 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.28% / 156.0us | 0.00% / 0.0us |  |
| 22 | runtime | `std::vector<int, std::allocator<int> >::~vector()` | 0.24% / 133.7us | 2.95% / 1.6ms |  |
| 23 | runtime | `std::vector<>::reserve(unsigned long)` | 0.16% / 89.1us | 0.00% / 0.0us |  |
| 24 | runtime | `void std::vector<>::_M_realloc_insert<>(...)` | 0.16% / 89.1us | 0.00% / 0.0us |  |
| 25 | solver | `tsp::BranchBoundSolver::shouldPrune(double, double) const` | 0.15% / 83.6us | 0.00% / 0.0us |  |
| 26 | solver | `std::vector<>::~vector()` | 0.15% / 83.6us | 1.79% / 997.2us |  |
| 27 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 0.12% / 66.9us | 0.00% / 0.0us |  |
| 28 | solver | `tsp::BranchBoundSolver::lkSearch(...) const` | 0.11% / 61.3us | 2.15% / 1.2ms |  |
| 29 | runtime | `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.09% / 50.1us | 0.00% / 0.0us |  |
| 30 | solver | `(below main)` | 0.00% / 0.0us | 99.13% / 55.2ms |  |

## Scope Total

- Selected function self share: `96.98%` of program Ir.
- Selected function estimated self time: `54.0ms`.
- Full CSV: `function_time_breakdown.csv`.
