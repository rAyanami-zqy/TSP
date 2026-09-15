#!/usr/bin/env python3
"""Root-only sweep for the adaptive initial-CLK gap threshold.

The experiment deliberately stops after the first/root-restarted Held-Karp
certificate.  It therefore measures initial-tour/CLK cost and the adaptive
trigger decision without entering root fixing or the exact BP search.
"""

from __future__ import annotations

import argparse
import csv
import io
import math
import statistics
import subprocess
from collections import defaultdict
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[2]
SELECTED_FIELDS = (
    "instance",
    "status",
    "cost",
    "root_lower_bound",
    "initial_upper_bound",
    "final_upper_bound",
    "final_lower_bound",
    "final_relative_gap",
    "initial_tour_seconds",
    "initial_clk_starts",
    "adaptive_clk_triggers",
    "adaptive_clk_improvements",
    "root_potential_iterations",
    "root_ascent_seconds",
    "instance_wall_seconds",
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--solver", type=Path,
        default=Path("/private/tmp/tsp-cphkmst-clk-gap-build/tsp_bb"))
    parser.add_argument(
        "--batch", type=Path,
        default=PROJECT_ROOT / "data/classic/batch-n200.txt")
    parser.add_argument(
        "--output", type=Path,
        default=PROJECT_ROOT
        / "outputs/phkmst-ablation/clk-gap-root-only-20260914")
    parser.add_argument("--repeats", type=int, default=3)
    parser.add_argument(
        "--gap-ratios", nargs="+", type=float,
        default=(0.005, 0.01, 0.015, 0.02, 0.03, 0.05))
    parser.add_argument("--root-ascent", default="hybrid-reverse")
    args = parser.parse_args()
    if args.repeats <= 0:
        parser.error("--repeats must be positive")
    if not args.gap_ratios or any(
            not math.isfinite(value) or value < 0
            for value in args.gap_ratios):
        parser.error("gap ratios must be finite and non-negative")
    return args


def config_label(mode: str, ratio: float | None) -> str:
    return mode if ratio is None else f"adaptive-{ratio:g}"


def run_batch(
    solver: Path,
    batch: Path,
    root_ascent: str,
    mode: str,
    ratio: float | None,
) -> list[dict[str, str]]:
    command = [
        str(solver),
        "--root-bound-only",
        "--hk-ascent", root_ascent,
        "--initial-clk", mode,
        "--adaptive-clk-additional-starts", "2",
        "--exact-max-n", "199",
    ]
    if ratio is not None:
        command.extend(("--adaptive-clk-gap-ratio", f"{ratio:.17g}"))
    command.extend(("--batch", str(batch)))
    completed = subprocess.run(
        command,
        cwd=PROJECT_ROOT,
        text=True,
        capture_output=True,
        check=False,
    )
    if completed.returncode != 0:
        raise RuntimeError(
            f"solver exited {completed.returncode}: {completed.stderr[-2000:]}")
    rows = list(csv.DictReader(io.StringIO(completed.stdout)))
    if len(rows) != 50:
        raise RuntimeError(f"expected 50 instances, received {len(rows)}")
    failed = [row for row in rows if row.get("status") != "ok"]
    if failed:
        raise RuntimeError(f"root-only run contains failures: {failed[:3]}")
    return rows


def numeric(row: dict[str, str], field: str) -> float:
    return float(row[field])


def median_rows(
    raw_rows: list[dict[str, str]],
) -> dict[tuple[str, str], dict[str, float]]:
    groups: dict[tuple[str, str], list[dict[str, str]]] = defaultdict(list)
    for row in raw_rows:
        groups[(row["config"], row["instance_name"])].append(row)

    result: dict[tuple[str, str], dict[str, float]] = {}
    numeric_fields = SELECTED_FIELDS[2:]
    for key, rows in groups.items():
        result[key] = {
            field: statistics.median(numeric(row, field) for row in rows)
            for field in numeric_fields
        }
    return result


def materially_slower(candidate: float, baseline: float) -> bool:
    return candidate - baseline > 0.001 and candidate > baseline * 1.10


def format_seconds(value: float) -> str:
    return f"{value:.6f}"


def write_outputs(
    output: Path,
    raw_rows: list[dict[str, str]],
    ratios: list[float],
) -> None:
    output.mkdir(parents=True, exist_ok=True)
    raw_fields = ("config", "mode", "gap_ratio", "repeat", "instance_name") \
        + SELECTED_FIELDS
    with (output / "raw_results.csv").open(
            "w", encoding="utf-8-sig", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=raw_fields)
        writer.writeheader()
        writer.writerows(raw_rows)

    medians = median_rows(raw_rows)
    instances = sorted({row["instance_name"] for row in raw_rows})
    single = {name: medians[("single", name)] for name in instances}
    triple = {name: medians[("triple", name)] for name in instances}
    triple_helpful = {
        name for name in instances
        if triple[name]["final_upper_bound"]
            < single[name]["final_upper_bound"] - 1e-9
    }
    triple_wall_overhead = {
        name for name in instances
        if materially_slower(
            triple[name]["instance_wall_seconds"],
            single[name]["instance_wall_seconds"])
    }
    triple_clk_overhead = {
        name for name in instances
        if materially_slower(
            triple[name]["initial_tour_seconds"],
            single[name]["initial_tour_seconds"])
    }

    detail_fields = (
        "instance", "single_gap", "single_ub", "triple_ub",
        "triple_ub_gain", "single_wall_seconds", "triple_wall_seconds",
        "triple_wall_delta", "single_initial_tour_seconds",
        "triple_initial_tour_seconds", "triple_clk_delta",
        "triple_helpful", "triple_material_wall_overhead",
        "triple_material_clk_overhead",
    ) + tuple(
        field
        for ratio in ratios
        for field in (
            f"adaptive_{ratio:g}_trigger",
            f"adaptive_{ratio:g}_improved",
            f"adaptive_{ratio:g}_starts",
            f"adaptive_{ratio:g}_ub",
            f"adaptive_{ratio:g}_wall_seconds",
        )
    )
    detail_rows: list[dict[str, object]] = []
    for name in instances:
        row: dict[str, object] = {
            "instance": name,
            "single_gap": single[name]["final_relative_gap"],
            "single_ub": single[name]["final_upper_bound"],
            "triple_ub": triple[name]["final_upper_bound"],
            "triple_ub_gain": single[name]["final_upper_bound"]
                - triple[name]["final_upper_bound"],
            "single_wall_seconds": single[name]["instance_wall_seconds"],
            "triple_wall_seconds": triple[name]["instance_wall_seconds"],
            "triple_wall_delta": triple[name]["instance_wall_seconds"]
                - single[name]["instance_wall_seconds"],
            "single_initial_tour_seconds": single[name]["initial_tour_seconds"],
            "triple_initial_tour_seconds": triple[name]["initial_tour_seconds"],
            "triple_clk_delta": triple[name]["initial_tour_seconds"]
                - single[name]["initial_tour_seconds"],
            "triple_helpful": int(name in triple_helpful),
            "triple_material_wall_overhead": int(name in triple_wall_overhead),
            "triple_material_clk_overhead": int(name in triple_clk_overhead),
        }
        for ratio in ratios:
            label = config_label("adaptive", ratio)
            adaptive = medians[(label, name)]
            prefix = f"adaptive_{ratio:g}"
            row[f"{prefix}_trigger"] = int(adaptive["adaptive_clk_triggers"])
            row[f"{prefix}_improved"] = int(adaptive["adaptive_clk_improvements"])
            row[f"{prefix}_starts"] = int(adaptive["initial_clk_starts"])
            row[f"{prefix}_ub"] = adaptive["final_upper_bound"]
            row[f"{prefix}_wall_seconds"] = adaptive["instance_wall_seconds"]
        detail_rows.append(row)
    with (output / "instance_comparison.csv").open(
            "w", encoding="utf-8-sig", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=detail_fields)
        writer.writeheader()
        writer.writerows(detail_rows)

    summary_fields = (
        "configuration", "gap_ratio", "triggered_instances",
        "improved_instances", "actual_clk_starts", "triple_helpful_instances",
        "triggered_triple_helpful", "missed_triple_helpful",
        "triple_material_wall_overhead_instances",
        "overhead_instances_not_triggered", "materially_slower_than_single",
        "total_median_wall_seconds", "wall_delta_vs_single",
        "total_median_initial_tour_seconds", "initial_tour_delta_vs_single",
    )
    single_wall = sum(row["instance_wall_seconds"] for row in single.values())
    single_initial = sum(row["initial_tour_seconds"] for row in single.values())
    summary_rows: list[dict[str, object]] = []

    def append_summary(label: str, ratio: float | None) -> None:
        current = {name: medians[(label, name)] for name in instances}
        triggered = {
            name for name, row in current.items()
            if int(row["adaptive_clk_triggers"]) != 0
        }
        improved = {
            name for name, row in current.items()
            if int(row["adaptive_clk_improvements"]) != 0
        }
        current_wall = sum(
            row["instance_wall_seconds"] for row in current.values())
        current_initial = sum(
            row["initial_tour_seconds"] for row in current.values())
        summary_rows.append({
            "configuration": label,
            "gap_ratio": "" if ratio is None else ratio,
            "triggered_instances": len(triggered),
            "improved_instances": len(improved),
            "actual_clk_starts": round(sum(
                row["initial_clk_starts"] for row in current.values())),
            "triple_helpful_instances": len(triple_helpful),
            "triggered_triple_helpful": len(triggered & triple_helpful),
            "missed_triple_helpful": len(triple_helpful - triggered),
            "triple_material_wall_overhead_instances": len(triple_wall_overhead),
            "overhead_instances_not_triggered": len(
                triple_wall_overhead - triggered),
            "materially_slower_than_single": sum(
                materially_slower(
                    current[name]["instance_wall_seconds"],
                    single[name]["instance_wall_seconds"])
                for name in instances),
            "total_median_wall_seconds": current_wall,
            "wall_delta_vs_single": current_wall - single_wall,
            "total_median_initial_tour_seconds": current_initial,
            "initial_tour_delta_vs_single": current_initial - single_initial,
        })

    append_summary("single", None)
    append_summary("triple", None)
    for ratio in ratios:
        append_summary(config_label("adaptive", ratio), ratio)

    with (output / "summary.csv").open(
            "w", encoding="utf-8-sig", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=summary_fields)
        writer.writeheader()
        writer.writerows(summary_rows)

    lines = [
        "# Adaptive CLK gap ratio：50 实例 root-only 扫描",
        "",
        "所有配置只执行初始 tour、根势上升和根 1-tree；不执行 root fixing、节点势更新或 BP。",
        "每格时间为各实例多次重复的中位数，再跨 50 个实例求和。",
        "",
        "| 配置 | 触发实例 | 改善实例 | CLK starts | 漏掉 triple 有益实例 | 避免 triple 增时实例 | 总 wall(s) | 相对 single(s) | 初始 tour(s) |",
        "|---|---:|---:|---:|---:|---:|---:|---:|---:|",
    ]
    for row in summary_rows:
        lines.append(
            f"| {row['configuration']} | {row['triggered_instances']} | "
            f"{row['improved_instances']} | {row['actual_clk_starts']} | "
            f"{row['missed_triple_helpful']} | "
            f"{row['overhead_instances_not_triggered']}/"
            f"{row['triple_material_wall_overhead_instances']} | "
            f"{format_seconds(float(row['total_median_wall_seconds']))} | "
            f"{float(row['wall_delta_vs_single']):+.6f} | "
            f"{format_seconds(float(row['total_median_initial_tour_seconds']))} |"
        )
    lines.extend((
        "",
        f"- triple 相对 single 能改善 UB 的实例：{len(triple_helpful)}。",
        f"- triple 具有实质 wall 增时（>10% 且 >1ms）的实例：{len(triple_wall_overhead)}。",
        f"- triple 具有实质初始 tour 增时（>10% 且 >1ms）的实例：{len(triple_clk_overhead)}。",
        "- `漏掉 triple 有益实例` 只说明该阈值没有触发这些实例；root-only 实验不测它们对完整 BP 节点数的后续影响。",
        "",
        "完整逐实例数据见 `instance_comparison.csv`，原始重复数据见 `raw_results.csv`。",
    ))
    (output / "README.md").write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    args = parse_args()
    solver = args.solver.resolve()
    batch = args.batch.resolve()
    if not solver.is_file():
        raise FileNotFoundError(solver)
    if not batch.is_file():
        raise FileNotFoundError(batch)
    ratios = sorted(set(args.gap_ratios))
    configurations = [("single", "single", None), ("triple", "triple", None)]
    configurations.extend(
        (config_label("adaptive", ratio), "adaptive", ratio)
        for ratio in ratios)

    raw_rows: list[dict[str, str]] = []
    for label, mode, ratio in configurations:
        for repeat in range(1, args.repeats + 1):
            print(f"running {label} repeat {repeat}/{args.repeats}", flush=True)
            rows = run_batch(
                solver, batch, args.root_ascent, mode, ratio)
            for source in rows:
                instance = source["instance"]
                local_name = Path(instance).name
                row = {
                    "config": label,
                    "mode": mode,
                    "gap_ratio": "" if ratio is None else f"{ratio:.17g}",
                    "repeat": str(repeat),
                    "instance_name": local_name,
                }
                row.update({field: source.get(field, "") for field in SELECTED_FIELDS})
                raw_rows.append(row)

    write_outputs(args.output.resolve(), raw_rows, ratios)
    print(f"wrote {args.output.resolve()}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
