import os, subprocess, sys
sys.path.insert(0,".")
import build_board as B
from kicadsch import Sch
d="C:/Users/phamn/Documents/PlatformIO/quickshifter-esp32c3/scratch/kicad_smoke"

def build(parts, ncs=True, notes=False, path=None):
    s=Sch(title="t", rev="1")
    for ref, lib, value, fp, x, y, ang, nets in parts:
        s.place(lib, ref, value, fp, x, y, ang, dnp=(ref in B.DNP))
        for p,n in nets.items(): s.stub(ref,p,n)
    if ncs:
        for ref,p in B.NO_CONNECT: s.no_connect(ref,p)
    if notes:
        for t,pt in B.NOTES: s.text(t,pt)
    s.emit(path)

def test(name, **kw):
    p=os.path.join(d,name+".kicad_sch")
    build(path=p, **kw)
    r=subprocess.run([r"C:\Program Files\KiCad\10.0\bin\kicad-cli.exe","sch","erc","--output",os.path.join(d,name+".rpt"),p],capture_output=True,text=True)
    ok="Failed to load" not in (r.stdout+r.stderr)
    print(f"{name:34s} {'LOAD-OK' if ok else 'LOAD-FAIL'} rc={r.returncode}")
    return ok

# progressive: add parts until it breaks
for n in range(1, len(B.PARTS)+1):
    if not test(f"c{n:02d}", parts=B.PARTS[:n], ncs=False):
        print("  -> breaks when adding:", B.PARTS[n-1][0], B.PARTS[n-1][1])
        break
