#!/usr/bin/env python3
"""Find all TSP instances with < 20 vertices and run exact solver on them."""

import subprocess
import sys
import os
import re
from pathlib import Path

SOLVER = os.path.join(os.path.dirname(__file__), "..", "build", "tsp_bb")
SEARCH_ROOTS = [
    os.path.join(os.path.dirname(__file__), "..", "data"),
    os.path.join(os.path.dirname(__file__), "..", "examples"),
]
MAX_N = 20  # strictly less than 20


def parse_dimension_tsp(filepath: str) -> int | None:
    """Parse DIMENSION from TSPLIB header."""
    with open(filepath, "r") as f:
        for line in f:
            line = line.strip()
            if line.upper().startswith("DIMENSION"):
                parts = line.split(":")
                if len(parts) == 2:
                    return int(parts[1].strip())
                # space-separated fallback
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


def looks_like_tsp(filepath: str) -> bool:
    """Check if file looks like TSPLIB format."""
    with open(filepath, "r") as f:
        for line in f:
            upper = line.strip().upper()
            if upper.startswith("NAME") or upper.startswith("TYPE") or upper.startswith("DIMENSION"):
                return True
            if upper.startswith("NODE_COORD_SECTION") or upper.startswith("EDGE_WEIGHT_SECTION"):
                return True
            # If first non-empty line doesn't look like TSPLIB, stop
            if upper and not upper.startswith("NAME") and not upper.startswith("COMMENT"):
                break
    return False


def get_dimension(filepath: str) -> int | None:
    """Try to determine the dimension of a TSP instance."""
    # Skip .opt.tour files, batch lists, archives, etc.
    name = os.path.basename(filepath)
    if name.endswith(".opt.tour") or name.startswith("batch"):
        return None
    if name.startswith("."):
        return None
    if name.endswith(".gz"):
        return None

    # Try TSPLIB format first
    dim = parse_dimension_tsp(filepath)
    if dim is not None:
        return dim

    # Try matrix format
    dim = parse_dimension_matrix(filepath)
    if dim is not None:
        return dim

    return None


def find_small_instances():
    """Find all instances with dimension < 20."""
    instances = []
    for root in SEARCH_ROOTS:
        if not os.path.isdir(root):
            continue
        for dirpath, dirnames, filenames in os.walk(root):
            # Skip _archives
            if "_archives" in dirpath:
                continue
            for fname in sorted(filenames):
                filepath = os.path.join(dirpath, fname)
                dim = get_dimension(filepath)
                if dim is not None and dim < MAX_N:
                    instances.append((filepath, dim))
    return instances


def run_solver(instance_path: str) -> tuple[str, str, int]:
    """Run solver and return (stdout, stderr, returncode)."""
    cmd = [SOLVER, "--method", "exact", "--branch-strategy", "exhaustive", "--exact-max-n", str(MAX_N), "--debug", instance_path]
    result = subprocess.run(cmd, capture_output=True, text=True, timeout=1800)
    return result.stdout, result.stderr, result.returncode


def main():
    if not os.path.isfile(SOLVER):
        print(f"Error: solver not found at {SOLVER}", file=sys.stderr)
        sys.exit(1)

    instances = find_small_instances()
    print(f"Found {len(instances)} instances with < {MAX_N} vertices.\n", file=sys.stderr)

    output_lines = []
    output_lines.append("# TSP Solver Results — exhaustive Branching Strategy (Debug)")
    output_lines.append("")
    output_lines.append(f"**Total instances found:** {len(instances)}  ")
    output_lines.append(f"**Solver:** `tsp_bb --method exact --branch-strategy exhaustive --exact-max-n {MAX_N} --debug`  ")
    output_lines.append(f"**Max vertices:** < {MAX_N}  ")
    output_lines.append("")
    output_lines.append("---")
    output_lines.append("")

    summary_ok = 0
    summary_fail = 0
    summary_errors = []

    for idx, (path, dim) in enumerate(instances, 1):
        rel_path = os.path.relpath(path, os.path.join(os.path.dirname(__file__), ".."))
        print(f"[{idx}/{len(instances)}] Running: {rel_path} (n={dim}) ...", file=sys.stderr)

        try:
            stdout, stderr, rc = run_solver(path)
        except subprocess.TimeoutExpired:
            output_lines.append(f"## {idx}. `{rel_path}` (n={dim})")
            output_lines.append("")
            output_lines.append("**Status:** TIMEOUT (>30m)")
            output_lines.append("")
            summary_fail += 1
            summary_errors.append(f"{rel_path}: TIMEOUT")
            continue
        except Exception as e:
            output_lines.append(f"## {idx}. `{rel_path}` (n={dim})")
            output_lines.append("")
            output_lines.append(f"**Status:** ERROR — {e}")
            output_lines.append("")
            summary_fail += 1
            summary_errors.append(f"{rel_path}: {e}")
            continue

        output_lines.append(f"## {idx}. `{rel_path}` (n={dim})")
        output_lines.append("")

        # Parse result
        if "Optimal cost:" in stdout or "Heuristic cost:" in stdout or "feasible" in stdout.lower():
            # Extract key metrics
            for line in stdout.splitlines():
                if any(k in line for k in ["Problem:", "Dimension:", "Method:", "Root lower bound:",
                                             "Initial upper bound:", "Nodes created:", "Nodes expanded:",
                                             "Pruned by bound:", "Pruned infeasible:", "Optimal cost:",
                                             "Heuristic cost:", "Tour:", "No feasible"]):
                    output_lines.append(f"```")
                    output_lines.append(line)
                    output_lines.append(f"```")
                    output_lines.append("")
            summary_ok += 1
        elif rc != 0:
            output_lines.append(f"**Status:** FAILED (exit code {rc})")
            output_lines.append("")
            summary_fail += 1
            summary_errors.append(f"{rel_path}: exit code {rc}")
        else:
            output_lines.append(f"**Status:** UNKNOWN")
            output_lines.append("")
            summary_fail += 1
            summary_errors.append(f"{rel_path}: unknown result")

        # Debug output
        if stderr:
            output_lines.append("<details>")
            output_lines.append("<summary>Debug output (stderr)</summary>")
            output_lines.append("")
            output_lines.append("```")
            output_lines.append(stderr.strip())
            output_lines.append("```")
            output_lines.append("</details>")
            output_lines.append("")

    # Summary
    output_lines.insert(4, f"**Successful:** {summary_ok}  ")
    output_lines.insert(5, f"**Failed:** {summary_fail}  ")
    output_lines.insert(6, "")

    output_lines.append("---")
    output_lines.append("")
    output_lines.append("## Summary")
    output_lines.append("")
    output_lines.append(f"| Result | Count |")
    output_lines.append(f"|--------|-------|")
    output_lines.append(f"| Optimal/Has Tour | {summary_ok} |")
    output_lines.append(f"| Failed/Timeout | {summary_fail} |")
    output_lines.append(f"| **Total** | {len(instances)} |")
    output_lines.append("")

    if summary_errors:
        output_lines.append("### Failures")
        output_lines.append("")
        for err in summary_errors:
            output_lines.append(f"- `{err}`")
        output_lines.append("")

    out_path = os.path.join(os.path.dirname(__file__), "..", "docs", "exhaustive-branch-results.md")
    with open(out_path, "w") as f:
        f.write("\n".join(output_lines))
    print(f"\nResults written to: {out_path}", file=sys.stderr)


if __name__ == "__main__":
    main()
