#!/usr/bin/env python3
#
# The cross-class check of the parameter tests of TCP.
#
# A parameter test must fail when its parameter has the value of another class: a test that
# passes for every value adds coverage, but no evidence. For each selected test, this script
# takes the value of each other class of the parameter from the catalog, runs the test again
# with that value on the command line (--<key>=<value>, which takes the place of the ini line
# of the test), and expects the verdict FAIL. A class whose observation is E (an error at the
# open) is no alternative, and a class with the same value as the test is not either: a facet.
#
# The script reruns the binary that the last run of the test built, in
# tests/protocol/tcp/work/<test>/, so run the tests first, from an INET environment:
#
#   opp_run_protocol_tests -m debug -w tcp -f parameter/ack
#   python3 tests/protocol/tcp/parameter/cross_class.py ack
#
# The arguments select the tests: a folder of a group ("ack"), or test names. Exit code 1 if
# a test passes with the value of another class, or if a run gives no verdict.
#
# SPDX-License-Identifier: LGPL-3.0-or-later
#

import os
import re
import subprocess
import sys

import check_catalog as catalog

SUITE = os.path.normpath(os.path.join(catalog.HERE, ".."))
VERDICT = re.compile(r"^PROTOCOLTEST (\S+): (PASS|FAIL)", re.MULTILINE)


def selected(path, selectors):
    relative = os.path.relpath(path, catalog.HERE)
    name = os.path.splitext(os.path.basename(path))[0]
    return not selectors or any(relative.split(os.sep)[0] == s or name == s for s in selectors)


def run(test_name, key, value):
    work = os.path.join(SUITE, "work", test_name)
    if not os.path.exists(os.path.join(work, "run")):
        return "NOT BUILT", ""
    result = subprocess.run(["sh", "run", "--%s=%s" % (key, value)], cwd=work, capture_output=True, text=True)
    verdicts = VERDICT.findall(result.stdout)
    if not verdicts:
        lines = [l for l in (result.stdout + result.stderr).splitlines() if "<!>" in l or "rror" in l]
        return "NO VERDICT", lines[0][:160] if lines else ""
    reason = re.search(r"^  reason: (.*)$", result.stdout, re.MULTILINE)
    return verdicts[-1][1], reason.group(1)[:160] if reason else ""


def main():
    selectors = sys.argv[1:]
    rows = catalog.read_catalog()
    tests = catalog.read_tests()
    by_test = {row["test"]: row for row in rows if row["test"]}
    problems, runs = 0, 0
    for path in sorted(tests):
        if not selected(path, selectors):
            continue
        row = by_test.get(path)
        meta = tests[path]
        name = os.path.splitext(os.path.basename(path))[0]
        if row is None:
            print("%s: no catalog row, skipped" % name)
            continue
        if "E" in row["observation"]:
            continue
        # one run for each other value: the facets of one value are one alternative
        alternatives = {}
        for r in rows:
            if (r["module"], r["parameter"]) == (row["module"], row["parameter"]) and r["value"] != row["value"] \
                    and "E" not in r["observation"]:
                alternatives.setdefault(r["value"], r)
        alternatives = list(alternatives.values())
        if not alternatives:
            print("%-48s no other class to try" % name)
            continue
        for alternative in alternatives:
            verdict, reason = run(name, meta["key"], alternative["value"])
            runs += 1
            good = verdict == "FAIL"
            problems += 0 if good else 1
            print("%-48s with %-10s (%s): %s%s" % (name, alternative["value"], alternative["cls"], verdict,
                                                   "" if good else "  <-- " + (reason or "the test does not separate the classes")))
    print("%d run(s), %d problem(s)" % (runs, problems))
    return 0 if problems == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
