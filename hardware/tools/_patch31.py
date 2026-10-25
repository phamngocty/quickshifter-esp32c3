p="build_pcb.py"; s=open(p,encoding="utf-8").read()
s = s.replace("    add_gnd_zones(board, netmap[\"GND\"])\n", "")
s = s.replace('''    routed = []
    failed = []
    for box, w, layer in add_gnd_pad_vias(board, placed, netmap, obstacles, holes,
                                          CLEARANCE):
        routed.append((box, w, layer))
''', '''    routed = []
    failed = []
''')
s = s.replace('''ROUTE_ORDER = ["RPM_LED_A",''', '''ROUTE_ORDER = ["GND", "RPM_LED_A",''')
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
