#!/usr/bin/env python3
"""Check that every icon a template references is present and packaged.

  1. Every "icon": {"filename": ...} in resources/*.json must exist in
     resources/Icons/ (the GUI loads icons from there).
  2. Every such icon, and every template .json in resources/, must be listed
     as a file in the Advanced Installer project Setup/QAquifolium.aip, so the
     Windows installer ships it.

Usage: python tools/check_icons.py [--no-installer]
Exit code 1 if anything is missing. Templates whose file name contains
"bkup", "_old" or "test_" are skipped (they are not loaded by the GUI).
"""
import glob, json, os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
RES = os.path.join(ROOT, "resources")
ICONS = os.path.join(RES, "Icons")
AIP = os.path.join(ROOT, "Setup", "QAquifolium.aip")
SKIP = ("bkup", "_old", "test_")


def template_files():
    for f in sorted(glob.glob(os.path.join(RES, "*.json"))):
        if not any(s in os.path.basename(f) for s in SKIP):
            yield f


def referenced_icons():
    refs = {}
    for f in template_files():
        try:
            d = json.load(open(f, encoding="utf8"))
        except ValueError as e:
            print(f"cannot parse {os.path.basename(f)}: {e}")
            continue
        for name, t in d.items():
            if isinstance(t, dict) and isinstance(t.get("icon"), dict):
                fn = t["icon"].get("filename", "").strip()
                if fn:
                    refs.setdefault(fn, []).append(f"{os.path.basename(f)}:{name}")
    return refs


def main():
    bad = False
    refs = referenced_icons()
    present = {f.lower(): f for f in os.listdir(ICONS)}
    missing = sorted(fn for fn in refs if fn.lower() not in present)
    case = sorted(fn for fn in refs if fn.lower() in present and fn not in present.values())
    print(f"{len(refs)} icons referenced by templates")
    for fn in missing:
        bad = True
        print(f"  MISSING in resources/Icons: {fn}  ({', '.join(refs[fn])})")
    for fn in case:
        print(f"  case differs (fine on Windows, fails on Linux): {fn} vs {present[fn.lower()]}")
    if "--no-installer" not in sys.argv and os.path.exists(AIP):
        aip = open(AIP, encoding="utf8", errors="replace").read()
        listed = {os.path.basename(p.replace("\\", "/")).lower()
                  for p in re.findall(r'SourcePath="([^"]+)"', aip)}
        nopkg = sorted(fn for fn in refs if fn not in missing and fn.lower() not in listed)
        nojson = sorted(os.path.basename(f) for f in template_files()
                        if os.path.basename(f).lower() not in listed)
        for fn in nopkg:
            bad = True
            print(f"  NOT IN INSTALLER (Setup/QAquifolium.aip): Icons/{fn}  ({', '.join(refs[fn])})")
        for fn in nojson:
            bad = True
            print(f"  NOT IN INSTALLER (Setup/QAquifolium.aip): {fn}")
    print("FAILED" if bad else "all icons present and packaged")
    return 1 if bad else 0


if __name__ == "__main__":
    sys.exit(main())
