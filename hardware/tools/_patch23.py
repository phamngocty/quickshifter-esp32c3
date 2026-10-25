p="build_pcb.py"; s=open(p,encoding="utf-8").read()
s = s.replace("""    for (sx, sy) in starts:
        node = (0, sx, sy)""",
"""    for (sl, sx, sy) in starts:
        node = (sl, sx, sy)""")
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
