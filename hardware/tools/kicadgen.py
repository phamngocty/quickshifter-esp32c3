"""
kicadgen.py - Minimal KiCad 9/10 file generator (schematic + symbol library).

No third-party deps. Emits .kicad_sch that `kicad-cli sch erc` accepts.

Why hand-rolled: KiCad ships no Python API for schematics, and the machine has
no network access to install `kiutils`/`skip`. The .kicad_sch format is a plain
S-expression, so we synthesize it directly and let kicad-cli validate it.
"""

from __future__ import annotations

import os
import re
import uuid as _uuid

KICAD_SHARE = r"C:\Program Files\KiCad\10.0\share\kicad"
SYMBOL_DIR = os.path.join(KICAD_SHARE, "symbols")

# Deterministic UUIDs so regenerating the file produces minimal diffs.
_NS = _uuid.UUID("6f9619ff-8b86-d011-b42d-00c04fc964ff")
_uuid_counter: dict[str, int] = {}


def uid(key: str) -> str:
    """Stable UUID derived from a caller-supplied key."""
    n = _uuid_counter.get(key, 0)
    _uuid_counter[key] = n + 1
    return str(_uuid.uuid5(_NS, f"{key}#{n}"))


# --------------------------------------------------------------------------
# S-expression parsing
# --------------------------------------------------------------------------

_TOKEN = re.compile(r'"(?:[^"\\]|\\.)*"|\(|\)|[^\s()]+')


def sexp_parse(text: str):
    """Parse an S-expression into nested lists of strings."""
    tokens = _TOKEN.findall(text)
    stack: list[list] = []
    root: list = []
    for tok in tokens:
        if tok == "(":
            new: list = []
            if stack:
                stack[-1].append(new)
            else:
                root.append(new)
            stack.append(new)
        elif tok == ")":
            if not stack:
                raise ValueError("unbalanced ')'")
            stack.pop()
        else:
            if not stack:
                continue
            stack[-1].append(tok)
    if stack:
        raise ValueError("unbalanced '('")
    return root


def _unquote(tok: str) -> str:
    if len(tok) >= 2 and tok[0] == '"' and tok[-1] == '"':
        return tok[1:-1].replace('\\"', '"')
    return tok


def head(node) -> str:
    return _unquote(node[0]) if node and isinstance(node[0], str) else ""


def children(node, name: str):
    for c in node:
        if isinstance(c, list) and head(c) == name:
            yield c


def child(node, name: str):
    for c in children(node, name):
        return c
    return None


def _extract_balanced(text: str, start: int) -> str:
    """Return text[start:end] where start indexes a '(' and end is past its match."""
    depth = 0
    i = start
    in_str = False
    while i < len(text):
        ch = text[i]
        if in_str:
            if ch == "\\":
                i += 2
                continue
            if ch == '"':
                in_str = False
        elif ch == '"':
            in_str = True
        elif ch == "(":
            depth += 1
        elif ch == ")":
            depth -= 1
            if depth == 0:
                return text[start : i + 1]
        i += 1
    raise ValueError("unbalanced parentheses while scanning")


class SymbolLib:
    """Reads .kicad_sym libraries from the KiCad installation."""

    _cache: dict[str, str] = {}

    @classmethod
    def _raw(cls, lib: str) -> str:
        if lib not in cls._cache:
            path = os.path.join(SYMBOL_DIR, lib + ".kicad_sym")
            with open(path, "r", encoding="utf-8") as fh:
                cls._cache[lib] = fh.read()
        return cls._cache[lib]

    @classmethod
    def raw_symbol_text(cls, lib: str, name: str) -> str:
        """Raw source text of `(symbol "NAME" ...)` from `lib`."""
        text = cls._raw(lib)
        needle = '(symbol "%s"' % name
        idx = text.find(needle)
        while idx != -1:
            # depth 1 only: the enclosing char chain must be the lib root
            before = text[:idx].rstrip()
            if before.endswith("(kicad_symbol_lib") or before.endswith(")"):
                break
            idx = text.find(needle, idx + 1)
        if idx == -1:
            raise KeyError(f"{lib}:{name} not found")
        return _extract_balanced(text, idx)

    child = staticmethod(child)

    @classmethod
    def symbol(cls, lib: str, name: str):
        return sexp_parse(cls.raw_symbol_text(lib, name))[0]

    @classmethod
    def resolve_extends(cls, lib: str, name: str, _depth: int = 0):
        """Return (raw_text, extends_base_or_None)."""
        sym = cls.symbol(lib, name)
        ext = child(sym, "extends")
        base = _unquote(ext[1]) if ext else None
        if base and _depth < 5:
            return cls.resolve_extends(lib, base, _depth + 1), base
        return cls.symbol(lib, name), None

    @classmethod
    def chain(cls, lib: str, name: str):
        """[derived..., base] symbols following `extends`."""
        out = []
        cur = name
        for _ in range(8):
            sym = cls.symbol(lib, cur)
            out.append(sym)
            ext = child(sym, "extends")
            if not ext:
                return out
            cur = _unquote(ext[1])
        raise ValueError(f"extends chain too deep for {lib}:{name}")

    @classmethod
    def flatten(cls, lib: str, name: str):
        """Derived symbol flattened into a standalone node (what KiCad caches).

        Sub-unit symbols must be renamed too: KiCad derives a unit's unit/style
        from the `<parent>_<unit>_<style>` suffix of the child symbol name.
        """
        chain = cls.chain(lib, name)          # [derived, ..., base]
        base = chain[-1]
        base_name = _symbol_name(base)
        props: dict[str, list] = {}
        for node in reversed(chain):          # base first, nearest derivation last
            for p in children(node, "property"):
                props[_unquote(p[1])] = p
        out = ["symbol", '"%s:%s"' % (lib, name)]
        for tok in base[1:]:
            if isinstance(tok, str):
                continue
            if head(tok) in ("property", "extends"):
                continue
            out.append(_rename_units(tok, base_name, name))
        for key in ("Reference", "Value", "Footprint", "Datasheet", "Description"):
            if key in props:
                out.append(props.pop(key))
        out.extend(props.values())
        return out

    @classmethod
    def pins_of(cls, lib: str, name: str) -> list["Pin"]:
        return collect_pins(cls.flatten(lib, name))

    @classmethod
    def embedded(cls, lib: str, name: str) -> str:
        """Symbol text ready to paste into a schematic's lib_symbols block."""
        return sexp_dump(cls.flatten(lib, name))


def _symbol_name(sym) -> str:
    for tok in sym[1:]:
        if isinstance(tok, str):
            return _unquote(tok)
    raise ValueError("anonymous symbol")


def _rename_units(node, old: str, new: str):
    """Recursively rename `<old>_<unit>_<style>` child symbol names to `<new>...`."""
    if isinstance(node, str):
        if node.startswith('"%s_' % old):
            return '"%s_' % new + node[len('"%s_' % old):]
        return node
    return [_rename_units(c, old, new) for c in node]


# --------------------------------------------------------------------------
# Symbol pin geometry
# --------------------------------------------------------------------------

class Pin:
    __slots__ = ("number", "name", "x", "y", "angle", "length", "etype")

    def __init__(self, number, name, x, y, angle, length, etype):
        self.number = number
        self.name = name
        self.x = x          # symbol-local, Y up
        self.y = y
        self.angle = angle  # 0=right, 90=up, 180=left, 270=down (pin points away)
        self.length = length
        self.etype = etype

    def __repr__(self):
        return f"Pin({self.number},{self.name} @{self.x},{self.y} a{self.angle})"


def collect_pins(sym, _seen=None) -> list[Pin]:
    """All pins of a symbol, including those in sub-units."""
    out: list[Pin] = []
    for sub in children(sym, "symbol"):
        for p in children(sub, "pin"):
            at = child(p, "at")
            length = child(p, "length")
            num = child(p, "number")
            nm = child(p, "name")
            out.append(
                Pin(
                    number=_unquote(num[1]) if num else "?",
                    name=_unquote(nm[1]) if nm else "~",
                    x=float(at[1]),
                    y=float(at[2]),
                    angle=float(at[3]) if len(at) > 3 else 0.0,
                    length=float(length[1]) if length else 0.0,
                    etype=p[1] if isinstance(p[1], str) else "passive",
                )
            )
    return out


def pin_position(px: float, py: float, angle: float, mirror: str | None,
                 sym_x: float, sym_y: float):
    """Absolute schematic coordinate of a symbol-local pin.

    Symbol space is Y-up; the schematic is Y-down, hence the sign flip.
    """
    if mirror == "x":
        py = -py
    elif mirror == "y":
        px = -px
    a = int(round(angle)) % 360
    if a == 90:
        rx, ry = -py, px
    elif a == 180:
        rx, ry = -px, -py
    elif a == 270:
        rx, ry = py, -px
    else:
        rx, ry = px, py
    return (round(sym_x + rx, 4), round(sym_y - ry, 4))


# --------------------------------------------------------------------------
# S-expression serialisation
# --------------------------------------------------------------------------

def sexp_dump(node, indent: int = 0) -> str:
    """Serialise a parsed S-expression back to KiCad-style text."""
    pad = "\t" * indent
    if not isinstance(node, list):
        return str(node)
    if not node:
        return "()"
    # (atom arg arg ...) -> single line unless a child is itself a list
    parts = []
    for item in node:
        parts.append(sexp_dump(item, indent + 1) if isinstance(item, list) else str(item))
    if all(not isinstance(i, list) for i in node) and len(" ".join(parts)) < 100:
        return pad + "(" + " ".join(parts) + ")"
    body = "\n".join(parts)
    return pad + "(\n" + body + "\n" + pad + ")"


def num(v: float) -> str:
    """Format a coordinate the way KiCad does (no trailing zeros)."""
    s = f"{v:.4f}".rstrip("0").rstrip(".")
    return s if s not in ("", "-0") else "0"
