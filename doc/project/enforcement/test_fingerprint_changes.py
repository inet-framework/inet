#!/usr/bin/env python3
"""Tests of the fingerprint categories (fingerprint_moves.py) and of the list that the baseline gate
(check-baselines.sh) wants in the message of a commit that changes tests/fingerprint/store.json.
"""

from pathlib import Path
import json
import subprocess
import sys
import tempfile
import unittest

HERE = Path(__file__).resolve().parent
sys.path.insert(0, str(HERE))
import fingerprint_moves  # noqa: E402
import test_check_baselines as base  # noqa: E402

STORE = "tests/fingerprint/store.json"
MOVES = HERE / "fingerprint_moves.py"
LIST_HEADING = "Baseline changes:"
FINGERPRINT_CHANGE = "Change: src | behavior.change | fingerprint"


def entry(folder="examples/a", fingerprint="1234-5678", ingredients="tplx", limit="10s", result="PASS",
          config="General", ini="omnetpp.ini", number=0, timestamp=1.0):
    """One entry of the store."""
    return {"working_directory": folder, "ini_file": ini, "config": config, "run_number": number,
            "sim_time_limit": limit, "test_result": result, "ingredients": ingredients,
            "fingerprint": fingerprint, "timestamp": timestamp, "itervars": ""}


def parsed(*entries):
    return fingerprint_moves.parse(json.dumps(list(entries)))


def lines_of(old, new):
    return [line for _, line in fingerprint_moves.categories(parsed(*old), parsed(*new))]


def kinds_of(old, new):
    return [kind for kind, _ in fingerprint_moves.categories(parsed(*old), parsed(*new))]


class FingerprintGateTestCase(base.GateTestCase):
    """An INET repository whose series changes the fingerprint store. Other test files use it too."""

    def setUp(self):
        super().setUp()
        self.series = "master..topic"

    def seed(self, entries, files=None):
        """Commits the store before the series, so that the series starts with this store."""
        self.write(self.inet, STORE, json.dumps(entries))
        for path, content in (files or {}).items():
            self.write(self.inet, path, content)
        self.git(self.inet, "add", "-A")
        self.git(self.inet, "commit", "-qm", "seed the store")
        self.series = f"{self.head(self.inet)}..topic"

    def store_text(self, revision):
        out = subprocess.run(["git", "show", f"{revision}:{STORE}"], cwd=self.inet, capture_output=True, text=True)
        return out.stdout if out.returncode == 0 else ""

    def append_source(self, count, path="src/inet/a/A.cc"):
        """Appends count lines to a source file. Each line is one changed line."""
        file = self.inet / path
        text = file.read_text(encoding="utf-8") if file.exists() else ""
        start = len(text.splitlines())
        file.write_text(text + "".join(f"int v{start + i};\n" for i in range(count)), encoding="utf-8")

    def computed_list(self, entries, reason=base.REASON):
        old = fingerprint_moves.parse(self.store_text("HEAD"))
        new = fingerprint_moves.parse(json.dumps(entries))
        return [line for _, line in fingerprint_moves.categories(old, new)], reason

    def store_commit(self, entries, subject=base.SUBJECT, block=None, source=0, trailer=base.CHANGE + "fingerprint",
                     reduction=None, trailers=(), files=None):
        """Commits a new store, the given number of source lines and the message. A block of None writes the
        computed list with a reason on each line, "none" leaves the list out, a function edits the lines of
        the list (the heading first). Returns the hash."""
        self.write(self.inet, STORE, json.dumps(entries))
        for path, content in (files or {}).items():
            self.write(self.inet, path, content)
        if source:
            self.append_source(source)
        lines, reason = self.computed_list(entries)
        block_lines = [LIST_HEADING] + [f"  {line} — {reason}" for line in lines]
        if block == "none":
            block_lines = []
        elif block is not None:
            block_lines = block(block_lines)
        parts = [subject, "The body.", "\n".join(block_lines)]
        if reduction is not None:
            parts.append("Reduction:\n" + reduction.rstrip("\n"))
        parts.append("\n".join(([trailer] if trailer else []) + list(trailers)))
        self.git(self.inet, "add", "-A")
        self.git(self.inet, "commit", "-q", "--allow-empty", "-m", "\n\n".join(part for part in parts if part))
        return self.head(self.inet)

    def fingerprint_gate(self, *args):
        return self.gate(*args, series=self.series)

    def violations(self, result):
        return [line.strip() for line in result.stdout.splitlines() if line.strip().startswith("VIOLATION")]


class CategoriesTest(unittest.TestCase):
    def test_a_run_that_is_only_in_the_new_store_is_added(self):
        found = fingerprint_moves.categories(parsed(entry("examples/a")),
                                             parsed(entry("examples/a"), entry("examples/b"), entry("showcases/c")))
        self.assertEqual(found, [("added", "fingerprint runs added (2; examples/b 1, showcases/c 1)")])

    def test_a_run_that_is_only_in_the_old_store_is_removed(self):
        found = fingerprint_moves.categories(parsed(entry("examples/a"), entry("examples/b")), parsed(entry("examples/a")))
        self.assertEqual(found, [("removed", "fingerprint runs removed (1; examples/b 1)")])

    def test_a_removed_and_an_added_run_with_equal_fingerprints_is_a_rename(self):
        old = [entry("examples/a"), entry("examples/a", ingredients="x", fingerprint="aaaa-bbbb")]
        new = [entry("examples/b", ini="other.ini", config="Fast", number=3),
               entry("examples/b", ini="other.ini", config="Fast", number=3, ingredients="x", fingerprint="aaaa-bbbb")]
        self.assertEqual(lines_of(old, new), ["fingerprint runs renamed (1): examples/a -> examples/b -f other.ini -c Fast -r 3"])

    def test_a_rename_names_the_default_ini_config_and_run_number_by_the_folder_only(self):
        self.assertEqual(fingerprint_moves.run_text(("examples/a", "omnetpp.ini", "General", "0")), "examples/a")
        self.assertEqual(fingerprint_moves.run_text(("examples/a", "x.ini", "General", "0")), "examples/a -f x.ini")
        self.assertEqual(fingerprint_moves.run_text(("examples/a", "omnetpp.ini", "Fast", "2")), "examples/a -c Fast -r 2")

    def test_runs_with_different_fingerprints_are_no_rename(self):
        found = lines_of([entry("examples/a")], [entry("examples/b", fingerprint="9999-9999")])
        self.assertEqual(found, ["fingerprint runs added (1; examples/b 1)", "fingerprint runs removed (1; examples/a 1)"])

    def test_changed_runs_are_grouped_by_the_pattern_of_changed_sets_and_the_larger_group_comes_first(self):
        old = [entry(f"examples/{name}", ingredients=sets, fingerprint="1111-1111")
               for name in "abc" for sets in ("tplx", "x")] + [entry("examples/d", ingredients="tplx")]
        new = [entry(f"examples/{name}", ingredients="tplx", fingerprint="2222-2222") for name in "ab"]
        new += [entry(f"examples/{name}", ingredients="x", fingerprint="1111-1111") for name in "ab"]
        new += [entry("examples/c", ingredients=sets, fingerprint="2222-2222") for sets in ("tplx", "x")]
        new += [entry("examples/d", ingredients="tplx", fingerprint="2222-2222")]
        self.assertEqual(lines_of(old, new), ["fingerprint runs changed in tplx (3; examples/a 1, examples/b 1, examples/d 1)",
                                              "fingerprint runs changed in tplx,x (1; examples/c 1)"])

    def test_ingredient_sets_that_come_are_added(self):
        found = fingerprint_moves.categories(parsed(entry()), parsed(entry(), entry(ingredients="x", fingerprint="aaaa-bbbb")))
        self.assertEqual(found, [("sets added", "fingerprint ingredient sets added (1; examples/a 1): x")])

    def test_ingredient_sets_that_go_are_removed(self):
        found = fingerprint_moves.categories(parsed(entry(), entry(ingredients="x", fingerprint="aaaa-bbbb")), parsed(entry()))
        self.assertEqual(found, [("sets removed", "fingerprint ingredient sets removed (1; examples/a 1): x")])

    def test_a_change_of_only_the_limit_has_another_limit_or_expected_result(self):
        found = fingerprint_moves.categories(parsed(entry()), parsed(entry(limit="20s")))
        self.assertEqual(found, [("other", "fingerprint runs with another limit or expected result (1; examples/a 1)")])

    def test_a_change_of_only_the_expected_result_has_another_limit_or_expected_result(self):
        found = fingerprint_moves.categories(parsed(entry()), parsed(entry(result="ERROR")))
        self.assertEqual(found, [("other", "fingerprint runs with another limit or expected result (1; examples/a 1)")])

    def test_a_change_of_only_the_timestamp_gives_no_category(self):
        self.assertEqual(fingerprint_moves.categories(parsed(entry(timestamp=1.0)), parsed(entry(timestamp=2.0))), [])

    def test_equal_stores_and_empty_stores_give_no_category(self):
        self.assertEqual(fingerprint_moves.categories(parsed(entry()), parsed(entry())), [])
        self.assertEqual(fingerprint_moves.categories(fingerprint_moves.parse(""), fingerprint_moves.parse("")), [])

    def test_the_kinds_come_in_a_fixed_order(self):
        old = [entry("examples/renamed"), entry("examples/removed", fingerprint="0000-0001"), entry("examples/changed"),
               entry("examples/more"), entry("examples/more", ingredients="x", fingerprint="aaaa-bbbb"),
               entry("examples/less"), entry("examples/less", ingredients="x", fingerprint="aaaa-bbbb"),
               entry("examples/limit"), entry("examples/same")]
        new = [entry("examples/renamed2"), entry("examples/added", fingerprint="0000-0002"),
               entry("examples/changed", fingerprint="9999-9999"),
               entry("examples/more"), entry("examples/more", ingredients="x", fingerprint="aaaa-bbbb"),
               entry("examples/more", ingredients="y", fingerprint="cccc-dddd"),
               entry("examples/less"), entry("examples/limit", limit="99s"), entry("examples/same")]
        self.assertEqual(kinds_of(old, new), ["added", "removed", "renamed", "changed", "sets added", "sets removed", "other"])

    def test_the_folders_text_counts_runs_by_folder_and_names_the_most_frequent_first(self):
        from baseline_summary import folders_text
        self.assertEqual(folders_text(["examples/a/x", "examples/b/y", "examples/a/z"]), "3; examples/a 2, examples/b 1")
        found = lines_of([], [entry("examples/a", number=n) for n in range(2)] + [entry("showcases/b")])
        self.assertEqual(found, ["fingerprint runs added (3; examples/a 2, showcases/b 1)"])


class CommandLineTest(unittest.TestCase):
    def run_moves(self, old, new, *options):
        with tempfile.TemporaryDirectory() as folder:
            paths = []
            for name, entries in (("old.json", old), ("new.json", new)):
                path = Path(folder) / name
                path.write_text(json.dumps(entries) if entries is not None else "", encoding="utf-8")
                paths.append(str(path))
            return subprocess.run([sys.executable, str(MOVES), *options, *paths], capture_output=True, text=True)

    def test_it_prints_the_runs_and_the_pattern_of_each_change_as_before(self):
        old = [entry(f"examples/{name}", ingredients=sets, fingerprint="1111-1111") for name in "abc" for sets in ("tplx", "x")]
        old += [entry("examples/d"), entry("examples/e")]
        new = [entry(f"examples/{name}", ingredients="tplx", fingerprint="2222-2222") for name in "ab"]
        new += [entry(f"examples/{name}", ingredients="x", fingerprint="1111-1111") for name in "ab"]
        new += [entry("examples/c", ingredients=sets, fingerprint="2222-2222") for sets in ("tplx", "x")]
        new += [entry("examples/d", limit="20s"), entry("examples/f")]  # d keeps its values, e is retired, f is new
        result = self.run_moves(old, new)
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, "2\ttplx\n1\t(other fields)\n1\ttplx,x\n")

    def test_an_empty_old_store_gives_no_output(self):
        result = self.run_moves(None, [entry()])
        self.assertEqual((result.returncode, result.stdout), (0, ""))

    def test_the_list_option_prints_the_list_for_the_message_with_an_empty_reason_on_each_line(self):
        old = [entry("examples/a"), entry("examples/b")]
        new = [entry("examples/a", fingerprint="9999-9999"), entry("examples/b"), entry("examples/c", fingerprint="5555-5555")]
        result = self.run_moves(old, new, "--list")
        self.assertEqual(result.returncode, 0, result.stderr)
        self.assertEqual(result.stdout, f"{LIST_HEADING}\n"
                                        "  fingerprint runs added (1; examples/c 1) — \n"
                                        "  fingerprint runs changed in tplx (1; examples/a 1) — \n")


class ListGateTest(FingerprintGateTestCase):
    def test_a_commit_that_changes_the_store_with_no_list_fails_and_prints_the_list(self):
        self.seed([entry("examples/a")])
        self.store_commit([entry("examples/a"), entry("examples/b")], block="none")
        result = self.fingerprint_gate()
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("fingerprints has no 'Baseline changes:' list", result.stdout)
        self.assertIn("  fingerprint runs added (1; examples/b 1) — ", result.stdout)

    def test_a_commit_with_the_computed_list_and_a_reason_on_each_line_passes(self):
        self.seed([entry("examples/a")])
        self.store_commit([entry("examples/a"), entry("examples/b")])
        result = self.fingerprint_gate()
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertEqual(self.violations(result), [])
        self.assertIn("1 commit(s) change fingerprint entries", result.stdout)

    def test_a_missing_reason_gives_one_violation_that_names_the_line(self):
        self.seed([entry("examples/a"), entry("examples/c", fingerprint="7777-7777")])
        self.store_commit([entry("examples/a"), entry("examples/b")],
                          block=lambda lines: [line.rsplit(" — ", 1)[0] + " — " if "removed" in line else line for line in lines])
        result = self.fingerprint_gate()
        self.assertEqual(result.returncode, 1, result.stdout)
        found = self.violations(result)
        self.assertEqual(len(found), 1, result.stdout)
        self.assertIn("no reason for: fingerprint runs removed (1; examples/c 1)", found[0])

    def test_a_list_in_another_order_fails(self):
        self.seed([entry("examples/a"), entry("examples/c", fingerprint="7777-7777")])
        self.store_commit([entry("examples/a"), entry("examples/b")], block=lambda lines: [lines[0], *reversed(lines[1:])])
        result = self.fingerprint_gate()
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("fingerprints has a 'Baseline changes:' list that is not the computed list", result.stdout)

    def test_a_list_with_a_line_that_is_not_computed_fails(self):
        self.seed([entry("examples/a")])
        self.store_commit([entry("examples/a"), entry("examples/b")],
                          block=lambda lines: [*lines, "  fingerprint runs removed (1; examples/z 1) — invented"])
        self.assertIn("that is not the computed list", self.fingerprint_gate().stdout)

    def test_a_trailer_that_does_not_name_fingerprint_fails(self):
        self.seed([entry("examples/a")])
        self.store_commit([entry("examples/a"), entry("examples/b")], trailer=base.CHANGE + "statistical")
        result = self.fingerprint_gate()
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("changes the fingerprint store, and its Change: trailer does not name 'fingerprint' (CR-OBL-BASELINE)",
                      result.stdout)

    def test_a_trailer_with_no_obligations_fails(self):
        self.seed([entry("examples/a")])
        self.store_commit([entry("examples/a"), entry("examples/b")], trailer=base.CHANGE + "-")
        self.assertIn("(CR-OBL-BASELINE)", self.fingerprint_gate().stdout)

    def test_a_trailer_with_a_question_mark_passes(self):
        self.seed([entry("examples/a")])
        self.store_commit([entry("examples/a"), entry("examples/b")], trailer=base.CHANGE + "?")
        result = self.fingerprint_gate()
        self.assertNotIn("CR-OBL-BASELINE", result.stdout)
        self.assertEqual(result.returncode, 0, result.stdout)

    def test_a_trailer_that_names_fingerprint_among_others_passes(self):
        self.seed([entry("examples/a")])
        self.store_commit([entry("examples/a"), entry("examples/b")], trailer=base.CHANGE + "statistical fingerprint")
        self.assertNotIn("CR-OBL-BASELINE", self.fingerprint_gate().stdout)

    def test_a_commit_with_no_trailer_has_no_obligation_violation(self):
        self.seed([entry("examples/a")])
        self.store_commit([entry("examples/a"), entry("examples/b")], trailer=None)
        result = self.fingerprint_gate()
        self.assertNotIn("CR-OBL-BASELINE", result.stdout)
        self.assertEqual(result.returncode, 0, result.stdout)

    def test_a_commit_that_does_not_touch_the_store_needs_no_list(self):
        self.seed([entry("examples/a")])
        self.append_source(1)
        self.git(self.inet, "add", "-A")
        self.git(self.inet, "commit", "-qm", f"{base.SUBJECT}\n\nThe body.\n\n{FINGERPRINT_CHANGE}")
        result = self.fingerprint_gate()
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn("0 commit(s) change fingerprint entries", result.stdout)

    def test_a_change_of_only_the_timestamp_needs_no_list(self):
        self.seed([entry("examples/a", timestamp=1.0)])
        self.store_commit([entry("examples/a", timestamp=2.0)], block="none")
        result = self.fingerprint_gate()
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn("0 commit(s) change fingerprint entries", result.stdout)

    def test_a_store_that_does_not_exist_at_the_parent_has_all_runs_added(self):
        self.store_commit([entry("examples/a"), entry("showcases/b")], block="none")
        result = self.fingerprint_gate()
        self.assertEqual(result.returncode, 1, result.stdout)
        self.assertIn("  fingerprint runs added (2; examples/a 1, showcases/b 1) — ", result.stdout)

    def test_a_store_that_does_not_exist_at_the_parent_passes_with_the_list_of_added_runs(self):
        self.store_commit([entry("examples/a"), entry("showcases/b")])
        result = self.fingerprint_gate()
        self.assertEqual(result.returncode, 0, result.stdout)

    def test_more_than_one_changed_line_gives_a_note(self):
        self.seed([entry("examples/a"), entry("examples/b", ingredients="x")])
        self.store_commit([entry("examples/a", fingerprint="2222-2222"),
                           entry("examples/b", ingredients="x", fingerprint="3333-3333")], source=1)
        result = self.fingerprint_gate()
        self.assertEqual(result.returncode, 0, result.stdout)
        self.assertIn("note: ", result.stdout)
        self.assertIn("has 2 lines of changed entries", result.stdout)

    def test_one_changed_line_gives_no_note_about_changed_entries(self):
        self.seed([entry("examples/a")])
        self.store_commit([entry("examples/a", fingerprint="2222-2222")], source=1)
        result = self.fingerprint_gate()
        self.assertNotIn("lines of changed entries", result.stdout)
        self.assertEqual(result.returncode, 0, result.stdout)


if __name__ == "__main__":
    unittest.main()
