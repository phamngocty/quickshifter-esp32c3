p="build_pcb.py"; s=open(p,encoding="utf-8").read()
s = s.replace('''               "TEST_PWM", "TEST_PWM_OUT",
               "TRIG", "SW_NODE", "+5V", "+12V_IN", "+12V_F", "+12V_P",
               "+12V_SENS", "+3V3"]''',
'''               "TEST_PWM", "TEST_PWM_OUT", "+3V3",
               "TRIG", "SW_NODE", "+5V", "+12V_IN", "+12V_F", "+12V_P",
               "+12V_SENS"]''')
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
