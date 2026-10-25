p="build_pcb.py"; s=open(p,encoding="utf-8").read()
s = s.replace('''               "TEST_PWM", "TEST_PWM_OUT",
               "TRIG", "SW_NODE", "+5V", "+12V_IN", "+12V_F", "+12V_P",
               "+12V_SENS", "+3V3", "GND"]''',
'''               "TEST_PWM", "TEST_PWM_OUT", "GND",
               "TRIG", "SW_NODE", "+5V", "+12V_IN", "+12V_F", "+12V_P",
               "+12V_SENS", "+3V3"]''')
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
