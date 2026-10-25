p="build_pcb.py"; s=open(p,encoding="utf-8").read()
s=s.replace('''    "J4": (160.0, 133.0, 0),     # TEST_PWM header
    "J5": (134.0, 135.0, 90),    # spare IO header (below U3, short hops)
}''','''    "J4": (160.0, 133.0, 0),     # TEST_PWM header
}

# Kept clear of parts so the router has real corridors to use. Without these
# the auto-packer fills every gap and a 2-layer board runs out of room.
KEEPOUT_CHANNELS = [
    (133.0, 131.5, 162.0, 137.0),   # horizontal channel under the module
    (157.5, 106.0, 162.5, 131.0),   # vertical channel right of the module
    (100.5, 110.5, 108.5, 137.0),   # vertical channel down the left edge
]''')
s=s.replace('''    for (hx, hy) in MOUNTING_HOLES:
        obstacles.append((hx - HOLE_KEEPOUT, hy - HOLE_KEEPOUT,
                          hx + HOLE_KEEPOUT, hy + HOLE_KEEPOUT))

    area = (BX0 + 0.5''','''    for (hx, hy) in MOUNTING_HOLES:
        obstacles.append((hx - HOLE_KEEPOUT, hy - HOLE_KEEPOUT,
                          hx + HOLE_KEEPOUT, hy + HOLE_KEEPOUT))
    obstacles.extend(KEEPOUT_CHANNELS)

    area = (BX0 + 0.5''')
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
