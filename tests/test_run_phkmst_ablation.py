"""run_phkmst_ablation.py 兼容层的轻量回归测试。"""

from __future__ import annotations

import sys
import unittest
from dataclasses import replace
from pathlib import Path
from types import SimpleNamespace


# 被测脚本沿用项目工具脚本的直接 import 方式，需要把 tools 放入模块路径。
PROJECT_ROOT = Path(__file__).resolve().parent.parent
sys.path.insert(0, str(PROJECT_ROOT / "tools"))

import run_phkmst_ablation as runner  # noqa: E402


class SolverCompatibilityTests(unittest.TestCase):
    HELP = """\
Usage: tsp_bb [options] input
Options:
  --exact-max-n <n>
  --hk-ascent <none|polyak|helsgaun|hybrid>
  --hk-potential-update <none|subtree-depth|subtree-adaptive>
  --hk-update-depth <n>
  --hk-update-iterations <n>
  --hk-update-gap-ratio <x>
  --hk-update-min-gap-ratio <x>
  --hk-update-budget <n>
  --debug
"""

    def setUp(self) -> None:
        self.interface = runner.parse_solver_interface(self.HELP)

    def test_code_configurations_bind_each_solver_and_its_arguments(self) -> None:
        for strategy in runner.SOLVER_CONFIGURATIONS:
            self.assertIsInstance(strategy.executable, Path)
            self.assertIsInstance(strategy.solver_args, tuple)

    def test_solver_arguments_accepts_copyable_command_line_text(self) -> None:
        self.assertEqual(
            runner.solver_arguments("--mode current --label 'two words'"),
            ("--mode", "current", "--label", "two words"),
        )

    def test_help_probe_extracts_options_and_enum_values(self) -> None:
        self.assertIsNotNone(self.interface.options)
        assert self.interface.options is not None
        self.assertTrue(self.interface.options["--exact-max-n"].takes_value)
        self.assertEqual(
            self.interface.options["--hk-potential-update"].accepted_values,
            frozenset({"none", "subtree-depth", "subtree-adaptive"}),
        )
        self.assertFalse(self.interface.options["--debug"].takes_value)

    def test_safe_defaults_are_omitted_but_meaningful_difference_is_rejected(
        self,
    ) -> None:
        default = runner.CONFIGURATION_BY_NAME["D0"]
        adapted_default = runner.adapt_strategy_arguments(
            default, self.interface)
        self.assertFalse(adapted_default.incompatibilities)
        self.assertIn("--hk-node-ascent=polyak", adapted_default.omitted)
        self.assertIn("--branch-edge-order=weight", adapted_default.omitted)

        incompatible = replace(
            default,
            name="unsupported-node-ascent",
            solver_args=runner.solver_arguments("--hk-node-ascent helsgaun"),
        )
        adapted_incompatible = runner.adapt_strategy_arguments(
            incompatible, self.interface)
        self.assertTrue(any(
            "--hk-node-ascent=helsgaun" in reason
            for reason in adapted_incompatible.incompatibilities
        ))

    def test_managed_options_follow_solver_capabilities(self) -> None:
        args = SimpleNamespace(
            concorde_seed=123,
            exact_max_n=130,
            debug=True,
            debug_interval=5_000_000,
        )
        adapted = runner.effective_args(
            args, runner.CONFIGURATION_BY_NAME["D0"], self.interface)
        self.assertIn("--exact-max-n", adapted.arguments)
        self.assertIn("--debug", adapted.arguments)
        self.assertNotIn("--debug-interval", adapted.arguments)
        self.assertIn("--debug-interval (runner-managed)", adapted.omitted)


class OutputCompatibilityTests(unittest.TestCase):
    def test_concorde_uses_only_time_result_and_branch_outputs(self) -> None:
        concorde = runner.CONFIGURATION_BY_NAME["Concorde"]
        self.assertEqual(
            runner.result_fields_for(concorde),
            (
                "run_id", "strategy", "repeat", "instance", "status",
                "wall_seconds", "result", "branches",
            ),
        )
        self.assertEqual(
            runner.summary_fields_for(concorde),
            (
                "run_id", "strategy", "status", "rows", "successful",
                "timeouts", "errors", "total_wall_seconds",
                "median_wall_seconds", "total_branches", "median_branches",
            ),
        )

    def test_configured_statistics_are_projected_from_solver_output(self) -> None:
        parsed = runner.parse_tspbb_statistics("""\
Root potential iterations: 144
Nodes created: 1,234
Nodes expanded: 987
Pruned by bound: 201
Pruned by infeasibility: 46
Search-node potential update candidates: 103
Search-node potential updates triggered: 17
Potential updates skipped strategy none: 1
Potential updates skipped update depth zero: 2
Potential updates skipped budget exhausted: 3
Potential updates skipped numerically unsafe: 4
Potential updates skipped invalid state: 5
Potential updates skipped zero violation: 6
Potential updates skipped zero iteration limit: 7
Potential updates skipped depth interval: 8
Potential updates skipped gap below minimum: 9
Potential updates skipped gap above maximum: 41
Search-node potential iterations: 91
Optimal cost: 2.6e1
""")
        self.assertEqual(parsed, {
            "result": 26.0,
            "branches": 1234,
            "nodes_expanded": 987,
            "pruned_by_bound": 201,
            "pruned_infeasible": 46,
            "root_potential_iterations": 144,
            "search_node_potential_update_candidates": 103,
            "search_node_potential_updates_triggered": 17,
            "search_node_potential_updates_skipped_strategy_none": 1,
            "search_node_potential_updates_skipped_update_depth_zero": 2,
            "search_node_potential_updates_skipped_budget_exhausted": 3,
            "search_node_potential_updates_skipped_numerically_unsafe": 4,
            "search_node_potential_updates_skipped_invalid_state": 5,
            "search_node_potential_updates_skipped_zero_violation": 6,
            "search_node_potential_updates_skipped_zero_iteration_limit": 7,
            "search_node_potential_updates_skipped_depth_interval": 8,
            "search_node_potential_updates_skipped_gap_below_minimum": 9,
            "search_node_potential_updates_skipped_gap_above_maximum": 41,
            "search_node_potential_iterations": 91,
        })

    def test_default_csv_fields_include_potential_statistics(self) -> None:
        self.assertEqual(
            runner.RESULT_FIELDS,
            (
                "run_id", "strategy", "repeat", "instance", "status",
                "wall_seconds", "result", "branches",
                "nodes_expanded",
                "pruned_by_bound", "pruned_infeasible",
                "root_potential_iterations",
                "search_node_potential_update_candidates",
                "search_node_potential_updates_triggered",
                "search_node_potential_updates_skipped_strategy_none",
                "search_node_potential_updates_skipped_update_depth_zero",
                "search_node_potential_updates_skipped_budget_exhausted",
                "search_node_potential_updates_skipped_numerically_unsafe",
                "search_node_potential_updates_skipped_invalid_state",
                "search_node_potential_updates_skipped_zero_violation",
                "search_node_potential_updates_skipped_zero_iteration_limit",
                "search_node_potential_updates_skipped_depth_interval",
                "search_node_potential_updates_skipped_gap_below_minimum",
                "search_node_potential_updates_skipped_gap_above_maximum",
                "search_node_potential_iterations",
            ),
        )
        self.assertEqual(
            runner.SUMMARY_FIELDS,
            (
                "run_id", "strategy", "status", "rows", "successful",
                "timeouts", "errors", "total_wall_seconds",
                "median_wall_seconds", "total_branches", "median_branches",
                "total_nodes_expanded", "median_nodes_expanded",
                "total_pruned_by_bound", "median_pruned_by_bound",
                "total_pruned_infeasible", "median_pruned_infeasible",
                "total_root_potential_iterations",
                "median_root_potential_iterations",
                "total_search_node_potential_update_candidates",
                "median_search_node_potential_update_candidates",
                "total_search_node_potential_updates_triggered",
                "median_search_node_potential_updates_triggered",
                "total_search_node_potential_updates_skipped_strategy_none",
                "median_search_node_potential_updates_skipped_strategy_none",
                "total_search_node_potential_updates_skipped_update_depth_zero",
                "median_search_node_potential_updates_skipped_update_depth_zero",
                "total_search_node_potential_updates_skipped_budget_exhausted",
                "median_search_node_potential_updates_skipped_budget_exhausted",
                "total_search_node_potential_updates_skipped_numerically_unsafe",
                "median_search_node_potential_updates_skipped_numerically_unsafe",
                "total_search_node_potential_updates_skipped_invalid_state",
                "median_search_node_potential_updates_skipped_invalid_state",
                "total_search_node_potential_updates_skipped_zero_violation",
                "median_search_node_potential_updates_skipped_zero_violation",
                "total_search_node_potential_updates_skipped_zero_iteration_limit",
                "median_search_node_potential_updates_skipped_zero_iteration_limit",
                "total_search_node_potential_updates_skipped_depth_interval",
                "median_search_node_potential_updates_skipped_depth_interval",
                "total_search_node_potential_updates_skipped_gap_below_minimum",
                "median_search_node_potential_updates_skipped_gap_below_minimum",
                "total_search_node_potential_updates_skipped_gap_above_maximum",
                "median_search_node_potential_updates_skipped_gap_above_maximum",
                "total_search_node_potential_iterations",
                "median_search_node_potential_iterations",
            ),
        )


if __name__ == "__main__":
    unittest.main()
