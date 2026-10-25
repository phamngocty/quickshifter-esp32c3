p="build_pcb.py"; s=open(p,encoding="utf-8").read()
s = s.replace('ROUTE_ORDER = ["GND", "RPM_LED_A",', 'ROUTE_ORDER = ["RPM_LED_A",')
s = s.replace("def route_net(grid, own_boxes, net, width, clearance, via_penalty=70.0, holes=()):",
              "def route_net(grid, own_boxes, net, width, clearance, via_penalty=400.0, holes=()):")
s = s.replace('''    routed = []
    failed = []
''', '''    routed = []
    failed = []
    for box, w, layer in add_gnd_pad_vias(board, placed, netmap, obstacles, holes,
                                          CLEARANCE):
        routed.append((box, w, layer))
''', 1)
s = s.replace('''    placement = compute_placement(sheet_fp)
    board, placed, netmap = build_board(nets, sheet_fp, placement)
''', '''    placement = compute_placement(sheet_fp)
    board, placed, netmap = build_board(nets, sheet_fp, placement)
    add_gnd_zones(board, netmap["GND"])
''')
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
