#!/usr/bin/env python3
"""Build the persistent fork worker against an already-built LKH tree."""

from __future__ import annotations

import argparse
import os
import subprocess
from pathlib import Path


ROOT = Path(__file__).resolve().parents[2]


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--lkh-source", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    parser.add_argument("--cc", default=os.environ.get("CC", "cc"))
    args = parser.parse_args()

    source = args.lkh_source.resolve()
    src = source / "SRC"
    objects = sorted((src / "OBJ").glob("*.o"))
    objects = [path for path in objects if path.name != "LKHmain.o"]
    if not objects:
        parser.error("LKH object files are missing; run make in LKH first")
    args.output.parent.mkdir(parents=True, exist_ok=True)
    build = args.output.parent / (args.output.name + "-objects")
    build.mkdir(parents=True, exist_ok=True)
    entry = build / "lkh_entry.o"
    worker = build / "lkh_fork_worker.o"
    common = [
        "-O3", "-march=native", f"-I{src / 'INCLUDE'}",
        "-DTWO_LEVEL_TREE", "-fcommon",
    ]
    subprocess.run([
        args.cc, "-c", str(src / "LKHmain.c"), "-o", str(entry),
        "-Dmain=LKH_entry", *common,
    ], check=True)
    subprocess.run([
        args.cc, "-c",
        str(ROOT / "tools/experiments/lkh_fork_worker.c"),
        "-o", str(worker), "-O3",
    ], check=True)
    subprocess.run([
        args.cc, "-o", str(args.output), str(worker), str(entry),
        *(str(path) for path in objects), "-lm",
    ], check=True)
    print(args.output)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
