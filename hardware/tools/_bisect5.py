import os, subprocess, sys
sys.path.insert(0,".")
import build_board as B
from kicadsch import Sch
d="C:/Users/phamn/Documents/PlatformIO/quickshifter-esp32c3/scratch/kicad_smoke"

def build(parts, path):
    s=Sch(title="t", rev="1")
    for ref, lib, value, fp, x, y, ang, nets in parts:
        s.place(lib, ref, value, fp, x, y, ang, dnp=(ref in B.DNP))
        for p,n in nets.items(): s.stub(ref,p,n)
    s.emit(path)

def ok(name, parts):
    p=os.path.join(d,name+".kicad_sch"); build(parts,p)
    r=subprocess.run([r"C:\Program Files\KiCad\10.0\bin\kicad-cli.exe","sch","erc","--output",os.path.join(d,name+".rpt"),p],capture_output=True,text=True)
    good = "Failed to load" not in (r.stdout+r.stderr)
    print(f"{name:10s} n={len(parts):2d} {'OK' if good else 'FAIL'}", flush=True)
    return good

# binary search on prefix length
lo, hi = 1, len(B.PARTS)
if not ok("chk_all", B.PARTS):
    while lo < hi:
        mid=(lo+hi)//2
        if ok(f"chk_{mid:02d}", B.PARTS[:mid]): lo=mid+1
        else: hi=mid
    print("first failing part:", B.PARTS[lo-1][0], B.PARTS[lo-1][1])
