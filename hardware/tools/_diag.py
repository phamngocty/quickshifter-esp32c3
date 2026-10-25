import os, sys, math
sys.path.insert(0,".")
import pcbnew
import build_pcb as P
from build_board import PARTS
sheet_fp = {p[0]: (p[3], p[2]) for p in PARTS}
placement = P.compute_placement(sheet_fp)
print("placement:")
for r in sorted(placement): print(f"   {r:4s} {placement[r][0]:7.2f} {placement[r][1]:7.2f} {placement[r][2]}")
nets = P.parse_netlist(os.path.join(P.PROJ, P.NAME + ".net"))
board, placed, netmap = P.build_board(nets, sheet_fp, placement)
per_net, obstacles = P.collect_pads(placed)
for net in ("/TRIG",):
    print(net, "pads:", [(round(x,2),round(y,2),round(hw,2),round(hh,2)) for (x,y,hw,hh) in per_net[net]])
