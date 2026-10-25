p="build_pcb.py"; s=open(p,encoding="utf-8").read()

# Route GND as a real net. Relying on the pour alone left a fragmented island
# holding U1's tab pad; a routed GND is verifiable by DRC.
s = s.replace('''               "TRIG", "SW_NODE", "+5V", "+12V_IN", "+12V_F", "+12V_P",
               "+12V_SENS", "+3V3"]''',
'''               "TRIG", "SW_NODE", "+5V", "+12V_IN", "+12V_F", "+12V_P",
               "+12V_SENS", "+3V3", "GND"]''')
s = s.replace('NET_WIDTH = {\n    "+12V_IN": 0.8,',
              'NET_WIDTH = {\n    "GND": 0.5, "+12V_IN": 0.8,')
# drop the per-pad GND via pass (GND is now routed)
s = s.replace('''    gnd_vias = add_gnd_pad_vias(board, placed, netmap, obstacles, holes, CLEARANCE)
    for box, w, layer in gnd_vias:
        routed.append((box, w, layer))
    print("  GND pad vias:", len(gnd_vias) // 2)
''', "")
s = s.replace("        if net == \"GND\" or net not in per_net:\n            continue",
              "        if net not in per_net:\n            continue")

# reference designators: smaller, centred above the part, so they stop
# colliding with the neighbouring part's silkscreen
s = s.replace("""        board.Add(fp)
        placed[ref] = fp
    return board, placed, netmap""",
"""        body = fp.GetBoundingBox(False, False)
        rt = fp.Reference()
        rt.SetTextSize(pcbnew.VECTOR2I_MM(0.8, 0.8))
        rt.SetTextThickness(pcbnew.FromMM(0.13))
        rt.SetPosition(pcbnew.VECTOR2I(int(body.GetX() + body.GetWidth() / 2.0),
                                       int(body.GetY() - pcbnew.FromMM(1.1))))
        board.Add(fp)
        placed[ref] = fp
    return board, placed, netmap""")
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
