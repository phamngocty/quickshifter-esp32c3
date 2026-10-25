p="build_pcb.py"; s=open(p,encoding="utf-8").read()
s=s.replace("""        result = None
        for cl, w in ((CLEARANCE, width), (0.21, width * 0.7),
                      (0.21, max(width * 0.5, 0.4)), (0.21, 0.3)):
            grid = build_grid_for(net, w, obstacles, routed, cl, holes)
            try:
                result = (w, route_net(grid, per_net[net], net, w, clearance=cl,
                                       holes=holes))
                break
            except RuntimeError:
                continue
        if result is None:
            print("  !! router failed on net %s" % net)""",
"""        result = None
        last = None
        for cl, w in ((CLEARANCE, width), (0.21, width * 0.7),
                      (0.21, max(width * 0.5, 0.4)), (0.21, 0.3)):
            grid = build_grid_for(net, w, obstacles, routed, cl, holes)
            try:
                result = (w, route_net(grid, per_net[net], net, w, clearance=cl,
                                       holes=holes))
                break
            except RuntimeError as exc:
                last = exc
                continue
        if result is None:
            print("  !! %s" % last)""")
open(p,"w",encoding="utf-8",newline="\n").write(s); print("ok")
