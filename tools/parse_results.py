#!/usr/bin/env python3
import re

with open('docs/strategy-comparison-results.md', 'r') as f:
    content = f.read()

# Split by instance sections (## N.)
sections = re.split(r'^## \d+\. ', content, flags=re.MULTILINE)

results = []

for section in sections[1:]:
    lines = section.strip().split('\n')

    # Instance name: first line has format 'path (n=X)'
    first_line = lines[0].strip()
    name_match = re.match(r'^`(.+?)`.+n=(\d+)', first_line)
    if not name_match:
        continue
    path = name_match.group(1)
    n = int(name_match.group(2))

    # Concorde info
    concorde_cost = None
    concorde_time = None
    for line in lines:
        m = re.match(r'\*\*Concorde optimal cost:\*\*\s*([\d]+)', line)
        if m:
            concorde_cost = int(m.group(1))
        m = re.match(r'\*\*Concorde time:\*\*\s*([\d]+(?:ms|s|m|h))', line)
        if m:
            concorde_time = m.group(1)

    # Find Smart section - collect lines between "Smart Branch Strategy" and "Simple Branch Strategy" or end of section
    smart_lines = []
    in_smart = False
    for line in lines:
        if 'Smart Branch Strategy' in line:
            in_smart = True
            continue
        elif 'Simple Branch Strategy' in line:
            in_smart = False
            continue
        if in_smart:
            smart_lines.append(line)

    # Parse detailed stats for tsp_bb and tsp_bb_old
    tsp_bb_data = {'cost': None, 'time': None, 'created': None, 'expanded': None, 'match': None}
    tsp_bb_old_data = {'cost': None, 'time': None, 'created': None, 'expanded': None, 'match': None}

    in_detail = False
    detail_lines = []
    for line in smart_lines:
        if 'Smart — Detailed Stats' in line or 'Smart - Detailed Stats' in line:
            in_detail = True
            continue
        if in_detail:
            if line.strip() == '' and len(detail_lines) > 3:
                break
            detail_lines.append(line.strip())

    # Parse detail rows - skip the table header and separator
    for line in detail_lines:
        if line.startswith('| tsp_bb |'):
            parts = [p.strip() for p in line.split('|')]
            # parts[0]='', parts[1]='tsp_bb', parts[2]=cost, parts[3]=time, parts[4]=created, parts[5]=expanded, parts[6]=pruned_bound, parts[7]=pruned_infeas, parts[8]=match
            if len(parts) >= 9:
                tsp_bb_data['cost'] = int(parts[2]) if parts[2].isdigit() else parts[2]
                tsp_bb_data['time'] = parts[3]
                tsp_bb_data['created'] = int(parts[4]) if parts[4].isdigit() else parts[4]
                tsp_bb_data['expanded'] = int(parts[5]) if parts[5].isdigit() else parts[5]
                tsp_bb_data['match'] = ':white_check_mark:' in parts[8]
        elif line.startswith('| tsp_bb_old |'):
            parts = [p.strip() for p in line.split('|')]
            if len(parts) >= 9:
                tsp_bb_old_data['cost'] = int(parts[2]) if parts[2].isdigit() else parts[2]
                tsp_bb_old_data['time'] = parts[3]
                tsp_bb_old_data['created'] = int(parts[4]) if parts[4].isdigit() else parts[4]
                tsp_bb_old_data['expanded'] = int(parts[5]) if parts[5].isdigit() else parts[5]
                tsp_bb_old_data['match'] = ':white_check_mark:' in parts[8]

    results.append({
        'path': path,
        'n': n,
        'concorde_cost': concorde_cost,
        'concorde_time': concorde_time,
        'tsp_bb': tsp_bb_data,
        'tsp_bb_old': tsp_bb_old_data
    })

print(f'Total instances found: {len(results)}')
print()

# Print compact table
print('| # | Instance | n | Concorde Cost | Concorde Time | tsp_bb Cost | tsp_bb Time | tsp_bb Created | tsp_bb Expanded | tsp_bb Match | tsp_bb_old Cost | tsp_bb_old Time | tsp_bb_old Created | tsp_bb_old Expanded | tsp_bb_old Match |')
print('|---|' + '---|' * 13)

for i, r in enumerate(results):
    bb = r['tsp_bb']
    old = r['tsp_bb_old']
    print(f'| {i+1} | {r["path"]} | {r["n"]} | {r["concorde_cost"]} | {r["concorde_time"]} | {bb["cost"]} | {bb["time"]} | {bb["created"]} | {bb["expanded"]} | {"Y" if bb["match"] else "N"} | {old["cost"]} | {old["time"]} | {old["created"]} | {old["expanded"]} | {"Y" if old["match"] else "N"} |')

print()
print('='*80)
print('SUMMARY STATISTICS')
print('='*80)

# Count how many matched
bb_matched = sum(1 for r in results if r['tsp_bb']['match'])
old_matched = sum(1 for r in results if r['tsp_bb_old']['match'])
total = len(results)

print(f'\nTotal instances: {total}')
print(f'tsp_bb SMART matched Concorde: {bb_matched}/{total}')
print(f'tsp_bb_old SMART matched Concorde: {old_matched}/{total}')

# Check if any missed
print(f'\n--- Instances where tsp_bb did NOT match ---')
for r in results:
    if not r['tsp_bb']['match']:
        print(f'  {r["path"]} (n={r["n"]}) - cost={r["tsp_bb"]["cost"]}, ref={r["concorde_cost"]}')

print(f'\n--- Instances where tsp_bb_old did NOT match ---')
for r in results:
    if not r['tsp_bb_old']['match']:
        print(f'  {r["path"]} (n={r["n"]}) - cost={r["tsp_bb_old"]["cost"]}, ref={r["concorde_cost"]}')
