p="build_pcb.py"; s=open(p,encoding="utf-8").read()
if "KEEPOUT_CHANNELS = [" not in s:
    marker = 'SMALL_ANCHOR = {'
    s = s.replace(marker, '''# Kept clear of parts so the router has real corridors to use.
KEEPOUT_CHANNELS = [
    (133.0, 130.5, 152.0, 137.0),   # channel under the module
    (155.0, 112.0, 160.0, 137.0),   # channel right of the module
]

''' + marker, 1)
open(p,"w",encoding="utf-8",newline="\n").write(s)
print("KEEPOUT_CHANNELS" in s)
