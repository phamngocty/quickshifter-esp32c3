p="build_pcb.py"; s=open(p,encoding="utf-8").read()
s=s.replace("not any(_overlap(box, o, 0.9) for o in obstacles):",
            "not any(_overlap(box, o, PACK_GAP) for o in obstacles):")
s=s.replace("RASTER = 0.10      # block cells whose *centre* is outside the rect too",
"""RASTER = 0.10      # block cells whose *centre* is outside the rect too

# Minimum gap between two placed parts. Anything below roughly 2x (track
# half-width + clearance) leaves the pads' keep-outs touching, which seals a
# pad inside a pocket that no router can escape.
PACK_GAP = 2.0""")
s=s.replace("    area = (BX0 + 0.5, BY0 + 0.5, BX1 - 0.5, BY1 - 0.5)",
            "    area = (BX0 + 1.0, BY0 + 1.0, BX1 - 1.0, BY1 - 1.0)")
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
