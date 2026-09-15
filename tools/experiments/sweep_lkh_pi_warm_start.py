#!/usr/bin/env python3
"""Sweep small local-ascent budgets from LKH PI_FILE warm starts."""

from __future__ import annotations

import argparse
import csv
import re
import statistics
import subprocess
from collections import defaultdict
from pathlib import Path

from benchmark_lkh_pi_reuse import (
    ROOT, dimension, instances,
)


CONFIGS = (
    ("replace", "hybrid-reverse", 0),
    ("polyak8", "polyak", 8),
    ("polyak16", "polyak", 16),
    ("polyak32", "polyak", 32),
    ("polyak64", "polyak", 64),
    ("polyak128", "polyak", 128),
    ("polyak-smoothed32", "polyak-smoothed", 32),
    ("polyak-smoothed64", "polyak-smoothed", 64),
    ("polyak-smoothed128", "polyak-smoothed", 128),
    ("polyak-smoothed-dynamic32", "polyak-smoothed-dynamic", 32),
    ("polyak-smoothed-dynamic64", "polyak-smoothed-dynamic", 64),
    ("polyak-smoothed-dynamic128", "polyak-smoothed-dynamic", 128),
    ("helsgaun16", "helsgaun", 16),
    ("helsgaun32", "helsgaun", 32),
    ("helsgaun64", "helsgaun", 64),
    ("hybrid-reverse16", "hybrid-reverse", 16),
    ("hybrid-reverse32", "hybrid-reverse", 32),
    ("hybrid-reverse64", "hybrid-reverse", 64),
    ("local-full", "hybrid-reverse", 400),
)
FIELDS = (
    "lower_bound", "upper_bound", "instance_wall_seconds",
    "root_ascent_seconds", "root_potential_iterations",
)


def args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--solver", type=Path, required=True)
    parser.add_argument(
        "--worker", type=Path, required=True,
        help="CMake-built isolated tsp_lkh_provider executable")
    parser.add_argument(
        "--batch", type=Path, default=ROOT / "data/classic/batch-n200.txt")
    parser.add_argument(
        "--output", type=Path,
        default=ROOT / "outputs/phkmst-ablation_914_ratio"
        / "lkh-pi-warm-sweep-20260915")
    parser.add_argument("--repeats", type=int, default=3)
    result = parser.parse_args()
    if result.repeats <= 0:
        parser.error("--repeats must be positive")
    return result


def field(output: str, label: str) -> float:
    match = re.search(rf"(?m)^{re.escape(label)}:\s*([^\s]+)\s*$", output)
    if match is None:
        raise RuntimeError(f"missing field {label}")
    return float(match.group(1))


def run_solver(
    solver: Path, provider: Path, instance: Path, repeat: int,
    strategy: str, iterations: int,
) -> dict[str, float]:
    command = [
        str(solver), "--root-bound-only", "--exact-max-n", "199",
        "--lkh-provider", str(provider), "--lkh-provider-failure", "error",
        "--lkh-runs", "1", "--lkh-max-trials", str(dimension(instance)),
        "--lkh-seed", str(repeat),
    ]
    if iterations == 0:
        command.extend(("--lkh-pi-mode", "replace"))
    elif iterations == 400:
        # Reference: same LKH tour, but original local ascent from zero.
        command.extend((
            "--lkh-pi-mode", "off", "--hk-ascent", strategy))
    else:
        command.extend((
            "--lkh-pi-mode", "warm-start",
            "--root-pi-refine-ascent", strategy,
            "--root-pi-refine-iterations", str(iterations)))
    command.append(str(instance))
    completed = subprocess.run(
        command, cwd=ROOT, text=True, capture_output=True, check=False)
    if completed.returncode:
        raise RuntimeError(completed.stderr[-2000:])
    return {
        "lower_bound": field(completed.stdout, "Final lower bound"),
        "upper_bound": field(completed.stdout, "Final upper bound"),
        "instance_wall_seconds": field(
            completed.stdout, "Instance wall seconds"),
        "root_ascent_seconds": field(completed.stdout, "Root ascent seconds"),
        "root_potential_iterations": field(
            completed.stdout, "Root potential iterations"),
    }


def write(args: argparse.Namespace, raw: list[dict[str, object]]) -> None:
    args.output.mkdir(parents=True, exist_ok=True)
    columns = ("repeat", "configuration", "instance") + FIELDS
    with (args.output / "raw_results.csv").open(
            "w", encoding="utf-8-sig", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=columns)
        writer.writeheader()
        writer.writerows(raw)

    groups: dict[tuple[str, str], list[dict[str, object]]] = defaultdict(list)
    for row in raw:
        groups[(str(row["configuration"]), str(row["instance"]))].append(row)
    medians = {key: {
        name: statistics.median(float(row[name]) for row in rows)
        for name in FIELDS
    } for key, rows in groups.items()}
    names = sorted({str(row["instance"]) for row in raw})
    replacement = {name: medians[("replace", name)] for name in names}
    local = {name: medians[("local-full", name)] for name in names}
    summary = []
    for config, _, _ in CONFIGS:
        current = {name: medians[(config, name)] for name in names}
        gains = [current[name]["lower_bound"] - replacement[name]["lower_bound"]
                 for name in names]
        local_delta = [current[name]["lower_bound"] - local[name]["lower_bound"]
                       for name in names]
        summary.append({
            "configuration": config,
            "improved_vs_replace": sum(value > 1e-6 for value in gains),
            "worse_vs_replace": sum(value < -1e-6 for value in gains),
            "total_lb_gain_vs_replace": sum(gains),
            "stronger_equal_weaker_vs_local": (
                f"{sum(value > 1e-6 for value in local_delta)}/"
                f"{sum(abs(value) <= 1e-6 for value in local_delta)}/"
                f"{sum(value < -1e-6 for value in local_delta)}"),
            "total_wall_seconds": sum(
                row["instance_wall_seconds"] for row in current.values()),
            "root_ascent_seconds": sum(
                row["root_ascent_seconds"] for row in current.values()),
            "root_iterations": sum(
                row["root_potential_iterations"] for row in current.values()),
            "max_lb_loss_vs_local": max(-value for value in local_delta),
        })
    with (args.output / "summary.csv").open(
            "w", encoding="utf-8-sig", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=tuple(summary[0].keys()))
        writer.writeheader()
        writer.writerows(summary)

    lines = [
        "# LKH PI warm-start 小预算扫描", "",
        f"{len(names)} 实例，{args.repeats} 次重复取逐实例中位数；"
        "每个配置均通过隔离式 provider 独立调用 LKH，墙钟包含该调用；"
        "根上升列只统计本地精修。",
        "", "| 配置 | 改善/变差 vs replace | LB总增益 | 强/平/弱 vs 本地完整 | 总时间(s) | 上升(s) | 迭代 | 最大LB损失 |",
        "|---|---:|---:|---:|---:|---:|---:|---:|",
    ]
    for row in summary:
        lines.append(
            f"| {row['configuration']} "
            f"| {row['improved_vs_replace']}/{row['worse_vs_replace']} "
            f"| {row['total_lb_gain_vs_replace']:.6f} "
            f"| {row['stronger_equal_weaker_vs_local']} "
            f"| {row['total_wall_seconds']:.6f} "
            f"| {row['root_ascent_seconds']:.6f} "
            f"| {row['root_iterations']:.0f} "
            f"| {row['max_lb_loss_vs_local']:.6f} |")
    selected = next(row for row in summary
                    if row["configuration"] == "polyak64")
    smoothed = next(row for row in summary
                    if row["configuration"] == "polyak-smoothed64")
    dynamic = next(row for row in summary
                   if row["configuration"] == "polyak-smoothed-dynamic64")
    replacement_row = next(row for row in summary
                           if row["configuration"] == "replace")
    lines.extend((
        "", "## 采用配置", "",
        "默认采用 `warm-start + polyak64`：外部 PI 先提供合法根下界，"
        "本地最多执行 64 次 Polyak 评估，并始终保留初始 PI 证书作为兜底。",
        "",
        f"在本批次中，它对 {selected['improved_vs_replace']} 个实例提高下界，"
        f"累计提高 {selected['total_lb_gain_vs_replace']:.6f}；相对 `replace` "
        f"增加根上升时间 "
        f"{selected['root_ascent_seconds'] - replacement_row['root_ascent_seconds']:.6f} 秒。",
        "更高预算 `polyak128` 留作强下界消融，不作为默认值。",
        "",
        f"固定平滑 64 改善 {smoothed['improved_vs_replace']} 个实例、累计增益 "
        f"{smoothed['total_lb_gain_vs_replace']:.6f}、最大本地下界损失 "
        f"{smoothed['max_lb_loss_vs_local']:.6f}；动态平滑 64 分别为 "
        f"{dynamic['improved_vs_replace']}、"
        f"{dynamic['total_lb_gain_vs_replace']:.6f}、"
        f"{dynamic['max_lb_loss_vs_local']:.6f}。它们与普通 Polyak 互不支配，"
        "因此作为参数化消融保留，不替换默认配置。",
    ))
    (args.output / "结论.md").write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    options = args()
    paths = instances(options.batch)
    raw: list[dict[str, object]] = []
    for repeat in range(1, options.repeats + 1):
        for index, instance in enumerate(paths, 1):
            if index == 1 or index % 10 == 0:
                print(
                    f"[{repeat}/{options.repeats}] {index}/{len(paths)}",
                    flush=True)
            expected_ub = None
            for config, strategy, iterations in CONFIGS:
                result = run_solver(
                    options.solver, options.worker, instance, repeat,
                    strategy, iterations)
                if expected_ub is None:
                    expected_ub = result["upper_bound"]
                elif result["upper_bound"] != expected_ub:
                    raise RuntimeError(
                        f"warm-start changed incumbent: {instance}")
                raw.append({
                    "repeat": repeat, "configuration": config,
                    "instance": str(instance), **result,
                })
    write(options, raw)
    print(options.output / "结论.md")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
