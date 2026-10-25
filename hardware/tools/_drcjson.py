import json
d = json.load(open(r"C:\Users\phamn\Documents\PlatformIO\quickshifter-esp32c3\hardware\quickshifter-c3-ign\drc.json", encoding="utf-8"))
for v in d.get("violations", []):
    if v["type"] in ("shorting_items", "solder_mask_bridge", "unconnected_items", "hole_to_hole"):
        print(v["type"], "-", v.get("description","")[:90])
        for it in v.get("items", []):
            print("    ", it.get("description","")[:110])
