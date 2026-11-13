#!/usr/bin/env python3
"""Tests of _scripts/github/baseline-candidates.py, which makes the candidate baseline files after a
failed run of the statistical or chart tests. A fake update command takes the place of opp_repl."""

import importlib.util
import json
import os
from pathlib import Path
import re
import subprocess
import sys
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[3]
SCRIPT = ROOT / "_scripts" / "github" / "baseline-candidates.py"
spec = importlib.util.spec_from_file_location("baseline_candidates", SCRIPT)
candidates = importlib.util.module_from_spec(spec)
spec.loader.exec_module(candidates)

SCA = 'version 3\nrun General-0-x\nattr experiment General\nattr measurement ""\nattr replication #0\nscalar Net.host "packets sent" {}\n'
OPP = 'SimulationProject(name="inet", test_parameters={"statistical": {"baseline": {"repository": "x/statistics", "ref": "%s", "folder": "statistics"}}})\n'


def git(cwd, *args):
    return subprocess.run(["git", "-c", "user.name=test", "-c", "user.email=test@example.invalid", *args], cwd=cwd,
                          check=True, capture_output=True, text=True).stdout.strip()


def result(parameters, result="FAIL", expected=False, reason=None):
    return {"parameters": parameters, "result": result, "expected": expected, "reason": reason}


class SelectionTest(unittest.TestCase):
    def test_an_unexpected_fail_and_a_missing_stored_result_need_a_candidate(self):
        results = [result("examples/a for 10s"),
                   result("examples/b for 10s", "ERROR", reason="Stored statistical results are not found"),
                   result("showcases/c/C.anf: Chart", "FAIL", reason="Baseline chart not found"),
                   result("examples/d for 10s", "ERROR", reason="The simulation stopped with an error"),
                   result("examples/e for 10s", "FAIL", expected=True),
                   result("examples/f for 10s", "PASS", expected=True)]
        self.assertEqual(candidates.selected_tests(results), ["examples/a for 10s", "examples/b for 10s", "showcases/c/C.anf: Chart"])

    def test_the_statistical_filter_selects_exactly_the_tests(self):
        option, value = candidates.test_filter("statistical", ["examples/a -c Wifi for 10s", "showcases/b -f study.ini -c Run for 5s",
                                                                "tutorials/c for 1s"])
        self.assertEqual(option, "-f")
        selected = ["SimulationConfig(working_directory=examples/a, ini_file=omnetpp.ini, config=Wifi)",
                    "SimulationConfig(working_directory=showcases/b, ini_file=study.ini, config=Run)",
                    "SimulationConfig(working_directory=tutorials/c, ini_file=omnetpp.ini, config=General)"]
        for text in selected:
            self.assertIsNotNone(re.search(value, text), text)
        for text in ["SimulationConfig(working_directory=examples/a, ini_file=omnetpp.ini, config=WifiLong)",
                     "SimulationConfig(working_directory=examples/ab, ini_file=omnetpp.ini, config=Wifi)"]:
            self.assertIsNone(re.search(value, text), text)

    def test_a_statistical_test_of_another_run_is_an_error(self):
        with self.assertRaises(ValueError):
            candidates.test_filter("statistical", ["examples/a -c Wifi -r 2 for 10s"])

    def test_the_chart_filter_selects_the_folders_of_the_analysis_files(self):
        option, value = candidates.test_filter("chart", ["showcases/a/A.anf: One", "showcases/a/A.anf: Two", "showcases/b/x/B.anf: Three"])
        self.assertEqual(option, "-w")
        self.assertEqual(value, "^(showcases/a|showcases/b/x)$")


class ScriptTest(unittest.TestCase):
    """The script in an INET-like work tree with a clone of the statistics repository."""

    def setUp(self):
        self.tempdir = tempfile.TemporaryDirectory()
        self.root = Path(self.tempdir.name) / "inet"
        statistics = self.root / "statistics"
        statistics.mkdir(parents=True)
        git(statistics, "init", "-q", "-b", "master")
        for name in ("a", "b"):
            (statistics / f"examples/{name}").mkdir(parents=True)
            (statistics / f"examples/{name}/omnetpp.ini-General-#0.sca").write_text(SCA.format(10))
        git(statistics, "add", "-A")
        git(statistics, "commit", "-q", "-m", "first")
        (self.root / "inet.opp").write_text(OPP % git(statistics, "rev-parse", "HEAD"))
        # the fake update writes new values for each test that its filter selects, and a work file
        self.update = Path(self.tempdir.name) / "update.py"
        self.update.write_text(
            "import re, sys\n"
            "value = sys.argv[sys.argv.index('-f') + 1]\n"
            "for name in ('a', 'b'):\n"
            "    if re.search(value, f'SimulationConfig(working_directory=examples/{name}, ini_file=omnetpp.ini, config=General)'):\n"
            f"        open(f'statistics/examples/{{name}}/omnetpp.ini-General-#0.sca', 'w').write({SCA!r}.format(11))\n"
            "open('statistics/examples/a/omnetpp.ini-General-#0.diff', 'w').write('work file')\n")
        self.output = Path(self.tempdir.name) / "out"

    def tearDown(self):
        self.tempdir.cleanup()

    def run_script(self, results):
        result_file = Path(self.tempdir.name) / "results.json"
        result_file.write_text(json.dumps({"results": results}))
        return subprocess.run([sys.executable, str(SCRIPT), "statistical", str(result_file), str(self.output),
                               "--update", f"{sys.executable} {self.update}"], cwd=self.root, capture_output=True, text=True)

    def test_the_candidates_of_the_failed_tests_and_their_block(self):
        out = self.run_script([result("examples/a for 10s"), result("examples/b for 10s", "PASS", expected=True)])
        self.assertEqual(out.returncode, 0, out.stdout + out.stderr)
        self.assertIn("1 statistical test(s) need a candidate", out.stdout)
        self.assertEqual((self.output / "examples/a/omnetpp.ini-General-#0.sca").read_text(), SCA.format(11))
        self.assertFalse((self.output / "examples/b").exists())
        self.assertFalse((self.output / "examples/a/omnetpp.ini-General-#0.diff").exists())
        block = (self.output / "moved-results.txt").read_text()
        self.assertIn("tests: 1 changed, 0 added, 0 removed", block)
        self.assertIn("- packets sent: 1 changed, +10 % to +10 %", block)
        self.assertEqual((self.output / "removed.txt").read_text(), "")
        self.assertIn("statistical baseline checkout:", (self.output / "environment.txt").read_text())

    def test_no_failed_test_makes_no_candidate(self):
        out = self.run_script([result("examples/a for 10s", "PASS", expected=True)])
        self.assertEqual(out.returncode, 0, out.stdout + out.stderr)
        self.assertIn("No statistical test needs a candidate", out.stdout)
        self.assertFalse(self.output.exists())


if __name__ == "__main__":
    unittest.main()
