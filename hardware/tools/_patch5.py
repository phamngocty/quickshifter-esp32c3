p="build_board.py"; s=open(p,encoding="utf-8").read()
s=s.replace('''    ("J5", "Connector_Generic:Conn_01x04",
     "SPARE IO", "Connector_PinHeader_2.54mm:PinHeader_1x04_P2.54mm_Vertical",
     150, 268, 180,
     {"1": "SPARE_IO7", "2": "SPARE_IO5", "3": "SPARE_IO10", "4": "GND"}),
''', "")
s=s.replace('''      "6": "RPM_SIG", "8": "SHIFT_SIG", "10": "IGN_DRV", "12": "TEST_PWM",
      "9": "SPARE_IO5", "11": "SPARE_IO7", "14": "SPARE_IO10"}),''',
'''      "6": "RPM_SIG", "8": "SHIFT_SIG", "10": "IGN_DRV", "12": "TEST_PWM"}),''')
s=s.replace('''NO_CONNECT = [("U3", "4"), ("U3", "5"), ("U3", "7"), ("U3", "13"),
              ("U3", "15"), ("U3", "16"), ("J3", "3")]''',
'''NO_CONNECT = [("U3", "4"), ("U3", "5"), ("U3", "7"), ("U3", "9"), ("U3", "11"),
              ("U3", "13"), ("U3", "14"), ("U3", "15"), ("U3", "16"), ("J3", "3")]''')
s=s.replace('    ("TEST / DỰ PHÒNG", (25, 258)),\n','')
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok2")
