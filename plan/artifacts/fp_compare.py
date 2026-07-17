#!/usr/bin/env python3
"""Compare two opp_run_fingerprint_tests logs subject-by-subject.

Parses the 'Checking fingerprint <params> <RESULT> ... calculated <fp> (correct <fp>)'
lines and compares the CALCULATED fingerprints between the two runs (the store
baselines are dead on the topic/infrastructure omnetpp companion, so PASS/FAIL
vs the store is meaningless; run-to-run identity is the parity signal).

Usage: fp_compare.py <reference.log> <subject.log>
"""
import re
import sys
from collections import defaultdict

LINE_RE = re.compile(
    r'(?P<params>(?:examples|showcases|tutorials|tests)/.+?) (?P<result>PASS|FAIL|ERROR)'
    r'(?: \((?:un)?expected\))?'
    r'(?: \((?P<reason>[^)]*)\))?'
    r'(?:.*? calculated (?P<calc>\S+))?'
)
ANSI_RE = re.compile(r'\x1b\[[0-9;]*m')


def parse(path):
    fps = {}      # (params, ingredients) -> calculated fp
    errors = {}   # params -> reason
    with open(path, errors='replace') as f:
        for line in f:
            line = ANSI_RE.sub('', line)
            m = LINE_RE.search(line)
            if not m:
                continue
            params = m.group('params')
            if m.group('result') == 'ERROR':
                errors[params] = m.group('reason')
            elif m.group('calc'):
                fp = m.group('calc')
                ingredients = fp.split('/', 1)[1] if '/' in fp else ''
                fps[(params, ingredients)] = fp
    return fps, errors


def group(keys):
    g = defaultdict(int)
    for params, _ in keys:
        g['/'.join(params.split('/')[:3]).split(' ')[0]] += 1
    return sorted(g.items(), key=lambda kv: -kv[1])


def main():
    ref_fps, ref_errs = parse(sys.argv[1])
    sub_fps, sub_errs = parse(sys.argv[2])

    both = ref_fps.keys() & sub_fps.keys()
    same = {k for k in both if ref_fps[k] == sub_fps[k]}
    diff = both - same
    ref_only = ref_fps.keys() - sub_fps.keys()
    sub_only = sub_fps.keys() - ref_fps.keys()

    print(f"reference: {len(ref_fps)} fingerprints, {len(ref_errs)} ERROR subjects")
    print(f"subject:   {len(sub_fps)} fingerprints, {len(sub_errs)} ERROR subjects")
    print(f"compared:  {len(both)}  identical: {len(same)}  differing: {len(diff)}")
    print(f"reference-only: {len(ref_only)}  subject-only: {len(sub_only)}")

    for title, keys in (("DIFFERING", diff), ("IDENTICAL", same)):
        print(f"\n{title} by example directory:")
        for d, n in group(keys):
            print(f"  {n:5d}  {d}")

    for title, only in (("REFERENCE-ONLY", ref_only), ("SUBJECT-ONLY", sub_only)):
        if only:
            print(f"\n{title} subjects:")
            for k in sorted(only):
                print(f"  {k[0]} [{k[1]}]")

    print("\nERROR subjects only in reference:")
    for p in sorted(ref_errs.keys() - sub_errs.keys()):
        print(f"  {p}: {ref_errs[p]}")
    print("ERROR subjects only in subject:")
    for p in sorted(sub_errs.keys() - ref_errs.keys()):
        print(f"  {p}: {sub_errs[p]}")
    print(f"ERROR subjects in both: {len(ref_errs.keys() & sub_errs.keys())}")


if __name__ == '__main__':
    main()
