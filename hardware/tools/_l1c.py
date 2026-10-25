import pcbnew
b = pcbnew.LoadBoard(r"C:\Users\phamn\Documents\PlatformIO\quickshifter-esp32c3\hardware\quickshifter-c3-ign\quickshifter-c3-ign.kicad_pcb")
fp = b.FindFootprintByReference("L1")
print("L1 at", fp.GetPosition().x/1e6, fp.GetPosition().y/1e6, "rot", fp.GetOrientationDegrees())
for p in fp.Pads():
    pos=p.GetPosition(); sz=p.GetSize()
    bb=p.GetBoundingBox()
    print(f"  pad {p.GetNumber()} net={p.GetNetname()} pos=({pos.x/1e6:.3f},{pos.y/1e6:.3f}) size=({sz.x/1e6:.3f},{sz.y/1e6:.3f}) numpy={p.GetShape()}")
    print(f"      bbox ({bb.GetX()/1e6:.3f},{bb.GetY()/1e6:.3f}) {bb.GetWidth()/1e6:.3f} x {bb.GetHeight()/1e6:.3f}")
    print(f"      clearance={p.GetOwnClearance()/1e6} localclear={p.GetLocalClearance()/1e6} maskmargin={p.GetSolderMaskExpansion()/1e6}")
