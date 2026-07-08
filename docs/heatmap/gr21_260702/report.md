# Function Time Share - gr21

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `437,536,663` Ir.
- Native elapsed: `80.6ms`.
- Callgrind elapsed: `3.633s`.

## Solver Stats

- `cost`: `2707`
- `initial_upper_bound`: `2707`
- `method`: `exact`
- `nodes_created`: `34213`
- `nodes_expanded`: `17314`
- `pruned_bound`: `16899`
- `pruned_infeasible`: `416`
- `root_lower_bound`: `2252`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 28.51% / 23.0ms | 34.03% / 27.4ms |  |
| 2 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 22.50% / 18.1ms | 30.92% / 24.9ms |  |
| 3 | solver | `{lambda(bool)#1}::operator()(bool) const` | 19.18% / 15.5ms | 1421.70% / 1.146s |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 4.99% / 4.0ms | 5.99% / 4.8ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:malloc` | 4.17% / 3.4ms | 7.38% / 5.9ms |  |
| 6 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 2.71% / 2.2ms | 6.90% / 5.6ms |  |
| 7 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 2.69% / 2.2ms | 3.19% / 2.6ms |  |
| 8 | runtime | `./malloc/./malloc/malloc.c:free` | 2.61% / 2.1ms | 8.88% / 7.2ms |  |
| 9 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 2.01% / 1.6ms | 2.01% / 1.6ms |  |
| 10 | solver | `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | 1.97% / 1.6ms | 1.97% / 1.6ms |  |
| 11 | runtime | `operator new(unsigned long)` | 1.49% / 1.2ms | 9.06% / 7.3ms |  |
| 12 | solver | `tsp::BranchBoundSolver::search(...)` | 0.97% / 782.0us | 1457.20% / 1.175s |  |
| 13 | solver | `{lambda(int, double, int)#1}>::_M_invoke(...)` | 0.92% / 741.7us | 2.54% / 2.0ms |  |
| 14 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 0.64% / 516.0us | 1.01% / 814.3us |  |
| 15 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.56% / 451.5us | 1.07% / 862.6us |  |
| 16 | runtime | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.42% / 338.6us | 0.00% / 0.0us |  |
| 17 | runtime | `./malloc/./malloc/arena.c:free` | 0.28% / 225.7us | 0.00% / 0.0us |  |
| 18 | solver | `operator()(...)::LevelChanges const&) const` | 0.25% / 201.5us | 0.00% / 0.0us |  |
| 19 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.24% / 193.5us | 0.00% / 0.0us |  |
| 20 | runtime | `./malloc/./malloc/malloc.c:_int_free_maybe_consolidate` | 0.22% / 177.4us | 0.00% / 0.0us |  |
| 21 | runtime | `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.21% / 169.3us | 0.00% / 0.0us |  |
| 22 | runtime | `operator delete(void*)` | 0.19% / 153.2us | 9.26% / 7.5ms |  |
| 23 | runtime | `operator delete(void*, unsigned long)` | 0.19% / 153.2us | 9.63% / 7.8ms |  |
| 24 | solver | `tsp::BranchBoundSolver::twoOpt(std::vector<>&, double&) const` | 0.13% / 104.8us | 0.00% / 0.0us |  |
| 25 | solver | `(below main)` | 0.00% / 0.0us | 99.55% / 80.3ms |  |
| 26 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 99.55% / 80.3ms |  |
| 27 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 99.55% / 80.3ms |  |
| 28 | solver | `main` | 0.00% / 0.0us | 99.54% / 80.2ms |  |
| 29 | solver | `(anonymous namespace)::solveInput(...)::CliOptions const&)` | 0.00% / 0.0us | 99.53% / 80.2ms |  |
| 30 | solver | `tsp::BranchBoundSolver::solve(tsp::BranchStrategy)` | 0.00% / 0.0us | 99.39% / 80.1ms |  |

## Scope Total

- Selected function self share: `98.05%` of program Ir.
- Selected function estimated self time: `79.0ms`.
- Full CSV: `function_time_breakdown.csv`.
