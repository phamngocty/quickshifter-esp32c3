p="build_pcb.py"; s=open(p,encoding="utf-8").read()
s=s.replace('    "J5": (180.0, 124.0, 0),     # spare IO header',
            '    "J5": (134.0, 135.0, 90),    # spare IO header (below U3, short hops)')
s=s.replace('VIA_D = 0.8\nVIA_DRILL = 0.4','VIA_D = 0.6\nVIA_DRILL = 0.3')
s=s.replace('    via_r = int(math.ceil(max(0.0, VIA_D / 2.0 + clearance - width / 2.0) / GRID))',
            '    via_r = max(1, int((VIA_D / 2.0 + clearance - RASTER) / GRID))')
s=s.replace('''ROUTE_ORDER = ["TRIG", "SW_NODE", "+5V", "+12V_IN", "+12V_F", "+12V_P",
               "+12V_SENS", "+3V3", "SPARE_IO7", "SPARE_IO5", "SPARE_IO10",
               "TEST_PWM"]''',
'''ROUTE_ORDER = ["TRIG", "SW_NODE", "+5V", "+12V_IN", "+12V_F", "+12V_P",
               "+12V_SENS", "+3V3", "SPARE_IO7", "SPARE_IO5", "SPARE_IO10",
               "TEST_PWM", "TEST_PWM_OUT", "RPM_SIG", "SHIFT_SIG",
               "IGBT_G", "IGN_DRV", "RPM_LED_A", "SHIFT_IN"]''')
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
