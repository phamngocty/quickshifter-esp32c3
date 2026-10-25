from kicadgen import SymbolLib
for lib,name in [("Regulator_Switching","LM2596S-5"),("Device","D_Schottky"),("Device","R"),("Isolator","PC817")]:
    s=SymbolLib.flatten(lib,name)
    print(lib,name,"->",[(p.number,p.name,p.etype) for p in SymbolLib.pins_of(lib,name)])
