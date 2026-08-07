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
| burma14 | 14 | 35.2ms | 34.3ms | -889.0us (-2.5%) | 124,666,779 | 112,488,432 | `tsp::BranchBoundSolver::computeO...` |

## Cross-Instance Function Trends

Functions that consistently changed across multiple instances:

| Function | Instances | Avg Delta | Max Abs Delta | Direction |
|---|---|---|---|---|

## Solver vs Runtime Overhead Trend

| Instance | Old Solver% | New Solver% | Old Runtime% | New Runtime% | Solver Δ |
|---|---|---|---|---|---|
| burma14 | 69.3% | 66.0% | 24.9% | 27.7% | -3.3% |
