import os, subprocess, sys
sys.path.insert(0,".")
from kicadsch import Sch
from kicadgen import sexp_dump
d="C:/Users/phamn/Documents/PlatformIO/quickshifter-esp32c3/scratch/kicad_smoke"
s=Sch(title="t", rev="1")
s.place("Regulator_Switching:LM2596S-5","U1","LM2596S-5.0","",100,100,0)
s.stub("U1","1","A")
for k in ["Regulator_Switching:LM2596S-5"]:
    txt = sexp_dump(s.libids[k])
    print(txt[:1800])
p=os.path.join(d,"lm.kicad_sch"); s.emit(p)
r=subprocess.run([r"C:\Program Files\KiCad\10.0\bin\kicad-cli.exe","sch","erc","--output",os.path.join(d,"lm.rpt"),p],capture_output=True,text=True)
print("rc=",r.returncode, "LOAD-FAIL" if "Failed to load" in r.stdout+r.stderr else "LOAD-OK")
