#!/usr/bin/env python3
"""Run reproducible PHKMST parameter ablations and verify exact costs.

The default ``smoke`` suite intentionally uses only three TSPLIB instances.
It exercises root ascent, node ascent, frequent node updates, and local/global
alpha branching before a larger experiment is attempted.

Outputs:
  configurations.csv      exact CLI dimensions of every selected group
  raw_runs.csv             one row per configuration/repeat/instance
  results_by_instance.csv median metrics and correctness by instance
  summary.csv              aggregate table used by the experiment report
  summary.md               compact human-readable version of the tables
"""

from __future__ import annotations

import argparse
import csv
import io
import math
import statistics
import subprocess
import tempfile
import time
from dataclasses import asdict, dataclass
from pathlib import Path
from typing import Any, Iterable


PROJECT_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_SOLVER = PROJECT_ROOT / "build" / "tsp_bb"
DEFAULT_BATCH = PROJECT_ROOT / "data" / "classic" / "batch-ablation-smoke.txt"
DEFAULT_OUTPUT = PROJECT_ROOT / "outputs" / "phkmst-ablation-smoke"

# Independent TSPLIB reference values for the default smoke set and the
# existing node-ascent batch. Unknown instances fall back to cross-strategy
# agreement with P0 and are marked as such in the output.
KNOWN_OPTIMA: dict[str, float] = {
    "dantzig42.tsp": 699.0,
    "att48.tsp": 10628.0,
    "bayg29.tsp": 1610.0,
    "bays29.tsp": 2020.0,
    "fri26.tsp": 937.0,
    "st70.tsp": 675.0,
    "eil76.tsp": 538.0,
    "rat99.tsp": 1211.0,
    "eil101.tsp": 629.0,
}

INTEGER_FIELDS = (
    "dimension",
    "nodes_created",
    "nodes_expanded",
    "pruned_by_bound",
    "pruned_infeasible",
    "potential_updates_attempted",
    "potential_updates_improved",
    "potential_updates_pruned",
    "potential_updates_rebuilt",
    "potential_updates_stopped_prunable",
    "potential_update_iterations",
)

FLOAT_FIELDS = (
    "cost",
    "root_lower_bound",
    "initial_upper_bound",
    "instance_wall_seconds",
    "potential_update_seconds",
    "potential_update_rebuild_seconds",
    "potential_update_total_gain",
    "potential_update_max_gain",
)


@dataclass(frozen=True)
class ExperimentConfig:
    config_id: str
    category: str
    root_ascent: str = "polyak"
    node_ascent: str = "polyak"
    potential_update: str = "none"
    update_depth: int = 2
    update_gap_ratio: float = 0.02
    update_iterations: int = 16
    update_budget: int = 5000
    branch_edge_order: str = "weight"
    description: str = ""

    @property
    def effective_node_ascent(self) -> str:
        return "off" if self.potential_update == "none" else self.node_ascent

    def solver_args(self) -> list[str]:
        args = [
            "--hk-ascent", self.root_ascent,
            "--hk-node-ascent", self.node_ascent,
            "--branch-edge-order", self.branch_edge_order,
            "--hk-potential-update", self.potential_update,
        ]
        if self.potential_update != "none":
            args.extend([
                "--hk-update-depth", str(self.update_depth),
                "--hk-update-gap-ratio", str(self.update_gap_ratio),
                "--hk-update-iterations", str(self.update_iterations),
                "--hk-update-budget", str(self.update_budget),
            ])
        return args


CONFIGS: dict[str, ExperimentConfig] = {
    # Root/node 2x3 core factorial.
    "P0": ExperimentConfig(
        "P0", "core", description="root Polyak; node updates disabled"),
    "P1": ExperimentConfig(
        "P1", "core", potential_update="subtree-adaptive",
        description="root Polyak; node Polyak; subtree adaptive"),
    "P2": ExperimentConfig(
        "P2", "core", node_ascent="helsgaun",
        potential_update="subtree-adaptive",
        description="root Polyak; node Helsgaun; subtree adaptive"),
    "H0": ExperimentConfig(
        "H0", "core", root_ascent="helsgaun",
        description="root Helsgaun; node updates disabled"),
    "H1": ExperimentConfig(
        "H1", "core", root_ascent="helsgaun",
        potential_update="subtree-adaptive",
        description="root Helsgaun; node Polyak; subtree adaptive"),
    "H2": ExperimentConfig(
        "H2", "core", root_ascent="helsgaun", node_ascent="helsgaun",
        potential_update="subtree-adaptive",
        description="root Helsgaun; node Helsgaun; subtree adaptive"),

    # Trigger/lifecycle ablation. P0 and P1 are the disabled/current anchors.
    "T1": ExperimentConfig(
        "T1", "trigger", potential_update="depth", update_depth=1,
        description="temporary update at every eligible DFS depth"),
    "T2": ExperimentConfig(
        "T2", "trigger", potential_update="depth", update_depth=2,
        description="temporary update every two DFS depths"),
    "T3": ExperimentConfig(
        "T3", "trigger", potential_update="adaptive", update_depth=1,
        description="temporary update when depth>=1 and relative gap<=2%"),
    "T4": ExperimentConfig(
        "T4", "trigger", potential_update="subtree-depth", update_depth=1,
        description="persistent subtree epoch at every eligible depth"),

    # Alpha branching ablation. All groups keep root Polyak and node updates off.
    "B1": ExperimentConfig(
        "B1", "alpha", branch_edge_order="root-alpha-asc",
        description="one maximum-degree vertex; alpha ascending"),
    "B2": ExperimentConfig(
        "B2", "alpha", branch_edge_order="root-alpha-desc",
        description="one maximum-degree vertex; alpha descending"),
    "B3": ExperimentConfig(
        "B3", "alpha", branch_edge_order="root-alpha-global-asc",
        description="all violation-touching edges; alpha ascending first"),
    "B4": ExperimentConfig(
        "B4", "alpha", branch_edge_order="root-alpha-global-desc",
        description="all violation-touching edges; alpha descending first"),
}

SUITES: dict[str, tuple[str, ...]] = {
    "smoke": ("P0", "P1", "P2", "H0", "T1", "B1", "B3", "B4"),
    "core": ("P0", "P1", "P2", "H0", "H1", "H2"),
    "trigger": ("P0", "T1", "T2", "T3", "T4", "P1"),
    "alpha": ("P0", "B1", "B2", "B3", "B4"),
    "all": tuple(CONFIGS),
}

RAW_FIELDS = (
    "config_id", "category", "repeat", "instance", "dimension", "status",
    "cost", "expected_cost", "reference_source", "cost_correct",
    "root_lower_bound", "initial_upper_bound", "instance_wall_seconds",
    "nodes_created",
    "nodes_expanded", "pruned_by_bound", "pruned_infeasible",
    "potential_updates_attempted", "potential_updates_improved",
    "potential_updates_pruned", "potential_updates_rebuilt",
    "potential_updates_stopped_prunable", "potential_update_iterations",
    "potential_update_seconds", "potential_update_rebuild_seconds",
    "potential_update_total_gain", "potential_update_max_gain",
    "batch_wall_seconds",
)

INSTANCE_FIELDS = (
    "config_id", "instance", "dimension", "repeats", "status", "cost",
    "expected_cost", "reference_source", "cost_correct",
    "root_lower_bound", "initial_upper_bound", "instance_wall_median_seconds",
    "instance_wall_ratio_vs_p0", "nodes_created", "nodes_expanded",
    "expanded_ratio_vs_p0", "pruned_by_bound",
    "pruned_infeasible", "potential_updates_attempted",
    "potential_updates_improved", "potential_updates_pruned",
    "potential_updates_rebuilt", "potential_updates_stopped_prunable",
    "potential_update_iterations", "potential_update_seconds",
    "potential_update_rebuild_seconds", "potential_update_total_gain",
    "potential_update_max_gain",
)

SUMMARY_FIELDS = (
    "config_id", "correct_instances", "total_instances", "nodes_created",
    "nodes_expanded", "expanded_ratio_vs_p0", "expanded_geomean_ratio_vs_p0",
    "wins", "ties", "losses", "batch_wall_median_seconds",
    "wall_ratio_vs_p0", "instance_wall_total_seconds",
    "instance_wall_geomean_ratio_vs_p0",
    "potential_updates_attempted", "improvement_rate",
    "direct_prune_rate", "average_iterations_per_attempt", "epoch_rebuilds",
    "gain_per_iteration", "potential_update_seconds", "potential_time_share",
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--solver", type=Path, default=DEFAULT_SOLVER,
                        help="tsp_bb executable")
    parser.add_argument("--suite", choices=SUITES, default="smoke",
                        help="parameter group to run")
    parser.add_argument("--batch-list", type=Path,
                        help="instance list; defaults to the three-instance smoke list")
    parser.add_argument("--instances", nargs="*", type=Path,
                        help="explicit instances instead of a batch list")
    parser.add_argument("--repeats", type=int, default=1,
                        help="interleaved repetitions; median wall time is reported")
    parser.add_argument("--timeout", type=float, default=120.0,
                        help="seconds allowed for one configuration batch")
    parser.add_argument("--exact-max-n", type=int, default=130,
                        help="solver exact-size guard")
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT,
                        help="directory for CSV and Markdown tables")
    parser.add_argument("--list-configs", action="store_true",
                        help="print the selected parameter commands and exit")
    args = parser.parse_args()
    if args.repeats <= 0:
        parser.error("--repeats must be positive")
    if args.timeout <= 0:
        parser.error("--timeout must be positive")
    if args.exact_max_n < 3:
        parser.error("--exact-max-n must be at least 3")
    return args


def resolve_instance(path: Path) -> Path:
    candidate = path if path.is_absolute() else PROJECT_ROOT / path
    candidate = candidate.resolve()
    if not candidate.is_file():
        raise FileNotFoundError(f"instance not found: {candidate}")
    return candidate


def read_batch_list(path: Path) -> list[Path]:
    list_path = path if path.is_absolute() else PROJECT_ROOT / path
    instances: list[Path] = []
    for raw_line in list_path.read_text(encoding="utf-8").splitlines():
        line = raw_line.strip()
        if line and not line.startswith("#"):
            instances.append(resolve_instance(Path(line)))
    return instances


def select_instances(args: argparse.Namespace) -> list[Path]:
    if args.instances:
        instances = [resolve_instance(path) for path in args.instances]
    else:
        instances = read_batch_list(args.batch_list or DEFAULT_BATCH)
    if not instances:
        raise ValueError("the experiment contains no instances")
    if len({path.resolve() for path in instances}) != len(instances):
        raise ValueError("the experiment contains duplicate instances")
    return instances


def parse_int(value: str) -> int:
    return int(value) if value else 0


def parse_float(value: str) -> float:
    return float(value) if value else math.nan


def costs_match(actual: float, expected: float) -> bool:
    return math.isfinite(actual) and math.isfinite(expected) and math.isclose(
        actual, expected, rel_tol=1e-9, abs_tol=1e-8)


def run_batch(
    solver: Path,
    config: ExperimentConfig,
    repeat: int,
    instances: list[Path],
    batch_list: Path,
    timeout: float,
    exact_max_n: int,
) -> tuple[list[dict[str, Any]], float]:
    command = [
        str(solver),
        *config.solver_args(),
        "--exact-max-n", str(exact_max_n),
        "--batch", str(batch_list),
    ]
    started = time.perf_counter()
    completed = subprocess.run(
        command,
        cwd=PROJECT_ROOT,
        check=False,
        text=True,
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE,
        timeout=timeout,
    )
    wall_seconds = time.perf_counter() - started
    rows = list(csv.DictReader(io.StringIO(completed.stdout)))
    if completed.returncode != 0 or len(rows) != len(instances):
        details = completed.stderr.strip() or completed.stdout.strip()
        raise RuntimeError(
            f"{config.config_id} repeat {repeat} failed with status "
            f"{completed.returncode}: {details}")

    expected_paths = {str(path.resolve()) for path in instances}
    actual_paths = {str(Path(row["instance"]).resolve()) for row in rows}
    if actual_paths != expected_paths:
        raise RuntimeError(
            f"{config.config_id} returned a different instance set")

    parsed_rows: list[dict[str, Any]] = []
    for row in rows:
        parsed: dict[str, Any] = {
            "config_id": config.config_id,
            "category": config.category,
            "repeat": repeat,
            "instance": str(Path(row["instance"]).resolve()),
            "status": row["status"],
            "batch_wall_seconds": wall_seconds,
        }
        for field in INTEGER_FIELDS:
            parsed[field] = parse_int(row[field])
        for field in FLOAT_FIELDS:
            parsed[field] = parse_float(row[field])
        parsed_rows.append(parsed)
    return parsed_rows, wall_seconds


def median(values: Iterable[float | int]) -> float:
    return float(statistics.median(list(values)))


def annotate_correctness(raw_rows: list[dict[str, Any]]) -> None:
    baseline_cost: dict[str, float] = {}
    for row in raw_rows:
        if row["config_id"] == "P0" and row["status"] == "ok":
            baseline_cost.setdefault(row["instance"], row["cost"])

    for row in raw_rows:
        name = Path(row["instance"]).name
        if name in KNOWN_OPTIMA:
            expected = KNOWN_OPTIMA[name]
            source = "known-optimum"
        else:
            expected = baseline_cost.get(row["instance"], math.nan)
            source = "P0-cross-check"
        row["expected_cost"] = expected
        row["reference_source"] = source
        row["cost_correct"] = (
            row["status"] == "ok" and costs_match(row["cost"], expected))


def aggregate_instances(raw_rows: list[dict[str, Any]]) -> list[dict[str, Any]]:
    grouped: dict[tuple[str, str], list[dict[str, Any]]] = {}
    for row in raw_rows:
        grouped.setdefault((row["config_id"], row["instance"]), []).append(row)

    results: list[dict[str, Any]] = []
    median_fields = (*INTEGER_FIELDS, *FLOAT_FIELDS)
    for (config_id, instance), rows in grouped.items():
        result: dict[str, Any] = {
            "config_id": config_id,
            "instance": instance,
            "repeats": len(rows),
            "status": "ok" if all(row["status"] == "ok" for row in rows) else "error",
            "expected_cost": rows[0]["expected_cost"],
            "reference_source": rows[0]["reference_source"],
            "cost_correct": all(row["cost_correct"] for row in rows),
        }
        for field in median_fields:
            result[field] = median(row[field] for row in rows)
        result["instance_wall_median_seconds"] = result.pop(
            "instance_wall_seconds")
        results.append(result)

    baseline = {
        row["instance"]: row for row in results if row["config_id"] == "P0"
    }
    for row in results:
        base = baseline[row["instance"]]
        row["expanded_ratio_vs_p0"] = (
            row["nodes_expanded"] + 1.0) / (base["nodes_expanded"] + 1.0)
        row["instance_wall_ratio_vs_p0"] = safe_ratio(
            row["instance_wall_median_seconds"],
            base["instance_wall_median_seconds"])
    return sorted(results, key=lambda row: (row["config_id"], row["instance"]))


def safe_ratio(numerator: float, denominator: float) -> float:
    return numerator / denominator if denominator != 0.0 else math.nan


def aggregate_summary(
    instance_rows: list[dict[str, Any]],
    batch_wall: dict[str, list[float]],
    selected_configs: list[ExperimentConfig],
) -> list[dict[str, Any]]:
    grouped = {
        config.config_id: [
            row for row in instance_rows if row["config_id"] == config.config_id
        ]
        for config in selected_configs
    }
    baseline = {row["instance"]: row for row in grouped["P0"]}
    baseline_total_expanded = sum(
        row["nodes_expanded"] for row in grouped["P0"])
    baseline_wall = median(batch_wall["P0"])

    summaries: list[dict[str, Any]] = []
    for config in selected_configs:
        rows = grouped[config.config_id]
        total_created = sum(row["nodes_created"] for row in rows)
        total_expanded = sum(row["nodes_expanded"] for row in rows)
        attempts = sum(row["potential_updates_attempted"] for row in rows)
        improved = sum(row["potential_updates_improved"] for row in rows)
        direct_pruned = sum(row["potential_updates_pruned"] for row in rows)
        iterations = sum(row["potential_update_iterations"] for row in rows)
        gain = sum(row["potential_update_total_gain"] for row in rows)
        potential_seconds = sum(row["potential_update_seconds"] for row in rows)
        wall = median(batch_wall[config.config_id])

        ratios = [row["expanded_ratio_vs_p0"] for row in rows]
        instance_wall_ratios = [
            row["instance_wall_ratio_vs_p0"] for row in rows]
        wins = sum(
            row["nodes_expanded"] < baseline[row["instance"]]["nodes_expanded"]
            for row in rows)
        ties = sum(
            row["nodes_expanded"] == baseline[row["instance"]]["nodes_expanded"]
            for row in rows)
        losses = len(rows) - wins - ties
        summaries.append({
            "config_id": config.config_id,
            "correct_instances": sum(row["cost_correct"] for row in rows),
            "total_instances": len(rows),
            "nodes_created": total_created,
            "nodes_expanded": total_expanded,
            "expanded_ratio_vs_p0": safe_ratio(
                total_expanded, baseline_total_expanded),
            "expanded_geomean_ratio_vs_p0": math.exp(
                sum(math.log(value) for value in ratios) / len(ratios)),
            "wins": wins,
            "ties": ties,
            "losses": losses,
            "batch_wall_median_seconds": wall,
            "wall_ratio_vs_p0": safe_ratio(wall, baseline_wall),
            "instance_wall_total_seconds": sum(
                row["instance_wall_median_seconds"] for row in rows),
            "instance_wall_geomean_ratio_vs_p0": math.exp(
                sum(math.log(value) for value in instance_wall_ratios)
                / len(instance_wall_ratios)),
            "potential_updates_attempted": attempts,
            "improvement_rate": safe_ratio(improved, attempts),
            "direct_prune_rate": safe_ratio(direct_pruned, attempts),
            "average_iterations_per_attempt": safe_ratio(iterations, attempts),
            "epoch_rebuilds": sum(
                row["potential_updates_rebuilt"] for row in rows),
            "gain_per_iteration": safe_ratio(gain, iterations),
            "potential_update_seconds": potential_seconds,
            "potential_time_share": safe_ratio(potential_seconds, wall),
        })
    return summaries


def write_csv(path: Path, fields: Iterable[str], rows: Iterable[dict[str, Any]]) -> None:
    field_list = list(fields)
    with path.open("w", encoding="utf-8", newline="") as target:
        writer = csv.DictWriter(
            target,
            fieldnames=field_list,
            extrasaction="ignore",
            lineterminator="\n",
        )
        writer.writeheader()
        writer.writerows(rows)


def format_number(value: Any, digits: int = 3) -> str:
    if isinstance(value, bool):
        return "yes" if value else "no"
    if isinstance(value, float):
        if not math.isfinite(value):
            return "-"
        return f"{value:.{digits}f}"
    return str(value)


def write_markdown(
    path: Path,
    configs: list[ExperimentConfig],
    summaries: list[dict[str, Any]],
    instance_rows: list[dict[str, Any]],
) -> None:
    lines = [
        "# PHKMST parameter ablation",
        "",
        "## Configurations",
        "",
        "| ID | Root | Node | Trigger | Depth | Gap | Branch order |",
        "|---|---|---|---|---:|---:|---|",
    ]
    for config in configs:
        lines.append(
            f"| {config.config_id} | {config.root_ascent} | "
            f"{config.effective_node_ascent} | {config.potential_update} | "
            f"{config.update_depth if config.potential_update != 'none' else '-'} | "
            f"{format_number(config.update_gap_ratio, 3) if config.potential_update != 'none' else '-'} | "
            f"{config.branch_edge_order} |")

    lines.extend([
        "",
        "## Aggregate results",
        "",
        "| ID | Correct | Created | Expanded | Exp/P0 | Geo/P0 | W/T/L | Batch wall(s) | Wall/P0 | Inst wall geo/P0 | Improve | Direct prune |",
        "|---|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|",
    ])
    for row in summaries:
        lines.append(
            f"| {row['config_id']} | {row['correct_instances']}/{row['total_instances']} | "
            f"{format_number(row['nodes_created'], 0)} | "
            f"{format_number(row['nodes_expanded'], 0)} | "
            f"{format_number(row['expanded_ratio_vs_p0'])} | "
            f"{format_number(row['expanded_geomean_ratio_vs_p0'])} | "
            f"{row['wins']}/{row['ties']}/{row['losses']} | "
            f"{format_number(row['batch_wall_median_seconds'])} | "
            f"{format_number(row['wall_ratio_vs_p0'])} | "
            f"{format_number(row['instance_wall_geomean_ratio_vs_p0'])} | "
            f"{format_number(row['improvement_rate'])} | "
            f"{format_number(row['direct_prune_rate'])} |")
    lines.extend([
        "",
        "`Exp/P0` is the ratio of aggregate expanded nodes. `Geo/P0` is the "
        "geometric mean of per-instance `(expanded+1)/(P0 expanded+1)`. "
        "`W/T/L` compares expanded nodes with P0.",
        "",
        "## Per-instance results",
        "",
        "| ID | Instance | Correct | Expanded | Exp/P0 | Wall(s) | Wall/P0 |",
        "|---|---|---:|---:|---:|---:|---:|",
    ])
    config_order = {
        config.config_id: index for index, config in enumerate(configs)}
    for row in sorted(
        instance_rows,
        key=lambda value: (
            config_order[value["config_id"]], Path(value["instance"]).name),
    ):
        lines.append(
            f"| {row['config_id']} | {Path(row['instance']).name} | "
            f"{format_number(row['cost_correct'])} | "
            f"{format_number(row['nodes_expanded'], 0)} | "
            f"{format_number(row['expanded_ratio_vs_p0'])} | "
            f"{format_number(row['instance_wall_median_seconds'], 6)} | "
            f"{format_number(row['instance_wall_ratio_vs_p0'])} |")
    lines.append("")
    path.write_text("\n".join(lines), encoding="utf-8")


def config_rows(configs: list[ExperimentConfig]) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for config in configs:
        row = asdict(config)
        row["effective_node_ascent"] = config.effective_node_ascent
        row["command_args"] = " ".join(config.solver_args())
        rows.append(row)
    return rows


def main() -> int:
    args = parse_args()
    selected_configs = [CONFIGS[name] for name in SUITES[args.suite]]
    if args.list_configs:
        for config in selected_configs:
            print(f"{config.config_id}: {' '.join(config.solver_args())}")
        return 0

    solver = args.solver.resolve()
    if not solver.is_file():
        raise FileNotFoundError(f"solver not found: {solver}")
    instances = select_instances(args)
    output_dir = args.output_dir.resolve()
    output_dir.mkdir(parents=True, exist_ok=True)

    raw_rows: list[dict[str, Any]] = []
    batch_wall: dict[str, list[float]] = {
        config.config_id: [] for config in selected_configs
    }
    with tempfile.NamedTemporaryFile(
        mode="w", encoding="utf-8", prefix="phkmst-ablation-",
        suffix=".txt", delete=False,
    ) as batch_file:
        for instance in instances:
            batch_file.write(f"{instance}\n")
        temporary_batch = Path(batch_file.name)

    try:
        for repeat in range(1, args.repeats + 1):
            # Reverse every second pass to reduce fixed-order timing bias.
            ordered = selected_configs if repeat % 2 else list(reversed(selected_configs))
            for config in ordered:
                print(
                    f"[{repeat}/{args.repeats}] {config.config_id} "
                    f"({len(instances)} instances)", flush=True)
                rows, wall = run_batch(
                    solver, config, repeat, instances, temporary_batch,
                    args.timeout, args.exact_max_n)
                raw_rows.extend(rows)
                batch_wall[config.config_id].append(wall)
    finally:
        temporary_batch.unlink(missing_ok=True)

    annotate_correctness(raw_rows)
    instance_rows = aggregate_instances(raw_rows)
    summaries = aggregate_summary(instance_rows, batch_wall, selected_configs)

    configuration_fields = (
        "config_id", "category", "root_ascent", "effective_node_ascent",
        "node_ascent", "potential_update", "update_depth", "update_gap_ratio",
        "update_iterations", "update_budget", "branch_edge_order",
        "description", "command_args",
    )
    write_csv(output_dir / "configurations.csv", configuration_fields,
              config_rows(selected_configs))
    write_csv(output_dir / "raw_runs.csv", RAW_FIELDS, raw_rows)
    write_csv(output_dir / "results_by_instance.csv", INSTANCE_FIELDS, instance_rows)
    write_csv(output_dir / "summary.csv", SUMMARY_FIELDS, summaries)
    write_markdown(
        output_dir / "summary.md", selected_configs, summaries, instance_rows)

    failures = [row for row in instance_rows if not row["cost_correct"]]
    print(f"Wrote experiment tables to {output_dir}")
    print(f"Correctness: {len(instance_rows) - len(failures)}/{len(instance_rows)}")
    if failures:
        for row in failures:
            print(
                f"[FAIL] {row['config_id']} {row['instance']}: "
                f"cost={row['cost']} expected={row['expected_cost']}")
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
