p="build_pcb.py"; s=open(p,encoding="utf-8").read()
s = s.replace("""            bb = pad.GetBoundingBox()
            x = (bb.GetX() + bb.GetWidth() / 2.0) / 1e6
            y = (bb.GetY() + bb.GetHeight() / 2.0) / 1e6
            hw, hh = bb.GetWidth() / 2e6, bb.GetHeight() / 2e6
            net = pad.GetNetname()""",
"""            ls = pad.GetLayerSet()
            if not (ls.Contains(pcbnew.F_Cu) or ls.Contains(pcbnew.B_Cu)):
                continue   # paste/aperture-only pad: no copper, so not an obstacle
            bb = pad.GetBoundingBox()
            x = (bb.GetX() + bb.GetWidth() / 2.0) / 1e6
            y = (bb.GetY() + bb.GetHeight() / 2.0) / 1e6
            hw, hh = bb.GetWidth() / 2e6, bb.GetHeight() / 2e6
            net = pad.GetNetname()""")
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
