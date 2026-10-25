import pcbnew
b = pcbnew.LoadBoard(r"C:\Users\phamn\Documents\PlatformIO\quickshifter-esp32c3\hardware\quickshifter-c3-ign\quickshifter-c3-ign.kicad_pcb")
f=b=0
n=0
for t in b.GetTracks():
    if isinstance(t, pcbnew.PCB_VIA):
        n+=1; continue
    if t.GetLayer()==pcbnew.F_Cu: f+=1
    else: b+=1
print("F.Cu tracks",f,"B.Cu tracks",b,"vias",n)
