# NEWHKMST comparison binary

Build the `NEWHKMST` branch in Release mode and copy the executable here as
`tsp_bb`:

```sh
git switch NEWHKMST
cmake -S . -B build-newhkmst-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-newhkmst-release -j
cp build-newhkmst-release/tsp_bb solver/NEWHKMST/tsp_bb
chmod +x solver/NEWHKMST/tsp_bb
```

`tools/compare_hkmst_newhkmst.py` automatically enables the recommended
persistent potential update strategy:

```text
subtree-adaptive, depth spacing 2, gap 2%, 16 iterations, budget 5000
```
