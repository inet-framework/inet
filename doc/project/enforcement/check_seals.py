#!/usr/bin/env python3
"""The seal gate: checks the SR-* flag rules and regenerates the index in audit/seal-list.md.

Run it through check-seals.sh, which enters the repository root first.
"""
import os, re, sys

ROOT = "doc/project"
CLOSED, OPEN = "\U0001F512", "⬜"          # 🔒 ⬜
UNITS = {"none", "whole", "section", "requirement", "rule", "decision", "row"}
HEADER = re.compile(r"^>\s*\*\*Kind:\*\*.*?\*\*Seal:\*\*\s*([^·]+?)\s*(?:·|$)", re.M)

def documents():
    for d, _, fs in os.walk(ROOT):
        for f in sorted(fs):
            if f.endswith(".md"):
                yield os.path.join(d, f)

def declared_unit(text, path):
    m = HEADER.search(text)
    if not m:
        return None, f"{path}: no Seal field in the document header (DR-HEADER)"
    raw = m.group(1).strip()
    complete = raw.endswith(", complete")
    base = raw[:-len(", complete")] if complete else raw
    base = base[len("by "):] if base.startswith("by ") else base
    base = base.split(";")[0].strip()
    if base not in UNITS:
        return None, f"{path}: unknown seal unit {raw!r} (SR-FLAG-COVERAGE)"
    return (base, complete, raw), None

def units_of(text, unit):
    """Return the body line that each sealable unit owns."""
    if unit in ("requirement", "rule", "decision"):
        pat = re.compile(r"(?m)^#{2,4} ((?:[A-Z]{1,6}-)+[A-Z0-9-]+)$")
    elif unit == "section":
        pat = re.compile(r"(?m)^#{2,3} (?!Index$)(.+)$")
    else:
        return []
    out = []
    for m in pat.finditer(text):
        rest = text[m.end():]
        body = next((ln for ln in rest.split("\n") if ln.strip()), "")
        out.append((m.group(1), body))
    return out

def main():
    write = "--write" in sys.argv
    problems, rows = [], []
    for path in documents():
        rel = os.path.relpath(path, ROOT)
        text = open(path, encoding="utf-8").read()
        # A mention is not a flag. Strip fenced blocks and inline code spans, so that a document
        # that *defines* the flags (rule/sealing.md) or quotes one is not read as carrying them.
        prose = re.sub(r"(?ms)^```.*?^```", "", text)
        prose = re.sub(r"`[^`\n]*`", "", prose)
        spec, err = declared_unit(text, rel)
        if err:
            problems.append(err)
            continue
        unit, complete, raw = spec

        # SR-FLAG-PLACEMENT: never in a heading
        for m in re.finditer(r"(?m)^#{1,6} .*$", prose):
            if CLOSED in m.group(0) or OPEN in m.group(0):
                problems.append(f"{rel}: a seal flag sits in a heading (SR-FLAG-PLACEMENT): {m.group(0)[:60]}")

        # A flag counts only where a flag can be: at the start of a line. A glyph inside a table
        # cell marks a path in the registry, or illustrates a rule; neither is a unit flag.
        flags = len(re.findall(r"(?m)^\s*(?:" + CLOSED + "|" + OPEN + ")", prose))
        if unit in ("none", "whole") and flags:
            problems.append(f"{rel}: declares Seal '{raw}' but carries {flags} inline flag(s) (SR-FLAG-COVERAGE)")
            continue
        if unit in ("none", "whole"):
            if unit == "whole":
                rows.append((rel, raw, 1 if "closed" in raw else 0, 0, 0 if "closed" in raw else 1))
            continue

        us = units_of(prose, unit)
        closed = sum(1 for _, b in us if b.lstrip().startswith(CLOSED))
        opened = sum(1 for _, b in us if b.lstrip().startswith(OPEN))
        bare = len(us) - closed - opened
        if complete and bare:
            problems.append(f"{rel}: declares 'complete' but {bare} of {len(us)} units carry no flag (SR-FLAG-COVERAGE)")
        if us and closed == len(us):
            problems.append(f"{rel}: every unit is closed — promote to 'Seal: whole; closed' (SR-PROMOTE)")
        rows.append((rel, raw, closed, opened, bare))

    # SR-STATE-WHERE: the generated index must match the tree
    reg = os.path.join(ROOT, "audit/seal-list.md")
    table = ["| Document | Unit | Closed | Open on purpose | Not considered |",
             "| --- | --- | ---: | ---: | ---: |"]
    for rel, raw, c, o, b in sorted(rows):
        table.append(f"| `{rel}` | {raw} | {c} | {o} | {b} |")
    block = "\n".join(table)
    text = open(reg, encoding="utf-8").read()
    m = re.search(r"(?s)(<!-- BEGIN SEAL INDEX -->\n)(.*?)(<!-- END SEAL INDEX -->)", text)
    if not m:
        problems.append("audit/seal-list.md: no BEGIN/END SEAL INDEX markers (SR-STATE-WHERE)")
    elif m.group(2).strip() != block.strip():
        if write:
            open(reg, "w", encoding="utf-8").write(text[:m.start(2)] + block + "\n" + text[m.end(2):])
            print("audit/seal-list.md: index rewritten")
        else:
            problems.append("audit/seal-list.md: the generated index does not match the tree "
                            "(SR-STATE-WHERE) — run with --write")

    for p in problems:
        print("  VIOLATION:", p)
    print()
    if problems:
        print(f"FAIL: {len(problems)} seal finding(s).")
        return 1
    print(f"PASS: {len(rows)} document(s) with a seal unit, index in step.")
    return 0

sys.exit(main())
