"""
build_pcb.py - Places and routes the Quickshifter ESP32-C3 (IGN) board.

Run with KiCad's bundled interpreter (it ships the `pcbnew` module):

    "C:\\Program Files\\KiCad\\10.0\\bin\\python.exe" build_pcb.py

Pipeline
    netlist (.net)        -> net assignment
    BIG[] + auto-pack     -> footprint placement (small parts snap to the free
                             slot nearest their schematic anchor)
    GND pour both layers  -> GND never needs routing
    grid A* router        -> everything else; the obstacle map is rebuilt per
                             net so that wide nets get wide clearance and vias
                             are only dropped where they physically fit
"""

from __future__ import annotations

import heapq
import math
import os
import sys

import pcbnew

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.dirname(HERE)
PROJ = os.path.join(ROOT, "quickshifter-c3-ign")
NAME = "quickshifter-c3-ign"

KICAD_FP = r"C:\Program Files\KiCad\10.0\share\kicad\footprints"
QS_FP = os.path.join(PROJ, "quickshifter.pretty")
QS_LIB = "QS"

BX0, BY0, BX1, BY1 = 100.0, 100.0, 182.0, 137.0
EDGE_KEEPOUT = 0.6

# ---- floor plan: large parts are hand placed ---------------------------------
BIG = {
    # The three terminal blocks lie along the left edge. Standing them up
    # sideways trades 35 mm of scarce board *width* for 36 mm of the 37 mm
    # height, which is what finally made this board routable at 82 x 37 mm.
    "J1": (105.5, 103.0, 270),   # bike: +12V / GND / RPM
    "J2": (105.5, 115.5, 270),   # SN04-N shift sensor
    "J3": (105.5, 127.5, 270),   # cut trigger wire (J3.1 <-> J3.2)
    "U3": (145.0, 118.0, 0),     # ESP32-C3 Super Mini
    "U1": (167.0, 108.0, 0),     # LM2596 buck
    "U2": (120.0, 130.0, 0),     # PC817 RPM opto
    "R1": (120.0, 124.0, 0),     # 4k7 axial, RPM input
    "L1": (176.0, 120.0, 0),     # 100 uH
    "C1": (113.0, 112.0, 0),     # 100 uF/25V bulk
    "C3": (113.0, 126.0, 0),     # 220 uF/16V
    "Q1": (168.0, 132.0, 0),     # IRGB14C40L ignition IGBT
    "J4": (158.0, 132.0, 0),     # TEST_PWM header
}

# Kept clear of parts so the router has real corridors to use.
KEEPOUT_CHANNELS = [
    (133.0, 130.5, 152.0, 137.0),   # channel under the module
    (155.0, 112.0, 160.0, 137.0),   # channel right of the module
]

SMALL_ANCHOR = {
    "F1": (152.0, 104.0), "F2": (158.0, 104.0), "D1": (152.0, 110.0),
    "D2": (158.0, 110.0), "C2": (132.0, 104.0), "C4": (176.0, 112.0),
    "D3": (160.0, 114.0), "C8": (176.0, 128.0),
    "D4": (128.0, 116.0), "R2": (126.0, 128.0), "C5": (132.0, 132.0),
    "R3": (138.0, 132.0),
    "R4": (128.0, 134.0), "R5": (134.0, 134.0), "D5": (140.0, 134.0),
    "C6": (146.0, 134.0),
    "R6": (162.0, 124.0), "R7": (155.0, 130.0), "D6": (164.0, 128.0),
    "C7": (174.0, 128.0),
}

MOUNTING_HOLES = [(116.0, 102.5), (178.0, 102.5), (114.0, 134.5), (178.0, 134.5)]
HOLE_KEEPOUT = 3.2

NET_WIDTH = {
    "GND": 0.5, "+12V_IN": 0.8, "+12V_F": 0.8, "+12V_P": 0.8, "+12V_SENS": 0.6,
    "SW_NODE": 1.2, "+5V": 1.0, "+3V3": 0.6, "TRIG": 2.0,
}
# Short local nets first: they get the pick of an empty board, wide power
# nets can always find a way round, and long signal nets come last.
ROUTE_ORDER = ["RPM_LED_A", "RPM_OPT", "RPM_SIG", "RPM_IN",
               "SHIFT_IN", "SHIFT_SIG", "IGBT_G", "IGN_DRV",
               "TEST_PWM", "TEST_PWM_OUT", "+3V3",
               "TRIG", "SW_NODE", "+5V", "+12V_IN", "+12V_F", "+12V_P",
               "+12V_SENS"]
DEFAULT_WIDTH = 0.4

GRID = 0.2
CLEARANCE = 0.21   # > the board's 0.2 mm default rule
VIA_D = 0.6
VIA_DRILL = 0.3
RASTER = 0.10      # block cells whose *centre* is outside the rect too

# Minimum gap between two placed parts. Anything below roughly 2x (track
# half-width + clearance) leaves the pads' keep-outs touching, which seals a
# pad inside a pocket that no router can escape.
PACK_GAP = 0.9
ESCAPE = 1.7       # every part needs at least one side this free, so its
                   # pads can actually be reached by a track


# --------------------------------------------------------------------------
# footprints / placement
# --------------------------------------------------------------------------

def fp_path(libid):
    lib, name = libid.split(":", 1)
    if lib == QS_LIB:
        return QS_FP, name
    return os.path.join(KICAD_FP, lib + ".pretty"), name


def body_box_rel(libid):
    lib, name = fp_path(libid)
    fp = pcbnew.FootprintLoad(lib, name)
    if fp is None:
        raise RuntimeError("cannot load footprint %s" % libid)
    fp.SetPosition(pcbnew.VECTOR2I_MM(0, 0))
    bb = fp.GetBoundingBox(False, False)
    return (bb.GetX() / 1e6, bb.GetY() / 1e6,
            (bb.GetX() + bb.GetWidth()) / 1e6, (bb.GetY() + bb.GetHeight()) / 1e6)


def _has_escape(box, obstacles, need=ESCAPE):
    """True if at least one side of `box` is open board (not another part).

    A part wedged on all four sides ends up with its pads' keep-outs merged
    into the neighbours' and no track can ever reach it.
    """
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


def _overlap(a, b, pad=0.0):
    return (a[0] - pad < b[2] and b[0] - pad < a[2]
            and a[1] - pad < b[3] and b[1] - pad < a[3])


def compute_placement(sheet_fp):
    placement = dict(BIG)
    obstacles = []
    for ref, (x, y, rot) in BIG.items():
        d = body_box_rel(sheet_fp[ref][0])
        obstacles.append((x + d[0], y + d[1], x + d[2], y + d[3]))
    for (hx, hy) in MOUNTING_HOLES:
        obstacles.append((hx - HOLE_KEEPOUT, hy - HOLE_KEEPOUT,
                          hx + HOLE_KEEPOUT, hy + HOLE_KEEPOUT))
    obstacles.extend(KEEPOUT_CHANNELS)

    area = (BX0 + 1.0, BY0 + 1.0, BX1 - 1.0, BY1 - 1.0)
    smalls = [r for r in sheet_fp if r not in BIG and not r.startswith("#")
              and sheet_fp[r][0]]
    rel = {ref: body_box_rel(sheet_fp[ref][0]) for ref in smalls}

    for ref in sorted(smalls, key=lambda r: -(
            (rel[r][2] - rel[r][0]) * (rel[r][3] - rel[r][1]))):
        d = rel[ref]
        ax, ay = SMALL_ANCHOR.get(ref, ((area[0] + area[2]) / 2, (area[1] + area[3]) / 2))
        best, best_cost = None, None
        for need in (ESCAPE, 1.2, 0.0):
            y = area[1] - d[1]
            while y + d[3] <= area[3]:
                x = area[0] - d[0]
                while x + d[2] <= area[2]:
                    box = (x + d[0], y + d[1], x + d[2], y + d[3])
                    if box[0] >= area[0] and box[2] <= area[2] and \
                       box[1] >= area[1] and box[3] <= area[3] and \
                       not any(_overlap(box, o, PACK_GAP) for o in obstacles) and \
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
            raise RuntimeError("no free slot for %s" % ref)
        x, y, box = best
        placement[ref] = (round(x, 2), round(y, 2), 0)
        obstacles.append(box)
    return placement


# --------------------------------------------------------------------------
# board
# --------------------------------------------------------------------------

def seg(board, x1, y1, x2, y2, layer=pcbnew.Edge_Cuts, width=0.1):
    s = pcbnew.PCB_SHAPE(board)
    s.SetShape(pcbnew.SHAPE_T_SEGMENT)
    s.SetStart(pcbnew.VECTOR2I_MM(x1, y1))
    s.SetEnd(pcbnew.VECTOR2I_MM(x2, y2))
    s.SetLayer(layer)
    s.SetWidth(pcbnew.FromMM(width))
    board.Add(s)


def parse_netlist(path):
    from kicadgen import sexp_parse, head, child, _unquote
    tree = sexp_parse(open(path, encoding="utf-8").read())[0]
    nets_node = next((n for n in tree if isinstance(n, list) and head(n) == "nets"), None)
    if nets_node is None:
        raise RuntimeError("no (nets ...) block in %s" % path)
    out = {}
    for net in nets_node:
        if not isinstance(net, list) or head(net) != "net":
            continue
        nm = child(net, "name")
        if nm is None:
            continue
        nodes = []
        for n in net:
            if isinstance(n, list) and head(n) == "node":
                r, p = child(n, "ref"), child(n, "pin")
                if r and p:
                    nodes.append((_unquote(r[1]), _unquote(p[1])))
        if nodes:
            name = _unquote(nm[1])
            # local nets carry the sheet path ("/TRIG"); power nets do not.
            out[name.lstrip("/")] = nodes
    return out


def build_board(nets, sheet_fp, placement):
    board = pcbnew.CreateEmptyBoard()
    board.GetDesignSettings().SetBoardThickness(pcbnew.FromMM(1.6))

    for a, b in (((BX0, BY0), (BX1, BY0)), ((BX1, BY0), (BX1, BY1)),
                 ((BX1, BY1), (BX0, BY1)), ((BX0, BY1), (BX0, BY0))):
        seg(board, a[0], a[1], b[0], b[1])

    lib, name = fp_path("MountingHole:MountingHole_3.2mm_M3")
    for i, (hx, hy) in enumerate(MOUNTING_HOLES, start=1):
        fp = pcbnew.FootprintLoad(lib, name)
        fp.SetReference("H%d" % i)
        fp.SetPosition(pcbnew.VECTOR2I_MM(hx, hy))
        board.Add(fp)

    netmap = {}
    for nname in nets:
        ni = pcbnew.NETINFO_ITEM(board, nname)
        board.Add(ni)
        netmap[nname] = ni

    pad_net = {}
    for nname, nodes in nets.items():
        for ref, pad in nodes:
            pad_net[(ref, pad)] = nname

    placed = {}
    for ref, (x, y, rot) in placement.items():
        libid, value = sheet_fp[ref]
        if not libid:
            continue
        lib, fname = fp_path(libid)
        fp = pcbnew.FootprintLoad(lib, fname)
        if fp is None:
            raise RuntimeError("cannot load footprint %s" % libid)
        fp.SetReference(ref)
        fp.SetValue(value)
        fp.SetPosition(pcbnew.VECTOR2I_MM(x, y))
        fp.SetOrientationDegrees(rot)
        for pad in fp.Pads():
            key = (ref, pad.GetNumber())
            if key in pad_net:
                pad.SetNet(netmap[pad_net[key]])
        body = fp.GetBoundingBox(False, False)
        rt = fp.Reference()
        rt.SetTextSize(pcbnew.VECTOR2I_MM(0.8, 0.8))
        rt.SetTextThickness(pcbnew.FromMM(0.13))
        rt.SetPosition(pcbnew.VECTOR2I(int(body.GetX() + body.GetWidth() / 2.0),
                                       int(body.GetY() - pcbnew.FromMM(1.1))))
        board.Add(fp)
        placed[ref] = fp
    return board, placed, netmap


def add_gnd_pad_vias(board, placed, netmap, obstacles, holes, clearance):
    """Route every SMD GND pad to its own via into the bottom-side ground pour.

    There is no top-side pour (it fragmented into unreachable islands), so an
    SMD GND pad has no path to ground unless we make one. Both the via site and
    the little connecting track are found on a grid that already knows about
    every other net's copper, so nothing gets shorted.
    """
    g = build_grid_for("GND", 0.4, obstacles, [], clearance, holes)
    net = netmap["GND"]
    added = []
    for ref, fp in placed.items():
        for pad in fp.Pads():
            if pad.GetNetname() != "GND":
                continue
            if pad.GetAttribute() == pcbnew.PAD_ATTRIB_PTH:
                continue                      # barrel already reaches B.Cu
            bb = pad.GetBoundingBox()
            cx = (bb.GetX() + bb.GetWidth() / 2.0) / 1e6
            cy = (bb.GetY() + bb.GetHeight() / 2.0) / 1e6
            src = (g.gx(cx), g.gy(cy))
            if not (g.free(0, src[0], src[1]) and g.free(1, src[0], src[1])):
                continue
            target = None
            reach = max(bb.GetWidth(), bb.GetHeight()) / 2e6 + 2.5
            radii = [1.2 + 0.0 * i for i in range(1)]
            r = 1.2
            while r <= reach:
                radii.append(r)
                r += 0.4
            for r in radii:
                for k in range(24):
                    import math as _m
                    ang = 2 * _m.pi * k / 24.0
                    ix, iy = g.gx(cx + r * _m.cos(ang)), g.gy(cy + r * _m.sin(ang))
                    if not (g.free(0, ix, iy) and g.free(1, ix, iy)):
                        continue
                    if not _via_fits(g, ix, iy, 2) or _in_hole(g, ix, iy, holes):
                        continue
                    target = (ix, iy)
                    break
                if target:
                    break
            if target is None:
                print("    (no GND via site for %s pad %s)" % (ref, pad.GetNumber()))
                continue
            path = astar(g, [(0, src[0], src[1])], {(0, target[0], target[1])},
                         16.0, 0, holes)
            if path is None:
                continue
            tracks, _ = path_to_geometry(g, path, 0.4)
            vx, vy = g.to_mm(target[0], target[1])
            v = pcbnew.PCB_VIA(board)
            v.SetPosition(pcbnew.VECTOR2I_MM(vx, vy))
            v.SetWidth(pcbnew.FromMM(VIA_D))
            v.SetDrill(pcbnew.FromMM(VIA_DRILL))
            v.SetViaType(pcbnew.VIATYPE_THROUGH)
            v.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu)
            v.SetNet(net)
            board.Add(v)
            for (x1, y1, x2, y2, layer, w) in tracks:
                t = pcbnew.PCB_TRACK(board)
                t.SetStart(pcbnew.VECTOR2I_MM(x1, y1))
                t.SetEnd(pcbnew.VECTOR2I_MM(x2, y2))
                t.SetWidth(pcbnew.FromMM(w))
                t.SetLayer(pcbnew.F_Cu if layer == 0 else pcbnew.B_Cu)
                t.SetNet(net)
                board.Add(t)
            box = (vx - VIA_D / 2, vy - VIA_D / 2, vx + VIA_D / 2, vy + VIA_D / 2)
            for layer in (0, 1):
                added.append((box, VIA_D, layer))
            for (x1, y1, x2, y2, layer, w) in tracks:
                added.append(((min(x1, x2), min(y1, y2), max(x1, x2), max(y1, y2)), w, layer))
            g.block(0, box[0] - 0.2, box[1] - 0.2, box[2] + 0.2, box[3] + 0.2)
            g.block(1, box[0] - 0.2, box[1] - 0.2, box[2] + 0.2, box[3] + 0.2)
    return added


def add_gnd_stitching(board, gnd_net):
    """Drop a via inside every F.Cu GND island.

    The top-side pour gets cut into islands by the routing. Any island that
    still touches a GND pad is not "isolated" so KiCad will not remove it,
    yet it is not connected to the rest of GND either. A via placed *inside*
    the filled polygon is always clear of other nets (the pour already keeps
    clearance), and ties the island to the bottom-side pour.
    """
    added = 0
    for zone in board.Zones():
        if zone.GetLayer() != pcbnew.F_Cu:
            continue
        poly = zone.GetFilledPolysList(pcbnew.F_Cu)
        for i in range(poly.OutlineCount()):
            outline = poly.Outline(i)
            bb = outline.BBox()
            cx = (bb.GetX() + bb.GetWidth() / 2.0)
            cy = (bb.GetY() + bb.GetHeight() / 2.0)
            if bb.GetWidth() < pcbnew.FromMM(1.2) or bb.GetHeight() < pcbnew.FromMM(1.2):
                continue
            pt = pcbnew.VECTOR2I(int(cx), int(cy))
            if not poly.Contains(pt):
                continue
            v = pcbnew.PCB_VIA(board)
            v.SetPosition(pt)
            v.SetWidth(pcbnew.FromMM(0.6))
            v.SetDrill(pcbnew.FromMM(0.3))
            v.SetViaType(pcbnew.VIATYPE_THROUGH)
            v.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu)
            v.SetNet(gnd_net)
            board.Add(v)
            added += 1
    return added


def add_gnd_zones(board, gnd_net):
    for layer in (pcbnew.B_Cu,):
        z = pcbnew.ZONE(board)
        z.SetLayer(layer)
        z.SetNet(gnd_net)
        z.SetLocalClearance(pcbnew.FromMM(0.3))
        z.SetMinThickness(pcbnew.FromMM(0.25))
        z.SetPadConnection(pcbnew.ZONE_CONNECTION_THERMAL)
        z.SetThermalReliefGap(pcbnew.FromMM(0.3))
        z.SetThermalReliefSpokeWidth(pcbnew.FromMM(0.6))
        z.SetIsFilled(True)
        o = z.Outline()
        o.NewOutline()
        for px, py in ((BX0 + 0.4, BY0 + 0.4), (BX1 - 0.4, BY0 + 0.4),
                       (BX1 - 0.4, BY1 - 0.4), (BX0 + 0.4, BY1 - 0.4)):
            o.Append(pcbnew.FromMM(px), pcbnew.FromMM(py))
        board.Add(z)


# --------------------------------------------------------------------------
# router
# --------------------------------------------------------------------------

class Grid:
    def __init__(self, x0, y0, x1, y1, pitch):
        self.x0, self.y0, self.p = x0, y0, pitch
        self.nx = int((x1 - x0) / pitch) + 1
        self.ny = int((y1 - y0) / pitch) + 1
        self.blocked = [bytearray(self.nx * self.ny) for _ in range(2)]

    def gx(self, x):
        return int(round((x - self.x0) / self.p))

    def gy(self, y):
        return int(round((y - self.y0) / self.p))

    def to_mm(self, ix, iy):
        return (round(self.x0 + ix * self.p, 3), round(self.y0 + iy * self.p, 3))

    def idx(self, ix, iy):
        return iy * self.nx + ix

    def inside(self, ix, iy):
        return 0 <= ix < self.nx and 0 <= iy < self.ny

    def _rect(self, layer, x0, y0, x1, y1, value):
        # Inflate by RASTER: a cell is only blocked by its centre, so without
        # this a track one cell outside the rect can still be too close.
        x0, y0, x1, y1 = x0 - RASTER, y0 - RASTER, x1 + RASTER, y1 + RASTER
        ix0, ix1 = self.gx(min(x0, x1)), self.gx(max(x0, x1))
        iy0, iy1 = self.gy(min(y0, y1)), self.gy(max(y0, y1))
        b = self.blocked[layer]
        for iy in range(max(0, iy0), min(self.ny, iy1 + 1)):
            base = iy * self.nx
            for ix in range(max(0, ix0), min(self.nx, ix1 + 1)):
                b[base + ix] = value

    def block(self, layer, x0, y0, x1, y1):
        self._rect(layer, x0, y0, x1, y1, 1)

    def unblock(self, layer, x0, y0, x1, y1):
        self._rect(layer, x0, y0, x1, y1, 0)

    def free(self, layer, ix, iy):
        return 0 <= ix < self.nx and 0 <= iy < self.ny \
            and not self.blocked[layer][self.idx(ix, iy)]


def astar(grid, starts, goals, via_penalty, via_radius, holes=()):
    """Multi-source A* over (layer, ix, iy).

    Growing the tree from *all* already-connected pads at once and stopping at
    the first unreached pad turns each merge step into a single search. Trying
    every (source, goal) pair instead is O(n^2) searches and does not finish
    for a net like GND with forty pads.

    `starts` and `goals` are iterables of (ix, iy).
    """
    goals = set(goals)
    probes = list({(g[1], g[2]) for g in goals})[:6]

    def h(x, y):
        return min(abs(x - gx) + abs(y - gy) for (gx, gy) in probes)

    openq = []
    came = {}
    g = {}
    for (sl, sx, sy) in starts:
        node = (sl, sx, sy)
        g[node] = 0.0
        came[node] = None
        heapq.heappush(openq, (h(sx, sy), node))
    while openq:
        _, cur = heapq.heappop(openq)
        if cur in goals:
            path = []
            n = cur
            while n is not None:
                path.append(n)
                n = came[n]
            return path[::-1]
        cl, cx, cy = cur
        base = g[cur]
        for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
            x2, y2 = cx + dx, cy + dy
            if grid.free(cl, x2, y2):
                nxt = (cl, x2, y2)
                ng = base + 1.0
                if ng < g.get(nxt, 1e18):
                    g[nxt] = ng
                    came[nxt] = cur
                    heapq.heappush(openq, (ng + h(x2, y2), nxt))
        other = 1 - cl
        if _via_fits(grid, cx, cy, via_radius) and not _in_hole(grid, cx, cy, holes):
            nxt = (other, cx, cy)
            ng = base + via_penalty
            if ng < g.get(nxt, 1e18):
                g[nxt] = ng
                came[nxt] = cur
                heapq.heappush(openq, (ng + h(cx, cy), nxt))
    return None


def _in_hole(grid, ix, iy, holes):
    """A via must never land inside another part's drilled hole."""
    x, y = grid.to_mm(ix, iy)
    for (hx, hy, hw, hh) in holes:
        if abs(x - hx) <= hw + 0.35 and abs(y - hy) <= hh + 0.35:
            return True
    return False


def _via_fits(grid, ix, iy, r):
    for dy in range(-r, r + 1):
        for dx in range(-r, r + 1):
            if not (grid.inside(ix + dx, iy + dy)
                    and not grid.blocked[0][grid.idx(ix + dx, iy + dy)]
                    and not grid.blocked[1][grid.idx(ix + dx, iy + dy)]):
                return False
    return True


def path_to_geometry(grid, path, width):
    tracks, vias = [], []
    layer = path[0][0]
    run_start = prev = path[0]
    direction = None

    def emit(a, b, lay):
        pa, pb = grid.to_mm(a[1], a[2]), grid.to_mm(b[1], b[2])
        if pa != pb:
            tracks.append((pa[0], pa[1], pb[0], pb[1], lay, width))

    for node in path[1:]:
        if node[0] != layer:
            emit(run_start, prev, layer)
            vias.append((grid.to_mm(prev[1], prev[2]), layer, node[0]))
            layer, run_start, prev, direction = node[0], node, node, None
            continue
        d = (node[1] - prev[1], node[2] - prev[2])
        if direction is not None and d != direction:
            emit(run_start, prev, layer)
            run_start = prev
        direction, prev = d, node
    emit(run_start, prev, layer)
    return tracks, vias


def route_net(grid, own_boxes, net, width, clearance, via_penalty=400.0, holes=()):
    """Route every pad of one net on a grid already blocked for this net.

    The search is layer aware. Starting every follow-up search on F.Cu looks
    harmless but is not: if the previous merge arrived on B.Cu with no via at
    that cell, the new search begins on copper that is not actually joined to
    it, and DRC reports a dangling track.

    This net's own pads are deliberately left unblocked by `build_grid_for`,
    and its own copper is never reserved in the grid (overlapping copper on the
    same net is harmless).
    """
    if len(own_boxes) < 2:
        return [], []
    nodes = {}
    for (px, py, hw, hh, layers) in own_boxes:
        cell = (grid.gx(px), grid.gy(py))
        for lay in layers:
            nodes.setdefault(cell, set()).add(lay)
    cells = list(nodes)
    tree = [(lay, cells[0][0], cells[0][1]) for lay in sorted(nodes[cells[0]])]
    remaining = set()
    for cell in cells[1:]:
        for lay in nodes[cell]:
            remaining.add((lay, cell[0], cell[1]))

    tracks, vias = [], []
    via_r = max(1, int((VIA_D / 2.0 + clearance - RASTER) / GRID))

    while remaining:
        path = astar(grid, tree, remaining, via_penalty, via_r, holes)
        if path is None:
            left = sorted(grid.to_mm(n[1], n[2]) for n in remaining)
            raise RuntimeError("router failed on net %s; unreached %s" % (net, left))
        last = path[-1]
        geom = path_to_geometry(grid, path, width)
        tracks.extend(geom[0])
        vias.extend(geom[1])
        tree.append(last)
        for (pt, la, lb) in geom[1]:
            if grid.gx(pt[0]) == last[1] and grid.gy(pt[1]) == last[2]:
                tree.append((1 - last[0], last[1], last[2]))
        remaining.discard(last)
        for node in [n for n in remaining if (n[1], n[2]) == (last[1], last[2])]:
            remaining.discard(node)
            tree.append(node)
    return tracks, vias


def collect_pads(placed):
    """(netname -> pads), obstacle boxes, and drilled-hole boxes."""
    per_net, obstacles, holes = {}, [], []
    for ref, fp in placed.items():
        for pad in fp.Pads():
            # Use the pad's real bounding box: GetSize() ignores a pad that is
            # rotated inside its footprint (e.g. the SRN8040 inductor pads are
            # 6.6 x 2.85 by size but stand upright on the board), and trusting
            # it made the router lay copper straight across the pad.
            ls = pad.GetLayerSet()
            if not (ls.Contains(pcbnew.F_Cu) or ls.Contains(pcbnew.B_Cu)):
                continue   # paste/aperture-only pad: no copper, so not an obstacle
            bb = pad.GetBoundingBox()
            x = (bb.GetX() + bb.GetWidth() / 2.0) / 1e6
            y = (bb.GetY() + bb.GetHeight() / 2.0) / 1e6
            hw, hh = bb.GetWidth() / 2e6, bb.GetHeight() / 2e6
            net = pad.GetNetname()
            box = (x - hw, y - hh, x + hw, y + hh)
            layers = (0, 1) if pad.GetAttribute() == pcbnew.PAD_ATTRIB_PTH else (0,)
            obstacles.append((box, net, layers))
            drill = pad.GetDrillSize()
            if drill.x > 0:
                holes.append((x, y, drill.x / 2e6, drill.y / 2e6))
            if net:
                per_net.setdefault(net, []).append((x, y, hw, hh, layers))
    return per_net, obstacles, holes


def build_grid_for(net, width, obstacles, routed, clearance, holes=()):
    """Obstacle map for one net: foreign copper inflated by this net's width."""
    g = Grid(BX0, BY0, BX1, BY1, GRID)
    reach = width / 2.0 + clearance
    edge = EDGE_KEEPOUT + width / 2
    g.block(0, BX0, BY0, BX1, BY0 + edge)
    g.block(0, BX0, BY1 - edge, BX1, BY1)
    g.block(0, BX0, BY0, BX0 + edge, BY1)
    g.block(0, BX1 - edge, BY0, BX1, BY1)
    g.block(1, BX0, BY0, BX1, BY0 + edge)
    g.block(1, BX0, BY1 - edge, BX1, BY1)
    g.block(1, BX0, BY0, BX0 + edge, BY1)
    g.block(1, BX1 - edge, BY0, BX1, BY1)
    for (hx, hy) in MOUNTING_HOLES:
        for layer in (0, 1):
            g.block(layer, hx - 2.6 - width / 2, hy - 2.6 - width / 2,
                    hx + 2.6 + width / 2, hy + 2.6 + width / 2)
    for (box, bnet, layers) in obstacles:
        if bnet == net:
            continue
        for layer in layers:
            g.block(layer, box[0] - reach, box[1] - reach,
                    box[2] + reach, box[3] + reach)
    for (box, w, layer) in routed:
        g.block(layer, box[0] - w / 2 - clearance - width / 2,
                box[1] - w / 2 - clearance - width / 2,
                box[2] + w / 2 + clearance + width / 2,
                box[3] + w / 2 + clearance + width / 2)
    return g


def main():
    nets = parse_netlist(os.path.join(PROJ, NAME + ".net"))
    sys.path.insert(0, HERE)
    from build_board import PARTS
    sheet_fp = {p[0]: (p[3], p[2]) for p in PARTS}

    placement = compute_placement(sheet_fp)
    board, placed, netmap = build_board(nets, sheet_fp, placement)
    add_gnd_zones(board, netmap["GND"])
    per_net, obstacles, holes = collect_pads(placed)

    order = ROUTE_ORDER + [n for n in sorted(per_net)
                           if n not in ROUTE_ORDER and n != "GND"
                           and not n.startswith("unconnected-")]
    routed = []
    failed = []
    for box, w, layer in add_gnd_pad_vias(board, placed, netmap, obstacles, holes,
                                          CLEARANCE):
        routed.append((box, w, layer))

    for net in order:
        if net not in per_net:
            continue
        width = NET_WIDTH.get(net, DEFAULT_WIDTH)
        # Retry ladder: a congested board sometimes only closes with a slightly
        # tighter rule. 0.21 mm still clears the board's 0.2 mm default rule.
        result = None
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
            print("  !! %s" % last)
            failed.append(net)
            continue
        w, (tracks, vias) = result
        for (x1, y1, x2, y2, layer, tw) in tracks:
            t = pcbnew.PCB_TRACK(board)
            t.SetStart(pcbnew.VECTOR2I_MM(x1, y1))
            t.SetEnd(pcbnew.VECTOR2I_MM(x2, y2))
            t.SetWidth(pcbnew.FromMM(tw))
            t.SetLayer(pcbnew.F_Cu if layer == 0 else pcbnew.B_Cu)
            t.SetNet(netmap[net])
            board.Add(t)
            routed.append(((min(x1, x2), min(y1, y2), max(x1, x2), max(y1, y2)), tw, layer))
        kept = []
        for (pt, la, lb) in vias:
            if any((pt[0] - k[0]) ** 2 + (pt[1] - k[1]) ** 2 < 0.30 ** 2 for k in kept):
                continue
            kept.append(pt)
            v = pcbnew.PCB_VIA(board)
            v.SetPosition(pcbnew.VECTOR2I_MM(pt[0], pt[1]))
            v.SetWidth(pcbnew.FromMM(VIA_D))
            v.SetDrill(pcbnew.FromMM(VIA_DRILL))
            v.SetViaType(pcbnew.VIATYPE_THROUGH)
            v.SetLayerPair(pcbnew.F_Cu, pcbnew.B_Cu)
            v.SetNet(netmap[net])
            board.Add(v)
            for layer in (0, 1):
                routed.append(((pt[0] - VIA_D / 2, pt[1] - VIA_D / 2,
                                pt[0] + VIA_D / 2, pt[1] + VIA_D / 2), VIA_D, layer))
        print("  routed %-12s %2d tracks %2d vias  w=%.1f" % (net, len(tracks), len(vias), w))

    filler = pcbnew.ZONE_FILLER(board)
    filler.Fill(board.Zones())
    out = os.path.join(PROJ, NAME + ".kicad_pcb")
    pcbnew.SaveBoard(out, board)
    print("wrote", out, " footprints:", len(placed),
          " failed nets:", failed if failed else "none")


if __name__ == "__main__":
    main()

