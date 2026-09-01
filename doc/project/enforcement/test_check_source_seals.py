#!/usr/bin/env python3

import os
from pathlib import Path
import subprocess
import tempfile
import unittest


CHECKER = Path(__file__).with_name("check-source-seals.sh").resolve()


class CheckSourceSealsTest(unittest.TestCase):
    def setUp(self):
        self.tempdir = tempfile.TemporaryDirectory()
        self.root = Path(self.tempdir.name)
        self.write(
            "doc/project/audit/seal-list.md",
            """# Seal list

## Sealed paths

| Status | Path | Note |
| --- | --- | --- |
| 🔒 | `sealed/` | active directory seal |
| 🔒 | `proto/Foo.msg` | active file seal |
<!--
| 🔒 | `common/INETDefs.h` | template only |
-->

## Sealed documents
""",
        )
        self.write("src/inet/sealed/Old.cc", "int value = 1;\n")
        self.write("src/inet/open/Keep.cc", "int keep = 1;\n")
        self.git("init", "-q")
        self.git("config", "user.email", "test@example.invalid")
        self.git("config", "user.name", "Seal Test")
        self.git("add", ".")
        self.git("commit", "-qm", "fixture")

    def tearDown(self):
        self.tempdir.cleanup()

    def write(self, relative_path, content):
        path = self.root / relative_path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")

    def git(self, *args):
        return subprocess.run(
            ["git", *args], cwd=self.root, check=True, capture_output=True, text=True
        )

    def head(self):
        return self.git("rev-parse", "HEAD").stdout.strip()

    def check(self, *args):
        return subprocess.run(
            ["bash", str(CHECKER), *args],
            cwd=self.root,
            capture_output=True,
            text=True,
        )

    def test_commented_template_row_is_not_a_seal(self):
        result = self.check("src/inet/common/INETDefs.h")
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_generated_message_sibling_inherits_exact_seal(self):
        result = self.check("src/inet/proto/Foo_m.cc")
        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("generated from sealed message file 'proto/Foo.msg'", result.stdout)

    def test_invalid_option_or_option_arguments_are_rejected(self):
        for args in (
            ("--bogus",),
            ("--diff", "src/inet/open/Keep.cc"),
            ("--base",),
            ("--base", "HEAD", "extra"),
        ):
            with self.subTest(args=args):
                result = self.check(*args)
                self.assertEqual(2, result.returncode, result.stdout + result.stderr)
                self.assertIn("error:", result.stderr)

    def test_invalid_base_returns_usage_error(self):
        result = self.check("--base", "missing-ref")
        self.assertEqual(2, result.returncode, result.stdout + result.stderr)
        self.assertIn("no merge base", result.stderr)

    def test_explicit_non_source_or_outside_path_is_rejected(self):
        for path in (
            "README.md",
            "src/inet/../outside.cc",
            "src/inet/./open/Keep.cc",
            "src/inet//open/Keep.cc",
            "/tmp/outside.cc",
        ):
            with self.subTest(path=path):
                result = self.check(path)
                self.assertEqual(2, result.returncode, result.stdout + result.stderr)
                self.assertIn("error:", result.stderr)

    def test_explicit_absolute_source_path_is_accepted(self):
        result = self.check(str(self.root / "src/inet/open/Keep.cc"))
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_unstaged_rename_out_of_sealed_directory_checks_old_path(self):
        os.rename(
            self.root / "src/inet/sealed/Old.cc",
            self.root / "src/inet/open/Renamed.cc",
        )
        result = self.check("--diff")
        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("src/inet/sealed/Old.cc", result.stdout)

    def test_staged_rename_out_of_sealed_directory_checks_old_path(self):
        os.rename(
            self.root / "src/inet/sealed/Old.cc",
            self.root / "src/inet/open/Renamed.cc",
        )
        self.git("add", "-A")
        result = self.check("--staged")
        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("src/inet/sealed/Old.cc", result.stdout)

    def test_base_mode_checks_committed_edit_to_sealed_file(self):
        base = self.head()
        self.write("src/inet/sealed/Old.cc", "int value = 2;\n")
        self.git("add", ".")
        self.git("commit", "-qm", "change sealed source")

        result = self.check("--base", base)

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("src/inet/sealed/Old.cc", result.stdout)

    def test_base_mode_checks_old_path_of_committed_rename(self):
        base = self.head()
        os.rename(
            self.root / "src/inet/sealed/Old.cc",
            self.root / "src/inet/open/Renamed.cc",
        )
        self.git("add", "-A")
        self.git("commit", "-qm", "rename sealed source")

        result = self.check("--base", base)

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("src/inet/sealed/Old.cc", result.stdout)

    def test_base_mode_uses_merge_base_registry_when_branch_removes_seal(self):
        base = self.head()
        registry = self.root / "doc/project/audit/seal-list.md"
        registry.write_text(
            registry.read_text(encoding="utf-8").replace(
                "| 🔒 | `sealed/` | active directory seal |\n", ""
            ),
            encoding="utf-8",
        )
        self.write("src/inet/sealed/Old.cc", "int value = 2;\n")
        self.git("add", ".")
        self.git("commit", "-qm", "remove seal and change source")

        result = self.check("--base", base)

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("src/inet/sealed/Old.cc", result.stdout)


if __name__ == "__main__":
    unittest.main()
