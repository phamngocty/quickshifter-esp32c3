p="build_pcb.py"; s=open(p,encoding="utf-8").read()

# (a) only drop genuinely duplicate vias: dropping merely-close ones broke nets
s = s.replace("< 1.1 ** 2 for k in kept", "< 0.30 ** 2 for k in kept")

# (b) the GND stub tracks must also be obstacles for later nets
s = s.replace("""            box = (vx - VIA_D / 2, vy - VIA_D / 2, vx + VIA_D / 2, vy + VIA_D / 2)
            for layer in (0, 1):
                added.append((box, VIA_D, layer))""",
"""            box = (vx - VIA_D / 2, vy - VIA_D / 2, vx + VIA_D / 2, vy + VIA_D / 2)
            for layer in (0, 1):
                added.append((box, VIA_D, layer))
            for (x1, y1, x2, y2, layer, w) in tracks:
                added.append(((min(x1, x2), min(y1, y2), max(x1, x2), max(y1, y2)), w, layer))""")

# (c) big SMD pads (the TO-263 tab is 9.4 x 10.8) need a longer search
s = s.replace("            for r in (1.2, 1.6, 2.0, 2.4, 3.0, 3.6):",
"""            reach = max(bb.GetWidth(), bb.GetHeight()) / 2e6 + 2.5
            radii = [1.2 + 0.0 * i for i in range(1)]
            r = 1.2
            while r <= reach:
                radii.append(r)
                r += 0.4
            for r in radii:""")
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
