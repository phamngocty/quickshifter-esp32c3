import os, sys
sys.path.insert(0,".")
import build_pcb as P
from build_board import PARTS
sheet_fp = {p[0]: (p[3], p[2]) for p in PARTS}
placement = P.compute_placement(sheet_fp)
nets = P.parse_netlist(os.path.join(P.PROJ, P.NAME + ".net"))
board, placed, netmap = P.build_board(nets, sheet_fp, placement)
per_net, obstacles, holes = P.collect_pads(placed)
order = P.ROUTE_ORDER + [n for n in sorted(per_net) if n not in P.ROUTE_ORDER and n!="GND" and not n.startswith("unconnected-")]
routed=[]
for net in order:
    if net=="GND" or net not in per_net: continue
    w = P.NET_WIDTH.get(net, P.DEFAULT_WIDTH)
    res=None
    for cl,ww in ((P.CLEARANCE,w),(0.21,w*0.7),(0.21,max(w*0.5,0.4)),(0.21,0.3)):
        g = P.build_grid_for(net, ww, obstacles, routed, cl, holes)
        b = sum(sum(g.blocked[l]) for l in (0,1))/(2*g.nx*g.ny)*100
        try:
            tr,vi = P.route_net(g, per_net[net], net, ww, clearance=cl, holes=holes)
            res=(ww,tr,vi); print(f"{net:14s} w={ww:<4} blocked_before={b:4.1f}%  segs={len(tr)}"); break
        except RuntimeError:
            continue
    if res is None:
        print(f"{net:14s} FAILED  blocked_before={b:4.1f}%")
        continue
    ww,tr,vi = res
    for (x1,y1,x2,y2,layer,tw) in tr:
        routed.append(((min(x1,x2),min(y1,y2),max(x1,x2),max(y1,y2)), tw, layer))
    for (pt,la,lb) in vi:
        for layer in (0,1):
            routed.append(((pt[0]-0.3,pt[1]-0.3,pt[0]+0.3,pt[1]+0.3), 0.6, layer))
