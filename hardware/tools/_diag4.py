import os, sys
sys.path.insert(0,".")
import build_pcb as P
from build_board import PARTS
sheet_fp = {p[0]: (p[3], p[2]) for p in PARTS}
placement = P.compute_placement(sheet_fp)
nets = P.parse_netlist(os.path.join(P.PROJ, P.NAME + ".net"))
board, placed, netmap = P.build_board(nets, sheet_fp, placement)
per_net, obstacles, holes = P.collect_pads(placed)
print("pads:", sum(len(v) for v in per_net.values()), "obstacles:", len(obstacles), "holes:", len(holes))
g0 = P.build_grid_for("__none__", 0.4, obstacles, [], 0.21, holes)
tot = g0.nx*g0.ny
b0 = sum(sum(g0.blocked[l]) for l in (0,1))
print(f"base blocked (no tracks): {b0/(2*tot)*100:.1f}%  cells={tot}")
# count big obstacles
big = [(round(b[2]-b[0],1), round(b[3]-b[1],1)) for (b,n,l) in obstacles]
big.sort(key=lambda t:-t[0]*t[1])
print("largest pad boxes:", big[:8])
