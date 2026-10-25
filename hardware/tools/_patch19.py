p="build_pcb.py"; s=open(p,encoding="utf-8").read()
old_start = s.index("def astar(grid, start, goal, via_penalty, via_radius, holes=()):")
old_end = s.index("def _in_hole(grid, ix, iy, holes):")
new = '''def astar(grid, starts, goals, via_penalty, via_radius, holes=()):
    """Multi-source A* over (layer, ix, iy).

    Growing the tree from *all* already-connected pads at once and stopping at
    the first unreached pad turns each merge step into a single search. Trying
    every (source, goal) pair instead is O(n^2) searches and does not finish
    for a net like GND with forty pads.

    `starts` and `goals` are iterables of (ix, iy).
    """
    goals = set(goals)
    openq = []
    came = {}
    g = {}
    for (sx, sy) in starts:
        node = (0, sx, sy)
        g[node] = 0.0
        came[node] = None
        heapq.heappush(openq, (0.0, node))
    while openq:
        _, cur = heapq.heappop(openq)
        if (cur[1], cur[2]) in goals:
            path = []
            n = cur
            while n is not None:
                path.append(n)
                n = came[n]
            return path[::-1]
        cl, cx, cy = cur
        base = g[cur]
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            x2, y2 = cx + dx, cy + dy
            if grid.free(cl, x2, y2):
                nxt = (cl, x2, y2)
                ng = base + 1.0
                if ng < g.get(nxt, 1e18):
                    g[nxt] = ng
                    came[nxt] = cur
                    heapq.heappush(openq, (ng + abs(x2 - cx) * 0.0 + 0.0, nxt))
        other = 1 - cl
        if _via_fits(grid, cx, cy, via_radius) and not _in_hole(grid, cx, cy, holes):
            nxt = (other, cx, cy)
            ng = base + via_penalty
            if ng < g.get(nxt, 1e18):
                g[nxt] = ng
                came[nxt] = cur
                heapq.heappush(openq, (ng, nxt))
    return None


'''
s = s[:old_start] + new + s[old_end:]

# route_net: one multi-source search per merge
old_rn = s.index("    while remaining:\n        best = None")
old_rn_end = s.index("def collect_pads(placed):")
new_rn = '''    while remaining:
        path = astar(grid, [n for n in tree], remaining, via_penalty, via_r, holes)
        if path is None:
            left = sorted(grid.to_mm(ix, iy) for (ix, iy) in remaining)
            raise RuntimeError("router failed on net %s; unreached %s" % (net, left))
        goal = (path[-1][1], path[-1][2])
        geom = path_to_geometry(grid, path, width)
        tracks.extend(geom[0])
        vias.extend(geom[1])
        tree.append(goal)
        remaining.discard(goal)
    return tracks, vias


'''
s = s[:old_rn] + new_rn + s[old_rn_end:]
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
