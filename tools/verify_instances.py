#!/usr/bin/env python3
"""Use an independent Held-Karp solver to verify exact tsp_bb results."""

from __future__ import annotations

import argparse
import math
import re
import subprocess
import sys
from pathlib import Path


MISSING_EDGE_TOKENS = {"inf", "infinity", "-", "x"}


def parse_weight(token: str) -> float:
    if token.lower() in MISSING_EDGE_TOKENS:
        return math.inf
    return float(token)


def read_matrix(path: Path) -> list[list[float]]:
    tokens = path.read_text(encoding="utf-8").split()
    if not tokens:
        raise ValueError("empty instance")

    n = int(tokens[0])
    if n < 3:
        raise ValueError("an instance must contain at least 3 vertices")
    expected = 1 + n * n
    if len(tokens) != expected:
        raise ValueError(
            f"expected {expected - 1} matrix entries, got {len(tokens) - 1}"
        )

    matrix: list[list[float]] = []
    cursor = 1
    for _ in range(n):
        matrix.append([parse_weight(value) for value in tokens[cursor : cursor + n]])
        cursor += n
    return matrix


def held_karp(matrix: list[list[float]]) -> float:
    """Return the exact optimum in O(n^2 * 2^n) time."""
    n = len(matrix)
    full_mask = (1 << (n - 1)) - 1
    dp: dict[tuple[int, int], float] = {}

    for vertex in range(1, n):
        weight = matrix[0][vertex]
        if math.isfinite(weight):
            dp[(1 << (vertex - 1), vertex)] = weight

    for mask in range(1, full_mask + 1):
        for last in range(1, n):
            current = dp.get((mask, last))
            if current is None:
                continue

            remaining = full_mask ^ mask
            while remaining:
                bit = remaining & -remaining
                next_vertex = bit.bit_length()
                edge = matrix[last][next_vertex]
                if math.isfinite(edge):
                    key = (mask | bit, next_vertex)
                    candidate = current + edge
                    if candidate < dp.get(key, math.inf):
                        dp[key] = candidate
                remaining -= bit

    optimum = math.inf
    for last in range(1, n):
        prefix = dp.get((full_mask, last), math.inf)
        closing = matrix[last][0]
        if math.isfinite(prefix) and math.isfinite(closing):
            optimum = min(optimum, prefix + closing)
    return optimum


def read_batch_list(path: Path) -> list[Path]:
    instances: list[Path] = []
    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if line and not line.startswith("#"):
            instances.append(Path(line))
    return instances


def run_solver(
    solver: Path, instance: Path, dimension: int, timeout: float
) -> tuple[bool, float, str]:
    completed = subprocess.run(
        [
            str(solver),
            "--exact-max-n",
            str(dimension),
            str(instance),
        ],
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=timeout,
    )
    output = completed.stdout + completed.stderr

    match = re.search(r"Optimal cost:\s*([0-9.+\-eE]+)", output)
    if match and completed.returncode == 0:
        return True, float(match.group(1)), output
    if "No feasible Hamiltonian tour found" in output and completed.returncode == 1:
        return False, math.inf, output
    raise RuntimeError(
        f"solver exited with status {completed.returncode} for {instance}:\n"
        f"{output.strip()}"
    )


def costs_match(
    actual: float,
    expected: float,
    relative_tolerance: float,
    absolute_tolerance: float,
) -> bool:
    if actual == expected:
        return True
    scale = max(abs(actual), abs(expected))
    tolerance = max(absolute_tolerance, relative_tolerance * scale)
    return abs(actual - expected) <= tolerance


def collect_instances(args: argparse.Namespace) -> list[Path]:
    instances = [Path(value) for value in args.instances]
    for list_file in args.batch_list:
        instances.extend(read_batch_list(Path(list_file)))
    return instances


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("instances", nargs="*", help="plain matrix instance files")
    parser.add_argument(
        "--batch-list", action="append", default=[], help="batch list to verify"
    )
    parser.add_argument("--solver", default="./build/tsp_bb", help="solver executable")
    parser.add_argument(
        "--max-n", type=int, default=12, help="largest dimension verified by Held-Karp"
    )
    parser.add_argument(
        "--relative-tolerance", type=float, default=1e-9, help="relative cost tolerance"
    )
    parser.add_argument(
        "--absolute-tolerance", type=float, default=0.0, help="absolute cost tolerance"
    )
    parser.add_argument("--timeout", type=float, default=30.0, help="seconds per instance")
    args = parser.parse_args()

    if args.max_n < 3:
        parser.error("--max-n must be at least 3")
    if args.relative_tolerance < 0.0 or args.absolute_tolerance < 0.0:
        parser.error("tolerances must be non-negative")
    if args.timeout <= 0.0:
        parser.error("--timeout must be positive")

    solver = Path(args.solver)
    if not solver.is_file():
        parser.error(f"solver does not exist: {solver}")

    instances = collect_instances(args)
    if not instances:
        parser.error("provide at least one instance or --batch-list")

    ok_count = 0
    skipped_count = 0
    failed_count = 0

    for instance in instances:
        try:
            matrix = read_matrix(instance)
            dimension = len(matrix)
            if dimension > args.max_n:
                print(f"[SKIP] {instance} n={dimension} exceeds --max-n={args.max_n}")
                skipped_count += 1
                continue

            expected = held_karp(matrix)
            feasible, actual, _ = run_solver(
                solver, instance, dimension, args.timeout
            )
            if math.isinf(expected):
                matched = not feasible
            else:
                matched = feasible and costs_match(
                    actual,
                    expected,
                    args.relative_tolerance,
                    args.absolute_tolerance,
                )

            expected_text = "infeasible" if math.isinf(expected) else f"{expected:.17g}"
            actual_text = "infeasible" if not feasible else f"{actual:.17g}"
            if matched:
                print(
                    f"[OK]   {instance} n={dimension} "
                    f"solver={actual_text} exact={expected_text}"
                )
                ok_count += 1
            else:
                print(
                    f"[FAIL] {instance} n={dimension} "
                    f"solver={actual_text} exact={expected_text}"
                )
                failed_count += 1
        except Exception as error:
            print(f"[ERROR] {instance}: {error}", file=sys.stderr)
            failed_count += 1

    print(
        f"Summary: ok={ok_count}, skipped={skipped_count}, failed={failed_count}"
    )
    return 0 if failed_count == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
