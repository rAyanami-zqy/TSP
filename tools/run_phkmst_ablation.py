#!/usr/bin/env python3
"""Run cached TSP strategies sequentially and write one table per strategy.

The subprocess model follows ``tools/compare_strategies.py``: every instance is
passed directly to its executable, subprocess wall time is measured externally,
timeouts are isolated per instance, and instances inside one strategy may run
in parallel.  Concorde uses a fixed seed and a private temporary directory.

Scheduling is strategy-major.  A strategy finishes all instance/repeat calls
and writes its own summary before the next strategy starts.  Every completed
call is printed and atomically checkpointed to both its strategy table and the
shared cache immediately.

Only three measurements are retained per call: wall time, result cost, and
branch count (``nodes_created`` for tsp_bb, ``bbnodes`` for Concorde).  A later
program can perform cross-strategy analysis from these small tables.
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import math
import os
import re
import shlex
import shutil
import statistics
import subprocess
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable

import compare_strategies as reference


PROJECT_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_SOLVER = PROJECT_ROOT / "build" / "tsp_bb"
DEFAULT_CONCORDE = PROJECT_ROOT / "concorde" / "TSP" / "concorde"
DEFAULT_BATCH = PROJECT_ROOT / "data" / "classic" / "batch-ablation-smoke.txt"
DEFAULT_OUTPUT_ROOT = PROJECT_ROOT / "outputs" / "phkmst-ablation"
CONCORDE_WORK_ROOT = PROJECT_ROOT / "TS"
DEFAULT_TIMEOUT = 120.0
DEFAULT_WORKERS = 5
DEFAULT_DEBUG_INTERVAL = 5_000_000
DEFAULT_CONCORDE_SEED = 123
CACHE_SCHEMA = 2


@dataclass(frozen=True)
class Strategy:
    name: str
    kind: str
    category: str
    solver_args: tuple[str, ...]
    description: str


def phkmst_args(
    *,
    root_ascent: str = "polyak",
    node_ascent: str = "polyak",
    potential_update: str = "none",
    update_depth: int = 2,
    update_gap_ratio: float = 0.02,
    update_iterations: int = 16,
    update_budget: int = 5000,
    branch_edge_order: str = "weight",
) -> tuple[str, ...]:
    args = [
        "--hk-ascent", root_ascent,
        "--hk-node-ascent", node_ascent,
        "--branch-edge-order", branch_edge_order,
        "--hk-potential-update", potential_update,
    ]
    if potential_update != "none":
        args.extend([
            "--hk-update-depth", str(update_depth),
            "--hk-update-gap-ratio", str(update_gap_ratio),
            "--hk-update-iterations", str(update_iterations),
            "--hk-update-budget", str(update_budget),
        ])
    return tuple(args)


# Explicit experiment catalogue.  Order here is the default execution order.
BUILTIN_STRATEGIES: tuple[Strategy, ...] = (
    Strategy(
        "Concorde", "concorde", "reference", (),
        "Concorde exact solver with a fixed seed"),
    Strategy(
        "P0", "tsp_bb", "core", phkmst_args(),
        "root Polyak; node potential updates disabled"),
    Strategy(
        "P1", "tsp_bb", "core",
        phkmst_args(potential_update="subtree-adaptive"),
        "root Polyak; node Polyak; subtree adaptive"),
    Strategy(
        "P2", "tsp_bb", "core",
        phkmst_args(
            node_ascent="helsgaun", potential_update="subtree-adaptive"),
        "root Polyak; node Helsgaun; subtree adaptive"),
    Strategy(
        "H0", "tsp_bb", "core", phkmst_args(root_ascent="helsgaun"),
        "root Helsgaun; node potential updates disabled"),
    Strategy(
        "H1", "tsp_bb", "core",
        phkmst_args(
            root_ascent="helsgaun", potential_update="subtree-adaptive"),
        "root Helsgaun; node Polyak; subtree adaptive"),
    Strategy(
        "H2", "tsp_bb", "core",
        phkmst_args(
            root_ascent="helsgaun", node_ascent="helsgaun",
            potential_update="subtree-adaptive"),
        "root Helsgaun; node Helsgaun; subtree adaptive"),
    Strategy(
        "T1", "tsp_bb", "trigger",
        phkmst_args(potential_update="depth", update_depth=1),
        "temporary node update at every eligible depth"),
    Strategy(
        "T2", "tsp_bb", "trigger",
        phkmst_args(potential_update="depth", update_depth=2),
        "temporary node update every two depths"),
    Strategy(
        "T3", "tsp_bb", "trigger",
        phkmst_args(potential_update="adaptive", update_depth=1),
        "temporary adaptive update from depth one at a 2% gap"),
    Strategy(
        "T4", "tsp_bb", "trigger",
        phkmst_args(potential_update="subtree-depth", update_depth=1),
        "persistent subtree epoch at every eligible depth"),
    Strategy(
        "B1", "tsp_bb", "alpha",
        phkmst_args(branch_edge_order="root-alpha-asc"),
        "one maximum-degree vertex; root alpha ascending"),
    Strategy(
        "B2", "tsp_bb", "alpha",
        phkmst_args(branch_edge_order="root-alpha-desc"),
        "one maximum-degree vertex; root alpha descending"),
    Strategy(
        "B3", "tsp_bb", "alpha",
        phkmst_args(branch_edge_order="root-alpha-global-asc"),
        "all violation-touching edges; root alpha ascending"),
    Strategy(
        "B4", "tsp_bb", "alpha",
        phkmst_args(branch_edge_order="root-alpha-global-desc"),
        "all violation-touching edges; root alpha descending"),
)

BUILTIN_BY_NAME = {strategy.name: strategy for strategy in BUILTIN_STRATEGIES}

RESULT_FIELDS = (
    "run_id",
    "strategy",
    "repeat",
    "instance",
    "status",
    "wall_seconds",
    "result",
    "branches",
)

CONFIGURATION_FIELDS = (
    "run_id",
    "strategy",
    "kind",
    "category",
    "description",
    "executable",
    "executable_sha256",
    "solver_args",
    "timeout_seconds",
    "workers",
    "repeats",
    "instance_count",
    "command",
)

SUMMARY_FIELDS = (
    "run_id",
    "strategy",
    "status",
    "rows",
    "successful",
    "timeouts",
    "errors",
    "total_wall_seconds",
    "median_wall_seconds",
    "total_branches",
    "median_branches",
)

MANAGED_TSP_OPTIONS = {
    "--batch", "--exact-max-n", "--debug", "--debug-interval",
}


def parse_custom_strategy(
    spec: str, parser: argparse.ArgumentParser,
) -> Strategy:
    if "=" not in spec:
        parser.error(
            "--strategy must use NAME=ARGS, for example "
            "'custom=--hk-ascent hybrid --hk-potential-update none'")
    name, raw_args = spec.split("=", 1)
    name = name.strip()
    if not name:
        parser.error("--strategy name cannot be empty")
    try:
        solver_args = tuple(shlex.split(raw_args))
    except ValueError as error:
        parser.error(f"invalid arguments for strategy {name!r}: {error}")
    for token in solver_args:
        option = token.split("=", 1)[0]
        if option in MANAGED_TSP_OPTIONS:
            parser.error(
                f"strategy {name!r}: {option} is managed by the runner")
    return Strategy(name, "tsp_bb", "custom", solver_args, "custom strategy")


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--solver", type=Path, default=DEFAULT_SOLVER,
                        help="compiled tsp_bb executable")
    parser.add_argument("--concorde", type=Path, default=DEFAULT_CONCORDE,
                        help="Concorde executable")
    parser.add_argument(
        "--configs", "--config", nargs="+", choices=tuple(BUILTIN_BY_NAME),
        help="built-in configurations in execution order; defaults to all",
    )
    parser.add_argument(
        "--strategy", action="append", default=[], metavar="NAME=ARGS",
        help="custom tsp_bb strategy; repeat in execution order",
    )
    parser.add_argument(
        "--solver-args",
        help="single custom strategy compatibility form",
    )
    parser.add_argument(
        "--output-root", "--output-dir", dest="output_root", type=Path,
        default=DEFAULT_OUTPUT_ROOT,
        help="root for the shared cache and per-strategy directories",
    )
    parser.add_argument("--cache", type=Path,
                        help="cache path; defaults to <output-root>/cache.json")
    parser.add_argument("--batch-list", type=Path,
                        help="instance list; defaults to the smoke list")
    parser.add_argument("--instances", nargs="*", type=Path,
                        help="explicit instances instead of a batch list")
    parser.add_argument("--limit", type=int,
                        help="use only the first N selected instances")
    parser.add_argument("--timeout", type=float, default=DEFAULT_TIMEOUT,
                        help="seconds allowed per configuration/instance call")
    parser.add_argument("--workers", type=int, default=DEFAULT_WORKERS,
                        help="parallel instances within the current configuration")
    parser.add_argument("--repeats", type=int, default=1,
                        help="independent calls per configuration/instance")
    parser.add_argument("--exact-max-n", type=int, default=130,
                        help="tsp_bb exact-size guard")
    parser.add_argument("--no-debug", dest="debug", action="store_false",
                        help="do not pass tsp_bb debug progress options")
    parser.set_defaults(debug=True)
    parser.add_argument("--debug-interval", type=int,
                        default=DEFAULT_DEBUG_INTERVAL)
    parser.add_argument("--concorde-seed", type=int,
                        default=DEFAULT_CONCORDE_SEED)
    parser.add_argument(
        "--max-runs", "--max-batches", dest="max_runs", type=int,
        help="run at most N uncached calls; a partial strategy stops the pipeline",
    )
    parser.add_argument("--fresh", "--restart", dest="fresh",
                        action="store_true", help="ignore and overwrite the cache")
    parser.add_argument("--list-instances", action="store_true")
    parser.add_argument("--list-configs", action="store_true",
                        help="list every built-in configuration and exit")
    parser.add_argument(
        "--print-commands", "--print-command", dest="print_commands",
        action="store_true",
        help="print selected commands and output directories, then exit",
    )
    args = parser.parse_args()

    if args.timeout <= 0:
        parser.error("--timeout must be positive")
    if args.workers <= 0:
        parser.error("--workers must be positive")
    if args.repeats <= 0:
        parser.error("--repeats must be positive")
    if args.exact_max_n < 3:
        parser.error("--exact-max-n must be at least 3")
    if args.debug_interval <= 0:
        parser.error("--debug-interval must be positive")
    if args.limit is not None and args.limit <= 0:
        parser.error("--limit must be positive")
    if args.max_runs is not None and args.max_runs <= 0:
        parser.error("--max-runs must be positive")
    selection_forms = sum(bool(value) for value in (
        args.configs, args.strategy, args.solver_args is not None))
    if selection_forms > 1:
        parser.error("use only one of --configs, --strategy, or --solver-args")

    if args.configs:
        args.strategies = [BUILTIN_BY_NAME[name] for name in args.configs]
    elif args.strategy:
        args.strategies = [
            parse_custom_strategy(spec, parser) for spec in args.strategy]
    elif args.solver_args is not None:
        args.strategies = [
            parse_custom_strategy(f"custom={args.solver_args}", parser)]
    else:
        args.strategies = list(BUILTIN_STRATEGIES)
    names = [strategy.name for strategy in args.strategies]
    if len(names) != len(set(names)):
        parser.error("configuration names must be unique")
    return args


def list_builtin_configs() -> None:
    for strategy in BUILTIN_STRATEGIES:
        args = shlex.join(strategy.solver_args) or "(managed Concorde arguments)"
        print(
            f"{strategy.name:<9} [{strategy.kind}/{strategy.category}] "
            f"{strategy.description}\n"
            f"          {args}")


def resolve_instance(path: Path) -> Path:
    candidate = path if path.is_absolute() else PROJECT_ROOT / path
    candidate = candidate.resolve()
    if not candidate.is_file():
        raise FileNotFoundError(f"instance not found: {candidate}")
    return candidate


def read_batch_list(path: Path) -> list[Path]:
    list_path = path if path.is_absolute() else PROJECT_ROOT / path
    list_path = list_path.resolve()
    if not list_path.is_file():
        raise FileNotFoundError(f"batch list not found: {list_path}")
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
    if len({path.resolve() for path in instances}) != len(instances):
        raise ValueError("the experiment contains duplicate instances")
    if args.limit is not None:
        instances = instances[:args.limit]
    if not instances:
        raise ValueError("the experiment contains no instances")
    return instances


def binary_digest(path: Path) -> str:
    digest = hashlib.sha256()
    with path.open("rb") as source:
        for block in iter(lambda: source.read(1024 * 1024), b""):
            digest.update(block)
    return digest.hexdigest()


def effective_args(args: argparse.Namespace, strategy: Strategy) -> list[str]:
    if strategy.kind == "concorde":
        return [*strategy.solver_args, "-s", str(args.concorde_seed)]
    result = [
        *strategy.solver_args, "--exact-max-n", str(args.exact_max_n)]
    if args.debug:
        result.extend(["--debug", "--debug-interval", str(args.debug_interval)])
    return result


def strategy_identity(
    args: argparse.Namespace,
    strategy: Strategy,
    executable: Path,
    executable_sha256: str,
    invocation_args: list[str],
    instances: list[Path],
) -> dict[str, Any]:
    return {
        "format_version": 2,
        "strategy": strategy.name,
        "kind": strategy.kind,
        "executable": str(executable),
        "executable_sha256": executable_sha256,
        "solver_args": invocation_args,
        "timeout_seconds": args.timeout,
        "workers": args.workers,
        "repeats": args.repeats,
        "instances": [str(instance) for instance in instances],
    }


def identity_fingerprint(identity: dict[str, Any]) -> str:
    encoded = json.dumps(
        identity, ensure_ascii=True, sort_keys=True, separators=(",", ":"),
    ).encode("utf-8")
    return hashlib.sha256(encoded).hexdigest()[:12]


def slugify(value: str) -> str:
    slug = re.sub(r"[^A-Za-z0-9._-]+", "-", value.strip()).strip("-._")
    return slug[:96] or "strategy"


def cache_key(repeat: int, instance: Path | str) -> str:
    return f"{repeat}:{instance}"


def empty_result(
    run_id: str, strategy: Strategy, repeat: int, instance: Path,
) -> dict[str, Any]:
    return {
        "run_id": run_id,
        "strategy": strategy.name,
        "repeat": repeat,
        "instance": str(instance),
        "status": "error",
        "wall_seconds": None,
        "result": None,
        "branches": None,
    }


def run_tspbb_once(
    run_id: str,
    strategy: Strategy,
    repeat: int,
    instance: Path,
    executable: Path,
    invocation_args: list[str],
    timeout: float,
) -> tuple[dict[str, Any], str | None]:
    command = [str(executable), *invocation_args, str(instance)]
    started = time.perf_counter()
    try:
        completed = subprocess.run(
            command, cwd=PROJECT_ROOT, capture_output=True, text=True,
            timeout=timeout)
        row = empty_result(run_id, strategy, repeat, instance)
        row["wall_seconds"] = time.perf_counter() - started
        stats = reference.parse_tspbb_stats(completed.stdout)
        row["result"] = stats.get("cost")
        row["branches"] = stats.get("nodes_created")
        if completed.returncode != 0:
            details = completed.stderr.strip() or completed.stdout.strip()
            return row, f"exit {completed.returncode}: {details[-500:]}"
        if not stats.get("feasible") or row["result"] is None:
            return row, "tsp_bb produced no parseable optimal result"
        row["status"] = "ok"
        return row, None
    except subprocess.TimeoutExpired:
        row = empty_result(run_id, strategy, repeat, instance)
        row["status"] = "timeout"
        row["wall_seconds"] = timeout
        return row, f"timeout after {timeout:g}s"
    except Exception as error:  # noqa: BLE001 - stored as an error result
        row = empty_result(run_id, strategy, repeat, instance)
        row["wall_seconds"] = time.perf_counter() - started
        return row, str(error)


def run_concorde_once(
    run_id: str,
    strategy: Strategy,
    repeat: int,
    instance: Path,
    executable: Path,
    invocation_args: list[str],
    timeout: float,
) -> tuple[dict[str, Any], str | None]:
    started = time.perf_counter()
    try:
        CONCORDE_WORK_ROOT.mkdir(parents=True, exist_ok=True)
        with tempfile.TemporaryDirectory(
            prefix="phkmst-concorde-", dir=CONCORDE_WORK_ROOT,
        ) as temporary_directory:
            temporary_root = Path(temporary_directory)
            prepared = Path(reference.ensure_tsplib_for_concorde(
                str(instance), temporary_directory))
            local_instance = temporary_root / prepared.name
            if prepared.resolve() != local_instance.resolve():
                shutil.copy2(prepared, local_instance)
            solution = temporary_root / f"{local_instance.stem}.sol"
            command = [
                str(executable), *invocation_args,
                "-o", str(solution), str(local_instance),
            ]
            completed = subprocess.run(
                command, cwd=temporary_root, capture_output=True, text=True,
                timeout=timeout)

        row = empty_result(run_id, strategy, repeat, instance)
        row["wall_seconds"] = time.perf_counter() - started
        stats = reference.parse_concorde_output(completed.stdout)
        row["result"] = stats.get("cost")
        row["branches"] = stats.get("bbnodes")
        if completed.returncode != 0:
            details = completed.stderr.strip() or completed.stdout.strip()
            return row, f"exit {completed.returncode}: {details[-500:]}"
        if not stats.get("feasible") or row["result"] is None:
            return row, "Concorde produced no parseable optimal result"
        row["status"] = "ok"
        return row, None
    except subprocess.TimeoutExpired:
        row = empty_result(run_id, strategy, repeat, instance)
        row["status"] = "timeout"
        row["wall_seconds"] = timeout
        return row, f"timeout after {timeout:g}s"
    except Exception as error:  # noqa: BLE001 - stored as an error result
        row = empty_result(run_id, strategy, repeat, instance)
        row["wall_seconds"] = time.perf_counter() - started
        return row, str(error)


def run_strategy_once(
    run_id: str,
    strategy: Strategy,
    repeat: int,
    instance: Path,
    executable: Path,
    invocation_args: list[str],
    timeout: float,
) -> tuple[dict[str, Any], str | None]:
    runner = run_concorde_once if strategy.kind == "concorde" else run_tspbb_once
    return runner(
        run_id, strategy, repeat, instance, executable, invocation_args,
        timeout)


def write_csv(path: Path, fields: Iterable[str], rows: Iterable[dict[str, Any]]) -> None:
    temporary_path: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="w", encoding="utf-8", newline="", dir=path.parent,
            prefix=f".{path.name}.", suffix=".tmp", delete=False,
        ) as target:
            temporary_path = Path(target.name)
            writer = csv.DictWriter(
                target, fieldnames=list(fields), extrasaction="ignore",
                lineterminator="\n")
            writer.writeheader()
            writer.writerows(rows)
        temporary_path.replace(path)
    finally:
        if temporary_path is not None:
            temporary_path.unlink(missing_ok=True)


def write_text(path: Path, content: str) -> None:
    temporary_path: Path | None = None
    try:
        with tempfile.NamedTemporaryFile(
            mode="w", encoding="utf-8", dir=path.parent,
            prefix=f".{path.name}.", suffix=".tmp", delete=False,
        ) as target:
            temporary_path = Path(target.name)
            target.write(content)
        temporary_path.replace(path)
    finally:
        if temporary_path is not None:
            temporary_path.unlink(missing_ok=True)


def load_cache(path: Path, fresh: bool) -> dict[str, Any]:
    empty: dict[str, Any] = {"schema": CACHE_SCHEMA, "runs": {}}
    if fresh or not path.is_file():
        return empty
    try:
        cache = json.loads(path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError) as error:
        print(f"Warning: ignoring unreadable cache: {error}")
        return empty
    if cache.get("schema") != CACHE_SCHEMA or not isinstance(cache.get("runs"), dict):
        print("Cache schema changed; starting with an empty cache.")
        return empty
    return cache


def save_cache(path: Path, cache: dict[str, Any]) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    write_text(
        path, json.dumps(cache, ensure_ascii=False, indent=2, sort_keys=True) + "\n")


def numeric_values(
    rows: Iterable[dict[str, Any]], field: str, integer: bool = False,
) -> list[int | float]:
    values: list[int | float] = []
    for row in rows:
        value = row.get(field)
        if value in (None, ""):
            continue
        try:
            parsed = int(value) if integer else float(value)
        except (TypeError, ValueError):
            continue
        if integer or math.isfinite(parsed):
            values.append(parsed)
    return values


def summarize_strategy(
    run_id: str, strategy: Strategy, rows: list[dict[str, Any]], status: str,
) -> dict[str, Any]:
    successful = [row for row in rows if row["status"] == "ok"]
    walls = numeric_values(successful, "wall_seconds")
    branches = numeric_values(successful, "branches", integer=True)
    return {
        "run_id": run_id,
        "strategy": strategy.name,
        "status": status,
        "rows": len(rows),
        "successful": len(successful),
        "timeouts": sum(row["status"] == "timeout" for row in rows),
        "errors": sum(row["status"] == "error" for row in rows),
        "total_wall_seconds": sum(walls) if walls else 0,
        "median_wall_seconds": statistics.median(walls) if walls else "",
        "total_branches": sum(branches) if branches else 0,
        "median_branches": statistics.median(branches) if branches else "",
    }


def configuration_row(
    args: argparse.Namespace,
    run_id: str,
    strategy: Strategy,
    executable: Path,
    executable_sha256: str,
    invocation_args: list[str],
    instance_count: int,
) -> dict[str, Any]:
    if strategy.kind == "concorde":
        command = [
            str(executable), *invocation_args,
            "-o", "<solution-file>", "<instance>",
        ]
    else:
        command = [str(executable), *invocation_args, "<instance>"]
    return {
        "run_id": run_id,
        "strategy": strategy.name,
        "kind": strategy.kind,
        "category": strategy.category,
        "description": strategy.description,
        "executable": str(executable),
        "executable_sha256": executable_sha256,
        "solver_args": shlex.join(invocation_args),
        "timeout_seconds": args.timeout,
        "workers": args.workers,
        "repeats": args.repeats,
        "instance_count": instance_count,
        "command": shlex.join(command),
    }


def persist_strategy(
    output_dir: Path,
    configuration: dict[str, Any],
    identity: dict[str, Any],
    run_id: str,
    strategy: Strategy,
    rows_by_key: dict[str, dict[str, Any]],
    instance_order: dict[str, int],
    total_rows: int,
    status: str,
) -> dict[str, Any]:
    rows = sorted(rows_by_key.values(), key=lambda row: (
        row["repeat"], instance_order[row["instance"]]))
    summary = summarize_strategy(run_id, strategy, rows, status)
    write_csv(
        output_dir / "configuration.csv", CONFIGURATION_FIELDS, [configuration])
    write_csv(output_dir / "results.csv", RESULT_FIELDS, rows)
    write_csv(output_dir / "summary.csv", SUMMARY_FIELDS, [summary])
    write_text(
        output_dir / "progress.json",
        json.dumps({
            "run_id": run_id,
            "run": identity,
            "status": status,
            "completed_rows": len(rows),
            "total_rows": total_rows,
        }, ensure_ascii=False, indent=2) + "\n")
    return summary


def print_strategy_summary(summary: dict[str, Any], output_dir: Path) -> None:
    print(f"Strategy table: {output_dir}")
    print(
        f"  rows={summary['rows']} ok={summary['successful']} "
        f"timeout={summary['timeouts']} error={summary['errors']}")
    if summary["successful"]:
        branches = summary["median_branches"]
        branch_text = f"{float(branches):.0f}" if branches != "" else "-"
        print(
            f"  median wall={float(summary['median_wall_seconds']):.6f}s "
            f"branches={branch_text}")


def validate_executable(path: Path, label: str) -> Path:
    resolved = path.resolve()
    if not resolved.is_file() or not os.access(resolved, os.X_OK):
        raise FileNotFoundError(f"executable {label} not found: {resolved}")
    return resolved


def main() -> int:
    args = parse_args()
    if args.list_configs:
        list_builtin_configs()
        return 0

    instances = select_instances(args)
    if args.list_instances:
        for instance in instances:
            print(instance)
        return 0

    needs_tsp = any(strategy.kind == "tsp_bb" for strategy in args.strategies)
    needs_concorde = any(
        strategy.kind == "concorde" for strategy in args.strategies)
    tsp_executable = (
        validate_executable(args.solver, "tsp_bb") if needs_tsp else None)
    concorde_executable = (
        validate_executable(args.concorde, "Concorde")
        if needs_concorde else None)
    executable_digests: dict[Path, str] = {}

    output_root = args.output_root.resolve()
    output_root.mkdir(parents=True, exist_ok=True)
    cache_path = args.cache.resolve() if args.cache else output_root / "cache.json"

    phases: list[dict[str, Any]] = []
    for strategy in args.strategies:
        executable = (
            concorde_executable if strategy.kind == "concorde"
            else tsp_executable)
        assert executable is not None
        if executable not in executable_digests:
            executable_digests[executable] = binary_digest(executable)
        executable_sha256 = executable_digests[executable]
        invocation_args = effective_args(args, strategy)
        identity = strategy_identity(
            args, strategy, executable, executable_sha256, invocation_args,
            instances)
        fingerprint = identity_fingerprint(identity)
        run_id = f"{slugify(strategy.name)}-{fingerprint}"
        output_dir = output_root / run_id
        phases.append({
            "strategy": strategy,
            "executable": executable,
            "invocation_args": invocation_args,
            "identity": identity,
            "fingerprint": fingerprint,
            "run_id": run_id,
            "output_dir": output_dir,
            "configuration": configuration_row(
                args, run_id, strategy, executable, executable_sha256,
                invocation_args, len(instances)),
        })

    if args.print_commands:
        for index, phase in enumerate(phases, 1):
            print(f"[{index}] configuration: {phase['strategy'].name}")
            print(f"    run_id: {phase['run_id']}")
            print(f"    output_dir: {phase['output_dir']}")
            print(f"    command: {phase['configuration']['command']}")
        return 0

    cache = load_cache(cache_path, args.fresh)
    if args.fresh:
        save_cache(cache_path, cache)
    instance_order = {
        str(instance): index for index, instance in enumerate(instances)}
    total_rows = args.repeats * len(instances)
    remaining_new_runs = args.max_runs
    any_failures = False

    for phase_index, phase in enumerate(phases, 1):
        strategy: Strategy = phase["strategy"]
        fingerprint: str = phase["fingerprint"]
        run_id: str = phase["run_id"]
        identity: dict[str, Any] = phase["identity"]
        output_dir: Path = phase["output_dir"]
        output_dir.mkdir(parents=True, exist_ok=True)

        print(
            f"\n=== Configuration {phase_index}/{len(phases)}: "
            f"{strategy.name} ===",
            flush=True)
        cache_entry = cache["runs"].get(fingerprint)
        if cache_entry is None:
            cache_entry = {"identity": identity, "results": {}}
            cache["runs"][fingerprint] = cache_entry
        elif cache_entry.get("identity") != identity:
            raise RuntimeError(f"cache fingerprint collision for {strategy.name}")
        cached_results = cache_entry.setdefault("results", {})

        # Successful and timeout records are reusable. Error rows stay in the
        # cache for inspection but remain pending and are overwritten on retry.
        rows_by_key: dict[str, dict[str, Any]] = {
            key: row for key, row in cached_results.items()
            if row.get("status") in {"ok", "timeout"}
        }
        all_tasks = [
            (repeat, instance)
            for repeat in range(1, args.repeats + 1)
            for instance in instances
            if cache_key(repeat, instance) not in rows_by_key
        ]
        scheduled_tasks = all_tasks
        if remaining_new_runs is not None:
            scheduled_tasks = all_tasks[:remaining_new_runs]

        persist_strategy(
            output_dir, phase["configuration"], identity, run_id, strategy,
            rows_by_key, instance_order, total_rows,
            "complete" if not all_tasks else "running")
        print(
            f"Cached {len(rows_by_key)}/{total_rows}; "
            f"running {len(scheduled_tasks)} call(s) with workers={args.workers}",
            flush=True)

        with ThreadPoolExecutor(max_workers=args.workers) as executor:
            futures = {
                executor.submit(
                    run_strategy_once, run_id, strategy, repeat, instance,
                    phase["executable"], phase["invocation_args"], args.timeout,
                ): (repeat, instance)
                for repeat, instance in scheduled_tasks
            }
            for future in as_completed(futures):
                repeat, instance = futures[future]
                row, error = future.result()
                key = cache_key(repeat, instance)
                rows_by_key[key] = row

                # Finest checkpoint unit: one configuration/repeat/instance.
                cached_results[key] = row
                save_cache(cache_path, cache)
                detail = f": {error}" if error else ""
                print(
                    f"[{len(rows_by_key)}/{total_rows}] repeat {repeat} "
                    f"{instance.name}: {row['status']} "
                    f"time={float(row['wall_seconds']):.6f}s "
                    f"result={row['result']} branches={row['branches']}"
                    f"{detail}",
                    flush=True)
                persist_strategy(
                    output_dir, phase["configuration"], identity, run_id,
                    strategy, rows_by_key, instance_order, total_rows, "running")

        if remaining_new_runs is not None:
            remaining_new_runs -= len(scheduled_tasks)
        phase_complete = len(rows_by_key) == total_rows
        summary = persist_strategy(
            output_dir, phase["configuration"], identity, run_id, strategy,
            rows_by_key, instance_order, total_rows,
            "complete" if phase_complete else "partial")
        print_strategy_summary(summary, output_dir)
        any_failures = any_failures or bool(
            summary["timeouts"] or summary["errors"])

        if not phase_complete:
            print(
                f"Configuration {strategy.name} is partial; later "
                "configurations were not started. Rerun to continue.",
                flush=True)
            break
        if remaining_new_runs == 0 and phase_index < len(phases):
            print(
                "The --max-runs budget is exhausted; later configurations "
                "were not started. Rerun to continue.",
                flush=True)
            break

    return 1 if any_failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
