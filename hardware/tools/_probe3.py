from kicadgen import SymbolLib, collect_pins
for lib,name in [("Regulator_Switching","LM2596S-5"),("Regulator_Switching","LM2596S-5.0"),
                 ("Device","D_TVS"),("Device","C_Polarized"),("Device","C_Polarized_Small"),
                 ("Device","Polyfuse_Small"),("Device","Polyfuse")]:
    try:
        s=SymbolLib.symbol(lib,name); print(f"{lib}:{name} OK pins:",[(p.number,p.name) for p in collect_pins(s)])
    except Exception as e: print(f"{lib}:{name} ERR")
