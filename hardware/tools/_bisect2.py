import os, subprocess, sys
sys.path.insert(0,".")
from kicadgen import sexp_parse, sexp_dump
d="C:/Users/phamn/Documents/PlatformIO/quickshifter-esp32c3/scratch/kicad_smoke"
src=open(os.path.join(d,"t.kicad_sch"),encoding="utf-8").read()
tree=sexp_parse(src)[0]
out=sexp_dump(tree)+"\n"
p=os.path.join(d,"w2_redump.kicad_sch")
open(p,"w",encoding="utf-8",newline="\n").write(out)
r=subprocess.run([r"C:\Program Files\KiCad\10.0\bin\kicad-cli.exe","sch","erc","--output",os.path.join(d,"w2.rpt"),p],capture_output=True,text=True)
print("W2 redump rc=",r.returncode)
print((r.stdout+r.stderr)[:600])
