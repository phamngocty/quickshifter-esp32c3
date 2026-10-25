import re
p="build_pcb.py"; s=open(p,encoding="utf-8").read()
start = s.index("def add_gnd_pad_vias(")
end = s.index("def add_gnd_stitching(")
new = '''def add_gnd_pad_vias(board, placed, netmap, obstacles, holes, clearance):
    """Route every SMD GND pad to its own via into the bottom-side ground pour.

    There is no top-side pour (it fragmented into unreachable islands), so an
    SMD GND pad has no path to ground unless we make one. Both the via site and
    the little connecting track are found on a grid that already knows about
    every other net's copper, so nothing gets shorted.
    """
    g = build_grid_for("GND", 0.4, obstacles, [], clearance, holes)
    net = netmap["GND"]
    added = []
    for ref, fp in placed.items():
        for pad in fp.Pads():
            if pad.GetNetname() != "GND":
                continue
            if pad.GetAttribute() == pcbnew.PAD_ATTRIB_PTH:
                continue                      # barrel already reaches B.Cu
            bb = pad.GetBoundingBox()
            cx = (bb.GetX() + bb.GetWidth() / 2.0) / 1e6
            cy = (bb.GetY() + bb.GetHeight() / 2.0) / 1e6
            src = (g.gx(cx), g.gy(cy))
            if not (g.free(0, src[0], src[1]) and g.free(1, src[0], src[1])):
                continue
            target = None
            for r in (1.2, 1.6, 2.0, 2.4, 3.0, 3.6):
                for k in range(24):
                    import math as _m
                    ang = 2 * _m.pi * k / 24.0
                    ix, iy = g.gx(cx + r * _m.cos(ang)), g.gy(cy + r * _m.sin(ang))
                    if not (g.free(0, ix, iy) and g.free(1, ix, iy)):
                        continue
                    if not _via_fits(g, ix, iy, 2) or _in_hole(g, ix, iy, holes):
                        continue
                    target = (ix, iy)
                    break
                if target:
                    break
            if target is None:
                print("    (no GND via site for %s pad %s)" % (ref, pad.GetNumber()))
                continue
            path = astar(g, (0, src[0], src[1]), (0, target[0], target[1]), 16.0, 0, holes)
            if path is None:
                continue
            tracks, _ = path_to_geometry(g, path, 0.4)
            vx, vy = g.to_mm(target[0], target[1])
            v = pcbnew.PCB_VIA(board)
            v.SetPosition(pcbnew.VECTOR2I_MM(vx, vy))
            v.SetWidth(pcbnew.FromMM(VIA_D))
            v.SetDrill(pcbnew.FromMM(VIA_DRILL))
            v.SetViaType(pcbnew.VIATYPE_THROUGH)
            v.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu)
            v.SetNet(net)
            board.Add(v)
            for (x1, y1, x2, y2, layer, w) in tracks:
                t = pcbnew.PCB_TRACK(board)
                t.SetStart(pcbnew.VECTOR2I_MM(x1, y1))
                t.SetEnd(pcbnew.VECTOR2I_MM(x2, y2))
                t.SetWidth(pcbnew.FromMM(w))
                t.SetLayer(pcbnew.F_Cu if layer == 0 else pcbnew.B_Cu)
                t.SetNet(net)
                board.Add(t)
            box = (vx - VIA_D / 2, vy - VIA_D / 2, vx + VIA_D / 2, vy + VIA_D / 2)
            for layer in (0, 1):
                added.append((box, VIA_D, layer))
            g.block(0, box[0] - 0.2, box[1] - 0.2, box[2] + 0.2, box[3] + 0.2)
            g.block(1, box[0] - 0.2, box[1] - 0.2, box[2] + 0.2, box[3] + 0.2)
    return added


'''
s = s[:start] + new + s[end:]
s = s.replace('    gnd_vias = add_gnd_pad_vias(board, placed, netmap["GND"])',
              '    gnd_vias = add_gnd_pad_vias(board, placed, netmap, obstacles, holes, CLEARANCE)')
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
