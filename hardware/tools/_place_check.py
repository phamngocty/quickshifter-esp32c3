import os, sys
sys.path.insert(0,".")
import pcbnew
from build_pcb import PLACEMENT, fp_path, BX0,BY0,BX1,BY1
from kicadgen import sexp_parse, head, child, _unquote
import build_board as B
sheet_fp = {p[0]: (p[3], p[2]) for p in B.PARTS}

boxes = {}
for ref, spec in PLACEMENT.items():
    if spec is None: continue
    libid, value = sheet_fp.get(ref, ("", ""))
    if not libid: continue
    lib, name = fp_path(libid)
    fp = pcbnew.FootprintLoad(lib, name)
    x,y,rot = spec
    fp.SetReference(ref); fp.SetPosition(pcbnew.VECTOR2I_MM(x,y)); fp.SetOrientationDegrees(rot)
    bb = fp.GetBoundingBox(False, False)
    bx0,by0 = bb.GetX()/1e6, bb.GetY()/1e6
    bx1,by1 = bx0+bb.GetWidth()/1e6, by0+bb.GetHeight()/1e6
    boxes[ref]=(bx0,by0,bx1,by1)
    if bx0 < BX0+0.3 or by0 < BY0+0.3 or bx1 > BX1-0.3 or by1 > BY1-0.3:
        print(f"OUT OF BOARD {ref:4s} {bx0:.1f},{by0:.1f} .. {bx1:.1f},{by1:.1f}")
refs=sorted(boxes)
for i,a in enumerate(refs):
    for b in refs[i+1:]:
        A,Bx=boxes[a],boxes[b]
        if A[0]<Bx[2] and Bx[0]<A[2] and A[1]<Bx[3] and Bx[1]<A[3]:
            print(f"OVERLAP {a} <-> {b}   {A[0]:.1f},{A[1]:.1f}..{A[2]:.1f},{A[3]:.1f} | {Bx[0]:.1f},{Bx[1]:.1f}..{Bx[2]:.1f},{Bx[3]:.1f}")
print("---- pads of interest ----")
for ref in ("J1","J2","J3","Q1","R4","R8","J4","J5","U3"):
    libid,_ = sheet_fp[ref]; lib,name = fp_path(libid)
    fp = pcbnew.FootprintLoad(lib,name); x,y,rot = PLACEMENT[ref]
    fp.SetPosition(pcbnew.VECTOR2I_MM(x,y)); fp.SetOrientationDegrees(rot)
    ps = [(p.GetNumber(), round(p.GetPosition().x/1e6,2), round(p.GetPosition().y/1e6,2)) for p in fp.Pads()]
    print(f"{ref:4s} {ps}")
