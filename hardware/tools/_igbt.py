import os, subprocess, sys
sys.path.insert(0,".")
from kicadsch import Sch, igbt_symbol
from kicadgen import sexp_dump
d="C:/Users/phamn/Documents/PlatformIO/quickshifter-esp32c3/scratch/kicad_smoke"
s=Sch(title="t", rev="1")
s.place("QS:IGBT_Ignition","Q1","IRGB14C40L","",100,100,0)
s.stub("Q1","1","A"); s.stub("Q1","2","B"); s.stub("Q1","3","C")
txt=sexp_dump(s.libids["QS:IGBT_Ignition"])
print(txt[:2500])
p=os.path.join(d,"igbt.kicad_sch"); s.emit(p)
r=subprocess.run([r"C:\Program Files\KiCad\10.0\bin\kicad-cli.exe","sch","erc","--output",os.path.join(d,"igbt.rpt"),p],capture_output=True,text=True)
print("rc=",r.returncode,"FAIL" if "Failed to load" in r.stdout+r.stderr else "OK")
