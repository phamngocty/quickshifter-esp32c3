import pcbnew
b = pcbnew.LoadBoard(r"C:\Users\phamn\Documents\PlatformIO\quickshifter-esp32c3\hardware\quickshifter-c3-ign\quickshifter-c3-ign.kicad_pcb")
for t in b.GetTracks():
    p = t.GetStart(); q = t.GetEnd()
    x1,y1,x2,y2 = p.x/1e6,p.y/1e6,q.x/1e6,q.y/1e6
    if 160 <= min(x1,x2) and max(x1,x2) <= 180 and 114 <= min(y1,y2) and max(y1,y2) <= 130:
        print(f"{t.GetNetname():10s} w={t.GetWidth()/1e6:.2f} ({x1:.2f},{y1:.2f})->({x2:.2f},{y2:.2f}) L={t.GetLayer()}")
