"""
kicadsch.py - Small KiCad 9/10 schematic builder used to generate the
Quickshifter ESP32-C3 board.

Design choice: every component pin is terminated with a short stub plus either
a net label or a power symbol. Generation is then purely local (no routing
puzzle), deterministic, and ERC sees exactly the netlist we intended.

Run the builder, then validate with:
    kicad-cli sch erc --output erc.rpt board.kicad_sch
"""

from __future__ import annotations

import math
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from kicadgen import (SymbolLib, collect_pins, head, num, pin_position,
                      sexp_dump, uid, _rename_units, _unquote)

PROJECT = "quickshifter-c3-ign"
ROOT_UUID = "b1c3d0e1-0001-4a00-9000-000000000001"

POWER = {"GND": "power:GND", "+5V": "power:+5V", "+3V3": "power:+3V3"}
POWER_PIN_ANGLE = {"power:GND": 270.0, "power:+5V": 90.0, "power:+3V3": 90.0}

GRID = 1.27  # KiCad's default connection grid (50 mil)


def snap(v: float) -> float:
    """Snap to the connection grid so ERC does not report endpoint_off_grid."""
    return round(round(v / GRID) * GRID, 4)


def esc(s: str) -> str:
    return str(s).replace("\\", "\\\\").replace('"', '\\"')


# --------------------------------------------------------------------------
# Custom symbols
# --------------------------------------------------------------------------

def esp32_symbol() -> list:
    """ESP32-C3 Super Mini module, DIP-style numbering (1..8 left, 9..16 right)."""
    left = [("1", "5V", "power_in"), ("2", "GND", "power_in"), ("3", "3V3", "power_out"),
            ("4", "IO0", "bidirectional"), ("5", "IO1", "bidirectional"),
            ("6", "IO2", "bidirectional"), ("7", "IO3", "bidirectional"),
            ("8", "IO4", "bidirectional")]
    right = [("9", "IO5", "bidirectional"), ("10", "IO6", "bidirectional"),
             ("11", "IO7", "bidirectional"), ("12", "IO8", "bidirectional"),
             ("13", "IO9", "bidirectional"), ("14", "IO10", "bidirectional"),
             ("15", "IO20", "bidirectional"), ("16", "IO21", "bidirectional")]

    def pin(number, name, etype, x, y, angle):
        return ["pin", etype, "line", ["at", num(x), num(y), num(angle)], ["length", "2.54"],
                ["name", '"%s"' % name, ["effects", ["font", ["size", "1.27", "1.27"]]]],
                ["number", '"%s"' % number, ["effects", ["font", ["size", "1.27", "1.27"]]]]]

    unit1 = ["symbol", '"ESP32-C3-SuperMini_1_1"']
    y = 8.89
    for number, name, etype in left:
        unit1.append(pin(number, name, etype, -12.7, y, 0.0))
        y -= 2.54
    y = -8.89
    for number, name, etype in right:
        unit1.append(pin(number, name, etype, 12.7, y, 180.0))
        y += 2.54

    return ["symbol", '"QS:ESP32-C3-SuperMini"',
            ["exclude_from_sim", "no"], ["in_bom", "yes"], ["on_board", "yes"],
            ["pin_names", ["offset", "0.762"]],
            ["property", '"Reference"', '"U"', ["at", "-10.16", "13.97", "0"],
             ["effects", ["font", ["size", "1.27", "1.27"]], ["justify", "left"]]],
            ["property", '"Value"', '"ESP32-C3-SuperMini"', ["at", "-10.16", "-13.97", "0"],
             ["effects", ["font", ["size", "1.27", "1.27"]], ["justify", "left"]]],
            ["property", '"Footprint"', '"QS:ESP32-C3-SuperMini"', ["at", "0", "0", "0"],
             ["effects", ["font", ["size", "1.27", "1.27"]], ["hide", "yes"]]],
            ["property", '"Datasheet"', '"https://www.espressif.com/sites/default/files/documentation/esp32-c3_datasheet_en.pdf"',
             ["at", "0", "0", "0"], ["effects", ["font", ["size", "1.27", "1.27"]], ["hide", "yes"]]],
            ["property", '"Description"', '"ESP32-C3 Super Mini module, 16 castellated pads at 2.54mm pitch"',
             ["at", "0", "0", "0"], ["effects", ["font", ["size", "1.27", "1.27"]], ["hide", "yes"]]],
            ["symbol", '"ESP32-C3-SuperMini_0_1"',
             ["rectangle", ["start", "-10.16", "11.43"], ["end", "10.16", "-11.43"],
              ["stroke", ["width", "0.254"], ["type", "default"]],
              ["fill", ["type", "background"]]]],
            unit1]


def igbt_symbol() -> list:
    """Ignition IGBT with TO-220 pinout: 1=G, 2=C, 3=E.

    KiCad derives a sub-unit's unit/style from its `<parent>_<unit>_<style>`
    name, so the inherited units are renamed along with the parent.
    """
    flat = SymbolLib.flatten("Transistor_IGBT", "IRG4PF50W")
    flat = _rename_units(flat, "IRG4PF50W", "IGBT_Ignition")
    flat[1] = '"QS:IGBT_Ignition"'
    for node in flat:
        if isinstance(node, list) and head(node) == "property":
            key = _unquote(node[1])
            if key == "Value":
                node[2] = '"IRGB14C40L"'
            elif key == "Footprint":
                node[2] = '"QS:TO-220_Standing"'
    return flat


# --------------------------------------------------------------------------
# Builder
# --------------------------------------------------------------------------

class Sch:
    def __init__(self, title="Quickshifter ESP32-C3", rev="v1.0", comments=()):
        self.title = title
        self.rev = rev
        self.comments = list(comments)
        self.libids: dict[str, list] = {}
        self.instances: list[list] = []
        self.wires: list[tuple] = []
        self.junctions: list[tuple] = []
        self.labels: list[tuple] = []
        self.texts: list[tuple] = []
        self.no_connects: list[tuple] = []
        self._pins: dict[str, dict[str, tuple]] = {}
        self._lib_of: dict[str, str] = {}
        self._angle: dict[str, float] = {}
        self._pwr = 0

    # -- library -----------------------------------------------------------
    def _ensure_lib(self, lib_id: str):
        if lib_id in self.libids:
            return
        if lib_id == "QS:ESP32-C3-SuperMini":
            self.libids[lib_id] = esp32_symbol()
        elif lib_id == "QS:IGBT_Ignition":
            self.libids[lib_id] = igbt_symbol()
        else:
            lib, name = lib_id.split(":", 1)
            self.libids[lib_id] = SymbolLib.flatten(lib, name)

    def _pins_of(self, lib_id: str):
        self._ensure_lib(lib_id)
        return collect_pins(self.libids[lib_id])

    # -- placement ---------------------------------------------------------
    def place(self, lib_id, ref, value, footprint, x, y, angle=0.0,
              dnp=False, hide_value=False):
        x, y = snap(x), snap(y)
        self._ensure_lib(lib_id)
        pins = self._pins_of(lib_id)
        self._pins[ref] = {p.number: pin_position(p.x, p.y, angle, None, x, y)
                           for p in pins}
        self._lib_of[ref] = lib_id
        self._angle[ref] = angle

        node = ["symbol", ["lib_id", '"%s"' % lib_id],
                ["at", num(x), num(y), num(angle)], ["unit", "1"],
                ["body_style", "1"],
                ["exclude_from_sim", "no"], ["in_bom", "yes"], ["on_board", "yes"],
                ["in_pos_files", "yes"],
                ["dnp", "yes" if dnp else "no"],
                ["uuid", '"%s"' % uid("sym:" + ref)]]
        props = [
            ("Reference", ref, (x + 2.54, y - 1.27), ref.startswith("#")),
            ("Value", value, (x + 2.54, y + 1.27), hide_value or ref.startswith("#")),
            ("Footprint", footprint, (x, y), True),
            ("Datasheet", "~", (x, y), True),
            ("Description", "", (x, y), True),
        ]
        for name, val, (px, py), hide in props:
            eff = ["effects", ["font", ["size", "1.27", "1.27"]]]
            if hide:
                eff.append(["hide", "yes"])
            node.append(["property", '"%s"' % name, '"%s"' % esc(val),
                         ["at", num(px), num(py), "0"], eff])
        for p in pins:
            node.append(["pin", '"%s"' % p.number,
                         ["uuid", '"%s"' % uid(f"pin:{ref}:{p.number}")]])
        node.append(["instances", ["project", '"%s"' % PROJECT,
                                   ["path", '"/%s"' % ROOT_UUID,
                                    ["reference", '"%s"' % ref], ["unit", "1"]]]])
        self.instances.append(node)
        return ref

    # -- geometry ----------------------------------------------------------
    def P(self, ref, pinno):
        return self._pins[ref][str(pinno)]

    def _pin_dir(self, ref, pinno):
        """Screen-space unit vector pointing away from the symbol body.

        KiCad's pin `angle` points from the connection point *into* the body,
        so the outward direction is angle + 180.
        """
        ang = self._angle[ref]
        for p in self._pins_of(self._lib_of[ref]):
            if p.number == str(pinno):
                a = math.radians(p.angle + 180.0 + ang)
                return (round(math.cos(a), 4), round(-math.sin(a), 4))
        raise KeyError(f"{ref}.{pinno}")

    def wire(self, *pts):
        for a, b in zip(pts, pts[1:]):
            a = (round(a[0], 3), round(a[1], 3))
            b = (round(b[0], 3), round(b[1], 3))
            if a != b:
                self.wires.append((a, b))

    def junction(self, *pts):
        self.junctions.extend((round(p[0], 3), round(p[1], 3)) for p in pts)

    def label(self, text, pt, angle=0.0):
        self.labels.append((text, (round(pt[0], 3), round(pt[1], 3)), angle))

    def text(self, s, pt, size=1.27):
        self.texts.append((s, (round(pt[0], 3), round(pt[1], 3)), size))

    def no_connect(self, ref, pinno):
        self.no_connects.append(self.P(ref, pinno))

    # -- pin termination ---------------------------------------------------
    def stub(self, ref, pinno, net, length=2.54):
        """Draw a stub off a pin and terminate it with `net` (label or power)."""
        px, py = self.P(ref, pinno)
        dx, dy = self._pin_dir(ref, pinno)
        end = (round(px + dx * length, 3), round(py + dy * length, 3))
        self.wire((px, py), end)
        self.terminate(net, end, (dx, dy))
        return end

    def terminate(self, net, pt, direction):
        if net in POWER:
            lib_id = POWER[net]
            base = POWER_PIN_ANGLE[lib_id]
            best, best_err = 0.0, 9e9
            for cand in (0.0, 90.0, 180.0, 270.0):
                r = math.radians(base + cand)
                v = (round(math.cos(r), 4), round(-math.sin(r), 4))
                err = (v[0] - direction[0]) ** 2 + (v[1] - direction[1]) ** 2
                if err < best_err:
                    best, best_err = cand, err
            self._pwr += 1
            self.place(lib_id, "#PWR%02d" % self._pwr, net, "", pt[0], pt[1], best)
        else:
            self.label(net, pt, 0.0 if direction[0] != 0 else 90.0)

    def power_net(self, ref, pinno, net):
        """Force a power-symbol termination regardless of stub direction."""
        px, py = self.P(ref, pinno)
        dx, dy = self._pin_dir(ref, pinno)
        end = (round(px + dx * 2.54, 3), round(py + dy * 2.54, 3))
        self.wire((px, py), end)
        self.terminate(net, end, (dx, dy))
        return end

    # -- emit --------------------------------------------------------------
    def emit(self, path):
        title_block = ["title_block", ["title", '"%s"' % esc(self.title)],
                       ["date", '"2026-10-10"'], ["rev", '"%s"' % esc(self.rev)]]
        for i, c in enumerate(self.comments):
            title_block.append(["comment", str(i + 1), '"%s"' % esc(c)])

        out = ["kicad_sch", ["version", "20260306"], ["generator", '"eeschema"'],
               ["generator_version", '"10.0"'], ["uuid", '"%s"' % ROOT_UUID],
               ["paper", '"A3"'], title_block,
               ["lib_symbols"] + [self.libids[k] for k in sorted(self.libids)]]

        for a, b in self.wires:
            out.append(["wire", ["pts", ["xy", num(a[0]), num(a[1])],
                                 ["xy", num(b[0]), num(b[1])]],
                        ["stroke", ["width", "0"], ["type", "default"]],
                        ["uuid", '"%s"' % uid("wire")]])
        for p in self.junctions:
            out.append(["junction", ["at", num(p[0]), num(p[1])], ["diameter", "0"],
                        ["color", "0", "0", "0", "0"], ["uuid", '"%s"' % uid("junc")]])
        for p in self.no_connects:
            out.append(["no_connect", ["at", num(p[0]), num(p[1])],
                        ["uuid", '"%s"' % uid("nc")]])
        for s, pt, size in self.texts:
            out.append(["text", '"%s"' % esc(s), ["exclude_from_sim", "no"],
                        ["at", num(pt[0]), num(pt[1]), "0"],
                        ["effects", ["font", ["size", num(size), num(size)]],
                         ["justify", "left", "bottom"]],
                        ["uuid", '"%s"' % uid("text")]])
        for t, pt, ang in self.labels:
            out.append(["label", '"%s"' % esc(t), ["at", num(pt[0]), num(pt[1]), num(ang)],
                        ["fields_autoplaced", "yes"],
                        ["effects", ["font", ["size", "1.27", "1.27"]],
                         ["justify", "left", "bottom"]],
                        ["uuid", '"%s"' % uid("label")]])
        out.extend(self.instances)
        out.append(["sheet_instances", ["path", '"/"', ["page", '"1"']]])

        with open(path, "w", encoding="utf-8", newline="\n") as fh:
            fh.write(sexp_dump(out) + "\n")
        return path
