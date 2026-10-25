import os, sys, zlib, struct
sys.path.insert(0,".")
import build_pcb as P
from build_board import PARTS

def write_png(path, w, h, rows):
    raw = b"".join(b"\x00" + bytes(r) for r in rows)
    def chunk(t, d):
        c = t + d
        return struct.pack(">I", len(d)) + c + struct.pack(">I", zlib.crc32(c) & 0xffffffff)
    png = b"\x89PNG\r\n\x1a\n"
    png += chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 0, 0, 0, 0))
    png += chunk(b"IDAT", zlib.compress(raw, 6))
    png += chunk(b"IEND", b"")
    open(path, "wb").write(png)

sheet_fp = {p[0]: (p[3], p[2]) for p in PARTS}
placement = P.compute_placement(sheet_fp)
nets = P.parse_netlist(os.path.join(P.PROJ, P.NAME + ".net"))
board, placed, netmap = P.build_board(nets, sheet_fp, placement)
per_net, obstacles, holes = P.collect_pads(placed)
g = P.build_grid_for("+3V3", 0.6, obstacles, [], 0.21, holes)
W,H = g.nx, g.ny
rows=[]
for layer in (0,1):
    for iy in range(H):
        rows.append([0 if g.blocked[layer][g.idx(ix,iy)] else 255 for ix in range(W)])
write_png(r"C:\Users\phamn\Documents\PlatformIO\quickshifter-esp32c3\scratch\grid.png", W, H*2+2, rows)
print("wrote grid.png (top=F.Cu, bottom=B.Cu; black=blocked)")
