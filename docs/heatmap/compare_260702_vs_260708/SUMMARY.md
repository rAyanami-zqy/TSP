# Heatmap Comparison Summary

## Solver Versions

- **Old**: `solver/tsp_bb_26_07_02/tsp_bb`
- **New**: `solver/tsp_bb_26_07_08/tsp_bb`

## Overview

- Total instances: 11
- Successfully compared: 7
- Timeout (excluded): 0
- Error (excluded): 4

### Error Instances (Excluded)

- ulysses16 (n=16): Instance: /home/wj/code/TSP/data/classic/tsplib/ulysses16.tsp
Output:   /home/wj/code/TSP/docs/heatmap/ulysses16_260708
Dimension: 16
  native timing ... done (18.883s)
  callgrind profiling ... TIMEO
- ulysses22 (n=22): Instance: /home/wj/code/TSP/data/classic/tsplib/ulysses22.tsp
Output:   /home/wj/code/TSP/docs/heatmap/ulysses22_260702
Dimension: 22
  native timing ... TIMEOUT after 5.00m
  callgrind profiling ... 
- bayg29 (n=29): Instance: /home/wj/code/TSP/data/classic/tsplib/bayg29.tsp
Output:   /home/wj/code/TSP/docs/heatmap/bayg29_260702
Dimension: 29
  native timing ... done (27.795s)
  callgrind profiling ... TIMEOUT aft
- bays29 (n=29): Instance: /home/wj/code/TSP/data/classic/tsplib/bays29.tsp
Output:   /home/wj/code/TSP/docs/heatmap/bays29_260702
Dimension: 29
  native timing ... TIMEOUT after 5.00m
  callgrind profiling ... TIMEOU

## Overall Performance

| Instance | n | Old Time | New Time | Delta | Old Ir | New Ir | Top Changer |
|---|---|---|---|---|---|---|---|
| burma14 | 14 | 38.3ms | 40.7ms | 2.4ms (+6.4%) | 125,968,557 | 137,241,840 | `tsp::BranchBoundSolver::search(....` |
| gr17 | 17 | 2.210s | 2.474s | 263.9ms (+11.9%) | 14,216,783,384 | 15,290,897,525 | `tsp::BranchBoundSolver::search(....` |
| gr21 | 21 | 80.6ms | 89.6ms | 9.0ms (+11.2%) | 437,536,663 | 468,190,922 | `tsp::BranchBoundSolver::search(....` |
| gr24 | 24 | 1.126s | 1.256s | 130.1ms (+11.6%) | 8,119,979,565 | 8,375,907,163 | `tsp::BranchBoundSolver::search(....` |
| fri26 | 26 | 1.937s | 2.198s | 260.9ms (+13.5%) | 14,738,129,521 | 15,174,385,039 | `tsp::BranchBoundSolver::search(....` |
| wi29 | 29 | 201.8ms | 225.4ms | 23.6ms (+11.7%) | 1,490,626,131 | 1,528,799,306 | `tsp::BranchBoundSolver::search(....` |
| dj38 | 38 | 113.7ms | 125.6ms | 11.8ms (+10.4%) | 870,679,889 | 910,025,230 | `tsp::BranchBoundSolver::search(....` |

## Cross-Instance Function Trends

Functions that consistently changed across multiple instances:

| Function | Instances | Avg Delta | Max Abs Delta | Direction |
|---|---|---|---|---|
| `tsp::BranchBoundSolver::search(...)::{lambda(bool)#1}::` | burma14, gr17, gr21, gr24, fri26, wi29, dj38 | -19.58% | 23.34% | 🔻 faster |
| `tsp::BranchBoundSolver::search(...)::{lambda(int)#2}::o` | burma14, gr17, gr21, gr24, fri26, wi29, dj38 | +17.76% | 21.93% | 🔺 slower |
| `tsp::BranchBoundSolver::buildBranchCandidates(...) cons` | burma14, gr17, gr21, gr24, fri26, wi29, dj38 | -5.68% | 7.10% | 🔻 faster |
| `tsp::BranchBoundSolver::bpPartition(...)` | burma14, gr17, gr21, gr24, fri26, wi29, dj38 | +4.40% | 5.00% | 🔺 slower |
| `./malloc/./malloc/malloc.c:_int_malloc` | burma14, gr17, gr21, gr24, fri26, wi29, dj38 | +3.24% | 4.23% | 🔺 slower |
| `./string/../sysdeps/x86_64/multiarch/memmove-vec-unalig` | burma14, gr17, gr21, gr24, fri26, wi29, dj38 | +1.73% | 3.86% | 🔺 slower |
| `void std::vector<tsp::BranchBoundSolver::Edge, std::all` | burma14, gr17, gr21, gr24, fri26, wi29, dj38 | -1.93% | 2.81% | 🔻 faster |
| `tsp::BranchBoundSolver::chooseBranchEdge(...) const` | burma14, gr17, gr21, gr24, fri26, wi29, dj38 | -2.03% | 2.75% | 🔻 faster |
| `tsp::BranchBoundSolver::computeOneTree(...) const` | burma14, gr17, gr21, gr24, fri26, wi29, dj38 | -0.82% | 1.61% | 🔻 faster |
| `tsp::BranchBoundSolver::search(...)::{lambda(...)::Leve` | burma14, gr17, gr21, gr24, fri26 | +0.85% | 1.22% | 🔺 slower |
| `tsp::BranchBoundSolver::search(...)` | burma14, gr17, gr21, gr24, fri26, wi29, dj38 | -0.48% | 0.89% | 🔻 faster |
| `./malloc/./malloc/malloc.c:malloc_consolidate` | burma14, gr17, gr21, gr24, fri26 | +0.68% | 0.89% | 🔺 slower |
| `./malloc/./malloc/malloc.c:unlink_chunk.isra.0` | burma14, gr17, gr21, gr24, fri26, wi29, dj38 | +0.49% | 0.80% | 🔺 slower |
| `std::vector<tsp::BranchBoundSolver::Edge, std::allocato` | burma14, gr17, gr21, gr24, fri26, wi29 | +0.43% | 0.66% | 🔺 slower |
| `./malloc/./malloc/malloc.c:_int_free_merge_chunk` | burma14, gr17, gr21, gr24, fri26, wi29, dj38 | +0.53% | 0.66% | 🔺 slower |

## Solver vs Runtime Overhead Trend

| Instance | Old Solver% | New Solver% | Old Runtime% | New Runtime% | Solver Δ |
|---|---|---|---|---|---|
| burma14 | 64.7% | 57.0% | 29.7% | 36.8% | -7.7% |
| gr17 | 68.7% | 60.5% | 27.7% | 35.3% | -8.2% |
| gr21 | 76.8% | 68.5% | 20.2% | 27.7% | -8.2% |
| gr24 | 79.2% | 72.4% | 18.5% | 24.7% | -6.8% |
| fri26 | 80.3% | 73.5% | 17.3% | 23.6% | -6.9% |
| wi29 | 83.4% | 77.1% | 13.8% | 18.9% | -6.3% |
| dj38 | 85.3% | 77.5% | 12.6% | 19.5% | -7.8% |
