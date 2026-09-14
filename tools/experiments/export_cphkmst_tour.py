#!/usr/bin/env python3
"""Export a validated research RESULT tour to the main solver's incumbent format."""
import argparse
import json
from pathlib import Path


def main():
    p = argparse.ArgumentParser()
    p.add_argument('results', type=Path)
    p.add_argument('--instance', required=True, help='instance filename stem, e.g. eil101')
    p.add_argument('--mode', default='clk-multi')
    p.add_argument('--seed', type=int, default=1)
    p.add_argument('--output', type=Path, required=True)
    args = p.parse_args()
    rows = [json.loads(line) for line in args.results.read_text().splitlines()]
    rows = [r for r in rows if r.get('status') == 'ok' and r.get('kind') == 'solve'
            and Path(r['instance']).stem == args.instance and r['mode'] == args.mode
            and r['seed'] == args.seed]
    if not rows:
        raise RuntimeError('no successful matching tour')
    row = min(rows, key=lambda r: r['cost'])
    if sorted(row['tour']) != list(range(row['n'])):
        raise RuntimeError('invalid permutation in results')
    # The main solver rechecks every edge and recomputes cost for its own input.
    with args.output.open('x') as f:
        f.write(str(row['n']) + '\n' + ' '.join(map(str, row['tour'])) + '\n')
    print(args.output)


if __name__ == '__main__':
    main()
