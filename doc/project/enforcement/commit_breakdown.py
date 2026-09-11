# Reads "<subject>\t<Change: value>" lines on stdin; prints the breakdown tables.
import sys, collections
ORDER = ["comment", "format", "location", "name", "refactor", "behavior"]
rows = []
for line in sys.stdin:
    if not line.strip(): continue
    s, _, t = line.rstrip("\n").partition("\t")
    f = [x.strip() for x in t.split("|")] if t.strip() else []
    rows.append(dict(s=s, scope=f[0] if f else "", kind=f[1] if len(f) > 1 else "",
                     obl=f[2] if len(f) > 2 else "", grp=f[3] if len(f) > 3 else ""))
n = len(rows)
def depths(k):
    """The depth tokens. A '+' inside the direction is not a depth mix:
    behavior.add+change is one depth, name+refactor is two."""
    if not k: return ["?"]
    out = []
    for p in k.split("+"):
        head = p.split(".")[0]
        if head in ORDER: out.append(head)
    return out or ["?"]
def depth_key(k): return "+".join(depths(k))
def area(s):   return s.split(".")[0] if s else "?"
def dirs(k):
    """Direction and intent, counted independently: a fix is also a change."""
    out = []
    for seg in k.split("."):
        for w in seg.split("+"):
            if w in ("add", "change", "remove", "fix"): out.append(w)
    return out
def table(title, c, order=None):
    if not c: return
    print(f"\n**{title}**\n\n| Value | Commits |\n| --- | --- |")
    keys = [k for k in (order or []) if k in c] + [k for k in c if not order or k not in order]
    for k in keys: print(f"| `{k}` | {c[k]} |")
d = collections.Counter(depth_key(r["kind"]) for r in rows)
print(f"**{n} commits.**")
table("By depth", d, ORDER)
inert = sum(1 for r in rows if "behavior" not in depths(r["kind"]))
print(f"\n**{inert} of {n} commits need no behavioral review** — every depth below `behavior`.")
table("By direction and intent", collections.Counter(x for r in rows for x in dirs(r["kind"])),
      ["add", "change", "remove", "fix"])
if n > 10:
    ds = [k for k in ORDER if d[k]] + sorted(k for k in d if k not in ORDER)
    print("\n**Area against depth**\n\n| Area | " + " | ".join(f"`{x}`" for x in ds) + " |")
    print("| --- |" + " --- |" * len(ds))
    for a in sorted({area(r["scope"]) for r in rows}):
        c = collections.Counter(depth_key(r["kind"]) for r in rows if area(r["scope"]) == a)
        print(f"| `{a}` | " + " | ".join(str(c[x]) if c[x] else "" for x in ds) + " |")
g = collections.Counter(r["grp"] or "(none)" for r in rows)
if len(g) > 1: table("By group", g)
o = collections.Counter(x for r in rows for x in r["obl"].split() if x != "-")
table("Obligations the series owes", o)
