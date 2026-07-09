#!/usr/bin/env python3
"""
Three-way heatmap comparison: 07_09 vs 07_02 and 07_09 vs 07_08.

Loads existing CSV profiles and generates comparison charts and summaries.
"""
from __future__ import annotations

import csv
import json
import os
import re
import sys
from collections import defaultdict
from dataclasses import dataclass, field

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))


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


def load_csv(csv_path: str) -> list[dict]:
    rows = []
    if not os.path.exists(csv_path):
        return rows
    with open(csv_path, newline="") as f:
        for row in csv.DictReader(f):
            rows.append(row)
    return rows


def load_meta(profile_dir: str) -> dict:
    meta_path = os.path.join(profile_dir, "profile_metadata.json")
    if os.path.exists(meta_path):
        with open(meta_path) as f:
            return json.load(f)
    return {}


def normalize_func(name: str) -> str:
    name = re.sub(r"<[^<>]*>", "<>", name)
    name = re.sub(r"\([^)]{20,}\)", "(...)", name)
    name = re.sub(r"'[0-9]+$", "", name)
    return name.strip()


def compare_two_csvs(old_csv: str, new_csv: str) -> list[dict]:
    old_rows = load_csv(old_csv)
    new_rows = load_csv(new_csv)
    old_by = {normalize_func(r["function"]): r for r in old_rows}
    new_by = {normalize_func(r["function"]): r for r in new_rows}
    all_funcs = set(old_by) | set(new_by)
    changes = []
    for func in all_funcs:
        o = old_by.get(func)
        n = new_by.get(func)
        op = float(o["self_pct_total"]) if o else 0.0
        np = float(n["self_pct_total"]) if n else 0.0
        oi = float(o["inclusive_pct_total"]) if o else 0.0
        ni = float(n["inclusive_pct_total"]) if n else 0.0
        if max(op, np) < 0.5 and max(oi, ni) < 1.0:
            continue
        changes.append({
            "function": func,
            "kind": (n or o).get("kind", "other"),
            "old_self_pct": op, "new_self_pct": np, "delta_self_pct": np - op,
            "old_incl_pct": oi, "new_incl_pct": ni, "delta_incl_pct": ni - oi,
            "old_est_sec": float(o["self_est_sec"]) if o and o.get("self_est_sec") else None,
            "new_est_sec": float(n["self_est_sec"]) if n and n.get("self_est_sec") else None,
        })
    changes.sort(key=lambda c: (-abs(c["delta_self_pct"]), -max(c["old_self_pct"], c["new_self_pct"])))
    return changes


def generate_comparison_charts(instance_name: str, dim: int, changes: list[dict],
                                old_label: str, new_label: str, out_dir: str) -> list[str]:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    paths = []
    top = changes[:20]
    if not top:
        return paths

    names = [c["function"][:55] for c in reversed(top)]
    old_pcts = [c["old_self_pct"] for c in reversed(top)]
    new_pcts = [c["new_self_pct"] for c in reversed(top)]
    y = list(range(len(names)))
    fig_h = max(6.0, len(top) * 0.36)

    # Chart 1: Side-by-side self time %
    fig, ax = plt.subplots(figsize=(14, fig_h))
    height = 0.35
    ax.barh([yi + height/2 for yi in y], old_pcts, height=height,
            color="#386cb0", edgecolor="#333", linewidth=0.3, label=f"Old ({old_label})")
    ax.barh([yi - height/2 for yi in y], new_pcts, height=height,
            color="#fb6a4a", edgecolor="#333", linewidth=0.3, label=f"New ({new_label})")
    ax.set_yticks(y)
    ax.set_yticklabels(names, fontfamily="monospace", fontsize=7.5)
    ax.set_xlabel("Self time % of total program Ir")
    ax.set_title(f"Self Time Share - {instance_name} (n={dim}): {old_label} vs {new_label}",
                 fontsize=12, fontweight="bold")
    ax.legend(loc="lower right"); ax.grid(axis="x", alpha=0.25, linestyle="--")
    max_pct = max(max(old_pcts), max(new_pcts)) if top else 1
    ax.set_xlim(0, max_pct * 1.18)
    fig.tight_layout()
    p1 = os.path.join(out_dir, f"{instance_name}_self_time_{old_label}_vs_{new_label}.png")
    fig.savefig(p1, dpi=180, bbox_inches="tight"); plt.close(fig); paths.append(p1)

    # Chart 2: Self time delta
    deltas = [c["delta_self_pct"] for c in reversed(top)]
    colors = ["#fc4e2a" if d > 0 else "#238b45" for d in deltas]
    fig, ax = plt.subplots(figsize=(11, fig_h))
    ax.barh(y, deltas, color=colors, edgecolor="#333", linewidth=0.3)
    ax.set_yticks(y); ax.set_yticklabels(names, fontfamily="monospace", fontsize=7.5)
    ax.axvline(x=0, color="black", linewidth=0.8)
    ax.set_xlabel("Δ Self time % (positive = slower in new)")
    ax.set_title(f"Self Time Delta ({old_label}→{new_label}) - {instance_name} (n={dim})",
                 fontsize=12, fontweight="bold")
    ax.grid(axis="x", alpha=0.25, linestyle="--")
    max_abs = max(abs(d) for d in deltas) if deltas else 1
    ax.set_xlim(-max_abs * 1.2, max_abs * 1.2)
    for i, d in enumerate(deltas):
        x_pos = d + (max_abs * 0.03 if d >= 0 else -max_abs * 0.03)
        ax.text(x_pos, i, f"{d:+.2f}%", va="center",
                ha="left" if d >= 0 else "right", fontsize=7)
    fig.tight_layout()
    p2 = os.path.join(out_dir, f"{instance_name}_self_delta_{old_label}_vs_{new_label}.png")
    fig.savefig(p2, dpi=180, bbox_inches="tight"); plt.close(fig); paths.append(p2)

    # Chart 3: Inclusive time delta
    incl_deltas = [c["delta_incl_pct"] for c in reversed(top)]
    incl_colors = ["#fc4e2a" if d > 0 else "#238b45" for d in incl_deltas]
    fig, ax = plt.subplots(figsize=(11, fig_h))
    ax.barh(y, incl_deltas, color=incl_colors, edgecolor="#333", linewidth=0.3)
    ax.set_yticks(y); ax.set_yticklabels(names, fontfamily="monospace", fontsize=7.5)
    ax.axvline(x=0, color="black", linewidth=0.8)
    ax.set_xlabel("Δ Inclusive time % (positive = slower in new)")
    ax.set_title(f"Inclusive Time Delta ({old_label}→{new_label}) - {instance_name} (n={dim})",
                 fontsize=12, fontweight="bold")
    ax.grid(axis="x", alpha=0.25, linestyle="--")
    max_abs_incl = max(abs(d) for d in incl_deltas) if incl_deltas else 1
    ax.set_xlim(-max_abs_incl * 1.2, max_abs_incl * 1.2)
    for i, d in enumerate(incl_deltas):
        x_pos = d + (max_abs_incl * 0.03 if d >= 0 else -max_abs_incl * 0.03)
        ax.text(x_pos, i, f"{d:+.2f}%", va="center",
                ha="left" if d >= 0 else "right", fontsize=7)
    fig.tight_layout()
    p3 = os.path.join(out_dir, f"{instance_name}_incl_delta_{old_label}_vs_{new_label}.png")
    fig.savefig(p3, dpi=180, bbox_inches="tight"); plt.close(fig); paths.append(p3)

    return paths


def generate_overall_summary(all_results: list[dict], comparison_label: str, out_dir: str) -> str:
    import matplotlib
    matplotlib.use("Agg")
    import matplotlib.pyplot as plt

    successful = [r for r in all_results if not r.get("skipped")]
    if not successful:
        return ""
    names = [r["instance"] for r in successful]
    old_times = [(r.get("old_native_sec") or 0) * 1000 for r in successful]
    new_times = [(r.get("new_native_sec") or 0) * 1000 for r in successful]
    dims = [r["dim"] for r in successful]

    fig, axes = plt.subplots(1, 2, figsize=(16, max(6, len(names) * 0.5)))
    y = list(range(len(names)))
    height = 0.35

    # Subplot 1: Side-by-side
    ax = axes[0]
    ax.barh([yi + height/2 for yi in y], old_times, height=height,
            color="#386cb0", edgecolor="#333", linewidth=0.3, label=f"Old ({comparison_label.split('_vs_')[0]})")
    ax.barh([yi - height/2 for yi in y], new_times, height=height,
            color="#fb6a4a", edgecolor="#333", linewidth=0.3, label=f"New ({comparison_label.split('_vs_')[1]})")
    ax.set_yticks(y)
    ax.set_yticklabels([f"{n} (n={d})" for n, d in zip(names, dims)], fontsize=8)
    ax.set_xlabel("Elapsed time (ms)")
    ax.set_title(f"Native Elapsed Time: {comparison_label}", fontsize=12, fontweight="bold")
    ax.legend(loc="lower right"); ax.grid(axis="x", alpha=0.25, linestyle="--")
    max_t = max(max(old_times), max(new_times)) if old_times else 1
    ax.set_xlim(0, max_t * 1.2)

    # Subplot 2: Speedup
    ax = axes[1]
    ratios, rlabels = [], []
    for r, ot, nt in zip(successful, old_times, new_times):
        ratio = (ot / nt) if ot > 0 and nt > 0 else 1.0
        ratios.append(ratio)
        rlabels.append(f"{r['instance']} (n={r['dim']})")
    colors = ["#238b45" if r >= 1.0 else "#fc4e2a" for r in ratios]
    ax.barh(list(range(len(ratios))), [r - 1.0 for r in ratios], left=[1.0]*len(ratios),
            color=colors, edgecolor="#333", linewidth=0.3)
    ax.axvline(x=1.0, color="black", linewidth=1.0, linestyle="--")
    ax.set_yticks(list(range(len(ratios))))
    ax.set_yticklabels(rlabels, fontsize=8)
    ax.set_xlabel(f"Speedup ratio ({comparison_label.split('_vs_')[1]}/{comparison_label.split('_vs_')[0]}, >1 = new faster)")
    ax.set_title(f"Speedup: {comparison_label}", fontsize=12, fontweight="bold")
    ax.grid(axis="x", alpha=0.25, linestyle="--")
    for i, r in enumerate(ratios):
        ax.text(r + 0.02, i, f"{r:.2f}x", va="center", fontsize=8)

    fig.tight_layout()
    path = os.path.join(out_dir, f"00_overall_{comparison_label}.png")
    fig.savefig(path, dpi=180, bbox_inches="tight"); plt.close(fig)
    return path


def write_summary(all_results: list[dict], old_label: str, new_label: str, out_path: str) -> str:
    with open(out_path, "w") as f:
        f.write(f"# Heatmap Comparison: {old_label} vs {new_label}\n\n")
        f.write(f"- **Old**: `solver/tsp_bb_{old_label}/tsp_bb`\n")
        f.write(f"- **New**: `solver/tsp_bb_{new_label}/tsp_bb`\n\n")
        successful = [r for r in all_results if not r.get("skipped")]
        skipped = [r for r in all_results if r.get("skipped")]
        f.write(f"## Overview\n\n")
        f.write(f"- Total: {len(all_results)}, Compared: {len(successful)}, Skipped: {len(skipped)}\n\n")

        # Performance table
        f.write("## Overall Performance\n\n")
        f.write("| Instance | n | Old Time | New Time | Delta | Old Ir | New Ir | Top Changer |\n")
        f.write("|---|---|---|---|---|---|---|---|\n")
        for r in successful:
            ot = fmt_seconds(r.get("old_native_sec"))
            nt = fmt_seconds(r.get("new_native_sec"))
            if r.get("old_native_sec") and r.get("new_native_sec"):
                delta = r["new_native_sec"] - r["old_native_sec"]
                pct = (delta / r["old_native_sec"] * 100) if r["old_native_sec"] > 0 else 0
                ds = f"{fmt_seconds(delta)} ({pct:+.1f}%)"
            else:
                ds = "-"
            tc = r.get("top_changer", "-")
            if tc and len(tc) > 35:
                tc = tc[:32] + "..."
            f.write(f"| {r['instance']} | {r['dim']} | {ot} | {nt} | {ds} "
                    f"| {r.get('old_program_ir', 0):,} | {r.get('new_program_ir', 0):,} | `{tc}` |\n")

        # Cross-instance function trends
        f.write("\n## Cross-Instance Function Trends\n\n")
        func_deltas = defaultdict(list)
        for r in successful:
            for ch in r.get("changes", []):
                func_deltas[ch["function"]].append({
                    "instance": r["instance"], "delta": ch["delta_self_pct"],
                    "old": ch["old_self_pct"], "new": ch["new_self_pct"],
                })
        trending = []
        for func, entries in func_deltas.items():
            if len(entries) < 2:
                continue
            avg_d = sum(e["delta"] for e in entries) / len(entries)
            max_abs = max(abs(e["delta"]) for e in entries)
            if max_abs < 0.5:
                continue
            trending.append((func, entries, avg_d, max_abs))
        trending.sort(key=lambda x: -x[3])
        f.write("| Function | Instances | Avg Delta | Max Abs Delta | Direction |\n")
        f.write("|---|---|---|---|---|\n")
        for func, entries, avg_d, max_abs in trending[:30]:
            inames = ", ".join(e["instance"] for e in entries)
            d = "🔺 slower" if avg_d > 0 else "🔻 faster" if avg_d < 0 else "➡ same"
            f.write(f"| `{func[:55]}` | {inames} | {avg_d:+.2f}% | {max_abs:.2f}% | {d} |\n")

        # Solver vs runtime trend
        f.write("\n## Solver vs Runtime Overhead Trend\n\n")
        f.write("| Instance | Old Solver% | New Solver% | Old Runtime% | New Runtime% | Solver Δ |\n")
        f.write("|---|---|---|---|---|---|\n")
        for r in successful:
            f.write(f"| {r['instance']} | {r.get('old_solver_self',0):.1f}% | {r.get('new_solver_self',0):.1f}% "
                    f"| {r.get('old_runtime_self',0):.1f}% | {r.get('new_runtime_self',0):.1f}% "
                    f"| {r.get('new_solver_self',0) - r.get('old_solver_self',0):+.1f}% |\n")
    return out_path


def main():
    instances = [
        ("burma14", 14, "data/classic/tsplib/burma14.tsp"),
        ("gr17", 17, "data/classic/tsplib/gr17.tsp"),
        ("gr21", 21, "data/classic/tsplib/gr21.tsp"),
        ("gr24", 24, "data/classic/tsplib/gr24.tsp"),
        ("fri26", 26, "data/classic/tsplib/fri26.tsp"),
        ("dj38", 38, "data/classic/national/dj38.tsp"),
    ]

    versions = {
        "260702": "solver/tsp_bb_26_07_02/tsp_bb",
        "260708": "solver/tsp_bb_26_07_08/tsp_bb",
        "260709": "solver/tsp_bb_26_07_09/tsp_bb",
    }

    comparisons = [("260702", "260709"), ("260708", "260709")]

    for old_ver, new_ver in comparisons:
        comp_label = f"{old_ver}_vs_{new_ver}"
        out_dir = os.path.join(PROJECT_ROOT, "docs", "heatmap", f"compare_{comp_label}")
        os.makedirs(out_dir, exist_ok=True)
        print(f"\n{'='*70}")
        print(f"Comparison: {old_ver} vs {new_ver}")
        print(f"Output: {out_dir}")
        print(f"{'='*70}")

        all_results = []
        for name, dim, _ in instances:
            old_dir = os.path.join(PROJECT_ROOT, "docs", "heatmap", f"{name}_{old_ver}")
            new_dir = os.path.join(PROJECT_ROOT, "docs", "heatmap", f"{name}_{new_ver}")
            if not os.path.isdir(old_dir) or not os.path.isdir(new_dir):
                print(f"  [{name}] SKIP: missing profile data")
                all_results.append({"instance": name, "dim": dim, "skipped": True})
                continue

            old_csv = os.path.join(old_dir, "function_time_breakdown.csv")
            new_csv = os.path.join(new_dir, "function_time_breakdown.csv")
            changes = compare_two_csvs(old_csv, new_csv)
            old_meta = load_meta(old_dir)
            new_meta = load_meta(new_dir)
            old_native = old_meta.get("native_elapsed_sec")
            new_native = new_meta.get("native_elapsed_sec")
            top_changer = changes[0]["function"] if changes else "-"

            # Program Ir from report
            old_ir = 0
            new_ir = 0
            for ver, vdir in [(old_ver, old_dir), (new_ver, new_dir)]:
                rp = os.path.join(vdir, "report.md")
                if os.path.exists(rp):
                    with open(rp) as f:
                        for line in f:
                            m = re.search(r"Program total: `([\d,]+)` Ir", line)
                            if m:
                                if ver == old_ver:
                                    old_ir = int(m.group(1).replace(",", ""))
                                else:
                                    new_ir = int(m.group(1).replace(",", ""))
                                break

            old_s = sum(c["old_self_pct"] for c in changes if c["kind"] == "solver")
            new_s = sum(c["new_self_pct"] for c in changes if c["kind"] == "solver")
            old_r = sum(c["old_self_pct"] for c in changes if c["kind"] == "runtime")
            new_r = sum(c["new_self_pct"] for c in changes if c["kind"] == "runtime")

            result = {
                "instance": name, "dim": dim, "skipped": False,
                "changes": changes, "top_changer": top_changer,
                "old_native_sec": old_native, "new_native_sec": new_native,
                "old_program_ir": old_ir, "new_program_ir": new_ir,
                "old_solver_self": old_s, "new_solver_self": new_s,
                "old_runtime_self": old_r, "new_runtime_self": new_r,
            }
            all_results.append(result)

            # Per-instance charts
            print(f"  [{name}] Generating charts...")
            charts = generate_comparison_charts(name, dim, changes, old_ver, new_ver, out_dir)
            for c in charts:
                print(f"    {os.path.basename(c)}")

            # Per-instance markdown
            md_path = os.path.join(out_dir, f"{name}_comparison.md")
            with open(md_path, "w") as f:
                f.write(f"# {name} (n={dim}): {old_ver} vs {new_ver}\n\n")
                f.write("## Performance\n\n")
                ot = fmt_seconds(old_native); nt = fmt_seconds(new_native)
                f.write(f"- Old ({old_ver}): {ot}\n")
                f.write(f"- New ({new_ver}): {nt}\n")
                if old_native and new_native:
                    d = new_native - old_native
                    pct = (d / old_native * 100) if old_native > 0 else 0
                    f.write(f"- Delta: {fmt_seconds(d)} ({pct:+.1f}%)\n")
                f.write(f"- Old Ir: {old_ir:,}, New Ir: {new_ir:,}\n")
                f.write(f"- Solver self%: {old_s:.1f}% → {new_s:.1f}% ({new_s - old_s:+.1f}%)\n")
                f.write(f"- Runtime self%: {old_r:.1f}% → {new_r:.1f}% ({new_r - old_r:+.1f}%)\n")
                f.write("\n## Top Changes\n\n")
                f.write("| # | Function | Kind | Old Self% | New Self% | Delta |\n")
                f.write("|---:|---|---|---:|---:|---:|\n")
                for i, ch in enumerate(changes[:25], 1):
                    f.write(f"| {i} | `{ch['function'][:55]}` | {ch['kind']} "
                            f"| {ch['old_self_pct']:.2f}% | {ch['new_self_pct']:.2f}% "
                            f"| {ch['delta_self_pct']:+.2f}% |\n")
            print(f"  [{name}] ✓")

        # Overall summary
        print(f"\n  Generating overall summary...")
        chart = generate_overall_summary(all_results, comp_label, out_dir)
        if chart:
            print(f"    {os.path.basename(chart)}")
        summary_path = os.path.join(out_dir, "SUMMARY.md")
        write_summary(all_results, old_ver, new_ver, summary_path)
        print(f"  Summary: {summary_path}")

        succ = [r for r in all_results if not r.get("skipped")]
        print(f"\n  Results: {len(succ)} compared, {len(all_results) - len(succ)} skipped")
        for r in succ:
            ot = fmt_seconds(r.get("old_native_sec"))
            nt = fmt_seconds(r.get("new_native_sec"))
            if r.get("old_native_sec") and r.get("new_native_sec"):
                pct = (r["new_native_sec"] - r["old_native_sec"]) / r["old_native_sec"] * 100
                print(f"    {r['instance']:>10} n={r['dim']:>2}: {ot} → {nt} ({pct:+.1f}%)")


if __name__ == "__main__":
    main()
