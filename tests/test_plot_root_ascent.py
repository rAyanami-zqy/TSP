"""Lightweight tests for the dependency-free root-ascent chart writer."""

from __future__ import annotations

import contextlib
import csv
import io
import sys
import tempfile
import unittest
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(PROJECT_ROOT / "tools"))

import plot_root_ascent as plotter  # noqa: E402


class RootAscentPlotTests(unittest.TestCase):
    def traces(self) -> dict[str, list[plotter.TracePoint]]:
        return {
            strategy: [
                plotter.TracePoint(strategy, 1, strategy, 1, 10.0, 10.0),
                plotter.TracePoint(strategy, 2, strategy, 2, 11.0, 11.0),
            ]
            for strategy in plotter.STRATEGIES
        }

    def test_svg_contains_all_strategies_and_concorde_reference(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / "trend.svg"
            plotter.write_svg_chart(
                output, "sample.tsp", 12.0, 2000, self.traces())
            content = output.read_text(encoding="utf-8")
        self.assertIn("Polyak", content)
        self.assertIn("Helsgaun", content)
        self.assertIn("Hybrid P→H", content)
        self.assertIn("Hybrid H→P", content)
        self.assertIn("Polyak + H direction fixed", content)
        self.assertIn("Polyak + H direction dynamic", content)
        self.assertIn("Concorde optimum: 12", content)
        self.assertIn("Per-phase cap: 2000", content)
        self.assertIn("thin = raw bound", content)
        self.assertIn('width="7380"', content)
        self.assertIn('viewBox="0 0 7380 720"', content)
        self.assertIn("400 iterations per 1600px", content)
        self.assertIn('<rect x="6810.0" y="453.0"', content)
        self.assertIn('<g data-series="polyak">', content)
        self.assertIn('<g data-series="helsgaun">', content)
        self.assertIn('<g data-series="hybrid">', content)
        self.assertIn('<g data-series="hybrid-reverse">', content)
        self.assertIn('<g data-series="polyak-smoothed">', content)
        self.assertIn('<g data-series="polyak-smoothed-dynamic">', content)
        self.assertIn('<g data-series="concorde">', content)
        self.assertIn('<g data-legend="polyak">', content)
        self.assertEqual(
            content.count('stroke-opacity="0.28"'), len(plotter.STRATEGIES))

    def test_combined_csv_keeps_raw_and_best_bounds(self) -> None:
        with tempfile.TemporaryDirectory() as temporary:
            output = Path(temporary) / "trend.csv"
            plotter.write_combined_csv(
                output, Path("sample.tsp"), 12.0, self.traces())
            with output.open(newline="", encoding="utf-8") as source:
                rows = list(csv.DictReader(source))
        self.assertEqual(len(rows), 2 * len(plotter.STRATEGIES))
        self.assertEqual(rows[0]["strategy"], "polyak")
        self.assertEqual(rows[0]["lower_bound"], "10")
        self.assertEqual(rows[0]["best_lower_bound"], "10")
        self.assertEqual(rows[0]["concorde_optimum"], "12")

    def test_cached_concorde_results_localize_another_checkout(self) -> None:
        instance = PROJECT_ROOT / "examples" / "five-city.txt"
        configured = "/another/checkout/data/../examples/five-city.txt"
        with tempfile.TemporaryDirectory() as temporary:
            source = Path(temporary) / "results.csv"
            with source.open("w", newline="", encoding="utf-8") as output:
                writer = csv.DictWriter(
                    output, fieldnames=["instance", "status", "result"])
                writer.writeheader()
                writer.writerow({
                    "instance": str(instance), "status": "ok", "result": "26"})
                writer.writerow({
                    "instance": configured, "status": "timeout", "result": ""})
            cached = plotter.load_cached_concorde_results(source)
        self.assertEqual(cached.instances, (instance.resolve(),))
        self.assertEqual(cached.optimum_by_instance[instance.resolve()], 26.0)

    def test_direction_smoothing_arguments_are_configurable(self) -> None:
        args = plotter.parse_args([
            "--root-ascent-smoothing-current-weight", "0.65",
            "--root-ascent-dynamic-cosine-scale", "0.15",
            "--root-ascent-dynamic-min-current-weight", "0.4",
            "--root-ascent-dynamic-max-current-weight", "0.85",
            "sample.tsp",
        ])
        self.assertEqual(args.root_ascent_smoothing_current_weight, 0.65)
        self.assertEqual(args.root_ascent_dynamic_cosine_scale, 0.15)
        self.assertEqual(args.root_ascent_dynamic_min_current_weight, 0.4)
        self.assertEqual(args.root_ascent_dynamic_max_current_weight, 0.85)

    def test_direction_smoothing_arguments_reject_invalid_order(self) -> None:
        with contextlib.redirect_stderr(io.StringIO()):
            with self.assertRaises(SystemExit):
                plotter.parse_args([
                    "--root-ascent-smoothing-current-weight", "0.4",
                    "--root-ascent-dynamic-min-current-weight", "0.5",
                    "sample.tsp",
                ])


if __name__ == "__main__":
    unittest.main()
