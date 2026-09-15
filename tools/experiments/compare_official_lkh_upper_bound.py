#!/usr/bin/env python3
"""Compare official LKH 2.0.11 with internal root-guided LK upper bounds.

LKH is run with RUNS=1 and MAX_TRIALS=n.  Its 1-based TOUR_SECTION is
validated, converted to the solver's 0-based initial-tour format, and supplied
as an incumbent while internal CLK is disabled.  Exact BP search is not run.
"""

from __future__ import annotations

import argparse
import csv
import io
import re
import statistics
import subprocess
import tempfile
import time
from collections import defaultdict
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parents[2]
CONFIGS = ("single-knn8", "root-alpha8", "official-lkh", "lkh-provider")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--solver", type=Path, required=True)
    parser.add_argument("--lkh", type=Path, required=True)
    parser.add_argument(
        "--batch", type=Path,
        default=PROJECT_ROOT / "data/classic/batch-n200.txt")
    parser.add_argument(
        "--output", type=Path,
        default=PROJECT_ROOT / "outputs/phkmst-ablation"
        / "official-lkh-upper-bound-20260915")
    parser.add_argument("--repeats", type=int, default=3)
    parser.add_argument("--root-ascent", default="hybrid-reverse")
    args = parser.parse_args()
    if args.repeats <= 0:
        parser.error("--repeats must be positive")
    return args


def read_instances(batch: Path) -> list[Path]:
    instances = []
    for raw in batch.read_text(encoding="utf-8").splitlines():
        line = raw.strip()
        if not line or line.startswith("#"):
            continue
        path = Path(line)
        if not path.is_absolute():
            path = PROJECT_ROOT / path
        instances.append(path.resolve())
    return instances


def dimension_of(path: Path) -> int:
    match = re.search(
        r"(?im)^\s*DIMENSION\s*:?\s*(\d+)\s*$",
        path.read_text(encoding="utf-8", errors="replace"))
    if match is None:
        raise RuntimeError(f"missing TSPLIB DIMENSION in {path}")
    return int(match.group(1))


def internal_batch(args: argparse.Namespace, extra: tuple[str, ...]) -> list[dict[str, object]]:
    command = [
        str(args.solver), "--root-bound-only",
        "--hk-ascent", args.root_ascent,
        "--initial-clk", "single", "--exact-max-n", "199",
        *extra, "--batch", str(args.batch),
    ]
    completed = subprocess.run(
        command, cwd=PROJECT_ROOT, text=True, capture_output=True, check=False)
    if completed.returncode != 0:
        raise RuntimeError(completed.stderr[-2000:])
    rows = list(csv.DictReader(io.StringIO(completed.stdout)))
    return [{
        "instance": str(Path(row["instance"]).resolve()),
        "upper_bound": float(row["final_upper_bound"]),
        "lower_bound": float(row["final_lower_bound"]),
        "wall_seconds": float(row["instance_wall_seconds"]),
        "heuristic_seconds": float(row["initial_tour_seconds"]),
    } for row in rows]


def parse_lkh_tour(path: Path, dimension: int) -> list[int]:
    lines = path.read_text(encoding="utf-8", errors="replace").splitlines()
    try:
        start = next(index for index, line in enumerate(lines)
                     if line.strip().upper() == "TOUR_SECTION") + 1
    except StopIteration as error:
        raise RuntimeError(f"LKH tour has no TOUR_SECTION: {path}") from error
    tour: list[int] = []
    for line in lines[start:]:
        token = line.strip()
        if token in ("-1", "EOF"):
            break
        if token:
            tour.append(int(token) - 1)
    if len(tour) != dimension or sorted(tour) != list(range(dimension)):
        raise RuntimeError(f"invalid LKH tour in {path}")
    return tour


def labeled_float(output: str, label: str) -> float:
    match = re.search(
        rf"(?m)^{re.escape(label)}:\s*([^\s]+)\s*$", output)
    if match is None:
        raise RuntimeError(f"missing solver field {label!r}")
    return float(match.group(1))


def run_lkh_and_provider(
    args: argparse.Namespace, instance: Path, repeat: int,
) -> tuple[dict[str, object], dict[str, object]]:
    dimension = dimension_of(instance)
    with tempfile.TemporaryDirectory(prefix="tsp-lkh-provider-") as temp_raw:
        temp = Path(temp_raw)
        parameter = temp / "run.par"
        lkh_tour = temp / "result.tour"
        parameter.write_text(
            "\n".join((
                f"PROBLEM_FILE = {instance}",
                f"OUTPUT_TOUR_FILE = {lkh_tour}",
                "RUNS = 1",
                f"MAX_TRIALS = {dimension}",
                f"SEED = {repeat}",
                "TRACE_LEVEL = 1",
                "",
            )), encoding="utf-8")
        started = time.perf_counter()
        completed = subprocess.run(
            [str(args.lkh), str(parameter)], cwd=PROJECT_ROOT,
            text=True, capture_output=True, check=False)
        lkh_wall = time.perf_counter() - started
        if completed.returncode != 0:
            raise RuntimeError(
                f"LKH failed for {instance}: {completed.stderr[-2000:]}")
        match = re.search(r"(?m)^Cost\.min\s*=\s*([^,\s]+)", completed.stdout)
        if match is None:
            raise RuntimeError(f"LKH reported no Cost.min for {instance}")
        lkh_cost = float(match.group(1))
        tour = parse_lkh_tour(lkh_tour, dimension)

        supplied = temp / "initial.tour"
        supplied.write_text(
            f"{dimension}\n" + " ".join(map(str, tour)) + "\n",
            encoding="utf-8")
        solver_started = time.perf_counter()
        integrated = subprocess.run(
            [
                str(args.solver), "--root-bound-only",
                "--hk-ascent", args.root_ascent,
                "--initial-clk", "off", "--initial-tour", str(supplied),
                "--exact-max-n", "199", str(instance),
            ], cwd=PROJECT_ROOT, text=True, capture_output=True, check=False)
        solver_wall = time.perf_counter() - solver_started
        if integrated.returncode != 0:
            raise RuntimeError(
                f"provider integration failed for {instance}: "
                f"{integrated.stderr[-2000:]}")
        integrated_ub = labeled_float(integrated.stdout, "Final upper bound")
        if integrated_ub > lkh_cost + 1e-9:
            raise RuntimeError(
                f"solver rejected a valid LKH incumbent for {instance}")
        lower_bound = labeled_float(integrated.stdout, "Final lower bound")
        return ({
            "instance": str(instance), "upper_bound": lkh_cost,
            "lower_bound": lower_bound, "wall_seconds": lkh_wall,
            "heuristic_seconds": lkh_wall,
        }, {
            "instance": str(instance), "upper_bound": integrated_ub,
            "lower_bound": lower_bound,
            "wall_seconds": lkh_wall + solver_wall,
            "heuristic_seconds": lkh_wall
                + labeled_float(integrated.stdout, "Initial tour seconds"),
        })


def summarize(args: argparse.Namespace, raw: list[dict[str, object]]) -> None:
    args.output.mkdir(parents=True, exist_ok=True)
    fields = (
        "repeat", "configuration", "instance", "upper_bound",
        "lower_bound", "wall_seconds", "heuristic_seconds")
    with (args.output / "raw_results.csv").open(
            "w", encoding="utf-8-sig", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=fields)
        writer.writeheader()
        writer.writerows(raw)

    groups: dict[tuple[str, str], list[dict[str, object]]] = defaultdict(list)
    for row in raw:
        groups[(str(row["configuration"]), str(row["instance"]))].append(row)
    medians = {
        key: {
            field: statistics.median(float(row[field]) for row in rows)
            for field in ("upper_bound", "lower_bound", "wall_seconds",
                          "heuristic_seconds")
        }
        for key, rows in groups.items()
    }
    instances = sorted({str(row["instance"]) for row in raw})
    baseline = {
        name: medians[("single-knn8", name)] for name in instances}
    alpha = {name: medians[("root-alpha8", name)] for name in instances}
    summary_rows = []
    for label in CONFIGS:
        current = {name: medians[(label, name)] for name in instances}
        versus_base = {
            name: baseline[name]["upper_bound"] - current[name]["upper_bound"]
            for name in instances}
        versus_alpha = {
            name: alpha[name]["upper_bound"] - current[name]["upper_bound"]
            for name in instances}
        summary_rows.append({
            "configuration": label,
            "wins_vs_single": sum(gain > 1e-9 for gain in versus_base.values()),
            "losses_vs_single": sum(gain < -1e-9 for gain in versus_base.values()),
            "wins_vs_alpha8": sum(gain > 1e-9 for gain in versus_alpha.values()),
            "losses_vs_alpha8": sum(gain < -1e-9 for gain in versus_alpha.values()),
            "total_gain_vs_single": sum(versus_base.values()),
            "total_wall_seconds": sum(
                row["wall_seconds"] for row in current.values()),
            "total_heuristic_seconds": sum(
                row["heuristic_seconds"] for row in current.values()),
        })
    summary_fields = tuple(summary_rows[0].keys())
    with (args.output / "summary.csv").open(
            "w", encoding="utf-8-sig", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=summary_fields)
        writer.writeheader()
        writer.writerows(summary_rows)

    details = []
    for name in instances:
        details.append({
            "instance": name,
            "single_ub": baseline[name]["upper_bound"],
            "alpha8_ub": alpha[name]["upper_bound"],
            "lkh_ub": medians[("official-lkh", name)]["upper_bound"],
            "provider_ub": medians[("lkh-provider", name)]["upper_bound"],
            "lkh_gain_vs_single": baseline[name]["upper_bound"]
                - medians[("official-lkh", name)]["upper_bound"],
            "lkh_gain_vs_alpha8": alpha[name]["upper_bound"]
                - medians[("official-lkh", name)]["upper_bound"],
        })
    with (args.output / "instance_comparison.csv").open(
            "w", encoding="utf-8-sig", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=tuple(details[0].keys()))
        writer.writeheader()
        writer.writerows(details)

    lines = [
        "# 官方 LKH 初始上界对照",
        "",
        "- LKH：2.0.11，官方源码临时编译；未将第三方源码/二进制放入仓库",
        "- 参数：`RUNS=1`、`MAX_TRIALS=n`、默认 `ALPHA`/5 candidates",
        f"- 实例：{len(instances)} 个；重复 {args.repeats} 次，逐实例取中位数",
        "- `lkh-provider`：校验并转换 LKH tour，以 `--initial-clk off` 注入根求解",
        "- 所有配置只到根证书，不执行 fixing/BP",
        "",
        "| 配置 | 对 single 胜/负 | 对 alpha8 胜/负 | UB 净降幅 vs single | 总墙钟(s) | 启发式耗时(s) |",
        "|---|---:|---:|---:|---:|---:|",
    ]
    for row in summary_rows:
        lines.append(
            f"| {row['configuration']} "
            f"| {row['wins_vs_single']}/{row['losses_vs_single']} "
            f"| {row['wins_vs_alpha8']}/{row['losses_vs_alpha8']} "
            f"| {row['total_gain_vs_single']:.3f} "
            f"| {row['total_wall_seconds']:.6f} "
            f"| {row['total_heuristic_seconds']:.6f} |")
    lines.extend((
        "",
        "说明：`official-lkh` 的墙钟只包含 LKH；`lkh-provider` 包含 LKH、"
        "NN+2-opt 兜底和本求解器根上升，因此二者不能直接作为同一阶段速度比较。",
        "Provider 保证保留 LKH tour；如果内部 NN+2-opt 恰好更好，最终 UB "
        "可以严格小于该次 LKH 的结果。",
        "",
        "来源：[LKH 官方主页](https://webhotel4.ruc.dk/~keld/research/LKH/)",
        "",
    ))
    (args.output / "结论.md").write_text("\n".join(lines), encoding="utf-8")


def main() -> int:
    args = parse_args()
    instances = read_instances(args.batch)
    raw: list[dict[str, object]] = []
    for repeat in range(1, args.repeats + 1):
        print(f"[{repeat}/{args.repeats}] internal baselines", flush=True)
        for label, extra in (
            ("single-knn8", ()),
            ("root-alpha8", (
                "--lk-candidate-set", "alpha", "--lk-candidates", "8",
                "--root-guided-lk", "once")),
        ):
            for row in internal_batch(args, extra):
                raw.append({"repeat": repeat, "configuration": label, **row})
        for index, instance in enumerate(instances, 1):
            if index == 1 or index % 10 == 0:
                print(
                    f"[{repeat}/{args.repeats}] LKH/provider {index}/{len(instances)}",
                    flush=True)
            lkh, provider = run_lkh_and_provider(args, instance, repeat)
            raw.append({
                "repeat": repeat, "configuration": "official-lkh", **lkh})
            raw.append({
                "repeat": repeat, "configuration": "lkh-provider", **provider})
    summarize(args, raw)
    print(args.output / "结论.md")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
