# Heatmap Comparison Summary

## Solver Versions

- **Old**: `solver/tsp_bb_26_07_02/tsp_bb`
- **New**: `solver/tsp_bb_26_07_08/tsp_bb`

## Overview

- Total instances: 1
- Successfully compared: 1
- Timeout (excluded): 0
- Error (excluded): 0

## Overall Performance

| Instance | n | Old Time | New Time | Delta | Old Ir | New Ir | Top Changer |
|---|---|---|---|---|---|---|---|
| fri26 | 26 | 810.1ms | 1.552s | 742.2ms (+91.6%) | 6,211,515,375 | 11,591,064,512 | `tsp::BranchBoundSolver::buildBra...` |

## Cross-Instance Function Trends

Functions that consistently changed across multiple instances:

| Function | Instances | Avg Delta | Max Abs Delta | Direction |
|---|---|---|---|---|

## Solver vs Runtime Overhead Trend

| Instance | Old Solver% | New Solver% | Old Runtime% | New Runtime% | Solver Δ |
|---|---|---|---|---|---|
| fri26 | 82.8% | 90.0% | 13.9% | 7.9% | +7.2% |
