# Function Time Share - gr21

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `468,190,922` Ir.
- Native elapsed: `89.6ms`.
- Callgrind elapsed: `4.135s`.

## Solver Stats

- `cost`: `2707`
- `initial_upper_bound`: `2707`
- `method`: `exact`
- `nodes_created`: `34213`
- `nodes_expanded`: `9911`
- `pruned_bound`: `16899`
- `pruned_infeasible`: `416`
- `root_lower_bound`: `2252`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 22.50% / 20.2ms | 23.85% / 21.4ms |  |
| 2 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 21.29% / 19.1ms | 31.36% / 28.1ms |  |
| 3 | solver | `{lambda(int)#2}::operator()(int) const` | 17.07% / 15.3ms | 1381.30% / 1.238s |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 6.59% / 5.9ms | 8.18% / 7.3ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 5.28% / 4.7ms | 7.11% / 6.4ms |  |
| 6 | runtime | `./malloc/./malloc/malloc.c:malloc` | 4.29% / 3.8ms | 12.52% / 11.2ms |  |
| 7 | solver | `tsp::BranchBoundSolver::bpPartition(...)` | 4.27% / 3.8ms | 28.28% / 25.3ms |  |
| 8 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 3.38% / 3.0ms | 3.38% / 3.0ms |  |
| 9 | runtime | `./malloc/./malloc/malloc.c:free` | 2.69% / 2.4ms | 10.09% / 9.0ms |  |
| 10 | runtime | `operator new(unsigned long)` | 1.54% / 1.4ms | 14.25% / 12.8ms |  |
| 11 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 1.17% / 1.0ms | 1.83% / 1.6ms |  |
| 12 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 1.00% / 896.2us | 2.65% / 2.4ms |  |
| 13 | runtime | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.96% / 860.3us | 0.00% / 0.0us |  |
| 14 | runtime | `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.88% / 788.6us | 0.98% / 878.2us |  |
| 15 | solver | `{lambda(int, double, int)#1}>::_M_invoke(...)` | 0.86% / 770.7us | 2.38% / 2.1ms |  |
| 16 | solver | `operator()(...)::LevelChanges&) const` | 0.67% / 600.4us | 0.00% / 0.0us |  |
| 17 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.53% / 475.0us | 1.35% / 1.2ms |  |
| 18 | solver | `tsp::BranchBoundSolver::search(...)` | 0.51% / 457.0us | 640.80% / 574.3ms |  |
| 19 | runtime | `./malloc/./malloc/malloc.c:_int_free_maybe_consolidate` | 0.39% / 349.5us | 0.00% / 0.0us |  |
| 20 | solver | `std::vector<>::reserve(unsigned long)` | 0.38% / 340.5us | 1.85% / 1.7ms |  |
| 21 | runtime | `./malloc/./malloc/arena.c:free` | 0.29% / 259.9us | 0.00% / 0.0us |  |
| 22 | solver | `operator()(...)::LevelChanges const&) const` | 0.23% / 206.1us | 0.00% / 0.0us |  |
| 23 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.23% / 206.1us | 0.00% / 0.0us |  |
| 24 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.20% / 179.2us | 0.00% / 0.0us |  |
| 25 | runtime | `operator delete(void*)` | 0.19% / 170.3us | 10.48% / 9.4ms |  |
| 26 | runtime | `operator delete(void*, unsigned long)` | 0.19% / 170.3us | 10.86% / 9.7ms |  |
| 27 | runtime | `std::vector<>::vector(std::vector<> const&)` | 0.14% / 125.5us | 0.00% / 0.0us |  |
| 28 | solver | `tsp::BranchBoundSolver::twoOpt(std::vector<>&, double&) const` | 0.12% / 107.5us | 0.00% / 0.0us |  |
| 29 | solver | `std::_Function_handler<>::_M_invoke(std::_Any_data const&, int&&)` | 0.00% / 0.0us | 1381.30% / 1.238s |  |
| 30 | solver | `(below main)` | 0.00% / 0.0us | 99.58% / 89.2ms |  |

## Scope Total

- Selected function self share: `97.84%` of program Ir.
- Selected function estimated self time: `87.7ms`.
- Full CSV: `function_time_breakdown.csv`.
