p="build_pcb.py"; s=open(p,encoding="utf-8").read()
old = """        best, best_cost = None, None
        y = area[1] - d[1]
        while y + d[3] <= area[3]:
            x = area[0] - d[0]
            while x + d[2] <= area[2]:
                box = (x + d[0], y + d[1], x + d[2], y + d[3])
                if box[0] >= area[0] and box[2] <= area[2] and \\
                   box[1] >= area[1] and box[3] <= area[3] and \\
                   not any(_overlap(box, o, PACK_GAP) for o in obstacles) and \\
                   _has_escape(box, obstacles):
                    cx, cy = (box[0] + box[2]) / 2, (box[1] + box[3]) / 2
                    cost = (cx - ax) ** 2 + (cy - ay) ** 2
                    if best_cost is None or cost < best_cost:
                        best, best_cost = (x, y, box), cost
                x += 0.5
            y += 0.5
        if best is None:
            raise RuntimeError("no free slot for %s" % ref)"""
new = """        best, best_cost = None, None
        for need in (ESCAPE, 1.2, 0.0):
            y = area[1] - d[1]
            while y + d[3] <= area[3]:
                x = area[0] - d[0]
                while x + d[2] <= area[2]:
                    box = (x + d[0], y + d[1], x + d[2], y + d[3])
                    if box[0] >= area[0] and box[2] <= area[2] and \\
                       box[1] >= area[1] and box[3] <= area[3] and \\
                       not any(_overlap(box, o, PACK_GAP) for o in obstacles) and \\
                       (need <= 0.0 or _has_escape(box, obstacles, need)):
                        cx, cy = (box[0] + box[2]) / 2, (box[1] + box[3]) / 2
                        cost = (cx - ax) ** 2 + (cy - ay) ** 2
                        if best_cost is None or cost < best_cost:
                            best, best_cost = (x, y, box), cost
                    x += 0.5
                y += 0.5
            if best is not None:
                break
        if best is None:
            raise RuntimeError("no free slot for %s" % ref)"""
assert old in s
s = s.replace(old, new)
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
