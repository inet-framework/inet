import re, sys
def verdicts(path):
    result = {}
    for line in open(path, errors="replace"):
        line = re.sub(r"\x1b\[[0-9;]*m", "", line)
        m = re.match(r"^\[\s*\d+/\d+\]\s+⏺ (\S+\.test) (PASS|FAIL|ERROR|SKIP|CANCEL)(?: \((?:un)?expected\))?", line)
        if m:
            result[m.group(1)] = m.group(2)
    return result
P = "/home/levy/workspace/ghci-coverage/profiles"
for kind in ["unit", "module", "protocol"]:
    c, d = verdicts(f"{P}/m1-{kind}.log"), verdicts(f"{P}/debug-{kind}.log")
    diff = sorted(t for t in set(c) | set(d) if c.get(t) != d.get(t))
    print(f"{kind}: coverage {len(c)} tests, debug {len(d)} tests, {len(diff)} differ")
    for t in diff:
        print(f"  {t}: coverage {c.get(t)}, debug {d.get(t)}")
