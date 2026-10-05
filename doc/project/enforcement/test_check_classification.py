#!/usr/bin/env python3

from pathlib import Path
import subprocess
import tempfile
import unittest


CHECKER = Path(__file__).with_name("check-classification.sh").resolve()


class CheckClassificationTest(unittest.TestCase):
    def setUp(self):
        self.tempdir = tempfile.TemporaryDirectory()
        self.root = Path(self.tempdir.name)
        self.git("init", "-q")
        self.git("config", "user.email", "test@example.invalid")
        self.git("config", "user.name", "Classification Test")
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

    def commit(self, subject, kind, value):
        self.write("src/inet/a/A.cc", f"int a = {value};\n")
        self.git("add", ".")
        self.git("commit", "-q", "-m", f"{subject}\n\nThe value changes.\n\nChange: src | {kind} | -")

    def check(self):
        return subprocess.run(["bash", str(CHECKER), f"{self.base}..HEAD"], cwd=self.root,
                              capture_output=True, text=True)

    def test_a_mixed_direction_gives_a_note(self):
        self.commit("add+fix: one value", "behavior.add+change.fix", 1)
        result = self.check()
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn("claims behavior.add+change.fix: can a part stand alone?", result.stdout)

    def test_one_direction_is_quiet(self):
        self.commit("fix: one value", "behavior.change.fix", 1)
        result = self.check()
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertNotIn("can a part stand alone?", result.stdout)


if __name__ == "__main__":
    unittest.main()
