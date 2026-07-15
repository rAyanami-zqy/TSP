# Function Time Share - fri26

## Measurement

- Cost basis: callgrind `Ir` instruction counts.
- Time estimate: native elapsed time multiplied by each function percentage.
- Self time is exclusive and additive. Inclusive time includes callees and is not additive.
- Scope: `solver-runtime`.
- Program total: `11,591,064,512` Ir.
- Native elapsed: `1.552s`.
- Callgrind elapsed: `1.39m`.

## Solver Stats

- `cost`: `937`
- `initial_upper_bound`: `937`
- `method`: `exact`
- `nodes_created`: `565572`
- `nodes_expanded`: `296513`
- `pruned_bound`: `269059`
- `pruned_infeasible`: `73`
- `root_lower_bound`: `824`

## Top Functions By Self Time

| # | Kind | Function | Self | Inclusive | Calls |
|---:|---|---|---:|---:|---:|
| 1 | solver | `BranchBoundSolver::updateOneTreeAfterCandidateRemoval(...) const` | 33.79% / 524.5ms | 48.16% / 747.6ms |  |
| 2 | solver | `tsp::BranchBoundSolver::findMstReplacement(...) const` | 30.27% / 469.9ms | 33.61% / 521.7ms |  |
| 3 | solver | `tsp::BranchBoundSolver::deactivateCandidate(...) const` | 6.58% / 102.1ms | 6.58% / 102.1ms |  |
| 4 | solver | `tsp::BranchBoundSolver::deactivateCandidatesAfterForce(...) const` | 4.84% / 75.1ms | 11.30% / 175.4ms |  |
| 5 | solver | `tsp::BranchBoundSolver::search(...)` | 4.57% / 70.9ms | 951.20% / 14.766s |  |
| 6 | solver | `tsp::BranchBoundSolver::restoreCandidates(...) const` | 4.42% / 68.6ms | 4.42% / 68.6ms |  |
| 7 | solver | `tsp::BranchBoundSolver::bpPartition(...)` | 4.01% / 62.2ms | 26.51% / 411.5ms |  |
| 8 | runtime | `./malloc/./malloc/malloc.c:_int_free` | 2.58% / 40.0ms | 2.85% / 44.2ms |  |
| 9 | runtime | `./malloc/./malloc/malloc.c:malloc` | 2.09% / 32.4ms | 2.54% / 39.4ms |  |
| 10 | runtime | `./malloc/./malloc/malloc.c:free` | 1.30% / 20.2ms | 4.29% / 66.6ms |  |
| 11 | solver | `tsp::BranchBoundSolver::updateOneTreeAfterForbid(...) const` | 0.90% / 14.0ms | 20.64% / 320.4ms |  |
| 12 | runtime | `operator new(unsigned long)` | 0.74% / 11.5ms | 3.37% / 52.3ms |  |
| 13 | solver | `tsp::BranchBoundSolver::replaceOneTreeEdge(...) const` | 0.60% / 9.3ms | 0.60% / 9.3ms |  |
| 14 | runtime | `./malloc/./malloc/malloc.c:_int_malloc` | 0.44% / 6.8ms | 0.00% / 0.0us |  |
| 15 | runtime | `./string/../sysdeps/x86_64/multiarch/memmove-vec-unaligned-erms.S...` | 0.41% / 6.4ms | 0.00% / 0.0us |  |
| 16 | runtime | `./string/../sysdeps/x86_64/multiarch/memset-vec-unaligned-erms.S:...` | 0.34% / 5.3ms | 0.00% / 0.0us |  |
| 17 | runtime | `std::vector<>::reserve(unsigned long)` | 0.31% / 4.8ms | 1.05% / 16.3ms |  |
| 18 | solver | `tsp::BranchBoundSolver::isTour(...) const` | 0.27% / 4.2ms | 0.00% / 0.0us |  |
| 19 | runtime | `std::vector<>::operator=(std::vector<> const&)` | 0.20% / 3.1ms | 0.00% / 0.0us |  |
| 20 | runtime | `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | 0.19% / 2.9ms | 0.00% / 0.0us |  |
| 21 | runtime | `./malloc/./malloc/arena.c:free` | 0.14% / 2.2ms | 0.00% / 0.0us |  |
| 22 | solver | `(below main)` | 0.00% / 0.0us | 99.98% / 1.552s |  |
| 23 | solver | `./csu/../csu/libc-start.c:__libc_start_main@@GLIBC_2.34` | 0.00% / 0.0us | 99.98% / 1.552s |  |
| 24 | solver | `./csu/../sysdeps/nptl/libc_start_call_main.h:(below main)` | 0.00% / 0.0us | 99.98% / 1.552s |  |
| 25 | solver | `main` | 0.00% / 0.0us | 99.98% / 1.552s |  |
| 26 | solver | `(anonymous namespace)::solveInput(...)::CliOptions const&)` | 0.00% / 0.0us | 99.98% / 1.552s |  |
| 27 | solver | `tsp::BranchBoundSolver::solve()` | 0.00% / 0.0us | 99.97% / 1.552s |  |
| 28 | runtime | `operator delete(void*, unsigned long)` | 0.00% / 0.0us | 4.66% / 72.3ms |  |
| 29 | runtime | `operator delete(void*)` | 0.00% / 0.0us | 4.48% / 69.5ms |  |

## Scope Total

- Selected function self share: `98.99%` of program Ir.
- Selected function estimated self time: `1.537s`.
- Full CSV: `function_time_breakdown.csv`.
