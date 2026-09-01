# PHKMST parameter ablation

## Configurations

| ID | Root | Node | Trigger | Depth | Gap | Branch order |
|---|---|---|---|---:|---:|---|
| P0 | polyak | off | none | - | - | weight |
| P1 | polyak | polyak | subtree-adaptive | 2 | 0.020 | weight |
| P2 | polyak | helsgaun | subtree-adaptive | 2 | 0.020 | weight |
| H0 | helsgaun | off | none | - | - | weight |
| T1 | polyak | polyak | depth | 1 | 0.020 | weight |
| B1 | polyak | off | none | - | - | root-alpha-asc |
| B3 | polyak | off | none | - | - | root-alpha-global-asc |
| B4 | polyak | off | none | - | - | root-alpha-global-desc |

## Aggregate results

| ID | Correct | Created | Expanded | Exp/P0 | Geo/P0 | W/T/L | Batch wall(s) | Wall/P0 | Inst wall geo/P0 | Improve | Direct prune |
|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| P0 | 3/3 | 564 | 432 | 1.000 | 1.000 | 0/3/0 | 0.020 | 1.000 | 1.000 | - | - |
| P1 | 3/3 | 67 | 34 | 0.079 | 0.272 | 3/0/0 | 0.018 | 0.928 | 0.944 | 1.000 | 0.700 |
| P2 | 3/3 | 124 | 55 | 0.127 | 0.362 | 3/0/0 | 0.019 | 0.957 | 0.977 | 1.000 | 0.750 |
| H0 | 3/3 | 2836 | 1965 | 4.549 | 8.527 | 0/0/3 | 0.017 | 0.863 | 0.865 | - | - |
| T1 | 3/3 | 74 | 25 | 0.058 | 0.164 | 3/0/0 | 0.018 | 0.927 | 0.939 | 0.985 | 0.667 |
| B1 | 3/3 | 677 | 511 | 1.183 | 1.114 | 0/1/2 | 0.017 | 0.854 | 0.887 | - | - |
| B3 | 3/3 | 1008 | 718 | 1.662 | 1.431 | 0/0/3 | 0.017 | 0.875 | 0.927 | - | - |
| B4 | 3/3 | 647 | 494 | 1.144 | 1.196 | 0/1/2 | 0.017 | 0.854 | 0.918 | - | - |

`Exp/P0` is the ratio of aggregate expanded nodes. `Geo/P0` is the geometric mean of per-instance `(expanded+1)/(P0 expanded+1)`. `W/T/L` compares expanded nodes with P0.

## Per-instance results

| ID | Instance | Correct | Expanded | Exp/P0 | Wall(s) | Wall/P0 |
|---|---|---:|---:|---:|---:|---:|
| P0 | att48.tsp | yes | 414 | 1.000 | 0.008010 | 1.000 |
| P0 | bays29.tsp | yes | 11 | 1.000 | 0.002553 | 1.000 |
| P0 | dantzig42.tsp | yes | 7 | 1.000 | 0.005597 | 1.000 |
| P1 | att48.tsp | yes | 24 | 0.060 | 0.008330 | 1.040 |
| P1 | bays29.tsp | yes | 7 | 0.667 | 0.002426 | 0.950 |
| P1 | dantzig42.tsp | yes | 3 | 0.500 | 0.004758 | 0.850 |
| P2 | att48.tsp | yes | 42 | 0.104 | 0.009267 | 1.157 |
| P2 | bays29.tsp | yes | 10 | 0.917 | 0.002320 | 0.909 |
| P2 | dantzig42.tsp | yes | 3 | 0.500 | 0.004964 | 0.887 |
| H0 | att48.tsp | yes | 1721 | 4.149 | 0.007777 | 0.971 |
| H0 | bays29.tsp | yes | 150 | 12.583 | 0.001973 | 0.773 |
| H0 | dantzig42.tsp | yes | 94 | 11.875 | 0.004823 | 0.862 |
| T1 | att48.tsp | yes | 21 | 0.053 | 0.008657 | 1.081 |
| T1 | bays29.tsp | yes | 3 | 0.333 | 0.002343 | 0.918 |
| T1 | dantzig42.tsp | yes | 1 | 0.250 | 0.004676 | 0.835 |
| B1 | att48.tsp | yes | 491 | 1.186 | 0.007531 | 0.940 |
| B1 | bays29.tsp | yes | 13 | 1.167 | 0.002341 | 0.917 |
| B1 | dantzig42.tsp | yes | 7 | 1.000 | 0.004531 | 0.810 |
| B3 | att48.tsp | yes | 694 | 1.675 | 0.007725 | 0.965 |
| B3 | bays29.tsp | yes | 13 | 1.167 | 0.002597 | 1.018 |
| B3 | dantzig42.tsp | yes | 11 | 1.500 | 0.004536 | 0.810 |
| B4 | att48.tsp | yes | 472 | 1.140 | 0.007548 | 0.942 |
| B4 | bays29.tsp | yes | 11 | 1.000 | 0.002600 | 1.019 |
| B4 | dantzig42.tsp | yes | 11 | 1.500 | 0.004503 | 0.805 |
