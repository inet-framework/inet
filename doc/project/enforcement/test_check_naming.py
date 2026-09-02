#!/usr/bin/env python3

from __future__ import annotations

import runpy
import shutil
import subprocess
import sys
import tempfile
import unittest
from pathlib import Path


CHECKER = Path(__file__).resolve().parent / "check-ned-msg-naming.py"
WRAPPER = Path(__file__).resolve().parent / "check-naming.sh"


class NamingCheckerTest(unittest.TestCase):
    def setUp(self) -> None:
        self.temporary = tempfile.TemporaryDirectory()
        self.root = Path(self.temporary.name)
        self.run_command("git", "init", "-q")
        self.run_command("git", "config", "user.email", "test@example.invalid")
        self.run_command("git", "config", "user.name", "Naming Test")

    def tearDown(self) -> None:
        self.temporary.cleanup()

    def run_command(self, *args: str) -> subprocess.CompletedProcess[str]:
        return subprocess.run(args, cwd=self.root, text=True, stdout=subprocess.PIPE, stderr=subprocess.PIPE)

    def write(self, relative: str, text: str) -> Path:
        path = self.root / relative
        path.parent.mkdir(parents=True, exist_ok=True)
        path.write_text(text, encoding="utf-8")
        return path

    def commit_all(self) -> None:
        self.run_command("git", "add", ".")
        result = self.run_command("git", "commit", "-qm", "fixture")
        self.assertEqual(0, result.returncode, result.stderr)

    def check(self, *args: str) -> subprocess.CompletedProcess[str]:
        return self.run_command(sys.executable, str(CHECKER), *args)

    def check_wrapper(self, *args: str) -> subprocess.CompletedProcess[str]:
        enforcement = self.root / "doc/project/enforcement"
        enforcement.mkdir(parents=True, exist_ok=True)
        shutil.copy2(CHECKER, enforcement / CHECKER.name)
        shutil.copy2(WRAPPER, enforcement / WRAPPER.name)
        return self.run_command("bash", str(enforcement / WRAPPER.name), *args)

    def head(self) -> str:
        result = self.run_command("git", "rev-parse", "HEAD")
        self.assertEqual(0, result.returncode, result.stderr)
        return result.stdout.strip()

    def test_explicit_clean_ned_and_msg(self) -> None:
        self.write("src/inet/foo/Foo.ned", """package inet.foo;
simple Foo
{
    parameters:
        bool enabled;
    gates:
        input in;
        output out;
    @signal[bytesInFlight*](type=long);
    @statistic[queueLength*](source=count);
}
""")
        self.write("src/inet/foo/Foo.msg", """namespace inet;
cplusplus {{
class bad_name {};
}}
cplusplus(Foo) {{
struct also_bad {};
}}
class Forward;
class SplitForward
;
class Helper { @existingClass; }
const int BAD_GLOBAL = 1;
class Foo
{
    int fieldName;
}
""")
        result = self.check("src/inet/foo/Foo.ned", "src/inet/foo/Foo.msg")
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_first_double_brace_ends_cplusplus_masking(self) -> None:
        self.write("src/inet/foo/Foo.msg", """namespace inet;
cplusplus {{
class ignored_bad_name {};
}}
class also_bad_name {}
""")

        result = self.check("src/inet/foo/Foo.msg")

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertNotIn("ignored_bad_name", result.stdout)
        self.assertIn("also_bad_name", result.stdout)

    def test_explicit_msg_requires_valid_namespace(self) -> None:
        for label, namespace in (
            ("missing", ""),
            ("uppercase", "namespace inet::Tcp;\n"),
            ("foreign", "namespace other::foo;\n"),
        ):
            with self.subTest(label=label):
                path = self.write("src/inet/foo/Foo.msg", f"{namespace}class Foo {{}}\n")
                result = self.check(str(path.relative_to(self.root)))
                self.assertEqual(1, result.returncode, result.stdout + result.stderr)
                self.assertIn("namespace", result.stdout)

    def test_explicit_msg_allows_multiple_valid_namespaces(self) -> None:
        path = self.write(
            "src/inet/foo/Foo.msg",
            """namespace inet;
class Helper {}
namespace inet::foo;
class Foo {}
""",
        )

        result = self.check(str(path.relative_to(self.root)))

        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_untracked_msg_missing_namespace_is_checked(self) -> None:
        self.write("README.md", "fixture\n")
        self.commit_all()
        self.write("src/inet/foo/Foo.msg", "class Foo {}\n")

        result = self.check()

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("MSG file must declare", result.stdout)

    def test_staged_msg_namespace_uses_index_content(self) -> None:
        path = self.write("src/inet/foo/Foo.msg", "namespace inet;\nclass Foo {}\n")
        self.commit_all()
        path.write_text("namespace inet::Tcp;\nclass Foo {}\n", encoding="utf-8")
        self.run_command("git", "add", str(path.relative_to(self.root)))
        path.write_text("namespace inet;\nclass Foo {}\n", encoding="utf-8")

        result = self.check("--staged")

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("inet::Tcp", result.stdout)

    def test_base_msg_namespace_uses_committed_head_content(self) -> None:
        path = self.write("src/inet/foo/Foo.msg", "namespace inet;\nclass Foo {}\n")
        self.commit_all()
        base = self.head()
        path.write_text("namespace other::foo;\nclass Foo {}\n", encoding="utf-8")
        self.commit_all()
        path.write_text("namespace inet;\nclass Foo {}\n", encoding="utf-8")

        result = self.check("--base", base)

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("other::foo", result.stdout)

    def test_renamed_msg_is_scanned_for_namespace(self) -> None:
        old_path = self.write("src/inet/foo/Old.msg", "namespace Inet;\nclass Renamed {}\n")
        self.commit_all()
        new_path = old_path.with_name("Renamed.msg")
        move = self.run_command(
            "git", "mv", str(old_path.relative_to(self.root)), str(new_path.relative_to(self.root))
        )
        self.assertEqual(0, move.returncode, move.stderr)

        for args in ((), ("--staged",)):
            with self.subTest(args=args):
                result = self.check(*args)
                self.assertEqual(1, result.returncode, result.stdout + result.stderr)
                self.assertIn("namespace 'Inet'", result.stdout)

    def test_ned_comment_after_even_backslash_run_is_stripped(self) -> None:
        self.write("src/inet/foo/Foo.ned", r'''package inet.foo;
simple Foo
{
    parameters:
        string value = default("literal://path/*part*/\\"); // @signal[BadSignal](type=long);
}
''')

        result = self.check("src/inet/foo/Foo.ned")

        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_msg_comment_after_even_backslash_run_is_stripped(self) -> None:
        self.write("src/inet/foo/Foo.msg", r'''namespace inet;
packet Foo
{
    string value = "literal://path/*part*/\\"; // int Bad_field;
}
''')

        result = self.check("src/inet/foo/Foo.msg")

        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_comment_markers_inside_strings_are_preserved(self) -> None:
        strip_comments = runpy.run_path(str(CHECKER))["strip_comments"]
        lines = [
            r'value = "literal://path"; // trailing comment',
            r'value = "literal/*part*/path"; /* trailing comment */',
            r'value = "escaped quote: \" // literal"; // trailing comment',
        ]

        self.assertEqual(
            [
                r'value = "literal://path"; ',
                r'value = "literal/*part*/path"; ',
                r'value = "escaped quote: \" // literal"; ',
            ],
            strip_comments(lines),
        )

    def test_reports_one_line_msg_field(self) -> None:
        self.write("src/inet/foo/Foo.msg", """namespace inet;
class Foo { int Bad_field; }
""")
        result = self.check("src/inet/foo/Foo.msg")
        self.assertEqual(1, result.returncode)
        self.assertIn("NR-MSG-FIELD", result.stdout)

    def test_reports_multiline_msg_field(self) -> None:
        self.write("src/inet/foo/Foo.msg", """namespace inet;
class Foo
{
    int
        Bad_field;
}
""")
        result = self.check("src/inet/foo/Foo.msg")
        self.assertEqual(1, result.returncode)
        self.assertIn("NR-MSG-FIELD", result.stdout)

    def test_braces_in_msg_string_do_not_change_type_context(self) -> None:
        self.write("src/inet/foo/Foo.msg", """namespace inet;
class Foo
{
    string marker = "{";
}
const int BAD_GLOBAL = 1;
""")
        result = self.check("src/inet/foo/Foo.msg")
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_multiline_msg_forward_is_not_a_definition(self) -> None:
        self.write("src/inet/foo/Foo.msg", """namespace inet;
class Foo
;
class Other {}
""")
        result = self.check("src/inet/foo/Foo.msg")
        self.assertEqual(1, result.returncode)
        self.assertIn("NR-PKG", result.stdout)

    def test_worktree_checks_multiline_forward_changed_to_nonconforming_definition(self) -> None:
        path = self.write("src/inet/foo/Foo.msg", """namespace inet;
class Foo {}
class bad_name
;
""")
        self.commit_all()
        path.write_text(path.read_text(encoding="utf-8").replace("\n;\n", "\n{}\n"), encoding="utf-8")

        result = self.check()

        self.assertEqual(1, result.returncode)
        self.assertIn("NR-MSG-TYPE", result.stdout)
        self.assertIn("bad_name", result.stdout)

    def test_staged_checks_multiline_forward_changed_to_nonconforming_definition(self) -> None:
        path = self.write("src/inet/foo/Foo.msg", """namespace inet;
class Foo {}
class bad_name
;
""")
        self.commit_all()
        path.write_text(path.read_text(encoding="utf-8").replace("\n;\n", "\n{}\n"), encoding="utf-8")
        self.run_command("git", "add", str(path.relative_to(self.root)))

        result = self.check("--staged")

        self.assertEqual(1, result.returncode)
        self.assertIn("NR-MSG-TYPE", result.stdout)
        self.assertIn("bad_name", result.stdout)

    def test_base_checks_multiline_forward_changed_to_nonconforming_definition(self) -> None:
        path = self.write("src/inet/foo/Foo.msg", """namespace inet;
class Foo {}
class bad_name
;
""")
        self.commit_all()
        base = self.head()
        path.write_text(path.read_text(encoding="utf-8").replace("\n;\n", "\n{}\n"), encoding="utf-8")
        self.commit_all()

        result = self.check("--base", base)

        self.assertEqual(1, result.returncode)
        self.assertIn("NR-MSG-TYPE", result.stdout)
        self.assertIn("bad_name", result.stdout)

    def test_changed_comment_does_not_activate_multiline_forward_declaration(self) -> None:
        path = self.write("src/inet/foo/Foo.msg", """namespace inet;
class Foo {}
class bad_name
;
""")
        self.commit_all()
        path.write_text(path.read_text(encoding="utf-8").replace("\n;\n", "\n// still forward\n;\n"), encoding="utf-8")

        result = self.check()

        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_reports_mechanical_rule_families(self) -> None:
        self.write("src/inet/foo/Bad.ned", """package inet.BadPath;
simple bad_type
{
    parameters:
        bool Bad_param;
    gates:
        input packet;
        output PacketOut;
    @signal[BadSignal](type=long);
    @statistic[bad-stat](source=count);
}
""")
        self.write("src/inet/foo/Bad.msg", """namespace inet;
class bad_type
{
    int Bad_field;
}
""")
        result = self.check("src/inet/foo/Bad.ned", "src/inet/foo/Bad.msg")
        self.assertEqual(1, result.returncode)
        for rule in (
            "NR-PKG", "NR-NED-TYPE", "NR-NED-PARAM", "NR-NED-GATE",
            "NR-NED-SIGNAL", "NR-MSG-TYPE", "NR-MSG-FIELD",
        ):
            self.assertIn(rule, result.stdout)

    def test_modified_file_checks_only_added_lines(self) -> None:
        path = self.write("src/inet/foo/Legacy.ned", """package inet.foo;
simple Legacy
{
    parameters:
        bool Bad_legacy;
}
""")
        self.commit_all()
        path.write_text(path.read_text(encoding="utf-8").replace("        bool Bad_legacy;", "        bool Bad_legacy;\n        bool goodName;"), encoding="utf-8")
        result = self.check()
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_modified_msg_checks_multiline_added_field(self) -> None:
        path = self.write("src/inet/foo/Foo.msg", """namespace inet;
class Foo
{
    int goodName;
}
""")
        self.commit_all()
        path.write_text(path.read_text(encoding="utf-8").replace("    int goodName;", "    int goodName;\n    int\n        Bad_field;"), encoding="utf-8")
        result = self.check()
        self.assertEqual(1, result.returncode)
        self.assertIn("NR-MSG-FIELD", result.stdout)

    def test_modified_msg_checks_changed_identifier_before_unchanged_semicolon(self) -> None:
        path = self.write("src/inet/foo/Foo.msg", """namespace inet;
class Foo
{
    int goodField
    ;
}
""")
        self.commit_all()
        path.write_text(path.read_text(encoding="utf-8").replace("goodField", "Bad_field"), encoding="utf-8")
        result = self.check()
        self.assertEqual(1, result.returncode)
        self.assertIn("NR-MSG-FIELD", result.stdout)

    def test_modified_msg_does_not_surface_adjacent_legacy_field(self) -> None:
        path = self.write("src/inet/foo/Foo.msg", """namespace inet;
class Foo
{
    int goodName;
    int Bad_legacy
    ;
}
""")
        self.commit_all()
        path.write_text(path.read_text(encoding="utf-8").replace("goodName", "betterName"), encoding="utf-8")
        result = self.check()
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_untracked_file_is_scanned_completely(self) -> None:
        self.write("README.md", "fixture\n")
        self.commit_all()
        self.write("src/inet/foo/NewThing.ned", """package inet.foo;
simple NewThing
{
    parameters:
        bool Bad_name;
}
""")
        result = self.check("--diff")
        self.assertEqual(1, result.returncode)
        self.assertIn("NR-NED-PARAM", result.stdout)

    def test_base_mode_checks_committed_content_not_worktree(self) -> None:
        path = self.write("src/inet/foo/Foo.ned", """package inet.foo;
simple Foo
{
    parameters:
        bool goodName;
}
""")
        self.commit_all()
        base = self.head()
        path.write_text(path.read_text(encoding="utf-8").replace("goodName", "Bad_committed"), encoding="utf-8")
        self.commit_all()
        path.write_text(path.read_text(encoding="utf-8").replace("Bad_committed", "Bad_worktree"), encoding="utf-8")

        result = self.check("--base", base)

        self.assertEqual(1, result.returncode)
        self.assertIn("Bad_committed", result.stdout)
        self.assertNotIn("Bad_worktree", result.stdout)

    def test_base_mode_preserves_added_line_filtering(self) -> None:
        path = self.write("src/inet/foo/Legacy.ned", """package inet.foo;
simple Legacy
{
    parameters:
        bool Bad_legacy;
}
""")
        self.commit_all()
        base = self.head()
        path.write_text(path.read_text(encoding="utf-8").replace("        bool Bad_legacy;", "        bool Bad_legacy;\n        bool goodName;"), encoding="utf-8")
        self.commit_all()

        result = self.check("--base", base)

        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_base_mode_checks_structural_deletions(self) -> None:
        path = self.write("src/inet/foo/Foo.ned", "package inet.foo;\nsimple Foo {}\n")
        self.commit_all()
        base = self.head()
        path.write_text("simple Foo {}\n", encoding="utf-8")
        self.commit_all()

        result = self.check("--base", base)

        self.assertEqual(1, result.returncode)
        self.assertIn("NR-PKG", result.stdout)

    def test_wrapper_base_mode_checks_clean_committed_branch(self) -> None:
        self.write("README.md", "fixture\n")
        self.commit_all()
        base = self.head()
        self.write("src/inet/foo/Foo.ned", """package inet.foo;
simple Foo
{
    parameters:
        bool Bad_committed;
}
""")
        self.commit_all()

        result = self.check_wrapper("--base", base)

        self.assertEqual(1, result.returncode)
        self.assertIn("NR-NED/MSG", result.stdout)
        self.assertIn("Bad_committed", result.stdout)

    def test_wrapper_default_preserves_worktree_mode(self) -> None:
        self.write("README.md", "fixture\n")
        self.commit_all()
        self.write("src/inet/foo/Foo.ned", """package inet.foo;
simple Foo
{
    parameters:
        bool Bad_worktree;
}
""")

        result = self.check_wrapper()

        self.assertEqual(1, result.returncode)
        self.assertIn("Bad_worktree", result.stdout)

    def test_wrapper_scoped_audit_checks_only_that_subtree(self) -> None:
        self.write("src/inet/inside/Inside.ned", """package inet.inside;
simple Inside
{
    parameters:
        bool Bad_inside;
}
""")
        self.write("src/inet/outside/Outside.ned", """package inet.outside;
simple Outside
{
    parameters:
        bool Bad_outside;
}
""")

        result = self.check_wrapper("src/inet/inside")

        self.assertEqual(1, result.returncode)
        self.assertIn("NR-NED/MSG", result.stdout)
        self.assertIn("Bad_inside", result.stdout)
        self.assertNotIn("Bad_outside", result.stdout)

    def test_wrapper_rejects_rule_gate_workflow_without_check_prefix(self) -> None:
        self.write("src/inet/foo/Foo.ned", "package inet.foo;\nsimple Foo {}\n")
        self.write(".github/workflows/project-gates.yml", "name: project gates\n")
        self.commit_all()

        result = self.check_wrapper()

        self.assertEqual(1, result.returncode)
        self.assertIn("NR-CI", result.stdout)
        self.assertIn(".github/workflows/project-gates.yml", result.stdout)

    def test_wrapper_accepts_ci_workflow_filename_forms(self) -> None:
        self.write("src/inet/foo/Foo.ned", "package inet.foo;\nsimple Foo {}\n")
        self.write(".github/workflows/build-linux.yml", "name: build linux\n")
        self.write(".github/workflows/enforcement-tests.yml", "name: enforcement tests\n")
        self.write(".github/workflows/check-sealing.yml", "name: check sealing\n")
        self.commit_all()

        result = self.check_wrapper()

        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_invalid_base_returns_usage_error_through_wrapper(self) -> None:
        self.write("src/inet/foo/Foo.ned", "package inet.foo;\nsimple Foo {}\n")
        self.commit_all()

        result = self.check_wrapper("--base", "missing-ref")

        self.assertEqual(2, result.returncode)
        self.assertIn("missing-ref", result.stderr)

    def test_wrapper_rejects_scope_outside_src_inet(self) -> None:
        self.write("src/inet/foo/Foo.ned", "package inet.foo;\nsimple Foo {}\n")

        result = self.check_wrapper(".")

        self.assertEqual(2, result.returncode)
        self.assertIn("scope must be src/inet", result.stderr)

    def test_staged_mode_checks_staged_additions(self) -> None:
        path = self.write("src/inet/foo/Stage.ned", """package inet.foo;
simple Stage
{
    parameters:
        bool goodName;
}
""")
        self.commit_all()
        path.write_text(path.read_text(encoding="utf-8").replace("        bool goodName;", "        bool goodName;\n        bool Bad_name;"), encoding="utf-8")
        self.run_command("git", "add", str(path.relative_to(self.root)))
        result = self.check("--staged")
        self.assertEqual(1, result.returncode)
        self.assertIn("NR-NED-PARAM", result.stdout)

    def test_staged_mode_reads_index_not_worktree(self) -> None:
        path = self.write("src/inet/foo/Stage.ned", """package inet.foo;
simple Stage
{
    parameters:
        bool goodName;
}
""")
        self.commit_all()
        path.write_text(path.read_text(encoding="utf-8").replace("goodName", "Bad_staged"), encoding="utf-8")
        self.run_command("git", "add", str(path.relative_to(self.root)))
        path.write_text(path.read_text(encoding="utf-8").replace("Bad_staged", "goodUnstaged"), encoding="utf-8")
        result = self.check("--staged")
        self.assertEqual(1, result.returncode)
        self.assertIn("Bad_staged", result.stdout)
        self.assertNotIn("goodUnstaged", result.stdout)

    def test_staged_mode_ignores_unstaged_violation(self) -> None:
        path = self.write("src/inet/foo/Stage.ned", """package inet.foo;
simple Stage
{
    parameters:
        bool originalName;
}
""")
        self.commit_all()
        path.write_text(path.read_text(encoding="utf-8").replace("originalName", "goodStaged"), encoding="utf-8")
        self.run_command("git", "add", str(path.relative_to(self.root)))
        path.write_text(path.read_text(encoding="utf-8").replace("goodStaged", "Bad_unstaged"), encoding="utf-8")
        result = self.check("--staged")
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)
        working_result = self.check()
        self.assertEqual(1, working_result.returncode)
        self.assertIn("Bad_unstaged", working_result.stdout)

    def test_staged_new_file_missing_from_worktree_is_checked(self) -> None:
        self.write("README.md", "fixture\n")
        self.commit_all()
        path = self.write("src/inet/foo/Stage.ned", """package inet.foo;
simple Stage
{
    parameters:
        bool Bad_staged;
}
""")
        self.run_command("git", "add", str(path.relative_to(self.root)))
        path.unlink()
        result = self.check("--staged")
        self.assertEqual(1, result.returncode)
        self.assertIn("Bad_staged", result.stdout)

    def test_staged_structural_deletion_uses_index(self) -> None:
        original = "package inet.foo;\nsimple Foo {}\n"
        path = self.write("src/inet/foo/Foo.ned", original)
        self.commit_all()
        path.write_text("simple Foo {}\n", encoding="utf-8")
        self.run_command("git", "add", str(path.relative_to(self.root)))
        path.write_text(original, encoding="utf-8")
        result = self.check("--staged")
        self.assertEqual(1, result.returncode)
        self.assertIn("NR-PKG", result.stdout)

    def test_msg_namespace_deletion_is_checked(self) -> None:
        path = self.write("src/inet/foo/Foo.msg", "namespace inet;\nclass Foo {}\n")
        self.commit_all()
        path.write_text("class Foo {}\n", encoding="utf-8")

        result = self.check()

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("MSG file must declare", result.stdout)

    def test_first_of_duplicate_msg_namespaces_deletion_is_checked(self) -> None:
        path = self.write(
            "src/inet/foo/Foo.msg",
            "namespace inet;\nclass Helper {}\nnamespace inet;\nclass Foo {}\n",
        )
        self.commit_all()
        path.write_text("class Helper {}\nnamespace inet;\nclass Foo {}\n", encoding="utf-8")

        result = self.check()

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("must precede the first type declaration", result.stdout)

    def test_staged_msg_namespace_deletion_uses_index(self) -> None:
        original = "namespace inet;\nclass Foo {}\n"
        path = self.write("src/inet/foo/Foo.msg", original)
        self.commit_all()
        path.write_text("class Foo {}\n", encoding="utf-8")
        self.run_command("git", "add", str(path.relative_to(self.root)))
        path.write_text(original, encoding="utf-8")

        result = self.check("--staged")

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("MSG file must declare", result.stdout)

    def test_base_msg_namespace_deletion_uses_head(self) -> None:
        path = self.write("src/inet/foo/Foo.msg", "namespace inet;\nclass Foo {}\n")
        self.commit_all()
        base = self.head()
        path.write_text("class Foo {}\n", encoding="utf-8")
        self.commit_all()
        path.write_text("namespace inet;\nclass Foo {}\n", encoding="utf-8")

        result = self.check("--base", base)

        self.assertEqual(1, result.returncode, result.stdout + result.stderr)
        self.assertIn("MSG file must declare", result.stdout)

    def test_structural_deletions_are_checked(self) -> None:
        ned_path = self.write("src/inet/foo/Foo.ned", "package inet.foo;\nsimple Foo {}\n")
        msg_path = self.write("src/inet/foo/Primary.msg", "class Primary {}\nclass Other {}\n")
        self.commit_all()
        ned_path.write_text("simple Foo {}\n", encoding="utf-8")
        msg_path.write_text("class Other {}\n", encoding="utf-8")
        result = self.check()
        self.assertEqual(1, result.returncode)
        self.assertIn("NR-PKG", result.stdout)

    def test_package_deletion_is_checked_in_legacy_package_path(self) -> None:
        path = self.write("src/inet/foo_bar/Foo.ned", "package inet.foo_bar;\nsimple Foo {}\n")
        self.commit_all()
        path.write_text("simple Foo {}\n", encoding="utf-8")
        result = self.check()
        self.assertEqual(1, result.returncode)
        self.assertIn("NR-PKG", result.stdout)

    def test_deletions_inside_comments_and_cplusplus_are_not_structural(self) -> None:
        path = self.write("src/inet/foo/Legacy.msg", """namespace inet;
/*
class Legacy {}
*/
cplusplus(Legacy) {{
class Legacy {};
}}
class Other {}
""")
        self.commit_all()
        text = path.read_text(encoding="utf-8")
        text = text.replace("class Legacy {}\n", "").replace("class Legacy {};\n", "")
        path.write_text(text, encoding="utf-8")
        result = self.check()
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_secondary_type_does_not_surface_legacy_file_mismatch(self) -> None:
        path = self.write("src/inet/foo/Legacy.msg", "class Other {}\n")
        self.commit_all()
        path.write_text("class Other {}\nclass Secondary {}\n", encoding="utf-8")
        result = self.check()
        self.assertEqual(0, result.returncode, result.stdout + result.stderr)

    def test_renamed_file_is_scanned_completely(self) -> None:
        old_path = self.write("src/inet/foo/Old.ned", "package inet.foo;\nsimple Old {}\n")
        self.commit_all()
        new_path = old_path.with_name("New.ned")
        result = self.run_command("git", "mv", str(old_path.relative_to(self.root)), str(new_path.relative_to(self.root)))
        self.assertEqual(0, result.returncode, result.stderr)
        result = self.check()
        self.assertEqual(1, result.returncode)
        self.assertIn("NR-PKG", result.stdout)
        staged_result = self.check("--staged")
        self.assertEqual(1, staged_result.returncode)
        self.assertIn("NR-PKG", staged_result.stdout)

    def test_copied_staged_file_is_scanned_completely(self) -> None:
        source = self.write("src/inet/foo/Source.ned", "package inet.foo;\nsimple Source {}\n")
        self.commit_all()
        copy = source.with_name("Copy.ned")
        copy.write_text(source.read_text(encoding="utf-8"), encoding="utf-8")
        self.run_command("git", "add", str(copy.relative_to(self.root)))
        result = self.check("--staged")
        self.assertEqual(1, result.returncode)
        self.assertIn("NR-PKG", result.stdout)

    def test_deleted_files_are_ignored_without_reading(self) -> None:
        path = self.write("src/inet/foo/Foo.ned", "package inet.foo;\nsimple Foo {}\n")
        self.commit_all()
        path.unlink()
        working_result = self.check()
        self.assertEqual(0, working_result.returncode, working_result.stdout + working_result.stderr)
        self.run_command("git", "add", "-u")
        staged_result = self.check("--staged")
        self.assertEqual(0, staged_result.returncode, staged_result.stdout + staged_result.stderr)

    def test_diagnostics_are_stably_sorted(self) -> None:
        self.write("src/inet/zeta/Zeta.ned", "package inet.zeta;\nsimple bad {}\n")
        self.write("src/inet/alpha/Alpha.ned", "package inet.alpha;\nsimple bad {}\n")
        result = self.check("src/inet/zeta/Zeta.ned", "src/inet/alpha/Alpha.ned")
        self.assertEqual(1, result.returncode)
        diagnostics = [line for line in result.stdout.splitlines() if line.startswith("src/")]
        self.assertEqual(diagnostics, sorted(diagnostics))

    def test_invalid_path_returns_usage_error(self) -> None:
        self.write("README.md", "fixture\n")
        result = self.check("README.md")
        self.assertEqual(2, result.returncode)

    def test_explicit_unsupported_file_returns_usage_error(self) -> None:
        self.write("src/inet/foo/Foo.cc", "// fixture\n")
        result = self.check("src/inet/foo/Foo.cc")
        self.assertEqual(2, result.returncode)
        self.assertIn("unsupported file type", result.stderr)


if __name__ == "__main__":
    unittest.main()
