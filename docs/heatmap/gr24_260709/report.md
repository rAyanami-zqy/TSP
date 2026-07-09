# Function Time Share - gr24

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `15,566,725,552` Ir.
- Native elapsed: `2.313s`.
- Callgrind elapsed: `1.97m`.

## Solver Stats

- `cost`: `1272`
- `initial_upper_bound`: `1272`
- `method`: `exact`
- `nodes_created`: `1069970`
- `nodes_expanded`: `297291`
- `pruned_bound`: `534969`
- `pruned_infeasible`: `29`
- `root_lower_bound`: `1081`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `tsp::BranchBoundSolver::buildBranchCandidates(...) const` | 27.00% / 624.5ms | 28.25% / 653.4ms |  |
| 2 | solver | `{lambda(int)#2}::operator()(int) const` | 19.88% / 459.8ms | 1968.00% / 45.519s |  |
| 3 | solver | `tsp::BranchBoundSolver::computeOneTree(...) const` | 18.24% / 421.9ms | 27.47% / 635.4ms |  |
| 4 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 5.62% / 130.0ms | 6.88% / 159.1ms |  |
| 5 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 4.79% / 110.8ms | 6.51% / 150.6ms |  |
| 6 | solver | `tsp::BranchBoundSolver::bpPartition(...)` | 4.33% / 100.2ms | 25.43% / 588.2ms |  |
| 7 | runtime | `./malloc/./malloc/malloc.c:malloc` | 3.94% / 91.1ms | 10.86% / 251.2ms |  |
| 8 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 3.69% / 85.3ms | 3.69% / 85.3ms |  |
| 9 | runtime | `./malloc/./malloc/malloc.c:free` | 2.47% / 57.1ms | 9.25% / 214.0ms |  |
| 10 | runtime | `operator new(unsigned long)` | 1.41% / 32.6ms | 12.45% / 288.0ms |  |
| 11 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 1.10% / 25.4ms | 1.71% / 39.6ms |  |
| 12 | solver | `void std::vector<>::_M_realloc_insert<>(...)` | 0.94% / 21.7ms | 2.49% / 57.6ms |  |
| 13 | runtime | `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | 0.84% / 19.4ms | 0.84% / 19.4ms |  |
| 14 | runtime | `./malloc/./malloc/malloc.c:malloc_consolidate` | 0.65% / 15.0ms | 0.00% / 0.0us |  |
| 15 | solver | `operator()(...)::LevelChanges&) const` | 0.64% / 14.8ms | 0.00% / 0.0us |  |
| 16 | solver | `tsp::BranchBoundSolver::search(...)` | 0.48% / 11.1ms | 1052.20% / 24.337s |  |
| 17 | runtime | `std::vector<>::_M_fill_assign(unsigned long, int const&)` | 0.47% / 10.9ms | 1.09% / 25.2ms |  |
| 18 | runtime | `./malloc/./malloc/malloc.c:_int_free_maybe_consolidate` | 0.37% / 8.6ms | 0.00% / 0.0us |  |
| 19 | solver | `std::vector<>::reserve(unsigned long)` | 0.35% / 8.1ms | 1.79% / 41.4ms |  |
| 20 | runtime | `./malloc/./malloc/arena.c:free` | 0.26% / 6.0ms | 0.00% / 0.0us |  |
| 21 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.24% / 5.6ms | 0.00% / 0.0us |  |
| 22 | solver | `operator()(...)::LevelChanges const&) const` | 0.22% / 5.1ms | 0.00% / 0.0us |  |
| 23 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.21% / 4.9ms | 0.00% / 0.0us |  |
| 24 | runtime | `operator delete(void*)` | 0.18% / 4.2ms | 9.60% / 222.0ms |  |
| 25 | solver | `std::_Function_handler<>::_M_invoke(std::_Any_data const&, int&&)` | 0.00% / 0.0us | 1968.00% / 45.519s |  |
| 26 | solver | `(below main)` | 0.00% / 0.0us | 99.99% / 2.313s |  |
| 27 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 99.99% / 2.313s |  |
| 28 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 99.99% / 2.313s |  |
| 29 | solver | `main` | 0.00% / 0.0us | 99.99% / 2.313s |  |
| 30 | solver | `(anonymous namespace)::solveInput(...)::CliOptions const&)` | 0.00% / 0.0us | 99.99% / 2.313s |  |

## Scope Total

- Selected function self share: `98.32%` of program Ir.
- Selected function estimated self time: `2.274s`.
- Full CSV: `function_time_breakdown.csv`.
