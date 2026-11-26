#!/usr/bin/env python3
"""Static check of an .ohq script against the resource templates it loads.

Does not build or run the model. Checks that every 'create' line names a
known type, sets only properties that type defines (or a composite's own
properties), links existing objects, and that a link attached to a
composite has a port for that link type on that end (external_links).

Usage:  python3 check_ohq.py model.ohq [more.ohq ...]
"""
import json
import re
import sys
from pathlib import Path

RES = Path(__file__).resolve().parent.parent.parent / "resources"
GEOM = {"name", "x", "y", "_width", "_height", "type", "from", "to"}


def templates(lines):
    types = {}
    for ln in lines:
        m = re.match(r"\s*(load|add)template;\s*filename\s*=\s*(.+)$", ln)
        if m:
            f = RES / Path(m.group(2).strip()).name
            for k, v in json.load(open(f)).items():
                if isinstance(v, dict) and "type" in v:
                    types[k] = v
    return types


def split_props(body):
    out = {}
    for part in body.split(","):
        if "=" in part:
            k, v = part.split("=", 1)
            out[k.strip()] = v.strip()
    return out


def check(path):
    lines = open(path).read().splitlines()
    types = templates(lines)
    objects, errors = {}, []
    for n, ln in enumerate(lines, 1):
        m = re.match(r"\s*create\s+(\w+);(.*)$", ln)
        if not m:
            continue
        kind, p = m.group(1), split_props(m.group(2))
        t, name = p.get("type"), p.get("name")
        if t not in types:
            errors.append(f"line {n}: unknown type '{t}'")
            continue
        defn = types[t]
        allowed = {k for k, v in defn.items() if isinstance(v, dict)} | GEOM
        for k in p:
            base = k.split(":")[0] if ":" in k else k
            if base not in allowed and ":" not in k:
                errors.append(f"line {n}: '{t}' has no property '{k}'")
        if kind == "link":
            for end, key in (("from", "from"), ("to", "to")):
                o = p.get(key)
                if o not in objects:
                    errors.append(f"line {n}: link '{name}' {end} '{o}' is not defined above")
                    continue
                otype = objects[o]
                if types.get(otype, {}).get("type") == "composite":
                    ports = types[otype].get("external_links", {})
                    port = ports.get(t, ports.get("*"))
                    if port is None or end not in port:
                        errors.append(f"line {n}: composite '{o}' ({otype}) has no '{end}' port for link type '{t}'")
        if name:
            objects[name] = t
    return errors, len(objects)


if __name__ == "__main__":
    bad = 0
    for a in sys.argv[1:]:
        errs, nobj = check(a)
        print(f"{a}: {nobj} objects, {'OK' if not errs else str(len(errs)) + ' error(s)'}")
        for e in errs:
            print("   ", e)
        bad += bool(errs)
    sys.exit(bad)
