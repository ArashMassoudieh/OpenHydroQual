#!/usr/bin/env python3
"""Check that every file the Windows installer packages is present.

Advanced Installer skips a listed file it cannot find and still reports a
successful build, so a package can come out without Qt (and the installed
program then fails to start). This check reads every SourcePath in
Setup/QAquifolium.aip and fails if any of them is missing. Advanced
Installer's own placeholders (<AI_...>) are skipped.

Usage: python tools/check_installer.py
Exit code 1 if anything is missing.
"""
import html, os, re, sys

ROOT = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
SETUP = os.path.join(ROOT, "Setup")
AIP = os.path.join(SETUP, "QAquifolium.aip")


def main():
    text = open(AIP, encoding="utf8").read()
    paths = sorted(set(html.unescape(p) for p in re.findall(r'SourcePath="([^"]*)"', text)))
    paths = [p for p in paths if not p.startswith("<AI_")]
    missing = [p for p in paths
               if not os.path.exists(os.path.normpath(os.path.join(SETUP, p.replace("\\", os.sep))))]
    print(f"{len(paths)} files listed in {os.path.relpath(AIP, ROOT)}")
    if missing:
        print(f"{len(missing)} missing (the installer would leave them out):")
        for p in missing:
            print("  " + p)
        print("Build Release|x64 and run windeployqt on the executables before packaging.")
        return 1
    print("all listed files present")
    return 0


if __name__ == "__main__":
    sys.exit(main())
