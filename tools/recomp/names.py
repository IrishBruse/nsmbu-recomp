#!/usr/bin/env python3
"""Class and function names for guest addresses, from references/headers.

usage (from recomp.py): names.load() -> {address: qualified name}

The headers tie a name to an address in three shapes (docs/names.md):

    // Address: 0xADDR          before a declaration: members and methods
    // symbol   Address: 0xADDR  on one line: statics and the disposer block
    * @par Address: 0xADDR      in a doxygen block: virtuals

`Address: Deleted` and `Address: //!!!!!!!!` are not addresses. The class comes from the
enclosing `class`; a nested `class` adds to the scope. A symbol read out of a declaration is
the identifier before the first `(`, so `sead::TTaskFactory<TitleScene>` keeps its own
namespace and is not prefixed with the class.
"""
import os
import re

here = os.path.dirname(os.path.abspath(__file__))
headers_dir = os.path.normpath(os.path.join(here, "..", "..", "references", "headers"))

ADDRESS = re.compile(r"Address:\s*(0x[0-9A-Fa-f]{8})\b")
CLASS = re.compile(r"^\s*(?:class|struct)\s+(\w+)")
FIELD = re.compile(r"([A-Za-z_][A-Za-z0-9_]*)\s*(?:=[^;]*)?;\s*$")
SKIP_DECL_PREFIX = ("#", "SEAD_", "enum", "typedef", "static_assert", "using")


def strip_code(line):
    return re.sub(r'"(?:[^"\\]|\\.)*"', '""', line).split("//")[0]


def symbol_of(text):
    """The symbol a declaration or comment fragment names: the identifier before `(`."""
    paren = text.find("(")
    if paren >= 0:
        head = text[:paren].split()
        return head[-1].strip() if head else None
    m = FIELD.search(text.strip())
    return m.group(1) if m else None


def qualify(scope, symbol, from_decl):
    if not symbol:
        return None
    if from_decl and ("::" in symbol or "<" in symbol):
        return symbol
    return "::".join(scope + [symbol]) if scope else symbol


class Parser:
    def __init__(self):
        self.names = {}
        self.conflicts = 0

    def put(self, addr, name):
        if not name:
            return
        old = self.names.setdefault(addr, name)
        if old != name:
            self.conflicts += 1

    def parse(self, text):
        lines = text.splitlines()
        depth = 0
        pending = None
        stack = []
        for i, line in enumerate(lines):
            m = ADDRESS.search(line)
            if m:
                self.add(int(m.group(1), 16), line, stack, pending, lines, i)
            code = strip_code(line)
            opens, closes = code.count("{"), code.count("}")
            new_depth = depth + opens - closes
            if pending is not None:
                if opens:
                    stack.append((pending, new_depth))
                    pending = None
                elif ";" in code or closes:
                    pending = None
            depth = new_depth
            while stack and depth < stack[-1][1]:
                stack.pop()
            if pending is None:
                c = CLASS.match(code)
                if c and not code.rstrip().endswith(";"):
                    pending = c.group(1)

    def scope(self, stack, pending):
        if pending and not stack:
            return [pending]
        return [name for name, _ in stack]

    def add(self, addr, line, stack, pending, lines, i):
        before = line.split("Address:")[0]
        text = before.replace("//", " ").replace("*", " ").replace("@par", " ").strip()
        if text:
            self.put(addr, qualify(self.scope(stack, pending), text.split()[0], from_decl=False))
            return
        for k in range(i + 1, min(i + 16, len(lines))):
            decl = strip_code(lines[k]).strip()
            if not decl or decl == "*/" or decl.endswith(":"):
                continue
            if decl.startswith(SKIP_DECL_PREFIX) and "(" not in decl:
                continue
            if decl.startswith("extern"):
                continue
            symbol = symbol_of(decl)
            if symbol:
                self.put(addr, qualify(self.scope(stack, pending), symbol, from_decl=True))
            return

    def load_tree(self):
        for root, dirs, files in os.walk(headers_dir):
            dirs[:] = [d for d in sorted(dirs) if d != ".git"]
            for fn in sorted(files):
                if fn.endswith(".h"):
                    with open(os.path.join(root, fn), errors="replace") as f:
                        self.parse(f.read())


def load():
    """{address int: name} for every `Address:` value in references/headers."""
    parser = Parser()
    parser.load_tree()
    if parser.conflicts:
        print("names.py: %d addresses with two names" % parser.conflicts)
    return parser.names


if __name__ == "__main__":
    print("%d names" % len(load()))
