#!/usr/bin/env python3
"""生成随机对称 TSP 距离矩阵实例。"""

from __future__ import annotations

import argparse
import math
import random
from pathlib import Path


def build_complete(rng: random.Random, n: int, min_weight: int, max_weight: int) -> list[list[float]]:
    """生成完整图：任意两个不同顶点之间都有一条边。"""
    matrix: list[list[float]] = [[0.0 for _ in range(n)] for _ in range(n)]
    for i in range(n):
        for j in range(i + 1, n):
            weight = float(rng.randint(min_weight, max_weight))
            matrix[i][j] = weight
            matrix[j][i] = weight
    return matrix


def build_sparse_hamiltonian(
    rng: random.Random,
    n: int,
    min_weight: int,
    max_weight: int,
    density: float,
) -> list[list[float]]:
    """生成稀疏图，并先随机嵌入一条 Hamilton 回路以保证实例可行。"""
    matrix: list[list[float]] = [[math.inf for _ in range(n)] for _ in range(n)]
    for i in range(n):
        matrix[i][i] = 0.0

    cycle = list(range(n))
    rng.shuffle(cycle)
    cycle_edges: set[tuple[int, int]] = set()
    # 先固定一条随机环，避免随机删边后没有任何可行 TSP 回路。
    for idx, u in enumerate(cycle):
        v = cycle[(idx + 1) % n]
        a, b = sorted((u, v))
        cycle_edges.add((a, b))

    for i in range(n):
        for j in range(i + 1, n):
            # 环上的边必保留；非环边按 density 概率额外加入。
            if (i, j) in cycle_edges or rng.random() <= density:
                weight = float(rng.randint(min_weight, max_weight))
                matrix[i][j] = weight
                matrix[j][i] = weight
    return matrix


def write_instance(path: Path, matrix: list[list[float]]) -> None:
    """写出本项目矩阵格式，缺边用 x 表示。"""
    with path.open("w", encoding="utf-8") as output:
        output.write(f"{len(matrix)}\n")
        for row in matrix:
            tokens = []
            for value in row:
                if math.isinf(value):
                    tokens.append("x")
                else:
                    tokens.append(str(int(value)))
            output.write(" ".join(tokens) + "\n")


def display_path(path: Path) -> str:
    """输出尽量使用相对路径，让 batch 清单更适合放进仓库。"""
    try:
        return str(path.relative_to(Path.cwd()))
    except ValueError:
        return str(path)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--output", default="examples/random", help="输出目录")
    parser.add_argument("--count", type=int, default=12, help="实例数量")
    parser.add_argument("--min-n", type=int, default=4, help="最小顶点数")
    parser.add_argument("--max-n", type=int, default=8, help="最大顶点数")
    parser.add_argument("--min-weight", type=int, default=1, help="最小边权")
    parser.add_argument("--max-weight", type=int, default=50, help="最大边权")
    parser.add_argument("--seed", type=int, default=20260508, help="随机种子")
    parser.add_argument("--prefix", default="random", help="输出文件名前缀")
    parser.add_argument(
        "--sparse-density",
        type=float,
        default=None,
        help="稀疏图额外边概率；不提供该参数时生成完整图",
    )
    parser.add_argument(
        "--list-file",
        default=None,
        help="批处理清单路径；默认写到 <output>/batch.txt",
    )
    args = parser.parse_args()

    if args.count <= 0:
        raise ValueError("--count must be positive")
    if args.min_n < 3 or args.max_n < args.min_n:
        raise ValueError("vertex count range must satisfy 3 <= min-n <= max-n")
    if args.min_weight <= 0 or args.max_weight < args.min_weight:
        raise ValueError("weight range must satisfy 0 < min-weight <= max-weight")
    if args.sparse_density is not None and not (0.0 <= args.sparse_density <= 1.0):
        raise ValueError("--sparse-density must be in [0, 1]")

    rng = random.Random(args.seed)
    output_dir = Path(args.output)
    output_dir.mkdir(parents=True, exist_ok=True)

    paths: list[Path] = []
    for idx in range(1, args.count + 1):
        n = rng.randint(args.min_n, args.max_n)
        # sparse_density 是否为空决定生成完整图还是稀疏图。
        if args.sparse_density is None:
            matrix = build_complete(rng, n, args.min_weight, args.max_weight)
            kind = "complete"
        else:
            matrix = build_sparse_hamiltonian(
                rng, n, args.min_weight, args.max_weight, args.sparse_density
            )
            kind = "sparse"

        path = output_dir / f"{args.prefix}-{idx:02d}-{kind}-n{n}.txt"
        write_instance(path, matrix)
        paths.append(path)

    list_path = Path(args.list_file) if args.list_file else output_dir / "batch.txt"
    list_path.parent.mkdir(parents=True, exist_ok=True)
    with list_path.open("w", encoding="utf-8") as output:
        output.write("# 随机生成的 TSP 实例清单\n")
        for path in paths:
            output.write(display_path(path) + "\n")

    print(f"Generated {len(paths)} instances")
    print(f"Batch list: {display_path(list_path)}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
