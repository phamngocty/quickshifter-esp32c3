import os, sys, math
sys.path.insert(0,".")
import build_pcb as P
from build_board import PARTS
sheet_fp = {p[0]: (p[3], p[2]) for p in PARTS}
placement = P.compute_placement(sheet_fp)
nets = P.parse_netlist(os.path.join(P.PROJ, P.NAME + ".net"))
board, placed, netmap = P.build_board(nets, sheet_fp, placement)
per_net, obstacles = P.collect_pads(placed)
for w, cl in ((2.0,0.25),):
    g = P.build_grid_for("/TRIG", w, obstacles, [], cl)
    try:
        tr, vi = P.route_net(g, per_net["/TRIG"], "/TRIG", w, cl)
        print("OK", len(tr), len(vi))
    except RuntimeError as e:
        print("FAIL:", e)
