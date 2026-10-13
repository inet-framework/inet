#!/usr/bin/env python3

from pathlib import Path
import subprocess
import tempfile
import unittest


CHECKER = Path(__file__).with_name("check-includes.sh").resolve()


class CheckIncludesTest(unittest.TestCase):
    def setUp(self):
        self.tempdir = tempfile.TemporaryDirectory()
        self.root = Path(self.tempdir.name)
        self.git("init", "-q")
        self.git("config", "user.email", "test@example.invalid")
        self.git("config", "user.name", "Includes Test")
        self.write("src/inet/a/A.h", "int a();\n")
        self.write("src/inet/a/Message.msg", "packet Message {}\n")
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

    def commit(self, content):
        self.write("src/inet/a/A.cc", content)
        self.git("add", ".")
        self.git("commit", "-qm", "a: change: include a header")

    def check(self):
        return subprocess.run(["bash", str(CHECKER), f"{self.base}..HEAD"], cwd=self.root,
                              capture_output=True, text=True)

    def test_a_header_in_the_tree_passes(self):
        self.commit('#include "inet/a/A.h"\n')
        result = self.check()
        self.assertEqual(result.returncode, 0, result.stdout)

    def test_a_header_from_a_message_file_passes(self):
        self.commit('#include "inet/a/Message_m.h"\n')
        result = self.check()
        self.assertEqual(result.returncode, 0, result.stdout)

    def test_a_header_that_the_build_generates_passes(self):
        self.commit('#include "inet/features.h"\n#include "inet/opp_defines.h"\n')
        result = self.check()
        self.assertEqual(result.returncode, 0, result.stdout)

    def test_a_missing_header_fails(self):
        self.commit('#include "inet/b/Later.h"\n')
        result = self.check()
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("inet/b/Later.h", result.stdout)


if __name__ == "__main__":
    unittest.main()
