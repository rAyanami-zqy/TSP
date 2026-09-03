#!/usr/bin/env python3
"""Summarize outputs produced by ``run_phkmst_ablation.py``.

The runner deliberately makes its result schema and strategy list extensible.
This program follows the same model: it discovers run directories, result
columns, repeats, and one-factor comparisons from the CSV files instead of
depending on a hard-coded list of strategy names or statistics.

The generated analysis contains:

* ``configurations.csv``: every discovered run and its flattened options;
* ``instance_details.csv``: one aggregated row per run and instance;
* ``pairwise_summary.csv``: long-form metrics for every comparison;
* ``逐实例组合表/*.csv``: full per-instance data for each comparison;
* an HTML report and a Markdown report.

By default, comparisons are the minimal star for every one-factor experiment.
Use ``--compare A:B`` for explicit pairs, ``--baseline A`` to compare one run
with all others, or ``--all-pairs`` when configurations differ in many fields.
Selectors may be a strategy name (when unique) or an exact run id.
"""

from __future__ import annotations

import argparse
import csv
import html
import math
import re
import shlex
import statistics
import sys
from collections import Counter, defaultdict
from dataclasses import dataclass
from itertools import combinations
from pathlib import Path
from typing import Any, Iterable, Sequence


PROJECT_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_INPUT_ROOT = PROJECT_ROOT / "outputs" / "phkmst-ablation"
DEFAULT_OUTPUT_DIR = DEFAULT_INPUT_ROOT / "analysis"

RESULT_METADATA_FIELDS = {
    "run_id", "strategy", "repeat", "instance", "status", "result",
}
SKIPPED_REASON_PREFIX = "search_node_potential_updates_skipped_"
SKIPPED_TOTAL_FIELD = "search_node_potential_updates_skipped_total"
PREFERRED_METRICS = (
    "wall_seconds",
    "branches",
    "nodes_expanded",
    "pruned_by_bound",
    "pruned_infeasible",
    "root_potential_iterations",
    "search_node_potential_update_candidates",
    "search_node_potential_updates_triggered",
    SKIPPED_TOTAL_FIELD,
    "search_node_potential_iterations",
)
DEFAULT_HTML_METRICS = (
    "wall_seconds",
    "branches",
    SKIPPED_TOTAL_FIELD,
)
METRIC_LABELS = {
    "wall_seconds": "运行时间",
    "branches": "分支创建数",
    "nodes_expanded": "展开节点数",
    "pruned_by_bound": "下界剪枝数",
    "pruned_infeasible": "不可行剪枝数",
    "root_potential_iterations": "根势迭代数",
    "search_node_potential_update_candidates": "节点势更新候选数",
    "search_node_potential_updates_triggered": "节点势更新触发数",
    SKIPPED_TOTAL_FIELD: "节点势更新跳过总数",
    "search_node_potential_iterations": "节点势迭代数",
    "search_node_potential_updates_skipped_strategy_none": "跳过：策略关闭",
    "search_node_potential_updates_skipped_update_depth_zero": "跳过：更新深度为零",
    "search_node_potential_updates_skipped_budget_exhausted": "跳过：预算耗尽",
    "search_node_potential_updates_skipped_numerically_unsafe": "跳过：数值不安全",
    "search_node_potential_updates_skipped_invalid_state": "跳过：状态无效",
    "search_node_potential_updates_skipped_zero_violation": "跳过：无度违规",
    "search_node_potential_updates_skipped_zero_iteration_limit": "跳过：迭代上限为零",
    "search_node_potential_updates_skipped_depth_interval": "跳过：深度间隔",
    "search_node_potential_updates_skipped_gap_below_minimum": "跳过：gap 低于下限",
    "search_node_potential_updates_skipped_gap_above_maximum": "跳过：gap 高于上限",
}
MISSING_FACTOR = "<未设置>"


@dataclass(frozen=True)
class InstanceResult:
    name: str
    source: str
    status: str
    observed_repeats: int
    expected_repeats: int
    successful_repeats: int
    timeouts: int
    errors: int
    result: float | None
    result_consistent: bool
    numeric: dict[str, float | None]
    text: dict[str, str]


@dataclass
class Run:
    directory: Path
    run_id: str
    strategy: str
    kind: str
    category: str
    description: str
    executable_sha256: str
    options: dict[str, str]
    timeout_seconds: float | None
    workers: int | None
    repeats: int
    declared_instance_count: int | None
    configuration: dict[str, str]
    instances: dict[str, InstanceResult]
    numeric_fields: tuple[str, ...]
    text_fields: tuple[str, ...]
    label: str = ""

    @property
    def selector(self) -> str:
        return self.run_id


@dataclass(frozen=True)
class Comparison:
    left: Run
    right: Run
    factor: str
    left_value: str
    right_value: str


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--input-root", type=Path, default=DEFAULT_INPUT_ROOT)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument(
        "--runs", nargs="+", metavar="RUN",
        help="only include these strategy names or run ids",
    )
    parser.add_argument(
        "--reference", default="Concorde",
        help="correctness reference strategy/run id; use 'none' to disable",
    )
    parser.add_argument(
        "--compare", action="append", default=[], metavar="LEFT:RIGHT",
        help="explicit comparison; may be repeated",
    )
    parser.add_argument(
        "--baseline", metavar="RUN",
        help="compare this strategy/run id with every other non-reference run",
    )
    parser.add_argument(
        "--all-pairs", action="store_true",
        help="compare all non-reference runs instead of detecting one-factor pairs",
    )
    parser.add_argument(
        "--metrics", nargs="+", metavar="COLUMN",
        help="result columns shown in report summary tables; CSV files always contain all",
    )
    parser.add_argument(
        "--list-runs", action="store_true",
        help="list discovered run selectors and exit",
    )
    return parser.parse_args()


def read_csv(path: Path) -> tuple[list[str], list[dict[str, str]]]:
    with path.open(encoding="utf-8-sig", newline="") as source:
        reader = csv.DictReader(source)
        if reader.fieldnames is None:
            raise ValueError(f"CSV 缺少表头：{path}")
        return list(reader.fieldnames), list(reader)


def finite_float(value: Any) -> float | None:
    if value in (None, ""):
        return None
    try:
        number = float(value)
    except (TypeError, ValueError):
        return None
    return number if math.isfinite(number) else None


def finite_int(value: Any) -> int | None:
    number = finite_float(value)
    if number is None or not number.is_integer():
        return None
    return int(number)


def median(values: Iterable[float | None]) -> float | None:
    available = [value for value in values if value is not None]
    return statistics.median(available) if available else None


def parse_options(raw_args: str) -> dict[str, str]:
    """Parse a displayed command line without evaluating shell syntax."""
    tokens = shlex.split(raw_args or "")
    options: dict[str, str] = {}
    positional = 0
    index = 0
    while index < len(tokens):
        token = tokens[index]
        if token.startswith("-"):
            if "=" in token:
                name, value = token.split("=", 1)
                options[name] = value
                index += 1
                continue
            if index + 1 < len(tokens) and not tokens[index + 1].startswith("--"):
                options[token] = tokens[index + 1]
                index += 2
                continue
            options[token] = "启用"
            index += 1
            continue
        options[f"<位置参数{positional}>"] = token
        positional += 1
        index += 1
    return options


def status_for(rows: Sequence[dict[str, str]], expected_repeats: int) -> str:
    statuses = [row.get("status", "error") for row in rows]
    if len(rows) < expected_repeats:
        return "partial"
    if statuses and all(status == "ok" for status in statuses):
        return "ok"
    if statuses and all(status == "timeout" for status in statuses):
        return "timeout"
    if statuses and all(status == "error" for status in statuses):
        return "error"
    return "mixed"


def aggregate_instance(
    name: str,
    rows: Sequence[dict[str, str]],
    expected_repeats: int,
    numeric_fields: Sequence[str],
    text_fields: Sequence[str],
) -> InstanceResult:
    successful = [row for row in rows if row.get("status") == "ok"]
    numeric: dict[str, float | None] = {}
    for field in numeric_fields:
        source_rows = successful
        # A timeout still has a useful externally measured wall time.
        if field == "wall_seconds" and not source_rows:
            source_rows = list(rows)
        numeric[field] = median(finite_float(row.get(field)) for row in source_rows)

    text_values: dict[str, str] = {}
    for field in text_fields:
        values = sorted({row.get(field, "") for row in successful if row.get(field, "")})
        text_values[field] = " | ".join(values)

    results = [
        value for value in (finite_float(row.get("result")) for row in successful)
        if value is not None
    ]
    result_consistent = all(
        math.isclose(value, results[0], rel_tol=0.0, abs_tol=1e-6)
        for value in results[1:]
    ) if results else True
    sources = sorted({row.get("instance", name) for row in rows})
    return InstanceResult(
        name=name,
        source=" | ".join(sources),
        status=status_for(rows, expected_repeats),
        observed_repeats=len(rows),
        expected_repeats=expected_repeats,
        successful_repeats=len(successful),
        timeouts=sum(row.get("status") == "timeout" for row in rows),
        errors=sum(row.get("status") == "error" for row in rows),
        result=median(results),
        result_consistent=result_consistent,
        numeric=numeric,
        text=text_values,
    )


def load_run(directory: Path) -> Run:
    configuration_path = directory / "configuration.csv"
    results_path = directory / "results.csv"
    _, configurations = read_csv(configuration_path)
    if len(configurations) != 1:
        raise ValueError(f"配置文件必须只有一行：{configuration_path}")
    configuration = configurations[0]
    result_fields, raw_rows = read_csv(results_path)

    skipped_reason_fields = [
        field for field in result_fields
        if field.startswith(SKIPPED_REASON_PREFIX) and field != SKIPPED_TOTAL_FIELD
    ]
    if skipped_reason_fields:
        for row in raw_rows:
            values = [finite_float(row.get(field)) for field in skipped_reason_fields]
            row[SKIPPED_TOTAL_FIELD] = (
                str(sum(value for value in values if value is not None))
                if any(value is not None for value in values) else ""
            )
        if SKIPPED_TOTAL_FIELD not in result_fields:
            result_fields.append(SKIPPED_TOTAL_FIELD)

    candidate_fields = [
        field for field in result_fields if field not in RESULT_METADATA_FIELDS
    ]
    numeric_fields: list[str] = []
    text_fields: list[str] = []
    for field in candidate_fields:
        values = [row.get(field, "") for row in raw_rows if row.get(field, "") != ""]
        if not values or all(finite_float(value) is not None for value in values):
            numeric_fields.append(field)
        else:
            text_fields.append(field)

    grouped: dict[str, list[dict[str, str]]] = defaultdict(list)
    seen_repeats: set[tuple[str, str]] = set()
    for row_number, row in enumerate(raw_rows, 2):
        raw_instance = row.get("instance", "")
        if not raw_instance:
            raise ValueError(f"结果第 {row_number} 行缺少 instance：{results_path}")
        name = Path(raw_instance).name
        repeat = row.get("repeat", "1")
        key = (name, repeat)
        if key in seen_repeats:
            raise ValueError(
                f"同一实例和 repeat 重复：{name}, repeat={repeat}（{results_path}）")
        seen_repeats.add(key)
        grouped[name].append(row)

    configured_repeats = finite_int(configuration.get("repeats"))
    observed_repeat = max(
        (finite_int(row.get("repeat")) or 1 for row in raw_rows), default=1)
    expected_repeats = configured_repeats or observed_repeat
    instances = {
        name: aggregate_instance(
            name, rows, expected_repeats, numeric_fields, text_fields)
        for name, rows in sorted(grouped.items())
    }
    return Run(
        directory=directory,
        run_id=configuration.get("run_id") or directory.name,
        strategy=configuration.get("strategy") or directory.name,
        kind=configuration.get("kind", ""),
        category=configuration.get("category", ""),
        description=configuration.get("description", ""),
        executable_sha256=configuration.get("executable_sha256", ""),
        options=parse_options(configuration.get("solver_args", "")),
        timeout_seconds=finite_float(configuration.get("timeout_seconds")),
        workers=finite_int(configuration.get("workers")),
        repeats=expected_repeats,
        declared_instance_count=finite_int(configuration.get("instance_count")),
        configuration=configuration,
        instances=instances,
        numeric_fields=tuple(numeric_fields),
        text_fields=tuple(text_fields),
    )


def load_runs(input_root: Path) -> tuple[list[Run], list[str]]:
    if not input_root.is_dir():
        raise ValueError(f"输入目录不存在：{input_root}")
    runs: list[Run] = []
    warnings: list[str] = []
    for directory in sorted(input_root.iterdir()):
        if not directory.is_dir():
            continue
        configuration_path = directory / "configuration.csv"
        results_path = directory / "results.csv"
        if not configuration_path.is_file():
            continue
        if not results_path.is_file():
            warnings.append(f"跳过缺少 results.csv 的运行目录：{directory.name}")
            continue
        runs.append(load_run(directory))
    if not runs:
        raise ValueError(f"在 {input_root} 下没有找到完整的运行目录")

    run_ids = [run.run_id for run in runs]
    duplicates = sorted(name for name, count in Counter(run_ids).items() if count > 1)
    if duplicates:
        raise ValueError(f"run_id 重复：{', '.join(duplicates)}")
    strategy_counts = Counter(run.strategy for run in runs)
    for run in runs:
        run.label = (
            run.strategy if strategy_counts[run.strategy] == 1
            else f"{run.strategy} [{run.run_id}]"
        )
    return runs, warnings


def resolve_run(runs: Sequence[Run], selector: str) -> Run:
    exact = [run for run in runs if selector in {run.run_id, run.label}]
    if len(exact) == 1:
        return exact[0]
    by_strategy = [run for run in runs if run.strategy == selector]
    if len(by_strategy) == 1:
        return by_strategy[0]
    if len(by_strategy) > 1:
        choices = ", ".join(run.run_id for run in by_strategy)
        raise ValueError(f"策略 {selector!r} 有多个运行，请改用 run_id：{choices}")
    raise ValueError(f"找不到运行：{selector}")


def filter_runs(runs: Sequence[Run], selectors: Sequence[str] | None) -> list[Run]:
    if not selectors:
        return list(runs)
    selected: list[Run] = []
    for selector in selectors:
        run = resolve_run(runs, selector)
        if run not in selected:
            selected.append(run)
    return selected


def factor_map(run: Run, factor_keys: Sequence[str]) -> dict[str, str]:
    values = {key: run.options.get(key, MISSING_FACTOR) for key in factor_keys}
    values["@executable"] = run.executable_sha256 or MISSING_FACTOR
    return values


def control_signature(run: Run) -> tuple[Any, ...]:
    return (run.kind, run.timeout_seconds, run.workers, run.repeats)


def factor_label(factor: str) -> str:
    if factor == "@executable":
        return "求解器版本"
    return factor.removeprefix("--").replace("-", " ")


def shortened_factor_value(factor: str, value: str) -> str:
    if factor == "@executable" and value != MISSING_FACTOR:
        return value[:12]
    return value


def differences(left: Run, right: Run, factor_keys: Sequence[str]) -> list[tuple[str, str, str]]:
    left_factors = factor_map(left, factor_keys)
    right_factors = factor_map(right, factor_keys)
    return [
        (key, left_factors[key], right_factors[key])
        for key in (*factor_keys, "@executable")
        if left_factors[key] != right_factors[key]
    ]


def comparison_from_pair(
    left: Run, right: Run, factor_keys: Sequence[str], fallback: str = "多因素",
) -> Comparison:
    changed = differences(left, right, factor_keys)
    if not changed:
        return Comparison(left, right, "重复运行", "相同配置", "相同配置")
    factor = "、".join(factor_label(item[0]) for item in changed) or fallback
    left_value = "；".join(
        f"{factor_label(key)}={shortened_factor_value(key, value)}"
        for key, value, _ in changed
    )
    right_value = "；".join(
        f"{factor_label(key)}={shortened_factor_value(key, value)}"
        for key, _, value in changed
    )
    return Comparison(left, right, factor, left_value, right_value)


def automatic_comparisons(runs: Sequence[Run]) -> list[Comparison]:
    """Build one compact star for each automatically detected factor group."""
    factor_keys = sorted({key for run in runs for key in run.options})
    factors = {run.run_id: factor_map(run, factor_keys) for run in runs}
    candidates: list[tuple[Run, Run, str]] = []
    for left, right in combinations(runs, 2):
        if control_signature(left) != control_signature(right):
            continue
        changed = [
            key for key in (*factor_keys, "@executable")
            if factors[left.run_id][key] != factors[right.run_id][key]
        ]
        if len(changed) == 1:
            candidates.append((left, right, changed[0]))

    degree = Counter(
        run.run_id for left, right, _ in candidates for run in (left, right))
    order = {run.run_id: index for index, run in enumerate(runs)}
    groups: dict[tuple[str, tuple[tuple[str, str], ...]], set[str]] = defaultdict(set)
    for left, right, factor in candidates:
        context = tuple(
            (key, factors[left.run_id][key])
            for key in (*factor_keys, "@executable") if key != factor
        )
        groups[(factor, context)].update((left.run_id, right.run_id))

    by_id = {run.run_id: run for run in runs}
    comparisons_found: list[Comparison] = []
    for (factor, _), member_ids in sorted(groups.items(), key=lambda item: item[0][0]):
        members = [by_id[run_id] for run_id in member_ids]
        baseline = min(
            members, key=lambda run: (-degree[run.run_id], order[run.run_id]))
        others = sorted(
            (run for run in members if run is not baseline),
            key=lambda run: order[run.run_id],
        )
        for right in others:
            comparisons_found.append(Comparison(
                baseline,
                right,
                factor_label(factor),
                shortened_factor_value(factor, factors[baseline.run_id][factor]),
                shortened_factor_value(factor, factors[right.run_id][factor]),
            ))
    return comparisons_found


def select_reference(runs: Sequence[Run], selector: str) -> tuple[Run | None, str | None]:
    if selector.lower() in {"", "none", "off"}:
        return None, None
    try:
        return resolve_run(runs, selector), None
    except ValueError:
        return None, f"未找到参考配置 {selector!r}；将继续生成报告，但不做参考结果校验"


def build_comparisons(
    args: argparse.Namespace, runs: Sequence[Run], reference: Run | None,
) -> list[Comparison]:
    factor_keys = sorted({key for run in runs for key in run.options})
    comparison_runs = [run for run in runs if run is not reference]
    if args.compare:
        output: list[Comparison] = []
        for specification in args.compare:
            if ":" not in specification:
                raise ValueError(f"--compare 必须采用 LEFT:RIGHT 格式：{specification}")
            left_name, right_name = specification.split(":", 1)
            left = resolve_run(runs, left_name)
            right = resolve_run(runs, right_name)
            if left is right:
                raise ValueError("不能把一个运行与自身比较")
            output.append(comparison_from_pair(left, right, factor_keys))
        return output
    if args.baseline:
        baseline = resolve_run(runs, args.baseline)
        return [
            comparison_from_pair(baseline, run, factor_keys)
            for run in comparison_runs if run is not baseline
        ]
    if args.all_pairs:
        return [
            comparison_from_pair(left, right, factor_keys)
            for left, right in combinations(comparison_runs, 2)
        ]
    output = automatic_comparisons(comparison_runs)
    if not output and len(comparison_runs) > 1:
        baseline = comparison_runs[0]
        output = [
            comparison_from_pair(baseline, run, factor_keys)
            for run in comparison_runs[1:]
        ]
    return output


def metric_order(metrics: Iterable[str]) -> list[str]:
    unique = set(metrics)
    preferred = [metric for metric in PREFERRED_METRICS if metric in unique]
    return preferred + sorted(unique - set(preferred))


def symmetric_change(left: float | None, right: float | None) -> float | None:
    """Return a signed, reciprocal-symmetric fold change.

    Doubling is ``+1`` (+100%), while halving is ``-1`` (-100%).  Reversing
    the comparison therefore only changes the sign.  A one-sided zero has no
    finite fold change; two zeros are treated as unchanged.
    """
    if left is None or right is None or left < 0 or right < 0:
        return None
    if left == right == 0:
        return 0.0
    if left == 0 or right == 0:
        return None
    if right >= left:
        return right / left - 1.0
    return -(left / right - 1.0)


def geometric_symmetric_change(ratios: Sequence[float]) -> float | None:
    usable = [ratio for ratio in ratios if ratio > 0.0]
    if not usable:
        return None
    geometric_ratio = math.exp(statistics.fmean(math.log(ratio) for ratio in usable))
    return symmetric_change(1.0, geometric_ratio)


def result_agreement(left: InstanceResult, right: InstanceResult) -> bool | None:
    if left.result is None or right.result is None:
        return None
    return math.isclose(left.result, right.result, rel_tol=0.0, abs_tol=1e-6)


def reference_results(run: Run, reference: Run | None) -> tuple[int, list[str]]:
    if reference is None or run is reference:
        return 0, []
    checked = 0
    mismatches: list[str] = []
    for name in sorted(set(run.instances) & set(reference.instances)):
        item = run.instances[name]
        expected = reference.instances[name]
        if item.status != "ok" or expected.status != "ok":
            continue
        agreement = result_agreement(item, expected)
        if agreement is None:
            continue
        checked += 1
        if not agreement:
            mismatches.append(name)
    return checked, mismatches


def pair_instances(comparison: Comparison) -> list[str]:
    return sorted(set(comparison.left.instances) | set(comparison.right.instances))


def ordered_pair_instances(
    comparison: Comparison, reference: Run | None,
) -> list[str]:
    """Keep successful rows first and rows containing a timeout at the bottom."""
    def failure_rank(name: str) -> int:
        items = [
            comparison.left.instances.get(name),
            comparison.right.instances.get(name),
        ]
        if reference is not None:
            items.append(reference.instances.get(name))
        statuses = [item.status if item else "missing" for item in items]
        if "timeout" in statuses:
            return 2
        if any(status != "ok" for status in statuses):
            return 1
        return 0

    return sorted(pair_instances(comparison), key=lambda name: (failure_rank(name), name))


def instance_metric(item: InstanceResult | None, metric: str) -> float | None:
    if item is None or item.status != "ok":
        return None
    return item.numeric.get(metric)


def comparable_change(
    left: InstanceResult | None,
    right: InstanceResult | None,
    metric: str,
) -> float | None:
    """Return a change only when both participating runs completed successfully."""
    return symmetric_change(instance_metric(left, metric), instance_metric(right, metric))


def csv_value(value: Any) -> Any:
    return "" if value is None else value


def pair_metric_statistics(comparison: Comparison, metric: str) -> dict[str, Any]:
    pairs: list[tuple[str, float, float]] = []
    raw_ratios: list[float] = []
    changes: list[float] = []
    for name in pair_instances(comparison):
        left = comparison.left.instances.get(name)
        right = comparison.right.instances.get(name)
        if left is None or right is None or left.status != "ok" or right.status != "ok":
            continue
        left_value = left.numeric.get(metric)
        right_value = right.numeric.get(metric)
        if left_value is None or right_value is None:
            continue
        pairs.append((name, left_value, right_value))
        change = symmetric_change(left_value, right_value)
        if change is not None:
            changes.append(change)
        if left_value > 0 and right_value > 0:
            raw_ratios.append(right_value / left_value)
    epsilon = 1e-12
    left_total = sum(left for _, left, _ in pairs)
    right_total = sum(right for _, _, right in pairs)
    return {
        "paired_values": len(pairs),
        "left_median": median(left for _, left, _ in pairs),
        "right_median": median(right for _, _, right in pairs),
        "median_symmetric_change": statistics.median(changes) if changes else None,
        "geometric_symmetric_change": geometric_symmetric_change(raw_ratios),
        "aggregate_symmetric_change": (
            symmetric_change(left_total, right_total) if pairs else None),
        "decreased": sum(change < -epsilon for change in changes),
        "tied": sum(abs(change) <= epsilon for change in changes),
        "increased": sum(change > epsilon for change in changes),
    }


def coverage_statistics(comparison: Comparison) -> dict[str, Any]:
    names = pair_instances(comparison)
    left_ok = {name for name in names if comparison.left.instances.get(name) and
               comparison.left.instances[name].status == "ok"}
    right_ok = {name for name in names if comparison.right.instances.get(name) and
                comparison.right.instances[name].status == "ok"}
    agreements = 0
    mismatches: list[str] = []
    for name in sorted(left_ok & right_ok):
        agreement = result_agreement(
            comparison.left.instances[name], comparison.right.instances[name])
        if agreement is True:
            agreements += 1
        elif agreement is False:
            mismatches.append(name)
    return {
        "left_ok": len(left_ok),
        "right_ok": len(right_ok),
        "common_ok": len(left_ok & right_ok),
        "left_only": sorted(left_ok - right_ok),
        "right_only": sorted(right_ok - left_ok),
        "result_agreements": agreements,
        "result_mismatches": mismatches,
    }


def all_metrics(runs: Sequence[Run]) -> list[str]:
    return metric_order(field for run in runs for field in run.numeric_fields)


def configuration_rows(runs: Sequence[Run]) -> tuple[list[str], list[dict[str, Any]]]:
    config_fields = sorted({field for run in runs for field in run.configuration})
    option_fields = sorted({field for run in runs for field in run.options})
    leading = ["directory", "selector", "label", "loaded_instances", "numeric_fields"]
    fields = leading + config_fields + [f"option:{field}" for field in option_fields]
    rows: list[dict[str, Any]] = []
    for run in runs:
        row: dict[str, Any] = {
            "directory": str(run.directory),
            "selector": run.selector,
            "label": run.label,
            "loaded_instances": len(run.instances),
            "numeric_fields": " ".join(run.numeric_fields),
            **run.configuration,
        }
        row.update({f"option:{field}": run.options.get(field, "") for field in option_fields})
        rows.append(row)
    return fields, rows


def instance_detail_rows(
    runs: Sequence[Run], metrics: Sequence[str], text_fields: Sequence[str],
) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for run in runs:
        for item in run.instances.values():
            row: dict[str, Any] = {
                "run_id": run.run_id,
                "strategy": run.strategy,
                "label": run.label,
                "instance": item.name,
                "source": item.source,
                "status": item.status,
                "observed_repeats": item.observed_repeats,
                "expected_repeats": item.expected_repeats,
                "successful_repeats": item.successful_repeats,
                "timeouts": item.timeouts,
                "errors": item.errors,
                "result": item.result if item.result is not None else "",
                "result_consistent": item.result_consistent,
            }
            row.update({metric: item.numeric.get(metric, "") for metric in metrics})
            row.update({field: item.text.get(field, "") for field in text_fields})
            rows.append(row)
    return rows


def pair_summary_rows(
    comparisons_found: Sequence[Comparison], metrics: Sequence[str], reference: Run | None,
) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    for index, comparison in enumerate(comparisons_found, 1):
        coverage = coverage_statistics(comparison)
        left_checked, left_reference_mismatches = reference_results(
            comparison.left, reference)
        right_checked, right_reference_mismatches = reference_results(
            comparison.right, reference)
        available = set(comparison.left.numeric_fields) & set(comparison.right.numeric_fields)
        for metric in metrics:
            if metric not in available:
                continue
            row = {
                "comparison": index,
                "factor": comparison.factor,
                "left": comparison.left.label,
                "right": comparison.right.label,
                "left_value": comparison.left_value,
                "right_value": comparison.right_value,
                **coverage,
                "left_reference_checked": left_checked,
                "left_reference_mismatches": ";".join(left_reference_mismatches),
                "right_reference_checked": right_checked,
                "right_reference_mismatches": ";".join(right_reference_mismatches),
                "metric": metric,
                **pair_metric_statistics(comparison, metric),
            }
            row["left_only"] = ";".join(coverage["left_only"])
            row["right_only"] = ";".join(coverage["right_only"])
            row["result_mismatches"] = ";".join(coverage["result_mismatches"])
            rows.append(row)
    return rows


def pair_detail_rows(
    comparison: Comparison, metrics: Sequence[str], reference: Run | None,
) -> list[dict[str, Any]]:
    rows: list[dict[str, Any]] = []
    available_set = set(comparison.left.numeric_fields) | set(comparison.right.numeric_fields)
    available = [metric for metric in metrics if metric in available_set]
    for name in ordered_pair_instances(comparison, reference):
        left = comparison.left.instances.get(name)
        right = comparison.right.instances.get(name)
        expected = reference.instances.get(name) if reference else None
        row: dict[str, Any] = {"instance": name}
        if reference:
            row.update({
                f"{reference.label}:status": expected.status if expected else "missing",
                f"{reference.label}:wall_seconds": csv_value(
                    instance_metric(expected, "wall_seconds")),
                f"{reference.label}:result": (
                    expected.result if expected and expected.status == "ok"
                    and expected.result is not None else ""),
                f"{reference.label}:branches": csv_value(
                    instance_metric(expected, "branches")),
            })
        row.update({
            f"{comparison.left.label}:status": left.status if left else "missing",
            f"{comparison.left.label}:successful_repeats": left.successful_repeats if left else 0,
            f"{comparison.left.label}:result": (
                left.result if left and left.status == "ok" and left.result is not None else ""),
            f"{comparison.right.label}:status": right.status if right else "missing",
            f"{comparison.right.label}:successful_repeats": right.successful_repeats if right else 0,
            f"{comparison.right.label}:result": (
                right.result if right and right.status == "ok" and right.result is not None else ""),
            "result_match": (
                result_agreement(left, right)
                if left and right and left.status == right.status == "ok" else ""),
        })
        if reference:
            row.update({
                f"{comparison.left.label}:matches_reference": (
                    result_agreement(left, expected)
                    if left and expected and left.status == expected.status == "ok" else ""),
                f"{comparison.right.label}:matches_reference": (
                    result_agreement(right, expected)
                    if right and expected and right.status == expected.status == "ok" else ""),
                f"{comparison.left.label}:wall_vs_{reference.label}_symmetric_change": (
                    csv_value(comparable_change(expected, left, "wall_seconds"))),
                f"{comparison.right.label}:wall_vs_{reference.label}_symmetric_change": (
                    csv_value(comparable_change(expected, right, "wall_seconds"))),
            })
        for metric in available:
            left_value = instance_metric(left, metric)
            right_value = instance_metric(right, metric)
            row[f"{comparison.left.label}:{metric}"] = left_value if left_value is not None else ""
            row[f"{comparison.right.label}:{metric}"] = right_value if right_value is not None else ""
            if metric in {"wall_seconds", "branches", SKIPPED_TOTAL_FIELD}:
                row[f"symmetric_change:{metric}"] = csv_value(
                    symmetric_change(left_value, right_value))
        rows.append(row)
    return rows


def write_csv(path: Path, rows: Sequence[dict[str, Any]], fields: Sequence[str]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    with path.open("w", encoding="utf-8-sig", newline="") as target:
        writer = csv.DictWriter(target, fieldnames=fields, extrasaction="ignore")
        writer.writeheader()
        writer.writerows(rows)


def metric_label(metric: str) -> str:
    return METRIC_LABELS.get(metric, metric.replace("_", " "))


def format_number(value: float | None) -> str:
    if value is None:
        return "—"
    if value.is_integer():
        return f"{int(value):,}"
    return f"{value:,.6g}"


def format_metric(metric: str, value: float | None) -> str:
    if value is None:
        return "—"
    if metric == "wall_seconds":
        return f"{value * 1000:,.3f} ms" if value < 1 else f"{value:,.3f} s"
    return format_number(value)


def format_change(value: float | None) -> str:
    if value is None:
        return "—"
    if abs(value) < 5e-5:
        return "0.00%"
    return f"{value:+.2%}"


def status_text(status: str) -> str:
    return {
        "ok": "成功",
        "timeout": "超时",
        "error": "错误",
        "mixed": "重复结果混合",
        "partial": "部分完成",
        "missing": "缺失",
    }.get(status, status)


def runtime_text(item: InstanceResult | None) -> str:
    if item is None:
        return "—"
    if item.status == "timeout":
        return "超时"
    if item.status != "ok":
        return status_text(item.status)
    return format_metric("wall_seconds", item.numeric.get("wall_seconds"))


def result_text(item: InstanceResult | None) -> str:
    if item is None or item.status != "ok" or item.result is None:
        return "—"
    return format_number(item.result)


def branches_text(item: InstanceResult | None) -> str:
    if item is None or item.status != "ok":
        return "—"
    return format_metric("branches", item.numeric.get("branches"))


def change_class(value: float | None) -> str:
    if value is None:
        return "missing"
    if value < -1e-12:
        return "down"
    if value > 1e-12:
        return "up"
    return "tie"


def markdown_report(
    input_root: Path,
    runs: Sequence[Run],
    comparisons_found: Sequence[Comparison],
    metrics: Sequence[str],
    reference: Run | None,
    warnings: Sequence[str],
) -> str:
    lines = [
        "# PHKMST 消融实验汇总",
        "",
        f"数据源：`{input_root}`。发现 {len(runs)} 个运行、"
        f"{len(comparisons_found)} 组对比。逐实例完整数据见同目录 CSV 文件。",
        "",
    ]
    if warnings:
        lines.extend(["## 数据提醒", ""] + [f"- {warning}" for warning in warnings] + [""])
    lines.extend([
        "## 运行概览",
        "",
        "| 配置 | run_id | 类型 | 已载入实例 | 成功 | 超时 | 错误/混合/部分 | 重复数 |",
        "| --- | --- | --- | ---: | ---: | ---: | ---: | ---: |",
    ])
    for run in runs:
        statuses = Counter(item.status for item in run.instances.values())
        other = sum(statuses[name] for name in ("error", "mixed", "partial"))
        lines.append(
            f"| {run.label} | `{run.run_id}` | {run.kind or '—'} | {len(run.instances)} | "
            f"{statuses['ok']} | {statuses['timeout']} | {other} | {run.repeats} |"
        )
    lines.extend(["", "## 自动化口径", ""])
    lines.extend([
        "- 多次重复按实例取成功运行的中位数；只要重复缺失或状态不一致，该实例会标记为部分/混合，不进入成对性能统计。",
        "- 默认只生成单因素对比。同一因素有多个水平时，以连接其他配置最多的运行作为基线，避免输出所有冗余组合。",
        "- 百分比采用对称倍数变化：右侧不小时为 `右/左-1`，右侧更小时为 `-(左/右-1)`；2 倍记 `+100%`，一半记 `-100%`，交换方向只改变符号。",
        "- 新增的数值型 results.csv 列会自动进入明细和成对汇总 CSV，无需修改本脚本。",
    ])
    if reference:
        lines.append(f"- 正确性参考：{reference.label}（`{reference.run_id}`）。")
    else:
        lines.append("- 当前没有可用的独立正确性参考；仅报告成对结果是否一致。")
    lines.extend([
        "",
        "## 节点势更新指标含义",
        "",
        "- **节点势更新候选数**：通过前置下界剪枝后，真正到达势更新判定点的非根逻辑搜索节点数；每个节点只计一次，是触发率的分母。",
        "- **节点势更新触发数**：候选节点通过策略、预算、数值安全、深度和 gap 等条件后，实际启动节点势优化的次数。",
        "- **节点势迭代数**：所有已触发更新内部执行的次梯度迭代轮数之和；一次触发最多可执行 `--hk-update-iterations` 轮。",
        "- **节点势更新跳过总数**：所有互斥跳过原因之和。求解器按第一个命中的原因计数，因此应满足 `候选数 = 触发数 + 跳过总数`。",
        "- `--hk-update-budget` 限制的是**每轮根搜索的触发次数**，不是迭代总数；设为 `0` 表示不限。正数预算下，初始探测轮最多使用 `min(budget, 1000)` 次；若 incumbent 改善并重启根搜索，计数器会清零，下一轮可再使用完整预算。因此整个求解的触发数可能超过配置预算，迭代数还会再乘上每次更新的迭代轮数。预算耗尽后访问的候选节点仍会逐个计入“跳过：预算耗尽”，所以跳过数也可能远大于预算。",
        "",
    ])

    for index, comparison in enumerate(comparisons_found, 1):
        coverage = coverage_statistics(comparison)
        left_checked, left_reference_mismatches = reference_results(
            comparison.left, reference)
        right_checked, right_reference_mismatches = reference_results(
            comparison.right, reference)
        lines.extend([
            f"## 对比 {index}：{comparison.left.label} → {comparison.right.label}",
            "",
            f"变化因素：**{comparison.factor}**（{comparison.left_value} → {comparison.right_value}）。",
            "",
            f"成功实例：{comparison.left.label}={coverage['left_ok']}，"
            f"{comparison.right.label}={coverage['right_ok']}，共同成功={coverage['common_ok']}；"
            f"右侧新增={len(coverage['right_only'])}，右侧丢失={len(coverage['left_only'])}。",
            "",
            "| 指标 | 配对数 | 左侧中位数 | 右侧中位数 | 对称变化中位数 | 几何平均对称变化 | 总量对称变化 | 下降/持平/上升 |",
            "| --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: |",
        ])
        available = set(comparison.left.numeric_fields) & set(comparison.right.numeric_fields)
        for metric in metrics:
            if metric not in available:
                continue
            item = pair_metric_statistics(comparison, metric)
            lines.append(
                f"| {metric_label(metric)} | {item['paired_values']} | "
                f"{format_metric(metric, item['left_median'])} | "
                f"{format_metric(metric, item['right_median'])} | "
                f"{format_change(item['median_symmetric_change'])} | "
                f"{format_change(item['geometric_symmetric_change'])} | "
                f"{format_change(item['aggregate_symmetric_change'])} | "
                f"{item['decreased']}/{item['tied']}/{item['increased']} |"
            )
        mismatches = coverage["result_mismatches"]
        lines.extend([
            "",
            "结果一致性：" + (
                "共同成功且有结果的实例均一致。"
                if not mismatches else "不一致实例：" + "、".join(f"`{name}`" for name in mismatches)
            ),
        ])
        if reference:
            reference_mismatches = sorted(set(
                left_reference_mismatches + right_reference_mismatches))
            lines.append(
                f"参考校验：左侧检查 {left_checked} 个，右侧检查 {right_checked} 个；" + (
                    "均与参考结果一致。" if not reference_mismatches
                    else "不一致实例：" + "、".join(
                        f"`{name}`" for name in reference_mismatches)
                )
            )
        lines.append("")
    return "\n".join(lines)


def html_cell(value: str, css_class: str = "") -> str:
    attribute = f' class="{css_class}"' if css_class else ""
    return f"<td{attribute}>{html.escape(value)}</td>"


def html_report(
    input_root: Path,
    runs: Sequence[Run],
    comparisons_found: Sequence[Comparison],
    summary_metrics: Sequence[str],
    reference: Run | None,
    warnings: Sequence[str],
) -> str:
    overview_rows: list[str] = []
    for run in runs:
        statuses = Counter(item.status for item in run.instances.values())
        overview_rows.append(
            "<tr>" + "".join([
                html_cell(run.label), html_cell(run.run_id), html_cell(run.kind or "—"),
                html_cell(str(len(run.instances)), "num"),
                html_cell(str(statuses["ok"]), "num"),
                html_cell(str(statuses["timeout"]), "num"),
                html_cell(str(statuses["partial"] + statuses["mixed"] + statuses["error"]), "num"),
                html_cell(run.description or "—"),
            ]) + "</tr>"
        )

    sections: list[str] = []
    navigation: list[str] = []
    for index, comparison in enumerate(comparisons_found, 1):
        coverage = coverage_statistics(comparison)
        left_checked, left_reference_mismatches = reference_results(
            comparison.left, reference)
        right_checked, right_reference_mismatches = reference_results(
            comparison.right, reference)
        reference_summary = ""
        if reference:
            reference_summary = (
                f"；参考校验 {left_checked + right_checked} 个，不一致 "
                f"{len(set(left_reference_mismatches + right_reference_mismatches))} 个"
            )
        navigation.append(
            f'<a href="#comparison-{index}">{index}. '
            f'{html.escape(comparison.left.label)} → {html.escape(comparison.right.label)}</a>')
        metric_rows: list[str] = []
        available = set(comparison.left.numeric_fields) & set(comparison.right.numeric_fields)
        raw_metrics = [
            metric
            for metric in metric_order(
                set(comparison.left.numeric_fields) | set(comparison.right.numeric_fields))
            if metric not in {"wall_seconds", "branches"}
        ]
        for metric in summary_metrics:
            if metric not in available:
                continue
            item = pair_metric_statistics(comparison, metric)
            change = item["aggregate_symmetric_change"]
            metric_rows.append("<tr>" + "".join([
                html_cell(metric_label(metric)),
                html_cell(str(item["paired_values"]), "num"),
                html_cell(format_metric(metric, item["left_median"]), "num"),
                html_cell(format_metric(metric, item["right_median"]), "num"),
                html_cell(format_change(item["median_symmetric_change"]), "num"),
                html_cell(format_change(item["geometric_symmetric_change"]), "num"),
                html_cell(format_change(change), f"num {change_class(change)}"),
                html_cell(f"{item['decreased']}/{item['tied']}/{item['increased']}", "num"),
            ]) + "</tr>")

        first_header = ['<th rowspan="2">实例</th>']
        second_header: list[str] = []
        if reference:
            first_header.append(
                f'<th colspan="3" class="reference-head">{html.escape(reference.label)}</th>')
            second_header.extend(["运行时间", "求解结果", "分支数"])
        side_columns = (4 if reference else 3) + len(raw_metrics)
        first_header.extend([
            f'<th colspan="{side_columns}" class="left-head">配置 A：'
            f'{html.escape(comparison.left.label)}</th>',
            f'<th colspan="{side_columns}" class="right-head">配置 B：'
            f'{html.escape(comparison.right.label)}</th>',
            '<th colspan="3" class="ratio-head">配置 B 相对配置 A</th>',
        ])
        second_header.append("运行时间")
        if reference:
            second_header.append(f"时间相对 {reference.label}")
        second_header.extend(
            ["求解结果", "分支数", *(metric_label(metric) for metric in raw_metrics)])
        second_header.append("运行时间")
        if reference:
            second_header.append(f"时间相对 {reference.label}")
        second_header.extend(
            ["求解结果", "分支数", *(metric_label(metric) for metric in raw_metrics)])
        second_header.extend(["时间变化", "分支数变化", "跳过总数变化"])

        detail_rows: list[str] = []
        for name in ordered_pair_instances(comparison, reference):
            expected = reference.instances.get(name) if reference else None
            left = comparison.left.instances.get(name)
            right = comparison.right.instances.get(name)
            cells = [html_cell(name)]
            if reference:
                cells.extend([
                    html_cell(runtime_text(expected), "num"),
                    html_cell(result_text(expected), "num"),
                    html_cell(branches_text(expected), "num"),
                ])
            left_reference_change = comparable_change(
                expected, left, "wall_seconds") if reference else None
            right_reference_change = comparable_change(
                expected, right, "wall_seconds") if reference else None
            pair_time_change = comparable_change(left, right, "wall_seconds")
            pair_branch_change = comparable_change(left, right, "branches")
            pair_skipped_change = comparable_change(
                left, right, SKIPPED_TOTAL_FIELD)
            cells.append(html_cell(runtime_text(left), "num"))
            if reference:
                cells.append(html_cell(
                    format_change(left_reference_change),
                    f"num {change_class(left_reference_change)}"))
            cells.extend([
                html_cell(result_text(left), "num"),
                html_cell(branches_text(left), "num"),
            ])
            cells.extend(
                html_cell(
                    format_metric(metric, instance_metric(left, metric)), "num")
                for metric in raw_metrics
            )
            cells.append(html_cell(runtime_text(right), "num"))
            if reference:
                cells.append(html_cell(
                    format_change(right_reference_change),
                    f"num {change_class(right_reference_change)}"))
            cells.extend([
                html_cell(result_text(right), "num"),
                html_cell(branches_text(right), "num"),
            ])
            cells.extend(
                html_cell(
                    format_metric(metric, instance_metric(right, metric)), "num")
                for metric in raw_metrics
            )
            cells.extend([
                html_cell(format_change(pair_time_change),
                          f"num {change_class(pair_time_change)}"),
                html_cell(format_change(pair_branch_change),
                          f"num {change_class(pair_branch_change)}"),
                html_cell(format_change(pair_skipped_change),
                          f"num {change_class(pair_skipped_change)}"),
            ])
            statuses = [left.status if left else "missing", right.status if right else "missing"]
            if expected:
                statuses.append(expected.status)
            row_class = ' class="timeout-row"' if "timeout" in statuses else ""
            detail_rows.append(f"<tr{row_class}>" + "".join(cells) + "</tr>")

        config_cards = "".join(
            '<article class="card">'
            f"<h3>{html.escape(title)}：{html.escape(run.label)}</h3>"
            f"<p>{html.escape(run.description or '无描述')}</p>"
            f"<code>{html.escape(run.configuration.get('solver_args', '') or '（无求解器参数）')}</code>"
            "</article>"
            for title, run in (("左侧", comparison.left), ("右侧", comparison.right))
        )
        sections.append(f"""
<section id="comparison-{index}">
  <h2>对比 {index}：{html.escape(comparison.left.label)} → {html.escape(comparison.right.label)}</h2>
  <p>变化因素：<strong>{html.escape(comparison.factor)}</strong>；
  {html.escape(comparison.left_value)} → {html.escape(comparison.right_value)}。</p>
  <p>共同成功 {coverage['common_ok']}；右侧新增 {len(coverage['right_only'])}；
  右侧丢失 {len(coverage['left_only'])}；结果不一致
  {len(coverage['result_mismatches'])}{reference_summary}。</p>
  <div class="cards">{config_cards}</div>
  <div class="table-wrap"><table><thead><tr>
    <th>指标</th><th>配对数</th><th>左侧中位数</th><th>右侧中位数</th>
    <th>对称变化中位数</th><th>几何平均对称变化</th><th>总量对称变化</th><th>下降/持平/上升</th>
  </tr></thead><tbody>{''.join(metric_rows)}</tbody></table></div>
  <details open><summary>逐实例明细（A/B 原始统计全部展示，仅时间与分支数计算变化）</summary>
  <div class="table-wrap detail"><table><thead>
  <tr>{''.join(first_header)}</tr>
  <tr>{''.join(f'<th>{html.escape(value)}</th>' for value in second_header)}</tr></thead>
  <tbody>{''.join(detail_rows)}</tbody></table></div></details>
</section>""")

    warning_html = "".join(f"<li>{html.escape(item)}</li>" for item in warnings)
    navigation_html = "".join(navigation) or "<span>没有可比较的运行。</span>"
    return f"""<!doctype html>
<html lang="zh-CN"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>PHKMST 消融实验汇总</title>
<style>
:root{{--ink:#172033;--muted:#5f6b7a;--line:#d8dee8;--navy:#29486f}}
*{{box-sizing:border-box}} body{{margin:0;background:#f5f7fa;color:var(--ink);font:13px/1.5 -apple-system,BlinkMacSystemFont,"Segoe UI","Microsoft YaHei",sans-serif}}
main{{max-width:1900px;margin:24px auto;padding:0 18px 60px}} h1{{margin-bottom:4px}} h2{{margin-top:34px;border-bottom:2px solid #aab6c6;padding-bottom:6px}}
.muted,p{{color:var(--muted)}} nav{{display:flex;flex-wrap:wrap;gap:8px;padding:12px;background:white;border:1px solid var(--line);border-radius:8px}}
nav a{{color:#245b9e;text-decoration:none;padding:3px 8px}} .warnings{{background:#fff5da;border-left:4px solid #d39b22;padding:8px 14px}}
.cards{{display:grid;grid-template-columns:repeat(2,minmax(0,1fr));gap:10px;margin:10px 0}} .card{{background:white;border:1px solid var(--line);border-radius:8px;padding:10px}}
.card h3{{margin:0 0 4px}} .card p{{margin:0 0 6px}} code{{white-space:pre-wrap;overflow-wrap:anywhere}}
.table-wrap{{overflow:auto;background:white;border:1px solid var(--line);border-radius:8px;margin:8px 0 14px;max-height:680px}}
table{{border-collapse:separate;border-spacing:0;width:100%}} th,td{{padding:6px 8px;border-right:1px solid var(--line);border-bottom:1px solid var(--line);white-space:nowrap}}
th{{position:sticky;top:0;background:var(--navy);color:white;z-index:1}} thead tr:nth-child(2) th{{top:31px}}
.reference-head{{background:#4a5f7f}} .left-head{{background:#526b42}} .right-head{{background:#76522f}} .ratio-head{{background:#644477}}
.num{{text-align:right;font-variant-numeric:tabular-nums}} .up{{background:#ffc7ce;color:#9c0006}} .down{{background:#c6efce;color:#006100}} .tie{{background:#eef1f5;color:#59616d}} .missing{{background:#f7f8fa;color:#8a93a0}}
.timeout-row td:first-child{{color:#9a5b00;font-weight:700}}
details summary{{cursor:pointer;font-weight:700;margin:10px 0}} .detail table{{min-width:1200px}} section{{scroll-margin-top:8px}}
@media(max-width:900px){{.cards{{grid-template-columns:1fr}}}}
</style></head><body><main>
<h1>PHKMST 消融实验汇总</h1>
<div class="muted">数据源：{html.escape(str(input_root))}；发现 {len(runs)} 个运行，{len(comparisons_found)} 组对比。</div>
<div class="muted">百分比采用对称倍数变化：2 倍为 +100%，一半为 -100%；交换比较方向只改变符号。</div>
{f'<ul class="warnings">{warning_html}</ul>' if warnings else ''}
<h2>运行概览</h2><div class="table-wrap"><table><thead><tr>
<th>配置</th><th>run_id</th><th>类型</th><th>实例</th><th>成功</th><th>超时</th><th>其他</th><th>描述</th>
</tr></thead><tbody>{''.join(overview_rows)}</tbody></table></div>
<h2>节点势更新指标含义</h2>
<ul class="definitions">
<li><strong>节点势更新候选数：</strong>到达势更新判定点的非根逻辑搜索节点数；每个节点只计一次，是触发率分母。</li>
<li><strong>节点势更新触发数：</strong>通过策略、预算、数值安全、深度和 gap 等条件后，实际启动节点势优化的次数。</li>
<li><strong>节点势迭代数：</strong>所有已触发更新内部执行的次梯度迭代轮数之和；一次触发可执行多轮。</li>
<li><strong>节点势更新跳过总数：</strong>所有互斥跳过原因之和；应满足“候选数 = 触发数 + 跳过总数”。</li>
<li><strong>预算口径：</strong><code>--hk-update-budget</code> 限制每轮根搜索的触发次数，不限制迭代总数；设为 <code>0</code> 表示不限。正数预算下，初始探测轮最多使用 <code>min(budget, 1000)</code> 次；incumbent 改善并重启后预算计数清零，下一轮可再使用完整预算。预算耗尽后的候选仍计入跳过数。</li>
</ul>
<h2>对比导航</h2><nav>{navigation_html}</nav>
{''.join(sections)}
</main></body></html>"""


def safe_slug(value: str) -> str:
    cleaned = re.sub(r"[^A-Za-z0-9._-]+", "-", value).strip("-.")
    return cleaned or "run"


def write_outputs(
    input_root: Path,
    output_dir: Path,
    runs: Sequence[Run],
    comparisons_found: Sequence[Comparison],
    reference: Run | None,
    warnings: Sequence[str],
    requested_report_metrics: Sequence[str] | None,
) -> tuple[Path, Path]:
    output_dir.mkdir(parents=True, exist_ok=True)
    metrics = all_metrics(runs)
    text_fields = sorted({field for run in runs for field in run.text_fields})
    report_metrics = (
        [metric for metric in requested_report_metrics if metric in metrics]
        if requested_report_metrics
        else [metric for metric in DEFAULT_HTML_METRICS if metric in metrics]
    )
    if not report_metrics and metrics:
        report_metrics = metrics[:2]

    config_fields, configs = configuration_rows(runs)
    write_csv(output_dir / "configurations.csv", configs, config_fields)

    instance_fields = [
        "run_id", "strategy", "label", "instance", "source", "status",
        "observed_repeats", "expected_repeats", "successful_repeats",
        "timeouts", "errors", "result", "result_consistent",
        *metrics, *text_fields,
    ]
    write_csv(
        output_dir / "instance_details.csv",
        instance_detail_rows(runs, metrics, text_fields),
        instance_fields,
    )

    summary = pair_summary_rows(comparisons_found, report_metrics, reference)
    summary_fields = [
        "comparison", "factor", "left", "right", "left_value", "right_value",
        "left_ok", "right_ok", "common_ok", "left_only", "right_only",
        "result_agreements", "result_mismatches",
        "left_reference_checked", "left_reference_mismatches",
        "right_reference_checked", "right_reference_mismatches",
        "metric", "paired_values",
        "left_median", "right_median", "median_symmetric_change",
        "geometric_symmetric_change", "aggregate_symmetric_change",
        "decreased", "tied", "increased",
    ]
    write_csv(output_dir / "pairwise_summary.csv", summary, summary_fields)

    tables_dir = output_dir / "逐实例组合表"
    tables_dir.mkdir(parents=True, exist_ok=True)
    for pattern in ("对比*.csv", "表*.csv"):
        for stale in tables_dir.glob(pattern):
            stale.unlink()
    for index, comparison in enumerate(comparisons_found, 1):
        rows = pair_detail_rows(comparison, metrics, reference)
        fields = list(rows[0]) if rows else ["instance"]
        filename = (
            f"对比{index:02d}_{safe_slug(comparison.left.label)}_vs_"
            f"{safe_slug(comparison.right.label)}.csv"
        )
        write_csv(tables_dir / filename, rows, fields)

    markdown_path = output_dir / "PHKMST逐表对比分析与结论.md"
    markdown_path.write_text(
        markdown_report(
            input_root, runs, comparisons_found, report_metrics, reference, warnings),
        encoding="utf-8",
    )
    html_path = output_dir / "PHKMST逐实例消融对比表.html"
    html_path.write_text(
        html_report(
            input_root, runs, comparisons_found, report_metrics, reference, warnings),
        encoding="utf-8",
    )
    return html_path, markdown_path


def main() -> int:
    args = parse_args()
    try:
        input_root = args.input_root.resolve()
        output_dir = args.output_dir.resolve()
        discovered, warnings = load_runs(input_root)
        runs = filter_runs(discovered, args.runs)
        if not runs:
            raise ValueError("筛选后没有可汇总的运行")
        if args.list_runs:
            for run in runs:
                print(
                    f"{run.run_id}\t{run.strategy}\t{run.kind}\t"
                    f"instances={len(run.instances)}\t{run.description}")
            return 0
        reference, reference_warning = select_reference(runs, args.reference)
        if reference_warning:
            warnings.append(reference_warning)
        comparisons_found = build_comparisons(args, runs, reference)
        if not comparisons_found:
            warnings.append("没有发现可比较的配置；已生成配置和逐实例汇总")
        unknown_metrics = sorted(set(args.metrics or ()) - set(all_metrics(runs)))
        if unknown_metrics:
            warnings.append("HTML 指标不存在，已忽略：" + ", ".join(unknown_metrics))
        html_path, markdown_path = write_outputs(
            input_root,
            output_dir,
            runs,
            comparisons_found,
            reference,
            warnings,
            args.metrics,
        )
    except (OSError, ValueError, csv.Error) as error:
        print(f"汇总失败：{error}", file=sys.stderr)
        return 2

    for warning in warnings:
        print(f"Warning: {warning}", file=sys.stderr)
    print(
        f"已汇总 {len(runs)} 个运行和 {len(comparisons_found)} 组对比：\n"
        f"  HTML: {html_path}\n"
        f"  Markdown: {markdown_path}\n"
        f"  CSV: {output_dir}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
