p="build_pcb.py"; s=open(p,encoding="utf-8").read()
s = s.replace("""    goals = set(goals)
    openq = []
    came = {}
    g = {}
    for (sx, sy) in starts:
        node = (0, sx, sy)
        g[node] = 0.0
        came[node] = None
        heapq.heappush(openq, (0.0, node))""",
"""    goals = set(goals)
    probes = list(goals)[:6]

    def h(x, y):
        return min(abs(x - gx) + abs(y - gy) for (gx, gy) in probes)

    openq = []
    came = {}
    g = {}
    for (sx, sy) in starts:
        node = (0, sx, sy)
        g[node] = 0.0
        came[node] = None
        heapq.heappush(openq, (h(sx, sy), node))""")
s = s.replace("""                    heapq.heappush(openq, (ng + abs(x2 - cx) * 0.0 + 0.0, nxt))""",
              """                    heapq.heappush(openq, (ng + h(x2, y2), nxt))""")
s = s.replace("""                heapq.heappush(openq, (ng, nxt))""",
              """                heapq.heappush(openq, (ng + h(cx, cy), nxt))""")
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
