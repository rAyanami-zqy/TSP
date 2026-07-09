# Heatmap Comparison: 260702 vs 260709

- **Old**: `solver/tsp_bb_260702/tsp_bb`
- **New**: `solver/tsp_bb_260709/tsp_bb`

## Overview

- Total: 6, Compared: 6, Skipped: 0

## Overall Performance

| Instance | n | Old Time | New Time | Delta | Old Ir | New Ir | Top Changer |
|---|---|---|---|---|---|---|---|
| burma14 | 14 | 38.3ms | 58.3ms | 20.0ms (+52.4%) | 125,968,557 | 258,068,469 | `tsp::BranchBoundSolver::search(....` |
| gr17 | 17 | 2.210s | 3.486s | 1.276s (+57.7%) | 14,216,783,384 | 21,761,551,205 | `tsp::BranchBoundSolver::search(....` |
| gr21 | 21 | 80.6ms | 173.2ms | 92.5ms (+114.8%) | 437,536,663 | 1,049,585,792 | `tsp::BranchBoundSolver::search(....` |
| gr24 | 24 | 1.126s | 2.313s | 1.187s (+105.4%) | 8,119,979,565 | 15,566,725,552 | `tsp::BranchBoundSolver::search(....` |
| fri26 | 26 | 1.937s | 10.453s | 8.516s (+439.7%) | 14,738,129,521 | 69,541,150,846 | `tsp::BranchBoundSolver::search(....` |
| dj38 | 38 | 113.7ms | 8.948s | 8.834s (+7767.1%) | 870,679,889 | 67,723,820,524 | `tsp::BranchBoundSolver::search(....` |

## Cross-Instance Function Trends

| Function | Instances | Avg Delta | Max Abs Delta | Direction |
|---|---|---|---|---|
| `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::` | burma14, gr17, gr21, gr24, fri26, dj38 | -19.29% | 23.34% | 🔻 faster |
| `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::o` | burma14, gr17, gr21, gr24, fri26, dj38 | +16.89% | 22.21% | 🔺 slower |
| `tsp::BranchBoundSolver::buildBranchCandidates(...) cons` | burma14, gr17, gr21, gr24, fri26, dj38 | -6.02% | 8.37% | 🔻 faster |
| `tsp::BranchBoundSolver::bpPartition(...)` | burma14, gr17, gr21, gr24, fri26, dj38 | +4.50% | 4.85% | 🔺 slower |
| `./malloc/./malloc/malloc.c:_int_malloc` | burma14, gr17, gr21, gr24, fri26, dj38 | +4.00% | 4.51% | 🔺 slower |
| `void std::vector<tsp::BranchBoundSolver::Edge, std::all` | burma14, gr17, gr21, gr24, fri26, dj38 | -1.92% | 2.77% | 🔻 faster |
| `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | burma14, gr17, gr21, gr24, fri26, dj38 | -2.09% | 2.75% | 🔻 faster |
| `tsp::BranchBoundSolver::computeOneTree(...) const` | burma14, gr17, gr21, gr24, fri26, dj38 | -1.41% | 2.45% | 🔻 faster |
| `./string/../sysdeps/x86_64/multiarch/memmove-vec-unalig` | burma14, gr17, gr21, gr24, fri26, dj38 | +1.50% | 1.84% | 🔺 slower |
| `tsp::BranchBoundSolver::search(...)::{lambda(...)::Leve` | burma14, gr17, gr21, gr24, fri26 | +0.90% | 1.34% | 🔺 slower |
| `std::_Function_handler<void (int, double, int), tsp::Br` | burma14, gr21, dj38 | -0.93% | 1.14% | 🔻 faster |
| `./malloc/./malloc/malloc.c:_int_free` | burma14, gr17, gr21, gr24, fri26, dj38 | +0.53% | 0.94% | 🔺 slower |
| `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | burma14, gr17, gr21, gr24, fri26, dj38 | +0.67% | 0.94% | 🔺 slower |
| `./malloc/./malloc/malloc.c:malloc_consolidate` | burma14, gr17, gr21, gr24, fri26 | +0.74% | 0.93% | 🔺 slower |
| `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | burma14, gr17, gr21, gr24, fri26, dj38 | +0.61% | 0.85% | 🔺 slower |
| `tsp::BranchBoundSolver::search(...)` | burma14, gr17, gr21, gr24, fri26, dj38 | -0.46% | 0.82% | 🔻 faster |
| `std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | burma14, gr17, gr21, gr24, fri26, dj38 | +0.44% | 0.71% | 🔺 slower |
| `./malloc/./malloc/malloc.c:malloc` | burma14, gr17, gr21, gr24, fri26, dj38 | +0.36% | 0.70% | 🔺 slower |

## Solver vs Runtime Overhead Trend

| Instance | Old Solver% | New Solver% | Old Runtime% | New Runtime% | Solver Δ |
|---|---|---|---|---|---|
| burma14 | 64.7% | 55.7% | 29.7% | 38.9% | -9.0% |
| gr17 | 68.7% | 59.1% | 27.7% | 36.5% | -9.5% |
| gr21 | 76.8% | 67.4% | 20.2% | 29.2% | -9.4% |
| gr24 | 79.2% | 71.9% | 18.5% | 25.2% | -7.3% |
| fri26 | 80.3% | 69.5% | 17.3% | 27.4% | -10.8% |
| dj38 | 85.3% | 76.4% | 12.6% | 20.7% | -8.9% |
