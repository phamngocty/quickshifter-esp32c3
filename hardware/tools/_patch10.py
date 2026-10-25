p="build_pcb.py"; s=open(p,encoding="utf-8").read()
old = """    for ref, fp in placed.items():
        for pad in fp.Pads():
            pos = pad.GetPosition()
            x, y = pos.x / 1e6, pos.y / 1e6
            sz = pad.GetSize()
            hw, hh = sz.x / 2e6, sz.y / 2e6
            net = pad.GetNetname()
            box = (x - hw, y - hh, x + hw, y + hh)"""
new = """    for ref, fp in placed.items():
        for pad in fp.Pads():
            # Use the pad's real bounding box: GetSize() ignores a pad that is
            # rotated inside its footprint (e.g. the SRN8040 inductor pads are
            # 6.6 x 2.85 by size but stand upright on the board), and trusting
            # it made the router lay copper straight across the pad.
            bb = pad.GetBoundingBox()
            x = (bb.GetX() + bb.GetWidth() / 2.0) / 1e6
            y = (bb.GetY() + bb.GetHeight() / 2.0) / 1e6
            hw, hh = bb.GetWidth() / 2e6, bb.GetHeight() / 2e6
            net = pad.GetNetname()
            box = (x - hw, y - hh, x + hw, y + hh)"""
assert old in s
s = s.replace(old, new)
# via de-duplication inside a net
old2 = """        for (pt, la, lb) in vias:
            v = pcbnew.PCB_VIA(board)"""
new2 = """        kept = []
        for (pt, la, lb) in vias:
            if any((pt[0] - k[0]) ** 2 + (pt[1] - k[1]) ** 2 < 1.1 ** 2 for k in kept):
                continue
            kept.append(pt)
            v = pcbnew.PCB_VIA(board)"""
assert old2 in s
s = s.replace(old2, new2)
s = s.replace('''        for (pt, la, lb) in vias:
            if any((pt[0] - k[0])''', '''        for (pt, la, lb) in kept_loop_marker:
            if any((pt[0] - k[0])''')
s = s.replace("        for (pt, la, lb) in kept_loop_marker:", "        for (pt, la, lb) in vias:")
s = s.replace('    "Q1": (170.0, 133.0, 0),', '    "Q1": (168.0, 133.0, 0),')
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
