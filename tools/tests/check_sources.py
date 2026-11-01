#!/usr/bin/env python3
"""Every Faust file the prototype loads is in git.

The design sources under `sound design/` are the G2 reference the port and the G5 null test
cite; a file that exists only on one machine is lost with it (a `*.lib` rule meant for MSVC once
dropped two Faust libraries). Checks that each `library("x")` / `import("x")` / `component("x")`
in a tracked .dsp or .lib resolves to a file git tracks (standard Faust libraries excepted), so a
.gitignore rule cannot drop one again.
Usage: python3 tools/tests/check_sources.py [repo root]
"""
import re
import subprocess
import sys
from pathlib import Path

ROOT = Path(sys.argv[1] if len(sys.argv) > 1 else Path(__file__).resolve().parents[2])
REF = re.compile(r'\b(?:library|import|component)\s*\(\s*"([^"]+)"\s*\)')
STD = {"stdfaust.lib"}   # the Faust distribution's libraries, resolved by the compiler


def tracked():
    out = subprocess.run(["git", "-C", str(ROOT), "ls-files", "-z"], capture_output=True, check=True).stdout
    return {ROOT / p for p in out.decode().split("\0") if p}


def main():
    files = tracked()
    faust = [p for p in files if p.suffix in (".dsp", ".lib") and "sound design" in p.parts]
    missing = []
    for src in sorted(faust):
        for name in REF.findall(src.read_text(encoding="utf-8")):
            if name in STD:
                continue
            target = (src.parent / name).resolve()
            if target not in {f.resolve() for f in files}:
                missing.append(f"{src.relative_to(ROOT)} -> {name}")
    print(f"{len(faust)} Faust sources tracked; {len(missing)} references not in git")
    for m in missing:
        print("  missing:", m)
    return 1 if missing else 0


if __name__ == "__main__":
    sys.exit(main())
