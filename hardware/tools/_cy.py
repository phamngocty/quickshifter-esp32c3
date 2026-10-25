import pcbnew
F = r"C:\Program Files\KiCad\10.0\share\kicad\footprints"
Q = r"C:\Users\phamn\Documents\PlatformIO\quickshifter-esp32c3\hardware\quickshifter-c3-ign\quickshifter.pretty"
items = [
 (F+"/TerminalBlock_Phoenix.pretty","TerminalBlock_Phoenix_PT-1,5-3-3.5-H_1x03_P3.50mm_Horizontal"),
 (F+"/Package_TO_SOT_SMD.pretty","TO-263-5_TabPin3"),
 (F+"/Package_TO_SOT_THT.pretty","TO-220-3_Vertical"),
 (F+"/Package_TO_SOT_THT.pretty","TO-220-3_Horizontal_TabDown"),
 (F+"/Package_DIP.pretty","DIP-4_W7.62mm"),
 (F+"/Capacitor_THT.pretty","CP_Radial_D6.3mm_P2.50mm"),
 (F+"/Inductor_SMD.pretty","L_Bourns_SRN8040TA"),
 (F+"/Resistor_THT.pretty","R_Axial_DIN0207_L6.3mm_D2.5mm_P10.16mm_Horizontal"),
 (F+"/Connector_PinHeader_2.54mm.pretty","PinHeader_1x04_P2.54mm_Vertical"),
 (Q,"ESP32-C3-SuperMini"),
 (F+"/Fuse.pretty","Fuse_1812_4532Metric"),
 (F+"/Diode_SMD.pretty","D_SMB"),
 (F+"/MountingHole.pretty","MountingHole_3.2mm_M3"),
]
for lib,name in items:
    fp = pcbnew.FootprintLoad(lib,name)
    body = fp.GetBoundingBox(False, False)   # no text, no invisible
    print(f"{name:60s} rel bbox x {body.GetX()/1e6:8.2f} y {body.GetY()/1e6:8.2f}  {body.GetWidth()/1e6:6.2f} x {body.GetHeight()/1e6:6.2f}")
