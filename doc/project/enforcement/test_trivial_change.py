#!/usr/bin/env python3
"""Tests of check-trivial-change.sh: the push check of S1 to S6 in plan/pending/master-guards.md."""

from pathlib import Path
import subprocess
import sys
import tempfile
import unittest

HERE = Path(__file__).resolve().parent
CHECKER = HERE / "check-trivial-change.sh"
sys.path.insert(0, str(HERE))
from trivial_change import first_code_change  # noqa: E402

SEAL_LIST = """# Seal list

## Sealed paths

| Status | Path | Note |
| --- | --- | --- |
| 🔒 | `sealed/` | active directory seal |

## Sealed documents
"""

CODE = """// The value of the module.
int value = 1; // the start value

/* A block comment
   on two lines. */
int other = 2;
"""


def cpp(old, new):
    return first_code_change(old, new, "c++") is None


def ned(old, new):
    return first_code_change(old, new, "ned") is None


class LexerTest(unittest.TestCase):
    def test_a_line_comment_and_a_block_comment_can_change(self):
        self.assertTrue(cpp(CODE, CODE.replace("start value", "first value").replace("two lines", "2 lines")))

    def test_a_comment_at_the_end_of_a_line_can_come_and_go(self):
        self.assertTrue(cpp("int a = 1;\n", "int a = 1; // one\n"))
        self.assertTrue(cpp("int a = 1; /* one */\n", "int a = 1;\n"))

    def test_a_new_comment_line_moves_the_code(self):
        self.assertFalse(cpp(CODE, "// more\n" + CODE))
        self.assertFalse(cpp(CODE, CODE.replace("on two lines. */", "on\n   three lines. */")))

    def test_an_empty_line_at_the_end_is_no_change_of_code(self):
        self.assertTrue(cpp(CODE, CODE + "\n\n"))

    def test_a_change_of_code_is_found(self):
        self.assertEqual(first_code_change(CODE, CODE.replace("= 2", "= 3"), "c++"), 6)

    def test_a_moved_line_of_code_is_found(self):
        old = "int a = 1;\nint b = 2;\n"
        self.assertFalse(cpp(old, "int b = 2;\nint a = 1;\n"))

    def test_a_comment_that_ends_with_a_backslash_continues_on_the_next_line(self):
        old = "// a comment\nint hidden = 1;\nint shown = 2;\n"
        new = "// a comment \\\nint hidden = 1;\nint shown = 2;\n"
        self.assertEqual(first_code_change(old, new, "c++"), 2)
        # a change in the continued part is a change of a comment
        self.assertTrue(cpp("// a \\\n b\nint x;\n", "// a \\\n c\nint x;\n"))

    def test_a_comment_marker_in_a_string_is_code(self):
        self.assertFalse(cpp('const char *s = "a // b";\n', 'const char *s = "a // c";\n'))
        self.assertFalse(cpp('const char *s = "a /* b */";\n', 'const char *s = "a /* c */";\n'))
        self.assertTrue(cpp('const char *s = "\\" // b"; // x\n', 'const char *s = "\\" // b"; // y\n'))

    def test_a_raw_string_is_code(self):
        old = 'auto s = R"x(a // b\n /* c )" */ d)x"; // e\n'
        self.assertFalse(cpp(old, old.replace("// b", "// B")))
        self.assertFalse(cpp(old, old.replace("/* c", "/* C")))
        self.assertTrue(cpp(old, old.replace("// e", "// E")))
        self.assertFalse(cpp('auto s = u8R"(a // b)";\n', 'auto s = u8R"(a // c)";\n'))

    def test_a_character_literal_and_a_digit_separator_do_not_start_a_string(self):
        self.assertTrue(cpp("char q = '\"'; // a\n", "char q = '\"'; // b\n"))
        self.assertTrue(cpp("int n = 1'000'000; // a\n", "int n = 1'000'000; // b\n"))
        # one separator: without the rule of numbers, the quote starts a literal that hides the comment
        self.assertTrue(cpp("int n = 0x1'000; // a\n", "int n = 0x1'000; // b\n"))
        self.assertTrue(cpp("auto c = u'x'; // a\n", "auto c = u'x'; // b\n"))
        self.assertFalse(cpp("char c = '/'; // a\n", "char c = '*'; // a\n"))

    def test_ned_comments_can_change(self):
        old = "// A module.\nsimple A\n{\n    parameters:\n        int x = 1; // the value\n}\n"
        self.assertTrue(ned(old, old.replace("A module.", "The module.").replace("the value", "a value")))
        self.assertFalse(ned(old, old.replace("x = 1", "x = 2")))

    def test_a_ned_string_and_a_property_value_are_code(self):
        self.assertFalse(ned('string s = "a // b";\n', 'string s = "a // c";\n'))
        self.assertFalse(ned("string s = 'a // b';\n", "string s = 'a // c';\n"))
        self.assertFalse(ned("@doc(see http://a.example);\n", "@doc(see http://b.example);\n"))
        self.assertTrue(ned('@display("i=block/x"); // a\n', '@display("i=block/x"); // b\n'))
        self.assertFalse(ned("@class(a(b) // c);\n", "@class(a(b) // d);\n"))


class GateTest(unittest.TestCase):
    def setUp(self):
        self.tempdir = tempfile.TemporaryDirectory()
        self.root = Path(self.tempdir.name)
        self.git("init", "-q")
        self.git("config", "user.email", "test@example.invalid")
        self.git("config", "user.name", "Push Check Test")
        self.write("doc/project/audit/seal-list.md", SEAL_LIST)
        self.write("doc/project/rule/sealing.md", "# Sealing\n\n> **Kind:** rule · **Status:** current · **Seal:** whole · **Owns:** —\n\nText.\n")
        self.write("doc/guide.md", "# A guide\n\nA text.\n")
        self.write("src/inet/a/A.cc", CODE)
        self.write("src/inet/sealed/S.cc", CODE)
        self.write("examples/a/data.txt", "1 2 3\n")
        self.write("tests/module/A.test", "%description: a test\n")
        self.git("add", ".")
        self.git("commit", "-qm", "fixture")
        self.git("branch", "-M", "master")
        self.git("checkout", "-qb", "topic")

    def tearDown(self):
        self.tempdir.cleanup()

    def git(self, *args):
        return subprocess.run(["git", *args], cwd=self.root, check=True, capture_output=True, text=True)

    def write(self, relative_path, content):
        path = self.root / relative_path
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(content, encoding="utf-8")

    def commit(self, subject, trailers="Change: doc | behavior.change | -"):
        self.git("add", "-A")
        self.git("commit", "-q", "-m", f"{subject}\n\n{trailers}".rstrip())

    def check(self):
        # an empty list of required contexts, outside the repository, keeps the audit away from the GitHub API
        required = Path(self.tempdir.name + "-required-contexts")
        required.write_text("", encoding="utf-8")
        self.addCleanup(required.unlink, missing_ok=True)
        return subprocess.run(["bash", str(CHECKER), "--base", "master", "--head", "HEAD", "--required-contexts-file", str(required)],
                              cwd=self.root, capture_output=True, text=True)

    def assert_trivial(self, result):
        self.assertEqual(result.returncode, 0, result.stdout + result.stderr)
        self.assertIn("TRIVIAL: the change can go to master", result.stdout)

    def assert_not_trivial(self, result, text):
        self.assertEqual(result.returncode, 1, result.stdout + result.stderr)
        self.assertIn("NOT TRIVIAL: this change needs a pull request", result.stdout)
        self.assertIn(text, result.stdout)

    def test_a_document_is_trivial(self):
        self.write("doc/guide.md", "# A guide\n\nThe text.\n")
        self.commit("doc: change: a word of the guide")
        self.assert_trivial(self.check())

    def test_a_comment_of_a_source_file_is_trivial(self):
        self.write("src/inet/a/A.cc", CODE.replace("start value", "first value"))
        self.commit("src: change: a word of a comment", "Change: src | behavior.change | -")
        self.assert_trivial(self.check())

    def test_a_change_of_code_is_not_trivial(self):
        self.write("src/inet/a/A.cc", CODE.replace("= 2", "= 3"))
        self.commit("src: change: a value", "Change: src | behavior.change | -")
        self.assert_not_trivial(self.check(), "src/inet/a/A.cc: a change of code, or of the line of code, at line 6")

    def test_a_new_source_file_is_not_trivial(self):
        self.write("src/inet/a/B.h", "// a header\n")
        self.commit("src: add: a header", "Change: src | behavior.add | -")
        self.assert_not_trivial(self.check(), "src/inet/a/B.h: a new C++ or NED file")

    def test_a_text_file_and_a_test_are_never_trivial(self):
        self.write("examples/a/data.txt", "1 2 4\n")
        self.write("tests/module/A.test", "%description: the test\n")
        self.commit("doc: change: two files")
        result = self.check()
        self.assert_not_trivial(result, "examples/a/data.txt: never trivial: a .txt file")
        self.assertIn("tests/module/A.test: never trivial: under tests/", result.stdout)

    def test_a_document_with_a_seal_unit_and_the_seal_registry_are_never_trivial(self):
        self.write("doc/project/rule/sealing.md", "# Sealing\n\n> **Kind:** rule · **Status:** current · **Seal:** whole · **Owns:** —\n\nThe text.\n")
        self.write("doc/project/audit/seal-list.md", SEAL_LIST + "\nA note.\n")
        self.commit("doc: change: the sealing rule")
        result = self.check()
        self.assert_not_trivial(result, "doc/project/rule/sealing.md: never trivial: a document with the seal unit 'whole'")
        self.assertIn("doc/project/audit/seal-list.md: never trivial: the seal registry", result.stdout)

    def test_a_comment_in_a_sealed_source_path_is_not_trivial(self):
        self.write("src/inet/sealed/S.cc", CODE.replace("start value", "first value"))
        self.commit("src: change: a word of a sealed comment", "Change: src | behavior.change | -")
        self.assert_not_trivial(self.check(), "the change touches a sealed source path")

    def test_a_pin_trailer_is_not_trivial(self):
        self.write("doc/guide.md", "# A guide\n\nThe text.\n")
        self.commit("doc: change: a word of the guide", "Change: doc | behavior.change | -\nStatistical-Baseline: " + "0" * 40)
        self.assert_not_trivial(self.check(), "has the pin trailer Statistical-Baseline:")

    def test_a_violation_of_the_audit_is_not_trivial(self):
        self.write("doc/guide.md", "# A guide\n\nThe text.\n")
        self.commit("doc: change: a word of the guide", "")
        self.assert_not_trivial(self.check(), "the audit has a violation")

    def test_a_change_of_the_file_mode_is_not_trivial(self):
        (self.root / "doc/guide.md").chmod(0o755)
        self.commit("doc: change: the mode of the guide")
        self.assert_not_trivial(self.check(), "doc/guide.md: a change of the file mode (100644 to 100755)")

    def test_a_branch_with_no_commit_is_not_trivial(self):
        self.assert_not_trivial(self.check(), "no changed file")

    def test_a_branch_behind_master_gives_a_note(self):
        self.write("doc/guide.md", "# A guide\n\nThe text.\n")
        self.commit("doc: change: a word of the guide")
        self.git("checkout", "-q", "master")
        self.write("doc/other.md", "# Other\n")
        self.commit("doc: add: another document", "Change: doc | behavior.add | -")
        self.git("checkout", "-q", "topic")
        result = self.check()
        self.assert_trivial(result)
        self.assertIn("note: master moved since the branch started", result.stdout)


if __name__ == "__main__":
    unittest.main()
