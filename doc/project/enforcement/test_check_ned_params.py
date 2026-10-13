#!/usr/bin/env python3

from pathlib import Path
import subprocess
import tempfile
import unittest


CHECKER = Path(__file__).with_name("check-ned-params.sh").resolve()


class CheckNedParamsTest(unittest.TestCase):
    def setUp(self):
        self.tempdir = tempfile.TemporaryDirectory()
        self.root = Path(self.tempdir.name)
        self.git("init", "-q")
        self.git("config", "user.email", "test@example.invalid")
        self.git("config", "user.name", "Ned Params Test")
        self.write("src/inet/a/A.ned", "simple A\n{\n    parameters:\n        bool known = true;\n}\n")
        self.write("src/inet/a/A.cc", "int a = 0;\n")
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

    def commit(self, files):
        for relative_path, content in files.items():
            self.write(relative_path, content)
        self.git("add", ".")
        self.git("commit", "-qm", "a: change: read a parameter")

    def check(self):
        return subprocess.run(["bash", str(CHECKER), f"{self.base}..HEAD"], cwd=self.root,
                              capture_output=True, text=True)

    def test_a_declared_parameter_passes(self):
        self.commit({"src/inet/a/A.cc": 'bool k = par("known");\n'})
        result = self.check()
        self.assertEqual(result.returncode, 0, result.stdout)

    def test_an_undeclared_parameter_in_the_library_fails(self):
        self.commit({"src/inet/a/A.cc": 'bool u = par("unknown");\n'})
        result = self.check()
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("unknown", result.stdout)

    def test_a_parameter_read_in_a_test_file_is_not_checked(self):
        # A .test file declares its parameters in its own embedded NED, which the gate does not read.
        self.commit({"src/inet/a/A.cc": "int a = 1;\n",
                     "tests/module/A_1.test": '%file: test.ned\nsimple T { parameters: bool probe; }\n'
                                              '%file: T.cc\nbool p = par("probe");\n'})
        result = self.check()
        self.assertEqual(result.returncode, 0, result.stdout)


if __name__ == "__main__":
    unittest.main()
