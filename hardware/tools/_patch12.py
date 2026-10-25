p="build_pcb.py"; s=open(p,encoding="utf-8").read()

# 1) more room between parts for silkscreen reference designators
s = s.replace("PACK_GAP = 0.9", "PACK_GAP = 1.3")

# 2) wider thermal spokes
s = s.replace("z.SetThermalReliefSpokeWidth(pcbnew.FromMM(0.4))",
              "z.SetThermalReliefSpokeWidth(pcbnew.FromMM(0.6))")

# 3) stitch every isolated F.Cu GND island down to the B.Cu pour
s = s.replace("""def add_gnd_zones(board, gnd_net):""",
"""def add_gnd_stitching(board, gnd_net):
    \"\"\"Drop a via inside every F.Cu GND island.

    The top-side pour gets cut into islands by the routing. Any island that
    still touches a GND pad is not \"isolated\" so KiCad will not remove it,
    yet it is not connected to the rest of GND either. A via placed *inside*
    the filled polygon is always clear of other nets (the pour already keeps
    clearance), and ties the island to the bottom-side pour.
    \"\"\"
    added = 0
    for zone in board.Zones():
        if zone.GetLayer() != pcbnew.F_Cu:
            continue
        poly = zone.GetFilledPolysList(pcbnew.F_Cu)
        for i in range(poly.OutlineCount()):
            outline = poly.Outline(i)
            bb = outline.BBox()
            cx = (bb.GetX() + bb.GetWidth() / 2.0)
            cy = (bb.GetY() + bb.GetHeight() / 2.0)
            if bb.GetWidth() < pcbnew.FromMM(1.2) or bb.GetHeight() < pcbnew.FromMM(1.2):
                continue
            pt = pcbnew.VECTOR2I(int(cx), int(cy))
            if not poly.Contains(pt):
                continue
            v = pcbnew.PCB_VIA(board)
            v.SetPosition(pt)
            v.SetWidth(pcbnew.FromMM(0.6))
            v.SetDrill(pcbnew.FromMM(0.3))
            v.SetViaType(pcbnew.VIATYPE_THROUGH)
            v.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu)
            v.SetNet(gnd_net)
            board.Add(v)
            added += 1
    return added


def add_gnd_zones(board, gnd_net):""")

# 4) call it after the first fill
s = s.replace("""    pcbnew.ZONE_FILLER(board).Fill(board.Zones())
    out = os.path.join(PROJ, NAME + ".kicad_pcb")""",
"""    filler = pcbnew.ZONE_FILLER(board)
    filler.Fill(board.Zones())
    stitched = add_gnd_stitching(board, netmap["GND"])
    filler.Fill(board.Zones())
    print("  GND stitching vias:", stitched)
    out = os.path.join(PROJ, NAME + ".kicad_pcb")""")
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
