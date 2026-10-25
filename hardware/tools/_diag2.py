import os, sys, math
sys.path.insert(0,".")
import pcbnew
import build_pcb as P
from build_board import PARTS
sheet_fp = {p[0]: (p[3], p[2]) for p in PARTS}
placement = P.compute_placement(sheet_fp)
nets = P.parse_netlist(os.path.join(P.PROJ, P.NAME + ".net"))
board, placed, netmap = P.build_board(nets, sheet_fp, placement)
per_net, obstacles = P.collect_pads(placed)
grid_probe = P.Grid(P.BX0, P.BY0, P.BX1, P.BY1, P.GRID)
for w, cl in ((2.0,0.25),(1.5,0.25),(1.2,0.21),(0.8,0.21),(0.4,0.21)):
    g = P.build_grid_for("/TRIG", w, obstacles, [], cl)
    # unblock own pads like route_net does
    reach = w/2.0 + cl
    own = per_net["/TRIG"]
    for (px,py,hw,hh) in own:
        for layer in (0,1):
            g.unblock(layer, px-hw-reach-0.15, py-hh-reach-0.15, px+hw+reach+0.15, py+hh+reach+0.15)
    via_r = int(math.ceil(max(0.0, P.VIA_D/2.0+cl-w/2.0)/P.GRID))
    s = (0, g.gx(145.5), g.gy(105.0)); t = (0, g.gx(170.0), g.gy(133.0))
    p = P.astar(g, s, t, 18.0, via_r)
    print(f"w={w} cl={cl} via_r={via_r} -> {'OK len %d'%len(p) if p else 'NO PATH'}")
    if p:
        pts=[g.to_mm(n[1],n[2]) for n in p]
        print("   waypoints:", pts[0], pts[len(pts)//4], pts[len(pts)//2], pts[3*len(pts)//4], pts[-1])
