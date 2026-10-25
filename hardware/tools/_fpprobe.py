import pcbnew
F = r"C:\Program Files\KiCad\10.0\share\kicad\footprints"
Q = r"C:\Users\phamn\Documents\PlatformIO\quickshifter-esp32c3\hardware\quickshifter-c3-ign\quickshifter.pretty"
items = [
 (F+"/TerminalBlock_Phoenix.pretty","TerminalBlock_Phoenix_PT-1,5-3-3.5-H_1x03_P3.50mm_Horizontal"),
 (F+"/Package_TO_SOT_SMD.pretty","TO-263-5_TabPin3"),
 (F+"/Package_TO_SOT_THT.pretty","TO-220-3_Horizontal_TabDown"),
 (F+"/Package_DIP.pretty","DIP-4_W7.62mm"),
 (F+"/Capacitor_THT.pretty","CP_Radial_D6.3mm_P2.50mm"),
 (F+"/Inductor_SMD.pretty","L_Bourns_SRN8040TA"),
 (F+"/Diode_SMD.pretty","D_SMA"),(F+"/Diode_SMD.pretty","D_SMB"),(F+"/Diode_SMD.pretty","D_SOD-123"),
 (F+"/Package_TO_SOT_SMD.pretty","SOT-23"),
 (F+"/Resistor_SMD.pretty","R_0805_2012Metric"),
 (F+"/Capacitor_SMD.pretty","C_0805_2012Metric"),
 (F+"/Fuse.pretty","Fuse_1812_4532Metric"),(F+"/Fuse.pretty","Fuse_1206_3216Metric"),
 (F+"/Connector_PinHeader_2.54mm.pretty","PinHeader_1x02_P2.54mm_Vertical"),
 (F+"/Connector_PinHeader_2.54mm.pretty","PinHeader_1x04_P2.54mm_Vertical"),
 (F+"/MountingHole.pretty","MountingHole_3.2mm_M3"),
 (F+"/Resistor_THT.pretty","R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal"),
 (Q,"ESP32-C3-SuperMini"),
]
for lib,name in items:
    fp = pcbnew.FootprintLoad(lib,name)
    if not fp: print("MISSING", lib, name); continue
    bb = fp.GetBoundingBox()
    pads = [(p.GetNumber(), round(p.GetPosition().x/1e6,2), round(p.GetPosition().y/1e6,2), round(p.GetSize().x/1e6,2), round(p.GetSize().y/1e6,2)) for p in fp.Pads()]
    print(f"{name}\n   bbox {bb.GetX()/1e6:.2f},{bb.GetY()/1e6:.2f} {bb.GetWidth()/1e6:.2f}x{bb.GetHeight()/1e6:.2f}  npads={len(pads)}")
    print("   pads:", pads[:8])
