#!/usr/bin/env python3
"""Pair opp_test results with a declared ``%# expected-result`` and report expected-vs-actual.

opp_test decides PASS/FAIL/ERROR purely from ``%contains`` etc. Every ``.test`` here asserts the
honest, spec-conformant verdict (``PROTOCOLTEST <name>: PASS``), so opp_test's result IS the true
conformance outcome. A ``%# expected-result: PASS|FAIL|ERROR`` comment (opp_test ignores ``%#``
lines) declares what we currently expect. This script reuses opp_repl's ``TestTaskResult`` model
to render the pair -- so a genuinely-failing NOT-MODELED test reads ``FAIL (expected)`` (green,
never faked into PASS), a regression reads ``FAIL (unexpected)``, and a gap that closes reads
``PASS (unexpected)``. It exits non-zero iff any result diverged from its expectation.

Usage: report_expected_results.py <opp_test_output_file> <test_file>...
Env:   OPP_REPL_PATH   prepend an opp_repl checkout to sys.path (else the installed one is used).
"""
import os
import re
import sys
from collections import Counter

if os.environ.get("OPP_REPL_PATH"):
    sys.path.insert(0, os.environ["OPP_REPL_PATH"])
from opp_repl.test.task import TestTaskResult  # noqa: E402  (after sys.path tweak)

_ANSI = re.compile(r"\x1b\[[0-9;]*m")
# opp_test's own buckets -> TestTaskResult vocabulary
_ACTUAL_MAP = {"EXPECTEDFAIL": "FAIL", "SKIPPED": "SKIP"}


def parse_actuals(text):
    """Map each test (by its opp_test-reported path) to PASS/FAIL/ERROR/SKIP.

    opp_test prints one line per test as ``*** <path>: <RESULT> (<reason>)``."""
    text = _ANSI.sub("", text)
    actuals = {}
    for m in re.finditer(r"^\*\*\*\s+(.+?):\s+(PASS|FAIL|ERROR|EXPECTEDFAIL|SKIPPED)\b", text, re.M):
        actuals[os.path.normpath(m.group(1).strip())] = _ACTUAL_MAP.get(m.group(2), m.group(2))
    return actuals


def declared_expected(test_path):
    """Read ``%# expected-result: <VALUE>`` from a .test file (default PASS)."""
    try:
        with open(test_path) as f:
            for line in f:
                m = re.match(r"^%#\s*expected-result\s*:\s*(\w+)\s*$", line)
                if m:
                    value = m.group(1).upper()
                    if value not in ("PASS", "FAIL", "ERROR"):
                        sys.exit(f"error: invalid '%# expected-result: {m.group(1)}' in "
                                 f"{test_path} (allowed: PASS, FAIL, ERROR)")
                    return value
    except OSError:
        pass
    return "PASS"


def main(argv):
    if len(argv) < 2:
        print("usage: report_expected_results.py <opp_test_output_file> <test_file>...", file=sys.stderr)
        return 2
    out_file, test_files = argv[1], argv[2:]
    with open(out_file) as f:
        actuals = parse_actuals(f.read())
    by_base = {}
    for k, v in actuals.items():
        by_base.setdefault(os.path.basename(k), v)

    results = []
    for tf in test_files:
        actual = actuals.get(os.path.normpath(tf)) or by_base.get(os.path.basename(tf)) or "ERROR"
        expected = declared_expected(tf)
        name = os.path.splitext(os.path.basename(tf))[0]
        # Reuse opp_repl's model: it computes .expected = (result == expected_result) and
        # renders "(expected)"/"(unexpected)" (with the fixed display, an unexpected PASS is loud).
        results.append((name, TestTaskResult(task=None, result=actual, expected_result=expected)))

    for name, r in sorted(results):
        print(f"  {name}: {r.get_description()}")

    expected_counts, unexpected_counts = Counter(), Counter()
    for _, r in results:
        (expected_counts if r.expected else unexpected_counts)[r.result] += 1
    parts = [f"{n} {res} (expected)" for res, n in sorted(expected_counts.items())]
    parts += [f"{n} {res} (unexpected)" for res, n in sorted(unexpected_counts.items())]
    n_unexpected = sum(unexpected_counts.values())
    print(f"\n{len(results)} tests: " + ", ".join(parts))
    if n_unexpected:
        print(f"UNEXPECTED: {n_unexpected} result(s) diverged from the declared expectation "
              f"(a regression, or a closed gap -- update the matrix).")
        return 1
    print("All results as expected.")
    return 0


if __name__ == "__main__":
    sys.exit(main(sys.argv))
