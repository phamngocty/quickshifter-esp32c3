import os, sys
sys.path.insert(0,".")
import build_pcb as P
from build_board import PARTS
sheet_fp = {p[0]: (p[3], p[2]) for p in PARTS}
placement = P.compute_placement(sheet_fp)
nets = P.parse_netlist(os.path.join(P.PROJ, P.NAME + ".net"))
board, placed, netmap = P.build_board(nets, sheet_fp, placement)
per_net, obstacles, holes = P.collect_pads(placed)
g = P.build_grid_for("+3V3", 0.6, obstacles, [], 0.21, holes)
W,H = g.nx, g.ny
px = bytearray()
for layer in (0,1):
    for iy in range(H):
        for ix in range(W):
            px.append(255 if g.blocked[layer][g.idx(ix,iy)] else 0)
    px.extend(b"\xff"*W)   # separator row
hdr = f"P5\n{W} {H*2+1}\n255\n".encode()
open(r"C:\Users\phamn\Documents\PlatformIO\quickshifter-esp32c3\scratch\grid.pgm","wb").write(hdr+bytes(px))
print("grid", W, H)
