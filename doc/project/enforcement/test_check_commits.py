#!/usr/bin/env python3

from pathlib import Path
import subprocess
import tempfile
import unittest


CHECKER = Path(__file__).with_name("check-commits.sh").resolve()

ROW = "/examples/{name}/, -f omnetpp.ini -c {name} -r 0, 10s, {tplx}/tplx;{tnl}/~tNl;1111-1111/~tND, PASS, wireless\n"


def rows(**fingerprints):
    """One fingerprint row per name; each value is (tplx, ~tNl)."""
    return "".join(ROW.format(name=name, tplx=tplx, tnl=tnl) for name, (tplx, tnl) in fingerprints.items())


class CheckCommitsTest(unittest.TestCase):
    def setUp(self):
        self.tempdir = tempfile.TemporaryDirectory()
        self.root = Path(self.tempdir.name)
        self.git("init", "-q")
        self.git("config", "user.email", "test@example.invalid")
        self.git("config", "user.name", "Commit Test")
        self.write("src/inet/a/A.cc", "int a = 0;\n")
        self.write("tests/fingerprint/examples.csv", rows(one=("aaaa-0001", "bbbb-0001"), two=("aaaa-0002", "bbbb-0002")))
        self.git("add", ".")
        self.git("commit", "-qm", "fixture")
        self.base = self.git("rev-parse", "HEAD").stdout.strip()

    def tearDown(self):
        self.tempdir.cleanup()

    def write(self, relative_path, content):
        path = self.root / relative_path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")

    def git(self, *args):
        return subprocess.run(["git", *args], cwd=self.root, check=True, capture_output=True, text=True)

    def commit(self, message, files):
        for relative_path, content in files.items():
            self.write(relative_path, content)
        self.git("add", ".")
        self.git("commit", "-q", "-m", message)

    def check(self):
        return subprocess.run(["bash", str(CHECKER), f"{self.base}..HEAD"], cwd=self.root,
                              capture_output=True, text=True)

    def test_rows_that_move_in_two_ways_give_a_note(self):
        self.commit("a: change: move two rows\n\nThe rows move for two reasons.", {
            "src/inet/a/A.cc": "int a = 1;\n",
            "tests/fingerprint/examples.csv": rows(one=("cccc-0001", "bbbb-0001"), two=("cccc-0002", "dddd-0002")),
        })
        result = self.check()
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn("moves fingerprint rows in 2 ways", result.stdout)
        self.assertIn("1 row changes tplx; 1 row changes tplx,~tNl", result.stdout)

    def test_rows_that_move_in_one_way_are_quiet(self):
        self.commit("a: change: move two rows\n\nThe rows move for one reason.", {
            "src/inet/a/A.cc": "int a = 1;\n",
            "tests/fingerprint/examples.csv": rows(one=("cccc-0001", "bbbb-0001"), two=("cccc-0002", "bbbb-0002")),
        })
        result = self.check()
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertNotIn("moves fingerprint rows", result.stdout)


if __name__ == "__main__":
    unittest.main()
