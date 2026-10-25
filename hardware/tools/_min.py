import os, sys
sys.path.insert(0, ".")
from kicadsch import Sch
s = Sch(title="t", rev="1")
s.place("Device:R","R1","10k","Resistor_SMD:R_0805_2012Metric", 100, 100, 0)
s.stub("R1","1","A")
s.stub("R1","2","B")
out = "C:/Users/phamn/Documents/PlatformIO/quickshifter-esp32c3/scratch/kicad_smoke/min.kicad_sch"
s.emit(out)
print("wrote", out, os.path.getsize(out))
