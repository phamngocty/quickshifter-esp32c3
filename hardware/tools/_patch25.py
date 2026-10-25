p="build_pcb.py"; s=open(p,encoding="utf-8").read()
old = s[s.index("BIG = {"):s.index("SMALL_ANCHOR = {")]
new = '''BIG = {
    # The three terminal blocks lie along the left edge. Standing them up
    # sideways trades 35 mm of scarce board *width* for 36 mm of the 37 mm
    # height, which is what finally made this board routable at 82 x 37 mm.
    "J1": (105.5, 106.0, 270),   # bike: +12V / GND / RPM
    "J2": (105.5, 118.5, 270),   # SN04-N shift sensor
    "J3": (105.5, 131.0, 270),   # cut trigger wire (J3.1 <-> J3.2)
    "U3": (145.0, 118.0, 0),     # ESP32-C3 Super Mini
    "U1": (167.0, 108.0, 0),     # LM2596 buck
    "U2": (120.0, 130.0, 0),     # PC817 RPM opto
    "R1": (120.0, 124.0, 0),     # 4k7 axial, RPM input
    "L1": (176.0, 120.0, 0),     # 100 uH
    "C1": (113.0, 112.0, 0),     # 100 uF/25V bulk
    "C3": (113.0, 126.0, 0),     # 220 uF/16V
    "Q1": (168.0, 132.0, 0),     # IRGB14C40L ignition IGBT
    "J4": (158.0, 132.0, 0),     # TEST_PWM header
}

'''
s = s.replace(old, new)
s = s.replace("MOUNTING_HOLES = [(178.0, 103.5), (178.0, 133.5), (104.0, 133.5), (104.0, 103.5)]",
              "MOUNTING_HOLES = [(116.0, 102.5), (178.0, 102.5), (116.0, 134.5), (178.0, 134.5)]")
s = s.replace('''SMALL_ANCHOR = {
    "F1": (156.0, 104.0), "F2": (150.0, 104.0), "D1": (156.0, 110.0),
    "D2": (150.0, 110.0), "C2": (116.0, 110.0), "C4": (176.0, 116.0),
    "D3": (160.0, 114.0), "C8": (158.0, 131.0),
    "D4": (126.0, 118.0), "R2": (126.0, 127.0), "C5": (132.0, 127.0),
    "R3": (138.0, 127.0),
    "R4": (126.0, 133.0), "R5": (133.0, 133.0), "D5": (140.0, 133.0),
    "C6": (147.0, 133.0),
    "R6": (162.0, 129.0), "R7": (162.0, 133.0), "D6": (166.0, 129.0),
    "C7": (176.0, 131.0),
}''','''SMALL_ANCHOR = {
    "F1": (152.0, 104.0), "F2": (158.0, 104.0), "D1": (152.0, 110.0),
    "D2": (158.0, 110.0), "C2": (132.0, 104.0), "C4": (176.0, 112.0),
    "D3": (160.0, 114.0), "C8": (176.0, 128.0),
    "D4": (128.0, 116.0), "R2": (126.0, 128.0), "C5": (132.0, 132.0),
    "R3": (138.0, 132.0),
    "R4": (128.0, 134.0), "R5": (134.0, 134.0), "D5": (140.0, 134.0),
    "C6": (146.0, 134.0),
    "R6": (162.0, 124.0), "R7": (155.0, 130.0), "D6": (164.0, 128.0),
    "C7": (174.0, 128.0),
}''')
s = s.replace('''KEEPOUT_CHANNELS = [
    (133.0, 131.5, 160.0, 137.0),   # horizontal channel under the module
]''','''KEEPOUT_CHANNELS = [
    (133.0, 130.5, 152.0, 137.0),   # channel under the module
    (155.0, 112.0, 160.0, 137.0),   # channel right of the module
]''')
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
