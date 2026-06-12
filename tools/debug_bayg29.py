#!/usr/bin/env python3
"""
Simulate the critical candidate filtering for bayg29 to check if
buildBranchCandidates incorrectly filters edges needed for the optimal tour.
"""
import sys
sys.path.insert(0, 'tools')
# We'll use the matrix directly

import re, math

def load_matrix(path):
    with open(path) as f:
        text = f.read()
    dim = int(re.search(r'DIMENSION\s*:\s*(\d+)', text).group(1))
    sec = re.search(r'EDGE_WEIGHT_SECTION\s*\n(.*?)(?:EOF|DISPLAY_DATA_SECTION)', text, re.DOTALL)
    raw = sec.group(1).strip()
    nums = []
    for token in raw.split():
        try:
            nums.append(int(token))
        except ValueError:
            pass

    fmt_match = re.search(r'EDGE_WEIGHT_FORMAT\s*:\s*(\S+)', text)
    fmt = fmt_match.group(1).upper() if fmt_match else 'FULL_MATRIX'

    dist = [[0]*dim for _ in range(dim)]
    if 'LOWER' in fmt:
        idx = 0
        for i in range(dim):
            for j in range(i):
                dist[i][j] = dist[j][i] = nums[idx]
                idx += 1
    elif 'UPPER' in fmt:
        idx = 0
        for i in range(dim):
            for j in range(i+1, dim):
                dist[i][j] = dist[j][i] = nums[idx]
                idx += 1
    else:
        idx = 0
        for i in range(dim):
            for j in range(dim):
                dist[i][j] = nums[idx]
                idx += 1
    return dim, dist

class DSU:
    def __init__(self, n):
        self.parent = list(range(n))
        self.rank = [0]*n
    def find(self, x):
        if self.parent[x] != x:
            self.parent[x] = self.find(self.parent[x])
        return self.parent[x]
    def unite(self, a, b):
        ra, rb = self.find(a), self.find(b)
        if ra == rb:
            return False
        if self.rank[ra] < self.rank[rb]:
            ra, rb = rb, ra
        self.parent[rb] = ra
        if self.rank[ra] == self.rank[rb]:
            self.rank[ra] += 1
        return True

def simulate_build_candidates(n, dist, forced_edges, forbidden_edges):
    """Simulate buildBranchCandidates filtering."""
    forced_degree = [0]*n
    dsu = DSU(n)

    # Validate forced edges
    for u, v in forced_edges:
        if not math.isfinite(dist[u][v]):
            return None, "infinite forced edge"
        if not dsu.unite(u, v):
            return None, "forced cycle"
        forced_degree[u] += 1
        forced_degree[v] += 1
        if forced_degree[u] > 2 or forced_degree[v] > 2:
            return None, "forced degree > 2"

    # Count component vertices
    comp_vertices = [0]*n
    for v in range(n):
        comp_vertices[dsu.find(v)] += 1

    # Build candidate set (all possible edges)
    all_edges = []
    for u in range(n):
        for v in range(u+1, n):
            if math.isfinite(dist[u][v]):
                all_edges.append((u, v, dist[u][v]))

    # Filter candidates
    kept = []
    removed = []
    forced_set = set(forced_edges)
    forbidden_set = set(forbidden_edges)

    for u, v, w in all_edges:
        remove = False
        if (u, v) in forced_set or (v, u) in forced_set:
            remove = True
        elif (u, v) in forbidden_set or (v, u) in forbidden_set:
            remove = True
        elif forced_degree[u] >= 2 or forced_degree[v] >= 2:
            remove = True
        elif u > 0 and v > 0:
            ru, rv = dsu.find(u), dsu.find(v)
            if ru == rv and comp_vertices[ru] < n:
                remove = True

        if remove:
            removed.append((u, v, w))
        else:
            kept.append((u, v, w))

    # Check available degree
    avail_deg = forced_degree[:]
    for u, v, w in kept:
        avail_deg[u] += 1
        avail_deg[v] += 1
    for v in range(n):
        if avail_deg[v] < 2:
            return None, f"vertex {v} has available degree {avail_deg[v]} < 2"

    return (kept, removed, forced_degree, comp_vertices, dsu), None

def compute_1tree_bound(n, dist, forced_edges, candidate_edges):
    """Compute 1-tree lower bound."""
    dsu = DSU(n)
    mst_cost = 0
    mst_edges = 0

    # Add forced edges (vertices 1..n-1 only)
    for u, v in forced_edges:
        if u > 0 and v > 0:
            dsu.unite(u, v)
            mst_cost += dist[u][v]
            mst_edges += 1

    # Sort candidates by weight
    candidates = sorted(candidate_edges, key=lambda e: e[2])

    # Kruskal: add non-vertex-0 edges until n-2 total
    for u, v, w in candidates:
        if mst_edges == n - 2:
            break
        if u == 0 or v == 0:
            continue
        if dsu.unite(u, v):
            mst_cost += w
            mst_edges += 1

    if mst_edges != n - 2:
        return None  # infeasible

    # Add 2 cheapest edges to vertex 0
    root_edges = []
    for u, v in forced_edges:
        if u == 0 or v == 0:
            root_edges.append((u, v, dist[u][v]))

    root_candidates = [(u, v, w) for u, v, w in candidates if u == 0 or v == 0]
    root_candidates.sort(key=lambda e: e[2])

    for u, v, w in root_candidates:
        if len(root_edges) == 2:
            break
        root_edges.append((u, v, w))

    if len(root_edges) != 2:
        return None  # infeasible

    total_cost = mst_cost + sum(w for _, _, w in root_edges)
    return total_cost

# Load bayg29
n, dist = load_matrix('/home/wj/code/TSP/data/classic/tsplib/bayg29.tsp')
print(f"Loaded bayg29: n={n}")

# Define edges from both tours
concorde_edges = {(0,27),(27,5),(5,11),(11,8),(8,25),(25,2),(2,28),(28,4),(4,20),(20,1),
                  (1,19),(19,9),(9,3),(3,14),(14,17),(17,13),(13,16),(16,21),(21,10),
                  (10,18),(18,24),(24,6),(6,22),(22,7),(7,26),(26,15),(15,12),(12,23),(23,0)}
bb_edges = {(7,26),(26,15),(15,12),(12,23),(23,0),(0,27),(27,5),(5,11),(11,8),(8,4),
            (4,25),(25,2),(2,28),(28,20),(20,1),(1,19),(19,9),(9,3),(3,14),(14,17),
            (17,13),(13,16),(16,21),(21,10),(10,18),(18,24),(24,6),(6,22),(22,7)}

# Normalize to (min, max) tuples
concorde_edges_norm = {(min(u,v), max(u,v)) for u,v in concorde_edges}
bb_edges_norm = {(min(u,v), max(u,v)) for u,v in bb_edges}

print(f"\nConcorde edges: {len(concorde_edges_norm)}")
print(f"B&B edges: {len(bb_edges_norm)}")
print(f"Common: {len(concorde_edges_norm & bb_edges_norm)}")
print(f"Only Concorde: {sorted(concorde_edges_norm - bb_edges_norm)}")
print(f"Only B&B: {sorted(bb_edges_norm - concorde_edges_norm)}")

# Key question: is the Concorde tour reachable through B&B search?
# Let's check if buildBranchCandidates ever incorrectly filters Concorde edges.

# 1. Root node: no forced edges
print("\n=== Root node (no forced edges) ===")
result, err = simulate_build_candidates(n, dist, set(), set())
if result:
    kept, removed, fd, cv, dsu = result
    print(f"  Kept: {len(kept)} candidates")
    # Check if all Concorde edges are in kept
    concorde_kept = [e for e in kept if (e[0],e[1]) in concorde_edges_norm]
    print(f"  Concorde edges in candidates: {len(concorde_kept)}/{len(concorde_edges_norm)}")
    missing = concorde_edges_norm - {(e[0],e[1]) for e in kept}
    if missing:
        print(f"  MISSING Concorde edges: {sorted(missing)}")

    # Compute root 1-tree bound
    bound = compute_1tree_bound(n, dist, set(), kept)
    print(f"  Root 1-tree bound: {bound}")
else:
    print(f"  ERROR: {err}")

# 2. Check what happens when we force edges from both tours
print("\n=== Forcing common edges ===")
common = concorde_edges_norm & bb_edges_norm
# Force all common edges
forced = set(common)
result, err = simulate_build_candidates(n, dist, forced, set())
if result:
    kept, removed, fd, cv, dsu = result
    print(f"  Forced: {len(forced)} edges")
    print(f"  Kept: {len(kept)} candidates, Removed: {len(removed)}")
    concorde_only = concorde_edges_norm - bb_edges_norm
    bb_only = bb_edges_norm - concorde_edges_norm
    concorde_kept = [e for e in kept if (e[0],e[1]) in concorde_only]
    bb_kept = [e for e in kept if (e[0],e[1]) in bb_only]
    print(f"  Concorde-only edges still available: {len(concorde_kept)}/{len(concorde_only)}")
    for e in concorde_only:
        status = "AVAILABLE" if any((c[0],c[1])==e for c in kept) else "REMOVED"
        print(f"    {e} dist={dist[e[0]][e[1]]}: {status}")
    print(f"  B&B-only edges still available: {len(bb_kept)}/{len(bb_only)}")
    for e in bb_only:
        status = "AVAILABLE" if any((c[0],c[1])==e for c in kept) else "REMOVED"
        print(f"    {e} dist={dist[e[0]][e[1]]}: {status}")

    bound = compute_1tree_bound(n, dist, forced, kept)
    print(f"  1-tree bound with common edges forced: {bound}")
else:
    print(f"  ERROR: {err}")

# 3. Simulate searching the "B&B path": force B&B-only edges
print("\n=== Forcing all B&B tour edges ===")
forced = set(bb_edges_norm)
result, err = simulate_build_candidates(n, dist, forced, set())
if result:
    kept, removed, fd, cv, dsu = result
    print(f"  Forced: {len(forced)} edges")
    print(f"  Kept: {len(kept)} candidates")
    bound = compute_1tree_bound(n, dist, forced, kept)
    print(f"  1-tree bound (should be 1618): {bound}")
    print(f"  isTour: {bound == 1618}")
else:
    print(f"  ERROR: {err}")

# 4. Simulate searching the "Concorde path": force Concorde-only edges
print("\n=== Forcing all Concorde tour edges ===")
forced = set(concorde_edges_norm)
result, err = simulate_build_candidates(n, dist, forced, set())
if result:
    kept, removed, fd, cv, dsu = result
    print(f"  Forced: {len(forced)} edges")
    print(f"  Kept: {len(kept)} candidates")
    bound = compute_1tree_bound(n, dist, forced, kept)
    print(f"  1-tree bound (should be 1610): {bound}")
    print(f"  isTour: {bound == 1610}")
else:
    print(f"  ERROR: {err}")

# 5. KEY TEST: Common edges forced, try forbidding the B&B-only edges
print("\n=== Common + forbid ALL B&B-only edges ===")
forced = set(common)
forbidden = {(4,8), (4,25), (20,28)}
result, err = simulate_build_candidates(n, dist, forced, forbidden)
if result:
    kept, removed, fd, cv, dsu = result
    bound = compute_1tree_bound(n, dist, forced, kept)
    print(f"  1-tree bound: {bound}")
    print(f"  < 1618? {'YES' if bound and bound < 1618 else 'NO (would prune!)' if bound and bound >= 1618 else 'INFEASIBLE'}")
    concorde_only_avail = [(e, dist[e[0]][e[1]]) for e in (concorde_edges_norm - bb_edges_norm) if any((c[0],c[1])==e for c in kept)]
    print(f"  Concorde-only edges available: {concorde_only_avail}")
else:
    print(f"  ERROR: {err}")

# 6. Deep dive: common + forbid(4,8) + force(8,25)
print("\n=== Common + forbid(4,8) + force(8,25) ===")
forced = set(common) | {(8,25)}
forbidden = {(4,8)}
result, err = simulate_build_candidates(n, dist, forced, forbidden)
if result:
    kept, removed, fd, cv, dsu = result
    print(f"  Forced degree: {sorted([(v,d) for v,d in enumerate(fd) if d>0])}")
    bound = compute_1tree_bound(n, dist, forced, kept)
    print(f"  1-tree bound: {bound}")
    # Check remaining Concorde edges
    for e in [(4,28), (4,20)]:
        avail = any((c[0],c[1])==e for c in kept)
        print(f"  Concorde edge {e} (dist={dist[e[0]][e[1]]}): {'AVAILABLE' if avail else 'REMOVED'}")
    for e in [(4,25), (20,28)]:
        avail = any((c[0],c[1])==e for c in kept)
        print(f"  B&B edge {e} (dist={dist[e[0]][e[1]]}): {'AVAILABLE' if avail else 'REMOVED'}")
else:
    print(f"  ERROR: {err}")

# 7. Deep dive: common + forbid(4,8) + force(8,25) + force(28,4)
print("\n=== Common + forbid(4,8) + force(8,25) + force(28,4) ===")
forced = set(common) | {(8,25), (4,28)}
forbidden = {(4,8)}
result, err = simulate_build_candidates(n, dist, forced, forbidden)
if result:
    kept, removed, fd, cv, dsu = result
    print(f"  Forced degree: {sorted([(v,d) for v,d in enumerate(fd) if d>0])}")
    bound = compute_1tree_bound(n, dist, forced, kept)
    print(f"  1-tree bound: {bound}")
    for e in [(4,20)]:
        avail = any((c[0],c[1])==e for c in kept)
        print(f"  Concorde edge {e} (dist={dist[e[0]][e[1]]}): {'AVAILABLE' if avail else 'REMOVED'}")
    for e in [(4,25), (20,28)]:
        avail = any((c[0],c[1])==e for c in kept)
        print(f"  B&B edge {e} (dist={dist[e[0]][e[1]]}): {'AVAILABLE' if avail else 'REMOVED'}")
else:
    print(f"  ERROR: {err}")

# 8. The crucial test: common + force(8,4) (first step of B&B path)
# Then what's the 1-tree bound for the forbid-(8,4) branch?
print("\n=== Root + force(8,4) [B&B first step] ===")
forced = {(8,4)}
result, err = simulate_build_candidates(n, dist, forced, set())
if result:
    kept, removed, fd, cv, dsu = result
    bound = compute_1tree_bound(n, dist, forced, kept)
    print(f"  1-tree bound with forced (8,4): {bound}")
else:
    print(f"  ERROR: {err}")

# 9. And the forbid-(8,4) branch from root
print("\n=== Root + forbid(8,4) ===")
result, err = simulate_build_candidates(n, dist, set(), {(8,4)})
if result:
    kept, removed, fd, cv, dsu = result
    bound = compute_1tree_bound(n, dist, set(), kept)
    print(f"  1-tree bound with forbidden (8,4): {bound}")
    print(f"  < 1618? {'YES' if bound and bound < 1618 else 'NO' if bound and bound >= 1618 else 'INFEASIBLE'}")
else:
    print(f"  ERROR: {err}")
