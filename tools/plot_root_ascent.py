#!/usr/bin/env python3
"""Plot root Held--Karp lower-bound trajectories for three ascent strategies.

Each selected instance is handled independently.  Concorde supplies the exact
reference cost, while ``tsp_bb`` is run three times in ``--root-bound-only``
mode for Polyak, Helsgaun, and Hybrid.  The solver's own ascent loops emit the
per-evaluation data, so this script contains no duplicate optimization logic.

The configured iteration count is a per-phase cap, matching the solver's
existing semantics: Hybrid first runs a capped Polyak phase and then a capped
Helsgaun phase.  Existing convergence tests may stop either phase early.

Examples:

  python3 tools/plot_root_ascent.py data/classic/tsplib/eil51.tsp
  python3 tools/plot_root_ascent.py --batch-list my-instances.txt
  python3 tools/plot_root_ascent.py --iterations 4000 a.tsp b.tsp
  python3 tools/plot_root_ascent.py --chart-width 1800 a.tsp
  python3 tools/plot_root_ascent.py --iterations-per-width 400 a.tsp
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import html
import json
import math
import re
import shutil
import subprocess
import sys
import tempfile
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from pathlib import Path
from typing import Iterable

import compare_strategies as concorde_support


PROJECT_ROOT = Path(__file__).resolve().parent.parent
DEFAULT_SOLVER = PROJECT_ROOT / "build" / "tsp_bb"
DEFAULT_CONCORDE = PROJECT_ROOT / "concorde" / "TSP" / "concorde"
DEFAULT_OUTPUT_ROOT = PROJECT_ROOT / "outputs" / "root-ascent-trends"
DEFAULT_ITERATIONS = 2000
DEFAULT_CHART_WIDTH = 1600
DEFAULT_ITERATIONS_PER_WIDTH = 400
DEFAULT_TIMEOUT = 1800.0
DEFAULT_CONCORDE_SEED = 123
STRATEGIES = ("polyak", "helsgaun", "hybrid")
STRATEGY_LABELS = {
    "polyak": "Polyak",
    "helsgaun": "Helsgaun",
    "hybrid": "Hybrid",
}
STRATEGY_COLORS = {
    "polyak": "#2563eb",
    "helsgaun": "#dc2626",
    "hybrid": "#059669",
}


@dataclass(frozen=True)
class TracePoint:
    strategy: str
    iteration: int
    phase: str
    phase_iteration: int
    lower_bound: float
    best_lower_bound: float


@dataclass(frozen=True)
class CachedConcordeResults:
    source: Path
    instances: tuple[Path, ...]
    optimum_by_instance: dict[Path, float]


def parse_args(argv: list[str] | None = None) -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description=(
            "Generate one root-ascent trend chart per selected TSP instance."
        )
    )
    parser.add_argument(
        "instances", nargs="*", type=Path,
        help="instance paths; may be combined with --batch-list")
    parser.add_argument(
        "--batch-list", action="append", default=[], type=Path,
        help="text file containing one instance path per line (repeatable)")
    parser.add_argument(
        "--iterations", type=int, default=DEFAULT_ITERATIONS,
        help=(
            "maximum evaluations per ascent phase "
            f"(default: {DEFAULT_ITERATIONS})"
        ))
    parser.add_argument(
        "--chart-width", type=int, default=DEFAULT_CHART_WIDTH,
        help=(
            "SVG width allocated to each iteration span "
            f"(default: {DEFAULT_CHART_WIDTH}; minimum: 800)"
        ))
    parser.add_argument(
        "--iterations-per-width", type=int,
        default=DEFAULT_ITERATIONS_PER_WIDTH,
        help=(
            "iterations expanded to one chart-width segment "
            f"(default: {DEFAULT_ITERATIONS_PER_WIDTH})"
        ))
    parser.add_argument(
        "--solver", type=Path, default=DEFAULT_SOLVER,
        help=f"tsp_bb executable (default: {DEFAULT_SOLVER})")
    parser.add_argument(
        "--concorde", type=Path, default=DEFAULT_CONCORDE,
        help=f"Concorde executable (default: {DEFAULT_CONCORDE})")
    parser.add_argument(
        "--concorde-results", type=Path,
        help=(
            "existing Concorde results.csv or its containing directory; "
            "when no instances are listed, plot every successful row"
        ))
    parser.add_argument(
        "--output-root", type=Path, default=DEFAULT_OUTPUT_ROOT,
        help=f"root directory for per-instance outputs (default: {DEFAULT_OUTPUT_ROOT})")
    parser.add_argument(
        "--exact-max-n", type=int, default=10000,
        help="maximum instance dimension accepted by tsp_bb (default: 10000)")
    parser.add_argument(
        "--timeout", type=float, default=DEFAULT_TIMEOUT,
        help=f"timeout in seconds for each process (default: {DEFAULT_TIMEOUT:g})")
    parser.add_argument(
        "--concorde-seed", type=int, default=DEFAULT_CONCORDE_SEED,
        help=f"fixed Concorde seed (default: {DEFAULT_CONCORDE_SEED})")
    parser.add_argument(
        "--workers", type=int, default=1,
        help="number of instances processed concurrently (default: 1)")
    args = parser.parse_args(argv)

    if args.iterations <= 0:
        parser.error("--iterations must be greater than zero")
    if args.chart_width < 800:
        parser.error("--chart-width must be at least 800 pixels")
    if args.iterations_per_width <= 0:
        parser.error("--iterations-per-width must be greater than zero")
    if args.exact_max_n <= 0:
        parser.error("--exact-max-n must be greater than zero")
    if not math.isfinite(args.timeout) or args.timeout <= 0:
        parser.error("--timeout must be finite and greater than zero")
    if args.workers <= 0:
        parser.error("--workers must be greater than zero")
    if not args.instances and not args.batch_list and args.concorde_results is None:
        parser.error(
            "select instances, provide --batch-list, or provide --concorde-results")
    return args


def resolve_existing_path(path: Path, extra_base: Path | None = None) -> Path:
    """Resolve a configured path using cwd, project root, then list location."""
    candidates = [path] if path.is_absolute() else [Path.cwd() / path, PROJECT_ROOT / path]
    if extra_base is not None and not path.is_absolute():
        candidates.append(extra_base / path)
    for candidate in candidates:
        if candidate.is_file():
            return candidate.resolve()
    bases = ", ".join(str(candidate) for candidate in candidates)
    raise FileNotFoundError(f"instance not found; tried: {bases}")


def configured_instances(
    direct_instances: Iterable[Path], batch_lists: Iterable[Path]
) -> list[Path]:
    configured: list[tuple[Path, Path | None]] = [
        (path, None) for path in direct_instances
    ]
    for configured_list in batch_lists:
        list_path = resolve_existing_path(configured_list)
        for line_number, raw_line in enumerate(
            list_path.read_text(encoding="utf-8").splitlines(), start=1
        ):
            line = raw_line.strip()
            if not line or line.startswith("#"):
                continue
            try:
                configured.append((Path(line), list_path.parent))
            except ValueError as error:
                raise ValueError(
                    f"invalid path at {list_path}:{line_number}: {line}"
                ) from error

    result: list[Path] = []
    seen: set[Path] = set()
    for path, base in configured:
        resolved = resolve_existing_path(path, base)
        if resolved not in seen:
            seen.add(resolved)
            result.append(resolved)
    return result


def localize_cached_instance(configured_path: str, source: Path) -> Path:
    """Map a result path from another checkout onto this repository's data tree."""
    path = Path(configured_path)
    candidates = [path] if path.is_absolute() else [source.parent / path, PROJECT_ROOT / path]
    parts = path.parts
    for index, part in enumerate(parts):
        if part == "data":
            candidates.append(PROJECT_ROOT.joinpath(*parts[index:]))
    for candidate in candidates:
        if candidate.is_file():
            return candidate.resolve()
    tried = ", ".join(str(candidate) for candidate in candidates)
    raise FileNotFoundError(
        f"cached Concorde instance {configured_path!r} is unavailable; tried: {tried}")


def load_cached_concorde_results(configured_path: Path) -> CachedConcordeResults:
    resolved = configured_path.expanduser().resolve()
    source = resolved / "results.csv" if resolved.is_dir() else resolved
    if not source.is_file():
        raise FileNotFoundError(f"Concorde results CSV not found: {source}")

    instances: list[Path] = []
    optimum_by_instance: dict[Path, float] = {}
    with source.open(newline="", encoding="utf-8") as input_file:
        reader = csv.DictReader(input_file)
        required = {"instance", "status", "result"}
        missing = required.difference(reader.fieldnames or ())
        if missing:
            raise ValueError(
                f"{source} is missing columns: {', '.join(sorted(missing))}")
        for line_number, row in enumerate(reader, start=2):
            if row["status"] != "ok":
                continue
            instance = localize_cached_instance(row["instance"], source)
            try:
                optimum = float(row["result"])
            except ValueError as error:
                raise ValueError(
                    f"invalid Concorde result at {source}:{line_number}: "
                    f"{row['result']!r}") from error
            if not math.isfinite(optimum):
                raise ValueError(
                    f"non-finite Concorde result at {source}:{line_number}")
            previous = optimum_by_instance.get(instance)
            if previous is not None and previous != optimum:
                raise ValueError(
                    f"conflicting Concorde results for {instance}: "
                    f"{previous} and {optimum}")
            if previous is None:
                instances.append(instance)
                optimum_by_instance[instance] = optimum

    if not instances:
        raise ValueError(f"{source} contains no successful Concorde results")
    return CachedConcordeResults(
        source=source, instances=tuple(instances),
        optimum_by_instance=optimum_by_instance)


def validate_executable(path: Path, label: str) -> Path:
    resolved = path.expanduser().resolve()
    if not resolved.is_file():
        raise FileNotFoundError(f"{label} executable not found: {resolved}")
    if not resolved.stat().st_mode & 0o111:
        raise PermissionError(f"{label} is not executable: {resolved}")
    return resolved


def process_failure(command: list[str], completed: subprocess.CompletedProcess[str]) -> str:
    details = completed.stderr.strip() or completed.stdout.strip()
    if len(details) > 1200:
        details = details[-1200:]
    return (
        f"command exited with {completed.returncode}: "
        f"{' '.join(command)}\n{details}"
    )


def run_concorde(
    instance: Path, executable: Path, seed: int, timeout: float, work_root: Path
) -> float:
    """Run Concorde in a disposable private directory and return its optimum."""
    with tempfile.TemporaryDirectory(prefix="concorde-", dir=work_root) as temporary:
        temporary_root = Path(temporary)
        prepared = Path(concorde_support.ensure_tsplib_for_concorde(
            str(instance), temporary))
        local_instance = temporary_root / prepared.name
        if prepared.resolve() != local_instance.resolve():
            shutil.copy2(prepared, local_instance)
        solution = temporary_root / f"{local_instance.stem}.sol"
        command = [
            str(executable), "-s", str(seed), "-o", str(solution),
            str(local_instance),
        ]
        completed = subprocess.run(
            command, cwd=temporary_root, capture_output=True, text=True,
            timeout=timeout, check=False)
        if completed.returncode != 0:
            raise RuntimeError(process_failure(command, completed))
        statistics = concorde_support.parse_concorde_output(completed.stdout)
        optimum = statistics.get("cost")
        if not statistics.get("feasible") or optimum is None:
            raise RuntimeError(
                "Concorde completed without a parseable Optimal Solution line")
        optimum = float(optimum)
        if not math.isfinite(optimum):
            raise RuntimeError(f"Concorde returned a non-finite optimum: {optimum}")
        return optimum


def read_trace(path: Path, expected_strategy: str) -> list[TracePoint]:
    required = {
        "strategy", "iteration", "phase", "phase_iteration",
        "lower_bound", "best_lower_bound",
    }
    points: list[TracePoint] = []
    with path.open(newline="", encoding="utf-8") as source:
        reader = csv.DictReader(source)
        missing = required.difference(reader.fieldnames or ())
        if missing:
            raise RuntimeError(
                f"trace {path} is missing columns: {', '.join(sorted(missing))}")
        for row in reader:
            point = TracePoint(
                strategy=row["strategy"],
                iteration=int(row["iteration"]),
                phase=row["phase"],
                phase_iteration=int(row["phase_iteration"]),
                lower_bound=float(row["lower_bound"]),
                best_lower_bound=float(row["best_lower_bound"]),
            )
            if point.strategy != expected_strategy:
                raise RuntimeError(
                    f"trace strategy {point.strategy!r} does not match "
                    f"{expected_strategy!r}")
            if point.iteration != len(points) + 1:
                raise RuntimeError(
                    f"trace iterations are not contiguous in {path}")
            if not math.isfinite(point.lower_bound) or not math.isfinite(
                point.best_lower_bound
            ):
                raise RuntimeError(f"trace contains a non-finite bound in {path}")
            if points and point.best_lower_bound + 1e-9 < points[-1].best_lower_bound:
                raise RuntimeError(f"best lower bound decreased in {path}")
            points.append(point)
    if not points:
        raise RuntimeError(f"solver produced an empty root-ascent trace: {path}")
    return points


def parse_solver_statistic(stdout: str, label: str) -> float | None:
    match = re.search(
        rf"^{re.escape(label)}:\s*([-+0-9.eE]+)\s*$", stdout,
        flags=re.MULTILINE)
    return float(match.group(1)) if match else None


def run_strategy(
    instance: Path,
    strategy: str,
    executable: Path,
    iterations: int,
    exact_max_n: int,
    timeout: float,
    trace_path: Path,
) -> tuple[list[TracePoint], dict[str, float | int | str]]:
    command = [
        str(executable),
        "--root-bound-only",
        "--hk-ascent", strategy,
        "--root-ascent-iterations", str(iterations),
        "--root-ascent-trace", str(trace_path),
        "--exact-max-n", str(exact_max_n),
        str(instance),
    ]
    completed = subprocess.run(
        command, cwd=PROJECT_ROOT, capture_output=True, text=True,
        timeout=timeout, check=False)
    if completed.returncode != 0:
        raise RuntimeError(process_failure(command, completed))
    if "Method: root-bound" not in completed.stdout:
        raise RuntimeError(f"{strategy} run did not report root-bound mode")
    nodes_expanded = parse_solver_statistic(completed.stdout, "Nodes expanded")
    if nodes_expanded != 0:
        raise RuntimeError(
            f"{strategy} unexpectedly expanded {nodes_expanded} search nodes")
    points = read_trace(trace_path, strategy)
    root_bound = parse_solver_statistic(completed.stdout, "Root lower bound")
    dimension = parse_solver_statistic(completed.stdout, "Dimension")
    reported_iterations = parse_solver_statistic(
        completed.stdout, "Root potential iterations")
    if reported_iterations is None or int(reported_iterations) != len(points):
        raise RuntimeError(
            f"{strategy} reported {reported_iterations} iterations but wrote "
            f"{len(points)} trace rows")
    return points, {
        "command": " ".join(command),
        "dimension": int(dimension) if dimension is not None else -1,
        "nodes_expanded": int(nodes_expanded),
        "evaluations": len(points),
        "root_lower_bound": root_bound if root_bound is not None else math.nan,
        "final_best_lower_bound": points[-1].best_lower_bound,
    }


def write_combined_csv(
    path: Path,
    instance: Path,
    optimum: float,
    traces: dict[str, list[TracePoint]],
) -> None:
    with path.open("w", newline="", encoding="utf-8") as output:
        fieldnames = [
            "instance", "strategy", "iteration", "phase", "phase_iteration",
            "lower_bound", "best_lower_bound", "concorde_optimum",
        ]
        writer = csv.DictWriter(output, fieldnames=fieldnames)
        writer.writeheader()
        for strategy in STRATEGIES:
            for point in traces[strategy]:
                writer.writerow({
                    "instance": str(instance),
                    "strategy": point.strategy,
                    "iteration": point.iteration,
                    "phase": point.phase,
                    "phase_iteration": point.phase_iteration,
                    "lower_bound": format(point.lower_bound, ".17g"),
                    "best_lower_bound": format(point.best_lower_bound, ".17g"),
                    "concorde_optimum": format(optimum, ".17g"),
                })


def format_number(value: float) -> str:
    magnitude = abs(value)
    if magnitude != 0 and (magnitude >= 1e6 or magnitude < 1e-3):
        return f"{value:.4g}"
    return f"{value:.6g}"


def svg_polyline(points: list[tuple[float, float]]) -> str:
    return " ".join(f"{x:.2f},{y:.2f}" for x, y in points)


def write_svg_chart(
    path: Path,
    instance_name: str,
    optimum: float,
    configured_iterations: int,
    traces: dict[str, list[TracePoint]],
    chart_width: int = DEFAULT_CHART_WIDTH,
    iterations_per_width: int = DEFAULT_ITERATIONS_PER_WIDTH,
) -> None:
    """Write a dependency-free SVG chart of raw and best bounds by iteration."""
    height = 720
    left, right, top, bottom = 110.0, 45.0, 75.0, 105.0
    requested_x_max = max(
        configured_iterations,
        max(point.iteration for points in traces.values() for point in points),
    )
    segment_count = max(
        1, math.ceil(requested_x_max / iterations_per_width))
    x_max = segment_count * iterations_per_width
    segment_plot_width = chart_width - left - right
    plot_width = segment_plot_width * segment_count
    width = int(round(left + plot_width + right))
    plot_height = height - top - bottom
    all_bounds = [optimum]
    all_bounds.extend(
        bound
        for points in traces.values()
        for point in points
        for bound in (point.lower_bound, point.best_lower_bound)
    )
    y_min = min(all_bounds)
    y_max = max(all_bounds)
    span = y_max - y_min
    padding = max(span * 0.06, max(1.0, abs(y_max)) * 0.005)
    y_min -= padding
    y_max += padding
    def map_x(iteration: float) -> float:
        return left + plot_width * iteration / x_max

    def map_y(bound: float) -> float:
        return top + plot_height * (y_max - bound) / (y_max - y_min)

    x_ticks = list(range(0, x_max + 1, iterations_per_width))
    y_ticks = [y_min + index * (y_max - y_min) / 5 for index in range(6)]
    elements: list[str] = [
        '<?xml version="1.0" encoding="UTF-8"?>',
        (f'<svg xmlns="http://www.w3.org/2000/svg" width="{width}" '
         f'height="{height}" viewBox="0 0 {width} {height}">'),
        f'<title>{html.escape(instance_name)} root potential ascent</title>',
        ('<desc>Polyak, Helsgaun, and Hybrid root lower-bound trajectories; '
         'the dashed line is the Concorde optimum.</desc>'),
        '<rect width="100%" height="100%" fill="#ffffff"/>',
        '<style>text{font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",'
        'Arial,sans-serif;fill:#172033}.tick{font-size:13px;fill:#596579}'
        '.axis{stroke:#334155;stroke-width:1.4}.grid{stroke:#dbe2ea;stroke-width:1}'
        '.legend{font-size:14px}.title{font-size:23px;font-weight:600}'
        '.subtitle{font-size:13px;fill:#64748b}</style>',
        (f'<text class="title" x="{chart_width / 2:.1f}" y="33" text-anchor="middle">'
         f'{html.escape(instance_name)} — root potential ascent</text>'),
        (f'<text class="subtitle" x="{chart_width / 2:.1f}" y="55" text-anchor="middle">'
         f'Per-phase cap: {configured_iterations}; {iterations_per_width} iterations '
         f'per {chart_width}px; thin = raw bound, bold = best-so-far'
         '</text>'),
    ]

    for tick in x_ticks:
        x = map_x(tick)
        elements.extend([
            f'<line class="grid" x1="{x:.2f}" y1="{top}" x2="{x:.2f}" y2="{top + plot_height}"/>',
            f'<text class="tick" x="{x:.2f}" y="{top + plot_height + 25}" text-anchor="middle">{tick}</text>',
        ])
    for tick in y_ticks:
        y = map_y(tick)
        elements.extend([
            f'<line class="grid" x1="{left}" y1="{y:.2f}" x2="{left + plot_width}" y2="{y:.2f}"/>',
            f'<text class="tick" x="{left - 14}" y="{y + 4:.2f}" text-anchor="end">{html.escape(format_number(tick))}</text>',
        ])
    elements.extend([
        f'<line class="axis" x1="{left}" y1="{top + plot_height}" x2="{left + plot_width}" y2="{top + plot_height}"/>',
        f'<line class="axis" x1="{left}" y1="{top}" x2="{left}" y2="{top + plot_height}"/>',
        f'<text x="{left + plot_width / 2:.2f}" y="{height - 34}" text-anchor="middle">Iteration</text>',
        (f'<text x="27" y="{top + plot_height / 2:.2f}" text-anchor="middle" '
         f'transform="rotate(-90 27 {top + plot_height / 2:.2f})">Root lower bound</text>'),
    ])

    optimum_y = map_y(optimum)
    elements.extend([
        '<g data-series="concorde">',
        (f'<line x1="{left}" y1="{optimum_y:.2f}" x2="{left + plot_width}" '
         f'y2="{optimum_y:.2f}" stroke="#111827" stroke-width="1.8" '
         'stroke-dasharray="8 6"/>'),
        (f'<text class="tick" x="{left + plot_width - 4}" y="{optimum_y - 8:.2f}" '
         f'text-anchor="end">Concorde optimum: {html.escape(format_number(optimum))}</text>'),
        '</g>',
    ])

    for strategy in STRATEGIES:
        trace = traces[strategy]
        elements.append(f'<g data-series="{strategy}">')
        raw_curve = [
            (map_x(point.iteration), map_y(point.lower_bound))
            for point in trace
        ]
        elements.append(
            f'<polyline points="{svg_polyline(raw_curve)}" fill="none" '
            f'stroke="{STRATEGY_COLORS[strategy]}" stroke-width="1.2" '
            'stroke-opacity="0.28" stroke-linejoin="round"/>'
        )
        curve = [(map_x(point.iteration), map_y(point.best_lower_bound)) for point in trace]
        # The achieved certificate remains valid after a strategy terminates, so
        # extend its best-so-far line to the configured comparison horizon.
        if trace[-1].iteration < x_max:
            curve.append((map_x(x_max), map_y(trace[-1].best_lower_bound)))
        elements.append(
            f'<polyline points="{svg_polyline(curve)}" fill="none" '
            f'stroke="{STRATEGY_COLORS[strategy]}" stroke-width="2.5" '
            'stroke-linejoin="round" stroke-linecap="round"/>'
        )
        elements.append('</g>')

    legend_width = 410
    legend_height = 82
    legend_box_x = left + plot_width - legend_width - 5
    legend_box_y = top + plot_height - legend_height - 5
    legend_x = legend_box_x + 10
    legend_y = legend_box_y + 17
    elements.append(
        f'<rect x="{legend_box_x}" y="{legend_box_y}" '
        f'width="{legend_width}" height="{legend_height}" '
        'rx="6" fill="#ffffff" fill-opacity="0.9"/>')
    for index, strategy in enumerate(STRATEGIES):
        y = legend_y + index * 25
        final_bound = traces[strategy][-1].best_lower_bound
        label = (
            f"{STRATEGY_LABELS[strategy]} "
            f"({len(traces[strategy])} evals, best {format_number(final_bound)})"
        )
        elements.extend([
            f'<g data-legend="{strategy}">',
            f'<line x1="{legend_x}" y1="{y}" x2="{legend_x + 30}" y2="{y}" stroke="{STRATEGY_COLORS[strategy]}" stroke-width="3"/>',
            f'<text class="legend" x="{legend_x + 39}" y="{y + 5}">{html.escape(label)}</text>',
            '</g>',
        ])

    elements.append('</svg>')
    path.write_text("\n".join(elements) + "\n", encoding="utf-8")


def safe_slug(instance: Path, used: set[str]) -> str:
    slug = re.sub(r"[^A-Za-z0-9_.-]+", "-", instance.stem).strip("-.") or "instance"
    if slug in used:
        digest = hashlib.sha256(str(instance).encode("utf-8")).hexdigest()[:8]
        slug = f"{slug}-{digest}"
    used.add(slug)
    return slug


def write_batch_outputs(
    output_root: Path,
    completed_instances: list[tuple[int, Path, Path, Path]],
) -> tuple[Path, Path]:
    """Write an instance-level CSV summary and a gallery for all SVG charts."""
    fieldnames = ["source_index", "instance", "dimension", "concorde_optimum"]
    for strategy in STRATEGIES:
        fieldnames.extend([
            f"{strategy}_lower_bound",
            f"{strategy}_gap_percent",
            f"{strategy}_evaluations",
        ])
    fieldnames.extend(["best_strategy", "chart"])

    summary_rows: list[dict[str, object]] = []
    gallery_cards: list[str] = []
    phase_caps: set[int] = set()
    for source_index, instance, chart, _combined in sorted(completed_instances):
        metadata = json.loads(
            (chart.parent / "metadata.json").read_text(encoding="utf-8"))
        phase_caps.add(int(metadata["iteration_limit_per_phase"]))
        optimum = float(metadata["concorde"]["optimum"])
        strategy_metadata = metadata["strategies"]
        bounds = {
            strategy: float(strategy_metadata[strategy]["root_lower_bound"])
            for strategy in STRATEGIES
        }
        best_bound = max(bounds.values())
        tolerance = 1e-9 * max(1.0, abs(best_bound))
        winners = [
            STRATEGY_LABELS[strategy]
            for strategy in STRATEGIES
            if best_bound - bounds[strategy] <= tolerance
        ]
        row: dict[str, object] = {
            "source_index": source_index,
            "instance": str(instance),
            "dimension": strategy_metadata["polyak"]["dimension"],
            "concorde_optimum": format(optimum, ".17g"),
            "best_strategy": "+".join(winners),
            "chart": chart.relative_to(output_root).as_posix(),
        }
        metric_cells: list[str] = []
        for strategy in STRATEGIES:
            bound = bounds[strategy]
            gap_percent = max(0.0, optimum - bound) / max(1.0, abs(optimum)) * 100.0
            evaluations = int(strategy_metadata[strategy]["evaluations"])
            row[f"{strategy}_lower_bound"] = format(bound, ".17g")
            row[f"{strategy}_gap_percent"] = format(gap_percent, ".10g")
            row[f"{strategy}_evaluations"] = evaluations
            metric_cells.append(
                "<tr>"
                f"<th>{html.escape(STRATEGY_LABELS[strategy])}</th>"
                f"<td>{html.escape(format_number(bound))}</td>"
                f"<td>{gap_percent:.3f}%</td>"
                f"<td>{evaluations}</td>"
                "</tr>")
        summary_rows.append(row)
        relative_chart = chart.relative_to(output_root).as_posix()
        inline_chart = chart.read_text(encoding="utf-8")
        if inline_chart.startswith("<?xml"):
            inline_chart = inline_chart.split("\n", 1)[1]
        switch_id_prefix = f"lines-{chart.parent.name}"
        switches = []
        for series, label in (
            ("polyak", "Polyak"),
            ("helsgaun", "Helsgaun"),
            ("hybrid", "Hybrid"),
            ("concorde", "Concorde"),
        ):
            switch_id = f"{switch_id_prefix}-{series}"
            switches.append(
                '<label class="line-switch" '
                f'for="{html.escape(switch_id)}">'
                f'<input id="{html.escape(switch_id)}" type="checkbox" '
                f'role="switch" data-series="{series}" checked>'
                f'<span class="line-swatch {series}" aria-hidden="true"></span>'
                f'<span>{label}</span></label>')
        gallery_cards.append(
            '<article class="card">'
            f'<h2>{source_index}. {html.escape(instance.name)}</h2>'
            f'<p>n={row["dimension"]} · Concorde optimum '
            f'{html.escape(format_number(optimum))} · best '
            f'{html.escape(" + ".join(winners))}</p>'
            f'<div class="chart-scroll">{inline_chart}</div>'
            '<table><thead><tr><th>Strategy</th><th>Lower bound</th>'
            '<th>Gap</th><th>Evals</th></tr></thead><tbody>'
            + "".join(metric_cells)
            + '</tbody></table>'
            '<fieldset class="line-switches"><legend>Visible lines</legend>'
            + "".join(switches)
            + '</fieldset>'
            f'<a class="chart-link" href="{html.escape(relative_chart)}">'
            'Open standalone SVG</a></article>')

    summary_path = output_root / "summary.csv"
    with summary_path.open("w", newline="", encoding="utf-8") as output:
        writer = csv.DictWriter(output, fieldnames=fieldnames)
        writer.writeheader()
        writer.writerows(summary_rows)

    gallery_path = output_root / "index.html"
    phase_cap_label = ", ".join(str(value) for value in sorted(phase_caps))
    gallery_path.write_text(
        """<!doctype html>
<html lang="en"><head><meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Root potential ascent trends</title>
<style>
body{margin:0;background:#f1f5f9;color:#172033;font-family:-apple-system,BlinkMacSystemFont,"Segoe UI",Arial,sans-serif}
header{padding:28px 4vw 18px;background:#0f172a;color:white}header h1{margin:0 0 8px;font-size:26px}header p{margin:0;color:#cbd5e1}
main{display:grid;grid-template-columns:1fr;gap:20px;max-width:1680px;margin:0 auto;padding:24px 3vw 40px}.card{background:white;border:1px solid #dbe2ea;border-radius:12px;padding:16px;box-shadow:0 3px 12px #0f172a12}.card h2{margin:0;font-size:19px}.card p{margin:7px 0 12px;color:#64748b;font-size:14px}.chart-scroll{overflow-x:auto;border:1px solid #e2e8f0}.chart-scroll svg{display:block;width:auto;max-width:none;height:720px}.card table{width:100%;margin-top:12px;border-collapse:collapse;font-size:13px}.card th,.card td{padding:6px 8px;border-bottom:1px solid #e2e8f0;text-align:right}.card th:first-child{text-align:left}.card thead th{color:#64748b}.line-switches{display:flex;flex-wrap:wrap;gap:10px 22px;margin:14px 0 8px;padding:10px 12px;border:1px solid #dbe2ea;border-radius:8px}.line-switches legend{padding:0 6px;color:#64748b;font-size:13px}.line-switch{display:inline-flex;align-items:center;gap:8px;cursor:pointer;font-size:14px}.line-switch input{width:17px;height:17px;margin:0}.line-swatch{display:inline-block;width:30px;height:3px;background:#2563eb}.line-swatch.helsgaun{background:#dc2626}.line-swatch.hybrid{background:#059669}.line-swatch.concorde{height:0;background:none;border-top:2px dashed #111827}.chart-link{display:inline-block;margin-top:4px;font-size:13px;color:#2563eb}
@media(max-width:620px){main{grid-template-columns:1fr;padding:12px}.card{padding:10px}}
</style></head><body>
<header><h1>Root potential ascent trends</h1>
<p>Polyak, Helsgaun, and Hybrid · Concorde references · """
        + html.escape(phase_cap_label)
        + """ evaluations per phase</p></header>
<main>
"""
        + "\n".join(gallery_cards)
        + """
</main>
<script>
document.addEventListener("change", function (event) {
  const control = event.target;
  if (!(control instanceof HTMLInputElement) ||
      !control.matches(".line-switches input[data-series]")) return;
  const card = control.closest(".card");
  const chart = card ? card.querySelector(".chart-scroll svg") : null;
  if (!chart) return;
  const series = control.dataset.series;
  chart.querySelectorAll('[data-series="' + series + '"]').forEach(function (group) {
    group.style.display = control.checked ? "" : "none";
  });
  chart.querySelectorAll('[data-legend="' + series + '"]').forEach(function (group) {
    group.style.opacity = control.checked ? "1" : "0.25";
  });
});
</script></body></html>
""",
        encoding="utf-8")
    return summary_path, gallery_path


def process_instance(
    instance: Path,
    output_directory: Path,
    solver: Path,
    concorde: Path | None,
    cached_concorde: CachedConcordeResults | None,
    iterations: int,
    chart_width: int,
    iterations_per_width: int,
    exact_max_n: int,
    timeout: float,
    concorde_seed: int,
) -> tuple[Path, Path]:
    output_directory.mkdir(parents=True, exist_ok=True)
    if cached_concorde is not None:
        try:
            optimum = cached_concorde.optimum_by_instance[instance]
        except KeyError as error:
            raise RuntimeError(
                f"no successful cached Concorde result for {instance}") from error
        concorde_metadata: dict[str, object] = {
            "reused": True,
            "source_results": str(cached_concorde.source),
            "optimum": optimum,
        }
    else:
        if concorde is None:
            raise RuntimeError("Concorde executable is not configured")
        optimum = run_concorde(
            instance, concorde, concorde_seed, timeout, output_directory)
        concorde_metadata = {
            "reused": False,
            "executable": str(concorde),
            "seed": concorde_seed,
            "optimum": optimum,
        }
    traces: dict[str, list[TracePoint]] = {}
    strategy_metadata: dict[str, dict[str, float | int | str]] = {}
    for strategy in STRATEGIES:
        trace_path = output_directory / f"{strategy}.csv"
        points, metadata = run_strategy(
            instance, strategy, solver, iterations, exact_max_n, timeout,
            trace_path)
        traces[strategy] = points
        strategy_metadata[strategy] = metadata

    combined_path = output_directory / "root-ascent-trends.csv"
    chart_path = output_directory / "root-ascent-trends.svg"
    write_combined_csv(combined_path, instance, optimum, traces)
    write_svg_chart(
        chart_path, instance.name, optimum, iterations, traces, chart_width,
        iterations_per_width)
    metadata = {
        "instance": str(instance),
        "iteration_limit_per_phase": iterations,
        "chart_width_px": chart_width,
        "iterations_per_chart_width": iterations_per_width,
        "root_bound_only": True,
        "concorde": concorde_metadata,
        "solver": str(solver),
        "strategies": strategy_metadata,
    }
    (output_directory / "metadata.json").write_text(
        json.dumps(metadata, indent=2, ensure_ascii=False) + "\n",
        encoding="utf-8")
    return chart_path, combined_path


def main(argv: list[str] | None = None) -> int:
    args = parse_args(argv)
    try:
        cached_concorde = (
            load_cached_concorde_results(args.concorde_results)
            if args.concorde_results is not None else None
        )
        if args.instances or args.batch_list:
            instances = configured_instances(args.instances, args.batch_list)
        elif cached_concorde is not None:
            instances = list(cached_concorde.instances)
        else:
            raise ValueError("no instances were selected")
        solver = validate_executable(args.solver, "tsp_bb")
        concorde = (
            None if cached_concorde is not None
            else validate_executable(args.concorde, "Concorde")
        )
        if cached_concorde is not None:
            missing = [
                instance for instance in instances
                if instance not in cached_concorde.optimum_by_instance
            ]
            if missing:
                raise ValueError(
                    "selected instances lack cached Concorde results: "
                    + ", ".join(str(instance) for instance in missing))
    except (OSError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 2

    output_root = args.output_root.expanduser().resolve()
    output_root.mkdir(parents=True, exist_ok=True)
    used_slugs: set[str] = set()
    jobs: list[tuple[int, Path, Path]] = []
    for index, instance in enumerate(instances, start=1):
        destination = output_root / safe_slug(instance, used_slugs)
        jobs.append((index, instance, destination))

    failures = 0
    completed_count = 0
    completed_instances: list[tuple[int, Path, Path, Path]] = []
    with ThreadPoolExecutor(max_workers=args.workers) as executor:
        future_jobs = {
            executor.submit(
                process_instance,
                instance, destination, solver, concorde, cached_concorde,
                args.iterations, args.chart_width, args.iterations_per_width,
                args.exact_max_n, args.timeout, args.concorde_seed,
            ): (index, instance)
            for index, instance, destination in jobs
        }
        for future in as_completed(future_jobs):
            index, instance = future_jobs[future]
            completed_count += 1
            try:
                chart, combined = future.result()
                print(
                    f"[{completed_count}/{len(instances)}; source #{index}] "
                    f"{instance.name}")
                print(f"  chart: {chart}")
                print(f"  data:  {combined}")
                completed_instances.append((index, instance, chart, combined))
            except (OSError, RuntimeError, subprocess.TimeoutExpired) as error:
                failures += 1
                print(
                    f"[{completed_count}/{len(instances)}; source #{index}] "
                    f"{instance.name}: {error}", file=sys.stderr)
    if completed_instances:
        summary, gallery = write_batch_outputs(output_root, completed_instances)
        print(f"summary: {summary}")
        print(f"gallery: {gallery}")
    return 1 if failures else 0


if __name__ == "__main__":
    raise SystemExit(main())
