#!/bin/bash
# Batch convert all examples/ raw matrix instances to TSPLIB format.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
EXAMPLES_DIR="$(cd "$SCRIPT_DIR/../examples" && pwd)"
CONVERTER="$SCRIPT_DIR/convert_to_tsplib.py"

echo "=== Converting root-level instances ==="
python3 "$CONVERTER" "$EXAMPLES_DIR/five-city.txt" -o "$EXAMPLES_DIR/five-city.tsp"

echo ""
echo "=== Converting random/complete instances ==="
python3 "$CONVERTER" --dir "$EXAMPLES_DIR/random/complete" -o "$EXAMPLES_DIR/random/complete/tsplib"

echo ""
echo "=== Converting random/sparse instances ==="
python3 "$CONVERTER" --dir "$EXAMPLES_DIR/random/sparse" -o "$EXAMPLES_DIR/random/sparse/tsplib"

echo ""
echo "=== Converting converted instances ==="
python3 "$CONVERTER" --dir "$EXAMPLES_DIR/converted" -o "$EXAMPLES_DIR/converted/tsplib"

echo ""
echo "Done. TSPLIB files created under:"
for d in \
    "$EXAMPLES_DIR/"*.tsp \
    "$EXAMPLES_DIR/random/complete/tsplib" \
    "$EXAMPLES_DIR/random/sparse/tsplib" \
    "$EXAMPLES_DIR/converted/tsplib" \
    "$EXAMPLES_DIR/tsplib"; do
    if [ -d "$d" ] || ls "$d"/*.tsp 2>/dev/null >&2; then
        echo "  $d"
    fi
done
