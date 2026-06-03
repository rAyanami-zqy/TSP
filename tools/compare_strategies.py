#!/usr/bin/env python3
"""
Unified TSP comparison script.

Algorithms compared:
  1. Held-Karp Dynamic Programming (ground truth, exact)
  2. tsp_bb Smart branch strategy  (exact B&B)
  3. tsp_bb Simple branch strategy (exact B&B)
  4. Concorde                       (exact, state-of-the-art)

Output: docs/strategy-comparison-results.md

Usage:
  python3 tools/compare_strategies.py

To change which instances are tested, edit the INSTANCE_SOURCES block below.
"""

import subprocess
import sys
import os
import re
import time
import math
import shutil
import tempfile
from pathlib import Path

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
SOLVER = os.path.join(PROJECT_ROOT, "build", "tsp_bb")
CONCORDE = os.path.join(PROJECT_ROOT, "concorde", "TSP", "concorde")
CONVERTER = os.path.join(PROJECT_ROOT, "tools", "convert_to_tsplib.py")
OUT_PATH = os.path.join(PROJECT_ROOT, "docs", "strategy-comparison-results.md")

# ── Global settings ──────────────────────────────────────────────────

TIMEOUT = 3600  # seconds per instance per algorithm
DEBUG_INTERVAL = 100000  # how often tsp_bb prints debug progress
CONCORDE_SEED = 123  # fixed seed for reproducibility

# ============================================================
# Instance Sources — edit this block to control which instances
# are included in the comparison. Each entry is a dict:
#
#   path      : directory relative to PROJECT_ROOT (required)
#   recursive : walk subdirectories? True/False (default True)
#   max_dim   : only include instances with n <= max_dim
#               set to None for no size limit
#   skip_dirs : subdirectories to skip when recursive
#               (default: ["_archives", "tsplib"])
#   extensions: file extensions to scan (default: [".txt", ".tsp"])
#
# Examples:
#   {"path": "data/classic/tsplib",  "max_dim": 29}    # TSPLIB, n<30
#   {"path": "data/classic/national","max_dim": 29}    # National, n<30
#   {"path": "data/classic",         "max_dim": 50}    # All classic, n<=50
#   {"path": "examples",             "max_dim": None}   # All examples
#   {"path": "data/classic/vlsi",    "max_dim": 500}   # Small VLSI only
# ============================================================
INSTANCE_SOURCES = [
    # Small project examples (hand-crafted)
    {"path": "examples", "recursive": True, "max_dim": 30,
     "skip_dirs": ["tsplib"]},

    # Classic TSPLIB benchmark — instances with fewer than 30 vertices
    #{"path": "data/classic/tsplib", "recursive": False, "max_dim": 29},
    # result: burma14(14) ulysses16(16) gr17(17) gr21(21) ulysses22(22)
    #         gr24(24) fri26(26) bayg29(29) bays29(29)

    # Classic National benchmark — instances with fewer than 30 vertices
    #{"path": "data/classic/national", "recursive": False, "max_dim": 29},
    # result: wi29(29)
]
# ============================================================

# Algorithm IDs
ALG_DP       = "DP"          # Held-Karp DP (ground truth)
ALG_SMART    = "Smart"       # tsp_bb exact + smart branch
ALG_SIMPLE   = "Simple"      # tsp_bb exact + simple branch
ALG_CONCORDE = "Concorde"    # Concorde exact solver

ALL_ALGORITHMS = [ALG_DP, ALG_SMART, ALG_SIMPLE, ALG_CONCORDE]

INF = float("inf")

DEFAULT_SKIP_DIRS = {"_archives", "tsplib"}
DEFAULT_EXTENSIONS = (".txt", ".tsp")


# ── Held-Karp DP (ground truth) ──────────────────────────────────────

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

def _parse_dimension(filepath: str) -> int | None:
    """Parse DIMENSION from TSPLIB header or first-line-number format."""
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
                line = line.replace(" ", "")  # handle "DIMENSION : 29"
                parts = line.split(":")
                if len(parts) == 2:
                    return int(parts[1].strip())
                parts = line.split()
                if len(parts) >= 2:
                    return int(parts[1])
    return None


def find_instances() -> list[tuple[str, int]]:
    """Discover instances from all configured INSTANCE_SOURCES.

    Returns a list of (absolute_path, dimension) tuples sorted by dimension
    then name.
    """
    instances = []
    seen = set()  # deduplicate by basename

    for cfg in INSTANCE_SOURCES:
        base_path = os.path.join(PROJECT_ROOT, cfg["path"])
        if not os.path.isdir(base_path):
            print(f"Warning: source not found, skipping: {base_path}", file=sys.stderr)
            continue

        recursive = cfg.get("recursive", True)
        max_dim = cfg.get("max_dim", None)
        skip_dirs = set(cfg.get("skip_dirs", DEFAULT_SKIP_DIRS))
        extensions = tuple(cfg.get("extensions", DEFAULT_EXTENSIONS))

        if recursive:
            walker = os.walk(base_path)
        else:
            walker = [(base_path, [], sorted(os.listdir(base_path)))]

        for dirpath, dirnames, filenames in walker:
            if recursive:
                dirnames[:] = [
                    d for d in dirnames
                    if not d.startswith(".") and d not in skip_dirs
                ]

            for fname in sorted(filenames):
                if fname.startswith("."):
                    continue
                if fname.endswith(".gz"):
                    continue
                if fname == "batch.txt":
                    continue
                if not fname.endswith(extensions):
                    continue

                filepath = os.path.join(dirpath, fname)
                dim = _parse_dimension(filepath)

                if dim is None or dim < 3:
                    continue
                if max_dim is not None and dim > max_dim:
                    continue

                # Deduplicate by filename
                if fname in seen:
                    continue
                seen.add(fname)

                instances.append((filepath, dim))

    # Sort by dimension, then name
    instances.sort(key=lambda x: (x[1], os.path.basename(x[0])))
    return instances


# ── Distance Matrix Loading ───────────────────────────────────────────

def load_distance_matrix(filepath: str) -> tuple[list[list[float]], int]:
    """Load distance matrix from file (raw matrix, TSPLIB coord, or TSPLIB explicit)."""
    with open(filepath, "r") as f:
        content = f.read()

    if "NODE_COORD_SECTION" in content.upper():
        return _load_tsplib_coord(filepath)
    if "EDGE_WEIGHT_SECTION" in content.upper():
        return _load_tsplib_explicit(filepath)

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
                    line = line.replace(" ", "")
                    parts = line.split(":")
                    dimension = int(parts[1].strip()) if len(parts) == 2 else int(line.split()[1])
                elif upper.startswith("EDGE_WEIGHT_TYPE"):
                    line = line.replace(" ", "")
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
                    line = line.replace(" ", "")
                    parts = line.split(":")
                    dimension = int(parts[1].strip()) if len(parts) == 2 else int(line.split()[1])
                elif upper.startswith("EDGE_WEIGHT_FORMAT"):
                    line = line.replace(" ", "")
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


# ── Format Detection ─────────────────────────────────────────────────

def is_tsplib_format(filepath: str) -> bool:
    """Check if a file is in TSPLIB format (has section markers or header keywords)."""
    with open(filepath, "r") as f:
        content = f.read(4096)
    upper = content.upper()
    for marker in ("EDGE_WEIGHT_SECTION", "NODE_COORD_SECTION",
                    "EDGE_WEIGHT_TYPE", "TYPE:", "NAME:"):
        if marker in upper:
            return True
    return False


# ── Concorde Wrapper ─────────────────────────────────────────────────

def ensure_tsplib_for_concorde(instance_path: str, tmpdir: str) -> str:
    """Convert instance to TSPLIB format if needed. Returns path Concorde can read."""
    if is_tsplib_format(instance_path):
        return instance_path

    basename = os.path.splitext(os.path.basename(instance_path))[0]
    out_path = os.path.join(tmpdir, basename + ".tsp")

    result = subprocess.run(
        [sys.executable, CONVERTER, instance_path, "-o", out_path],
        capture_output=True, text=True, timeout=10
    )
    if result.returncode != 0:
        raise RuntimeError(f"TSPLIB conversion failed: {result.stderr}")

    return out_path


def parse_concorde_output(stdout: str) -> dict:
    """Extract Concorde solving statistics."""
    stats = {
        "cost": None, "feasible": False, "time": None,
        "bbnodes": None, "status": "unknown",
    }
    for line in stdout.splitlines():
        if "Optimal Solution:" in line:
            m = re.search(r"Optimal Solution:\s*([\d.]+)", line)
            if m:
                stats["cost"] = float(m.group(1))
                stats["feasible"] = True
                stats["status"] = "optimal"
        elif "Total Running Time:" in line:
            m = re.search(r"Total Running Time:\s*([\d.]+)", line)
            if m:
                stats["time"] = float(m.group(1))
        elif "Number of bbnodes:" in line:
            m = re.search(r"Number of bbnodes:\s*(\d+)", line)
            if m:
                stats["bbnodes"] = int(m.group(1))
        elif "ERROR:" in line:
            stats["status"] = "error"
            stats["error_msg"] = line.strip()
    return stats


def run_concorde(instance_path: str, seed: int = 123) -> dict:
    """Run Concorde on an instance. Returns stats dict with elapsed time."""
    tmpdir = tempfile.mkdtemp(prefix="concorde_tmp_")
    try:
        tsplib_path = ensure_tsplib_for_concorde(instance_path, tmpdir)

        cmd = [
            CONCORDE,
            "-s", str(seed),
            tsplib_path,
        ]
        t0 = time.perf_counter()
        try:
            result = subprocess.run(cmd, capture_output=True, text=True, timeout=TIMEOUT)
            elapsed = time.perf_counter() - t0
            stats = parse_concorde_output(result.stdout)
            stats["elapsed"] = elapsed
            stats["timeout"] = False
            stats["error"] = None
            stats["stderr"] = result.stderr
        except subprocess.TimeoutExpired:
            stats = _empty_stats_concorde()
            stats["elapsed"] = TIMEOUT
            stats["timeout"] = True
        except Exception as e:
            stats = _empty_stats_concorde()
            stats["elapsed"] = time.perf_counter() - t0
            stats["error"] = str(e)
        return stats
    finally:
        shutil.rmtree(tmpdir, ignore_errors=True)


def _empty_stats_concorde() -> dict:
    return {
        "cost": None, "feasible": False, "time": None, "bbnodes": None,
        "status": "unknown", "elapsed": 0.0, "timeout": False,
        "error": None, "stderr": "",
    }


# ── tsp_bb Solver Wrapper ────────────────────────────────────────────

def parse_tspbb_stats(stdout: str) -> dict:
    """Extract solver statistics from tsp_bb human-readable output."""
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


def run_tspbb(instance_path: str, strategy: str) -> dict:
    """Run tsp_bb solver with a branch strategy. Returns stats dict with elapsed time."""
    cmd = [
        SOLVER,
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
        stats = parse_tspbb_stats(result.stdout)
        stats["elapsed"] = elapsed
        stats["timeout"] = False
        stats["error"] = None
        stats["stderr"] = result.stderr
    except subprocess.TimeoutExpired:
        stats = _empty_stats_tspbb()
        stats["elapsed"] = TIMEOUT
        stats["timeout"] = True
    except Exception as e:
        stats = _empty_stats_tspbb()
        stats["elapsed"] = time.perf_counter() - t0
        stats["error"] = str(e)
    return stats


def _empty_stats_tspbb() -> dict:
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
    if sec < 0.001:
        return "<1ms"
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


def cost_match(alg_cost: float | None, dp_cost: float) -> str:
    """Return emoji match indicator."""
    if alg_cost is None:
        return "-"
    if dp_cost >= INF / 2:
        return "-"
    if abs(alg_cost - dp_cost) < 1e-6:
        return ":white_check_mark:"
    return ":x:"


def _build_source_description() -> str:
    """Build a human-readable description of the configured instance sources."""
    parts = []
    for cfg in INSTANCE_SOURCES:
        path = cfg["path"]
        max_dim = cfg.get("max_dim")
        dim_str = f"n <= {max_dim}" if max_dim is not None else "all sizes"
        parts.append(f"`{path}` ({dim_str})")
    return ", ".join(parts)


# ── Main ──────────────────────────────────────────────────────────────

def main():
    # Validate prerequisites
    errors = []
    if not os.path.isfile(SOLVER):
        errors.append(f"tsp_bb solver not found: {SOLVER} (run: cd build && cmake .. && make)")
    if not os.path.isfile(CONCORDE):
        errors.append(f"Concorde not found: {CONCORDE}")
    if errors:
        for e in errors:
            print(f"Error: {e}", file=sys.stderr)
        sys.exit(1)

    instances = find_instances()
    if not instances:
        print("Error: no instances found matching the configured sources", file=sys.stderr)
        print("Check INSTANCE_SOURCES in the script header.", file=sys.stderr)
        sys.exit(1)

    source_desc = _build_source_description()
    print(f"Found {len(instances)} instances from {len(INSTANCE_SOURCES)} source(s):", file=sys.stderr)
    for p, d in instances:
        print(f"  n={d:3d}  {os.path.relpath(p, PROJECT_ROOT)}", file=sys.stderr)
    print(file=sys.stderr)

    lines = []
    lines.append("# TSP Algorithm Comparison")
    lines.append("")
    lines.append("## Algorithms Compared")
    lines.append("")
    lines.append("| Algorithm | Description |")
    lines.append("|---|---|")
    lines.append("| **DP** | Held-Karp Dynamic Programming (exact, ground truth) |")
    lines.append("| **Smart** | Branch & Bound with smart branching (1-tree degree + edge candidate) |")
    lines.append("| **Simple** | Branch & Bound with simple branching (max-degree vertex) |")
    lines.append("| **Concorde** | State-of-the-art Concorde TSP solver (exact, with QSopt LP) |")
    lines.append("")
    lines.append(f"**Instances:** {len(instances)} from {source_desc}  ")
    lines.append(f"**Timeout:** {TIMEOUT}s ({TIMEOUT // 3600}h) per method per instance  ")
    lines.append(f"**Ground truth:** Held-Karp DP  ")
    lines.append("")
    lines.append("---")
    lines.append("")

    os.makedirs(os.path.dirname(OUT_PATH), exist_ok=True)

    def flush():
        with open(OUT_PATH, "w") as f:
            f.write("\n".join(lines))

    flush()

    summary = {
        alg: {"ok": 0, "timeout": 0, "error": 0, "wrong_cost": 0,
              "total_time": 0.0, "total_bbnodes": 0, "count_nodes": 0}
        for alg in [ALG_SMART, ALG_SIMPLE, ALG_CONCORDE]
    }

    per_instance = []

    for idx, (path, dim) in enumerate(instances, 1):
        rel_path = os.path.relpath(path, PROJECT_ROOT)
        print(f"\n[{idx}/{len(instances)}] {rel_path} (n={dim})", file=sys.stderr)

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

        print(f"  DP...", file=sys.stderr, end=" ", flush=True)
        dp_t0 = time.perf_counter()
        dp_cost, dp_tour = held_karp(matrix)
        dp_elapsed = time.perf_counter() - dp_t0
        dp_feasible = dp_cost < INF / 2
        if dp_feasible:
            print(f"optimal={dp_cost:.0f} time={fmt_time(dp_elapsed)}", file=sys.stderr)
        else:
            print(f"INFEASIBLE", file=sys.stderr)

        # ── Run all algorithms ──
        results = {}

        results[ALG_DP] = {
            "cost": dp_cost if dp_feasible else None,
            "feasible": dp_feasible,
            "elapsed": dp_elapsed,
            "bbnodes": None,
            "timeout": False,
            "error": None,
            "stderr": "",
            "root_lb": None, "init_ub": None,
            "nodes_created": None, "nodes_expanded": None,
            "pruned_bound": None, "pruned_infeasible": None,
            "tour": None, "tour_list": dp_tour if dp_feasible else None,
        }

        # Smart B&B
        print(f"  Smart...", file=sys.stderr, end=" ", flush=True)
        st = run_tspbb(path, "smart")
        results[ALG_SMART] = st
        _print_solver_status(ALG_SMART, st, dp_cost)

        # Simple B&B
        print(f"  Simple...", file=sys.stderr, end=" ", flush=True)
        st = run_tspbb(path, "simple")
        results[ALG_SIMPLE] = st
        _print_solver_status(ALG_SIMPLE, st, dp_cost)

        # Concorde
        print(f"  Concorde...", file=sys.stderr, end=" ", flush=True)
        st = run_concorde(path, CONCORDE_SEED)
        results[ALG_CONCORDE] = st
        if st["timeout"]:
            print(f"TIMEOUT", file=sys.stderr)
        elif st["error"]:
            print(f"ERROR: {st['error']}", file=sys.stderr)
        else:
            c_cost = st.get("cost")
            c_time = st.get("elapsed", 0)
            c_bb = st.get("bbnodes")
            if c_cost is not None:
                match = "OK" if abs(c_cost - dp_cost) < 1e-6 else "WRONG"
                print(f"cost={c_cost:.0f} {match} time={fmt_time(c_time)} bbnodes={c_bb}", file=sys.stderr)
            else:
                print(f"no solution time={fmt_time(c_time)}", file=sys.stderr)

        # ── Update summary ──
        for alg in [ALG_SMART, ALG_SIMPLE, ALG_CONCORDE]:
            st = results[alg]
            if st["timeout"]:
                summary[alg]["timeout"] += 1
            elif st["error"]:
                summary[alg]["error"] += 1
            elif st.get("feasible"):
                if dp_feasible and abs(st["cost"] - dp_cost) > 1e-6:
                    summary[alg]["wrong_cost"] += 1
                else:
                    summary[alg]["ok"] += 1
                summary[alg]["total_time"] += st["elapsed"]
                bbnodes = None
                if alg == ALG_CONCORDE:
                    bbnodes = st.get("bbnodes")
                else:
                    bbnodes = st.get("nodes_created")
                if bbnodes is not None:
                    summary[alg]["total_bbnodes"] += bbnodes
                    summary[alg]["count_nodes"] += 1
            else:
                summary[alg]["ok"] += 1
                summary[alg]["total_time"] += st["elapsed"]

        # ── Instance output ──
        lines.append(f"## {idx}. `{rel_path}` (n={dim})")
        lines.append("")
        lines.append(f"**DP optimal cost:** {fmt_cost(dp_cost) if dp_feasible else 'infeasible'}  ")
        lines.append(f"**DP time:** {fmt_time(dp_elapsed)}  ")
        lines.append("")

        if dp_tour:
            tour_str = " -> ".join(str(v) for v in dp_tour) + f" -> {dp_tour[0]}"
            lines.append(f"**DP optimal tour:** `{tour_str}`  ")
            lines.append("")

        # Results table
        lines.append("| Algorithm | Cost | Match DP | Time | Nodes Created | Nodes Expanded | Pruned(Bound) | Pruned(Infeas) | BBNodes |")
        lines.append("|-----------|------|----------|------|---------------|----------------|---------------|----------------|---------|")

        for alg in ALL_ALGORITHMS:
            st = results[alg]
            label = alg

            if st["timeout"]:
                lines.append(f"| {label} | TIMEOUT | - | {fmt_time(st['elapsed'])} | - | - | - | - | - |")
            elif st["error"]:
                lines.append(f"| {label} | ERROR | - | - | - | - | - | - | - |")
            elif alg == ALG_DP:
                cost_s = fmt_cost(st["cost"]) if st["feasible"] else "infeasible"
                lines.append(
                    f"| {label} | {cost_s} | N/A | {fmt_time(st['elapsed'])} "
                    f"| - | - | - | - | - |"
                )
            else:
                cost_s = fmt_cost(st.get("cost"))
                match = cost_match(st.get("cost"), dp_cost)
                if alg == ALG_CONCORDE:
                    lines.append(
                        f"| {label} | {cost_s} | {match} | {fmt_time(st['elapsed'])} "
                        f"| - | - | - | - | {fmt_val(st.get('bbnodes'))} |"
                    )
                else:
                    lines.append(
                        f"| {label} | {cost_s} | {match} | {fmt_time(st['elapsed'])} "
                        f"| {fmt_val(st.get('nodes_created'))} | {fmt_val(st.get('nodes_expanded'))} "
                        f"| {fmt_val(st.get('pruned_bound'))} | {fmt_val(st.get('pruned_infeasible'))} "
                        f"| - |"
                    )
        lines.append("")

        # Tours
        tour_lines = []
        for alg in ALL_ALGORITHMS:
            st = results[alg]
            if alg == ALG_DP:
                if st.get("tour_list"):
                    tour = st["tour_list"]
                    t_str = " -> ".join(str(v) for v in tour) + f" -> {tour[0]}"
                    tour_lines.append(
                        f"- **{alg} (ground truth):** `{t_str}` cost={fmt_cost(st['cost'])}"
                    )
            else:
                if st.get("tour_list") and len(st["tour_list"]) > 1:
                    unique_tour = list(dict.fromkeys(st["tour_list"]))
                    cost_info = ""
                    if dp_feasible and st.get("feasible"):
                        if abs(st["cost"] - dp_cost) < 1e-6:
                            cost_info = " (=DP)"
                        else:
                            cost_info = " (DIFFERS from DP)"
                    tour_lines.append(
                        f"- **{alg}:** `{' -> '.join(str(v) for v in unique_tour)}` "
                        f"cost={fmt_cost(st.get('cost'))}{cost_info}"
                    )

        if tour_lines:
            lines.append("**Tours found:**")
            lines.extend(tour_lines)
            lines.append("")

        # Debug output (collapsible)
        for alg in [ALG_SMART, ALG_SIMPLE, ALG_CONCORDE]:
            st = results[alg]
            debug_text = st.get("stderr", "")
            if debug_text.strip():
                lines.append(f"<details>")
                lines.append(f"<summary>{alg} debug output</summary>")
                lines.append("")
                lines.append("```")
                lines.append(debug_text.strip())
                lines.append("```")
                lines.append("</details>")
                lines.append("")

        # ── Collect per-instance data ──
        dp_solved = results[ALG_DP]["feasible"]
        per_instance.append({
            "idx": idx,
            "name": rel_path,
            "n": dim,
            "dp_cost": fmt_cost(results[ALG_DP]["cost"]) if dp_solved else "infeasible",
            "dp_time": fmt_time(results[ALG_DP]["elapsed"]),
            "smart_cost": fmt_cost(results[ALG_SMART].get("cost")) if results[ALG_SMART].get("feasible") else ("ERR" if results[ALG_SMART]["error"] else "TO" if results[ALG_SMART]["timeout"] else "-"),
            "smart_time": fmt_time(results[ALG_SMART]["elapsed"]),
            "simple_cost": fmt_cost(results[ALG_SIMPLE].get("cost")) if results[ALG_SIMPLE].get("feasible") else ("ERR" if results[ALG_SIMPLE]["error"] else "TO" if results[ALG_SIMPLE]["timeout"] else "-"),
            "simple_time": fmt_time(results[ALG_SIMPLE]["elapsed"]),
            "concorde_cost": fmt_cost(results[ALG_CONCORDE].get("cost")) if results[ALG_CONCORDE].get("feasible") else ("ERR" if results[ALG_CONCORDE]["error"] else "TO" if results[ALG_CONCORDE]["timeout"] else "-"),
            "concorde_time": fmt_time(results[ALG_CONCORDE]["elapsed"]),
        })

        lines.append("---")
        lines.append("")
        flush()

    # ── Summary ──
    lines.append("")
    lines.append("## Summary")
    lines.append("")

    lines.append("| Algorithm | Solved | Timeout | Error | Wrong Cost | Total Time | Avg Time | Avg B&B Nodes | Total B&B Nodes |")
    lines.append("|-----------|--------|---------|-------|------------|------------|----------|---------------|-----------------|")

    for alg in [ALG_SMART, ALG_SIMPLE, ALG_CONCORDE]:
        s = summary[alg]
        ok = s["ok"]
        timeout = s["timeout"]
        error = s["error"]
        wrong = s["wrong_cost"]
        total_time = s["total_time"]
        avg_time = total_time / ok if ok > 0 else 0
        avg_bb = s["total_bbnodes"] / s["count_nodes"] if s["count_nodes"] > 0 else 0
        lines.append(
            f"| {alg} | {ok} | {timeout} | {error} | {wrong} | {fmt_time(total_time)} "
            f"| {fmt_time(avg_time)} | {avg_bb:.0f} | {s['total_bbnodes']} |"
        )
    lines.append("")

    # Per-instance matrix
    lines.append("### Per-Instance Results Matrix")
    lines.append("")
    header = "| # | Instance | n | DP Cost | Smart Cost | Simple Cost | Concorde Cost | DP Time | Smart Time | Simple Time | Concorde Time |"
    lines.append(header)
    sep = "|---|---|---|---|---|---|---|---|---|---|---|"
    lines.append(sep)

    for pi in per_instance:
        lines.append(
            f"| {pi['idx']} | `{pi['name']}` | {pi['n']} "
            f"| {pi['dp_cost']} | {pi['smart_cost']} | {pi['simple_cost']} | {pi['concorde_cost']} "
            f"| {pi['dp_time']} | {pi['smart_time']} | {pi['simple_time']} | {pi['concorde_time']} |"
        )
    lines.append("")

    flush()

    print(f"\nResults written to: {OUT_PATH}", file=sys.stderr)


def _print_solver_status(label: str, stats: dict, dp_cost: float):
    """Print a one-line solver status to stderr."""
    if stats["timeout"]:
        print(f"TIMEOUT", file=sys.stderr)
    elif stats["error"]:
        print(f"ERROR: {stats['error']}", file=sys.stderr)
    elif stats["feasible"]:
        cost = stats["cost"]
        expanded = stats.get("nodes_expanded")
        if dp_cost < INF / 2:
            tag = "OK (=DP)" if abs(cost - dp_cost) < 1e-6 else f"WRONG (dp={dp_cost:.0f})"
        else:
            tag = "OK (infeasible verified)"
        print(f"cost={cost:.0f} {tag} time={fmt_time(stats['elapsed'])} expanded={expanded}", file=sys.stderr)
    else:
        print(f"INFEASIBLE", file=sys.stderr)


if __name__ == "__main__":
    main()
