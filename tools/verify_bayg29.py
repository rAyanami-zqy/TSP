#!/usr/bin/env python3
"""Verify bayg29 tour costs and investigate the 1610 vs 1618 discrepancy."""
import re, math, sys, os

def load_matrix(path):
    """Load TSPLIB format distance matrix."""
    with open(path) as f:
        text = f.read()
    # Find dimension
    dim = int(re.search(r'DIMENSION\s*:\s*(\d+)', text).group(1))
    # Find edge weight section (stop at EOF or DISPLAY_DATA_SECTION)
    sec = re.search(r'EDGE_WEIGHT_SECTION\s*\n(.*?)(?:EOF|DISPLAY_DATA_SECTION)', text, re.DOTALL)
    if not sec:
        sec = re.search(r'EDGE_WEIGHT_SECTION\s*\n(.*)', text, re.DOTALL)
    raw = sec.group(1).strip()
    nums = []
    for token in raw.split():
        try:
            nums.append(int(token))
        except ValueError:
            pass
    fmt = re.search(r'EDGE_WEIGHT_FORMAT\s*:\s*(\S+)', text)
    fmt = fmt.group(1) if fmt else 'FULL_MATRIX'

    if 'LOWER' in fmt.upper():
        # Lower diagonal row-wise
        dist = [[0]*dim for _ in range(dim)]
        idx = 0
        for i in range(dim):
            for j in range(i):
                dist[i][j] = dist[j][i] = nums[idx]
                idx += 1
        return dim, dist
    elif 'UPPER' in fmt.upper():
        dist = [[0]*dim for _ in range(dim)]
        idx = 0
        for i in range(dim):
            for j in range(i+1, dim):
                dist[i][j] = dist[j][i] = nums[idx]
                idx += 1
        return dim, dist
    else:
        # FULL_MATRIX
        dist = [[0]*dim for _ in range(dim)]
        idx = 0
        for i in range(dim):
            for j in range(dim):
                dist[i][j] = nums[idx]
                idx += 1
        return dim, dist

def tour_cost(tour, dist):
    """Compute cost of a tour (list of vertex indices)."""
    n = len(dist)
    cost = 0
    for i in range(len(tour)):
        u = tour[i]
        v = tour[(i+1) % len(tour)]
        cost += dist[u][v]
    return cost

def parse_tour(tour_str):
    """Parse tour like '0 -> 27 -> 5 -> ... -> 0' (last vertex repeats first)"""
    verts = [int(x.strip()) for x in tour_str.split('->')]
    if len(verts) > 1 and verts[0] == verts[-1]:
        verts = verts[:-1]  # remove duplicate closing vertex
    return verts

def edges_from_tour(tour):
    """Get set of edges from tour."""
    edges = set()
    for i in range(len(tour)):
        u = tour[i]
        v = tour[(i+1) % len(tour)]
        edges.add((min(u,v), max(u,v)))
    return edges

# Load bayg29
dim, dist = load_matrix('/home/wj/code/TSP/data/classic/tsplib/bayg29.tsp')
print(f"Loaded bayg29: dimension={dim}")

# Concorde tour
concorde_tour = parse_tour("0 -> 27 -> 5 -> 11 -> 8 -> 25 -> 2 -> 28 -> 4 -> 20 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 13 -> 16 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 7 -> 26 -> 15 -> 12 -> 23 -> 0")
concorde_cost = tour_cost(concorde_tour, dist)
print(f"Concorde tour cost: {concorde_cost}")

# B&B tour
bb_tour = parse_tour("7 -> 26 -> 15 -> 12 -> 23 -> 0 -> 27 -> 5 -> 11 -> 8 -> 4 -> 25 -> 2 -> 28 -> 20 -> 1 -> 19 -> 9 -> 3 -> 14 -> 17 -> 13 -> 16 -> 21 -> 10 -> 18 -> 24 -> 6 -> 22 -> 7")
bb_cost = tour_cost(bb_tour, dist)
print(f"B&B tour cost: {bb_cost}")

concorde_edges = edges_from_tour(concorde_tour)
bb_edges = edges_from_tour(bb_tour)

only_concorde = concorde_edges - bb_edges
only_bb = bb_edges - concorde_edges
common = concorde_edges & bb_edges

print(f"\nCommon edges: {len(common)}")
print(f"Edges only in Concorde: {sorted(only_concorde)}")
print(f"Edges only in B&B: {sorted(only_bb)}")

print("\nCost breakdown:")
for u, v in sorted(only_concorde):
    print(f"  Concorde only ({u},{v}): {dist[u][v]}")
for u, v in sorted(only_bb):
    print(f"  B&B only ({u},{v}): {dist[u][v]}")

concorde_only_cost = sum(dist[u][v] for u,v in only_concorde)
bb_only_cost = sum(dist[u][v] for u,v in only_bb)
print(f"\nConcorde-only edges total: {concorde_only_cost}")
print(f"B&B-only edges total: {bb_only_cost}")
print(f"Delta (Concorde - B&B): {concorde_only_cost - bb_only_cost}")

# Verify the tours are valid
concorde_degrees = [0]*dim
for u,v in concorde_edges:
    concorde_degrees[u] += 1
    concorde_degrees[v] += 1
print(f"\nConcorde tour degrees: {concorde_degrees}")
print(f"All degree 2: {all(d==2 for d in concorde_degrees)}")

bb_degrees = [0]*dim
for u,v in bb_edges:
    bb_degrees[u] += 1
    bb_degrees[v] += 1
print(f"B&B tour degrees: {bb_degrees}")
print(f"All degree 2: {all(d==2 for d in bb_degrees)}")

# Check critical edge weights
print("\nCritical edge weights:")
for (u,v) in [(8,25),(28,4),(4,20),(8,4),(4,25),(28,20)]:
    print(f"  dist[{u}][{v}] = {dist[u][v]}")
