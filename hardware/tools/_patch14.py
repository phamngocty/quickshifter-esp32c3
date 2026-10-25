p="build_pcb.py"; s=open(p,encoding="utf-8").read()
block = """    gnd_vias = add_gnd_pad_vias(board, placed, netmap["GND"])
    for box, w, layer in gnd_vias:
        routed.append((box, w, layer))
    print("  GND pad vias:", len(gnd_vias) // 2)

"""
assert block in s
s = s.replace(block, "", 1)
anchor = """    routed = []
    failed = []
"""
assert anchor in s
s = s.replace(anchor, anchor + block, 1)
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
