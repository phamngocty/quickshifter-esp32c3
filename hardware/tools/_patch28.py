p="build_pcb.py"; s=open(p,encoding="utf-8").read()
s = s.replace("            path = astar(g, (0, src[0], src[1]), (0, target[0], target[1]), 16.0, 0, holes)",
              "            path = astar(g, [(0, src[0], src[1])], {(0, target[0], target[1])},\n                         16.0, 0, holes)")
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
