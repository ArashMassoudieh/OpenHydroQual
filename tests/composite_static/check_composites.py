#!/usr/bin/env python3
"""Static consistency check of the composite types defined in resources/*.json.

Does not build or run any model. For every "composite" type it checks what
Composite::Propagate() and Composite::ResolvePort() will need at load time:

* every member type and internal-link type is defined, preferably in the
  template file or one of the files it requires (recursively); a type found
  only in another template is a warning, a type found nowhere is an error;
* internal links join existing members, and external ports name existing
  members;
* every applyto target "<member or internal link>#<property>" exists;
* a target whose property is not a value/balance/constant (expression,
  source, timeseries, ...) is mapped from the bare name of a composite
  property, since Propagate() passes those through instead of evaluating them;
* every identifier in a numeric mapping, criteria or default expression is a
  property of the composite (or a known function).

Usage:  python3 check_composites.py [-v] [template.json ...]   (-v lists warnings)
        (no arguments: every composite template under resources/)
Exit status is the number of templates with errors.
"""
import json
import re
import sys
from pathlib import Path

RES = Path(__file__).resolve().parent.parent.parent / "resources"
NUMERIC = {"value", "balance", "constant"}
FUNCS = {"_min", "_max", "_pos", "_abs", "_sqt", "_sqr", "_exp", "_log", "_hsd", "_sgn", "_mon", "_bkw", "_lpw", "_ups"}
SKIP_KEYS = {"type", "typecategory", "description", "helptext", "icon", "members", "internal_links", "external_links", "name"}


def load(name, seen):
    if name in seen:
        return {}
    seen.add(name)
    path = RES / name
    if not path.exists():
        return {}
    data = json.load(open(path))
    types = {}
    for req in data.get("requires", []):
        types.update(load(req, seen))
    for k, v in data.items():
        if isinstance(v, dict) and "type" in v:
            types[k] = v
    return types


def props(defn):
    return {k: v.get("type") for k, v in defn.items() if isinstance(v, dict) and k not in SKIP_KEYS}


def idents(expr):
    expr = re.sub(r"(?<![A-Za-z_])[0-9.]+[eE][-+]?[0-9]+", "0", expr)  # drop numbers such as 1e-12
    return set(re.findall(r"[A-Za-z_][A-Za-z0-9_]*", expr)) - FUNCS


def all_types():
    types = {}
    for f in sorted(RES.glob("*.json")):
        try:
            data = json.load(open(f))
        except Exception:
            continue
        for k, v in data.items():
            if isinstance(v, dict) and "type" in v:
                types.setdefault(k, (f.name, v))
    return types


ALL = all_types()


def check(path):
    data = json.load(open(path))
    types = load(path.name, set())
    errors, warnings = [], []

    def lookup(t, kind, what):
        """Type definition from the requires chain, else from any template (warning), else None (error)."""
        if t in types and types[t].get("type") == kind:
            return types[t]
        if t in ALL and ALL[t][1].get("type") == kind:
            warnings.append(f"{what}: type '{t}' comes from {ALL[t][0]}, which is not in this file's requires chain")
            return ALL[t][1]
        return None

    for cname, comp in data.items():
        if not (isinstance(comp, dict) and comp.get("type") == "composite"):
            continue
        cprops = props(comp)
        members = comp.get("members", {})
        links = comp.get("internal_links", {})
        owners = {}
        for m, d in members.items():
            t = d.get("type")
            defn = lookup(t, "block", f"{cname}: member {m}")
            if defn is None:
                errors.append(f"{cname}: member {m} has unknown block type '{t}'")
            else:
                owners[m] = props(defn)
        for l, d in links.items():
            t = d.get("type")
            defn = lookup(t, "link", f"{cname}: internal link {l}")
            if defn is None:
                errors.append(f"{cname}: internal link {l} has unknown link type '{t}'")
            else:
                owners[l] = props(defn)
            for end in ("from", "to"):
                if d.get(end) not in members:
                    errors.append(f"{cname}: internal link {l} '{end}' = '{d.get(end)}' is not a member")
        for t, port in comp.get("external_links", {}).items():
            if t != "*" and lookup(t, "link", f"{cname}: external link") is None:
                warnings.append(f"{cname}: external link type '{t}' is not defined in any template")
            for end in ("from", "to"):
                if end in port and port[end] not in members:
                    # Some templates name the receiving block type here; that end is then never resolved to a member.
                    (warnings if port[end] in ALL else errors).append(
                        f"{cname}: external link '{t}' {end} = '{port[end]}' is not a member")
        for p, d in comp.items():
            if p in SKIP_KEYS or not isinstance(d, dict):
                continue
            for key in ("criteria", "default"):
                if key in d and isinstance(d[key], str) and re.search(r"[A-Za-z_]", d[key]):
                    bad = idents(d[key]) - set(cprops)
                    if bad and key == "criteria":
                        errors.append(f"{cname}.{p}: {key} uses unknown names {sorted(bad)}")
            for target, expr in d.get("applyto", {}).items():
                parts = target.split("#")
                if len(parts) != 2:
                    errors.append(f"{cname}.{p}: target '{target}' is not <member>#<property>")
                    continue
                owner, q = parts
                if owner not in owners:
                    errors.append(f"{cname}.{p}: target owner '{owner}' is not a member or internal link")
                    continue
                if q not in owners[owner]:
                    errors.append(f"{cname}.{p}: '{owner}' has no property '{q}'")
                    continue
                if owners[owner][q] in NUMERIC:
                    bad = idents(expr) - set(cprops)
                    if bad:
                        errors.append(f"{cname}.{p}: mapping of {target} uses unknown names {sorted(bad)}")
                elif expr not in cprops:
                    errors.append(f"{cname}.{p}: {target} is of type '{owners[owner][q]}', so it must map from a bare property name, not '{expr}'")
    return errors, warnings


def main(argv):
    verbose = "-v" in argv
    argv = [a for a in argv if a != "-v"]
    files = [Path(a) for a in argv] or sorted(RES.glob("*.json"))
    failed = 0
    for f in files:
        f = f if f.is_absolute() else Path.cwd() / f
        try:
            data = json.load(open(f))
        except Exception:
            continue
        if not any(isinstance(v, dict) and v.get("type") == "composite" for v in data.values()):
            continue
        errs, warns = check(f)
        print(f"{f.name}: {'OK' if not errs else str(len(errs)) + ' error(s)'}"
              + (f", {len(warns)} warning(s)" if warns else ""))
        for e in errs:
            print("    ERROR  ", e)
        if verbose:
            for w in warns:
                print("    warning", w)
        failed += bool(errs)
    return failed


if __name__ == "__main__":
    sys.exit(main(sys.argv[1:]))
