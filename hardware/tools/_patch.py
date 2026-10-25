import re
p="build_pcb.py"
s=open(p,encoding="utf-8").read()

s = s.replace(
"def route_net(grid, own_boxes, net, width, clearance, via_penalty=18.0):",
"def route_net(grid, own_boxes, net, width, clearance, via_penalty=18.0, holes=()):")
s = s.replace(
"""        for src in tree:
            for goal in remaining:
                p = astar(grid, (0, src[0], src[1]), (0, goal[0], goal[1]),
                          via_penalty, via_r)""",
"""        for src in tree:
            for goal in remaining:
                p = astar(grid, (0, src[0], src[1]), (0, goal[0], goal[1]),
                          via_penalty, via_r, holes)""")
s = s.replace(
"def astar(grid, start, goal, via_penalty, via_radius):",
"def astar(grid, start, goal, via_penalty, via_radius, holes=()):")
s = s.replace(
"        if _via_fits(grid, cx, cy, via_radius):",
"        if _via_fits(grid, cx, cy, via_radius) and not _in_hole(grid, cx, cy, holes):")
s = s.replace(
"""def _via_fits(grid, ix, iy, r):""",
"""def _in_hole(grid, ix, iy, holes):
    \"\"\"A via must never land inside another part's drilled hole.\"\"\"
    x, y = grid.to_mm(ix, iy)
    for (hx, hy, hw, hh) in holes:
        if abs(x - hx) <= hw + 0.35 and abs(y - hy) <= hh + 0.35:
            return True
    return False


def _via_fits(grid, ix, iy, r):""")

# collect_pads: also return pad drill boxes
s = s.replace(
"""def collect_pads(placed):
    \"\"\"(netname -> [(x, y, hw, hh)]), plus (box, net, layers) obstacle list.\"\"\"
    per_net, obstacles = {}, []""",
"""def collect_pads(placed):
    \"\"\"(netname -> pads), obstacle boxes, and drilled-hole boxes.\"\"\"
    per_net, obstacles, holes = {}, [], []""")
s = s.replace(
"""            if net:
                per_net.setdefault(net, []).append((x, y, hw, hh))
    return per_net, obstacles""",
"""            drill = pad.GetDrillSize()
            if drill.x > 0:
                holes.append((x, y, drill.x / 2e6, drill.y / 2e6))
            if net:
                per_net.setdefault(net, []).append((x, y, hw, hh))
    return per_net, obstacles, holes""")

s = s.replace(
"def build_grid_for(net, width, obstacles, routed, clearance):",
"def build_grid_for(net, width, obstacles, routed, clearance, holes=()):")
open(p,"w",encoding="utf-8",newline="\n").write(s)
print("patched")
