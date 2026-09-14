#!/usr/bin/env python3
"""Sequential, resumable experiments. JSONL is raw evidence, never a cache of guesses."""
import argparse
import hashlib
import json
from pathlib import Path
import statistics
import subprocess
import time

ROOT = Path(__file__).resolve().parents[2]
TRAIN = 'dantzig42 att48 bayg29 bays29 fri26 st70 eil76 rat99 eil101'.split()
HOLDOUT = 'brazil58 eil51 gr48 gr96 rd100 lin105'.split()
MODES = 'baseline knn12 knn20 kd8 quad2 delaunay union clk-zero clk-knn clk-geo clk-walk clk-multi'.split()


def main():
    p = argparse.ArgumentParser()
    p.add_argument('--executable', type=Path, required=True)
    p.add_argument('--output', type=Path, required=True)
    p.add_argument('--suite', choices=['screen', 'root50', 'confirm', 'hard', 'pricing'], required=True)
    p.add_argument('--modes', nargs='+')
    p.add_argument('--instances', nargs='+', help='override the suite instance paths')
    p.add_argument('--repeats', type=int, default=1)
    p.add_argument('--timeout', type=float, default=10)
    args = p.parse_args()
    exe = args.executable.resolve()
    fingerprint = hashlib.sha256(exe.read_bytes()).hexdigest()
    args.output.parent.mkdir(parents=True, exist_ok=True)
    rows = [json.loads(x) for x in args.output.read_text().splitlines()] if args.output.exists() else []
    if any(x['binary_sha256'] != fingerprint for x in rows):
        raise RuntimeError('output already belongs to a different executable')
    done = {(r['mode'],r['instance'],r['repeat'],r['suite']) for r in rows}
    instances = [f'data/classic/tsplib/{name}.tsp' for name in TRAIN+HOLDOUT]
    modes = args.modes or MODES
    if args.suite in ('root50','hard'):
        instances = (ROOT/'data/classic/batch-n200.txt').read_text().splitlines()
    if args.suite == 'hard':
        instances = [i for i in instances if Path(i).stem not in TRAIN+HOLDOUT]
    if args.suite == 'pricing':
        modes = ['pricing']
    if args.instances:
        instances = args.instances
    started = time.monotonic()
    with args.output.open('a') as f:
        for rep in range(args.repeats):
            # Reverse mode order on alternate repeats; no simultaneous solvers.
            for instance in instances:
                for mode in modes[::1 if rep%2==0 else -1]:
                    key = mode,instance,rep+1,args.suite
                    if key in done:
                        continue
                    row = dict(mode=mode,instance=instance,repeat=rep+1,suite=args.suite,
                               binary_sha256=fingerprint,timeout_seconds=args.timeout,
                               group='train' if Path(instance).stem in TRAIN else 'holdout')
                    command = [str(exe),mode,instance,'root' if args.suite=='root50' else 'exact',str(rep+1)]
                    row['command'] = command
                    t = time.monotonic()
                    try:
                        r = subprocess.run(command,cwd=ROOT,text=True,capture_output=True,timeout=args.timeout)
                        result = [json.loads(x[7:]) for x in r.stdout.splitlines() if x.startswith('RESULT ')]
                        if r.returncode or len(result)!=1:
                            raise RuntimeError(r.stderr + r.stdout[-3000:])
                        row.update(result[0])
                        row.setdefault('status','ok')
                    except subprocess.TimeoutExpired:
                        row['status']='timeout'
                    except Exception as e:
                        row.update(status='error',error=str(e))
                    row['process_seconds']=time.monotonic()-t
                    f.write(json.dumps(row,ensure_ascii=False)+'\n'); f.flush()
                    rows.append(row)
                    print(f"{args.suite} {rep+1} {mode:12} {Path(instance).stem:10} {row['status']:14} "
                          f"{row.get('seconds',row['process_seconds']):.4f}s ub={row.get('ub')} nodes={row.get('nodes')}",flush=True)
    # Within each instance, exact results must agree, including across modes/seeds.
    costs={}
    for row in rows:
        if row['status']=='ok' and row.get('kind')=='solve' and not row.get('root_only'):
            costs.setdefault(row['instance'],set()).add(row['cost'])
    if any(len(v)!=1 for v in costs.values()):
        raise RuntimeError(f'exact cost disagreement: {costs}')
    print(f'Completed in {time.monotonic()-started:.1f}s; {len(rows)} durable rows',flush=True)
    # Totals are meaningful only on the same fully completed instances/seeds.
    complete = {mode: {(r['instance'], r['repeat']) for r in rows
                       if r['mode'] == mode and r['status'] == 'ok' and 'seconds' in r}
                for mode in modes}
    common = set.intersection(*complete.values()) if complete else set()
    print('Timing totals below use only common completed instance/seed pairs:', len(common))
    for mode in modes:
        for group in ('train','holdout'):
            rr=[r for r in rows if r['mode']==mode and r['group']==group
                and (r['instance'], r['repeat']) in common]
            per_rep=[sum(r['seconds'] for r in rr if r['repeat']==rep+1) for rep in range(args.repeats)]
            if rr:
                print(mode,group,'median_total',statistics.median(per_rep),'nodes',sum(r['nodes'] for r in rr))


if __name__=='__main__':
    main()
