p="build_pcb.py"; s=open(p,encoding="utf-8").read()
s = s.replace('    "J1": (105.5, 106.0, 270),', '    "J1": (105.5, 103.0, 270),')
s = s.replace('    "J2": (105.5, 118.5, 270),', '    "J2": (105.5, 115.5, 270),')
s = s.replace('    "J3": (105.5, 131.0, 270),', '    "J3": (105.5, 127.5, 270),')
s = s.replace("MOUNTING_HOLES = [(116.0, 102.5), (178.0, 102.5), (116.0, 134.5), (178.0, 134.5)]",
              "MOUNTING_HOLES = [(116.0, 102.5), (178.0, 102.5), (114.0, 134.5), (178.0, 134.5)]")
# Keep the router on F.Cu where it can: every B.Cu detour cuts the ground pour.
s = s.replace("def route_net(grid, own_boxes, net, width, clearance, via_penalty=18.0, holes=()):",
              "def route_net(grid, own_boxes, net, width, clearance, via_penalty=70.0, holes=()):")
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
