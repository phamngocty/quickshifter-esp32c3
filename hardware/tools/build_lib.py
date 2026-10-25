"""
build_lib.py - Emits the project-local KiCad libraries and project files:

    hardware/quickshifter-c3-ign/quickshifter.kicad_sym      (symbols)
    hardware/quickshifter-c3-ign/quickshifter.pretty/*.kicad_mod
    hardware/quickshifter-c3-ign/sym-lib-table
    hardware/quickshifter-c3-ign/fp-lib-table
    hardware/quickshifter-c3-ign/quickshifter-c3-ign.kicad_pro

Only two symbols are custom: the ESP32-C3 Super Mini module (KiCad ships no
footprint for it) and the ignition IGBT (renamed from KiCad's IRG4PF50W so the
value/footprint defaults match the part actually used). Everything else comes
straight from the stock KiCad libraries.
"""

from __future__ import annotations

import json
import os
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))

from kicadgen import sexp_dump, uid
from kicadsch import esp32_symbol, igbt_symbol

HERE = os.path.dirname(os.path.abspath(__file__))
OUT = os.path.join(os.path.dirname(HERE), "quickshifter-c3-ign")

PROJECT = "quickshifter-c3-ign"
LIB_NICK = "QS"

# ESP32-C3 Super Mini mechanical data (datasheet / community footprint).
MOD_LEN = 22.5      # long axis, X
MOD_WID = 18.0      # short axis, Y
PAD_PITCH = 2.54
ROW_SPACING = 15.24
PAD_SIZE = 1.8
PAD_DRILL = 1.0
PAD_SPAN = 7 * PAD_PITCH          # 17.78

# pad number -> (x, y) in footprint coordinates (Y down, origin at centre)
# Published module pinout: row 1 top->bottom 5V,GND,3V3,IO0,IO1,IO2,IO3,IO4;
# row 2 top->bottom IO5,IO6,IO7,IO8,IO9,IO10,IO20,IO21.
PAD_NAMES = ["5V", "GND", "3V3", "IO0", "IO1", "IO2", "IO3", "IO4",
             "IO5", "IO6", "IO7", "IO8", "IO9", "IO10", "IO20", "IO21"]


def pad_positions():
    out = {}
    n = 1
    for row, y in ((0, -ROW_SPACING / 2.0), (1, ROW_SPACING / 2.0)):
        for i in range(8):
            x = -PAD_SPAN / 2.0 + i * PAD_PITCH
            out[n] = (round(x, 3), round(y, 3))
            n += 1
    return out


def esp32_footprint() -> list:
    pos = pad_positions()
    half_l, half_w = MOD_LEN / 2.0, MOD_WID / 2.0

    def line(x1, y1, x2, y2, layer="F.SilkS", width=0.12):
        return ["fp_line", ["start", f"{x1}", f"{y1}"], ["end", f"{x2}", f"{y2}"],
                ["stroke", ["width", f"{width}"], ["type", "solid"]], ["layer", layer]]

    fp = ["footprint", '"ESP32-C3-SuperMini"',
          ["version", "20260206"], ["generator", '"pcbnew"'],
          ["generator_version", '"10.0"'], ["layer", '"F.Cu"'],
          ["descr", '"ESP32-C3 Super Mini module, 22.5x18mm, 2x8 castellated pads at 2.54mm pitch, 15.24mm row spacing"'],
          ["tags", '"ESP32 ESP32-C3 module SuperMini castellated"'],
          ["property", '"Reference"', '"U"', ["at", "0", f"{-half_w - 1.5}", "0"],
           ["layer", '"F.SilkS"'],
           ["effects", ["font", ["size", "1", "1"], ["thickness", "0.15"]]]],
          ["property", '"Value"', '"ESP32-C3-SuperMini"', ["at", "0", f"{half_w + 1.5}", "0"],
           ["layer", '"F.Fab"'],
           ["effects", ["font", ["size", "1", "1"], ["thickness", "0.15"]]]],
          ["property", '"Datasheet"', '""', ["at", "0", "0", "0"], ["layer", '"F.Fab"'],
           ["hide", "yes"], ["effects", ["font", ["size", "1", "1"], ["thickness", "0.15"]]]],
          ["property", '"Description"', '""', ["at", "0", "0", "0"], ["layer", '"F.Fab"'],
           ["hide", "yes"], ["effects", ["font", ["size", "1", "1"], ["thickness", "0.15"]]]],
          ["attr", "through_hole"],
          ["duplicate_pad_numbers_are_jumpers", "no"]]

    # Courtyard
    for a, b in (((-half_l - 0.25, -half_w - 0.25), (half_l + 0.25, -half_w - 0.25)),
                 ((half_l + 0.25, -half_w - 0.25), (half_l + 0.25, half_w + 0.25)),
                 ((half_l + 0.25, half_w + 0.25), (-half_l - 0.25, half_w + 0.25)),
                 ((-half_l - 0.25, half_w + 0.25), (-half_l - 0.25, -half_w - 0.25))):
        fp.append(line(a[0], a[1], b[0], b[1], "F.CrtYd", 0.05))

    # Fab body outline
    for a, b in (((-half_l, -half_w), (half_l, -half_w)),
                 ((half_l, -half_w), (half_l, half_w)),
                 ((half_l, half_w), (-half_l, half_w)),
                 ((-half_l, half_w), (-half_l, -half_w))):
        fp.append(line(a[0], a[1], b[0], b[1], "F.Fab", 0.1))

    # Silk body outline
    for a, b in (((-half_l, -half_w), (half_l, -half_w)),
                 ((half_l, -half_w), (half_l, half_w)),
                 ((half_l, half_w), (-half_l, half_w)),
                 ((-half_l, half_w), (-half_l, -half_w))):
        fp.append(line(a[0], a[1], b[0], b[1], "F.SilkS", 0.12))

    # Pin-1 marker + USB-C end marker
    fp.append(line(-half_l, -half_w, -half_l + 1.5, -half_w, "F.SilkS", 0.3))
    fp.append(["fp_text", "user", '"USB-C"', ["at", f"{-half_l + 2.6}", "0", "90"],
               ["layer", '"F.SilkS"'],
               ["effects", ["font", ["size", "0.8", "0.8"], ["thickness", "0.13"]]]])
    fp.append(["fp_text", "user", '"1"', ["at", f"{-half_l - 1.2}", f"{-half_w + 0.9}", "0"],
               ["layer", '"F.SilkS"'],
               ["effects", ["font", ["size", "0.8", "0.8"], ["thickness", "0.13"]]]])

    for n, (x, y) in sorted(pos.items()):
        shape = "rect" if n == 1 else "circle"
        fp.append(["pad", f'"{n}"', "thru_hole", shape,
                   ["at", f"{x}", f"{y}"], ["size", f"{PAD_SIZE}", f"{PAD_SIZE}"],
                   ["drill", f"{PAD_DRILL}"], ["layers", '"*.Cu"', '"*.Mask"'],
                   ["remove_unused_layers", "no"]])
        # per-pad function label, offset outside the row. Kept on F.Fab: at
        # 0.8 mm the minimum silkscreen text height, and silkscreen printed
        # under a module is invisible anyway.
        dy = -1.9 if y < 0 else 1.9
        fp.append(["fp_text", "user", f'"{PAD_NAMES[n - 1]}"',
                   ["at", f"{x}", f"{y + dy}", "0"], ["layer", '"F.Fab"'],
                   ["effects", ["font", ["size", "0.8", "0.8"], ["thickness", "0.13"]]]])

    fp.append(["embedded_fonts", "no"])
    return fp


def to220_standing_footprint() -> list:
    """TO-220 with the leads bent 90 deg and the tab standing free.

    KiCad's own TO-220 footprints reserve 21 mm for the mounting tab. On a
    37 mm-tall board that is unaffordable, and this IGBT only conducts for
    ~100 ms at a time so it needs no heatsink.
    """
    fp = ["footprint", '"TO-220_Standing"',
          ["version", "20260206"], ["generator", '"pcbnew"'],
          ["generator_version", '"10.0"'], ["layer", '"F.Cu"'],
          ["descr", '"TO-220 standing upright, 3 leads at 2.54mm pitch, no tab hole"'],
          ["tags", '"TO-220 TO220 standing vertical IGBT MOSFET"'],
          ["property", '"Reference"', '"Q"', ["at", "0", "-5.2", "0"], ["layer", '"F.SilkS"'],
           ["effects", ["font", ["size", "1", "1"], ["thickness", "0.15"]]]],
          ["property", '"Value"', '"TO-220_Standing"', ["at", "0", "5.2", "0"],
           ["layer", '"F.Fab"'],
           ["effects", ["font", ["size", "1", "1"], ["thickness", "0.15"]]]],
          ["property", '"Datasheet"', '""', ["at", "0", "0", "0"], ["layer", '"F.Fab"'],
           ["hide", "yes"], ["effects", ["font", ["size", "1", "1"], ["thickness", "0.15"]]]],
          ["property", '"Description"', '""', ["at", "0", "0", "0"], ["layer", '"F.Fab"'],
           ["hide", "yes"], ["effects", ["font", ["size", "1", "1"], ["thickness", "0.15"]]]],
          ["attr", "through_hole"],
          ["duplicate_pad_numbers_are_jumpers", "no"]]

    def line(x1, y1, x2, y2, layer, width):
        return ["fp_line", ["start", f"{x1}", f"{y1}"], ["end", f"{x2}", f"{y2}"],
                ["stroke", ["width", f"{width}"], ["type", "solid"]], ["layer", layer]]

    # body footprint (the package itself stands up off the board)
    for a, b in (((-5.2, -2.3), (5.2, -2.3)), ((5.2, -2.3), (5.2, 2.3)),
                 ((5.2, 2.3), (-5.2, 2.3)), ((-5.2, 2.3), (-5.2, -2.3))):
        fp.append(line(a[0], a[1], b[0], b[1], '"F.Fab"', 0.1))
    for a, b in (((-5.8, -2.8), (5.8, -2.8)), ((5.8, -2.8), (5.8, 2.8)),
                 ((5.8, 2.8), (-5.8, 2.8)), ((-5.8, 2.8), (-5.8, -2.8))):
        fp.append(line(a[0], a[1], b[0], b[1], '"F.CrtYd"', 0.05))
    # silk: pin-1 corner and the lead row
    fp.append(line(-5.2, -2.8, 5.2, -2.8, '"F.SilkS"', 0.12))
    fp.append(line(-4.6, -2.8, -3.4, -2.8, '"F.SilkS"', 0.3))
    fp.append(["fp_text", "user", '"G  C  E"', ["at", "0", "3.6", "0"], ["layer", '"F.Fab"'],
               ["effects", ["font", ["size", "0.8", "0.8"], ["thickness", "0.13"]]]])

    for num, x in (("1", -2.54), ("2", 0.0), ("3", 2.54)):
        fp.append(["pad", f'"{num}"', "thru_hole", "rect" if num == "1" else "circle",
                   ["at", f"{x}", "0"], ["size", "1.9", "2.0"], ["drill", "1.0"],
                   ["layers", '"*.Cu"', '"*.Mask"'], ["remove_unused_layers", "no"]])
    fp.append(["embedded_fonts", "no"])
    return fp


def strip_prefix(node):
    """Schematic lib_symbols use 'QS:Name'; .kicad_sym files use 'Name'."""
    out = []
    for i, tok in enumerate(node):
        if i == 1 and isinstance(tok, str) and tok.startswith(f'"{LIB_NICK}:'):
            out.append('"' + tok[1:-1].split(":", 1)[1] + '"')
        elif isinstance(tok, list):
            out.append(strip_prefix(tok))
        else:
            out.append(tok)
    return out


def main():
    os.makedirs(OUT, exist_ok=True)
    pretty = os.path.join(OUT, "quickshifter.pretty")
    os.makedirs(pretty, exist_ok=True)

    # ---- symbol library ----
    lib = ["kicad_symbol_lib", ["version", "20251024"],
           ["generator", '"kicad_symbol_editor"'], ["generator_version", '"10.0"'],
           strip_prefix(esp32_symbol()), strip_prefix(igbt_symbol())]
    with open(os.path.join(OUT, "quickshifter.kicad_sym"), "w",
              encoding="utf-8", newline="\n") as fh:
        fh.write(sexp_dump(lib) + "\n")

    # ---- footprint library ----
    with open(os.path.join(pretty, "ESP32-C3-SuperMini.kicad_mod"), "w",
              encoding="utf-8", newline="\n") as fh:
        fh.write(sexp_dump(esp32_footprint()) + "\n")
    with open(os.path.join(pretty, "TO-220_Standing.kicad_mod"), "w",
              encoding="utf-8", newline="\n") as fh:
        fh.write(sexp_dump(to220_standing_footprint()) + "\n")

    # ---- library tables ----
    with open(os.path.join(OUT, "sym-lib-table"), "w", encoding="utf-8", newline="\n") as fh:
        fh.write('(sym_lib_table\n'
                 '\t(version 7)\n'
                 f'\t(lib (name "{LIB_NICK}")(type "KiCad")'
                 f'(uri "${{KIPRJMOD}}/quickshifter.kicad_sym")(options "")(descr "Quickshifter custom symbols"))\n'
                 ')\n')
    with open(os.path.join(OUT, "fp-lib-table"), "w", encoding="utf-8", newline="\n") as fh:
        fh.write('(fp_lib_table\n'
                 '\t(version 7)\n'
                 f'\t(lib (name "{LIB_NICK}")(type "KiCad")'
                 f'(uri "${{KIPRJMOD}}/quickshifter.pretty")(options "")(descr "Quickshifter custom footprints"))\n'
                 ')\n')

    # ---- project file ----
    pro = {
        "board": {
            "design_settings": {
                "defaults": {"board_outline_line_width": 0.1, "copper_line_width": 0.2},
                "rules": {"min_copper_edge_clearance": 0.3},
                "track_widths": [0.0, 0.4, 0.6, 1.0, 2.0],
                "via_dimensions": [{"diameter": 0.0, "drill": 0.0},
                                   {"diameter": 0.8, "drill": 0.4}],
            },
            "layer_presets": [],
            "viewports": [],
        },
        "boards": [],
        "cvpcb": {"equivalence_files": []},
        "erc": {"erc_exclusions": [], "meta": {"version": 0}, "rule_severities": {}},
        "libraries": {"pinned_footprint_libs": [], "pinned_symbol_libs": []},
        "meta": {"filename": PROJECT + ".kicad_pro", "version": 1},
        "net_settings": {
            "classes": [
                {"name": "Default", "clearance": 0.2, "track_width": 0.4,
                 "via_diameter": 0.8, "via_drill": 0.4, "microvia_diameter": 0.3,
                 "microvia_drill": 0.1, "wire_width": 6, "bus_width": 12},
                {"name": "PWR", "clearance": 0.25, "track_width": 1.0,
                 "via_diameter": 0.8, "via_drill": 0.4, "microvia_diameter": 0.3,
                 "microvia_drill": 0.1, "wire_width": 6, "bus_width": 12},
                {"name": "TRIG", "clearance": 0.3, "track_width": 2.0,
                 "via_diameter": 0.8, "via_drill": 0.4, "microvia_diameter": 0.3,
                 "microvia_drill": 0.1, "wire_width": 6, "bus_width": 12},
            ],
            "meta": {"version": 4},
            "net_colors": None,
        },
        "pcbnew": {"last_paths": {"gencad": "", "idf": "", "netlist": "",
                                  "specctra_dsn": "", "step": "", "vrml": ""},
                   "page_layout_descr_file": ""},
        "schematic": {"legacy_lib_dir": "", "legacy_lib_list": []},
        "sheets": [],
        "text_variables": {},
    }
    with open(os.path.join(OUT, PROJECT + ".kicad_pro"), "w",
              encoding="utf-8", newline="\n") as fh:
        json.dump(pro, fh, indent=2)
        fh.write("\n")

    print("wrote symbol lib, footprint lib, lib tables and project file in", OUT)


if __name__ == "__main__":
    main()
