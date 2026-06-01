#!/usr/bin/env python3
"""
Compare 3 branch strategies (Smart, Exhaustive, Simple) on all TSP instances
under examples/. Verifies optimality via Held-Karp DP from verify_instances.py.

Output: docs/strategy-comparison-results.md
"""

import subprocess
import sys
import os
import re
import time
import math
from pathlib import Path

PROJECT_ROOT = os.path.join(os.path.dirname(__file__), "..")
SOLVER = os.path.join(PROJECT_ROOT, "build", "tsp_bb")
EXAMPLES_DIR = os.path.join(PROJECT_ROOT, "examples")
OUT_PATH = os.path.join(PROJECT_ROOT, "docs", "strategy-comparison-results.md")

TIMEOUT = 3600  # 1 hour per instance-strategy
DEBUG_INTERVAL = 10000
STRATEGIES = ["smart", "exhaustive", "simple"]
STRATEGY_LABELS = {"smart": "Smart", "exhaustive": "Exhaustive", "simple": "Simple"}

INF = float("inf")

# ── Held-Karp DP (adapted from verify_instances.py) ────────────────────

def held_karp(matrix: list[list[float]]) -> tuple[float, list[int]]:
    """Return (optimal_cost, optimal_tour). Returns (INF, []) if infeasible."""
    n = len(matrix)
    if n < 3:
        return INF, []

    dp: dict[tuple[int, int], tuple[float, int]] = {}
    for v in range(1, n):
        if math.isfinite(matrix[0][v]):
            dp[(1 << (v - 1), v)] = (matrix[0][v], -1)

    full_mask = (1 << (n - 1)) - 1
    for mask in range(1, full_mask + 1):
        for last in range(1, n):
            entry = dp.get((mask, last))
            if entry is None:
                continue
            current_cost, _ = entry

            remaining = full_mask ^ mask
            bitset = remaining
            while bitset:
                bit = bitset & -bitset
                nxt = bit.bit_length()
                if math.isfinite(matrix[last][nxt]):
                    next_mask = mask | bit
                    new_cost = current_cost + matrix[last][nxt]
                    prev = dp.get((next_mask, nxt))
                    if prev is None or new_cost < prev[0]:
                        dp[(next_mask, nxt)] = (new_cost, last)
                bitset -= bit

    best_cost = INF
    best_last = -1
    for last in range(1, n):
        entry = dp.get((full_mask, last))
        if entry is None:
            continue
        if not math.isfinite(matrix[last][0]):
            continue
        total = entry[0] + matrix[last][0]
        if total < best_cost:
            best_cost = total
            best_last = last

    if best_last < 0:
        return INF, []

    # Reconstruct tour
    tour = []
    mask = full_mask
    v = best_last
    while v != -1:
        tour.append(v)
        _, parent = dp[(mask, v)]
        mask ^= (1 << (v - 1))
        v = parent
    tour.append(0)
    tour.reverse()
    return best_cost, tour


# ── Instance Discovery ────────────────────────────────────────────────

def find_example_instances() -> list[tuple[str, int]]:
    """Find all TSP instances under examples/. Returns [(path, dimension), ...]."""
    instances = []
    for dirpath, dirnames, filenames in os.walk(EXAMPLES_DIR):
        dirnames[:] = [d for d in dirnames if not d.startswith(".")]
        for fname in sorted(filenames):
            if fname.startswith(".") or fname.endswith(".gz"):
                continue
            if fname == "batch.txt":
                continue
            filepath = os.path.join(dirpath, fname)
            dim = _parse_dimension(filepath)
            if dim is not None and dim >= 3:
                instances.append((filepath, dim))
    return instances


def _parse_dimension(filepath: str) -> int | None:
    """Parse DIMENSION from TSPLIB header or matrix format (first token)."""
    with open(filepath, "r") as f:
        first_line = f.readline().strip()
        tokens = first_line.split()
        if tokens and tokens[0].isdigit():
            n = int(tokens[0])
            if n >= 3:
                return n

    with open(filepath, "r") as f:
        for line in f:
            upper = line.strip().upper()
            if upper.startswith("DIMENSION"):
                parts = line.split(":")
                if len(parts) == 2:
                    return int(parts[1].strip())
                parts = line.split()
                if len(parts) >= 2:
                    return int(parts[1])
    return None


# ── Distance Matrix Loading ───────────────────────────────────────────

def load_distance_matrix(filepath: str) -> tuple[list[list[float]], int]:
    """Load distance matrix from file (matrix or TSPLIB). Returns (matrix, n)."""
    with open(filepath, "r") as f:
        content = f.read()

    if "NODE_COORD_SECTION" in content.upper():
        return _load_tsplib_coord(filepath)
    if "EDGE_WEIGHT_SECTION" in content.upper():
        return _load_tsplib_explicit(filepath)

    # Plain matrix: first token = n, then n*n values
    tokens = content.split()
    n = int(tokens[0])
    matrix = [[0.0] * n for _ in range(n)]
    idx = 1
    for i in range(n):
        for j in range(n):
            matrix[i][j] = _parse_weight(tokens[idx])
            idx += 1
    return matrix, n


def _parse_weight(token: str) -> float:
    lower = token.lower()
    if lower in ("x", "-", "inf", "infinity"):
        return INF
    return float(token)


def _load_tsplib_coord(filepath: str) -> tuple[list[list[float]], int]:
    coords = []
    edge_weight_type = "EUC_2D"
    dimension = 0
    section = None

    with open(filepath, "r") as f:
        for line in f:
            line = line.strip()
            upper = line.upper()
            if upper == "NODE_COORD_SECTION":
                section = "coord"
                continue
            if upper == "EOF":
                break
            if section is None:
                if upper.startswith("DIMENSION"):
                    parts = line.split(":")
                    dimension = int(parts[1].strip()) if len(parts) == 2 else int(line.split()[1])
                elif upper.startswith("EDGE_WEIGHT_TYPE"):
                    parts = line.split(":")
                    edge_weight_type = parts[1].strip().upper() if len(parts) == 2 else line.split()[1].upper()
            elif section == "coord":
                parts = line.split()
                if len(parts) >= 3:
                    coords.append((float(parts[1]), float(parts[2])))

    n = dimension if dimension > 0 else len(coords)
    if edge_weight_type == "GEO":
        return _geo_matrix(coords, n), n
    return _euc2d_matrix(coords, n), n


def _euc2d_matrix(coords: list, n: int) -> list[list[float]]:
    matrix = [[0.0] * n for _ in range(n)]
    for i in range(n):
        for j in range(n):
            if i == j:
                continue
            dx = coords[i][0] - coords[j][0]
            dy = coords[i][1] - coords[j][1]
            matrix[i][j] = round(math.sqrt(dx * dx + dy * dy))
    return matrix


def _geo_matrix(coords: list, n: int) -> list[list[float]]:
    PI = math.pi
    matrix = [[0.0] * n for _ in range(n)]

    def to_rad(deg_val):
        d = int(deg_val)
        m = deg_val - d
        return PI * (d + 5.0 * m / 3.0) / 180.0

    for i in range(n):
        for j in range(n):
            if i == j:
                continue
            lat_a = to_rad(coords[i][0])
            lon_a = to_rad(coords[i][1])
            lat_b = to_rad(coords[j][0])
            lon_b = to_rad(coords[j][1])
            q1 = math.cos(lon_a - lon_b)
            q2 = math.cos(lat_a - lat_b)
            q3 = math.cos(lat_a + lat_b)
            arg = 0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3)
            arg = max(-1.0, min(1.0, arg))
            matrix[i][j] = int(6378.388 * math.acos(arg) + 1.0)
    return matrix


def _load_tsplib_explicit(filepath: str) -> tuple[list[list[float]], int]:
    dimension = 0
    edge_weight_format = "FULL_MATRIX"
    section = None
    values = []

    with open(filepath, "r") as f:
        for line in f:
            line = line.strip()
            upper = line.upper()
            if upper == "EDGE_WEIGHT_SECTION":
                section = "weights"
                continue
            if upper == "EOF":
                break
            if section is None:
                if upper.startswith("DIMENSION"):
                    parts = line.split(":")
                    dimension = int(parts[1].strip()) if len(parts) == 2 else int(line.split()[1])
                elif upper.startswith("EDGE_WEIGHT_FORMAT"):
                    parts = line.split(":")
                    edge_weight_format = parts[1].strip().upper() if len(parts) == 2 else line.split()[1].upper()
            elif section == "weights":
                for token in line.split():
                    values.append(_parse_weight(token))

    matrix = [[0.0] * dimension for _ in range(dimension)]
    fmt = edge_weight_format
    idx = 0

    if fmt == "FULL_MATRIX":
        for i in range(dimension):
            for j in range(dimension):
                matrix[i][j] = values[idx]
                idx += 1
    elif fmt == "UPPER_ROW":
        for i in range(dimension):
            for j in range(i + 1, dimension):
                matrix[i][j] = matrix[j][i] = values[idx]
                idx += 1
    elif fmt == "LOWER_ROW":
        for i in range(dimension):
            for j in range(i):
                matrix[i][j] = matrix[j][i] = values[idx]
                idx += 1
    elif fmt == "UPPER_DIAG_ROW":
        for i in range(dimension):
            for j in range(i, dimension):
                matrix[i][j] = matrix[j][i] = values[idx]
                idx += 1
    elif fmt == "LOWER_DIAG_ROW":
        for i in range(dimension):
            for j in range(i + 1):
                matrix[i][j] = matrix[j][i] = values[idx]
                idx += 1
    else:
        for i in range(dimension):
            for j in range(dimension):
                if idx < len(values):
                    matrix[i][j] = values[idx]
                    idx += 1
    return matrix, dimension


# ── Solver Wrapper ────────────────────────────────────────────────────

def parse_solver_stats(stdout: str) -> dict:
    """Extract solver statistics from human-readable output."""
    stats = {
        "cost": None, "feasible": False, "root_lb": None, "init_ub": None,
        "nodes_created": None, "nodes_expanded": None,
        "pruned_bound": None, "pruned_infeasible": None,
        "tour": None, "tour_list": None,
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
                tour_str = m.group(1).strip()
                stats["tour"] = tour_str
                parts = tour_str.replace("->", " ").split()
                try:
                    stats["tour_list"] = [int(x) for x in parts if x.lstrip("-").isdigit()]
                except ValueError:
                    pass
    return stats


def run_strategy(instance_path: str, strategy: str) -> dict:
    """Run solver with a branch strategy. Returns stats dict with elapsed time."""
    cmd = [
        SOLVER,
        "--method", "exact",
        "--branch-strategy", strategy,
        "--exact-max-n", "30",
        "--debug",
        "--debug-interval", str(DEBUG_INTERVAL),
        instance_path,
    ]
    t0 = time.perf_counter()
    try:
        result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
        elapsed = time.perf_counter() - t0
        stats = parse_solver_stats(result.stdout)
        stats["elapsed"] = elapsed
        stats["timeout"] = False
        stats["error"] = None
        stats["stderr"] = result.stderr
    except subprocess.TimeoutExpired:
        stats = _empty_stats()
        stats["elapsed"] = TIMEOUT
        stats["timeout"] = True
    except Exception as e:
        stats = _empty_stats()
        stats["elapsed"] = time.perf_counter() - t0
        stats["error"] = str(e)
    return stats


def _empty_stats() -> dict:
    return {
        "cost": None, "feasible": False, "root_lb": None, "init_ub": None,
        "nodes_created": None, "nodes_expanded": None,
        "pruned_bound": None, "pruned_infeasible": None,
        "tour": None, "tour_list": None,
        "elapsed": 0.0, "timeout": False, "error": None, "stderr": "",
    }


# ── Formatting ────────────────────────────────────────────────────────

def fmt_cost(val) -> str:
    if val is None or val >= INF / 2:
        return "-"
    return f"{val:.0f}"


def fmt_time(sec: float | None) -> str:
    if sec is None:
        return "-"
    if sec < 1:
        return f"{sec * 1000:.0f}ms"
    if sec < 60:
        return f"{sec:.1f}s"
    if sec >= 3600:
        return f"{sec / 3600:.1f}h"
    return f"{sec / 60:.1f}m"


def fmt_val(val, template="{}") -> str:
    if val is None:
        return "-"
    return template.format(val)


# ── Main ──────────────────────────────────────────────────────────────

def main():
    if not os.path.isfile(SOLVER):
        print(f"Error: solver not found at {SOLVER}", file=sys.stderr)
        print("Build first: cd build && cmake .. && make", file=sys.stderr)
        sys.exit(1)

    instances = find_example_instances()
    if not instances:
        print("Error: no instances found under examples/", file=sys.stderr)
        sys.exit(1)

    print(f"Found {len(instances)} instances in examples/\n", file=sys.stderr)

    lines = []
    lines.append("# TSP Branch Strategy Comparison — Smart vs Exhaustive vs Simple")
    lines.append("")
    lines.append(f"**Instances:** {len(instances)} (from `examples/` only)  ")
    lines.append(f"**Verification:** Held-Karp Dynamic Programming  ")
    lines.append(f"**Timeout:** {TIMEOUT}s ({TIMEOUT // 3600}h) per strategy per instance  ")
    lines.append("")
    lines.append("---")
    lines.append("")

    os.makedirs(os.path.dirname(OUT_PATH), exist_ok=True)

    def flush():
        with open(OUT_PATH, "w") as f:
            f.write("\n".join(lines))

    flush()

    summary = {s: {"ok": 0, "timeout": 0, "error": 0, "wrong_cost": 0,
                   "total_time": 0.0, "total_nodes_expanded": 0, "count_nodes": 0,
                   "total_nodes_created": 0}
               for s in STRATEGIES}

    for idx, (path, dim) in enumerate(instances, 1):
        rel_path = os.path.relpath(path, PROJECT_ROOT)
        print(f"[{idx}/{len(instances)}] {rel_path} (n={dim})", file=sys.stderr)

        # ── Load matrix + DP ground truth ──
        try:
            matrix, n_loaded = load_distance_matrix(path)
        except Exception as e:
            print(f"  SKIP: failed to load — {e}", file=sys.stderr)
            lines.append(f"## {idx}. `{rel_path}` (n={dim})")
            lines.append("")
            lines.append(f"**Status:** SKIP — failed to load: {e}")
            lines.append("")
            lines.append("---")
            lines.append("")
            flush()
            continue

        print(f"  Held-Karp DP...", file=sys.stderr, end=" ", flush=True)
        dp_t0 = time.perf_counter()
        dp_cost, dp_tour = held_karp(matrix)
        dp_elapsed = time.perf_counter() - dp_t0
        if dp_cost >= INF / 2:
            print(f"INFEASIBLE", file=sys.stderr)
            dp_cost_str = "infeasible"
        else:
            print(f"optimal={dp_cost:.0f} time={fmt_time(dp_elapsed)}", file=sys.stderr)
            dp_cost_str = fmt_cost(dp_cost)

        # ── Run all 3 strategies ──
        strategies_data = {}
        for strategy in STRATEGIES:
            label = STRATEGY_LABELS[strategy]
            print(f"  {label}...", file=sys.stderr, end=" ", flush=True)
            stats = run_strategy(path, strategy)
            strategies_data[strategy] = stats

            if stats["timeout"]:
                print(f"TIMEOUT", file=sys.stderr)
                summary[strategy]["timeout"] += 1
            elif stats["error"]:
                print(f"ERROR: {stats['error']}", file=sys.stderr)
                summary[strategy]["error"] += 1
            elif stats["feasible"]:
                correct = True
                if dp_cost < INF / 2 and abs(stats["cost"] - dp_cost) > 1e-6:
                    correct = False
                    summary[strategy]["wrong_cost"] += 1
                    print(f"cost={stats['cost']:.0f} WRONG (dp={dp_cost:.0f})", file=sys.stderr)
                else:
                    tag = "OK" if dp_cost >= INF / 2 else f"OK (=DP)"
                    print(f"cost={stats['cost']:.0f} {tag} time={fmt_time(stats['elapsed'])} expanded={stats['nodes_expanded']}", file=sys.stderr)
                summary[strategy]["ok"] += 1
                summary[strategy]["total_time"] += stats["elapsed"]
                if stats["nodes_expanded"] is not None:
                    summary[strategy]["total_nodes_expanded"] += stats["nodes_expanded"]
                    summary[strategy]["count_nodes"] += 1
                if stats["nodes_created"] is not None:
                    summary[strategy]["total_nodes_created"] += stats["nodes_created"]
            else:
                print(f"INFEASIBLE", file=sys.stderr)
                summary[strategy]["ok"] += 1
                summary[strategy]["total_time"] += stats["elapsed"]

        # ── Instance output ──
        lines.append(f"## {idx}. `{rel_path}` (n={dim})")
        lines.append("")
        lines.append(f"**DP optimal cost:** {dp_cost_str}  ")
        lines.append(f"**DP time:** {fmt_time(dp_elapsed)}  ")
        lines.append("")

        if dp_tour:
            tour_str = " -> ".join(str(v) for v in dp_tour) + f" -> {dp_tour[0]}"
            lines.append(f"**DP optimal tour:** `{tour_str}`  ")
            lines.append("")

        # Comparison table
        lines.append("| Strategy | Cost | Match | Time | Created | Expanded | Pruned(Bound) | Pruned(Infeas) | Root LB | Init UB |")
        lines.append("|----------|------|-------|------|---------|----------|---------------|----------------|---------|---------|")

        for strategy in STRATEGIES:
            tag = STRATEGY_LABELS[strategy]
            st = strategies_data[strategy]

            if st["timeout"]:
                lines.append(f"| {tag} | TIMEOUT | - | {fmt_time(st['elapsed'])} | - | - | - | - | - | - |")
            elif st["error"]:
                lines.append(f"| {tag} | ERROR | - | - | - | - | - | - | - | - |")
            else:
                cost_s = fmt_cost(st["cost"])
                if dp_cost < INF / 2 and st["feasible"]:
                    match = ":white_check_mark:" if abs(st["cost"] - dp_cost) < 1e-6 else ":x:"
                elif not st["feasible"] and dp_cost >= INF / 2:
                    match = ":white_check_mark:"
                elif not st["feasible"]:
                    match = ":x:"
                else:
                    match = "-"
                lines.append(
                    f"| {tag} | {cost_s} | {match} | {fmt_time(st['elapsed'])} "
                    f"| {fmt_val(st['nodes_created'])} | {fmt_val(st['nodes_expanded'])} "
                    f"| {fmt_val(st['pruned_bound'])} | {fmt_val(st['pruned_infeasible'])} "
                    f"| {fmt_cost(st['root_lb'])} | {fmt_cost(st['init_ub'])} |"
                )
        lines.append("")

        # Per-strategy tours
        tour_lines = []
        for strategy in STRATEGIES:
            tag = STRATEGY_LABELS[strategy]
            st = strategies_data[strategy]
            if st.get("tour_list") and len(st["tour_list"]) > 1:
                unique_tour = list(dict.fromkeys(st["tour_list"]))
                cost_info = ""
                if dp_cost < INF / 2 and st["feasible"]:
                    if abs(st["cost"] - dp_cost) < 1e-6:
                        cost_info = " (=DP)"
                    else:
                        cost_info = " (DIFFERS from DP)"
                tour_lines.append(
                    f"- **{tag}:** `{' -> '.join(str(v) for v in unique_tour)}` "
                    f"cost={fmt_cost(st['cost'])}{cost_info}"
                )

        if tour_lines:
            lines.append("**Tours found:**")
            lines.extend(tour_lines)
            lines.append("")

        # Debug output (collapsible)
        for strategy in STRATEGIES:
            tag = STRATEGY_LABELS[strategy]
            st = strategies_data[strategy]
            debug_text = st.get("stderr", "")
            if debug_text.strip():
                lines.append(f"<details>")
                lines.append(f"<summary>{tag} debug output</summary>")
                lines.append("")
                lines.append("```")
                lines.append(debug_text.strip())
                lines.append("```")
                lines.append("</details>")
                lines.append("")

        lines.append("---")
        lines.append("")
        flush()

    # ── Summary ──
    lines.append("")
    lines.append("## Summary")
    lines.append("")
    lines.append("| Strategy | Solved | Timeout | Error | Wrong Cost | Total Time | Avg Time | Avg Expanded | Total Expanded | Total Created |")
    lines.append("|----------|--------|---------|-------|------------|------------|----------|--------------|----------------|---------------|")

    for strategy in STRATEGIES:
        tag = STRATEGY_LABELS[strategy]
        s = summary[strategy]
        ok = s["ok"]
        timeout = s["timeout"]
        error = s["error"]
        wrong = s["wrong_cost"]
        total_time = s["total_time"]
        avg_time = total_time / ok if ok > 0 else 0
        avg_nodes = s["total_nodes_expanded"] / s["count_nodes"] if s["count_nodes"] > 0 else 0
        lines.append(
            f"| {tag} | {ok} | {timeout} | {error} | {wrong} | {fmt_time(total_time)} "
            f"| {fmt_time(avg_time)} | {avg_nodes:.0f} | {s['total_nodes_expanded']} | {s['total_nodes_created']} |"
        )
    lines.append("")
    flush()

    print(f"\nResults written to: {OUT_PATH}", file=sys.stderr)


if __name__ == "__main__":
    main()
