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


if __name__ == "__main__":
    unittest.main()
