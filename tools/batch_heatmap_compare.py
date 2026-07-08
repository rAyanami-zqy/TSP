#!/usr/bin/env python3
"""
Batch heatmap comparison between two solver versions.

Profiles both solvers on TSP instances up to a given max size,
then generates comparison tables and a summary report.

Usage:
  python3 tools/batch_heatmap_compare.py \
      --solver-old solver/tsp_bb_26_07_02/tsp_bb \
      --solver-new solver/tsp_bb_26_07_08/tsp_bb \
      --max-size 38 --min-size 14
"""

from __future__ import annotations

import argparse
import csv
import os
import re
import subprocess
import sys
import time
from collections import defaultdict
from dataclasses import dataclass, field

# ---------------------------------------------------------------------------
# Paths
# ---------------------------------------------------------------------------
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
HEATMAP_TOOL = os.path.join(PROJECT_ROOT, "tools", "heatmap_analysis.py")
DEFAULT_OUT_ROOT = os.path.join(PROJECT_ROOT, "docs", "heatmap")

# ---------------------------------------------------------------------------
# Helpers
# ---------------------------------------------------------------------------

def eprint(*args, **kwargs):
    print(*args, file=sys.stderr, **kwargs)


def fmt_seconds(value: float | None) -> str:
    if value is None:
        return "-"
    if value < 0.001:
        return f"{value * 1_000_000:.1f}us"
    if value < 1:
        return f"{value * 1000:.1f}ms"
    if value < 60:
        return f"{value:.3f}s"
    return f"{value / 60:.2f}m"


def read_instance_dimension(path: str) -> int | None:
    try:
        with open(path) as f:
            text = f.read(8192)
    except OSError:
        return None
    m = re.search(r"(?im)^\s*DIMENSION\s*:?\s*(\d+)\s*$", text)
    if m:
        return int(m.group(1))
    tokens = text.split()
    if tokens and tokens[0].isdigit():
        return int(tokens[0])
    return None


def find_instances(max_size: int, min_size: int = 0) -> list[tuple[int, str, str]]:
    """Return sorted list of (dimension, name, path) for instances <= max_size."""
    search_dirs = [
        os.path.join(PROJECT_ROOT, "data", "classic", "tsplib"),
        os.path.join(PROJECT_ROOT, "data", "classic", "national"),
    ]
    instances = []
    for search_dir in search_dirs:
        if not os.path.isdir(search_dir):
            continue
        for fname in sorted(os.listdir(search_dir)):
            if not fname.endswith(".tsp"):
                continue
            fpath = os.path.join(search_dir, fname)
            dim = read_instance_dimension(fpath)
            if dim is None:
                continue
            if dim < min_size or dim > max_size:
                continue
            name = os.path.splitext(fname)[0]
            instances.append((dim, name, fpath))
    instances.sort(key=lambda x: (x[0], x[1]))
    return instances


def run_heatmap_for_solver(instance_path: str, solver_binary: str,
                           out_dir: str, timeout: int,
                           suffix: str) -> dict:
    """Run heatmap_analysis.py --profile for one solver. Returns metadata dict."""
    full_out = f"{out_dir}_{suffix}"
    os.makedirs(full_out, exist_ok=True)

    cmd = [
        sys.executable, HEATMAP_TOOL,
        instance_path,
        "--solver", solver_binary,
        "--out-dir", full_out,
        "--profile",
        "--timeout", str(timeout),
        "--top-n", "30",
        "--scope", "solver-runtime",
    ]

    eprint(f"    Running: {' '.join(cmd)}")
    started = time.perf_counter()
    try:
        result = subprocess.run(
            cmd, capture_output=True, text=True,
            timeout=timeout * 5,  # callgrind is ~3x slower, give buffer
        )
    except subprocess.TimeoutExpired:
        elapsed = time.perf_counter() - started
        eprint(f"    TIMEOUT after {fmt_seconds(elapsed)}")
        return {"timeout": True, "elapsed_sec": elapsed, "out_dir": full_out}
    elapsed = time.perf_counter() - started

    if result.returncode != 0:
        tail = (result.stderr or result.stdout)[-1200:]
        eprint(f"    FAILED (exit {result.returncode}): {tail}")
        return {"timeout": False, "error": True, "elapsed_sec": elapsed,
                "out_dir": full_out, "stderr": tail}

    eprint(f"    Done in {fmt_seconds(elapsed)}")

    # Load metadata
    meta_path = os.path.join(full_out, "profile_metadata.json")
    metadata = {}
    if os.path.exists(meta_path):
        import json
        with open(meta_path) as f:
            metadata = json.load(f)

    return {
        "timeout": False,
        "error": False,
        "elapsed_sec": elapsed,
        "out_dir": full_out,
        "metadata": metadata,
    }


def load_csv(csv_path: str) -> list[dict]:
    """Load function_time_breakdown.csv into list of dicts."""
    rows = []
    if not os.path.exists(csv_path):
        return rows
    with open(csv_path, newline="") as f:
        reader = csv.DictReader(f)
        for row in reader:
            rows.append(row)
    return rows


# ---------------------------------------------------------------------------
# Comparison logic
# ---------------------------------------------------------------------------

def normalize_func_name(name: str) -> str:
    """Normalize function names for comparison across versions."""
    # Collapse template parameters
    name = re.sub(r"<[^<>]*>", "<>", name)
    # Collapse long parenthesized args
    name = re.sub(r"\([^)]{20,}\)", "(...)", name)
    # Remove address-like suffixes
    name = re.sub(r"'[0-9]+$", "", name)
    return name.strip()


def compare_csvs(old_csv: str, new_csv: str) -> dict:
    """Compare two function_time_breakdown.csv files. Returns comparison data."""
    old_rows = load_csv(old_csv)
    new_rows = load_csv(new_csv)

    old_by_func = {}
    for row in old_rows:
        key = normalize_func_name(row["function"])
        old_by_func[key] = row

    new_by_func = {}
    for row in new_rows:
        key = normalize_func_name(row["function"])
        new_by_func[key] = row

    all_funcs = set(old_by_func.keys()) | set(new_by_func.keys())

    changes = []
    for func in all_funcs:
        old_row = old_by_func.get(func)
        new_row = new_by_func.get(func)

        old_pct = float(old_row["self_pct_total"]) if old_row else 0.0
        new_pct = float(new_row["self_pct_total"]) if new_row else 0.0
        old_incl = float(old_row["inclusive_pct_total"]) if old_row else 0.0
        new_incl = float(new_row["inclusive_pct_total"]) if new_row else 0.0

        delta_self = new_pct - old_pct
        delta_incl = new_incl - old_incl

        # Only include significant functions (>= 0.5% in either version)
        if max(old_pct, new_pct) < 0.5 and max(old_incl, new_incl) < 1.0:
            continue

        changes.append({
            "function": func,
            "kind": new_row["kind"] if new_row else (old_row["kind"] if old_row else "other"),
            "old_self_pct": old_pct,
            "new_self_pct": new_pct,
            "delta_self_pct": delta_self,
            "old_incl_pct": old_incl,
            "new_incl_pct": new_incl,
            "delta_incl_pct": delta_incl,
            "old_est_sec": float(old_row["self_est_sec"]) if old_row and old_row["self_est_sec"] else None,
            "new_est_sec": float(new_row["self_est_sec"]) if new_row and new_row["self_est_sec"] else None,
            "old_calls": int(old_row["calls"]) if old_row and old_row["calls"] else 0,
            "new_calls": int(new_row["calls"]) if new_row and new_row["calls"] else 0,
        })

    changes.sort(key=lambda c: (-abs(c["delta_self_pct"]), -max(c["old_self_pct"], c["new_self_pct"])))

    return {
        "changes": changes,
        "total_old_functions": len(old_rows),
        "total_new_functions": len(new_rows),
    }


# ---------------------------------------------------------------------------
# Output generators
# ---------------------------------------------------------------------------

def write_instance_comparison_md(instance_name: str, dim: int,
                                 old_meta: dict, new_meta: dict,
                                 comparison: dict, out_path: str) -> str:
    """Write per-instance comparison markdown."""
    changes = comparison["changes"]
    old_native = old_meta.get("native_elapsed_sec")
    new_native = new_meta.get("native_elapsed_sec")
    old_stats = old_meta.get("solver_stats", {})
    new_stats = new_meta.get("solver_stats", {})

    with open(out_path, "w") as f:
        f.write(f"# Heatmap Comparison: {instance_name} (n={dim})\n\n")

        # Overall time comparison
        f.write("## Overall Performance\n\n")
        f.write("| Metric | Old (0702) | New (0708) | Delta |\n")
        f.write("|---|---|---|---|\n")

        if old_native and new_native:
            delta = new_native - old_native
            pct = (delta / old_native * 100) if old_native > 0 else 0
            f.write(f"| Native elapsed | {fmt_seconds(old_native)} | {fmt_seconds(new_native)} "
                    f"| {fmt_seconds(delta)} ({pct:+.1f}%) |\n")

        # Solver stats comparison
        all_stat_keys = set(old_stats.keys()) | set(new_stats.keys())
        for key in sorted(all_stat_keys):
            ov = old_stats.get(key, "-")
            nv = new_stats.get(key, "-")
            f.write(f"| `{key}` | `{ov}` | `{nv}` | |\n")

        f.write("\n## Top Changes by Self Time %\n\n")
        f.write("| # | Function | Kind | Old Self% | New Self% | Delta | Old ms | New ms |\n")
        f.write("|---:|---|---|---:|---:|---:|---:|---:|\n")

        for rank, ch in enumerate(changes[:25], start=1):
            old_ms = (ch["old_est_sec"] * 1000) if ch["old_est_sec"] is not None else None
            new_ms = (ch["new_est_sec"] * 1000) if ch["new_est_sec"] is not None else None
            f.write(
                f"| {rank} | `{ch['function'][:60]}` | {ch['kind']} "
                f"| {ch['old_self_pct']:.2f}% | {ch['new_self_pct']:.2f}% "
                f"| {ch['delta_self_pct']:+.2f}% "
                f"| {old_ms:.2f}ms" if old_ms else f"| {rank} | `{ch['function'][:60]}` | {ch['kind']} "
                f"| {ch['old_self_pct']:.2f}% | {ch['new_self_pct']:.2f}% "
                f"| {ch['delta_self_pct']:+.2f}% | -")
            if new_ms is not None:
                f.write(f" | {new_ms:.2f}ms")
            else:
                f.write(" | -")
            f.write(" |\n")

        # New functions in 0708
        new_funcs = [c for c in changes if c["old_self_pct"] == 0 and c["new_self_pct"] > 0]
        if new_funcs:
            f.write("\n## New Functions in 0708\n\n")
            f.write("| Function | Self% | Incl% |\n")
            f.write("|---|---|---:|\n")
            for c in new_funcs[:10]:
                f.write(f"| `{c['function'][:60]}` | {c['new_self_pct']:.2f}% | {c['new_incl_pct']:.2f}% |\n")

        # Removed functions
        removed_funcs = [c for c in changes if c["new_self_pct"] == 0 and c["old_self_pct"] > 0]
        if removed_funcs:
            f.write("\n## Removed Functions (was in 0702)\n\n")
            f.write("| Function | Self% | Incl% |\n")
            f.write("|---|---|---:|\n")
            for c in removed_funcs[:10]:
                f.write(f"| `{c['function'][:60]}` | {c['old_self_pct']:.2f}% | {c['old_incl_pct']:.2f}% |\n")

        f.write(f"\n## Summary\n\n")
        f.write(f"- Old version functions: {comparison['total_old_functions']}\n")
        f.write(f"- New version functions: {comparison['total_new_functions']}\n")

        # Aggregate solver vs runtime
        old_solver_self = sum(c["old_self_pct"] for c in changes if c["kind"] == "solver")
        new_solver_self = sum(c["new_self_pct"] for c in changes if c["kind"] == "solver")
        old_runtime_self = sum(c["old_self_pct"] for c in changes if c["kind"] == "runtime")
        new_runtime_self = sum(c["new_self_pct"] for c in changes if c["kind"] == "runtime")
        f.write(f"- Solver self%: {old_solver_self:.1f}% → {new_solver_self:.1f}% ({new_solver_self - old_solver_self:+.1f}%)\n")
        f.write(f"- Runtime self%: {old_runtime_self:.1f}% → {new_runtime_self:.1f}% ({new_runtime_self - old_runtime_self:+.1f}%)\n")

    return out_path


def generate_comparison_heatmaps(instance_name: str, dim: int,
                                 changes: list[dict],
                                 old_native: float | None,
                                 new_native: float | None,
                                 out_dir: str) -> list[str]:
    """Generate comparison heatmaps: side-by-side and delta charts."""
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        import numpy as np
    except ModuleNotFoundError:
        eprint("  matplotlib not available, skipping comparison heatmaps")
        return []

    paths = []
    top = changes[:20]  # top 20 by abs delta
    if not top:
        return paths

    # --- Chart 1: Side-by-side self time % comparison ---
    names = [c["function"][:55] for c in reversed(top)]
    old_pcts = [c["old_self_pct"] for c in reversed(top)]
    new_pcts = [c["new_self_pct"] for c in reversed(top)]
    y = list(range(len(names)))

    fig_h = max(6.0, len(top) * 0.36)
    fig, ax = plt.subplots(figsize=(14, fig_h))
    height = 0.35
    ax.barh([yi + height/2 for yi in y], old_pcts, height=height,
            color="#386cb0", edgecolor="#333", linewidth=0.3, label=f"Old (0702)")
    ax.barh([yi - height/2 for yi in y], new_pcts, height=height,
            color="#fb6a4a", edgecolor="#333", linewidth=0.3, label=f"New (0708)")
    ax.set_yticks(y)
    ax.set_yticklabels(names, fontfamily="monospace", fontsize=7.5)
    ax.set_xlabel("Self time % of total program Ir")
    ax.set_title(f"Self Time Share Comparison - {instance_name} (n={dim})", fontsize=12, fontweight="bold")
    ax.legend(loc="lower right")
    ax.grid(axis="x", alpha=0.25, linestyle="--")
    max_pct = max(max(old_pcts), max(new_pcts)) if top else 1
    ax.set_xlim(0, max_pct * 1.18)
    fig.tight_layout()
    path1 = os.path.join(out_dir, f"{instance_name}_04_self_time_comparison.png")
    fig.savefig(path1, dpi=180, bbox_inches="tight")
    plt.close(fig)
    paths.append(path1)

    # --- Chart 2: Self time delta (diverging bar) ---
    deltas = [c["delta_self_pct"] for c in reversed(top)]
    colors = ["#fc4e2a" if d > 0 else "#238b45" for d in deltas]

    fig, ax = plt.subplots(figsize=(11, fig_h))
    ax.barh(y, deltas, color=colors, edgecolor="#333", linewidth=0.3)
    ax.set_yticks(y)
    ax.set_yticklabels(names, fontfamily="monospace", fontsize=7.5)
    ax.axvline(x=0, color="black", linewidth=0.8)
    ax.set_xlabel("Δ Self time % (positive = slower in new)")
    ax.set_title(f"Self Time Delta (0702→0708) - {instance_name} (n={dim})", fontsize=12, fontweight="bold")
    ax.grid(axis="x", alpha=0.25, linestyle="--")
    max_abs = max(abs(d) for d in deltas) if deltas else 1
    ax.set_xlim(-max_abs * 1.2, max_abs * 1.2)
    for i, (d, name) in enumerate(zip(deltas, reversed(top))):
        x_pos = d + (max_abs * 0.03 if d >= 0 else -max_abs * 0.03)
        ha = "left" if d >= 0 else "right"
        ax.text(x_pos, len(deltas) - 1 - i, f"{d:+.2f}%", va="center", ha=ha, fontsize=7)
    fig.tight_layout()
    path2 = os.path.join(out_dir, f"{instance_name}_05_self_time_delta.png")
    fig.savefig(path2, dpi=180, bbox_inches="tight")
    plt.close(fig)
    paths.append(path2)

    # --- Chart 3: Inclusive time delta ---
    incl_deltas = [c["delta_incl_pct"] for c in reversed(top)]
    incl_colors = ["#fc4e2a" if d > 0 else "#238b45" for d in incl_deltas]

    fig, ax = plt.subplots(figsize=(11, fig_h))
    ax.barh(y, incl_deltas, color=incl_colors, edgecolor="#333", linewidth=0.3)
    ax.set_yticks(y)
    ax.set_yticklabels(names, fontfamily="monospace", fontsize=7.5)
    ax.axvline(x=0, color="black", linewidth=0.8)
    ax.set_xlabel("Δ Inclusive time % (positive = slower in new)")
    ax.set_title(f"Inclusive Time Delta (0702→0708) - {instance_name} (n={dim})", fontsize=12, fontweight="bold")
    ax.grid(axis="x", alpha=0.25, linestyle="--")
    max_abs_incl = max(abs(d) for d in incl_deltas) if incl_deltas else 1
    ax.set_xlim(-max_abs_incl * 1.2, max_abs_incl * 1.2)
    for i, d in enumerate(incl_deltas):
        x_pos = d + (max_abs_incl * 0.03 if d >= 0 else -max_abs_incl * 0.03)
        ha = "left" if d >= 0 else "right"
        ax.text(x_pos, len(incl_deltas) - 1 - i, f"{d:+.2f}%", va="center", ha=ha, fontsize=7)
    fig.tight_layout()
    path3 = os.path.join(out_dir, f"{instance_name}_06_inclusive_time_delta.png")
    fig.savefig(path3, dpi=180, bbox_inches="tight")
    plt.close(fig)
    paths.append(path3)

    return paths


def generate_overall_summary_chart(all_results: list[dict], out_dir: str) -> str:
    """Generate overall summary chart: performance comparison across all instances."""
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
        import numpy as np
    except ModuleNotFoundError:
        return ""

    successful = [r for r in all_results if not r.get("skipped")]
    if not successful:
        return ""

    names = [r["instance"] for r in successful]
    old_times = [(r.get("old_native_sec") or 0) * 1000 for r in successful]  # ms
    new_times = [(r.get("new_native_sec") or 0) * 1000 for r in successful]  # ms
    dims = [r["dim"] for r in successful]

    fig, axes = plt.subplots(1, 2, figsize=(16, max(6, len(names) * 0.4)))

    # --- Subplot 1: Side-by-side time comparison ---
    ax = axes[0]
    y = list(range(len(names)))
    height = 0.35
    ax.barh([yi + height/2 for yi in y], old_times, height=height,
            color="#386cb0", edgecolor="#333", linewidth=0.3, label="Old (0702)")
    ax.barh([yi - height/2 for yi in y], new_times, height=height,
            color="#fb6a4a", edgecolor="#333", linewidth=0.3, label="New (0708)")
    ax.set_yticks(y)
    ax.set_yticklabels([f"{n} (n={d})" for n, d in zip(names, dims)], fontsize=8)
    ax.set_xlabel("Elapsed time (ms)")
    ax.set_title("Native Elapsed Time Comparison", fontsize=12, fontweight="bold")
    ax.legend(loc="lower right")
    ax.grid(axis="x", alpha=0.25, linestyle="--")
    max_t = max(max(old_times), max(new_times)) if old_times else 1
    ax.set_xlim(0, max_t * 1.2)

    # --- Subplot 2: Speedup ratio ---
    ax = axes[1]
    ratios = []
    ratio_labels = []
    for r, ot, nt in zip(successful, old_times, new_times):
        if ot > 0 and nt > 0:
            ratio = ot / nt  # >1 means new is faster
            ratios.append(ratio)
            ratio_labels.append(f"{r['instance']} (n={r['dim']})")
        else:
            ratios.append(1.0)
            ratio_labels.append(f"{r['instance']} (n={r['dim']})")

    colors = ["#238b45" if r >= 1.0 else "#fc4e2a" for r in ratios]
    bars = ax.barh(list(range(len(ratios))), [r - 1.0 for r in ratios], left=[1.0]*len(ratios),
                   color=colors, edgecolor="#333", linewidth=0.3)
    ax.axvline(x=1.0, color="black", linewidth=1.0, linestyle="--")
    ax.set_yticks(list(range(len(ratios))))
    ax.set_yticklabels(ratio_labels, fontsize=8)
    ax.set_xlabel("Speedup ratio (new/old, >1 = new faster)")
    ax.set_title("Speedup Ratio (0702→0708)", fontsize=12, fontweight="bold")
    ax.grid(axis="x", alpha=0.25, linestyle="--")
    for i, r in enumerate(ratios):
        ax.text(r + 0.02, i, f"{r:.2f}x", va="center", fontsize=8)

    fig.tight_layout()
    path = os.path.join(out_dir, "00_overall_comparison.png")
    fig.savefig(path, dpi=180, bbox_inches="tight")
    plt.close(fig)
    return path


def write_summary_md(all_results: list[dict], out_path: str) -> str:
    """Write overall summary markdown comparing all instances."""
    with open(out_path, "w") as f:
        f.write("# Heatmap Comparison Summary\n\n")
        f.write("## Solver Versions\n\n")
        f.write("- **Old**: `solver/tsp_bb_26_07_02/tsp_bb`\n")
        f.write("- **New**: `solver/tsp_bb_26_07_08/tsp_bb`\n\n")

        successful = [r for r in all_results if not r.get("skipped")]
        timeout_instances = [r for r in all_results if r.get("skipped_reason") == "timeout"]
        error_instances = [r for r in all_results if r.get("skipped_reason") == "error"]

        f.write(f"## Overview\n\n")
        f.write(f"- Total instances: {len(all_results)}\n")
        f.write(f"- Successfully compared: {len(successful)}\n")
        f.write(f"- Timeout (excluded): {len(timeout_instances)}\n")
        f.write(f"- Error (excluded): {len(error_instances)}\n\n")

        if timeout_instances:
            f.write("### Timeout Instances (Excluded)\n\n")
            for r in timeout_instances:
                f.write(f"- {r['instance']} (n={r['dim']})\n")
            f.write("\n")

        if error_instances:
            f.write("### Error Instances (Excluded)\n\n")
            for r in error_instances:
                f.write(f"- {r['instance']} (n={r['dim']}): {r.get('error_msg', 'unknown')}\n")
            f.write("\n")

        if not successful:
            f.write("**No successful comparisons.**\n")
            return out_path

        # Overall performance table
        f.write("## Overall Performance\n\n")
        f.write("| Instance | n | Old Time | New Time | Delta | Old Ir | New Ir | Top Changer |\n")
        f.write("|---|---|---|---|---|---|---|---|\n")

        for r in successful:
            name = r["instance"]
            dim = r["dim"]
            old_time = fmt_seconds(r.get("old_native_sec"))
            new_time = fmt_seconds(r.get("new_native_sec"))
            if r.get("old_native_sec") and r.get("new_native_sec"):
                delta = r["new_native_sec"] - r["old_native_sec"]
                pct = (delta / r["old_native_sec"] * 100) if r["old_native_sec"] > 0 else 0
                delta_str = f"{fmt_seconds(delta)} ({pct:+.1f}%)"
            else:
                delta_str = "-"
            old_ir = r.get("old_program_ir", 0)
            new_ir = r.get("new_program_ir", 0)
            top_changer = r.get("top_changer", "-")
            if top_changer and len(top_changer) > 35:
                top_changer = top_changer[:32] + "..."

            f.write(f"| {name} | {dim} | {old_time} | {new_time} | {delta_str} "
                    f"| {old_ir:,} | {new_ir:,} | `{top_changer}` |\n")

        # Cross-instance function trend analysis
        f.write("\n## Cross-Instance Function Trends\n\n")
        f.write("Functions that consistently changed across multiple instances:\n\n")

        # Aggregate function deltas across all instances
        func_deltas = defaultdict(list)
        for r in successful:
            for ch in r.get("changes", []):
                func_deltas[ch["function"]].append({
                    "instance": r["instance"],
                    "delta_self": ch["delta_self_pct"],
                    "old_self": ch["old_self_pct"],
                    "new_self": ch["new_self_pct"],
                })

        # Show functions that changed in >= 2 instances with significant delta
        trending = []
        for func, entries in func_deltas.items():
            if len(entries) < 2:
                continue
            avg_delta = sum(e["delta_self"] for e in entries) / len(entries)
            max_abs_delta = max(abs(e["delta_self"]) for e in entries)
            if max_abs_delta < 0.5:
                continue
            trending.append((func, entries, avg_delta, max_abs_delta))

        trending.sort(key=lambda x: -x[3])  # sort by max absolute delta

        f.write("| Function | Instances | Avg Delta | Max Abs Delta | Direction |\n")
        f.write("|---|---|---|---|---|\n")
        for func, entries, avg_delta, max_abs_delta in trending[:30]:
            instance_names = ", ".join(e["instance"] for e in entries)
            direction = "🔺 slower" if avg_delta > 0 else "🔻 faster" if avg_delta < 0 else "➡ same"
            f.write(f"| `{func[:55]}` | {instance_names} "
                    f"| {avg_delta:+.2f}% | {max_abs_delta:.2f}% | {direction} |\n")

        # Solver vs runtime overhead trend
        f.write("\n## Solver vs Runtime Overhead Trend\n\n")
        f.write("| Instance | Old Solver% | New Solver% | Old Runtime% | New Runtime% | Solver Δ |\n")
        f.write("|---|---|---|---|---|---|\n")
        for r in successful:
            old_s = r.get("old_solver_self", 0)
            new_s = r.get("new_solver_self", 0)
            old_r = r.get("old_runtime_self", 0)
            new_r = r.get("new_runtime_self", 0)
            f.write(f"| {r['instance']} | {old_s:.1f}% | {new_s:.1f}% "
                    f"| {old_r:.1f}% | {new_r:.1f}% | {new_s - old_s:+.1f}% |\n")

    return out_path


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Batch heatmap comparison between two tsp_bb solver versions")
    parser.add_argument("--solver-old", required=True,
                        help="Path to old solver binary (e.g., solver/tsp_bb_26_07_02/tsp_bb)")
    parser.add_argument("--solver-new", required=True,
                        help="Path to new solver binary (e.g., solver/tsp_bb_26_07_08/tsp_bb)")
    parser.add_argument("--max-size", type=int, default=38,
                        help="Maximum instance dimension (default: 38)")
    parser.add_argument("--min-size", type=int, default=0,
                        help="Minimum instance dimension (default: 0)")
    parser.add_argument("--timeout", type=int, default=600,
                        help="Native solver timeout per instance in seconds (default: 600)")
    parser.add_argument("--out-dir", default=None,
                        help="Output root directory (default: docs/heatmap)")
    parser.add_argument("--no-profile", action="store_true",
                        help="Skip profiling; only generate comparison from existing data")
    args = parser.parse_args()

    solver_old = os.path.abspath(args.solver_old)
    solver_new = os.path.abspath(args.solver_new)
    out_root = args.out_dir or DEFAULT_OUT_ROOT

    if not os.path.isfile(solver_old):
        raise SystemExit(f"Old solver not found: {solver_old}")
    if not os.path.isfile(solver_new):
        raise SystemExit(f"New solver not found: {solver_new}")

    # Determine version labels
    old_label = os.path.basename(os.path.dirname(solver_old)).replace("tsp_bb_", "").replace("_", "")
    new_label = os.path.basename(os.path.dirname(solver_new)).replace("tsp_bb_", "").replace("_", "")

    instances = find_instances(args.max_size, args.min_size)
    eprint(f"Found {len(instances)} instances with size {args.min_size}-{args.max_size}:")
    for dim, name, path in instances:
        eprint(f"  n={dim:>4}  {name}")

    # Create comparison output directory
    compare_dir = os.path.join(PROJECT_ROOT, "docs", "heatmap",
                               f"compare_{old_label}_vs_{new_label}")
    os.makedirs(compare_dir, exist_ok=True)

    all_results = []

    for dim, name, path in instances:
        eprint(f"\n{'='*70}")
        eprint(f"[{name}] n={dim}  →  {path}")
        eprint(f"{'='*70}")

        instance_out = os.path.join(out_root, name)
        result = {"instance": name, "dim": dim, "path": path}

        if not args.no_profile:
            # Profile old solver
            eprint(f"  [1/2] Profiling OLD solver ({old_label})...")
            old_result = run_heatmap_for_solver(
                path, solver_old, instance_out, args.timeout, old_label)

            if old_result.get("timeout") or old_result.get("error"):
                result["skipped"] = True
                result["skipped_reason"] = "timeout" if old_result.get("timeout") else "error"
                result["error_msg"] = old_result.get("stderr", "")[:200]
                eprint(f"  ⚠ OLD solver failed/timeout, skipping {name}")
                all_results.append(result)
                continue

            # Profile new solver
            eprint(f"  [2/2] Profiling NEW solver ({new_label})...")
            new_result = run_heatmap_for_solver(
                path, solver_new, instance_out, args.timeout, new_label)

            if new_result.get("timeout") or new_result.get("error"):
                result["skipped"] = True
                result["skipped_reason"] = "timeout" if new_result.get("timeout") else "error"
                result["error_msg"] = new_result.get("stderr", "")[:200]
                eprint(f"  ⚠ NEW solver failed/timeout, skipping {name}")
                all_results.append(result)
                continue

            result["old_out_dir"] = old_result["out_dir"]
            result["new_out_dir"] = new_result["out_dir"]
            result["old_meta"] = old_result.get("metadata", {})
            result["new_meta"] = new_result.get("metadata", {})
        else:
            old_dir = f"{instance_out}_{old_label}"
            new_dir = f"{instance_out}_{new_label}"
            if not os.path.isdir(old_dir) or not os.path.isdir(new_dir):
                eprint(f"  ⚠ Missing profile data, skipping {name}")
                result["skipped"] = True
                result["skipped_reason"] = "missing_data"
                all_results.append(result)
                continue
            result["old_out_dir"] = old_dir
            result["new_out_dir"] = new_dir

        # Compare CSVs
        old_csv = os.path.join(result["old_out_dir"], "function_time_breakdown.csv")
        new_csv = os.path.join(result["new_out_dir"], "function_time_breakdown.csv")
        comparison = compare_csvs(old_csv, new_csv)
        result["changes"] = comparison["changes"]

        if comparison["changes"]:
            result["top_changer"] = comparison["changes"][0]["function"]

        # Extract metadata
        old_meta = result.get("old_meta", {})
        new_meta = result.get("new_meta", {})
        if not old_meta:
            old_meta = _load_meta(result["old_out_dir"])
        if not new_meta:
            new_meta = _load_meta(result["new_out_dir"])

        result["old_native_sec"] = old_meta.get("native_elapsed_sec")
        result["new_native_sec"] = new_meta.get("native_elapsed_sec")

        # Compute solver vs runtime aggregates
        old_s = sum(c["old_self_pct"] for c in comparison["changes"] if c["kind"] == "solver")
        new_s = sum(c["new_self_pct"] for c in comparison["changes"] if c["kind"] == "solver")
        old_r = sum(c["old_self_pct"] for c in comparison["changes"] if c["kind"] == "runtime")
        new_r = sum(c["new_self_pct"] for c in comparison["changes"] if c["kind"] == "runtime")
        result["old_solver_self"] = old_s
        result["new_solver_self"] = new_s
        result["old_runtime_self"] = old_r
        result["new_runtime_self"] = new_r

        # Count Ir from report
        result["old_program_ir"] = _get_ir_from_report(result["old_out_dir"])
        result["new_program_ir"] = _get_ir_from_report(result["new_out_dir"])

        # Generate comparison heatmaps
        eprint(f"  Generating comparison heatmaps...")
        comp_plots = generate_comparison_heatmaps(
            name, dim, comparison["changes"],
            result.get("old_native_sec"), result.get("new_native_sec"),
            compare_dir)
        for p in comp_plots:
            eprint(f"    {os.path.basename(p)}")

        # Write per-instance comparison
        instance_md = os.path.join(compare_dir, f"{name}_comparison.md")
        write_instance_comparison_md(name, dim, old_meta, new_meta, comparison, instance_md)
        eprint(f"  ✓ Comparison written: {instance_md}")

        all_results.append(result)

    # Generate overall summary chart
    eprint(f"\nGenerating overall summary chart...")
    overall_chart = generate_overall_summary_chart(all_results, compare_dir)
    if overall_chart:
        eprint(f"  {os.path.basename(overall_chart)}")

    # Write summary
    summary_path = os.path.join(compare_dir, "SUMMARY.md")
    write_summary_md(all_results, summary_path)
    eprint(f"\n{'='*70}")
    eprint(f"Summary written: {summary_path}")

    # Print quick summary to stderr
    successful = [r for r in all_results if not r.get("skipped")]
    skipped = [r for r in all_results if r.get("skipped")]
    eprint(f"\nResults: {len(successful)} compared, {len(skipped)} skipped")

    for r in successful:
        ot = fmt_seconds(r.get("old_native_sec"))
        nt = fmt_seconds(r.get("new_native_sec"))
        delta_str = ""
        if r.get("old_native_sec") and r.get("new_native_sec"):
            pct = (r["new_native_sec"] - r["old_native_sec"]) / r["old_native_sec"] * 100
            delta_str = f" ({pct:+.1f}%)"
        eprint(f"  {r['instance']:>15s} n={r['dim']:>2}: {ot} → {nt}{delta_str}")


def _load_meta(out_dir: str) -> dict:
    import json
    meta_path = os.path.join(out_dir, "profile_metadata.json")
    if os.path.exists(meta_path):
        with open(meta_path) as f:
            return json.load(f)
    return {}


def _get_ir_from_report(out_dir: str) -> int:
    report_path = os.path.join(out_dir, "report.md")
    if not os.path.exists(report_path):
        return 0
    with open(report_path) as f:
        for line in f:
            m = re.search(r"Program total: `([\d,]+)` Ir", line)
            if m:
                return int(m.group(1).replace(",", ""))
    return 0


if __name__ == "__main__":
    main()
