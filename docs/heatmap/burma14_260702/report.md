# Function Time Share - burma14

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `250,955,896` Ir.
- Native elapsed: `56.6ms`.
- Callgrind elapsed: `2.945s`.

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
| 1 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 15.29% / 8.7ms | 15.56% / 8.8ms |  |
| 2 | runtime | `void std::vector<>::_M_realloc_insert<>(...)` | 11.80% / 6.7ms | 34.11% / 19.3ms |  |
| 3 | runtime | `./malloc/./malloc/malloc.c:malloc` | 11.16% / 6.3ms | 13.94% / 7.9ms |  |
| 4 | solver | `BranchBoundSolver::updateOneTreeAfterCandidateRemoval(...) const` | 10.58% / 6.0ms | 42.96% / 24.3ms |  |
| 5 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 9.27% / 5.2ms | 13.77% / 7.8ms |  |
| 6 | runtime | `./malloc/./malloc/malloc.c:free` | 7.25% / 4.1ms | 23.59% / 13.3ms |  |
| 7 | solver | `tsp::BranchBoundSolver::searchSmart(...)` | 5.98% / 3.4ms | 1286.80% / 728.1ms |  |
| 8 | runtime | `operator new(unsigned long)` | 4.14% / 2.3ms | 18.60% / 10.5ms |  |
| 9 | solver | `tsp::BranchBoundSolver::updateOneTreeAfterForbid(...) const` | 3.46% / 2.0ms | 26.54% / 15.0ms |  |
| 10 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 2.60% / 1.5ms | 2.76% / 1.6ms |  |
| 11 | solver | `operator()<>({lambda(auto:1&, int, int)#1}&, int, int) const` | 2.37% / 1.3ms | 2.40% / 1.4ms |  |
| 12 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 1.84% / 1.0ms | 1.84% / 1.0ms |  |
| 13 | solver | `operator()<>({lambda(auto:1&, int, int)#1}&, int, int) const` | 1.70% / 961.9us | 1.76% / 995.8us |  |
| 14 | solver | `tsp::BranchBoundSolver::chooseSmartBranchEdge(...) const` | 1.17% / 662.0us | 1.17% / 662.0us |  |
| 15 | solver | `{lambda(int, double, int)#1}>::_M_invoke(...)` | 1.11% / 628.0us | 2.92% / 1.7ms |  |
| 16 | runtime | `void std::vector<>::_M_realloc_insert<>(...)` | 1.04% / 588.4us | 3.26% / 1.8ms |  |
| 17 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.94% / 531.9us | 1.99% / 1.1ms |  |
| 18 | runtime | `./malloc/./malloc/arena.c:free` | 0.78% / 441.3us | 0.00% / 0.0us |  |
| 19 | runtime | `std::vector<>::vector(...)` | 0.58% / 328.2us | 1.32% / 746.9us |  |
| 20 | runtime | `operator delete(void*)` | 0.52% / 294.2us | 24.63% / 13.9ms |  |
| 21 | runtime | `operator delete(void*, unsigned long)` | 0.52% / 294.2us | 25.66% / 14.5ms |  |
| 22 | runtime | `std::vector<std::vector<>, std::allocator<> >::~vector()` | 0.44% / 249.0us | 5.02% / 2.8ms |  |
| 23 | runtime | `std::vector<>::operator=(std::vector<> const&)` | 0.31% / 175.4us | 0.88% / 497.9us |  |
| 24 | runtime | `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.28% / 158.4us | 0.00% / 0.0us |  |
| 25 | runtime | `std::vector<int, std::allocator<int> >::~vector()` | 0.18% / 101.8us | 2.20% / 1.2ms |  |
| 26 | solver | `tsp::BranchBoundSolver::shouldPrune(double, double) const` | 0.14% / 79.2us | 0.00% / 0.0us |  |
| 27 | solver | `std::vector<>::~vector()` | 0.13% / 73.6us | 1.77% / 1.0ms |  |
| 28 | solver | `tsp::BranchBoundSolver::lkSearch(...) const` | 0.10% / 56.6us | 1.96% / 1.1ms |  |
| 29 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.09% / 50.9us | 0.00% / 0.0us |  |
| 30 | solver | `(below main)` | 0.00% / 0.0us | 99.21% / 56.1ms |  |

## Scope Total

- Selected function self share: `95.77%` of program Ir.
- Selected function estimated self time: `54.2ms`.
- Full CSV: `function_time_breakdown.csv`.
