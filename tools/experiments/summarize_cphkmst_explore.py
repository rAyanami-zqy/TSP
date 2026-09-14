#!/usr/bin/env python3
"""Recompute report figures from raw JSONL; fail on exact-result disagreement."""
import argparse
from collections import Counter
import json
from pathlib import Path
from statistics import median


def main():
    p = argparse.ArgumentParser()
    p.add_argument('directory', type=Path)
    p.add_argument('--output', type=Path)
    args = p.parse_args()
    def read(name):
        return [json.loads(line) for line in (args.directory / name).read_text().splitlines()]
    summary = {}
    files = ['screen-corrected.jsonl', 'confirm-baseline.jsonl', 'confirm-compact.jsonl',
             'epochs-interleaved.jsonl', 'hard35.jsonl', 'verify15.jsonl', 'verify-extra4.jsonl',
             'clean-repro-check.jsonl']
    exact_costs = {}
    for name in files:
        for row in read(name):
            if row['status'] == 'ok' and row.get('kind') == 'solve' and not row['root_only']:
                exact_costs.setdefault(row['instance'], set()).add(row['cost'])
    if any(len(costs) != 1 for costs in exact_costs.values()):
        raise RuntimeError('exact optimum disagreement')
    summary['exact_instances_agree'] = len(exact_costs)
    summary['exact_costs'] = {Path(k).stem: next(iter(v)) for k, v in exact_costs.items()}
    fields = ['cost', 'ub', 'lb', 'nodes', 'expanded', 'iterations', 'fixed_zero',
              'fixed_one', 'active', 'tour']
    epochs = read('epochs-interleaved.jsonl')
    key = lambda r: (r['mode'], r['instance'], r['repeat'])
    pairs = {variant: {key(r): r for r in epochs if r['variant'] == variant}
             for variant in ['baseline', 'compact']}
    if pairs['baseline'].keys() != pairs['compact'].keys():
        raise RuntimeError('unmatched epoch runs')
    for k, a in pairs['baseline'].items():
        b = pairs['compact'][k]
        if any(a[field] != b[field] for field in fields):
            raise RuntimeError(f'epoch changed exact search: {k}')
    summary['interleaved_equal_pairs'] = len(pairs['baseline'])
    summary['epoch_median_group_seconds'] = {}
    for mode in ['baseline', 'clk-multi']:
        for group in ['train', 'holdout']:
            times = {variant: median(sum(r['seconds'] for r in epochs
                                         if r['variant'] == variant and r['mode'] == mode
                                         and r['group'] == group and r['repeat'] == rep)
                                     for rep in [1, 2, 3]) for variant in pairs}
            times['reduction_percent'] = 100 * (1 - times['compact'] / times['baseline'])
            summary['epoch_median_group_seconds'][mode + '/' + group] = times
    summary['three_seed_group_seconds'] = {}
    for binary in ['baseline', 'compact']:
        rows = read('confirm-' + binary + '.jsonl')
        for mode in ['baseline', 'clk-knn', 'clk-walk', 'clk-multi']:
            for group in ['train', 'holdout']:
                rr = [r for r in rows if r['mode'] == mode and r['group'] == group]
                summary['three_seed_group_seconds']['/'.join([binary, mode, group])] = {
                    'median': median(sum(r['seconds'] for r in rr if r['repeat'] == rep)
                                     for rep in [1, 2, 3]),
                    'nodes_per_seed': [sum(r['nodes'] for r in rr if r['repeat'] == rep)
                                       for rep in [1, 2, 3]]}
    rows = read('root50.jsonl')
    base = {r['instance']: r for r in rows if r['mode'] == 'baseline'}
    summary['root50'] = {}
    for mode in dict.fromkeys(r['mode'] for r in rows):
        rr = [r for r in rows if r['mode'] == mode and r['status'] == 'ok']
        summary['root50'][mode] = dict(
            applicable=len(rr), improved=sum(r['ub'] < base[r['instance']]['ub'] for r in rr),
            worsened=sum(r['ub'] > base[r['instance']]['ub'] for r in rr),
            seconds=sum(r['seconds'] for r in rr),
            matched_baseline_seconds=sum(base[r['instance']]['seconds'] for r in rr))
    rows = read('pricing50.jsonl')
    if len(rows) != 50 or any(r['status'] != 'ok' for r in rows):
        raise RuntimeError('pricing audit incomplete')
    summary['pricing'] = {k: sum(r[k] for r in rows) for k in [
        'trials', 'rounds', 'added', 'fallbacks', 'uncertified_overestimates', 'forced_tests',
        'full_seconds', 'pricing_seconds']}
    rows = read('hard35.jsonl')
    summary['hard35'] = {}
    success = {mode: {r['instance']: r for r in rows if r['mode'] == mode and r['status'] == 'ok'}
               for mode in ['baseline', 'clk-multi']}
    common = success['baseline'].keys() & success['clk-multi'].keys()
    for mode in success:
        rr = [r for r in rows if r['mode'] == mode]
        summary['hard35'][mode] = dict(statuses=dict(Counter(r['status'] for r in rr)),
            common_completed=len(common),
            common_seconds=sum(success[mode][k]['seconds'] for k in common),
            capped_process_seconds=sum(min(r['process_seconds'], r['timeout_seconds']) for r in rr))
    summary['hard35']['additional_completed'] = sorted(
        Path(k).stem for k in success['clk-multi'].keys() - success['baseline'].keys())
    summary['spatial'] = read('spatial.jsonl')
    rendered = json.dumps(summary, ensure_ascii=False, indent=2) + '\n'
    if args.output:
        args.output.write_text(rendered)
    else:
        print(rendered, end='')


if __name__ == '__main__':
    main()
