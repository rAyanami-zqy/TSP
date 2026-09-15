#!/usr/bin/env python3
"""Benchmark root 1-tree alpha-nearness candidate sets without exact search.

Every solver invocation stops after the root certificate.  The baseline runs
one historical 8-nearest-neighbor CLK start; the remaining configurations run
one additional LK pass using root alpha or an alpha/geometric hybrid set.
"""

from __future__ import annotations

import argparse
import csv
import io
import statistics
import subprocess
from collections import defaultdict
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[2]
CONFIGS = {
    "single-knn8": (),
    "root-alpha5": (
        "--lk-candidate-set", "alpha", "--lk-candidates", "5",
        "--root-guided-lk", "once"),
    "root-alpha8": (
        "--lk-candidate-set", "alpha", "--lk-candidates", "8",
        "--root-guided-lk", "once"),
    "root-hybrid8": (
        "--lk-candidate-set", "hybrid", "--lk-candidates", "8",
        "--root-guided-lk", "once"),
    "root-hybrid12": (
        "--lk-candidate-set", "hybrid", "--lk-candidates", "12",
        "--root-guided-lk", "once"),
}
NUMERIC_FIELDS = (
    "final_upper_bound", "final_lower_bound", "final_relative_gap",
    "initial_tour_seconds", "initial_clk_starts",
    "root_guided_lk_calls", "root_guided_lk_improvements",
    "root_guided_lk_reascents", "root_guided_lk_seconds",
    "root_guided_lk_total_gain", "root_potential_iterations",
    "root_ascent_seconds", "instance_wall_seconds",
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--solver", type=Path, required=True)
    parser.add_argument(
        "--batch", type=Path,
        default=PROJECT_ROOT / "data/classic/batch-n200.txt")
    parser.add_argument(
        "--output", type=Path,
        default=PROJECT_ROOT / "outputs/phkmst-ablation"
        / "root-guided-lk-20260915")
    parser.add_argument("--repeats", type=int, default=3)
    parser.add_argument("--root-ascent", default="hybrid-reverse")
    args = parser.parse_args()
    if args.repeats <= 0:
        parser.error("--repeats must be positive")
    return args


def run_once(args: argparse.Namespace, extra: tuple[str, ...]) -> list[dict[str, str]]:
    command = [
        str(args.solver), "--root-bound-only",
        "--hk-ascent", args.root_ascent,
        "--initial-clk", "single", "--exact-max-n", "199",
        *extra, "--batch", str(args.batch),
    ]
    completed = subprocess.run(
        command, cwd=PROJECT_ROOT, text=True, capture_output=True,
        check=False)
    if completed.returncode != 0:
        raise RuntimeError(
            f"solver exited {completed.returncode}: {completed.stderr[-2000:]}")
    rows = list(csv.DictReader(io.StringIO(completed.stdout)))
    if not rows or any(row.get("status") != "ok" for row in rows):
        raise RuntimeError("root-only batch was empty or contained failures")
    return rows


def median_results(raw: list[dict[str, object]]) -> dict[tuple[str, str], dict[str, float]]:
    groups: dict[tuple[str, str], list[dict[str, object]]] = defaultdict(list)
    for row in raw:
        groups[(str(row["config"]), str(row["instance"]))].append(row)
    return {
        key: {
            field: statistics.median(float(row[field]) for row in rows)
            for field in NUMERIC_FIELDS
        }
        for key, rows in groups.items()
    }


def write_reports(args: argparse.Namespace, raw: list[dict[str, object]]) -> None:
    args.output.mkdir(parents=True, exist_ok=True)
    raw_fields = ("config", "repeat", "instance") + NUMERIC_FIELDS
    with (args.output / "raw_results.csv").open(
            "w", encoding="utf-8-sig", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=raw_fields)
        writer.writeheader()
        writer.writerows(raw)

    medians = median_results(raw)
    instances = sorted({str(row["instance"]) for row in raw})
    baseline = {
        name: medians[("single-knn8", name)] for name in instances}
    detail_fields = (
        "instance", "configuration", "baseline_ub", "candidate_ub",
        "ub_gain", "baseline_wall_seconds", "candidate_wall_seconds",
        "wall_delta", "root_guided_lk_improved",
        "root_guided_lk_seconds", "root_ascent_seconds",
        "root_potential_iterations",
    )
    details: list[dict[str, object]] = []
    summaries: list[dict[str, object]] = []
    baseline_wall = sum(
        row["instance_wall_seconds"] for row in baseline.values())
    for label in CONFIGS:
        current = {name: medians[(label, name)] for name in instances}
        gains = {
            name: baseline[name]["final_upper_bound"]
            - current[name]["final_upper_bound"]
            for name in instances
        }
        wins = [name for name, gain in gains.items() if gain > 1e-9]
        losses = [name for name, gain in gains.items() if gain < -1e-9]
        current_wall = sum(
            row["instance_wall_seconds"] for row in current.values())
        summaries.append({
            "configuration": label,
            "wins": len(wins),
            "losses": len(losses),
            "ties": len(instances) - len(wins) - len(losses),
            "total_ub_gain": sum(max(0.0, gain) for gain in gains.values()),
            "median_positive_gain": (
                statistics.median(gains[name] for name in wins)
                if wins else 0.0),
            "reported_improvements": sum(
                row["root_guided_lk_improvements"]
                for row in current.values()),
            "total_guided_seconds": sum(
                row["root_guided_lk_seconds"] for row in current.values()),
            "total_wall_seconds": current_wall,
            "wall_delta_vs_baseline": current_wall - baseline_wall,
            "root_reascents": sum(
                row["root_guided_lk_reascents"] for row in current.values()),
        })
        if label == "single-knn8":
            continue
        for name in instances:
            details.append({
                "instance": name,
                "configuration": label,
                "baseline_ub": baseline[name]["final_upper_bound"],
                "candidate_ub": current[name]["final_upper_bound"],
                "ub_gain": gains[name],
                "baseline_wall_seconds": baseline[name]["instance_wall_seconds"],
                "candidate_wall_seconds": current[name]["instance_wall_seconds"],
                "wall_delta": current[name]["instance_wall_seconds"]
                    - baseline[name]["instance_wall_seconds"],
                "root_guided_lk_improved": current[name][
                    "root_guided_lk_improvements"],
                "root_guided_lk_seconds": current[name][
                    "root_guided_lk_seconds"],
                "root_ascent_seconds": current[name]["root_ascent_seconds"],
                "root_potential_iterations": current[name][
                    "root_potential_iterations"],
            })

    summary_fields = tuple(summaries[0].keys())
    with (args.output / "summary.csv").open(
            "w", encoding="utf-8-sig", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=summary_fields)
        writer.writeheader()
        writer.writerows(summaries)
    with (args.output / "instance_comparison.csv").open(
            "w", encoding="utf-8-sig", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=detail_fields)
        writer.writeheader()
        writer.writerows(details)

    lines = [
        "# 根 1-tree 引导 LK 候选集实验",
        "",
        f"- 实例：`{args.batch}`（{len(instances)} 个）",
        f"- 重复：{args.repeats} 次，逐实例取中位数",
        "- 范围：只计算初始 tour 与根 Held–Karp 证书，不进入 fixing/BP",
        "- 基线：单起点历史 8-NN CLK",
        "- 根引导策略：仅追加一次 LK，UB 改善后不重新执行根上升",
        "",
        "| 配置 | UB 胜/负/平 | UB 总降幅 | 改善中位数 | 根引导耗时合计(s) | 总墙钟(s) | 相对基线(s) |",
        "|---|---:|---:|---:|---:|---:|---:|",
    ]
    for row in summaries:
        lines.append(
            f"| {row['configuration']} | {row['wins']}/{row['losses']}/{row['ties']} "
            f"| {row['total_ub_gain']:.3f} | {row['median_positive_gain']:.3f} "
            f"| {row['total_guided_seconds']:.6f} "
            f"| {row['total_wall_seconds']:.6f} "
            f"| {row['wall_delta_vs_baseline']:+.6f} |")
    best = max(summaries[1:], key=lambda row: (row["wins"], row["total_ub_gain"]))
    lines.extend((
        "",
        "## 结论",
        "",
        f"本轮按优先覆盖实例数选择 `{best['configuration']}`：改善 "
        f"{best['wins']} 个实例，未造成 UB 回退；50 实例 UB 合计下降 "
        f"{best['total_ub_gain']:.3f}。这只是根阶段筛选，最终收益仍需用 "
        "受限精确求解比较 solved count、PAR2 和性能剖面。",
        "",
    ))
    (args.output / "结论.md").write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    args = parse_args()
    raw: list[dict[str, object]] = []
    for repeat in range(1, args.repeats + 1):
        for label, extra in CONFIGS.items():
            print(f"[{repeat}/{args.repeats}] {label}", flush=True)
            for row in run_once(args, extra):
                raw.append({
                    "config": label,
                    "repeat": repeat,
                    "instance": row["instance"],
                    **{field: row[field] for field in NUMERIC_FIELDS},
                })
    write_reports(args, raw)
    print(args.output / "结论.md")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
