"""Tests for the PHKMST ablation report writer."""

from __future__ import annotations

import sys
import unittest
from pathlib import Path


PROJECT_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(PROJECT_ROOT / "tools"))

import summarize_phkmst_ablation as summarizer  # noqa: E402


class AblationHtmlReportTests(unittest.TestCase):
    def make_run(self, label: str, value: float) -> summarizer.Run:
        numeric = {
            "wall_seconds": value,
            "branches": value,
            **{
                field: value
                for field in summarizer.COLLAPSIBLE_SKIP_REASON_FIELDS
            },
        }
        instance = summarizer.InstanceResult(
            name="sample.tsp",
            source="sample.tsp",
            status="ok",
            observed_repeats=1,
            expected_repeats=1,
            successful_repeats=1,
            timeouts=0,
            errors=0,
            result=100.0,
            result_consistent=True,
            numeric=numeric,
            text={},
        )
        return summarizer.Run(
            directory=Path(label),
            run_id=label,
            strategy=label,
            kind="test",
            category="test",
            description="",
            executable_sha256="",
            options={},
            timeout_seconds=None,
            workers=None,
            repeats=1,
            declared_instance_count=1,
            configuration={"solver_args": ""},
            instances={instance.name: instance},
            numeric_fields=tuple(numeric),
            text_fields=(),
            label=label,
        )

    def configured_run(
        self, label: str, value: float, **option_overrides: str,
    ) -> summarizer.Run:
        run = self.make_run(label, value)
        run.options.update({
            "--hk-ascent": "polyak",
            "--hk-node-ascent": "polyak",
            "--branch-edge-order": "weight",
            "--hk-potential-update": "subtree-adaptive",
            "--hk-update-depth": "1",
            "--hk-update-gap-ratio": "0.02",
            "--hk-update-min-gap-ratio": "0.0",
            "--hk-update-iterations": "32",
            "--hk-update-budget": "0",
            **option_overrides,
        })
        return run

    def test_skip_reason_columns_are_collapsed_and_expandable(self) -> None:
        left = self.make_run("A", 1.0)
        right = self.make_run("B", 2.0)
        comparison = summarizer.Comparison(left, right, "test", "A", "B")

        report = summarizer.html_report(
            Path("input"), [left, right], [comparison], [], None, [])

        self.assertIn(
            'class="skip-columns-toggle" aria-expanded="false"', report)
        self.assertIn("展开跳过原因（10 列/侧）", report)
        self.assertEqual(report.count('data-collapsed-colspan="3"'), 2)
        self.assertEqual(report.count('data-expanded-colspan="13"'), 2)
        self.assertEqual(report.count('class="num skip-reason-column"'), 20)
        for field in summarizer.COLLAPSIBLE_SKIP_REASON_FIELDS:
            self.assertEqual(
                report.count(
                    f'class="skip-reason-column">'
                    f'{summarizer.metric_label(field)}</th>'),
                2,
            )
        self.assertIn(
            ".detail-table .skip-reason-column{display:none}", report)
        self.assertIn(
            'table.classList.toggle("show-skip-reasons", expanded)', report)

    def test_comparisons_follow_requested_group_order(self) -> None:
        iteration = summarizer.Comparison(
            self.configured_run("i32", 1.0),
            self.configured_run("i8", 2.0,
                                **{"--hk-update-iterations": "8"}),
            "hk update iterations", "32", "8")
        depth = summarizer.Comparison(
            self.configured_run("d1", 1.0),
            self.configured_run("d2", 2.0,
                                **{"--hk-update-depth": "2"}),
            "hk update depth", "1", "2")
        ratio = summarizer.Comparison(
            self.configured_run("r2", 1.0),
            self.configured_run("r5", 2.0,
                                **{"--hk-update-gap-ratio": "0.05"}),
            "hk update gap ratio", "0.02", "0.05")
        strategy = summarizer.Comparison(
            self.configured_run("polyak", 1.0),
            self.configured_run("helsgaun", 2.0,
                                **{"--hk-node-ascent": "helsgaun"}),
            "hk node ascent", "polyak", "helsgaun")
        other = summarizer.Comparison(
            self.configured_run("root-polyak", 1.0),
            self.configured_run("root-hybrid", 2.0,
                                **{"--hk-ascent": "hybrid"}),
            "hk ascent", "polyak", "hybrid")

        ordered = summarizer.organize_comparisons(
            [other, strategy, ratio, depth, iteration])

        self.assertEqual(
            [summarizer.comparison_group_id(item) for item in ordered],
            [
                "polyak-iterations",
                "polyak-depth-iterations-32",
                "polyak-ratio-iterations-32",
                "node-ascent-strategy",
                "other",
            ],
        )

    def test_iteration_comparison_names_the_better_setting(self) -> None:
        comparison = summarizer.Comparison(
            self.configured_run("i32", 1.0),
            self.configured_run("i64", 2.0,
                                **{"--hk-update-iterations": "64"}),
            "hk update iterations", "32", "64")

        verdict = summarizer.iteration_comparison_verdict(comparison)

        self.assertIsNotNone(verdict)
        assert verdict is not None
        self.assertIn("hk-update-iterations=32 更优", verdict)
        self.assertIn("1.000 s 对 2.000 s", verdict)

    def test_focused_profile_has_only_requested_three_sections(self) -> None:
        strategies = summarizer.focused_profile_selectors("Concorde")
        runs = [self.make_run(strategy, 1.0) for strategy in strategies]

        comparisons = summarizer.focused_profile_comparisons(runs)

        self.assertEqual(len(runs), 17)
        self.assertEqual(len(comparisons), 22)
        self.assertEqual(
            [summarizer.comparison_group_id(item) for item in comparisons],
            ["focused-p32"] * 8
            + ["focused-p32-no-node-update"] * 8
            + ["focused-p33-iterations"] * 6,
        )
        self.assertEqual(
            [(item.left.strategy, item.right.strategy) for item in comparisons[-6:]],
            [
                ("P33>1%", "P33>1%64"),
                ("P33>1%", "P33>1%128"),
                ("P33>2%", "P33>2%64"),
                ("P33>2%", "P33>2%128"),
                ("P33>5%", "P33>5%64"),
                ("P33>5%", "P33>5%128"),
            ],
        )


if __name__ == "__main__":
    unittest.main()
