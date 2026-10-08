#!/usr/bin/env python3
"""Write weak stubs for Wind Waker guest functions the runtime still names.

usage: guest_stubs.py OUT.c

The NSMBU recompiler does not emit these addresses.
The runtime hook code still refers to them.
A weak stub satisfies the link.
A real function in the generated game code replaces the stub.
"""
import glob
import os
import re
import sys

root = os.path.normpath(os.path.join(os.path.dirname(__file__), "..", ".."))
pat = re.compile(r"\bf_([0-9A-F]{8})(_orig)?\b")
fence = re.compile(r"\bT60_FENCE_(?:LINK|ANY)\(([0-9A-F]{8})")


def symbols():
    found = set()
    for ext in ("c", "cpp", "h", "mm"):
        for path in glob.glob(os.path.join(root, "runtime", "**", "*." + ext), recursive=True):
            if path.endswith("wwhd_guest_stubs.c"):
                continue
            with open(path, errors="replace") as f:
                text = f.read()
            for addr, orig in pat.findall(text):
                found.add("f_%s%s" % (addr, orig))
            for addr in fence.findall(text):
                found.add("f_%s_orig" % addr)
    return sorted(found)


def main(out):
    names = symbols()
    lines = ['#include "ppc.h"', ""]
    for name in names:
        addr = name[2:10]
        lines.append("__attribute__((weak)) void %s(Cpu* __restrict c) { ppc_unimplemented(c, 0x%s, 0); }" % (name, addr))
    lines.append("")
    os.makedirs(os.path.dirname(os.path.abspath(out)), exist_ok=True)
    with open(out, "w") as f:
        f.write("\n".join(lines))
    print("%d weak guest stubs -> %s" % (len(names), out))


if __name__ == "__main__":
    main(sys.argv[1] if len(sys.argv) > 1 else os.path.join(root, "runtime", "src", "wwhd_guest_stubs.c"))
