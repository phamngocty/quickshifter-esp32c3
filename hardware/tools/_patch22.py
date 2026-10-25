p="build_pcb.py"; s=open(p,encoding="utf-8").read()

# collect_pads: carry the pad's copper layers with the pad
s = s.replace("""            if net:
                per_net.setdefault(net, []).append((x, y, hw, hh))
    return per_net, obstacles, holes""",
"""            if net:
                per_net.setdefault(net, []).append((x, y, hw, hh, layers))
    return per_net, obstacles, holes""")

# astar: layer-aware goal set
s = s.replace("        if (cur[1], cur[2]) in goals:", "        if cur in goals:")
s = s.replace("    goals = set(goals)\n    probes = list(goals)[:6]",
              "    goals = set(goals)\n    probes = list({(g[1], g[2]) for g in goals})[:6]")

# route_net: track the arrival layer, start from it next time
old = s[s.index("def route_net(grid, own_boxes"):s.index("def collect_pads(placed):")]
new = '''def route_net(grid, own_boxes, net, width, clearance, via_penalty=18.0, holes=()):
    """Route every pad of one net on a grid already blocked for this net.

    The search is layer aware. Starting every follow-up search on F.Cu looks
    harmless but is not: if the previous merge arrived on B.Cu with no via at
    that cell, the new search begins on copper that is not actually joined to
    it, and DRC reports a dangling track.

    This net's own pads are deliberately left unblocked by `build_grid_for`,
    and its own copper is never reserved in the grid (overlapping copper on the
    same net is harmless).
    """
    if len(own_boxes) < 2:
        return [], []
    nodes = {}
    for (px, py, hw, hh, layers) in own_boxes:
        cell = (grid.gx(px), grid.gy(py))
        for lay in layers:
            nodes.setdefault(cell, set()).add(lay)
    cells = list(nodes)
    tree = [(lay, cells[0][0], cells[0][1]) for lay in sorted(nodes[cells[0]])]
    remaining = set()
    for cell in cells[1:]:
        for lay in nodes[cell]:
            remaining.add((lay, cell[0], cell[1]))

    tracks, vias = [], []
    via_r = max(1, int((VIA_D / 2.0 + clearance - RASTER) / GRID))

    while remaining:
        path = astar(grid, tree, remaining, via_penalty, via_r, holes)
        if path is None:
            left = sorted(grid.to_mm(n[1], n[2]) for n in remaining)
            raise RuntimeError("router failed on net %s; unreached %s" % (net, left))
        last = path[-1]
        geom = path_to_geometry(grid, path, width)
        tracks.extend(geom[0])
        vias.extend(geom[1])
        tree.append(last)
        for (pt, la, lb) in geom[1]:
            if grid.gx(pt[0]) == last[1] and grid.gy(pt[1]) == last[2]:
                tree.append((1 - last[0], last[1], last[2]))
        remaining.discard(last)
        for node in [n for n in remaining if (n[1], n[2]) == (last[1], last[2])]:
            remaining.discard(node)
            tree.append(node)
    return tracks, vias


'''
s = s.replace(old, new)

# per_net entries now carry layers
s = s.replace("        boxes = [(x, y, hw, hh) for (x, y, hw, hh, ref, num) in per_net[net]]",
              "        boxes = [(b[0], b[1], b[2], b[3], b[4]) for b in per_net[net]]")
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
