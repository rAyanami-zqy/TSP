#!/usr/bin/env python3
"""使用独立 Held-Karp 精确算法校验求解器输出。"""

from __future__ import annotations

import argparse
import math
import re
import subprocess
import sys
from pathlib import Path


def parse_weight(token: str) -> float:
    """解析本项目矩阵格式中的边权；缺边符号转换成 infinity。"""
    lowered = token.lower()
    if lowered in {"inf", "infinity", "-", "x"}:
        return math.inf
    return float(token)


def read_matrix(path: Path) -> list[list[float]]:
    """读取本项目矩阵格式。"""
    tokens = path.read_text(encoding="utf-8").split()
    if not tokens:
        raise ValueError("empty instance")

    n = int(tokens[0])
    expected = 1 + n * n
    if len(tokens) != expected:
        raise ValueError(f"expected {expected - 1} matrix entries, got {len(tokens) - 1}")

    matrix: list[list[float]] = []
    cursor = 1
    for _ in range(n):
        row = [parse_weight(token) for token in tokens[cursor : cursor + n]]
        matrix.append(row)
        cursor += n
    return matrix


def held_karp(matrix: list[list[float]]) -> float:
    """Held-Karp 动态规划，时间复杂度 O(n^2 2^n)，适合小规模精确校验。"""
    n = len(matrix)
    if n < 3:
        return math.inf

    # 状态使用 bitmask 表示已访问的非 0 顶点集合，last 表示当前终点。
    dp: dict[tuple[int, int], float] = {}
    for v in range(1, n):
        if math.isfinite(matrix[0][v]):
            dp[(1 << (v - 1), v)] = matrix[0][v]

    full_mask = (1 << (n - 1)) - 1
    for mask in range(1, full_mask + 1):
        for last in range(1, n):
            current = dp.get((mask, last))
            if current is None:
                continue

            remaining = full_mask ^ mask
            bitset = remaining
            while bitset:
                bit = bitset & -bitset
                nxt = bit.bit_length()
                if math.isfinite(matrix[last][nxt]):
                    next_mask = mask | bit
                    key = (next_mask, nxt)
                    value = current + matrix[last][nxt]
                    if value < dp.get(key, math.inf):
                        dp[key] = value
                bitset -= bit

    best = math.inf
    for last in range(1, n):
        current = dp.get((full_mask, last), math.inf)
        if math.isfinite(current) and math.isfinite(matrix[last][0]):
            best = min(best, current + matrix[last][0])
    return best


def read_batch_list(path: Path) -> list[Path]:
    """读取批处理清单，忽略空行和 # 注释行。"""
    result: list[Path] = []
    for raw_line in path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if not line or line.startswith("#"):
            continue
        result.append(Path(line))
    return result


def run_solver(solver: Path, instance: Path, timeout: float) -> tuple[bool, float, str]:
    """调用 C++ 求解器，并从人类可读输出中提取最优值。"""
    completed = subprocess.run(
        [str(solver), str(instance)],
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=timeout,
    )

    output = completed.stdout + completed.stderr
    match = re.search(r"Optimal cost:\s*([0-9.+\-eE]+)", output)
    if match:
        return True, float(match.group(1)), output

    if "No feasible Hamiltonian tour found" in output:
        return False, math.inf, output

    raise RuntimeError(f"solver failed for {instance}:\n{output.strip()}")


def collect_instances(args: argparse.Namespace) -> list[Path]:
    """合并命令行直接给出的实例和 batch-list 中的实例。"""
    instances = [Path(value) for value in args.instances]
    for list_file in args.batch_list:
        instances.extend(read_batch_list(Path(list_file)))
    return instances


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("instances", nargs="*", help="实例文件")
    parser.add_argument("--batch-list", action="append", default=[], help="批处理清单文件")
    parser.add_argument("--solver", default="./build/tsp_bb", help="求解器可执行文件路径")
    parser.add_argument("--max-n", type=int, default=12, help="Held-Karp 校验的最大顶点数")
    parser.add_argument("--tolerance", type=float, default=1e-6, help="最优值比较容差")
    parser.add_argument("--timeout", type=float, default=30.0, help="单实例求解超时秒数")
    args = parser.parse_args()

    solver = Path(args.solver)
    instances = collect_instances(args)
    if not instances:
        parser.error("provide at least one instance or --batch-list")

    ok_count = 0
    skip_count = 0
    fail_count = 0

    for instance in instances:
        try:
            matrix = read_matrix(instance)
            n = len(matrix)
            # Held-Karp 是指数级算法，超过阈值时只跳过验证，不代表求解器不能运行。
            if n > args.max_n:
                print(f"[SKIP] {instance} n={n} exceeds --max-n={args.max_n}")
                skip_count += 1
                continue

            exact = held_karp(matrix)
            solver_feasible, solver_cost, _ = run_solver(solver, instance, args.timeout)

            if math.isinf(exact):
                matched = not solver_feasible
            else:
                matched = solver_feasible and abs(solver_cost - exact) <= args.tolerance

            if matched:
                exact_text = "infeasible" if math.isinf(exact) else f"{exact:.10g}"
                solver_text = "infeasible" if not solver_feasible else f"{solver_cost:.10g}"
                print(f"[OK]   {instance} n={n} solver={solver_text} exact={exact_text}")
                ok_count += 1
            else:
                exact_text = "infeasible" if math.isinf(exact) else f"{exact:.10g}"
                solver_text = "infeasible" if not solver_feasible else f"{solver_cost:.10g}"
                print(f"[FAIL] {instance} n={n} solver={solver_text} exact={exact_text}")
                fail_count += 1
        except Exception as ex:
            print(f"[ERROR] {instance}: {ex}", file=sys.stderr)
            fail_count += 1

    print(f"Summary: ok={ok_count}, skipped={skip_count}, failed={fail_count}")
    return 0 if fail_count == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
