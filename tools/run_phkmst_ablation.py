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

The default output retains wall time, result cost, created and expanded node
counts, root potential iterations, search-node potential-update triggers, and
search-node potential iterations.  A later program can perform cross-strategy
analysis from these small tables.

``tsp_bb`` has several archived versions whose command-line options are not
identical.  This runner probes each selected executable's ``--help`` output and
only adds runner-managed options that it supports.  Strategy options are kept
strict: an unsupported option that would change the experiment makes that
configuration incompatible instead of silently changing its meaning.  Extra
human-readable statistics emitted by newer solvers are accepted and ignored
unless they map to the stable ``result`` or ``branches`` columns below.
Solver paths and their individual arguments are declared together in
``SOLVER_CONFIGURATIONS``; the CLI only selects those complete configurations.
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
import sys
import tempfile
import time
from concurrent.futures import ThreadPoolExecutor, as_completed
from dataclasses import dataclass
from pathlib import Path
from typing import Any, Iterable

import compare_strategies as reference


PROJECT_ROOT = Path(__file__).resolve().parent.parent
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
    """一个可独立执行、缓存和汇总的实验配置。

    每项都必须同时声明 ``executable`` 和 ``solver_args``，使求解器版本与它
    自己的参数保持在同一位置。命令行只按名称选择配置，不再拼装这两部分。
    """

    name: str
    kind: str
    category: str
    executable: Path
    solver_args: tuple[str, ...]
    description: str


@dataclass(frozen=True)
class OutputStatistic:
    """把求解器的一条人类可读统计映射到稳定的 CSV 列。

    用户若要扩充输出，只需在 ``OUTPUT_STATISTICS`` 中新增一项。解析器会容忍
    求解器输出任意其他 ``标签: 值`` 行；只有这里声明的统计才写入结果表。

    ``kind`` 支持 ``int``、``float`` 和 ``text``。``summarize=True`` 会自动在
    summary.csv 中增加 ``total_<column>`` 与 ``median_<column>`` 两列。
    ``concorde_key`` 是 compare_strategies.py 对 Concorde 的规范化字段名；仅
    tsp_bb 提供的统计可留空。至少应保留一个 ``required=True`` 的结果字段，
    默认即 ``result``，用于判断求解是否成功。
    """

    column: str
    tspbb_labels: tuple[str, ...]
    kind: str
    summarize: bool = False
    concorde_key: str | None = None
    required: bool = False


@dataclass(frozen=True)
class SolverOption:
    """从求解器帮助文本中提取的一项长选项能力。

    ``takes_value`` 只用于把 ``--option value`` 视为同一个参数组；
    ``accepted_values`` 仅在帮助文本明确使用 ``a|b|c`` 列出枚举时才设置。
    数值、路径等自由取值不能仅凭 ``<n>``、``<path>`` 做进一步校验。
    """

    takes_value: bool
    accepted_values: frozenset[str] | None = None


@dataclass(frozen=True)
class SolverInterface:
    """一次能力探测的结果，供同一可执行文件的所有配置复用。"""

    options: dict[str, SolverOption] | None
    help_text: str
    warning: str | None = None


@dataclass(frozen=True)
class AdaptedArguments:
    """某配置针对一个具体求解器版本完成适配后的参数。"""

    arguments: tuple[str, ...]
    omitted: tuple[str, ...]
    incompatibilities: tuple[str, ...]


# ============================================================================
# 用户配置区 1：稳定输出列
# ----------------------------------------------------------------------------
# 每项声明一个 results.csv 列；summarize=True 时还会自动生成对应的 total 与
# median 汇总列。新增字段会进入缓存身份，使缺少这些统计的旧缓存不会被复用。
# ============================================================================
OUTPUT_STATISTICS: tuple[OutputStatistic, ...] = (
    OutputStatistic(
        "result",
        ("Optimal cost", "Optimal tour cost", "Optimum cost"),
        "float",
        concorde_key="cost",
        required=True,
    ),
    OutputStatistic(
        "branches",
        ("Nodes created", "B&B nodes", "BB nodes", "Branch-and-bound nodes"),
        "int",
        summarize=True,
        concorde_key="bbnodes",
    ),
    # 真正通过前置剪枝并进入 tour 检查或 BP 划分的搜索节点数。
    OutputStatistic(
        column="nodes_expanded",
        tspbb_labels=("Nodes expanded",),
        kind="int",
        summarize=True,
    ),
    # 下界已经不可能严格改善当前上界而被剪掉的节点数。
    OutputStatistic(
        column="pruned_by_bound",
        tspbb_labels=("Pruned by bound",),
        kind="int",
        summarize=True,
    ),
    # forced/forbidden 约束下无法形成合法 1-tree 而被剪掉的节点数。
    OutputStatistic(
        column="pruned_infeasible",
        tspbb_labels=("Pruned infeasible", "Pruned by infeasibility"),
        kind="int",
        summarize=True,
    ),
    # 根节点势优化总轮次。
    OutputStatistic(
        column="root_potential_iterations",
        tspbb_labels=("Root potential iterations",),
        kind="int",
        summarize=True,
    ),
    # 实际进入节点势更新判定的非根逻辑搜索节点数，是计算触发率的分母。
    OutputStatistic(
        column="search_node_potential_update_candidates",
        tspbb_labels=("Search-node potential update candidates",),
        kind="int",
        summarize=True,
    ),
    # 搜索节点触发势优化次数；第二个标签兼容较早版本的输出名称。
    OutputStatistic(
        column="search_node_potential_updates_triggered",
        tspbb_labels=(
            "Search-node potential updates triggered",
            "Potential updates attempted",
        ),
        kind="int",
        summarize=True,
    ),
    # 以下十项是新版求解器输出的互斥未触发原因。对每个候选节点只记录
    # 判定顺序中的第一个原因，因此它们与 triggered 之和应等于 candidates。
    OutputStatistic(
        column="search_node_potential_updates_skipped_strategy_none",
        tspbb_labels=("Potential updates skipped strategy none",),
        kind="int",
        summarize=True,
    ),
    OutputStatistic(
        column="search_node_potential_updates_skipped_update_depth_zero",
        tspbb_labels=("Potential updates skipped update depth zero",),
        kind="int",
        summarize=True,
    ),
    OutputStatistic(
        column="search_node_potential_updates_skipped_budget_exhausted",
        tspbb_labels=("Potential updates skipped budget exhausted",),
        kind="int",
        summarize=True,
    ),
    OutputStatistic(
        column="search_node_potential_updates_skipped_numerically_unsafe",
        tspbb_labels=("Potential updates skipped numerically unsafe",),
        kind="int",
        summarize=True,
    ),
    OutputStatistic(
        column="search_node_potential_updates_skipped_invalid_state",
        tspbb_labels=("Potential updates skipped invalid state",),
        kind="int",
        summarize=True,
    ),
    OutputStatistic(
        column="search_node_potential_updates_skipped_zero_violation",
        tspbb_labels=("Potential updates skipped zero violation",),
        kind="int",
        summarize=True,
    ),
    OutputStatistic(
        column="search_node_potential_updates_skipped_zero_iteration_limit",
        tspbb_labels=("Potential updates skipped zero iteration limit",),
        kind="int",
        summarize=True,
    ),
    OutputStatistic(
        column="search_node_potential_updates_skipped_depth_interval",
        tspbb_labels=("Potential updates skipped depth interval",),
        kind="int",
        summarize=True,
    ),
    OutputStatistic(
        column="search_node_potential_updates_skipped_gap_below_minimum",
        tspbb_labels=("Potential updates skipped gap below minimum",),
        kind="int",
        summarize=True,
    ),
    OutputStatistic(
        column="search_node_potential_updates_skipped_gap_above_maximum",
        tspbb_labels=("Potential updates skipped gap above maximum",),
        kind="int",
        summarize=True,
    ),
    # 搜索节点势优化总轮次；第二个标签兼容较早版本的输出名称。
    OutputStatistic(
        column="search_node_potential_iterations",
        tspbb_labels=(
            "Search-node potential iterations",
            "Potential update iterations",
        ),
        kind="int",
        summarize=True,
    ),
)

SUPPORTED_STATISTIC_KINDS = {"int", "float", "text"}
# 旧版缓存身份默认只对应这两个统计列。用户添加结果列时，新增配置会写入身份，
# 迫使求解器重新运行以获得新统计；保持默认配置时则继续复用现有 schema=2 缓存。
LEGACY_RESULT_STATISTIC_COLUMNS = ("result", "branches")


def solver_arguments(command_line: str) -> tuple[str, ...]:
    """把便于复制粘贴的命令行参数字符串转换为安全的参数数组。

    用户配置历史版本时通常已有一段可运行命令。使用本函数即可直接粘贴选项，
    不必手工给每个 token 加引号；``shlex.split`` 只负责解析，不会执行其中的
    shell 展开或命令替换。
    """

    return tuple(shlex.split(command_line))


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


# ============================================================================
# 用户配置区 2：求解器、参数与实验顺序
# ----------------------------------------------------------------------------
# 这里的顺序就是默认执行顺序。每个求解器独立配置 executable 与 solver_args；
# 不同版本支持的参数无需保持一致，兼容层会按各自 --help 分别校验。例如：
#
# Strategy(
#     name="legacy",
#     kind="tsp_bb",
#     category="version",
#     executable=PROJECT_ROOT / "build" / "tsp_bb_26_07_02",
#     solver_args=solver_arguments("--branch-strategy smart"),
#     description="2026-07-02 archived solver"),
# Strategy(
#     name="current",
#     kind="tsp_bb",
#     category="version",
#     executable=PROJECT_ROOT / "build" / "tsp_bb",
#     solver_args=solver_arguments(
#         "--hk-ascent polyak --hk-potential-update none"),
#     description="current solver"),
#
# 每个二进制摘要和自己的最终参数仍参与原有缓存指纹，因此不同版本的结果
# 绝不会误用同一缓存记录。若长期对比的就是上述两个版本，可直接把示例复制
# 到下方元组中，再用 ``--configs legacy current`` 选择它们。
# ============================================================================
SOLVER_CONFIGURATIONS: tuple[Strategy, ...] = (
    # Strategy(
    #     name="Concorde",
    #     kind="concorde",
    #     category="reference",
    #     executable=PROJECT_ROOT / "concorde" / "TSP" / "concorde",
    #     solver_args=(),
    #     description="Concorde exact solver with a fixed seed"),
    # Strategy(
    #     name="D0",
    #     kind="tsp_bb",
    #     category="core",
    #     executable=PROJECT_ROOT / "build" / "tsp_bb",
    #     solver_args=solver_arguments("""
    #         --hk-ascent polyak
    #         --hk-node-ascent polyak
    #         --branch-edge-order weight
    #         --hk-potential-update subtree-adaptive
    #         --hk-update-depth 1
    #         --hk-update-gap-ratio 0.02
    #         --hk-update-min-gap-ratio 0.0
    #         --hk-update-iterations 16
    #         --hk-update-budget 5000
    #     """),
    #     description="persistent adaptive potential update",
    # ),
    # Strategy(
    #     name="P16",
    #     kind="tsp_bb",
    #     category="core",
    #     executable=PROJECT_ROOT / "build" / "tsp_bb",
    #     solver_args=solver_arguments("""
    #         --hk-ascent polyak
    #         --hk-node-ascent polyak
    #         --branch-edge-order weight
    #         --hk-potential-update subtree-adaptive
    #         --hk-update-depth 1
    #         --hk-update-gap-ratio 0.02
    #         --hk-update-min-gap-ratio 0.0
    #         --hk-update-iterations 16
    #         --hk-update-budget 0
    #     """),
    #     description="hk-update-budget 0",
    # ),
    # Strategy(
    #     name="P8",
    #     kind="tsp_bb",
    #     category="core",
    #     executable=PROJECT_ROOT / "build" / "tsp_bb",
    #     solver_args=solver_arguments("""
    #         --hk-ascent polyak
    #         --hk-node-ascent polyak
    #         --branch-edge-order weight
    #         --hk-potential-update subtree-adaptive
    #         --hk-update-depth 1
    #         --hk-update-gap-ratio 0.02
    #         --hk-update-min-gap-ratio 0.0
    #         --hk-update-iterations 8
    #         --hk-update-budget 0
    #     """),
    #     description="hk-update-budget 100000",
    # ),
    # Strategy(
    #     name="P32",
    #     kind="tsp_bb",
    #     category="core",
    #     executable=PROJECT_ROOT / "build" / "tsp_bb",
    #     solver_args=solver_arguments("""
    #         --hk-ascent polyak
    #         --hk-node-ascent polyak
    #         --branch-edge-order weight
    #         --hk-potential-update subtree-adaptive
    #         --hk-update-depth 1
    #         --hk-update-gap-ratio 0.02
    #         --hk-update-min-gap-ratio 0.0
    #         --hk-update-iterations 32
    #         --hk-update-budget 0
    #     """),
    #     description="hk-update-iterations 32",
    # ),
    # Strategy(
    #     name="P64",
    #     kind="tsp_bb",
    #     category="core",
    #     executable=PROJECT_ROOT / "build" / "tsp_bb",
    #     solver_args=solver_arguments("""
    #         --hk-ascent polyak
    #         --hk-node-ascent polyak
    #         --branch-edge-order weight
    #         --hk-potential-update subtree-adaptive
    #         --hk-update-depth 1
    #         --hk-update-gap-ratio 0.02
    #         --hk-update-min-gap-ratio 0.0
    #         --hk-update-iterations 64
    #         --hk-update-budget 0
    #     """),
    #     description="hk-update-iterations 64",
    # ),
    Strategy(
        name="P32r5h1",
        kind="tsp_bb",
        category="core",
        executable=PROJECT_ROOT / "build" / "tsp_bb",
        solver_args=solver_arguments("""
            --hk-ascent polyak
            --hk-node-ascent polyak
            --branch-edge-order weight
            --hk-potential-update subtree-adaptive
            --hk-update-depth 1
            --hk-update-gap-ratio 0.05
            --hk-update-min-gap-ratio 0.0
            --hk-update-iterations 32
            --hk-update-budget 0
        """),
        description="hk-update-iterations 32 h1 r5",
    ),
    Strategy(
        name="P32r5h2",
        kind="tsp_bb",
        category="core",
        executable=PROJECT_ROOT / "build" / "tsp_bb",
        solver_args=solver_arguments("""
            --hk-ascent polyak
            --hk-node-ascent polyak
            --branch-edge-order weight
            --hk-potential-update subtree-adaptive
            --hk-update-depth 2
            --hk-update-gap-ratio 0.05
            --hk-update-min-gap-ratio 0.0
            --hk-update-iterations 32
            --hk-update-budget 0
        """),
        description="hk-update-iterations 32 h2 r5",
    ),
    Strategy(
        name="P128",
        kind="tsp_bb",
        category="core",
        executable=PROJECT_ROOT / "build" / "tsp_bb",
        solver_args=solver_arguments("""
            --hk-ascent polyak
            --hk-node-ascent polyak
            --branch-edge-order weight
            --hk-potential-update subtree-adaptive
            --hk-update-depth 1
            --hk-update-gap-ratio 0.02
            --hk-update-min-gap-ratio 0.0
            --hk-update-iterations 128
            --hk-update-budget 0
        """),
        description="hk-update-iterations 128",
    ),
    Strategy(
        name="PH332",
        kind="tsp_bb",
        category="core",
        executable=PROJECT_ROOT / "build" / "tsp_bb",
        solver_args=solver_arguments("""
            --hk-ascent polyak
            --hk-node-ascent polyak
            --branch-edge-order weight
            --hk-potential-update subtree-adaptive
            --hk-update-depth 3
            --hk-update-gap-ratio 0.02
            --hk-update-min-gap-ratio 0.0
            --hk-update-iterations 32
            --hk-update-budget 0
        """),
        description="depth 3 hk-update-iterations 32",
    ),
    Strategy(
        name="PH432",
        kind="tsp_bb",
        category="core",
        executable=PROJECT_ROOT / "build" / "tsp_bb",
        solver_args=solver_arguments("""
            --hk-ascent polyak
            --hk-node-ascent polyak
            --branch-edge-order weight
            --hk-potential-update subtree-adaptive
            --hk-update-depth 4
            --hk-update-gap-ratio 0.02
            --hk-update-min-gap-ratio 0.0
            --hk-update-iterations 32
            --hk-update-budget 0
        """),
        description="depth 4 hk-update-iterations 32",
    ),
    Strategy(
        name="PR1532",
        kind="tsp_bb",
        category="core",
        executable=PROJECT_ROOT / "build" / "tsp_bb",
        solver_args=solver_arguments("""
            --hk-ascent polyak
            --hk-node-ascent polyak
            --branch-edge-order weight
            --hk-potential-update subtree-adaptive
            --hk-update-depth 2
            --hk-update-gap-ratio 0.015
            --hk-update-min-gap-ratio 0.0
            --hk-update-iterations 32
            --hk-update-budget 0
        """),
        description="hk-update-gap-ratio 0.015 depth 2 iterations 32",
    ),
    Strategy(
        name="PAA",
        kind="tsp_bb",
        category="core",
        executable=PROJECT_ROOT / "build" / "tsp_bb",
        solver_args=solver_arguments("""
            --hk-ascent polyak
            --hk-node-ascent polyak
            --branch-edge-order root-alpha-global-desc 
            --hk-potential-update subtree-adaptive
            --hk-update-depth 1
            --hk-update-gap-ratio 0.02
            --hk-update-min-gap-ratio 0.0
            --hk-update-iterations 32
            --hk-update-budget 0
        """),
        description="root-alpha-global-desc iterations 32",
    ),
    Strategy(
        name="hybrid32",
        kind="tsp_bb",
        category="core",
        executable=PROJECT_ROOT / "build" / "tsp_bb",
        solver_args=solver_arguments("""
            --hk-ascent hybrid
            --hk-node-ascent polyak
            --branch-edge-order root-alpha-global-desc 
            --hk-potential-update subtree-adaptive
            --hk-update-depth 1
            --hk-update-gap-ratio 0.02
            --hk-update-min-gap-ratio 0.0
            --hk-update-iterations 32
            --hk-update-budget 0
        """),
        description="hybrid iterations 32",
    )
    # Strategy(
    #     name="H2",
    #     kind="tsp_bb",
    #     category="core",
    #     executable=PROJECT_ROOT / "build" / "tsp_bb",
    #     solver_args=solver_arguments("""
    #         --hk-ascent polyak
    #         --hk-node-ascent polyak
    #         --branch-edge-order weight
    #         --hk-potential-update subtree-adaptive
    #         --hk-update-depth 2
    #         --hk-update-gap-ratio 0.02
    #         --hk-update-min-gap-ratio 0.0
    #         --hk-update-iterations 16
    #         --hk-update-budget 0
    #     """),
    #     description="hk-update-depth 2",
    # ),
    # Strategy(
    #     name="H3",
    #     kind="tsp_bb",
    #     category="core",
    #     executable=PROJECT_ROOT / "build" / "tsp_bb",
    #     solver_args=solver_arguments("""
    #         --hk-ascent polyak
    #         --hk-node-ascent polyak
    #         --branch-edge-order weight
    #         --hk-potential-update subtree-adaptive
    #         --hk-update-depth 3
    #         --hk-update-gap-ratio 0.02
    #         --hk-update-min-gap-ratio 0.0
    #         --hk-update-iterations 16
    #         --hk-update-budget 0
    #     """),
    #     description="hk-update-depth 3",
    # ),
    # Strategy(
    #     name="H4",
    #     kind="tsp_bb",
    #     category="core",
    #     executable=PROJECT_ROOT / "build" / "tsp_bb",
    #     solver_args=solver_arguments("""
    #         --hk-ascent polyak
    #         --hk-node-ascent polyak
    #         --branch-edge-order weight
    #         --hk-potential-update subtree-adaptive
    #         --hk-update-depth 4
    #         --hk-update-gap-ratio 0.02
    #         --hk-update-min-gap-ratio 0.0
    #         --hk-update-iterations 16
    #         --hk-update-budget 0
    #     """),
    #     description="hk-update-depth 4",
    # ),
    # Strategy(
    #     name="H232",
    #     kind="tsp_bb",
    #     category="core",
    #     executable=PROJECT_ROOT / "build" / "tsp_bb",
    #     solver_args=solver_arguments("""
    #         --hk-ascent polyak
    #         --hk-node-ascent polyak
    #         --branch-edge-order weight
    #         --hk-potential-update subtree-adaptive
    #         --hk-update-depth 2
    #         --hk-update-gap-ratio 0.02
    #         --hk-update-min-gap-ratio 0.0
    #         --hk-update-iterations 32
    #         --hk-update-budget 0
    #     """),
    #     description="hk-update-depth 2 --hk-update-iterations 32",
    # ),
    # Strategy(
    #     name="helsgaun32",
    #     kind="tsp_bb",
    #     category="core",
    #     executable=PROJECT_ROOT / "build" / "tsp_bb",
    #     solver_args=solver_arguments("""
    #         --hk-ascent polyak
    #         --hk-node-ascent helsgaun
    #         --branch-edge-order weight
    #         --hk-potential-update subtree-adaptive
    #         --hk-update-depth 1
    #         --hk-update-gap-ratio 0.02
    #         --hk-update-min-gap-ratio 0.0
    #         --hk-update-iterations 32
    #         --hk-update-budget 0
    #     """),
    #     description="helsgaun --hk-update-iterations 32",
    # ),
    # Strategy(
    #     name="helsgaun64",
    #     kind="tsp_bb",
    #     category="core",
    #     executable=PROJECT_ROOT / "build" / "tsp_bb",
    #     solver_args=solver_arguments("""
    #         --hk-ascent polyak
    #         --hk-node-ascent helsgaun
    #         --branch-edge-order weight
    #         --hk-potential-update subtree-adaptive
    #         --hk-update-depth 1
    #         --hk-update-gap-ratio 0.02
    #         --hk-update-min-gap-ratio 0.0
    #         --hk-update-iterations 64
    #         --hk-update-budget 0
    #     """),
    #     description="helsgaun --hk-update-iterations 64",
    # ),
    # Strategy(
    #     name="helsgaun128",
    #     kind="tsp_bb",
    #     category="core",
    #     executable=PROJECT_ROOT / "build" / "tsp_bb",
    #     solver_args=solver_arguments("""
    #         --hk-ascent polyak
    #         --hk-node-ascent helsgaun
    #         --branch-edge-order weight
    #         --hk-potential-update subtree-adaptive
    #         --hk-update-depth 1
    #         --hk-update-gap-ratio 0.02
    #         --hk-update-min-gap-ratio 0.0
    #         --hk-update-iterations 128
    #         --hk-update-budget 0
    #     """),
    #     description="helsgaun --hk-update-iterations 128",
    # ),
    # Strategy(
    #     name="hybrid",
    #     kind="tsp_bb",
    #     category="core",
    #     executable=PROJECT_ROOT / "build" / "tsp_bb",
    #     solver_args=solver_arguments("""
    #         --hk-ascent hybrid
    #         --hk-node-ascent polyak
    #         --branch-edge-order weight
    #         --hk-potential-update subtree-adaptive
    #         --hk-update-depth 1
    #         --hk-update-gap-ratio 0.02
    #         --hk-update-min-gap-ratio 0.0
    #         --hk-update-iterations 16
    #         --hk-update-budget 0
    #     """),
    #     description="hk-ascent hybrid --hk-update-iterations 16",
    # ),
    # Strategy(
    #     name="HP1",
    #     kind="tsp_bb",
    #     category="core",
    #     executable=PROJECT_ROOT / "build" / "tsp_bb",
    #     solver_args=solver_arguments("""
    #         --hk-ascent hybrid
    #         --hk-node-ascent polyak
    #         --branch-edge-order weight
    #         --hk-potential-update subtree-adaptive
    #         --hk-update-depth 1
    #         --hk-update-gap-ratio 0.01
    #         --hk-update-min-gap-ratio 0.0
    #         --hk-update-iterations 16
    #         --hk-update-budget 0
    #     """),
    #     description=" hk-update-gap-ratio 0.01",
    # ),
    # Strategy(
    #     name="HP132",
    #     kind="tsp_bb",
    #     category="core",
    #     executable=PROJECT_ROOT / "build" / "tsp_bb",
    #     solver_args=solver_arguments("""
    #         --hk-ascent hybrid
    #         --hk-node-ascent polyak
    #         --branch-edge-order weight
    #         --hk-potential-update subtree-adaptive
    #         --hk-update-depth 1
    #         --hk-update-gap-ratio 0.01
    #         --hk-update-min-gap-ratio 0.0
    #         --hk-update-iterations 32
    #         --hk-update-budget 0
    #     """),
    #     description=" hk-update-gap-ratio 0.01 hk-update-iterations 32",
    # ),
    # Strategy(
    #     name="HP25",
    #     kind="tsp_bb",
    #     category="core",
    #     executable=PROJECT_ROOT / "build" / "tsp_bb",
    #     solver_args=solver_arguments("""
    #         --hk-ascent hybrid
    #         --hk-node-ascent polyak
    #         --branch-edge-order weight
    #         --hk-potential-update subtree-adaptive
    #         --hk-update-depth 1
    #         --hk-update-gap-ratio 0.05
    #         --hk-update-min-gap-ratio 0.02
    #         --hk-update-iterations 16
    #         --hk-update-budget 0
    #     """),
    #     description=" hk-update-gap-ratio 0.02-0.05",
    # ),
    # Strategy(
    #     name="HP2532",
    #     kind="tsp_bb",
    #     category="core",
    #     executable=PROJECT_ROOT / "build" / "tsp_bb",
    #     solver_args=solver_arguments("""
    #         --hk-ascent hybrid
    #         --hk-node-ascent polyak
    #         --branch-edge-order weight
    #         --hk-potential-update subtree-adaptive
    #         --hk-update-depth 1
    #         --hk-update-gap-ratio 0.05
    #         --hk-update-min-gap-ratio 0.02
    #         --hk-update-iterations 32
    #         --hk-update-budget 0
    #     """),
    #     description=" hk-update-gap-ratio 0.02-0.05",
    # )
)

CONFIGURATION_BY_NAME = {
    strategy.name: strategy for strategy in SOLVER_CONFIGURATIONS}


def validate_solver_configurations() -> None:
    """校验代码配置区的结构，给编辑配置的用户提供就近错误信息。"""

    names = [strategy.name for strategy in SOLVER_CONFIGURATIONS]
    duplicate_names = sorted({name for name in names if names.count(name) > 1})
    if duplicate_names:
        raise ValueError(
            "duplicate solver configuration names: "
            + ", ".join(duplicate_names))
    for strategy in SOLVER_CONFIGURATIONS:
        if not strategy.name.strip():
            raise ValueError("solver configuration name cannot be empty")
        if strategy.kind not in {"tsp_bb", "concorde"}:
            raise ValueError(
                f"configuration {strategy.name!r} has unsupported kind "
                f"{strategy.kind!r}")
        if not isinstance(strategy.executable, Path):
            raise ValueError(
                f"configuration {strategy.name!r} executable must be a Path")
        if not isinstance(strategy.solver_args, tuple) or not all(
            isinstance(argument, str) for argument in strategy.solver_args
        ):
            raise ValueError(
                f"configuration {strategy.name!r} solver_args must be a "
                "tuple of strings; use solver_arguments(...) for convenience")

RESULT_BASE_FIELDS = (
    "run_id",
    "strategy",
    "repeat",
    "instance",
    "status",
    # 运行器从启动子进程到进程结束测得的真实经过时间，单位为秒。
    "wall_seconds",
)
RESULT_FIELDS = RESULT_BASE_FIELDS + tuple(
    statistic.column for statistic in OUTPUT_STATISTICS)

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

SUMMARY_BASE_FIELDS = (
    "run_id",
    "strategy",
    "status",
    "rows",
    "successful",
    "timeouts",
    "errors",
    "total_wall_seconds",
    "median_wall_seconds",
)
SUMMARY_FIELDS = SUMMARY_BASE_FIELDS + tuple(
    field
    for statistic in OUTPUT_STATISTICS
    if statistic.summarize
    for field in (
        f"total_{statistic.column}", f"median_{statistic.column}")
)


def output_statistics_for(strategy: Strategy) -> tuple[OutputStatistic, ...]:
    """返回当前求解器类型真正需要写入结果表的统计列。

    Concorde 仅承担参考求解器职责。它只保留声明了 ``concorde_key`` 的
    ``result`` 与 ``branches``；PHKMST 专属的节点、势更新和剪枝统计不会以
    大量空列混入 Concorde 表。tsp_bb 仍使用用户配置区 1 的完整稳定列。
    """

    if strategy.kind == "concorde":
        return tuple(
            statistic
            for statistic in OUTPUT_STATISTICS
            if statistic.concorde_key is not None)
    return OUTPUT_STATISTICS


def result_fields_for(strategy: Strategy) -> tuple[str, ...]:
    """按求解器类型生成逐实例 CSV 表头；固定标识与 wall time 始终保留。"""

    return RESULT_BASE_FIELDS + tuple(
        statistic.column for statistic in output_statistics_for(strategy))


def summary_fields_for(strategy: Strategy) -> tuple[str, ...]:
    """按求解器类型生成汇总 CSV 表头。"""

    return SUMMARY_BASE_FIELDS + tuple(
        field
        for statistic in output_statistics_for(strategy)
        if statistic.summarize
        for field in (
            f"total_{statistic.column}", f"median_{statistic.column}")
    )

# 这些是本脚本已知会消费一个值的 tsp_bb 选项。能力探测也会从帮助文本识别
# ``<value>``，此集合则保证旧版本未列出某选项时仍能把“选项 + 值”作为整体
# 报错，而不会把值错当成位置参数继续传给求解器。
KNOWN_VALUE_OPTIONS = {
    "--branch-strategy",
    "--exact-max-n",
    "--debug-interval",
    "--hk-ascent",
    "--hk-node-ascent",
    "--branch-edge-order",
    "--hk-potential-update",
    "--hk-update-depth",
    "--hk-update-gap-ratio",
    "--hk-update-iterations",
    "--hk-update-budget",
}

# 有些能力后来才被拆成独立开关。旧版本没有这些开关时，只有显式请求下列
# 历史固定行为才可安全省略；任何非默认值都必须判为不兼容，防止把 P2 静默
# 跑成 P1、把 alpha 分支实验静默跑成 weight。
SAFE_OMISSIONS = {
    ("--hk-node-ascent", "polyak"),
    ("--branch-edge-order", "weight"),
    ("--hk-potential-update", "none"),
}


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--configs", "--config", nargs="+", choices=tuple(CONFIGURATION_BY_NAME),
        help="code-defined configurations in execution order; defaults to all",
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
                        help="list every code-defined configuration and exit")
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
    if args.configs:
        args.strategies = [CONFIGURATION_BY_NAME[name] for name in args.configs]
    else:
        args.strategies = list(SOLVER_CONFIGURATIONS)
    names = [strategy.name for strategy in args.strategies]
    if len(names) != len(set(names)):
        parser.error("configuration names must be unique")
    return args


def list_solver_configurations() -> None:
    for strategy in SOLVER_CONFIGURATIONS:
        args = shlex.join(strategy.solver_args) or "(managed Concorde arguments)"
        print(
            f"{strategy.name:<9} [{strategy.kind}/{strategy.category}] "
            f"{strategy.description}\n"
            f"          {args}\n"
            f"          solver: {strategy.executable}")


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


def parse_solver_interface(help_text: str) -> SolverInterface:
    """从帮助文本中提取长选项、是否带值以及显式枚举值。

    这里只依赖所有已知版本都具备的 ``--help`` 人类可读接口，不绑定版本号或
    编译日期。帮助中的枚举可能跨行（例如较长的 branch-edge-order 列表），
    因此按“本选项起点到下一选项起点”的区间解析，而不是逐行解析。
    """

    option_matches = list(re.finditer(
        r"(?m)^[ \t]*(--[A-Za-z0-9][A-Za-z0-9-]*)(?=[ \t=]|$)",
        help_text,
    ))
    options: dict[str, SolverOption] = {}
    for index, match in enumerate(option_matches):
        name = match.group(1)
        end = (
            option_matches[index + 1].start()
            if index + 1 < len(option_matches)
            else len(help_text)
        )
        section = help_text[match.end():end]
        placeholder = re.search(r"<([^>]*)>", section, flags=re.DOTALL)
        accepted_values: frozenset[str] | None = None
        if placeholder is not None and "|" in placeholder.group(1):
            # 去除折行产生的空白；CLI 枚举值本身不含空白。
            compact = re.sub(r"\s+", "", placeholder.group(1))
            accepted_values = frozenset(
                value for value in compact.split("|") if value)
        options[name] = SolverOption(
            takes_value=placeholder is not None,
            accepted_values=accepted_values,
        )
    return SolverInterface(options=options or None, help_text=help_text)


def probe_solver_interface(executable: Path) -> SolverInterface:
    """探测一个求解器版本的 CLI；同一二进制在 main 中只调用一次。

    极老版本可能只支持 ``-h``，所以 ``--help`` 未得到可解析选项时会再试一次。
    如果程序能启动但帮助格式不可识别，则回退到旧运行器行为（原样传参），并
    返回告警；如果程序根本无法在当前平台启动，则直接报告错误，避免为每个
    实例重复产生相同的 ``exec format`` 失败记录。
    """

    collected_text: list[str] = []
    launch_errors: list[str] = []
    completed_probe = False
    for help_option in ("--help", "-h"):
        try:
            completed = subprocess.run(
                [str(executable), help_option],
                cwd=PROJECT_ROOT,
                capture_output=True,
                text=True,
                timeout=5,
            )
            completed_probe = True
        except (OSError, subprocess.SubprocessError) as error:
            launch_errors.append(f"{help_option}: {error}")
            continue
        text = "\n".join(
            part for part in (completed.stdout, completed.stderr) if part)
        collected_text.append(text)
        interface = parse_solver_interface(text)
        if interface.options:
            return interface

    if not completed_probe:
        details = "; ".join(launch_errors) or "unknown launch error"
        raise RuntimeError(
            f"cannot execute solver {executable} to probe its interface: {details}")

    help_text = "\n".join(collected_text)
    return SolverInterface(
        options=None,
        help_text=help_text,
        warning=(
            f"could not recognize options from {executable} help output; "
            "passing arguments without compatibility filtering"),
    )


def _option_name(token: str) -> str | None:
    """返回 ``--name=value`` 或 ``--name`` 的规范化长选项名。"""

    if not token.startswith("--") or token == "--":
        return None
    return token.split("=", 1)[0]


def _has_option(arguments: Iterable[str], option: str) -> bool:
    return any(_option_name(token) == option for token in arguments)


def adapt_strategy_arguments(
    strategy: Strategy,
    interface: SolverInterface,
) -> AdaptedArguments:
    """按求解器能力验证策略参数，并只省略可证明等价的旧版固定默认值。

    策略参数描述实验语义，不能像 debug 参数一样随意删除。比如旧版本不支持
    ``--hk-node-ascent helsgaun`` 时，删除它会把 P2 变成 Polyak；这里会把配置
    标记为不兼容。只有 ``SAFE_OMISSIONS`` 中明确登记的历史固定行为允许省略。
    """

    if interface.options is None:
        return AdaptedArguments(strategy.solver_args, (), ())

    adapted: list[str] = []
    omitted: list[str] = []
    incompatibilities: list[str] = []
    tokens = list(strategy.solver_args)
    index = 0
    while index < len(tokens):
        token = tokens[index]
        name = _option_name(token)
        if name is None:
            # 短选项和位置参数保留原样；不同历史程序可能有自己的短选项，
            # 仅凭 tsp_bb 的长选项帮助无法安全判断它们。
            adapted.append(token)
            index += 1
            continue

        inline_value = token.split("=", 1)[1] if "=" in token else None
        capability = interface.options.get(name)
        takes_value = (
            inline_value is not None
            or name in KNOWN_VALUE_OPTIONS
            or (capability is not None and capability.takes_value)
        )
        value = inline_value
        group = [token]
        if inline_value is None and takes_value:
            if index + 1 >= len(tokens):
                incompatibilities.append(f"{name} is missing its value")
                index += 1
                continue
            value = tokens[index + 1]
            group.append(value)
            index += 1

        if capability is None:
            if value is not None and (name, value) in SAFE_OMISSIONS:
                omitted.append(f"{name}={value}")
            else:
                rendered = f"{name}={value}" if value is not None else name
                incompatibilities.append(
                    f"solver does not support strategy option {rendered}")
        elif (
            capability.accepted_values is not None
            and value is not None
            and value not in capability.accepted_values
        ):
            allowed = "|".join(sorted(capability.accepted_values))
            incompatibilities.append(
                f"solver rejects {name}={value}; accepted values: {allowed}")
        else:
            adapted.extend(group)
        index += 1

    # 早期 BP 版本公开了唯一的 --branch-strategy=smart，并要求显式传入；
    # 新版本将该策略内建且删除了参数。能力驱动地补齐它，无需维护版本号列表。
    if (
        "--branch-strategy" in interface.options
        and not _has_option(adapted, "--branch-strategy")
    ):
        adapted[0:0] = ["--branch-strategy", "smart"]

    return AdaptedArguments(
        tuple(adapted), tuple(omitted), tuple(incompatibilities))


def supports_option(interface: SolverInterface, option: str) -> bool:
    """帮助不可解析时返回 True，以保持旧版运行器的原样传参回退行为。"""

    return interface.options is None or option in interface.options


def effective_args(
    args: argparse.Namespace,
    strategy: Strategy,
    interface: SolverInterface | None = None,
) -> AdaptedArguments:
    """构造最终命令参数；Concorde 路径保持原逻辑不变。"""

    if strategy.kind == "concorde":
        return AdaptedArguments(
            (*strategy.solver_args, "-s", str(args.concorde_seed)), (), ())

    if interface is None:
        # 保留函数可单独复用的便利性；main 总会传入真实探测结果。
        interface = SolverInterface(options=None, help_text="")
    strategy_result = adapt_strategy_arguments(strategy, interface)
    result = list(strategy_result.arguments)
    omitted = list(strategy_result.omitted)

    # exact-max-n 是保护稠密矩阵展开的 runner 级选项，而不是实验变量。旧版本
    # 没有该保护时可以安全不传，不应因此让整个策略失败。
    if supports_option(interface, "--exact-max-n"):
        result.extend(["--exact-max-n", str(args.exact_max_n)])
    else:
        omitted.append("--exact-max-n (runner-managed)")

    # debug 只影响进度输出。某些版本有 --debug 但没有可调 interval，此时仍
    # 启用 debug，并仅省略 interval；反过来绝不单独传 interval。
    if args.debug:
        if supports_option(interface, "--debug"):
            result.append("--debug")
            if supports_option(interface, "--debug-interval"):
                result.extend(["--debug-interval", str(args.debug_interval)])
            else:
                omitted.append("--debug-interval (runner-managed)")
        else:
            omitted.append("--debug and --debug-interval (runner-managed)")

    return AdaptedArguments(
        tuple(result), tuple(omitted), strategy_result.incompatibilities)


def strategy_identity(
    args: argparse.Namespace,
    strategy: Strategy,
    executable: Path,
    executable_sha256: str,
    invocation_args: list[str],
    instances: list[Path],
) -> dict[str, Any]:
    identity: dict[str, Any] = {
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
    strategy_statistics = output_statistics_for(strategy)
    statistic_columns = tuple(
        statistic.column for statistic in strategy_statistics)
    if statistic_columns != LEGACY_RESULT_STATISTIC_COLUMNS:
        # 旧缓存只保存 result/branches，没有原始 stdout。若用户新加列却沿用旧
        # 指纹，新列只能永远为空；把声明写入身份可精确失效这些缓存。
        identity["output_statistics"] = [
            {
                "column": statistic.column,
                "tspbb_labels": list(statistic.tspbb_labels),
                "kind": statistic.kind,
                "concorde_key": statistic.concorde_key,
            }
            for statistic in strategy_statistics
        ]
    return identity


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


def normalize_statistic_label(label: str) -> str:
    """统一大小写、连字符和空白，让轻微文案差异不影响统计匹配。"""

    return " ".join(re.sub(r"[_-]+", " ", label).casefold().split())


def validate_output_configuration() -> None:
    """校验用户配置区，避免跑完长实验后才发现列定义有歧义。"""

    columns = [statistic.column for statistic in OUTPUT_STATISTICS]
    duplicates = sorted({column for column in columns if columns.count(column) > 1})
    if duplicates:
        raise ValueError(
            "duplicate OUTPUT_STATISTICS columns: " + ", ".join(duplicates))
    reserved = sorted(set(columns) & set(RESULT_BASE_FIELDS))
    if reserved:
        raise ValueError(
            "OUTPUT_STATISTICS columns conflict with fixed result columns: "
            + ", ".join(reserved))
    invalid_kinds = sorted({
        statistic.kind
        for statistic in OUTPUT_STATISTICS
        if statistic.kind not in SUPPORTED_STATISTIC_KINDS
    })
    if invalid_kinds:
        raise ValueError(
            "unsupported OUTPUT_STATISTICS kinds: " + ", ".join(invalid_kinds))
    text_summaries = [
        statistic.column
        for statistic in OUTPUT_STATISTICS
        if statistic.kind == "text" and statistic.summarize
    ]
    if text_summaries:
        raise ValueError(
            "text OUTPUT_STATISTICS cannot be summarized: "
            + ", ".join(text_summaries))
    if not any(statistic.required for statistic in OUTPUT_STATISTICS):
        raise ValueError(
            "OUTPUT_STATISTICS must contain at least one required result field")

    # 同一标签若映射到两列，解析结果将依赖配置顺序；这里明确禁止这种歧义。
    label_owners: dict[str, str] = {}
    for statistic in OUTPUT_STATISTICS:
        for label in statistic.tspbb_labels:
            normalized = normalize_statistic_label(label)
            previous = label_owners.setdefault(normalized, statistic.column)
            if previous != statistic.column:
                raise ValueError(
                    f"tsp_bb statistic label {label!r} is shared by "
                    f"{previous!r} and {statistic.column!r}")


def parse_statistic_value(raw_value: str, kind: str) -> Any | None:
    """把标签后的文本转换为配置指定类型，非法值返回 None 而不中断批次。"""

    value = raw_value.strip()
    if kind == "text":
        return value
    # 支持整数、小数和科学计数法；逗号仅按数字分组符处理。尾部单位或说明文字
    # 不影响开头的数值，例如 ``1.25 seconds`` 仍可解析为 1.25。
    numeric_text = value.replace(",", "")
    match = re.match(
        r"[-+]?(?:\d+(?:\.\d*)?|\.\d+)(?:[eE][-+]?\d+)?",
        numeric_text,
    )
    if match is None:
        return None
    try:
        number = float(match.group(0))
    except ValueError:
        return None
    if not math.isfinite(number):
        return None
    if kind == "float":
        return number
    if not number.is_integer():
        return None
    return int(number)


def parse_tspbb_statistics(stdout: str, stderr: str = "") -> dict[str, Any]:
    """解析所有声明过的 tsp_bb 统计，忽略新增或未知标签。

    求解器版本可以任意增加 ``Label: value`` 行，解析器先收集完整标签表，再按
    ``OUTPUT_STATISTICS`` 投影到稳定列。这样新增内部统计不会改变默认 CSV，
    用户若想输出它，只需在顶部配置区登记标签和类型。stdout/stderr 一并读取，
    兼容把结果写到不同流的历史版本。
    """

    labeled_values: dict[str, str] = {}
    for line in "\n".join((stdout, stderr)).splitlines():
        match = re.match(r"^\s*([^:=]+?)\s*[:=]\s*(.*?)\s*$", line)
        if match is None:
            continue
        label = normalize_statistic_label(match.group(1))
        labeled_values[label] = match.group(2)

    parsed: dict[str, Any] = {
        statistic.column: None for statistic in OUTPUT_STATISTICS}
    for statistic in OUTPUT_STATISTICS:
        for label in statistic.tspbb_labels:
            raw_value = labeled_values.get(normalize_statistic_label(label))
            if raw_value is None:
                continue
            parsed[statistic.column] = parse_statistic_value(
                raw_value, statistic.kind)
            break
    return parsed


def missing_required_statistics(
    row: dict[str, Any], strategy: Strategy,
) -> list[str]:
    """只检查当前求解器类型承诺提供的必需字段。"""

    return [
        statistic.column
        for statistic in output_statistics_for(strategy)
        if statistic.required and row.get(statistic.column) is None
    ]


def empty_result(
    run_id: str, strategy: Strategy, repeat: int, instance: Path,
) -> dict[str, Any]:
    row = {
        "run_id": run_id,
        "strategy": strategy.name,
        "repeat": repeat,
        "instance": str(instance),
        "status": "error",
        "wall_seconds": None,
    }
    row.update({statistic.column: None for statistic in OUTPUT_STATISTICS})
    return row


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
        row.update(parse_tspbb_statistics(completed.stdout, completed.stderr))
        if completed.returncode != 0:
            details = completed.stderr.strip() or completed.stdout.strip()
            return row, f"exit {completed.returncode}: {details[-500:]}"
        missing = missing_required_statistics(row, strategy)
        if missing:
            return row, (
                "tsp_bb produced no parseable required statistic(s): "
                + ", ".join(missing))
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
        for statistic in OUTPUT_STATISTICS:
            if statistic.concorde_key is not None:
                row[statistic.column] = stats.get(statistic.concorde_key)
        if completed.returncode != 0:
            details = completed.stderr.strip() or completed.stdout.strip()
            return row, f"exit {completed.returncode}: {details[-500:]}"
        missing = missing_required_statistics(row, strategy)
        if not stats.get("feasible") or missing:
            detail = ", ".join(missing) if missing else "feasible result"
            return row, f"Concorde produced no parseable {detail}"
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
    summary = {
        "run_id": run_id,
        "strategy": strategy.name,
        "status": status,
        "rows": len(rows),
        "successful": len(successful),
        "timeouts": sum(row["status"] == "timeout" for row in rows),
        "errors": sum(row["status"] == "error" for row in rows),
        "total_wall_seconds": sum(walls) if walls else 0,
        "median_wall_seconds": statistics.median(walls) if walls else "",
    }
    # 汇总列由 OUTPUT_STATISTICS 驱动。新增数值统计时只需设置
    # summarize=True，此处会自动沿用现有的 total/median 约定。
    for statistic in output_statistics_for(strategy):
        if not statistic.summarize:
            continue
        values = numeric_values(
            successful, statistic.column, integer=statistic.kind == "int")
        summary[f"total_{statistic.column}"] = sum(values) if values else 0
        summary[f"median_{statistic.column}"] = (
            statistics.median(values) if values else "")
    return summary


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
    write_csv(
        output_dir / "results.csv", result_fields_for(strategy), rows)
    write_csv(
        output_dir / "summary.csv", summary_fields_for(strategy), [summary])
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
        message = (
            f"  median wall={float(summary['median_wall_seconds']):.6f}s")
        # 默认配置保留旧控制台文本；用户移除 branches 汇总时也不会 KeyError。
        if "median_branches" in summary:
            branches = summary["median_branches"]
            branch_text = f"{float(branches):.0f}" if branches != "" else "-"
            message += f" branches={branch_text}"
        print(message)


def validate_executable(path: Path, label: str) -> Path:
    resolved = path.resolve()
    if not resolved.is_file() or not os.access(resolved, os.X_OK):
        raise FileNotFoundError(f"executable {label} not found: {resolved}")
    return resolved


def main() -> int:
    args = parse_args()
    try:
        validate_solver_configurations()
        validate_output_configuration()
    except ValueError as error:
        print(f"Invalid code configuration: {error}", file=sys.stderr)
        return 2
    if args.list_configs:
        list_solver_configurations()
        return 0

    instances = select_instances(args)
    if args.list_instances:
        for instance in instances:
            print(instance)
        return 0

    concorde_strategy = next(
        (strategy for strategy in args.strategies
         if strategy.kind == "concorde"),
        None,
    )
    concorde_executable = None
    if concorde_strategy is not None:
        concorde_executable = validate_executable(
            concorde_strategy.executable,
            "Concorde",
        )

    # 每个配置直接读取自己的 executable，不存在会把多个版本意外覆盖成同一
    # 路径的全局开关。最终仍按配置名保存映射，允许同一批次混用多个版本。
    tsp_executables: dict[str, Path] = {}
    for strategy in args.strategies:
        if strategy.kind != "tsp_bb":
            continue
        tsp_executables[strategy.name] = validate_executable(
            strategy.executable, f"tsp_bb for {strategy.name}")

    executable_digests: dict[Path, str] = {}
    solver_interfaces: dict[Path, SolverInterface] = {}
    for executable in dict.fromkeys(tsp_executables.values()):
        try:
            interface = probe_solver_interface(executable)
        except RuntimeError as error:
            print(f"Solver compatibility probe failed: {error}", file=sys.stderr)
            return 2
        solver_interfaces[executable] = interface
        if interface.warning:
            print(f"Warning: {interface.warning}", file=sys.stderr)

    output_root = args.output_root.resolve()
    output_root.mkdir(parents=True, exist_ok=True)
    cache_path = args.cache.resolve() if args.cache else output_root / "cache.json"

    phases: list[dict[str, Any]] = []
    for strategy in args.strategies:
        executable = (
            concorde_executable if strategy.kind == "concorde"
            else tsp_executables[strategy.name])
        assert executable is not None
        if executable not in executable_digests:
            executable_digests[executable] = binary_digest(executable)
        executable_sha256 = executable_digests[executable]
        interface = (
            None if strategy.kind == "concorde"
            else solver_interfaces[executable])
        adapted = effective_args(args, strategy, interface)
        if adapted.incompatibilities:
            details = "; ".join(adapted.incompatibilities)
            print(
                f"Warning: skipping incompatible configuration "
                f"{strategy.name}: {details}",
                file=sys.stderr,
            )
            continue
        if adapted.omitted:
            print(
                f"Compatibility {strategy.name}: omitted "
                + ", ".join(adapted.omitted),
                file=sys.stderr,
            )
        invocation_args = list(adapted.arguments)
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

    if not phases:
        print(
            "No compatible configurations remain. Adjust --configs or edit "
            "executable/solver_args in the code configuration area.",
            file=sys.stderr,
        )
        return 2

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
                    f"result={row.get('result')} branches={row.get('branches')}"
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
