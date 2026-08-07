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
| burma14 | 14 | 36.1ms | 36.2ms | 109.1us (+0.3%) | 119,115,966 | 111,472,960 | `tsp::BranchBoundSolver::computeO...` |

## Cross-Instance Function Trends

Functions that consistently changed across multiple instances:

| Function | Instances | Avg Delta | Max Abs Delta | Direction |
|---|---|---|---|---|

## Solver vs Runtime Overhead Trend

| Instance | Old Solver% | New Solver% | Old Runtime% | New Runtime% | Solver Δ |
|---|---|---|---|---|---|
| burma14 | 68.3% | 63.1% | 25.3% | 30.3% | -5.2% |
