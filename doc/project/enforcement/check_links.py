#!/usr/bin/env python3
"""Check every relative markdown link in a tree: the file exists, and the #anchor exists."""
import os, re, sys, unicodedata

def slug(heading):
    s = heading.strip().lower()
    s = "".join(c for c in s if c.isalnum() or c in " -_")
    return s.replace(" ", "-")

def anchors(path):
    out = set()
    seen = {}
    for line in open(path, encoding="utf-8"):
        m = re.match(r"^(#{1,6})\s+(.*?)\s*$", line)
        if m:
            base = slug(re.sub(r"[`*_]", "", m.group(2)))
            n = seen.get(base, 0)
            seen[base] = n + 1
            out.add(base if n == 0 else f"{base}-{n}")
    return out

root = sys.argv[1] if len(sys.argv) > 1 else "doc/project"
files = []
for d, _, fs in os.walk(root):
    for f in fs:
        if f.endswith(".md"):
            files.append(os.path.join(d, f))

cache = {}
bad = 0
for f in sorted(files):
    text = open(f, encoding="utf-8").read()
    text = re.sub(r"(?ms)^```.*?^```", "", text)      # fenced blocks
    text = re.sub(r"(?s)<!--.*?-->", "", text)         # html comments (templates)
    text = re.sub(r"`[^`\n]*`", "", text)             # inline code spans
    for m in re.finditer(r"\[([^\]]*)\]\(([^)\s]+)\)", text):
        target = m.group(2)
        if target.startswith(("http://", "https://", "mailto:")):
            continue
        path, _, frag = target.partition("#")
        base = os.path.dirname(f)
        full = os.path.normpath(os.path.join(base, path)) if path else f
        if not os.path.exists(full):
            print(f"MISSING FILE  {f}: [{m.group(1)}]({target})")
            bad += 1
            continue
        if frag and full.endswith(".md"):
            if full not in cache:
                cache[full] = anchors(full)
            if frag.lower() not in cache[full]:
                print(f"MISSING ANCHOR {f}: [{m.group(1)}]({target})")
                bad += 1
print(f"--- {len(files)} files, {bad} broken links ---")
sys.exit(1 if bad else 0)
