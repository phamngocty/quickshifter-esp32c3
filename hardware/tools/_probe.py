from kicadgen import SymbolLib, collect_pins, pin_position
tests = [
    ("Device","R"), ("Device","C"), ("Device","D"), ("Device","D_Zener"),
    ("Device","D_Schottky"), ("Device","L"), ("Device","Q_NIGBT_CEG"),
    ("Isolator","PC817"), ("Connector_Generic","Conn_01x03"),
    ("power","GND"), ("power","+5V"), ("power","+3V3"),
    ("Transistor_IGBT","IRG4PF50W"),
]
for lib,name in tests:
    try:
        sym = SymbolLib.symbol(lib,name)
        ext = SymbolLib.child(sym,"extends")
        pins = collect_pins(sym)
        print(f"{lib}:{name}  extends={ext[1] if ext else None}  pins={len(pins)}")
        for p in pins:
            print("    ", p)
    except Exception as e:
        print(f"{lib}:{name}  ERROR {e}")
