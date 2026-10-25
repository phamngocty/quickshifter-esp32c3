p="build_pcb.py"; s=open(p,encoding="utf-8").read()
s = s.replace("PACK_GAP = 1.3", "PACK_GAP = 0.9")

# F.Cu pour out: a fragmented top pour created islands that no amount of
# stitching could keep joined. Keep the bottom pour as the ground plane and
# give every SMD GND pad its own via into it.
s = s.replace("def add_gnd_zones(board, gnd_net):\n    for layer in (pcbnew.F_Cu, pcbnew.B_Cu):",
              "def add_gnd_zones(board, gnd_net):\n    for layer in (pcbnew.B_Cu,):")

old = """    filler = pcbnew.ZONE_FILLER(board)
    filler.Fill(board.Zones())
    stitched = add_gnd_stitching(board, netmap["GND"])
    filler.Fill(board.Zones())
    print("  GND stitching vias:", stitched)"""
new = """    filler = pcbnew.ZONE_FILLER(board)
    filler.Fill(board.Zones())"""
assert old in s
s = s.replace(old, new)

# insert the GND-via pass just before routing
old2 = """    order = ROUTE_ORDER + [n for n in sorted(per_net)"""
new2 = """    gnd_vias = add_gnd_pad_vias(board, placed, netmap["GND"])
    for box, w, layer in gnd_vias:
        routed.append((box, w, layer))
    print("  GND pad vias:", len(gnd_vias) // 2)

    order = ROUTE_ORDER + [n for n in sorted(per_net)"""
assert old2 in s
s = s.replace(old2, new2, 1)

s = s.replace("""def add_gnd_stitching(board, gnd_net):""",
"""def add_gnd_pad_vias(board, placed, gnd_net, offset=1.25):
    \"\"\"Give every SMD GND pad its own via into the bottom-side ground pour.

    There is no top-side pour (it fragmented into islands), so an SMD GND pad
    has no path to ground unless we provide one. The via is placed just
    outside the part, along the axis of its own pad row.
    \"\"\"
    added = []
    for ref, fp in placed.items():
        pads = list(fp.Pads())
        for pad in pads:
            if pad.GetNetname() != "GND" or pad.GetAttribute() == pcbnew.PAD_ATTRIB_PTH:
                continue
            bb = pad.GetBoundingBox()
            cx = (bb.GetX() + bb.GetWidth() / 2.0) / 1e6
            cy = (bb.GetY() + bb.GetHeight() / 2.0) / 1e6
            hw, hh = bb.GetWidth() / 2e6, bb.GetHeight() / 2e6
            others = [q for q in pads if q is not pad]
            if not others:
                continue
            ox = sum(q.GetPosition().x for q in others) / len(others) / 1e6
            oy = sum(q.GetPosition().y for q in others) / len(others) / 1e6
            dx, dy = cx - ox, cy - oy
            n = (dx * dx + dy * dy) ** 0.5 or 1.0
            dx, dy = dx / n, dy / n
            for sign in (1, -1):
                vx, vy = cx + sign * dx * offset, cy + sign * dy * offset
                if not (BX0 + 1.2 < vx < BX1 - 1.2 and BY0 + 1.2 < vy < BY1 - 1.2):
                    continue
                break
            else:
                continue
            v = pcbnew.PCB_VIA(board)
            v.SetPosition(pcbnew.VECTOR2I_MM(vx, vy))
            v.SetWidth(pcbnew.FromMM(VIA_D))
            v.SetDrill(pcbnew.FromMM(VIA_DRILL))
            v.SetViaType(pcbnew.VIATYPE_THROUGH)
            v.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu)
            v.SetNet(gnd_net)
            board.Add(v)
            t = pcbnew.PCB_TRACK(board)
            t.SetStart(pcbnew.VECTOR2I_MM(cx, cy))
            t.SetEnd(pcbnew.VECTOR2I_MM(vx, vy))
            t.SetWidth(pcbnew.FromMM(0.4))
            t.SetLayer(pcbnew.F_Cu)
            t.SetNet(gnd_net)
            board.Add(t)
            box = (min(cx, vx) - 0.4, min(cy, vy) - 0.4,
                   max(cx, vx) + 0.4, max(cy, vy) + 0.4)
            added.append((box, 0.9, 0))
            added.append((box, 0.9, 1))
    return added


def add_gnd_stitching(board, gnd_net):""")
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
