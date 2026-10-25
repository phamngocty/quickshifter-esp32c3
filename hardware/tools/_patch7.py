p="build_pcb.py"; s=open(p,encoding="utf-8").read()
s=s.replace("PACK_GAP = 2.0","PACK_GAP = 0.9\nESCAPE = 1.7       # every part needs at least one side this free, so its\n                   # pads can actually be reached by a track")
s=s.replace("""def _overlap(a, b, pad=0.0):""",
"""def _has_escape(box, obstacles, need=ESCAPE):
    \"\"\"True if at least one side of `box` is open board (not another part).

    A part wedged on all four sides ends up with its pads' keep-outs merged
    into the neighbours' and no track can ever reach it.
    \"\"\"
    strips = ((box[0] - need, box[1], box[0], box[3]),
              (box[2], box[1], box[2] + need, box[3]),
              (box[0], box[1] - need, box[2], box[1]),
              (box[0], box[3], box[2], box[3] + need))
    for st in strips:
        if st[0] < BX0 + 0.3 or st[2] > BX1 - 0.3 or st[1] < BY0 + 0.3 or st[3] > BY1 - 0.3:
            continue
        if not any(_overlap(st, o) for o in obstacles):
            return True
    return False


def _overlap(a, b, pad=0.0):""")
s=s.replace("                   not any(_overlap(box, o, PACK_GAP) for o in obstacles):",
            "                   not any(_overlap(box, o, PACK_GAP) for o in obstacles) and \\\n                   _has_escape(box, obstacles):")
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
