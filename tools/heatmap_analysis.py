#!/usr/bin/env python3
"""
Function time-share analysis for the tsp_bb solver.

The profiler uses valgrind/callgrind to get deterministic instruction counts
per function. Because callgrind does not measure native per-function seconds,
this script optionally runs the solver once without valgrind, then scales each
function's callgrind percentage by that native elapsed time. The reported
seconds are therefore estimated native wall-time contributions; the percentages
come directly from callgrind Ir counts.

Typical usage:
  python3 tools/heatmap_analysis.py data/classic/tsplib/burma14.tsp --profile
  python3 tools/heatmap_analysis.py data/classic/tsplib/burma14.tsp \
      --load-profile docs/heatmap/burma14

Outputs:
  docs/heatmap/<instance>/function_time_breakdown.csv
  docs/heatmap/<instance>/report.md
  docs/heatmap/<instance>/01_self_time_share.png
  docs/heatmap/<instance>/02_inclusive_time_share.png
  docs/heatmap/<instance>/03_function_metric_heatmap.png
"""

from __future__ import annotations

import argparse
import csv
import json
import os
import re
import shutil
import subprocess
import sys
import time
from dataclasses import dataclass, field
from typing import Iterable


# ---------------------------------------------------------------------------
# Project paths and output names
# ---------------------------------------------------------------------------

PROJECT_ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))
DEFAULT_SOLVER = os.path.join(PROJECT_ROOT, "build", "tsp_bb")
DEFAULT_OUT_ROOT = os.path.join(PROJECT_ROOT, "docs", "heatmap")

CALLGRIND_OUT = "callgrind.out"
CALLGRIND_SELF = "callgrind_self.txt"
CALLGRIND_INCLUSIVE = "callgrind_inclusive.txt"
CALLGRIND_TREE = "callgrind_tree.txt"
METADATA_JSON = "profile_metadata.json"
BREAKDOWN_CSV = "function_time_breakdown.csv"
REPORT_MD = "report.md"

# Legacy names from older versions of this tool.
LEGACY_CALLGRIND_ANNOTATE = "callgrind_annotate.txt"
LEGACY_CALLGRIND_FLAT = "callgrind_flat.txt"


# ---------------------------------------------------------------------------
# Data structures
# ---------------------------------------------------------------------------

@dataclass
class SolverRun:
    elapsed_sec: float | None = None
    returncode: int | None = None
    stdout: str = ""
    stderr: str = ""
    stats: dict[str, str] = field(default_factory=dict)


@dataclass
class FunctionInfo:
    name: str
    file: str = "???"
    kind: str = "other"
    self_ir: int = 0
    inclusive_ir: int = 0
    self_pct: float = 0.0
    inclusive_pct: float = 0.0
    calls: int = 0

    def display_name(self, max_len: int = 72) -> str:
        return shorten_func(self.name, max_len)


@dataclass
class Profile:
    functions: dict[str, FunctionInfo] = field(default_factory=dict)
    program_total_ir: int = 0
    native_elapsed_sec: float | None = None
    callgrind_elapsed_sec: float | None = None
    solver_stats: dict[str, str] = field(default_factory=dict)


# ---------------------------------------------------------------------------
# Small helpers
# ---------------------------------------------------------------------------

def eprint(*args, **kwargs) -> None:
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


def fmt_ir(value: int) -> str:
    if value >= 1_000_000_000:
        return f"{value / 1_000_000_000:.2f}B"
    if value >= 1_000_000:
        return f"{value / 1_000_000:.2f}M"
    if value >= 1_000:
        return f"{value / 1_000:.1f}K"
    return str(value)


def pct_to_seconds(percent: float, native_elapsed: float | None) -> float | None:
    if native_elapsed is None:
        return None
    return native_elapsed * percent / 100.0


def safe_pct(ir_count: int, total_ir: int) -> float:
    return (ir_count / total_ir * 100.0) if total_ir > 0 else 0.0


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


# ---------------------------------------------------------------------------
# Solver command and profiling collection
# ---------------------------------------------------------------------------

def solver_help_text(solver_binary: str) -> str:
    try:
        result = subprocess.run(
            [solver_binary, "--help"],
            capture_output=True,
            text=True,
            timeout=5,
        )
    except Exception:
        return ""
    return (result.stdout or "") + (result.stderr or "")


def build_solver_cmd(instance_path: str, solver_binary: str, exact_max_n: int,
                     strategy: str | None, debug: bool = False) -> list[str]:
    help_text = solver_help_text(solver_binary)
    cmd = [solver_binary]
    if "--branch-strategy" in help_text and strategy:
        cmd += ["--branch-strategy", strategy]
    if "--exact-max-n" in help_text:
        cmd += ["--exact-max-n", str(exact_max_n)]
    if debug and "--debug" in help_text:
        cmd += ["--debug", "--debug-interval", "1000000"]
    cmd.append(instance_path)
    return cmd


def parse_solver_stats(stdout: str, stderr: str) -> dict[str, str]:
    text = stdout + "\n" + stderr
    stats: dict[str, str] = {}
    patterns = {
        "cost": r"Optimal cost:\s*([^\n]+)",
        "nodes_created": r"Nodes created:\s*([^\n]+)",
        "nodes_expanded": r"Nodes expanded:\s*([^\n]+)",
        "pruned_bound": r"Pruned by bound:\s*([^\n]+)",
        "pruned_infeasible": r"Pruned infeasible:\s*([^\n]+)",
        "root_lower_bound": r"Root lower bound:\s*([^\n]+)",
        "initial_upper_bound": r"Initial upper bound:\s*([^\n]+)",
        "method": r"Method:\s*([^\n]+)",
    }
    for key, pattern in patterns.items():
        m = re.search(pattern, text)
        if m:
            stats[key] = m.group(1).strip()
    return stats


def run_native_solver(instance_path: str, solver_binary: str, exact_max_n: int,
                      strategy: str | None, timeout: int) -> SolverRun:
    cmd = build_solver_cmd(instance_path, solver_binary, exact_max_n, strategy)
    eprint("  native timing ...", end=" ", flush=True)
    started = time.perf_counter()
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=timeout,
        )
    except subprocess.TimeoutExpired as exc:
        elapsed = time.perf_counter() - started
        eprint(f"TIMEOUT after {fmt_seconds(elapsed)}")
        return SolverRun(
            elapsed_sec=elapsed,
            returncode=None,
            stdout=exc.stdout or "",
            stderr=exc.stderr or "",
        )
    elapsed = time.perf_counter() - started
    eprint(f"done ({fmt_seconds(elapsed)})")
    if result.returncode != 0:
        tail = (result.stderr or result.stdout)[-800:]
        raise SystemExit(f"native solver failed with exit {result.returncode}:\n{tail}")
    return SolverRun(
        elapsed_sec=elapsed,
        returncode=result.returncode,
        stdout=result.stdout,
        stderr=result.stderr,
        stats=parse_solver_stats(result.stdout, result.stderr),
    )


def require_tool(name: str) -> str:
    path = shutil.which(name)
    if not path:
        raise SystemExit(f"required tool not found on PATH: {name}")
    return path


def run_callgrind(instance_path: str, solver_binary: str, exact_max_n: int,
                  strategy: str | None, timeout: int, out_dir: str) -> float:
    require_tool("valgrind")
    require_tool("callgrind_annotate")

    os.makedirs(out_dir, exist_ok=True)
    cg_out = os.path.join(out_dir, CALLGRIND_OUT)
    solver_cmd = build_solver_cmd(instance_path, solver_binary, exact_max_n,
                                  strategy, debug=False)
    cmd = [
        "valgrind",
        "--tool=callgrind",
        f"--callgrind-out-file={cg_out}",
        "--dump-instr=yes",
        "--collect-jumps=yes",
        "--",
        *solver_cmd,
    ]

    eprint("  callgrind profiling ...", end=" ", flush=True)
    started = time.perf_counter()
    try:
        result = subprocess.run(
            cmd,
            capture_output=True,
            text=True,
            timeout=timeout * 3,
        )
    except subprocess.TimeoutExpired:
        elapsed = time.perf_counter() - started
        eprint(f"TIMEOUT after {fmt_seconds(elapsed)}")
        raise SystemExit("callgrind profiling timed out")
    elapsed = time.perf_counter() - started
    if result.returncode != 0:
        tail = (result.stderr or result.stdout)[-1200:]
        eprint(f"FAILED ({fmt_seconds(elapsed)})")
        raise SystemExit(f"callgrind failed with exit {result.returncode}:\n{tail}")
    eprint(f"done ({fmt_seconds(elapsed)})")

    annotate_jobs = [
        (CALLGRIND_SELF, ["callgrind_annotate", "--auto=yes", cg_out]),
        (CALLGRIND_INCLUSIVE,
         ["callgrind_annotate", "--auto=yes", "--inclusive=yes", cg_out]),
        (CALLGRIND_TREE,
         ["callgrind_annotate", "--auto=yes", "--inclusive=yes", "--tree=both", cg_out]),
    ]
    for filename, annotate_cmd in annotate_jobs:
        eprint(f"  writing {filename} ...", end=" ", flush=True)
        with open(os.path.join(out_dir, filename), "w") as out:
            subprocess.run(
                annotate_cmd,
                stdout=out,
                stderr=subprocess.DEVNULL,
                timeout=120,
                check=False,
            )
        eprint("done")

    return elapsed


def save_metadata(out_dir: str, instance_path: str, solver_binary: str,
                  strategy: str | None, exact_max_n: int,
                  native_run: SolverRun | None,
                  callgrind_elapsed_sec: float | None) -> None:
    data = {
        "instance": os.path.abspath(instance_path),
        "solver": os.path.abspath(solver_binary),
        "strategy": strategy,
        "exact_max_n": exact_max_n,
        "native_elapsed_sec": native_run.elapsed_sec if native_run else None,
        "callgrind_elapsed_sec": callgrind_elapsed_sec,
        "solver_stats": native_run.stats if native_run else {},
    }
    with open(os.path.join(out_dir, METADATA_JSON), "w") as f:
        json.dump(data, f, indent=2, sort_keys=True)


def load_metadata(profile_dir: str) -> dict:
    path = os.path.join(profile_dir, METADATA_JSON)
    if not os.path.exists(path):
        return {}
    with open(path) as f:
        return json.load(f)


# ---------------------------------------------------------------------------
# Callgrind annotate parsing
# ---------------------------------------------------------------------------

def clean_func_name(raw: str) -> str:
    name = raw.strip()
    name = re.sub(r"^[<>*]\s+", "", name)
    name = re.sub(r"^\?\?\?:", "", name)
    name = re.sub(r"'[0-9]+$", "", name)
    name = re.sub(r"\s+", " ", name).strip()
    return name


def clean_file(raw: str) -> str:
    file_loc = raw.strip()
    file_loc = re.sub(r"^\[|\]$", "", file_loc)
    if not file_loc or file_loc == "???":
        return "???"
    parts = file_loc.split("/")
    if len(parts) >= 3:
        return "/".join(parts[-3:])
    return file_loc


def shorten_func(name: str, max_len: int = 72) -> str:
    if len(name) <= max_len:
        return name
    short = re.sub(r"<[^<>]*>", "<>", name)
    for _ in range(4):
        short = re.sub(r"<[^<>]*<[^<>]*>[^<>]*>", "<>", short)
    short = re.sub(r"\([^)]{30,}\)", "(...)", short)
    if len(short) <= max_len:
        return short
    parts = short.split("::")
    if len(parts) >= 2:
        candidate = "::".join(parts[-2:])
        if len(candidate) <= max_len:
            return candidate
        candidate = parts[-1]
        if len(candidate) <= max_len:
            return candidate
    return short[:max_len - 3] + "..."


def parse_func_tail(rest: str) -> tuple[str, str, int]:
    rest = rest.strip()
    file_loc = "???"
    calls = 0

    m = re.search(r"\[(.+?)\]\s*$", rest)
    if m:
        file_loc = m.group(1).strip()
        rest = rest[:m.start()].strip()

    m = re.search(r"\(([\d,]+)x\)\s*$", rest)
    if m:
        calls = int(m.group(1).replace(",", ""))
        rest = rest[:m.start()].strip()

    return clean_func_name(rest), clean_file(file_loc), calls


def parse_annotate_flat(path: str) -> tuple[dict[str, tuple[int, float, str, int]], int]:
    """Return name -> (Ir, pct, file, calls), plus PROGRAM TOTALS Ir."""
    entries: dict[str, tuple[int, float, str, int]] = {}
    total_ir = 0
    if not os.path.exists(path):
        return entries, total_ir

    with open(path, errors="replace") as f:
        for line in f:
            m_total = re.match(
                r"^\s*([\d,]+)\s+\(\s*[\d.]+%\)\s+PROGRAM TOTALS",
                line,
            )
            if m_total:
                total_ir = int(m_total.group(1).replace(",", ""))
                continue

            m = re.match(r"^\s*([\d,]+)\s+\(\s*([\d.]+)%\)\s+(.+?)\s*$", line)
            if not m:
                continue

            ir_count = int(m.group(1).replace(",", ""))
            pct = float(m.group(2))
            rest = m.group(3).strip()
            if not rest or rest.startswith("PROGRAM TOTALS") or set(rest) == {"-"}:
                continue

            name, file_loc, calls = parse_func_tail(rest)
            if not name or name == "PROGRAM TOTALS":
                continue

            old = entries.get(name)
            if old is None or ir_count > old[0]:
                entries[name] = (ir_count, pct, file_loc, calls)

    return entries, total_ir


def classify_function(name: str, file_loc: str) -> str:
    low_name = name.lower()
    low_file = file_loc.lower()

    solver_markers = (
        "tsp::", "branchboundsolver", "tspproblem", "solveinput",
        "runsinglefile", "runbatch", "main",
    )
    solver_files = ("tspsolver.cpp", "tspsolver.hpp", "main.cpp", "/src/", "/include/")
    if any(marker in low_name for marker in solver_markers):
        return "solver"
    if any(marker in low_file for marker in solver_files):
        return "solver"

    runtime_markers = (
        "std::", "operator new", "operator delete", "malloc", "free",
        "memmove", "memcpy", "memset", "libc++", "vector<",
        "__uninitialized", "__tree", "dyld-stub",
    )
    if any(marker in low_name for marker in runtime_markers):
        return "runtime"
    if any(marker in low_file for marker in ("libc++", "libsystem_malloc", "libsystem_platform")):
        return "runtime"

    return "system"


def load_profile(profile_dir: str) -> Profile:
    self_path = os.path.join(profile_dir, CALLGRIND_SELF)
    incl_path = os.path.join(profile_dir, CALLGRIND_INCLUSIVE)

    if not os.path.exists(self_path):
        self_path = os.path.join(profile_dir, LEGACY_CALLGRIND_FLAT)
    if not os.path.exists(incl_path):
        incl_path = os.path.join(profile_dir, LEGACY_CALLGRIND_ANNOTATE)

    self_entries, self_total = parse_annotate_flat(self_path)
    incl_entries, incl_total = parse_annotate_flat(incl_path)

    metadata = load_metadata(profile_dir)
    profile = Profile()
    profile.program_total_ir = incl_total or self_total
    profile.native_elapsed_sec = metadata.get("native_elapsed_sec")
    profile.callgrind_elapsed_sec = metadata.get("callgrind_elapsed_sec")
    profile.solver_stats = metadata.get("solver_stats") or {}

    names = set(self_entries) | set(incl_entries)
    for name in names:
        self_ir, self_pct, self_file, self_calls = self_entries.get(name, (0, 0.0, "???", 0))
        incl_ir, incl_pct, incl_file, incl_calls = incl_entries.get(name, (0, 0.0, self_file, 0))
        file_loc = incl_file if incl_file != "???" else self_file
        if profile.program_total_ir > 0:
            if self_pct == 0.0 and self_ir:
                self_pct = safe_pct(self_ir, profile.program_total_ir)
            if incl_pct == 0.0 and incl_ir:
                incl_pct = safe_pct(incl_ir, profile.program_total_ir)
        profile.functions[name] = FunctionInfo(
            name=name,
            file=file_loc,
            kind=classify_function(name, file_loc),
            self_ir=self_ir,
            inclusive_ir=incl_ir,
            self_pct=self_pct,
            inclusive_pct=incl_pct,
            calls=max(self_calls, incl_calls),
        )

    return profile


# ---------------------------------------------------------------------------
# Filtering and tables
# ---------------------------------------------------------------------------

def select_functions(profile: Profile, scope: str) -> list[FunctionInfo]:
    funcs = list(profile.functions.values())
    if scope == "solver":
        funcs = [f for f in funcs if f.kind == "solver"]
    elif scope == "solver-runtime":
        funcs = [f for f in funcs if f.kind in {"solver", "runtime"}]
    elif scope == "all":
        pass
    else:
        raise ValueError(f"unknown scope: {scope}")
    funcs = [f for f in funcs if f.self_ir > 0 or f.inclusive_ir > 0]
    return sorted(funcs, key=lambda f: (-f.self_ir, -f.inclusive_ir, f.name))


def write_breakdown_csv(profile: Profile, funcs: list[FunctionInfo],
                        out_dir: str) -> str:
    path = os.path.join(out_dir, BREAKDOWN_CSV)
    native_elapsed = profile.native_elapsed_sec
    with open(path, "w", newline="") as f:
        writer = csv.writer(f)
        writer.writerow([
            "rank", "kind", "function", "file", "calls",
            "self_ir", "self_pct_total", "self_est_sec",
            "inclusive_ir", "inclusive_pct_total", "inclusive_est_sec",
        ])
        for rank, func in enumerate(funcs, start=1):
            writer.writerow([
                rank,
                func.kind,
                func.name,
                func.file,
                func.calls,
                func.self_ir,
                f"{func.self_pct:.6f}",
                "" if native_elapsed is None else f"{pct_to_seconds(func.self_pct, native_elapsed):.9f}",
                func.inclusive_ir,
                f"{func.inclusive_pct:.6f}",
                "" if native_elapsed is None else f"{pct_to_seconds(func.inclusive_pct, native_elapsed):.9f}",
            ])
    return path


def write_report(profile: Profile, funcs: list[FunctionInfo], out_dir: str,
                 instance_name: str, scope: str, top_n: int) -> str:
    path = os.path.join(out_dir, REPORT_MD)
    native_elapsed = profile.native_elapsed_sec
    top = funcs[:top_n]

    with open(path, "w") as f:
        f.write(f"# Function Time Share - {instance_name}\n\n")
        f.write("## Measurement\n\n")
        f.write("- Cost basis: callgrind `Ir` instruction counts.\n")
        f.write("- Time estimate: native elapsed time multiplied by each function percentage.\n")
        f.write("- Self time is exclusive and additive. Inclusive time includes callees and is not additive.\n")
        f.write(f"- Scope: `{scope}`.\n")
        f.write(f"- Program total: `{profile.program_total_ir:,}` Ir.\n")
        f.write(f"- Native elapsed: `{fmt_seconds(native_elapsed)}`.\n")
        f.write(f"- Callgrind elapsed: `{fmt_seconds(profile.callgrind_elapsed_sec)}`.\n")
        if profile.solver_stats:
            f.write("\n## Solver Stats\n\n")
            for key, value in profile.solver_stats.items():
                f.write(f"- `{key}`: `{value}`\n")

        f.write("\n## Top Functions By Self Time\n\n")
        f.write("| # | Kind | Function | Self | Inclusive | Calls |\n")
        f.write("|---:|---|---|---:|---:|---:|\n")
        for rank, func in enumerate(top, start=1):
            self_sec = fmt_seconds(pct_to_seconds(func.self_pct, native_elapsed))
            incl_sec = fmt_seconds(pct_to_seconds(func.inclusive_pct, native_elapsed))
            f.write(
                f"| {rank} | {func.kind} | `{func.display_name(68)}` "
                f"| {func.self_pct:.2f}% / {self_sec} "
                f"| {func.inclusive_pct:.2f}% / {incl_sec} "
                f"| {func.calls or ''} |\n"
            )

        total_self_pct = sum(func.self_pct for func in funcs)
        f.write("\n## Scope Total\n\n")
        f.write(f"- Selected function self share: `{total_self_pct:.2f}%` of program Ir.\n")
        f.write(f"- Selected function estimated self time: `{fmt_seconds(pct_to_seconds(total_self_pct, native_elapsed))}`.\n")
        f.write(f"- Full CSV: `{BREAKDOWN_CSV}`.\n")

    return path


def print_summary(profile: Profile, funcs: list[FunctionInfo], top_n: int) -> None:
    eprint("")
    eprint("=" * 96)
    eprint("Function time-share summary")
    eprint(f"Program total: {profile.program_total_ir:,} Ir")
    eprint(f"Native elapsed: {fmt_seconds(profile.native_elapsed_sec)}")
    eprint(f"Callgrind elapsed: {fmt_seconds(profile.callgrind_elapsed_sec)}")
    eprint("-" * 96)
    eprint(f"{'#':>2} {'Kind':<8} {'Self':>17} {'Inclusive':>17}  Function")
    eprint("-" * 96)
    for rank, func in enumerate(funcs[:top_n], start=1):
        self_sec = fmt_seconds(pct_to_seconds(func.self_pct, profile.native_elapsed_sec))
        incl_sec = fmt_seconds(pct_to_seconds(func.inclusive_pct, profile.native_elapsed_sec))
        eprint(
            f"{rank:>2} {func.kind:<8} "
            f"{func.self_pct:>6.2f}%/{self_sec:>9} "
            f"{func.inclusive_pct:>6.2f}%/{incl_sec:>9}  "
            f"{func.display_name(44)}"
        )
    eprint("=" * 96)


# ---------------------------------------------------------------------------
# Plotting
# ---------------------------------------------------------------------------

def annotate_barh(ax, values: Iterable[float], labels: Iterable[str],
                  pad: float) -> None:
    for i, (value, label) in enumerate(zip(values, labels)):
        ax.text(value + pad, i, label, va="center", ha="left", fontsize=8)


def kind_color(kind: str) -> str:
    return {
        "solver": "#386cb0",
        "runtime": "#fdb462",
        "system": "#b3b3b3",
        "other": "#8dd3c7",
    }.get(kind, "#8dd3c7")


def load_pyplot():
    try:
        import matplotlib
        matplotlib.use("Agg")
        import matplotlib.pyplot as plt
    except ModuleNotFoundError as exc:
        raise SystemExit(
            "matplotlib is required to generate heatmaps. "
            "Install it with: pip install matplotlib"
        ) from exc
    return plt


def plot_self_time_share(profile: Profile, funcs: list[FunctionInfo],
                         out_dir: str, instance_name: str, top_n: int) -> str:
    plt = load_pyplot()
    path = os.path.join(out_dir, "01_self_time_share.png")
    legacy_path = os.path.join(out_dir, "function_overhead.png")
    top = funcs[:top_n]

    if not top:
        fig, ax = plt.subplots(figsize=(10, 3))
        ax.text(0.5, 0.5, "No function data", ha="center", va="center")
        ax.axis("off")
        fig.savefig(path, dpi=180, bbox_inches="tight")
        fig.savefig(legacy_path, dpi=180, bbox_inches="tight")
        plt.close(fig)
        return path

    names = [f.display_name(58) for f in reversed(top)]
    pcts = [f.self_pct for f in reversed(top)]
    secs = [pct_to_seconds(f.self_pct, profile.native_elapsed_sec) for f in reversed(top)]
    colors = [kind_color(f.kind) for f in reversed(top)]
    y = list(range(len(names)))

    fig_h = max(6.0, len(top) * 0.34)
    fig, ax = plt.subplots(figsize=(13.5, fig_h))
    ax.barh(y, pcts, color=colors, edgecolor="#333333", linewidth=0.35)
    ax.set_yticks(y)
    ax.set_yticklabels(names, fontfamily="monospace", fontsize=8)
    ax.set_xlabel("Self cost (% of total program Ir)")
    ax.set_title(
        f"Top {len(top)} Self Time Share - {instance_name}\n"
        f"Native elapsed {fmt_seconds(profile.native_elapsed_sec)}, "
        f"program total {profile.program_total_ir:,} Ir",
        fontsize=12,
        fontweight="bold",
    )
    ax.grid(axis="x", alpha=0.25, linestyle="--")
    max_pct = max(pcts) if pcts else 1.0
    labels = [
        f"{pct:.2f}%"
        + (f" / {fmt_seconds(sec)}" if sec is not None else "")
        for pct, sec in zip(pcts, secs)
    ]
    annotate_barh(ax, pcts, labels, max(0.02, max_pct * 0.015))
    ax.set_xlim(0, max_pct * 1.24 + 0.05)

    legend_handles = [
        plt.Line2D([0], [0], color=kind_color("solver"), lw=6, label="solver"),
        plt.Line2D([0], [0], color=kind_color("runtime"), lw=6, label="runtime"),
        plt.Line2D([0], [0], color=kind_color("system"), lw=6, label="system"),
    ]
    ax.legend(handles=legend_handles, loc="lower right")

    fig.tight_layout()
    fig.savefig(path, dpi=180, bbox_inches="tight")
    fig.savefig(legacy_path, dpi=180, bbox_inches="tight")
    plt.close(fig)
    return path


def plot_inclusive_time_share(profile: Profile, funcs: list[FunctionInfo],
                              out_dir: str, instance_name: str, top_n: int) -> str:
    plt = load_pyplot()
    path = os.path.join(out_dir, "02_inclusive_time_share.png")
    top = sorted(funcs, key=lambda f: (-f.inclusive_ir, -f.self_ir, f.name))[:top_n]
    if not top:
        return path

    names = [f.display_name(58) for f in reversed(top)]
    self_pcts = [f.self_pct for f in reversed(top)]
    incl_pcts = [f.inclusive_pct for f in reversed(top)]
    y = list(range(len(names)))
    height = 0.38
    y_lower = [yi - height / 2 for yi in y]
    y_upper = [yi + height / 2 for yi in y]

    fig_h = max(6.0, len(top) * 0.36)
    fig, ax = plt.subplots(figsize=(13.5, fig_h))
    ax.barh(y_lower, incl_pcts, height=height, color="#9ecae1",
            edgecolor="#333333", linewidth=0.25, label="inclusive")
    ax.barh(y_upper, self_pcts, height=height, color="#fb6a4a",
            edgecolor="#333333", linewidth=0.25, label="self")
    ax.set_yticks(y)
    ax.set_yticklabels(names, fontfamily="monospace", fontsize=8)
    ax.set_xlabel("% of total program Ir")
    ax.set_title(
        f"Inclusive vs Self Cost - {instance_name}\n"
        "Inclusive includes callees and is not additive",
        fontsize=12,
        fontweight="bold",
    )
    ax.grid(axis="x", alpha=0.25, linestyle="--")
    ax.legend(loc="lower right")

    max_pct = max(max(incl_pcts), max(self_pcts)) if top else 1.0
    for i, (sp, ip, func) in enumerate(zip(self_pcts, incl_pcts, reversed(top))):
        self_sec = fmt_seconds(pct_to_seconds(sp, profile.native_elapsed_sec))
        incl_sec = fmt_seconds(pct_to_seconds(ip, profile.native_elapsed_sec))
        ax.text(ip + max_pct * 0.01, i - height / 2,
                f"{ip:.1f}% / {incl_sec}", va="center", fontsize=7)
        ax.text(sp + max_pct * 0.01, i + height / 2,
                f"{sp:.1f}% / {self_sec}", va="center", fontsize=7)
    ax.set_xlim(0, max_pct * 1.28 + 0.05)

    fig.tight_layout()
    fig.savefig(path, dpi=180, bbox_inches="tight")
    plt.close(fig)
    return path


def plot_metric_heatmap(profile: Profile, funcs: list[FunctionInfo],
                        out_dir: str, instance_name: str, top_n: int) -> str:
    plt = load_pyplot()
    path = os.path.join(out_dir, "03_function_metric_heatmap.png")
    top = funcs[:top_n]
    if not top:
        return path

    native = profile.native_elapsed_sec
    rows = [f.display_name(60) for f in top]
    self_ms = [
        (pct_to_seconds(f.self_pct, native) or 0.0) * 1000.0 for f in top
    ]
    incl_ms = [
        (pct_to_seconds(f.inclusive_pct, native) or 0.0) * 1000.0 for f in top
    ]
    raw = [
        [func.self_pct, func.inclusive_pct, self_ms[i], incl_ms[i]]
        for i, func in enumerate(top)
    ]
    columns = ["Self %", "Incl %", "Self ms", "Incl ms"]

    # Normalize each column independently so both percent and ms columns are readable.
    column_max = [
        max((row[col] for row in raw), default=0.0)
        for col in range(len(columns))
    ]
    normalized = [
        [
            (row[col] / column_max[col]) if column_max[col] > 0 else 0.0
            for col in range(len(columns))
        ]
        for row in raw
    ]

    fig_h = max(6.0, len(top) * 0.34)
    fig, ax = plt.subplots(figsize=(11.5, fig_h))
    im = ax.imshow(normalized, aspect="auto", cmap="YlOrRd", vmin=0, vmax=1)
    ax.set_yticks(list(range(len(rows))))
    ax.set_yticklabels(rows, fontfamily="monospace", fontsize=8)
    ax.set_xticks(list(range(len(columns))))
    ax.set_xticklabels(columns, fontsize=9)
    ax.set_title(
        f"Function Metric Heatmap - {instance_name}\n"
        "Cells are column-normalized; text shows actual values",
        fontsize=12,
        fontweight="bold",
    )

    for i, row in enumerate(raw):
        for j, value in enumerate(row):
            if j < 2:
                label = f"{value:.2f}%"
            else:
                label = f"{value:.1f}"
            color = "white" if normalized[i][j] > 0.55 else "#222222"
            ax.text(j, i, label, ha="center", va="center", fontsize=7, color=color)

    cbar = fig.colorbar(im, ax=ax, shrink=0.7)
    cbar.set_label("Column-normalized intensity")
    fig.tight_layout()
    fig.savefig(path, dpi=180, bbox_inches="tight")
    plt.close(fig)
    return path


def generate_plots(profile: Profile, funcs: list[FunctionInfo], out_dir: str,
                   instance_name: str, top_n: int) -> list[str]:
    paths: list[str] = []
    plotters = [
        lambda: plot_self_time_share(profile, funcs, out_dir, instance_name, top_n),
        lambda: plot_inclusive_time_share(profile, funcs, out_dir, instance_name, top_n),
        lambda: plot_metric_heatmap(profile, funcs, out_dir, instance_name, min(top_n, 25)),
    ]
    for plotter in plotters:
        try:
            paths.append(plotter())
        except SystemExit as exc:
            eprint(f"Plot generation skipped: {exc}")
            break
    return paths


# ---------------------------------------------------------------------------
# Main
# ---------------------------------------------------------------------------

def main() -> None:
    parser = argparse.ArgumentParser(
        description="Generate clear function time-share heatmaps for tsp_bb")
    parser.add_argument("instance", help="Path to TSP instance")
    parser.add_argument("--solver", default=DEFAULT_SOLVER,
                        help=f"Path to tsp_bb binary (default: {DEFAULT_SOLVER})")
    parser.add_argument("--out-dir",
                        help="Output directory (default: docs/heatmap/<instance>)")
    parser.add_argument("--profile", action="store_true",
                        help="Run native timing and collect callgrind data")
    parser.add_argument("--load-profile",
                        help="Load an existing profile directory")
    parser.add_argument("--timeout", type=int, default=1800,
                        help="Native solver timeout in seconds")
    parser.add_argument("--exact-max-n", type=int, default=100,
                        help="Pass --exact-max-n to solvers that support it")
    parser.add_argument("--strategy", default="smart",
                        help="Pass --branch-strategy to legacy solvers that support it")
    parser.add_argument("--top-n", type=int, default=30,
                        help="Number of functions shown in charts/tables")
    parser.add_argument("--scope", choices=["solver", "solver-runtime", "all"],
                        default="solver-runtime",
                        help="Function scope to display")
    parser.add_argument("--no-native-timing", action="store_true",
                        help="Skip native timing; report percentages without estimated seconds")
    args = parser.parse_args()

    instance_path = os.path.abspath(args.instance)
    instance_name = os.path.splitext(os.path.basename(instance_path))[0]
    out_dir = args.out_dir or os.path.join(DEFAULT_OUT_ROOT, instance_name)
    os.makedirs(out_dir, exist_ok=True)

    eprint(f"Instance: {instance_path}")
    eprint(f"Output:   {out_dir}")
    dim = read_instance_dimension(instance_path)
    if dim is not None:
        eprint(f"Dimension: {dim}")

    if args.load_profile:
        profile_dir = os.path.abspath(args.load_profile)
        eprint(f"Loading profile: {profile_dir}")
        profile = load_profile(profile_dir)
    elif args.profile:
        native_run = None
        if not args.no_native_timing:
            native_run = run_native_solver(
                instance_path, args.solver, args.exact_max_n,
                args.strategy, args.timeout)
        callgrind_elapsed = run_callgrind(
            instance_path, args.solver, args.exact_max_n,
            args.strategy, args.timeout, out_dir)
        save_metadata(out_dir, instance_path, args.solver, args.strategy,
                      args.exact_max_n, native_run, callgrind_elapsed)
        profile = load_profile(out_dir)
    else:
        raise SystemExit("Use --profile to collect data or --load-profile DIR to reuse data.")

    if not profile.functions:
        raise SystemExit("No function data found. Check callgrind_annotate output.")

    funcs = select_functions(profile, args.scope)
    if not funcs:
        raise SystemExit(f"No functions matched scope: {args.scope}")

    print_summary(profile, funcs, args.top_n)

    csv_path = write_breakdown_csv(profile, funcs, out_dir)
    report_path = write_report(profile, funcs, out_dir, instance_name,
                               args.scope, args.top_n)
    plot_paths = generate_plots(profile, funcs, out_dir, instance_name,
                                args.top_n)

    eprint("")
    eprint("Generated:")
    eprint(f"  {csv_path}")
    eprint(f"  {report_path}")
    for path in plot_paths:
        eprint(f"  {path}")


if __name__ == "__main__":
    main()
