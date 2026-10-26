import sys
sys.path.insert(0, "/home/levy/workspace/opp_repl-coverage-tests")
from opp_repl.test.coverage import read_lcov_files, merge_lcov_tests
P = "/home/levy/workspace/ghci-coverage/profiles"
def covered(files):
    merged = merge_lcov_tests(read_lcov_files(files))
    lines = {(s, l) for s, r in merged.items() for l, c in r["lines"].items() if c}
    functions = {(s, f) for s, r in merged.items() for f, c in r["functions"].items() if c}
    branches = {(s, b) for s, r in merged.items() for b, c in r["branches"].items() if c}
    return lines, functions, branches
suites = {"fingerprint": [f"fingerprint-{i}" for i in range(4)], "unit": ["unit"], "module": ["module"], "packet": ["packet"], "queueing": ["queueing"], "protocol": ["protocol"]}
m1_names = {"packet": "packet2"}
all1, all2 = [], []
for suite, jobs in suites.items():
    f1 = [f"{P}/m1-{m1_names.get(j, j)}.lcov" for j in jobs]; f2 = [f"{P}/m2-{j}.lcov" for j in jobs]
    all1 += f1; all2 += f2
    a, b = covered(f1), covered(f2)
    print(suite, "lines only m1/only m2:", len(a[0] - b[0]), len(b[0] - a[0]), " functions:", len(a[1] - b[1]), len(b[1] - a[1]), " branches:", len(a[2] - b[2]), len(b[2] - a[2]), " covered lines:", len(a[0]), len(b[0]))
a, b = covered(all1), covered(all2)
print("ALL lines only m1/only m2:", len(a[0] - b[0]), len(b[0] - a[0]), " functions:", len(a[1] - b[1]), len(b[1] - a[1]), " branches:", len(a[2] - b[2]), len(b[2] - a[2]), " covered lines:", len(a[0]), len(b[0]))
for name, diff in [("only m1", sorted(a[0] - b[0])), ("only m2", sorted(b[0] - a[0]))]:
    files = {}
    for s, l in diff: files[s] = files.get(s, 0) + 1
    print(name, sorted(files.items(), key=lambda x: -x[1])[:12])
