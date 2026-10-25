p="build_pcb.py"; s=open(p,encoding="utf-8").read()
s = s.replace('''               "TEST_PWM", "TEST_PWM_OUT", "GND",
               "TRIG"''', '''               "TEST_PWM", "TEST_PWM_OUT",
               "TRIG"''')
s = s.replace('''    routed = []
    failed = []
''', '''    routed = []
    failed = []
    for box, w, layer in add_gnd_pad_vias(board, placed, netmap, obstacles, holes,
                                          CLEARANCE):
        routed.append((box, w, layer))
''')
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
