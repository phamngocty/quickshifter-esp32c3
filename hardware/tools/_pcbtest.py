import pcbnew, os
print("pcbnew", pcbnew.GetBuildVersion())
b = pcbnew.CreateEmptyBoard()
print("board ok", b)
lib = r"C:\Users\phamn\Documents\PlatformIO\quickshifter-esp32c3\hardware\quickshifter-c3-ign\quickshifter.pretty"
fp = pcbnew.FootprintLoad(lib, "ESP32-C3-SuperMini")
print("fp:", fp, "pads:", fp.GetPadCount() if fp else None)
for p in fp.Pads():
    print("  pad", p.GetNumber(), p.GetPosition().x/1e6, p.GetPosition().y/1e6, p.GetDrillSize().x/1e6)
