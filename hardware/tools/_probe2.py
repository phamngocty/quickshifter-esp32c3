from kicadgen import SymbolLib, collect_pins
for lib,name in [("Device","Fuse"),("Device","Polyfuse"),("Device","TestPoint"),
                 ("Device","R_Small"),("Device","C_Small"),("Device","Jumper"),
                 ("Mechanical","MountingHole"),("Connector","TestPoint")]:
    try:
        s=SymbolLib.symbol(lib,name); print(lib,name,"OK pins:",[(p.number,p.name,p.etype) for p in collect_pins(s)])
    except Exception as e: print(lib,name,"ERR",e)
print()
raw = SymbolLib.raw_symbol_text("Transistor_IGBT","IRG4PF50W")
print("IRG4PF50W raw len", len(raw))
