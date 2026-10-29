#!/usr/bin/env python3
#
# The completeness check of the parameter tests of TCP.
#
# Reads three sources and reports where they disagree:
#   - the NED files of the TCP modules: the parameters, and the defaults that LinuxTcp and
#     LegacyTcp set;
#   - the catalog doc/project/evidence/model/tcp/parameters.md: one row for each class of
#     values of a parameter;
#   - the test files below this folder: the %# lines that name the parameter, the class, the
#     value and the ini key of each test.
#
# Errors: a row of a parameter that does not exist; a row without its test file, or a test
# file without its row; a test whose %# lines differ from its row, or whose ini does not set
# its key to its value; a default of a preset that no row of its parameter names. A parameter
# without rows is listed as pending; with --strict, it is an error too.
#
# Usage: python3 tests/protocol/tcp/parameter/check_catalog.py [--strict]   (from any folder)
#
# SPDX-License-Identifier: LGPL-3.0-or-later
#

import os
import re
import sys

HERE = os.path.dirname(os.path.abspath(__file__))
ROOT = os.path.normpath(os.path.join(HERE, "../../../.."))
CATALOG = os.path.join(ROOT, "doc/project/evidence/model/tcp/parameters.md")
NED_FOLDERS = ["src/inet/transportlayer/tcp"]
NED_FILES = ["src/inet/transportlayer/tcp_common/TcpSpoof.ned"]
PRESETS = ["LinuxTcp", "LegacyTcp"]

DECLARATION = re.compile(r"^\s+(?:volatile\s+)?(?:bool|int|double|string)\s+(\w+)")
ASSIGNMENT = re.compile(r"^\s+([\w*.]+)\s*=\s*default\((.*)\);")
MODULE = re.compile(r"^\s*simple\s+(\w+)")
ROW = re.compile(r"^\|\s*`(\w+)\.(\w+):([\w-]+)`\s*\|(.*)$")
LINK = re.compile(r"\]\(([^)]+\.test)\)")
META = re.compile(r"^%#\s*(parameter|class|value|key|expected-result)\s*:\s*(.*?)\s*$")


def strip_comment(line):
    return line.split("//", 1)[0]


def read_ned():
    """Returns {module: [parameter, ...]} and {preset: {(module, parameter): value}}."""
    files = [os.path.join(ROOT, f) for f in NED_FILES]
    for folder in NED_FOLDERS:
        for dirpath, _, names in os.walk(os.path.join(ROOT, folder)):
            files += [os.path.join(dirpath, n) for n in names if n.endswith(".ned")]
    parameters, presets = {}, {p: {} for p in PRESETS}
    for path in sorted(files):
        module = None
        for line in open(path, encoding="utf-8"):
            line = strip_comment(line)
            m = MODULE.match(line)
            if m:
                module = m.group(1)
                parameters.setdefault(module, [])
                continue
            if module is None:
                continue
            m = DECLARATION.match(line)
            if m:
                parameters[module].append(m.group(1))
                continue
            m = ASSIGNMENT.match(line)
            if m and module in presets:
                name, value = m.group(1), m.group(2).strip()
                # conn*.algorithm.hystartEnabled: a parameter of the algorithm module
                target = ("TcpCubic", name.rsplit(".", 1)[1]) if name.startswith("conn") else ("Tcp", name)
                presets[module][target] = value
    return {m: p for m, p in parameters.items() if p}, presets


def read_catalog():
    """Returns the rows: dicts of module, parameter, cls, value, preset, test (path or None)."""
    rows = []
    if not os.path.exists(CATALOG):
        return rows
    for number, line in enumerate(open(CATALOG, encoding="utf-8"), 1):
        m = ROW.match(line)
        if not m:
            continue
        cells = [c.strip() for c in m.group(4).split("|")]
        link = LINK.search(line)
        # the columns after the class: value, preset, claim, source, observation, network, test, verdict
        rows.append({
            "line": number, "module": m.group(1), "parameter": m.group(2), "cls": m.group(3),
            "value": cells[0].strip("`") if cells else "",
            "preset": cells[1] if len(cells) > 1 else "",
            "observation": cells[4] if len(cells) > 4 else "",
            "test": os.path.normpath(os.path.join(os.path.dirname(CATALOG), link.group(1))) if link else None,
        })
    return rows


def read_tests():
    """Returns {path: {meta..., 'ini': [lines]}} for each .test file below this folder."""
    tests = {}
    for dirpath, dirnames, names in os.walk(HERE):
        dirnames[:] = [d for d in dirnames if d != "work"]
        for name in names:
            if not name.endswith(".test"):
                continue
            path = os.path.join(dirpath, name)
            meta, ini, section = {}, [], None
            for line in open(path, encoding="utf-8"):
                m = META.match(line)
                if m:
                    meta[m.group(1)] = m.group(2)
                    continue
                if line.startswith("%"):
                    section = line.strip()
                    continue
                if section is not None and section.startswith("%inifile"):
                    ini.append(line.strip())
            meta["ini"] = ini
            tests[os.path.normpath(path)] = meta
    return tests


def main():
    strict = "--strict" in sys.argv[1:]
    parameters, presets = read_ned()
    rows = read_catalog()
    tests = read_tests()
    errors, pending = [], []

    def error(text):
        errors.append(text)

    known = {(m, p) for m, ps in parameters.items() for p in ps}
    rows_by_parameter = {}
    for row in rows:
        key = (row["module"], row["parameter"])
        rows_by_parameter.setdefault(key, []).append(row)
        where = "catalog line %d (%s.%s:%s)" % (row["line"], row["module"], row["parameter"], row["cls"])
        if key not in known:
            error("%s: %s.%s is not a parameter of the TCP modules" % (where, *key))
        if row["test"] is None:
            error("%s: the row names no test file" % where)
            continue
        meta = tests.get(row["test"])
        if meta is None:
            error("%s: the test file %s does not exist" % (where, os.path.relpath(row["test"], ROOT)))
            continue
        name = "%s.%s" % key
        if meta.get("parameter") != name or meta.get("class") != row["cls"]:
            error("%s: the test says %%# parameter: %s, %%# class: %s" % (where, meta.get("parameter"), meta.get("class")))
        if meta.get("value") != row["value"]:
            error("%s: the row has the value %s, the test %s" % (where, row["value"], meta.get("value")))
        ini_key = meta.get("key")
        if not ini_key:
            error("%s: the test has no %%# key" % where)
        elif not any(re.match(r"^%s\s*=\s*%s\s*(#.*)?$" % (re.escape(ini_key), re.escape(row["value"])), l) for l in meta["ini"]):
            error("%s: the ini of the test does not set %s = %s" % (where, ini_key, row["value"]))

    linked = {row["test"] for row in rows if row["test"]}
    for path in sorted(tests):
        if path not in linked:
            error("%s: no catalog row names this test" % os.path.relpath(path, ROOT))

    for module in sorted(parameters):
        for parameter in parameters[module]:
            if (module, parameter) not in rows_by_parameter:
                pending.append("%s.%s" % (module, parameter))

    for preset, values in presets.items():
        for key, value in sorted(values.items()):
            if key in rows_by_parameter and not any(preset in r["preset"] for r in rows_by_parameter[key]):
                error("%s sets %s.%s = %s, and no row of the parameter names %s" % (preset, *key, value, preset))

    total = sum(len(p) for p in parameters.values())
    print("parameters: %d, with rows: %d, pending: %d; rows: %d; tests: %d"
          % (total, total - len(pending), len(pending), len(rows), len(tests)))
    if pending and (strict or "--pending" in sys.argv[1:]):
        print("pending: " + ", ".join(pending))
    for text in errors:
        print("ERROR: " + text)
    if strict:
        errors += pending
    print("PASS" if not errors else "FAIL: %d error(s)" % len(errors))
    return 0 if not errors else 1


if __name__ == "__main__":
    sys.exit(main())
