p="build_pcb.py"; s=open(p,encoding="utf-8").read()
s=s.replace('''KEEPOUT_CHANNELS = [
    (133.0, 131.5, 162.0, 137.0),   # horizontal channel under the module
    (157.5, 106.0, 162.5, 131.0),   # vertical channel right of the module
    (100.5, 110.5, 108.5, 137.0),   # vertical channel down the left edge
]''','''KEEPOUT_CHANNELS = [
    (133.0, 131.5, 160.0, 137.0),   # horizontal channel under the module
]''')
s=s.replace('''ROUTE_ORDER = ["TRIG", "SW_NODE", "+5V", "+12V_IN", "+12V_F", "+12V_P",
               "+12V_SENS", "+3V3", "SPARE_IO7", "SPARE_IO5", "SPARE_IO10",
               "TEST_PWM", "TEST_PWM_OUT", "RPM_SIG", "SHIFT_SIG",
               "IGBT_G", "IGN_DRV", "RPM_LED_A", "SHIFT_IN"]''',
'''# Short local nets first: they get the pick of an empty board, wide power
# nets can always find a way round, and long signal nets come last.
ROUTE_ORDER = ["RPM_LED_A", "RPM_OPT", "RPM_SIG", "RPM_IN",
               "SHIFT_IN", "SHIFT_SIG", "IGBT_G", "IGN_DRV",
               "TEST_PWM", "TEST_PWM_OUT",
               "TRIG", "SW_NODE", "+5V", "+12V_IN", "+12V_F", "+12V_P",
               "+12V_SENS", "+3V3"]''')
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
