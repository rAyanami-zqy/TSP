#!/usr/bin/env python3
"""Interleaved baseline/compacted-epoch runs, with equal exact search assertions."""
import argparse
import hashlib
import json
from pathlib import Path
import subprocess
from run_cphkmst_explore import ROOT, TRAIN, HOLDOUT


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--baseline', type=Path, required=True)
    p.add_argument('--compact', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--repeats', type=int, default=3)
    args = p.parse_args()
    binaries = dict(baseline=args.baseline.resolve(), compact=args.compact.resolve())
    hashes = {k: hashlib.sha256(v.read_bytes()).hexdigest() for k, v in binaries.items()}
    fields = ['cost', 'ub', 'lb', 'nodes', 'expanded', 'iterations', 'fixed_zero',
              'fixed_one', 'active', 'tour']
    args.output.parent.mkdir(parents=True, exist_ok=True)
    # Fail rather than silently overwrite an earlier experiment.
    with args.output.open('x') as f:
        for rep in range(args.repeats):
            for name in TRAIN + HOLDOUT:
                for mode in ['baseline', 'clk-multi']:
                    pair = {}
                    for variant in list(binaries)[::1 if rep % 2 == 0 else -1]:
                        command = [str(binaries[variant]), mode,
                                   f'data/classic/tsplib/{name}.tsp', 'exact', '1']
                        proc = subprocess.run(command, cwd=ROOT, capture_output=True,
                                              text=True, timeout=15, check=True)
                        result = [json.loads(line[7:]) for line in proc.stdout.splitlines()
                                  if line.startswith('RESULT ')]
                        if len(result) != 1 or result[0]['status'] != 'ok':
                            raise RuntimeError(proc.stdout)
                        row = result[0]
                        row.update(variant=variant, binary_sha256=hashes[variant],
                                   repeat=rep + 1, command=command,
                                   group='train' if name in TRAIN else 'holdout')
                        pair[variant] = row
                        f.write(json.dumps(row) + '\n'); f.flush()
                    for field in fields:
                        if pair['baseline'][field] != pair['compact'][field]:
                            raise RuntimeError(f'{name}/{mode}: changed {field}')
                    print(rep + 1, name, mode,
                          {v: round(r['seconds'], 4) for v, r in pair.items()}, flush=True)


if __name__ == '__main__':
    main()
