#!/usr/bin/env python3
"""Build native optional Concorde libraries and the isolated research executable."""
import argparse
from pathlib import Path
import os
import shutil
import subprocess

ROOT=Path(__file__).resolve().parents[2]

def run(command,cwd,env=None):
    subprocess.run(command,cwd=cwd,env=env,check=True)

def main():
    p=argparse.ArgumentParser()
    p.add_argument('--build',type=Path,required=True)
    p.add_argument('--baseline',action='store_true',help='disable epoch compaction')
    p.add_argument('--verify',action='store_true')
    p.add_argument('--spatial',action='store_true',help='build the isolated KD query benchmark')
    args=p.parse_args()
    build=args.build.resolve(); build.mkdir(parents=True,exist_ok=True)
    ccbuild=build/'concorde-build'; ccbuild.mkdir(exist_ok=True)
    if not (ccbuild/'UTIL/util.a').exists():
        env=os.environ.copy()
        env['CFLAGS']='-O3 -Wno-error=implicit-function-declaration -Wno-error=implicit-int'
        # The 2003 config.sub accepts names containing "local" unchanged. Tests
        # still run with the native compiler; this is not cross compilation.
        run([str(ROOT/'concorde/configure'),'--host=native-local',f'--srcdir={ROOT / "concorde"}'],ccbuild,env)
        # Source checkout may contain a generated Linux config.h. Ensure quoted
        # includes in machdefs.h find the freshly configured native config.
        for header in (ROOT/'concorde/INCLUDE').glob('*.h'):
            if header.name!='config.h': shutil.copy2(header,ccbuild/'INCLUDE'/header.name)
    for name in ('UTIL','KDTREE','FMATCH','LINKERN','EDGEGEN'):
        if not (ccbuild/name/(name.lower()+'.a')).exists():
            run(['make','-B','-j4',name.lower()+'.a'],ccbuild/name)
    command=[os.environ.get('CXX','c++'),'-std=c++17','-O3','-Wall','-Wextra',
             '-I'+str(ROOT/'include'),'-I'+str(ccbuild/'INCLUDE')]
    if args.spatial:
        command += [str(ROOT/'tools/experiments/cphkmst_spatial.cpp'),
                    str(ccbuild/'KDTREE/kdtree.a'), str(ccbuild/'UTIL/util.a'),
                    '-lm', '-o', str(build/'spatial')]
        run(command,ROOT)
        print(build/'spatial')
        return
    if args.baseline: command+=['-DTSP_DISABLE_EPOCH_COMPACTION=1']
    if args.verify: command+=['-DTSP_VERIFY_INCREMENTAL_STATE=1']
    command += [str(ROOT/'tools/experiments/cphkmst_explore.cpp'),str(ROOT/'src/TspSolver.cpp')]
    for name in ('EDGEGEN','LINKERN','KDTREE','FMATCH','UTIL'):
        command.append(str(ccbuild/name/(name.lower()+'.a')))
    output=build/('explore-'+('baseline' if args.baseline else 'compact')+('-verify' if args.verify else ''))
    command+=['-lm','-o',str(output)]
    run(command,ROOT)
    print(output)

if __name__=='__main__': main()
