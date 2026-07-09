# Heatmap Comparison: 260708 vs 260709

- **Old**: `solver/tsp_bb_260708/tsp_bb`
- **New**: `solver/tsp_bb_260709/tsp_bb`

## Overview

- Total: 6, Compared: 6, Skipped: 0

## Overall Performance

| Instance | n | Old Time | New Time | Delta | Old Ir | New Ir | Top Changer |
|---|---|---|---|---|---|---|---|
| burma14 | 14 | 40.7ms | 58.3ms | 17.6ms (+43.2%) | 137,241,840 | 258,068,469 | `std::_Function_handler<void (int...` |
| gr17 | 17 | 2.474s | 3.486s | 1.012s (+40.9%) | 15,290,897,525 | 21,761,551,205 | `tsp::BranchBoundSolver::computeO...` |
| gr21 | 21 | 89.6ms | 173.2ms | 83.5ms (+93.2%) | 468,190,922 | 1,049,585,792 | `tsp::BranchBoundSolver::search(....` |
| gr24 | 24 | 1.256s | 2.313s | 1.057s (+84.1%) | 8,375,907,163 | 15,566,725,552 | `tsp::BranchBoundSolver::computeO...` |
| fri26 | 26 | 2.198s | 10.453s | 8.255s (+375.6%) | 15,174,385,039 | 69,541,150,846 | `tsp::BranchBoundSolver::buildBra...` |
| dj38 | 38 | 125.6ms | 8.948s | 8.823s (+7026.0%) | 910,025,230 | 67,723,820,524 | `./string/../sysdeps/x86_64/multi...` |

## Cross-Instance Function Trends

| Function | Instances | Avg Delta | Max Abs Delta | Direction |
|---|---|---|---|---|
| `./string/../sysdeps/x86_64/multiarch/memmove-vec-unalig` | burma14, gr17, gr21, gr24, fri26, dj38 | -0.28% | 2.39% | 🔻 faster |
| `tsp::BranchBoundSolver::buildBranchCandidates(...) cons` | burma14, gr17, gr21, gr24, fri26, dj38 | -0.37% | 2.34% | 🔻 faster |
| `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::o` | burma14, gr17, gr21, gr24, fri26, dj38 | -0.48% | 1.98% | 🔻 faster |
| `./malloc/./malloc/malloc.c:_int_malloc` | burma14, gr17, gr21, gr24, fri26, dj38 | +0.62% | 1.55% | 🔺 slower |
| `tsp::BranchBoundSolver::computeOneTree(...) const` | burma14, gr17, gr21, gr24, fri26, dj38 | -0.49% | 1.31% | 🔻 faster |
| `std::_Function_handler<void (int, double, int), tsp::Br` | burma14, gr21, dj38 | -0.84% | 1.09% | 🔻 faster |
| `./malloc/./malloc/malloc.c:_int_free` | burma14, gr17, gr21, gr24, fri26, dj38 | +0.40% | 0.83% | 🔺 slower |
| `./malloc/./malloc/malloc.c:malloc` | burma14, gr17, gr21, gr24, fri26, dj38 | +0.34% | 0.68% | 🔺 slower |

## Solver vs Runtime Overhead Trend

| Instance | Old Solver% | New Solver% | Old Runtime% | New Runtime% | Solver Δ |
|---|---|---|---|---|---|
| burma14 | 57.0% | 55.7% | 36.8% | 38.9% | -1.3% |
| gr17 | 60.5% | 59.1% | 35.3% | 36.5% | -1.3% |
| gr21 | 68.5% | 67.4% | 27.7% | 29.2% | -1.2% |
| gr24 | 72.4% | 71.9% | 24.7% | 25.2% | -0.5% |
| fri26 | 73.5% | 69.5% | 23.6% | 27.4% | -4.0% |
| dj38 | 77.7% | 76.4% | 19.5% | 20.7% | -1.2% |
