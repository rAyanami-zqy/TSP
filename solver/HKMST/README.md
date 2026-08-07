# HKMST comparison binary

Build the original `HKMST` branch in Release mode and copy the executable here
as `tsp_bb`:

```sh
git switch HKMST
cmake -S . -B build-hkmst-release -DCMAKE_BUILD_TYPE=Release
cmake --build build-hkmst-release -j
cp build-hkmst-release/tsp_bb solver/HKMST/tsp_bb
chmod +x solver/HKMST/tsp_bb
```

`tools/compare_hkmst_newhkmst.py` invokes this binary without potential-update
arguments, so it remains the experiment baseline.
