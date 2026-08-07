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
| burma14 | 14 | 55.7ms | 45.8ms | -9936.7us (-17.8%) | 229,601,724 | 208,312,526 | `tsp::BranchBoundSolver::computeO...` |

## Cross-Instance Function Trends

Functions that consistently changed across multiple instances:

| Function | Instances | Avg Delta | Max Abs Delta | Direction |
|---|---|---|---|---|

## Solver vs Runtime Overhead Trend

| Instance | Old Solver% | New Solver% | Old Runtime% | New Runtime% | Solver Δ |
|---|---|---|---|---|---|
| burma14 | 68.2% | 61.9% | 27.4% | 33.5% | -6.3% |
