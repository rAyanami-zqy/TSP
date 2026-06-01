#!/usr/bin/env python3
"""Compare 3 branch strategies (Smart, Exhaustive, Simple) on all TSP instances with < 30 vertices."""

import subprocess
import sys
import os
import re
import time
from pathlib import Path

SOLVER = os.path.join(os.path.dirname(__file__), "..", "build", "tsp_bb")
SEARCH_ROOTS = [
    os.path.join(os.path.dirname(__file__), "..", "data"),
    os.path.join(os.path.dirname(__file__), "..", "examples"),
]
MAX_N = 30  # strictly less than 30
TIMEOUT = 3600  # 1 hour per instance-strategy run
DEBUG_INTERVAL = 10000
STRATEGIES = ["smart", "exhaustive", "simple"]
STRATEGY_LABELS = {"smart": "Smart", "exhaustive": "Exhaustive", "simple": "Simple"}


def parse_dimension_tsp(filepath: str) -> int | None:
    """Parse DIMENSION from TSPLIB header."""
    with open(filepath, "r") as f:
        for line in f:
            line = line.strip()
            if line.upper().startswith("DIMENSION"):
                parts = line.split(":")
                if len(parts) == 2:
                    return int(parts[1].strip())
                parts = line.split()
                if len(parts) >= 2:
                    return int(parts[1])
    return None


def parse_dimension_matrix(filepath: str) -> int | None:
    """Parse dimension from matrix format (first token is n)."""
    with open(filepath, "r") as f:
        content = f.read().split()
        if content:
            try:
                return int(content[0])
            except ValueError:
                return None
    return None


def get_dimension(filepath: str) -> int | None:
    """Try to determine the dimension of a TSP instance."""
    name = os.path.basename(filepath)
    if name.endswith(".opt.tour") or name.startswith("batch"):
        return None
    if name.startswith("."):
        return None
    if name.endswith(".gz"):
        return None

    dim = parse_dimension_tsp(filepath)
    if dim is not None:
        return dim

    dim = parse_dimension_matrix(filepath)
    if dim is not None:
        return dim

    return None


def find_small_instances():
    """Find all instances with dimension < MAX_N."""
    instances = []
    for root in SEARCH_ROOTS:
        if not os.path.isdir(root):
            continue
        for dirpath, dirnames, filenames in os.walk(root):
            if "_archives" in dirpath:
                continue
            for fname in sorted(filenames):
                filepath = os.path.join(dirpath, fname)
                dim = get_dimension(filepath)
                if dim is not None and dim < MAX_N:
                    instances.append((filepath, dim))
    return instances


def parse_stats(stdout: str) -> dict:
    """Extract solver statistics from stdout."""
    stats = {
        "cost": None,
        "feasible": False,
        "root_lb": None,
        "init_ub": None,
        "nodes_created": None,
        "nodes_expanded": None,
        "pruned_bound": None,
        "pruned_infeasible": None,
        "tour": None,
    }

    for line in stdout.splitlines():
        if "Optimal cost:" in line:
            m = re.search(r"Optimal cost:\s*([\d.]+)", line)
            if m:
                stats["cost"] = float(m.group(1))
                stats["feasible"] = True
        elif "Root lower bound:" in line:
            m = re.search(r"Root lower bound:\s*([\d.]+)", line)
            if m:
                stats["root_lb"] = float(m.group(1))
        elif "Initial upper bound:" in line:
            m = re.search(r"Initial upper bound:\s*([\d.]+)", line)
            if m:
                stats["init_ub"] = float(m.group(1))
        elif "Nodes created:" in line:
            m = re.search(r"Nodes created:\s*(\d+)", line)
            if m:
                stats["nodes_created"] = int(m.group(1))
        elif "Nodes expanded:" in line:
            m = re.search(r"Nodes expanded:\s*(\d+)", line)
            if m:
                stats["nodes_expanded"] = int(m.group(1))
        elif "Pruned by bound:" in line:
            m = re.search(r"Pruned by bound:\s*(\d+)", line)
            if m:
                stats["pruned_bound"] = int(m.group(1))
        elif "Pruned infeasible:" in line:
            m = re.search(r"Pruned infeasible:\s*(\d+)", line)
            if m:
                stats["pruned_infeasible"] = int(m.group(1))
        elif "Tour:" in line:
            m = re.search(r"Tour:\s*(.*)", line)
            if m:
                stats["tour"] = m.group(1).strip()

    return stats


def format_val(val, fmt="{}"):
    """Format a value for table display, showing '-' for None."""
    if val is None:
        return "-"
    return fmt.format(val)


def format_cost(val):
    if val is None:
        return "-"
    return f"{val:.0f}"


def format_time(seconds):
    if seconds is None:
        return "-"
    if seconds < 60:
        return f"{seconds:.1f}s"
    elif seconds < 3600:
        return f"{seconds / 60:.1f}m"
    else:
        return f"{seconds / 3600:.1f}h"


def run_strategy(instance_path: str, strategy: str) -> dict:
    """Run solver with a given branch strategy. Returns stats dict + elapsed + stderr."""
    cmd = [
        SOLVER,
        "--method", "exact",
        "--branch-strategy", strategy,
        "--exact-max-n", str(MAX_N),
        "--debug",
        "--debug-interval", str(DEBUG_INTERVAL),
        instance_path,
    ]
    t0 = time.perf_counter()
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        elapsed = time.perf_counter() - t0
        stats = parse_stats(result.stdout)
        stats["elapsed"] = elapsed
        stats["timeout"] = False
        stats["stderr"] = result.stderr
        stats["rc"] = result.returncode
    except subprocess.TimeoutExpired:
        elapsed = TIMEOUT
        stats = {
            "cost": None, "feasible": False, "root_lb": None, "init_ub": None,
            "nodes_created": None, "nodes_expanded": None,
            "pruned_bound": None, "pruned_infeasible": None,
            "tour": None, "elapsed": elapsed, "timeout": True, "stderr": "", "rc": -1,
        }
    except Exception as e:
        elapsed = time.perf_counter() - t0
        stats = {
            "cost": None, "feasible": False, "root_lb": None, "init_ub": None,
            "nodes_created": None, "nodes_expanded": None,
            "pruned_bound": None, "pruned_infeasible": None,
            "tour": None, "elapsed": elapsed, "timeout": False, "stderr": "", "rc": -1,
            "error": str(e),
        }

    return stats


def main():
    if not os.path.isfile(SOLVER):
        print(f"Error: solver not found at {SOLVER}", file=sys.stderr)
        sys.exit(1)

    instances = find_small_instances()
    print(f"Found {len(instances)} instances with < {MAX_N} vertices.\n", file=sys.stderr)

    output_lines = []
    output_lines.append("# TSP Branch Strategy Comparison — Smart vs Exhaustive vs Simple")
    output_lines.append("")
    output_lines.append(f"**Total instances:** {len(instances)}  ")
    output_lines.append(f"**Strategies:** Smart, Exhaustive, Simple  ")
    output_lines.append(f"**Max vertices:** < {MAX_N}  ")
    output_lines.append(f"**Debug interval:** {DEBUG_INTERVAL}  ")
    output_lines.append(f"**Timeout per run:** {TIMEOUT}s ({TIMEOUT // 3600}h)  ")
    output_lines.append("")
    output_lines.append("| Result | Count |")
    output_lines.append("|--------|-------|")
    output_lines.append(f"| Instances found | {len(instances)} |")
    output_lines.append("")
    output_lines.append("---")
    output_lines.append("")

    # Summary accumulators
    summary = {s: {"ok": 0, "timeout": 0, "error": 0, "total_time": 0.0, "total_nodes_expanded": 0, "count_nodes": 0}
               for s in STRATEGIES}

    for idx, (path, dim) in enumerate(instances, 1):
        rel_path = os.path.relpath(path, os.path.join(os.path.dirname(__file__), ".."))
        print(f"\n[{idx}/{len(instances)}] {rel_path} (n={dim})", file=sys.stderr)

        strategies_data = {}

        for strategy in STRATEGIES:
            label = STRATEGY_LABELS[strategy]
            print(f"  Running {label}...", file=sys.stderr, end=" ", flush=True)
            stats = run_strategy(path, strategy)
            strategies_data[strategy] = stats

            if stats.get("timeout"):
                print(f"TIMEOUT", file=sys.stderr)
                summary[strategy]["timeout"] += 1
            elif stats.get("error"):
                print(f"ERROR: {stats['error']}", file=sys.stderr)
                summary[strategy]["error"] += 1
            elif stats["feasible"]:
                print(f"cost={stats['cost']:.0f} time={format_time(stats['elapsed'])} expanded={stats['nodes_expanded']}", file=sys.stderr)
                summary[strategy]["ok"] += 1
                summary[strategy]["total_time"] += stats["elapsed"]
                if stats["nodes_expanded"] is not None:
                    summary[strategy]["total_nodes_expanded"] += stats["nodes_expanded"]
                    summary[strategy]["count_nodes"] += 1
            else:
                print(f"INFEASIBLE", file=sys.stderr)
                summary[strategy]["ok"] += 1
                summary[strategy]["total_time"] += stats["elapsed"]

        # --- Instance header ---
        output_lines.append(f"## {idx}. `{rel_path}` (n={dim})")
        output_lines.append("")

        # --- Comparison table ---
        output_lines.append("| Strategy | Cost | Time | Nodes Created | Nodes Expanded | Pruned (Bound) | Pruned (Infeasible) | Root LB | Init UB |")
        output_lines.append("|----------|------|------|---------------|----------------|----------------|---------------------|---------|---------|")

        # Determine consensus cost for highlighting
        costs = {}
        for s in STRATEGIES:
            st = strategies_data[s]
            if st["feasible"] and not st.get("timeout"):
                costs[s] = st["cost"]

        for strategy in STRATEGIES:
            tag = STRATEGY_LABELS[strategy]
            st = strategies_data[strategy]

            if st.get("timeout"):
                output_lines.append(f"| {tag} | TIMEOUT | {format_time(st['elapsed'])} | - | - | - | - | - | - |")
            elif st.get("error"):
                output_lines.append(f"| {tag} | ERROR | - | - | - | - | - | - | - |")
            else:
                cost_str = format_cost(st["cost"])
                time_str = format_time(st["elapsed"])
                created = format_val(st["nodes_created"])
                expanded = format_val(st["nodes_expanded"])
                pruned_b = format_val(st["pruned_bound"])
                pruned_i = format_val(st["pruned_infeasible"])
                root_lb = format_cost(st["root_lb"])
                init_ub = format_cost(st["init_ub"])
                output_lines.append(
                    f"| {tag} | {cost_str} | {time_str} | {created} | {expanded} | {pruned_b} | {pruned_i} | {root_lb} | {init_ub} |"
                )

        output_lines.append("")

        # --- Debug outputs ---
        for strategy in STRATEGIES:
            tag = STRATEGY_LABELS[strategy]
            st = strategies_data[strategy]
            debug_output = st.get("stderr", "")
            if debug_output.strip():
                output_lines.append(f"<details>")
                output_lines.append(f"<summary>{tag} debug output</summary>")
                output_lines.append("")
                output_lines.append("```")
                output_lines.append(debug_output.strip())
                output_lines.append("```")
                output_lines.append("</details>")
                output_lines.append("")

        output_lines.append("---")
        output_lines.append("")

    # --- Final summary ---
    output_lines.append("")
    output_lines.append("## Summary")
    output_lines.append("")
    output_lines.append("| Strategy | Solved | Timeout | Error | Avg Time (s) | Avg Nodes Expanded | Total Nodes Expanded |")
    output_lines.append("|----------|--------|---------|-------|--------------|-------------------|---------------------|")

    for strategy in STRATEGIES:
        tag = STRATEGY_LABELS[strategy]
        s = summary[strategy]
        ok = s["ok"]
        timeout = s["timeout"]
        error = s["error"]
        avg_time = s["total_time"] / ok if ok > 0 else 0
        avg_nodes = s["total_nodes_expanded"] / s["count_nodes"] if s["count_nodes"] > 0 else 0
        total_nodes = s["total_nodes_expanded"]
        output_lines.append(f"| {tag} | {ok} | {timeout} | {error} | {avg_time:.1f} | {avg_nodes:.0f} | {total_nodes} |")

    output_lines.append("")

    out_path = os.path.join(os.path.dirname(__file__), "..", "docs", "branch-strategy-comparison.md")
    with open(out_path, "w") as f:
        f.write("\n".join(output_lines))
    print(f"\n\nResults written to: {out_path}", file=sys.stderr)


if __name__ == "__main__":
    main()
