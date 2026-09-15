#!/usr/bin/env python3
"""Benchmark LKH PI reuse and a persistent fork worker at root only."""

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


ROOT = Path(__file__).resolve().parents[2]
CONFIGS = (
    "single-knn8", "lkh-exec", "lkh-worker",
    "provider-local-ascent", "provider-pi-replace", "provider-pi-warm16",
)
FIELDS = (
    "upper_bound", "lower_bound", "wall_seconds", "heuristic_seconds",
    "root_ascent_seconds", "root_potential_iterations",
)


def arguments() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--solver", type=Path, required=True)
    parser.add_argument("--lkh", type=Path, required=True)
    parser.add_argument(
        "--worker", type=Path, required=True,
        help="CMake-built isolated tsp_lkh_provider executable")
    parser.add_argument(
        "--batch", type=Path, default=ROOT / "data/classic/batch-n200.txt")
    parser.add_argument(
        "--output", type=Path,
        default=ROOT / "outputs/phkmst-ablation_914_ratio"
        / "lkh-pi-reuse-20260915")
    parser.add_argument("--repeats", type=int, default=3)
    args = parser.parse_args()
    if args.repeats <= 0:
        parser.error("--repeats must be positive")
    return args


def instances(batch: Path) -> list[Path]:
    result = []
    for raw in batch.read_text(encoding="utf-8").splitlines():
        value = raw.strip()
        if not value or value.startswith("#"):
            continue
        path = Path(value)
        result.append((path if path.is_absolute() else ROOT / path).resolve())
    return result


def dimension(path: Path) -> int:
    match = re.search(
        r"(?im)^\s*DIMENSION\s*:?\s*(\d+)\s*$",
        path.read_text(encoding="utf-8", errors="replace"))
    if match is None:
        raise RuntimeError(f"missing DIMENSION: {path}")
    return int(match.group(1))


def internal_batch(args: argparse.Namespace, extra: tuple[str, ...]) -> list[dict[str, object]]:
    completed = subprocess.run([
        str(args.solver), "--root-bound-only", "--hk-ascent", "hybrid-reverse",
        "--exact-max-n", "199", *extra,
        "--batch", str(args.batch),
    ], cwd=ROOT, text=True, capture_output=True, check=False)
    if completed.returncode:
        raise RuntimeError(completed.stderr[-2000:])
    rows = list(csv.DictReader(io.StringIO(completed.stdout)))
    return [{
        "instance": str(Path(row["instance"]).resolve()),
        "upper_bound": float(row["final_upper_bound"]),
        "lower_bound": float(row["final_lower_bound"]),
        "wall_seconds": float(row["instance_wall_seconds"]),
        "heuristic_seconds": float(row["initial_tour_seconds"]),
        "root_ascent_seconds": float(row["root_ascent_seconds"]),
        "root_potential_iterations": float(row["root_potential_iterations"]),
    } for row in rows]


class Worker:
    def __init__(self, executable: Path) -> None:
        self.process = subprocess.Popen(
            [str(executable)], stdin=subprocess.PIPE, stdout=subprocess.PIPE,
            text=True, bufsize=1)
        if self.process.stdout is None or self.process.stdout.readline().strip() != "READY":
            raise RuntimeError("LKH worker did not become ready")

    def run(self, parameter: Path) -> float:
        if self.process.stdin is None or self.process.stdout is None:
            raise RuntimeError("LKH worker pipe is closed")
        self.process.stdin.write(str(parameter) + "\n")
        self.process.stdin.flush()
        response = self.process.stdout.readline().strip().split()
        if len(response) != 3 or response[:2] != ["DONE", "0"]:
            raise RuntimeError(f"LKH worker failed: {' '.join(response)}")
        return float(response[2])

    def close(self) -> None:
        if self.process.poll() is not None:
            return
        assert self.process.stdin is not None
        self.process.stdin.write("QUIT\n")
        self.process.stdin.flush()
        self.process.wait(timeout=5)


def parameter_file(
    directory: Path, instance: Path, repeat: int,
) -> tuple[Path, Path, Path]:
    directory.mkdir(parents=True)
    tour = directory / "result.tour"
    pi = directory / "result.pi"
    parameter = directory / "run.par"
    parameter.write_text("\n".join((
        f"PROBLEM_FILE = {instance}",
        f"OUTPUT_TOUR_FILE = {tour}",
        f"PI_FILE = {pi}",
        "RUNS = 1",
        f"MAX_TRIALS = {dimension(instance)}",
        f"SEED = {repeat}",
        "TRACE_LEVEL = 0",
        "",
    )), encoding="utf-8")
    return parameter, tour, pi


def parse_tour(path: Path, n: int) -> tuple[list[int], float]:
    text = path.read_text(encoding="utf-8", errors="replace")
    cost_match = re.search(r"(?im)^COMMENT\s*:\s*Length\s*=\s*([^\s]+)", text)
    if cost_match is None:
        raise RuntimeError(f"missing LKH tour length: {path}")
    lines = text.splitlines()
    start = next(i for i, line in enumerate(lines)
                 if line.strip().upper() == "TOUR_SECTION") + 1
    tour = []
    for line in lines[start:]:
        token = line.strip()
        if token in ("-1", "EOF"):
            break
        if token:
            tour.append(int(token) - 1)
    if len(tour) != n or sorted(tour) != list(range(n)):
        raise RuntimeError(f"invalid LKH tour: {path}")
    return tour, float(cost_match.group(1))


def label(output: str, name: str) -> float:
    match = re.search(rf"(?m)^{re.escape(name)}:\s*([^\s]+)\s*$", output)
    if match is None:
        raise RuntimeError(f"missing field {name}")
    return float(match.group(1))


def solver_provider(
    args: argparse.Namespace, instance: Path, repeat: int, mode: str,
) -> dict[str, object]:
    command = [
        str(args.solver), "--root-bound-only", "--exact-max-n", "199",
        "--lkh-provider", str(args.worker), "--lkh-provider-failure", "error",
        "--lkh-runs", "1", "--lkh-max-trials", str(dimension(instance)),
        "--lkh-seed", str(repeat),
    ]
    if mode == "local":
        command.extend((
            "--lkh-pi-mode", "off", "--hk-ascent", "hybrid-reverse"))
    elif mode == "replace":
        command.extend((
            "--hk-ascent", "hybrid-reverse", "--lkh-pi-mode", "replace"))
    elif mode == "warm16":
        command.extend((
            "--lkh-pi-mode", "warm-start",
            "--root-pi-refine-ascent", "polyak",
            "--root-pi-refine-iterations", "16"))
    else:
        raise ValueError(mode)
    command.append(str(instance))
    completed = subprocess.run(
        command, cwd=ROOT, text=True, capture_output=True, check=False)
    if completed.returncode:
        raise RuntimeError(completed.stderr[-2000:])
    provider_seconds = label(completed.stdout, "LKH provider seconds")
    return {
        "instance": str(instance),
        "upper_bound": label(completed.stdout, "Final upper bound"),
        "lower_bound": label(completed.stdout, "Final lower bound"),
        "wall_seconds": label(completed.stdout, "Instance wall seconds"),
        "heuristic_seconds": provider_seconds
            + label(completed.stdout, "Initial tour seconds"),
        "root_ascent_seconds": label(completed.stdout, "Root ascent seconds"),
        "root_potential_iterations": label(
            completed.stdout, "Root potential iterations"),
    }


def medians(raw: list[dict[str, object]]) -> dict[tuple[str, str], dict[str, float]]:
    groups: dict[tuple[str, str], list[dict[str, object]]] = defaultdict(list)
    for row in raw:
        groups[(str(row["configuration"]), str(row["instance"]))].append(row)
    return {key: {
        field: statistics.median(float(row[field]) for row in rows)
        for field in FIELDS
    } for key, rows in groups.items()}


def reports(args: argparse.Namespace, raw: list[dict[str, object]]) -> None:
    args.output.mkdir(parents=True, exist_ok=True)
    columns = ("repeat", "configuration", "instance") + FIELDS
    with (args.output / "raw_results.csv").open(
            "w", encoding="utf-8-sig", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=columns)
        writer.writeheader()
        writer.writerows(raw)
    values = medians(raw)
    names = sorted({str(row["instance"]) for row in raw})
    baseline = {name: values[("single-knn8", name)] for name in names}
    local = {name: values[("provider-local-ascent", name)] for name in names}
    summary = []
    for config in CONFIGS:
        current = {name: values[(config, name)] for name in names}
        gains = [baseline[name]["upper_bound"] - current[name]["upper_bound"]
                 for name in names]
        lb_losses = [local[name]["lower_bound"] - current[name]["lower_bound"]
                     for name in names]
        summary.append({
            "configuration": config,
            "ub_wins_vs_single": sum(gain > 1e-9 for gain in gains),
            "ub_losses_vs_single": sum(gain < -1e-9 for gain in gains),
            "total_ub_gain_vs_single": sum(gains),
            "total_wall_seconds": sum(row["wall_seconds"] for row in current.values()),
            "total_heuristic_seconds": sum(
                row["heuristic_seconds"] for row in current.values()),
            "total_root_ascent_seconds": sum(
                row["root_ascent_seconds"] for row in current.values()),
            "total_root_iterations": sum(
                row["root_potential_iterations"] for row in current.values()),
            "median_lb_loss_vs_provider_local": statistics.median(lb_losses),
            "max_lb_loss_vs_provider_local": max(lb_losses),
        })
    with (args.output / "summary.csv").open(
            "w", encoding="utf-8-sig", newline="") as stream:
        writer = csv.DictWriter(stream, fieldnames=tuple(summary[0].keys()))
        writer.writeheader()
        writer.writerows(summary)

    lines = [
        "# LKH 势复用与长驻 worker 实验", "",
        f"- {len(names)} 实例，{args.repeats} 次重复，逐实例取中位数；只到根证书。",
        "- worker 为长期存活父进程，每任务 fork 已链接 LKH 的子进程，消除 exec/装载。",
        "- PI replace 将 LKH PI_FILE 第一节点重标号为内部根 0，跳过本地上升。",
        "- PI warm16 从外部势开始执行最多 16 轮本地 Polyak。", "",
        "| 配置 | UB 胜/负 | UB 净降幅 | 总时间(s) | 根上升(s) | 根迭代 | LB损失中位/最大 |",
        "|---|---:|---:|---:|---:|---:|---:|",
    ]
    for row in summary:
        lines.append(
            f"| {row['configuration']} "
            f"| {row['ub_wins_vs_single']}/{row['ub_losses_vs_single']} "
            f"| {row['total_ub_gain_vs_single']:.3f} "
            f"| {row['total_wall_seconds']:.6f} "
            f"| {row['total_root_ascent_seconds']:.6f} "
            f"| {row['total_root_iterations']:.0f} "
            f"| {row['median_lb_loss_vs_provider_local']:.6f}/"
            f"{row['max_lb_loss_vs_provider_local']:.6f} |")
    by_config = {row["configuration"]: row for row in summary}
    exec_time = by_config["lkh-exec"]["total_wall_seconds"]
    worker_time = by_config["lkh-worker"]["total_wall_seconds"]
    local_time = by_config["provider-local-ascent"]["total_wall_seconds"]
    replace_time = by_config["provider-pi-replace"]["total_wall_seconds"]
    lb_deltas = [
        values[("provider-pi-replace", name)]["lower_bound"]
        - values[("provider-local-ascent", name)]["lower_bound"]
        for name in names
    ]
    lines.extend((
        "", "## 结论", "",
        f"- 长驻 fork worker 比逐实例 exec 节省 {exec_time - worker_time:.6f}s "
        f"（{100.0 * (exec_time - worker_time) / exec_time:.2f}%）。",
        f"- PI replace 比原 provider 本地完整上升节省 "
        f"{local_time - replace_time:.6f}s（"
        f"{100.0 * (local_time - replace_time) / local_time:.2f}%）。",
        f"- PI replace 的根 LB 相对本地完整上升：更强 "
        f"{sum(value > 1e-6 for value in lb_deltas)}、持平 "
        f"{sum(abs(value) <= 1e-6 for value in lb_deltas)}、更弱 "
        f"{sum(value < -1e-6 for value in lb_deltas)}；最大损失 "
        f"{-min(lb_deltas):.6f}。是否默认启用仍需完整精确搜索验证。",
    ))
    (args.output / "结论.md").write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> int:
    args = arguments()
    paths = instances(args.batch)
    raw: list[dict[str, object]] = []
    worker = Worker(args.worker)
    try:
        for repeat in range(1, args.repeats + 1):
            print(f"[{repeat}/{args.repeats}] internal", flush=True)
            for config, extra in (("single-knn8", ()),):
                for row in internal_batch(args, extra):
                    raw.append({"repeat": repeat, "configuration": config, **row})
            with tempfile.TemporaryDirectory(prefix="tsp-lkh-pi-") as temp_raw:
                temp = Path(temp_raw)
                for index, instance in enumerate(paths, 1):
                    if index == 1 or index % 10 == 0:
                        print(f"[{repeat}/{args.repeats}] {index}/{len(paths)}", flush=True)
                    n = dimension(instance)
                    exec_par, exec_tour, _ = parameter_file(
                        temp / f"exec-{index}", instance, repeat)
                    started = time.perf_counter()
                    completed = subprocess.run(
                        [str(args.lkh), str(exec_par)], cwd=ROOT,
                        stdout=subprocess.DEVNULL, stderr=subprocess.DEVNULL)
                    exec_seconds = time.perf_counter() - started
                    if completed.returncode:
                        raise RuntimeError(f"LKH exec failed: {instance}")
                    _, exec_cost = parse_tour(exec_tour, n)

                    worker_par, worker_tour, _ = parameter_file(
                        temp / f"worker-{index}", instance, repeat)
                    worker_seconds = worker.run(worker_par)
                    _, worker_cost = parse_tour(worker_tour, n)
                    if worker_cost != exec_cost:
                        raise RuntimeError(f"worker changed LKH result: {instance}")
                    providers = {
                        "provider-local-ascent": solver_provider(
                            args, instance, repeat, "local"),
                        "provider-pi-replace": solver_provider(
                            args, instance, repeat, "replace"),
                        "provider-pi-warm16": solver_provider(
                            args, instance, repeat, "warm16"),
                    }
                    local_lb = providers["provider-local-ascent"]["lower_bound"]
                    for config, seconds in (
                        ("lkh-exec", exec_seconds),
                        ("lkh-worker", worker_seconds),
                    ):
                        raw.append({
                            "repeat": repeat, "configuration": config,
                            "instance": str(instance), "upper_bound": worker_cost,
                            "lower_bound": local_lb, "wall_seconds": seconds,
                            "heuristic_seconds": seconds, "root_ascent_seconds": 0.0,
                            "root_potential_iterations": 0.0,
                        })
                    for config, row in providers.items():
                        raw.append({"repeat": repeat, "configuration": config, **row})
    finally:
        worker.close()
    reports(args, raw)
    print(args.output / "结论.md")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
