#!/usr/bin/env python3
"""
Function runtime overhead chart for the tsp_bb solver.

Runs the solver under valgrind callgrind, parses the profile, and generates
a function overhead bar chart — showing which functions consume the most CPU time.

Usage:
  python3 tools/heatmap_analysis.py data/classic/tsplib/burma14.tsp --profile

Output: docs/heatmap/<instance>/function_overhead.png

Dependencies: pip install matplotlib numpy (valgrind required for profiling)
"""

from __future__ import annotations

import argparse
import os
import re
import subprocess
import sys
from collections import defaultdict
from typing import Optional

import numpy as np

import matplotlib
matplotlib.use("Agg")
import matplotlib.pyplot as plt

# ---------------------------------------------------------------------------
# Project paths
# ---------------------------------------------------------------------------
PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
DEFAULT_SOLVER = os.path.join(PROJECT_ROOT, "build", "tsp_bb")
OUT_DIR = os.path.join(PROJECT_ROOT, "docs", "heatmap")

CALLGRIND_OUT = "callgrind.out"
CALLGRIND_ANNOTATE = "callgrind_annotate.txt"
CALLGRIND_FLAT = "callgrind_flat.txt"


# ---------------------------------------------------------------------------
# Data structures
# ---------------------------------------------------------------------------

class CallEdge:
    """A directed edge in the call graph: caller → callee."""
    __slots__ = ("caller", "callee", "ir_count")
    def __init__(self, caller: str, callee: str, ir_count: int = 0):
        self.caller = caller
        self.callee = callee
        self.ir_count = ir_count


class FunctionInfo:
    """Summary info for one function."""
    __slots__ = ("name", "file", "total_ir", "self_ir", "pct")
    def __init__(self, name: str = "", file: str = "",
                 total_ir: int = 0, self_ir: int = 0, pct: float = 0.0):
        self.name = name
        self.file = file
        self.total_ir = total_ir   # inclusive Ir (incl. children)
        self.self_ir = self_ir     # exclusive Ir (self only)
        self.pct = pct


class CallGraph:
    """Parsed call graph from callgrind."""
    def __init__(self):
        self.edges: list[CallEdge] = []
        self.functions: dict[str, FunctionInfo] = {}
        self.total_ir: int = 0
        self.program_total_ir: int = 0  # from PROGRAM TOTALS line


# ---------------------------------------------------------------------------
# Profiling collection
# ---------------------------------------------------------------------------

def run_solver_callgrind(instance_path: str, solver_binary: str,
                         strategy: str, timeout: int, out_dir: str) -> str:
    """Run tsp_bb under valgrind callgrind. Returns stderr (debug log)."""
    os.makedirs(out_dir, exist_ok=True)
    cg_out = os.path.join(out_dir, CALLGRIND_OUT)

    cmd = [
        "valgrind",
        "--tool=callgrind",
        f"--callgrind-out-file={cg_out}",
        "--dump-instr=yes",
        "--collect-jumps=yes",
        "--",
        solver_binary,
        "--branch-strategy", strategy,
        "--exact-max-n", "100",
        "--debug",
        "--debug-interval", "1",
        instance_path,
    ]
    print(f"  callgrind running ...", file=sys.stderr, end=" ", flush=True)
    try:
        result = subprocess.run(cmd, capture_output=True, text=True,
                                timeout=timeout * 3)
    except subprocess.TimeoutExpired:
        print("TIMEOUT", file=sys.stderr)
        return ""
    if result.returncode != 0:
        tail = result.stderr[-400:] if result.stderr else "(no output)"
        print(f"FAILED (exit {result.returncode})", file=sys.stderr)
        raise SystemExit(f"valgrind callgrind failed:\n{tail}")
    print("done", file=sys.stderr)

    annotate_path = os.path.join(out_dir, CALLGRIND_ANNOTATE)
    print(f"  callgrind_annotate (tree) ...", file=sys.stderr, end=" ", flush=True)
    with open(annotate_path, "w") as af:
        subprocess.run(
            ["callgrind_annotate", "--auto=yes", "--inclusive=yes",
             "--tree=both", cg_out],
            stdout=af, stderr=subprocess.DEVNULL, timeout=60,
        )
    print("done", file=sys.stderr)

    # Also generate a flat profile with self (exclusive) Ir
    flat_path = os.path.join(out_dir, CALLGRIND_FLAT)
    print(f"  callgrind_annotate (flat) ...", file=sys.stderr, end=" ", flush=True)
    with open(flat_path, "w") as af:
        subprocess.run(
            ["callgrind_annotate", "--auto=yes", cg_out],
            stdout=af, stderr=subprocess.DEVNULL, timeout=60,
        )
    print("done", file=sys.stderr)

    return result.stderr if result.stderr else ""


# ---------------------------------------------------------------------------
# Function name cleaning
# ---------------------------------------------------------------------------

def _clean_func_name(raw: str) -> str:
    """Clean up callgrind-annotated function names.

    Removes callgrind cycle markers ('2), ??? prefixes, and normalizes whitespace.
    Also strips the leading '<' or '>' marker prefix if present.
    """
    name = raw.strip()
    # Remove callgrind brace markers: *  func, < func, >   func  →  func
    name = re.sub(r'^[<>]\s+', '', name)
    # Remove leading ???: prefix (unknown ELF source indicator)
    name = re.sub(r'^\?\?\?:', '', name)
    # Remove cycle-number suffix like '2, '3 etc.
    name = re.sub(r"'[0-9]+$", '', name)
    # Collapse whitespace
    name = re.sub(r'\s+', ' ', name).strip()
    # Remove leading/trailing whitespace around colons
    return name


def _clean_file(raw: str) -> str:
    """Extract and shorten file path."""
    raw = raw.strip()
    # Remove square brackets if present
    raw = re.sub(r'^\[|\]$', '', raw)
    # Keep only the filename and line number
    parts = raw.split("/")
    if len(parts) >= 2:
        return "/".join(parts[-2:])
    return raw


def _shorten_func(name: str, max_len: int = 40) -> str:
    """Shorten C++ qualified names for display.

    Handles long template signatures by keeping the key method/function name
    and collapsing type parameters.
    """
    if len(name) <= max_len:
        return name

    # Remove call count suffixes like " (18,322x)" — should already be
    # stripped by _parse_func_info, but be defensive
    name = re.sub(r'\s+\(\d[\d,]*x\)\s*$', '', name)

    # Collapse all template arguments to <>
    name = re.sub(r'<[^<>]*>', '<>', name)
    # Handle nested templates
    for _ in range(3):
        name = re.sub(r'<[^<>]*<[^<>]*>[^<>]*>', '<>', name)

    # Collapse parameter lists within lambdas
    name = re.sub(r'\{lambda\([^)]*\)', '{lambda', name)

    if len(name) <= max_len:
        return name

    # Try to find the most meaningful short name
    parts = name.split("::")
    if len(parts) >= 2:
        # Look for a function-like name at the end (contains parentheses)
        for i in range(len(parts) - 1, -1, -1):
            if '(' in parts[i]:
                short = "::".join(parts[max(0, i-1):])
                if len(short) <= max_len:
                    return short
                short = parts[i]
                if len(short) <= max_len:
                    return short
                return short[:max_len - 3] + "..."

        # Fallback: last 2 segments
        short = "::".join(parts[-2:])
        if len(short) <= max_len:
            return short
        short = parts[-1]
        if len(short) <= max_len:
            return short
        return parts[-1][:max_len - 3] + "..."

    return name[:max_len - 3] + "..."


# ---------------------------------------------------------------------------
# Call tree parsing
# ---------------------------------------------------------------------------

def _parse_ir_pct(line: str) -> tuple[int, float]:
    """Extract Ir count and percentage from a callgrind line.

    e.g. '1,234,567  ( 12.34%)  ...' → (1234567, 12.34)
    """
    m = re.match(r'^\s*([\d,]+)\s+\(\s*([\d.]+)%\)', line)
    if m:
        return int(m.group(1).replace(",", "")), float(m.group(2))
    return 0, 0.0


def _parse_func_info(rest: str) -> tuple[str, str, int]:
    """Parse function name, file, and call count from the remainder of a
    callgrind tree line (after the marker).

    Returns (func_name, file, call_count).
    """
    rest = rest.strip()

    # Step 1: extract file path [...] at the end
    file_loc = "???"
    m = re.search(r'\[(.+?)\]\s*$', rest)
    if m:
        file_loc = m.group(1).strip()
        rest = rest[:m.start()].strip()

    # Step 2: extract call count (Nx) at the end (now file path is gone)
    call_count = 1
    m = re.search(r'\(([\d,]+)x\)\s*$', rest)
    if m:
        call_count = int(m.group(1).replace(",", ""))
        rest = rest[:m.start()].strip()

    func_name = _clean_func_name(rest)
    file_loc = _clean_file(file_loc)

    return func_name, file_loc, call_count


def parse_callgrind_call_tree(annotate_path: str) -> CallGraph:
    """Parse the call-tree section of callgrind_annotate --tree=both output.

    The format encodes a call tree via markers and indentation:

        Ir  file:function
        -------------------------------------------------------
        <ir> (<pct>%)  < caller_name (Nx) [file]     ← caller of * below
        <ir> (<pct>%)  * function_name (Nx) [file]   ← function itself
        <ir> (<pct>%)  > callee_name (Nx) [file]     ← callee of * above
            <ir> (<pct>%)  > sub_callee (Nx) [file]  ← callee of > above

    Indentation (leading spaces before Ir count) indicates call depth.
    """
    cg = CallGraph()
    if not os.path.exists(annotate_path):
        return cg

    with open(annotate_path) as f:
        lines = f.readlines()

    # Find the start of the profiled functions section
    start_idx = -1
    for i, line in enumerate(lines):
        if re.match(r'^\s*Ir\s+file:function', line):
            start_idx = i + 1
            break
    if start_idx < 0:
        return cg

    # Skip separator line
    if start_idx < len(lines) and '---' in lines[start_idx]:
        start_idx += 1

    # Parse PROGRAM TOTALS before the tree
    for line in lines[:start_idx]:
        m = re.match(r'^\s*([\d,]+)\s+\(\s*([\d.]+)%\)\s+PROGRAM TOTALS', line)
        if m:
            cg.program_total_ir = int(m.group(1).replace(",", ""))
            break

    # Stack tracks the current call chain: list of (depth, func_name, marker)
    # marker is '*', '>', or '<' — used to distinguish same-depth siblings
    stack: list[tuple[int, str, str]] = []
    # Pending callers ('<' lines) that will call the next '*'
    pending_callers: list[tuple[int, str, int]] = []  # (depth, name, ir)

    for i in range(start_idx, len(lines)):
        line = lines[i]
        if not line.strip():
            continue

        # Compute depth from leading whitespace
        depth = len(line) - len(line.lstrip(' '))

        # Find the marker character after the "%)  " pattern
        marker_m = re.search(r'%\)\s+(<|\*|>)\s+', line)
        if not marker_m:
            continue

        marker = marker_m.group(1)
        rest = line[marker_m.end():]

        ir_count, pct = _parse_ir_pct(line)
        func_name, file_loc, call_count = _parse_func_info(rest)

        if not func_name:
            continue

        if marker == '*':
            # --- Function definition ---
            # If no pending callers, this is a new call-tree root → clear stack
            if not pending_callers:
                stack.clear()

            # Pop deeper entries (including same-depth '>') to make room
            while stack and stack[-1][0] >= depth:
                stack.pop()

            # All pending callers are direct callers of this function
            for pdepth, pname, pir in pending_callers:
                cg.edges.append(CallEdge(caller=pname, callee=func_name,
                                         ir_count=pir))
            pending_callers.clear()

            # If there's a parent in the stack, it also calls this function
            if stack:
                cg.edges.append(CallEdge(caller=stack[-1][1], callee=func_name,
                                         ir_count=ir_count))

            stack.append((depth, func_name, marker))
            _record_function(cg, func_name, file_loc, ir_count, pct)

        elif marker == '<':
            # --- Caller entry (calls the next '*' seen) ---
            pending_callers.append((depth, func_name, ir_count))
            _record_function(cg, func_name, file_loc, ir_count, pct)

        elif marker == '>':
            # --- Callee entry ---
            # Pop deeper entries AND same-depth '>' siblings (but not same-depth '*')
            while stack and stack[-1][0] >= depth and stack[-1][2] != '*':
                stack.pop()

            if stack:
                caller = stack[-1][1]
                cg.edges.append(CallEdge(caller=caller, callee=func_name,
                                         ir_count=ir_count))

            stack.append((depth, func_name, marker))
            _record_function(cg, func_name, file_loc, ir_count, pct)

    # Compute total Ir
    if cg.functions:
        cg.total_ir = max(f.total_ir for f in cg.functions.values())

    return cg


def _record_function(cg: CallGraph, name: str, file_loc: str,
                     ir_count: int, pct: float) -> None:
    """Record or update function info, keeping the entry with highest Ir."""
    if name not in cg.functions or ir_count > cg.functions[name].total_ir:
        cg.functions[name] = FunctionInfo(
            name=name, file=file_loc, total_ir=ir_count, self_ir=0, pct=pct)


def merge_flat_self_ir(cg: CallGraph, flat_path: str) -> None:
    """Parse the flat callgrind annotate output (without --inclusive) to
    extract self (exclusive) Ir for each function, and merge into the
    existing CallGraph function entries.

    Flat profile format:
        Ir  file:function
        -------------------------------------------------------
        1,234,567 (12.34%)  ns::func()  [file:line]
    """
    if not os.path.exists(flat_path):
        return

    with open(flat_path) as f:
        text = f.read()

    # Also parse PROGRAM TOTALS from the flat profile if not already set
    if cg.program_total_ir == 0:
        m = re.search(r'^\s*([\d,]+)\s+\(\s*[\d.]+\s*%\)\s+PROGRAM TOTALS', text, re.MULTILINE)
        if m:
            cg.program_total_ir = int(m.group(1).replace(",", ""))

    # Parse each function line from the flat profile
    for line in text.splitlines():
        # Match: <ir> (<pct>%)  <func_name>  [<file>]
        m = re.match(
            r'^\s*([\d,]+)\s+\(\s*([\d.]+)%\)\s+(.+?)\s+\[(.+?)\]\s*$', line)
        if not m:
            continue

        self_ir = int(m.group(1).replace(",", ""))
        pct = float(m.group(2))
        raw_name = m.group(3).strip()
        file_loc = m.group(4).strip()

        func_name = _clean_func_name(raw_name)
        file_loc = _clean_file(file_loc)

        # Merge: update self_ir for matching functions
        if func_name in cg.functions:
            cg.functions[func_name].self_ir = self_ir
            # Use the flat profile's percentage (self % of program total)
            cg.functions[func_name].pct = pct
        else:
            cg.functions[func_name] = FunctionInfo(
                name=func_name, file=file_loc, total_ir=self_ir,
                self_ir=self_ir, pct=pct)


# ---------------------------------------------------------------------------
# Distance matrix loading (kept for TSPLIB support)
# ---------------------------------------------------------------------------

def _is_tsplib(content: str) -> bool:
    upper = content[:4096].upper()
    for marker in ("EDGE_WEIGHT_SECTION", "NODE_COORD_SECTION",
                   "EDGE_WEIGHT_TYPE", "TYPE:", "NAME:"):
        if marker in upper:
            return True
    return False


def load_distance_matrix(filepath: str) -> tuple[np.ndarray, int]:
    """Load a TSP distance matrix from file."""
    with open(filepath) as f:
        content = f.read()

    if not _is_tsplib(content):
        tokens = content.split()
        n = int(tokens[0])
        matrix = np.zeros((n, n))
        idx = 1
        for i in range(n):
            for j in range(n):
                token = tokens[idx].lower()
                idx += 1
                if token in ("x", "-", "inf", "infinity"):
                    matrix[i, j] = np.nan
                else:
                    matrix[i, j] = float(token)
        return matrix, n

    return _load_tsplib_coords(content)


def _load_tsplib_coords(content: str) -> tuple[np.ndarray, int]:
    lines = content.splitlines()
    coords = []
    dimension = 0
    edge_weight_type = "EUC_2D"
    in_section = False
    for line in lines:
        line = line.strip()
        upper = line.upper()
        if upper == "NODE_COORD_SECTION":
            in_section = True
            continue
        if in_section:
            if upper in ("EOF", "DISPLAY_DATA_SECTION", "TOUR_SECTION",
                         "EDGE_WEIGHT_SECTION", "DEPOT_SECTION"):
                break
            parts = line.split()
            if len(parts) >= 3:
                coords.append((float(parts[1]), float(parts[2])))
        else:
            if upper.startswith("DIMENSION"):
                line = line.replace(" ", "")
                parts = line.split(":")
                dimension = int(parts[1]) if len(parts) == 2 else int(line.split()[1])
            elif upper.startswith("EDGE_WEIGHT_TYPE"):
                line = line.replace(" ", "")
                parts = line.split(":")
                edge_weight_type = (parts[1].strip().upper() if len(parts) == 2
                                    else line.split()[1].upper())
    n = dimension if dimension > 0 else len(coords)
    matrix = np.zeros((n, n))
    for i in range(n):
        for j in range(n):
            if i == j:
                continue
            dx = coords[i][0] - coords[j][0]
            dy = coords[i][1] - coords[j][1]
            if edge_weight_type == "GEO":
                import math
                PI = math.pi
                def to_rad(deg_val):
                    d = int(deg_val); m = deg_val - d
                    return PI * (d + 5.0 * m / 3.0) / 180.0
                lat_a = to_rad(coords[i][0]); lon_a = to_rad(coords[i][1])
                lat_b = to_rad(coords[j][0]); lon_b = to_rad(coords[j][1])
                q1 = math.cos(lon_a - lon_b); q2 = math.cos(lat_a - lat_b)
                q3 = math.cos(lat_a + lat_b)
                arg = 0.5 * ((1.0 + q1) * q2 - (1.0 - q1) * q3)
                arg = max(-1.0, min(1.0, arg))
                matrix[i, j] = int(6378.388 * math.acos(arg) + 1.0)
            else:
                matrix[i, j] = np.sqrt(dx * dx + dy * dy)
    return matrix, n


# ---------------------------------------------------------------------------
# Solver function filter
# ---------------------------------------------------------------------------

def _is_solver_func(f: FunctionInfo) -> bool:
    """Check if a function belongs to the solver (not system library)."""
    # Must come from the solver binary
    if "tsp_bb" not in f.file:
        return False
    # Exclude system-library-like entries
    if f.name.startswith("0x") or f.name.startswith("???"):
        return False
    if "below main" in f.name.lower():
        return False
    if re.match(r'^[0-9a-fA-Fx]+$', f.name[:10]):
        return False
    return True


# ---------------------------------------------------------------------------
# Function overhead chart
# ---------------------------------------------------------------------------

def function_overhead_chart(cg: CallGraph, instance_name: str,
                            out_dir: str, top_n: int = 30) -> str:
    """Generate a function runtime overhead chart.

    Vertical bars (with an overlaid line) showing the top-N solver functions
    by Ir (instruction fetch) overhead percentage — effectively CPU time share.

    X-axis: function names (shortened)
    Y-axis: % of total Ir
    """
    out_path = os.path.join(out_dir, "function_overhead.png")
    os.makedirs(out_dir, exist_ok=True)

    # Sort by self Ir (exclusive cost — function's own instructions only)
    solver_funcs = sorted(
        [f for f in cg.functions.values() if _is_solver_func(f)],
        key=lambda f: -f.self_ir)

    if not solver_funcs:
        fig, ax = plt.subplots(figsize=(10, 4))
        ax.text(0.5, 0.5, "No solver function data — run with --profile",
                ha="center", va="center", transform=ax.transAxes,
                fontsize=14, color="gray")
        fig.tight_layout()
        fig.savefig(out_path, dpi=150, bbox_inches="tight")
        plt.close(fig)
        return out_path

    # ── Select top-N and compute self-Ir percentages ──
    funcs = solver_funcs[:top_n]
    # Normalize by program total Ir: each function's SELF cost / total * 100
    # Self cost is always ≤ program total, so percentages are always ≤ 100%
    base_ir = cg.program_total_ir if cg.program_total_ir > 0 else max(1, sum(f.self_ir for f in funcs))
    names = [_shorten_func(f.name, 45) for f in funcs]
    pcts = np.array([f.self_ir / base_ir * 100.0 for f in funcs])
    irs = np.array([f.self_ir for f in funcs])

    # ── Plot ──
    fig_w = max(14, top_n * 0.35)
    fig_h = max(7, top_n * 0.25)
    fig, ax = plt.subplots(figsize=(fig_w, fig_h))

    x = np.arange(len(names))
    colors = plt.cm.YlOrRd(0.2 + pcts / pcts.max() * 0.8)

    # Bars
    bars = ax.bar(x, pcts, color=colors, edgecolor="#555", linewidth=0.4, zorder=3)

    # Line overlay connecting bar tops
    ax.plot(x, pcts, "o-", color="#c0392b", linewidth=1.8, markersize=5,
            markerfacecolor="white", markeredgecolor="#c0392b",
            markeredgewidth=1.5, zorder=4)

    # Annotate each bar with % and Ir count
    for i, (pct, ir) in enumerate(zip(pcts, irs)):
        if ir >= 1_000_000_000:
            ir_label = f"{ir / 1_000_000_000:.1f}B"
        elif ir >= 1_000_000:
            ir_label = f"{ir / 1_000_000:.1f}M"
        elif ir >= 1_000:
            ir_label = f"{ir / 1_000:.0f}K"
        else:
            ir_label = str(int(ir))
        ax.text(i, pct + pcts.max() * 0.015, f"{pct:.1f}%",
                ha="center", va="bottom", fontsize=7, fontweight="bold",
                color="#333")
        ax.text(i, pct - pcts.max() * 0.03, ir_label,
                ha="center", va="top", fontsize=6, color="#666",
                rotation=90)

    ax.set_xticks(x)
    ax.set_xticklabels(names, rotation=60, ha="right", fontsize=7,
                       fontfamily="monospace")
    ax.set_ylabel("Ir Overhead (%)", fontsize=12)
    ax.set_xlim(-0.6, top_n - 0.4)
    ax.set_ylim(0, pcts.max() * 1.18)
    ax.yaxis.set_major_formatter(plt.FuncFormatter(lambda v, _: f"{v:.0f}%"))
    ax.grid(axis="y", alpha=0.3, linestyle="--", linewidth=0.5)

    ax.set_title(f"Function Runtime Overhead (Self Cost) — {instance_name}\n"
                 f"({len(solver_funcs)} solver functions, "
                 f"program total {cg.program_total_ir:,} Ir, "
                 f"top {top_n} shown)",
                 fontsize=13, fontweight="bold")

    fig.tight_layout()
    fig.savefig(out_path, dpi=150, bbox_inches="tight")
    plt.close(fig)

    return out_path


# ---------------------------------------------------------------------------
# Text summary
# ---------------------------------------------------------------------------

def print_call_summary(cg: CallGraph, top_n: int = 20) -> None:
    """Print a readable text summary of the call graph to stderr."""
    if not cg.functions:
        print("\n(no function data)", file=sys.stderr)
        return

    # Filter to solver functions only
    solver_names = {f.name for f in cg.functions.values()
                    if _is_solver_func(f)}
    if not solver_names:
        print("\n(no solver functions)", file=sys.stderr)
        return

    solver_edges = [(e.caller, e.callee, e.ir_count) for e in cg.edges
                    if e.caller in solver_names and e.callee in solver_names]

    sep = "=" * 80
    print(f"\n{sep}", file=sys.stderr)
    print(f"  Solver Call Graph: {len(solver_names)} functions, "
          f"{len(solver_edges)} call edges",
          file=sys.stderr)
    if cg.program_total_ir > 0:
        print(f"  Program total: {cg.program_total_ir:,} Ir",
              file=sys.stderr)
    print(sep, file=sys.stderr)

    solver_funcs = sorted(
        [f for f in cg.functions.values() if f.name in solver_names],
        key=lambda f: -f.self_ir)

    print(f"\n  Top Functions by Self Ir (exclusive):", file=sys.stderr)
    print(f"  {'Function':<50} {'Self Ir':>14}  {'File':<25}",
          file=sys.stderr)
    print(f"  {'-'*48}  {'-'*12}  {'-'*23}", file=sys.stderr)
    for func in solver_funcs[:top_n]:
        fname = _shorten_func(func.name, 49)
        print(f"  {fname:<50} {func.self_ir:>14,}  {func.file}",
              file=sys.stderr)

    # Deduplicate edges for display
    edge_agg: dict[tuple[str, str], int] = defaultdict(int)
    for caller, callee, ir in solver_edges:
        edge_agg[(caller, callee)] += ir
    sorted_edges = sorted(edge_agg.items(), key=lambda x: -x[1])

    print(f"\n  Top Call Edges by Ir:", file=sys.stderr)
    print(f"  {'Caller':<38} → {'Callee':<38} {'Ir':>14}",
          file=sys.stderr)
    print(f"  {'-'*36}   {'-'*36} {'-'*12}", file=sys.stderr)
    for (caller, callee), ir_count in sorted_edges[:top_n]:
        c1 = _shorten_func(caller, 37)
        c2 = _shorten_func(callee, 37)
        print(f"  {c1:<38} → {c2:<38} {ir_count:>14,}",
              file=sys.stderr)


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main():
    parser = argparse.ArgumentParser(
        description="Function runtime overhead chart for tsp_bb solver")
    parser.add_argument("instance", help="Path to TSP instance file")
    parser.add_argument("--solver", default=DEFAULT_SOLVER,
                        help=f"Path to tsp_bb binary (default: {DEFAULT_SOLVER})")
    parser.add_argument("--strategy", choices=["smart", "simple"],
                        default="smart", help="Branch strategy")
    parser.add_argument("--timeout", type=int, default=1800,
                        help="Solver timeout in seconds")
    parser.add_argument("--profile", action="store_true",
                        help="Collect callgrind profiling data (requires valgrind)")
    parser.add_argument("--load-profile",
                        help="Replay saved profiling data directory")
    parser.add_argument("--out-dir",
                        help="Output directory (default: docs/heatmap/<instance>)")
    parser.add_argument("--top-n", type=int, default=30,
                        help="Number of top functions in heatmap (default: 30)")
    args = parser.parse_args()

    instance_path = os.path.abspath(args.instance)
    instance_name = os.path.splitext(os.path.basename(instance_path))[0]

    if args.out_dir:
        out_dir = args.out_dir
    else:
        out_dir = os.path.join(OUT_DIR, instance_name)

    print(f"Instance: {instance_path}", file=sys.stderr)
    print(f"Output:  {out_dir}", file=sys.stderr)
    os.makedirs(out_dir, exist_ok=True)

    # ── Load distance matrix ──
    matrix, n = load_distance_matrix(instance_path)
    print(f"Matrix:  {n}×{n}", file=sys.stderr)

    # ── Obtain call graph ──
    if args.load_profile:
        print(f"Loading profile from: {args.load_profile}", file=sys.stderr)
        cg = parse_callgrind_call_tree(
            os.path.join(args.load_profile, CALLGRIND_ANNOTATE))
        merge_flat_self_ir(cg, os.path.join(args.load_profile, CALLGRIND_FLAT))
    elif args.profile:
        print(f"Profiling solver ({args.strategy})...", file=sys.stderr)
        run_solver_callgrind(instance_path, args.solver, args.strategy,
                             args.timeout, out_dir)
        cg = parse_callgrind_call_tree(
            os.path.join(out_dir, CALLGRIND_ANNOTATE))
        merge_flat_self_ir(cg, os.path.join(out_dir, CALLGRIND_FLAT))
    else:
        print("Error: --profile flag required", file=sys.stderr)
        print("Usage: python3 tools/heatmap_analysis.py <instance> --profile",
              file=sys.stderr)
        sys.exit(1)

    # ── Text summary ──
    print_call_summary(cg, top_n=15)

    # ── Heatmap ──
    print(f"\nGenerating function overhead chart...", file=sys.stderr)
    path = function_overhead_chart(cg, instance_name, out_dir, top_n=args.top_n)
    print(f"  → {path}", file=sys.stderr)
    print("Done.", file=sys.stderr)


if __name__ == "__main__":
    main()
