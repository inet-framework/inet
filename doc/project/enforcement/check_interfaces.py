#!/usr/bin/env python3
"""The interface gate: a C++ class named I<Stem> holds no implementation.

The rules are NR-CPP-TYPE (the I<Stem> prefix is a promise) and AR-ORG-CONTRACT-PURITY (what the
promise means).  An interface may hold only:

  - pure virtual declarations            virtual T f(...) = 0;
  - a virtual destructor                  virtual ~I() {}   /  = default;
  - signal identities                     static simsignal_t xSignal;
  - type declarations                     enum, typedef, using, a nested class or struct, friend
  - inheritance from other interfaces     an empty body is fine

Anything else is a finding: a method with a body, a non-virtual function, a data member, a static
that is not a signal.  A class named I<Stem> with no pure virtual at all is a naming finding: it is
not an interface and must not carry the prefix.

Run it through check-interfaces.sh, which enters the repository root first.  Exit 0 = clean.
"""
import glob, os, re, sys
from collections import Counter

# Names that are sanctioned in audit/naming-exceptions.md and are not interfaces.
SANCTIONED_NOT_INTERFACE = {"IPsec", "IPsecRule"}          # NS-01: the canonical spelling

# AS-02 in audit/architecture-exceptions.md: a static that names an enum the contract itself
# declares is part of its vocabulary, like a signal identity.  Matched by name shape.
ENUM_NAME_HELPER = re.compile(r"^static\s+(const\s+char\s*\*\s*\w*(Name|Str)\s*\(|cEnum\s*\*)")

# AS-04: a named scalar constant the contract defines is vocabulary too, like an enum value.
NAMED_CONSTANT = re.compile(r"^static\s+const(expr)?\s+(int|bool|double|unsigned|long|short|char|uint\d+_t|int\d+_t)\s+[A-Z_][A-Z0-9_]*\s*=\s*[-\w.]+\s*;")

# AS-03: a body that computes only from the class's own pure virtuals cannot hide a break --
# rename the pure virtual and the body stops compiling.  That is the hazard the rule exists for,
# and it is absent here.  Sanctioned by class, because the shape is not cheap to prove by regex.
SANCTIONED_FORWARDERS = {
    "L3Socket.h::ICallback", "Ipv4Socket.h::ICallback", "Ipv6Socket.h::ICallback",  # narrow-type shims
    "IPacketComparatorFunction.h::IPacketComparatorFunction",                        # less() -> comparePackets()
    "IL3AddressType.h::IL3AddressType",                                              # bytes = (bits + 7) / 8
    "IIeee80211Mode.h::IIeee80211Mode",                                              # const_cast wrappers
}

CLASS = re.compile(r"^\s*class\s+(?:INET_API\s+)?(?!INET_API\b)(I[A-Z]\w*)\b(?!.*;\s*$)")

def logical_lines(path):
    buf = ""
    for line in open(path, encoding="utf-8", errors="replace"):
        t = line.split("//")[0].rstrip("\n")
        buf = (buf + " " + t.strip()) if buf else t
        if buf.count("(") > buf.count(")") and len(buf) < 3000:
            continue
        yield buf
        buf = ""
    if buf:
        yield buf

def classify(st):
    if re.match(r"^(public|protected|private)\s*:", st) or st in ("{", "};", "}", ""):
        return None
    if re.search(r"\bvirtual\b.*=\s*0\s*;", st):                                     return "pure"
    if re.search(r"\bvirtual\s+~\w+\(\)\s*(\{\s*\}|=\s*default\s*;|;)", st):          return "dtor"
    if re.match(r"^static\s+(const\s+)?simsignal_t\b", st):                          return "signal"
    if re.match(r"^(typedef|using|enum|struct|class|friend)\b", st):                 return "type"
    if re.search(r"\bvirtual\b.*\)\s*(const\s*)?(override\s*)?\{\s*\}\s*$", st):     return "BODY: no-op default"
    if re.search(r"\bvirtual\b.*\)\s*(const\s*)?(override\s*)?\{", st):              return "BODY: implementation"
    if re.search(r"\bvirtual\b.*\)\s*(const\s*)?;\s*$", st):                          return "virtual declared, not pure"
    if re.match(r"^static\b", st):                                                    return "static, not a signal"
    if re.search(r"\w+\s*\([^)]*\)\s*(const\s*)?(\{|;)", st) and not re.match(r"^\w[\w:<>,\s\*&]*\s+\w+\s*(=|;)", st):
        return "non-virtual function"
    if re.search(r"[;=]", st) and not st.startswith(("#", "/*", "*")):
        return "data member"
    return None

def scan(root):
    out = []
    for fn in sorted(glob.glob(os.path.join(root, "**/*.h"), recursive=True)):
        if fn.endswith("_m.h"):
            continue
        lines = list(logical_lines(fn))
        i = 0
        while i < len(lines):
            m = CLASS.match(lines[i])
            if not m:
                i += 1
                continue
            name, depth, body, started, j = m.group(1), 0, [], False, i
            while j < len(lines):
                s = lines[j]
                if not started:
                    if "{" in s:
                        started = True
                        depth += s.count("{") - s.count("}")
                    j += 1
                    continue
                body.append(s)
                depth += s.count("{") - s.count("}")
                if depth <= 0:
                    break
                j += 1
            i = j + 1
            kinds, nested, evidence = [], 0, []
            for raw in body:
                st = raw.strip()
                # Inside a nested type or a member body: those lines belong to it, not to the
                # interface.  A nested struct's fields are not the interface's data members, and
                # the statements of a default body are not extra functions -- the body itself is
                # the finding, counted once on its opening line.
                if nested > 0:
                    nested += st.count("{") - st.count("}")
                    continue
                opens = st.count("{") - st.count("}")
                if re.match(r"^(enum|struct|class|union)\b", st):
                    kinds.append("type")
                    nested = opens
                    continue
                if ENUM_NAME_HELPER.match(st) or NAMED_CONSTANT.match(st):
                    kinds.append("type")
                    continue
                if re.match(r"^(protected|private|public)\s*:\s*I\w*\(\)\s*(\{\s*\}|=\s*default;)", st) or re.match(r"^I\w*\(\)\s*(\{\s*\}|=\s*default;)\s*$", st):
                    kinds.append("dtor")          # a trivial default constructor is the destructor's twin
                    continue
                k = classify(st)
                if k and k.startswith("BODY") and opens > 0:
                    nested = opens                # a multi-line body: skip its statements
                if k:
                    kinds.append(k)
                    if k not in ("pure", "dtor", "signal", "type"):
                        evidence.append((k, st[:110]))
            counts = Counter(kinds)
            out.append((os.path.relpath(fn, root), name, counts, evidence))
    return out

def main():
    verbose = "--verbose" in sys.argv
    args = [a for a in sys.argv[1:] if not a.startswith("--")]
    root = args[0] if args else "src/inet"
    problems, notes, clean = [], [], 0
    for fn, name, c, ev in scan(root):
        if name in SANCTIONED_NOT_INTERFACE:
            continue
        if f"{os.path.basename(fn)}::{name}" in SANCTIONED_FORWARDERS:
            clean += 1
            continue
        bad = {k: v for k, v in c.items() if k not in ("pure", "dtor", "signal", "type")}
        # Clean: has a pure virtual and nothing else, or is an empty marker that only inherits.
        # A non-empty body with no pure virtual at all is not an interface, whatever it holds:
        # a class of enums and constants over a non-interface base is a value type, not a role.
        if not bad and (c["pure"] or not c):
            clean += 1
            continue
        if not bad:
            # Only types, no pure virtual of its own.  Either a marker interface that inherits its
            # role and declares a type -- INetworkProtocol, IPhysicalLayer -- or an enum holder
            # that is not an interface at all -- IRadioSignal.  Telling them apart means resolving
            # the base classes, which this gate does not do.  A note, for a reviewer.
            notes.append((fn, name))
            continue
        if not c["pure"]:
            problems.append((fn, name, "NOT AN INTERFACE (no pure virtual) -- rename it, or make it one", bad, ev))
        elif bad:
            problems.append((fn, name, "holds implementation", bad, ev))
    for fn, name, why, bad, ev in problems:
        detail = ", ".join(f"{v} {k}" for k, v in sorted(bad.items()))
        print(f"  VIOLATION: {name:36s} {fn}\n             {why}: {detail}")
        if verbose:
            for k, line in ev:
                print(f"               [{k}]  {line}")
    for fn, name in notes:
        print(f"  note: {name:36s} {fn}\n        no pure virtual of its own and only type declarations: an interface by"
              f" inheritance, or not one at all -- check its bases")
    print()
    if problems:
        print(f"FAIL: {len(problems)} class(es) named I<Stem> break the promise; {clean} keep it; {len(notes)} need a reviewer.")
        print("      A body belongs in <Stem>Base (AR-ORG-CONTRACT-PURITY); a class that is not an")
        print("      interface must not carry the prefix (NR-CPP-TYPE). Record a sanctioned case as an")
        print("      NS-* or AS-* row, and add its name to SANCTIONED_NOT_INTERFACE here if it is a name.")
        return 1
    print(f"PASS: {clean} interface(s) keep the I<Stem> promise; {len(notes)} need a reviewer.")
    return 0

sys.exit(main())
