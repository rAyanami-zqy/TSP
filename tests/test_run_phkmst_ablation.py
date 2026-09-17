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

    def test_branch_ablation_configurations_keep_p32_baseline_neutral(self) -> None:
        names = (
            "GAPMST-P32-LKH-BP-base",
            "GAPMST-P32-LKH-BP-lift2",
            "GAPMST-P32-LKH-BP-split2",
            "GAPMST-P32-LKH-BP-lift2-split2",
            "GAPMST-P32-LKH-BP-ascent-strong",
            "GAPMST-P32-LKH-BP-ascent-strong-lift2",
            "GAPMST-P32-LKH-BP-ascent-strong-split2",
            "GAPMST-P32-LKH-BP-ascent-strong-lift2-split2",
        )
        for name in names:
            strategy = runner.CONFIGURATION_BY_NAME[name]
            arguments = list(strategy.solver_args)

            def value(option: str) -> str:
                return arguments[arguments.index(option) + 1]

            self.assertEqual(strategy.executable, PROJECT_ROOT / "build" / "tsp_bb")
            self.assertEqual(value("--hk-update-iterations"), "32")
            self.assertEqual(value("--hk-update-budget"), "0")
            self.assertEqual(value("--hk-update-max-depth"), "0")
            self.assertEqual(value("--hk-update-skip-last-edges"), "0")
            self.assertEqual(value("--hk-update-min-gap-change-ratio"), "0")
            self.assertEqual(value("--root-candidate-compaction"), "off")
            self.assertFalse(any(
                option in arguments
                for option in (
                    "--hk-update-large-gap-ratio",
                    "--hk-update-shallow-depth",
                    "--hk-update-slow-warm-depth",
                    "--hk-update-probe-updates",
                )
            ))

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
        default = replace(
            runner.CONFIGURATION_BY_NAME["GAPMST-P32-LKH"],
            name="default",
            solver_args=runner.solver_arguments(
                "--hk-node-ascent polyak --branch-edge-order weight "
                "--hk-potential-update none"),
        )
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
        default = replace(
            runner.CONFIGURATION_BY_NAME["GAPMST-P32-LKH"],
            name="default",
            solver_args=runner.solver_arguments(
                "--hk-node-ascent polyak --branch-edge-order weight "
                "--hk-potential-update none"),
        )
        adapted = runner.effective_args(
            args, default, self.interface)
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
Root fixing calls: 2
Root fixing tested: 80
Root fixing fixed zero: 30
Root fixing tree tested: 40
Root fixing fixed one: 3
Root fixing active after: 200
Root fixing seconds: 0.125
Root candidate compaction calls: 2
Root candidate edges before: 1000
Root candidate edges after: 203
Root candidate compaction seconds: 0.015
LKH provider calls: 1
LKH provider failures: 0
LKH provider seconds: 0.125
Search-node potential update candidates: 103
Search-node potential updates triggered: 17
Potential updates skipped strategy none: 1
Potential updates skipped update depth zero: 2
Potential updates skipped budget exhausted: 3
Potential updates skipped numerically unsafe: 4
Potential updates skipped invalid state: 5
Potential updates skipped zero violation: 6
Potential updates skipped zero iteration limit: 7
Potential updates skipped max depth: 10
Potential updates skipped near leaf: 12
Potential updates skipped depth interval: 8
Potential updates skipped gap change below minimum: 11
Potential update gap-change shallow bypasses: 13
Branch lift-first reorders: 14
Branch zero-gain splits: 15
Branch ascent-strong probes: 16
Branch ascent-strong seconds: 0.375
Search-node potential iterations: 91
Optimal cost: 2.6e1
""")
        expected = {
            "result": 26.0,
            "branches": 1234,
            "nodes_expanded": 987,
            "pruned_by_bound": 201,
            "pruned_infeasible": 46,
            "root_fixing_calls": 2,
            "root_fixing_tested": 80,
            "root_fixing_fixed_zero": 30,
            "root_fixing_tree_tested": 40,
            "root_fixing_fixed_one": 3,
            "root_fixing_active_after": 200,
            "root_fixing_seconds": 0.125,
            "root_candidate_compaction_calls": 2,
            "root_candidate_edges_before": 1000,
            "root_candidate_edges_after": 203,
            "root_candidate_compaction_seconds": 0.015,
            "lkh_provider_calls": 1,
            "lkh_provider_failures": 0,
            "lkh_provider_seconds": 0.125,
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
            "search_node_potential_updates_skipped_max_depth": 10,
            "search_node_potential_updates_skipped_near_leaf": 12,
            "search_node_potential_updates_skipped_depth_interval": 8,
            "search_node_potential_updates_skipped_gap_change_below_minimum": 11,
            "potential_update_gap_change_shallow_bypasses": 13,
            "branch_lift_first_reorders": 14,
            "branch_zero_gain_splits": 15,
            "branch_ascent_strong_probes": 16,
            "branch_ascent_strong_seconds": 0.375,
            "search_node_potential_iterations": 91,
        }
        for field, value in expected.items():
            self.assertEqual(parsed[field], value)
        self.assertIsNone(parsed["final_upper_bound"])
        self.assertIsNone(parsed["replacement_seconds"])

    def test_default_csv_fields_include_potential_statistics(self) -> None:
        self.assertEqual(runner.RESULT_FIELDS[:6], runner.RESULT_BASE_FIELDS)
        self.assertEqual(len(runner.RESULT_FIELDS), len(set(runner.RESULT_FIELDS)))
        for field in (
            "final_upper_bound", "final_lower_bound", "final_relative_gap",
            "initial_tour_seconds", "root_ascent_seconds",
            "lkh_provider_calls", "lkh_provider_failures",
            "lkh_provider_seconds",
            "root_candidate_compaction_seconds",
            "branch_lift_first_reorders", "branch_zero_gain_splits",
            "branch_ascent_strong_probes", "branch_ascent_strong_seconds",
            "potential_update_seconds", "potential_update_rebuild_seconds",
            "replacement_seconds",
        ):
            self.assertIn(field, runner.RESULT_FIELDS)
        for field in (
            "total_initial_tour_seconds", "median_root_ascent_seconds",
            "total_root_candidate_compaction_seconds",
            "total_replacement_seconds",
        ):
            self.assertIn(field, runner.SUMMARY_FIELDS)

    def test_timeout_progress_recovers_latest_certificate_and_nodes(self) -> None:
        parsed = runner.parse_tspbb_progress("", """\
[tsp-debug] initial incumbent: cost=120
[tsp-debug] LKH provider: calls=1 failures=0 seconds=0.125
[tsp-debug] initial tour timing: seconds=0.5 clk_starts=1
[tsp-debug] root reduced-cost fixing: tested=80 fixed_zero=30 tree_tested=40 fixed_one=3 active=200 seconds=0.1
[tsp-debug] root candidate compaction: before=1000 after=203 active=200 forced=3 seconds=0.015
[tsp-debug] root: lower_bound=100 best=120 created=1 expanded=0 initial_tour_seconds=0.5 root_ascent_seconds=0.2 root_fixing_seconds=0.1 root_candidate_compaction_seconds=0.015 replacement_seconds=0.01 search=bp-chain
[tsp-debug] new incumbent: cost=110 source=bp-node depth=4
[tsp-debug] progress: expanded=500 created=900 depth=8 bound=105 best=110 pruned_bound=3 pruned_infeasible=4 potential_seconds=2.5 potential_rebuild_seconds=0.4 replacement_seconds=1.25
""")
        self.assertEqual(parsed["final_upper_bound"], 110.0)
        self.assertEqual(parsed["final_lower_bound"], 100.0)
        self.assertAlmostEqual(parsed["final_relative_gap"], 10.0 / 110.0)
        self.assertEqual(parsed["branches"], 900)
        self.assertEqual(parsed["nodes_expanded"], 500)
        self.assertEqual(parsed["root_fixing_calls"], 1)
        self.assertEqual(parsed["root_fixing_fixed_zero"], 30)
        self.assertEqual(parsed["root_candidate_compaction_calls"], 1)
        self.assertEqual(parsed["root_candidate_edges_before"], 1000)
        self.assertEqual(parsed["root_candidate_edges_after"], 203)
        self.assertEqual(parsed["root_candidate_compaction_seconds"], 0.015)
        self.assertEqual(parsed["lkh_provider_calls"], 1)
        self.assertEqual(parsed["lkh_provider_failures"], 0)
        self.assertEqual(parsed["lkh_provider_seconds"], 0.125)
        self.assertEqual(parsed["replacement_seconds"], 1.25)


if __name__ == "__main__":
    unittest.main()
