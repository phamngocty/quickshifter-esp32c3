import pcbnew, sys
b = pcbnew.LoadBoard(r"C:\Users\phamn\Documents\PlatformIO\quickshifter-esp32c3\hardware\quickshifter-c3-ign\quickshifter-c3-ign.kicad_pcb")
tr = list(b.GetTracks())
print("tracks+vias:", len(tr))
n=0
for t in tr:
    if isinstance(t, pcbnew.PCB_VIA):
        continue
    p, q = t.GetStart(), t.GetEnd()
    x1,y1,x2,y2 = p.x/1e6,p.y/1e6,q.x/1e6,q.y/1e6
    if min(x1,x2) <= 174.9 and max(x1,x2) >= 165.5 and min(y1,y2) <= 126 and max(y1,y2) >= 118:
        print(f"{t.GetNetname():10s} w={t.GetWidth()/1e6:.2f} L{t.GetLayer()} ({x1:.2f},{y1:.2f})->({x2:.2f},{y2:.2f})")
        n+=1
print("near L1:", n)
